/* $Id$ */
/* Author: Wolfgang Bangerth, Texas A&M University, 2007 */

/*    $Id$       */
/*                                                                */
/*    Copyright (C) 2007, 2008 by the deal.II authors */
/*                                                                */
/*    This file is subject to QPL and may not be  distributed     */
/*    without copyright and license information. Please refer     */
/*    to the file deal.II/doc/license.html for the  text  and     */
/*    further information on this license.                        */

				 // @sect3{Include files}

				 // We include the functionality
				 // of these well-known deal.II
				 // library files and some C++
				 // header files.
#include <base/quadrature_lib.h>
#include <base/logstream.h>
#include <base/function.h>
#include <base/utilities.h>

#include <lac/block_vector.h>
#include <lac/full_matrix.h>
#include <lac/block_sparse_matrix.h>
#include <lac/solver_gmres.h>
#include <lac/solver_cg.h>
#include <lac/precondition.h>
#include <lac/sparse_direct.h>
#include <lac/sparse_ilu.h>
#include <lac/block_matrix_array.h>

#include <grid/tria.h>
#include <grid/grid_generator.h>
#include <grid/tria_accessor.h>
#include <grid/tria_iterator.h>
#include <grid/tria_boundary_lib.h>
#include <grid/grid_tools.h>
#include <grid/grid_refinement.h>

#include <dofs/dof_handler.h>
#include <dofs/dof_renumbering.h>
#include <dofs/dof_accessor.h>
#include <dofs/dof_tools.h>
#include <dofs/dof_constraints.h>

#include <fe/fe_q.h>
#include <fe/fe_system.h>
#include <fe/fe_values.h>
#include <fe/mapping_q1.h>

#include <numerics/vectors.h>
#include <numerics/matrices.h>
#include <numerics/data_out.h>
#include <numerics/error_estimator.h>
#include <numerics/solution_transfer.h>

#include <fstream>
#include <sstream>

                                 // This is Trilinos
#include <Epetra_SerialComm.h>
#include <Epetra_Map.h>
#include <Epetra_CrsMatrix.h>
#include <Epetra_Vector.h>
#include <Teuchos_ParameterList.hpp>
#include <ml_include.h>
#include <ml_MultiLevelPreconditioner.h>

				 // Next, we import all deal.II
				 // names into global namespace
using namespace dealii;


				 // @sect3{Defining the AMG preconditioner}

				 // This implements an AMG
				 // preconditioner based on the
				 // Trilinos ML implementation.
				 // What this class does is twofold.
				 // When the constructor of the class
				 // is invoked, a ML preconditioner
				 // object is created based on the
				 // DoFHandler and matrix
				 // that we want the preconditioner to
				 // be based on. A call of
				 // the respective <code>vmult</code>
				 // function does call the respective
				 // operation in the Trilinos package,
				 // where it is called 
				 // <code>ApplyInverse</code>.
 
				 // There are a few pecularities in
				 // the constructor. Since the
				 // Trilinos objects we want to use are
				 // heavily dependent on Epetra objects,
				 // the fundamental construction
				 // routines for vectors and 
				 // matrices in Trilinos, we do a 
				 // copy of our deal.II preconditioner
				 // matrix to a Epetra matrix. This 
				 // is of course not optimal, but for
				 // the time being there is no direct
				 // support for our data interface.
				 // When doing this time-consuming 
				 // operation, we can still profit 
				 // from the fact that some of the
				 // entries in the preconditioner matrix
				 // are zero and hence can be 
				 // neglected.
class PreconditionerTrilinosAmg : public Subscriptor
{
  public:
    PreconditionerTrilinosAmg ();
    
    void initialize (const SparseMatrix<double> &preconditioner_matrix,
		     const std::vector<double>  &null_space,
		     const unsigned int          null_space_dimension,
		     const bool                  higher_order_elements,
		     const bool                  elliptic,
		     const bool                  output_details);

    void vmult (Vector<double>       &dst,
		const Vector<double> &src) const;

  private:
    
    ML_Epetra::MultiLevelPreconditioner* ml_precond;
    
    Epetra_SerialComm  communicator;
    std::auto_ptr<Epetra_Map>       Map;
    std::auto_ptr<Epetra_CrsMatrix> Matrix;
};


PreconditionerTrilinosAmg::PreconditionerTrilinosAmg ()
{}

void PreconditionerTrilinosAmg::initialize (
		const SparseMatrix<double> &preconditioner_matrix,
		const std::vector<double>  &null_space,
		const unsigned int          null_space_dimension,
		const bool                  elliptic,
		const bool                  higher_order_elements,
		const bool                  output_details
		)
{
  unsigned int n_u = preconditioner_matrix.m();
  const SparsityPattern *sparsity_pattern = 
      &(preconditioner_matrix.get_sparsity_pattern());
  
				 // Init Epetra Matrix, avoid 
				 // storing the nonzero elements.
  {
    Map.reset (new Epetra_Map(n_u, 0, communicator));
    
    std::vector<int> row_lengths (n_u);
    for (unsigned int row=0; row<n_u; ++row)
      {
	const unsigned int temporary_row_length = 
	    sparsity_pattern->row_length (row);
	unsigned int local_length = 0;
	for (unsigned int col=0; col<temporary_row_length; ++col)
	  {
	    unsigned int col_index = sparsity_pattern->column_number (row, col);
	    if (std::abs(preconditioner_matrix (row, col_index)) > 1e-13)
	      local_length += 1;
	  }
	row_lengths[row] = local_length;
      }
  
    Matrix.reset (new Epetra_CrsMatrix(Copy, *Map, &row_lengths[0], true));
  
    const unsigned int max_nonzero_entries
      = *std::max_element (row_lengths.begin(), row_lengths.end());
  
    std::vector<double> values(max_nonzero_entries, 0);
    std::vector<int> row_indices(max_nonzero_entries);
  
    for (unsigned int row=0; row<n_u; ++row)
      {
	const unsigned int temporary_row_length = 
	    sparsity_pattern->row_length (row);
	
	row_indices.resize (row_lengths[row], 0);
	values.resize (row_lengths[row], 0.);
  
	unsigned int col_counter = 0;
	for (unsigned int col=0; col<temporary_row_length; ++col)
	  {
	    unsigned int col_index = sparsity_pattern->column_number (row, col);
	    if (std::abs(preconditioner_matrix (row, col_index)) > 1e-13)
	      {
		row_indices[col_counter] = 
		    sparsity_pattern->column_number (row, col);
		values[col_counter] = 
		    preconditioner_matrix (row, row_indices[col_counter]);
		++col_counter;
	      }
	  }
	Assert (col_counter == static_cast<unsigned int>(row_lengths[row]),
		ExcMessage("Filtering out zeros could not "
			    "be successfully finished!"));
  
	Matrix->InsertGlobalValues(row, row_lengths[row],
				   &values[0], &row_indices[0]);
      }
      
    Matrix->FillComplete();
  }
  
				 // Build the AMG preconditioner.
  Teuchos::ParameterList MLList;
  
				 // The implementation is able
				 // to distinguish between
				 // matrices from elliptic problems
				 // and convection dominated 
				 // problems. We use the standard
				 // options for elliptic problems,
				 // except that we use a 
				 // Chebyshev smoother instead
				 // of a symmetric Gauss-Seidel
				 // smoother. For most elliptic 
				 // problems, Chebyshev is better
				 // than Gauss-Seidel (SSOR).
  if (elliptic)
    {
      ML_Epetra::SetDefaults("SA",MLList);
      MLList.set("smoother: type", "Chebyshev");
      MLList.set("smoother: sweeps", 4);
    }
  else
    {
      ML_Epetra::SetDefaults("NSSA",MLList);
      MLList.set("aggregation: type", "Uncoupled");
      MLList.set("aggregation: block scaling", true);
    }
  
  if (output_details)
    MLList.set("ML output", 10);
  else
    MLList.set("ML output", 0);
  
  if (higher_order_elements)
    MLList.set("aggregation: type", "MIS");
  
  Assert (n_u * null_space_dimension == null_space.size(),
	  ExcDimensionMismatch(n_u * null_space_dimension,
			       null_space.size()));
  
  if (null_space_dimension > 1)
    {
      MLList.set("null space: type", "pre-computed");
      MLList.set("null space: dimension", int(null_space_dimension));
      MLList.set("null space: vectors", (double *)&null_space[0]);
    }
  
  ml_precond = new ML_Epetra::MultiLevelPreconditioner(*Matrix, MLList, true);

  if (output_details)
    ml_precond->PrintUnused(0);
}

				 // For the implementation of the
				 // <code>vmult</code> function we
				 // note that invoking a call of 
				 // the Trilinos preconditioner 
				 // requires us to use Epetra vectors
				 // as well. Luckily, it is sufficient
				 // to provide a view, i.e., feed 
				 // Trilinos with a pointer to the
				 // data, so we avoid copying the
				 // content of the vectors during
				 // the iteration. In the declaration
				 // of the right hand side, we need
				 // to cast the source vector (that
				 // is <code>const</code> in all deal.II 
				 // calls) to non-constant value, as
				 // this is the way Trilinos wants to
				 // have them.
void PreconditionerTrilinosAmg::vmult (Vector<double>       &dst,
					    const Vector<double> &src) const
{
  Epetra_Vector LHS (View, *Map, dst.begin());
  Epetra_Vector RHS (View, *Map, const_cast<double*>(src.begin()));
  
  int res = ml_precond->ApplyInverse (RHS, LHS);
  
  Assert (res == 0,
	  ExcMessage ("Trilinos AMG MultiLevel preconditioner returned "
		      "errorneously!"));
}



				 // @sect3{The <code>BoussinesqFlowProblem</code> class template}

				 // The definition of this class is
				 // mainly based on the step-22 tutorial
				 // program. Most of the data types are
				 // the same as there. However, we
				 // deal with a time-dependent system now,
				 // and there is temperature to take care
				 // of as well, so we need some additional
				 // function and variable declarations.
				 // Furthermore, we have a slightly more
				 // sophisticated solver we are going to
				 // use, so there is a second pointer
				 // to a sparse ILU for a pressure
				 // mass matrix as well.
template <int dim>
class BoussinesqFlowProblem
{
  public:
    BoussinesqFlowProblem (const unsigned int degree);
    void run ();

  private:
    void setup_dofs ();
    void assemble_stokes_preconditioner ();
    void build_stokes_preconditioner ();
    void assemble_stokes_system ();
    void assemble_temperature_system ();
    double get_maximal_velocity () const;
    double get_maximal_temperature () const;
    void solve ();
    void output_results () const;
    void refine_mesh (const unsigned int max_grid_level);

    Triangulation<dim>        triangulation;

    const unsigned int        degree;
    
    FESystem<dim>             stokes_fe;
    DoFHandler<dim>           stokes_dof_handler;
    ConstraintMatrix          stokes_constraints;
    
    BlockSparsityPattern      stokes_sparsity_pattern;
    BlockSparseMatrix<double> stokes_matrix;
    BlockSparsityPattern      stokes_preconditioner_sparsity_pattern;
    BlockSparseMatrix<double> stokes_preconditioner_matrix;

    BlockVector<double>       stokes_solution;
    BlockVector<double>       stokes_rhs;

    
    FE_Q<dim>                 temperature_fe;
    DoFHandler<dim>           temperature_dof_handler;
    ConstraintMatrix          temperature_constraints;
    
    SparsityPattern           temperature_sparsity_pattern;
    SparseMatrix<double>      temperature_matrix;

    Vector<double>            temperature_solution;
    Vector<double>            old_temperature_solution;
    Vector<double>            old_old_temperature_solution;
    Vector<double>            temperature_rhs;


    double time_step;
    double old_time_step;
    unsigned int timestep_number;

    boost::shared_ptr<PreconditionerTrilinosAmg>  Amg_preconditioner;
    boost::shared_ptr<SparseILU<double> > Mp_preconditioner;

    bool rebuild_matrices;
    bool rebuild_preconditioner;
};




				 // @sect3{Equation data}

				 // Again, the next stage in the program
				 // is the definition of the equation 
				 // data, that is, the various
				 // boundary conditions, the right hand
				 // side and the initial condition (remember
				 // that we're about to solve a time-
				 // dependent system). The basic strategy
				 // for this definition is the same as in
				 // step-22. Regarding the details, though,
				 // there are some differences.

				 // The first
				 // thing is that we don't set any boundary
				 // conditions on the velocity, as is
				 // explained in the introduction. So
				 // what is left are two conditions for
				 // pressure <i>p</i> and temperature
				 // <i>T</i>.

				 // Secondly, we set an initial
				 // condition for all problem variables,
				 // i.e., for <b>u</b>, <i>p</i> and <i>T</i>,
				 // so the function has <i>dim+2</i>
				 // components.
				 // In this case, we choose a very simple
				 // test case, where everything is zero.

				 // @sect4{Boundary values}
template <int dim>
class PressureBoundaryValues : public Function<dim>
{
  public:
    PressureBoundaryValues () : Function<dim>(1) {}

    virtual double value (const Point<dim>   &p,
                          const unsigned int  component = 0) const;
};


template <int dim>
double
PressureBoundaryValues<dim>::value (const Point<dim>  &/*p*/,
                                    const unsigned int /*component*/) const
{
  return 0;
}



template <int dim>
class TemperatureBoundaryValues : public Function<dim>
{
  public:
    TemperatureBoundaryValues () : Function<dim>(1) {}

    virtual double value (const Point<dim>   &p,
                          const unsigned int  component = 0) const;
};



template <int dim>
double
TemperatureBoundaryValues<dim>::value (const Point<dim> &p,
                                      const unsigned int /*component*/) const
{
//TODO: leftover from olden times. replace by something sensible once we have
//diffusion in the temperature field
  if (p[0] == 0)
    return 1;
  else
    return 0;
}



				 // @sect4{Initial values}
template <int dim>
class TemperatureInitialValues : public Function<dim>
{
  public:
    TemperatureInitialValues () : Function<dim>(1) {}

    virtual double value (const Point<dim>   &p,
                          const unsigned int  component = 0) const;

    virtual void vector_value (const Point<dim> &p,
                               Vector<double>   &value) const;
};


template <int dim>
double
TemperatureInitialValues<dim>::value (const Point<dim>  &,
                           const unsigned int) const
{
  return 0;
}


template <int dim>
void
TemperatureInitialValues<dim>::vector_value (const Point<dim> &p,
                                  Vector<double>   &values) const
{
  for (unsigned int c=0; c<this->n_components; ++c)
    values(c) = TemperatureInitialValues<dim>::value (p, c);
}



				 // @sect4{Right hand side}
				 // 
				 // The last definition of this kind
				 // is the one for the right hand
				 // side function. Again, the content
				 // of the function is very
				 // basic and zero in most of the
				 // components, except for a source
				 // of temperature in some isolated
				 // regions near the bottom of the
				 // computational domain, as is explained
				 // in the problem description in the
				 // introduction.
template <int dim>
class TemperatureRightHandSide : public Function<dim>
{
  public:
    TemperatureRightHandSide () : Function<dim>(1) {}

    virtual double value (const Point<dim>   &p,
                          const unsigned int  component = 0) const;

    virtual void vector_value (const Point<dim> &p,
                               Vector<double>   &value) const;
};


template <int dim>
double
TemperatureRightHandSide<dim>::value (const Point<dim>  &p,
				      const unsigned int /*component*/) const
{
  static const Point<dim> source_centers[3]
    = { (dim == 2 ? Point<dim>(.3,.1) : Point<dim>(.3,.5,.1)),
	(dim == 2 ? Point<dim>(.45,.1) : Point<dim>(.45,.5,.1)),
	(dim == 2 ? Point<dim>(.75,.1) : Point<dim>(.75,.5,.1)) };
  static const double source_radius
    = (dim == 2 ? 1./32 : 1./8);
      
  return ((source_centers[0].distance (p) < source_radius)
	  ||
	  (source_centers[1].distance (p) < source_radius)
	  ||
	  (source_centers[2].distance (p) < source_radius)
	  ?
	  1
	  :
	  0);
}


template <int dim>
void
TemperatureRightHandSide<dim>::vector_value (const Point<dim> &p,
					     Vector<double>   &values) const
{
  for (unsigned int c=0; c<this->n_components; ++c)
    values(c) = TemperatureRightHandSide<dim>::value (p, c);
}




				 // @sect3{Linear solvers and preconditioners}

				 // This section introduces some
				 // objects that are used for the
				 // solution of the linear equations of
				 // Stokes system that we need to
				 // solve in each time step. The basic
				 // structure is still the same as
				 // in step-20, where Schur complement
				 // based preconditioners and solvers
				 // have been introduced, with the 
				 // actual interface taken from step-22.

				 // @sect4{The <code>InverseMatrix</code> class template}

				 // This class is an interface to
				 // calculate the action of an
				 // "inverted" matrix on a vector
				 // (using the <code>vmult</code>
				 // operation)
				 // in the same way as the corresponding
				 // function in step-22: when the
				 // product of an object of this class
				 // is requested, we solve a linear
				 // equation system with that matrix
				 // using the CG method, accelerated
				 // by a preconditioner of (templated) class
				 // <code>Preconditioner</code>.
template <class Matrix, class Preconditioner>
class InverseMatrix : public Subscriptor
{
  public:
    InverseMatrix (const Matrix         &m,
		   const Preconditioner &preconditioner);

    void vmult (Vector<double>       &dst,
                const Vector<double> &src) const;

  private:
    const SmartPointer<const Matrix> matrix;
    const Preconditioner &preconditioner;
};


template <class Matrix, class Preconditioner>
InverseMatrix<Matrix,Preconditioner>::InverseMatrix (const Matrix &m,
						     const Preconditioner &preconditioner)
                :
                matrix (&m),
		preconditioner (preconditioner)
{}



template <class Matrix, class Preconditioner>
void InverseMatrix<Matrix,Preconditioner>::vmult (Vector<double>       &dst,
						  const Vector<double> &src) const
{
  SolverControl solver_control (src.size(), 1e-6*src.l2_norm());
  SolverCG<> cg (solver_control);

  dst = 0;

  try
    {
      cg.solve (*matrix, dst, src, preconditioner);
    }
  catch (std::exception &e)
    {
      Assert (false, ExcMessage(e.what()));
    }
}

				 // @sect4{Schur complement preconditioner}

				 // This is the implementation
				 // of the Schur complement
				 // preconditioner as described
				 // in the section on improved
				 // solvers in step-22.
				 // 
				 // The basic 
				 // concept of the preconditioner is 
				 // different to the solution 
				 // strategy used in step-20 and 
				 // step-22. There, the Schur
				 // complement was used for a 
				 // two-stage solution of the linear
				 // system. Recall that the process
				 // in the Schur complement solver is
				 // a Gaussian elimination of
				 // a 2x2 block matrix, where each
				 // block is solved iteratively. 
				 // Here, the idea is to let 
				 // an iterative solver act on the
				 // whole system, and to use 
				 // a Schur complement for 
				 // preconditioning. As usual when
				 // dealing with preconditioners, we
				 // don't intend to exacly set up a 
				 // Schur complement, but rather use
				 // a good approximation to the
				 // Schur complement for the purpose of
				 // preconditioning.
				 // 
				 // So the question is how we can
				 // obtain a good preconditioner.
				 // Let's have a look at the 
				 // preconditioner matrix <i>P</i>
				 // acting on the block system, built
				 // as
				 // @f{eqnarray*}
				 //   P^{-1}
				 //   = 
				 //   \left(\begin{array}{cc}
				 //     A^{-1} & 0 \\ S^{-1} B A^{-1} & -S^{-1}
				 //   \end{array}\right)
				 // @f}
				 // using the Schur complement 
				 // $S = B A^{-1} B^T$. If we apply
				 // this matrix in the solution of 
				 // a linear system, convergence of
				 // an iterative Krylov-based solver
				 // will be governed by the matrix
				 // @f{eqnarray*}
				 //   P^{-1}\left(\begin{array}{cc}
				 //     A & B^T \\ B & 0
				 //   \end{array}\right) 
 				 //  = 
				 //   \left(\begin{array}{cc}
				 //     I & A^{-1} B^T \\ 0 & 0
				 //   \end{array}\right),
				 // @f}
				 // which turns out to be very simple.
				 // A GMRES solver based on exact
				 // matrices would converge in two
				 // iterations, since there are
				 // only two distinct eigenvalues.
				 // Such a preconditioner for the
				 // blocked Stokes system has been 
				 // proposed by Silvester and Wathen,
				 // Fast iterative solution of 
				 // stabilised Stokes systems part II. 
				 // Using general block preconditioners.
				 // (SIAM J. Numer. Anal., 31 (1994),
				 // pp. 1352-1367).
				 // 
				 // The deal.II users who have already
				 // gone through the step-20 and step-22 
				 // tutorials can certainly imagine
				 // how we're going to implement this.
				 // We replace the inverse matrices
				 // in $P^{-1}$ using the InverseMatrix
				 // class, and the inverse Schur 
				 // complement will be approximated
				 // by the pressure mass matrix $M_p$.
				 // Having this in mind, we define a
				 // preconditioner class with a 
				 // <code>vmult</code> functionality,
				 // which is all we need for the
				 // interaction with the usual solver
				 // functions further below in the
				 // program code.
				 // 
				 // First the declarations. These
				 // are similar to the definition of
				 // the Schur complement in step-20,
				 // with the difference that we need
				 // some more preconditioners in
				 // the constructor.
template <class PreconditionerA, class PreconditionerMp>
class BlockSchurPreconditioner : public Subscriptor
{
  public:
    BlockSchurPreconditioner (const BlockSparseMatrix<double>         &S,
          const InverseMatrix<SparseMatrix<double>,PreconditionerMp>  &Mpinv,
          const PreconditionerA &Apreconditioner);

  void vmult (BlockVector<double>       &dst,
              const BlockVector<double> &src) const;

  private:
    const SmartPointer<const BlockSparseMatrix<double> > stokes_matrix;
    const SmartPointer<const InverseMatrix<SparseMatrix<double>,
                       PreconditionerMp > > m_inverse;
    const PreconditionerA &a_preconditioner;

    mutable Vector<double> tmp;

};

template <class PreconditionerA, class PreconditionerMp>
BlockSchurPreconditioner<PreconditionerA, PreconditionerMp>::BlockSchurPreconditioner(
          const BlockSparseMatrix<double>                            &S,
          const InverseMatrix<SparseMatrix<double>,PreconditionerMp> &Mpinv,
          const PreconditionerA &Apreconditioner
          )
                :
                stokes_matrix           (&S),
                m_inverse               (&Mpinv),
                a_preconditioner        (Apreconditioner),
                tmp                     (S.block(1,1).m())
{
}


				 // This is the <code>vmult</code>
				 // function. We implement
				 // the action of $P^{-1}$ as described
				 // above in three successive steps.
				 // The first step multiplies
				 // the velocity vector by a 
				 // preconditioner of the matrix <i>A</i>.
				 // The resuling velocity vector
				 // is then multiplied by $B$ and
				 // subtracted from the pressure.
				 // This second step only acts on 
				 // the pressure vector and is 
				 // accomplished by the command
				 // SparseMatrix::residual. Next, 
				 // we change the sign in the 
				 // temporary pressure vector and
				 // finally multiply by the pressure
				 // mass matrix to get the final
				 // pressure vector.
template <class PreconditionerA, class PreconditionerMp>
void BlockSchurPreconditioner<PreconditionerA, PreconditionerMp>::vmult (
                                     BlockVector<double>       &dst,
                                     const BlockVector<double> &src) const
{
  a_preconditioner.vmult (dst.block(0), src.block(0));
  stokes_matrix->block(1,0).residual(tmp, dst.block(0), src.block(1));
  tmp *= -1;
  m_inverse->vmult (dst.block(1), tmp);
}



				 // @sect3{BoussinesqFlowProblem class implementation}

				 // @sect4{BoussinesqFlowProblem::BoussinesqFlowProblem}
				 // 
				 // The constructor of this class is
				 // an extension of the constructor
				 // in step-22. We need to include 
				 // the temperature in the definition
				 // of the finite element. As discussed
				 // in the introduction, we are going 
				 // to use discontinuous elements 
				 // of one degree less than for pressure
				 // there. Moreover, we initialize
				 // the time stepping as well as the
				 // options for the matrix assembly 
				 // and preconditioning.
template <int dim>
BoussinesqFlowProblem<dim>::BoussinesqFlowProblem (const unsigned int degree)
                :
		triangulation (Triangulation<dim>::maximum_smoothing),
                degree (degree),
                stokes_fe (FE_Q<dim>(degree+1), dim,
			   FE_Q<dim>(degree), 1),
		stokes_dof_handler (triangulation),
		temperature_fe (degree),
                temperature_dof_handler (triangulation),
                time_step (0),
		old_time_step (0),
		timestep_number (0),
		rebuild_matrices (true),
		rebuild_preconditioner (true)
{}



				 // @sect4{BoussinesqFlowProblem::setup_dofs}
				 // 
				 // This function does the same as
				 // in most other tutorial programs. 
				 // As a slight difference, the 
				 // program is called with a 
				 // parameter <code>setup_matrices</code>
				 // that decides whether to 
				 // recreate the sparsity pattern
				 // and the associated stiffness
				 // matrix.
				 // 
				 // The body starts by assigning dofs on 
				 // basis of the chosen finite element,
				 // and then renumbers the dofs 
				 // first using the Cuthill_McKee
				 // algorithm (to generate a good
				 // quality ILU during the linear
				 // solution process) and then group
				 // components of velocity, pressure
				 // and temperature together. This 
				 // happens in complete analogy to
				 // step-22.
				 // 
				 // We then proceed with the generation
				 // of the hanging node constraints
				 // that arise from adaptive grid
				 // refinement. Next we impose
				 // the no-flux boundary conditions
				 // $\vec{u}\cdot \vec{n}=0$ by adding
				 // a respective constraint to the
				 // hanging node constraints
				 // matrix. The second parameter in 
				 // the function describes the first 
				 // of the velocity components
				 // in the total dof vector, which is 
				 // zero here. The parameter 
				 // <code>no_normal_flux_boundaries</code>
				 // sets the no flux b.c. to those
				 // boundaries with boundary indicator
				 // zero.
template <int dim>
void BoussinesqFlowProblem<dim>::setup_dofs ()
{
  std::vector<unsigned int> stokes_block_component (dim+1,0);
  stokes_block_component[dim] = 1;
  
  {
    stokes_dof_handler.distribute_dofs (stokes_fe);
    DoFRenumbering::Cuthill_McKee (stokes_dof_handler);
    DoFRenumbering::component_wise (stokes_dof_handler, stokes_block_component);
    
    stokes_constraints.clear ();
    DoFTools::make_hanging_node_constraints (stokes_dof_handler,
					     stokes_constraints);
    std::set<unsigned char> no_normal_flux_boundaries;
    no_normal_flux_boundaries.insert (0);
    VectorTools::compute_no_normal_flux_constraints (stokes_dof_handler, 0,
						     no_normal_flux_boundaries,
						     stokes_constraints);
    stokes_constraints.close ();
  }
  {
    temperature_dof_handler.distribute_dofs (temperature_fe);
    DoFRenumbering::Cuthill_McKee (temperature_dof_handler);

    temperature_constraints.clear ();
    DoFTools::make_hanging_node_constraints (temperature_dof_handler,
					     temperature_constraints);
    temperature_constraints.close ();
  }
  
  std::vector<unsigned int> stokes_dofs_per_block (2);
  DoFTools::count_dofs_per_block (stokes_dof_handler, stokes_dofs_per_block,
				  stokes_block_component);
  
  const unsigned int n_u = stokes_dofs_per_block[0],
                     n_p = stokes_dofs_per_block[1],
		     n_T = temperature_dof_handler.n_dofs();

  std::cout << "Number of active cells: "
            << triangulation.n_active_cells()
	    << " (on "
	    << triangulation.n_levels()
	    << " levels)"
            << std::endl
            << "Number of degrees of freedom: "
            << n_u + n_p + n_T
            << " (" << n_u << '+' << n_p << '+'<< n_T <<')'
            << std::endl
            << std::endl;


  
				 // The next step is to 
				 // create the sparsity 
				 // pattern for the system matrix 
				 // based on the Boussinesq 
				 // system. As in step-22, 
				 // we choose to create the
				 // pattern not as in the
				 // first tutorial programs,
				 // but by using the blocked
				 // version of 
				 // CompressedSetSparsityPattern.
				 // The reason for doing this 
				 // is mainly a memory issue,
				 // that is, the basic procedures
				 // consume too much memory
				 // when used in three spatial
				 // dimensions as we intend
				 // to do for this program.
				 // 
				 // So, in case we need
				 // to recreate the matrices,
				 // we first release the
				 // stiffness matrix from the
				 // sparsity pattern and then
				 // set up an object of the 
				 // BlockCompressedSetSparsityPattern
				 // consisting of three blocks. 
				 // Each of these blocks is
				 // initialized with the
				 // respective number of 
				 // degrees of freedom. 
				 // Once the blocks are 
				 // created, the overall size
				 // of the sparsity pattern
				 // is initiated by invoking 
				 // the <code>collect_sizes()</code>
				 // command, and then the
				 // sparsity pattern can be
				 // filled with information.
				 // Then, the hanging
				 // node constraints are applied
				 // to the temporary sparsity
				 // pattern, which is finally
				 // then completed and copied
				 // into the general sparsity
				 // pattern structure.
  
				 // Observe that we use a 
				 // coupling argument for 
				 // telling the function
				 // <code>make_stokes_sparsity_pattern</code>
				 // which components actually
				 // will hold data and which 
				 // we're going to neglect.
				 // 
				 // After these actions, we 
				 // need to reassign the 
				 // system matrix structure to
				 // the sparsity pattern.
  {
    stokes_matrix.clear ();

    BlockCompressedSetSparsityPattern csp (2,2);
 
    csp.block(0,0).reinit (n_u, n_u);
    csp.block(0,1).reinit (n_u, n_p);
    csp.block(1,0).reinit (n_p, n_u);
    csp.block(1,1).reinit (n_p, n_p);
      
    csp.collect_sizes ();

    Table<2,DoFTools::Coupling> coupling (dim+1, dim+1);

				     // build the sparsity pattern. note
				     // that all dim velocities couple with
				     // each other and with the pressures,
				     // but that not all of the other
				     // components couple:
    switch (dim)
      {
	case 2:
	{
	  static const bool coupling_matrix[3][3]
	    = {{ 1, 1,   1},
	       { 1, 1,   1},
		 
	       { 1, 1,   0}};
	  for (unsigned int c=0; c<dim+1; ++c)
	    for (unsigned int d=0; d<dim+1; ++d)
	      if (coupling_matrix[c][d] == true)
		coupling[c][d] = DoFTools::always;
	      else
		coupling[c][d] = DoFTools::none;

	  break;
	}

	case 3:
	{
	  static const bool coupling_matrix[4][4]
	    = {{ 1, 1, 1,   1},
	       { 1, 1, 1,   1},
	       { 1, 1, 1,   1},
		 
	       { 1, 1, 1,   0}};
	  for (unsigned int c=0; c<dim+1; ++c)
	    for (unsigned int d=0; d<dim+1; ++d)
	      if (coupling_matrix[c][d] == true)
		coupling[c][d] = DoFTools::always;
	      else
		coupling[c][d] = DoFTools::none;

	  break;
	}

	default:
	      Assert (false, ExcNotImplemented());
      } 
      
    DoFTools::make_sparsity_pattern (stokes_dof_handler, coupling, csp);
    stokes_constraints.condense (csp);
    stokes_sparsity_pattern.copy_from (csp);

    stokes_matrix.reinit (stokes_sparsity_pattern);
  }
  
  {
    Amg_preconditioner.reset ();
    Mp_preconditioner.reset ();
    stokes_preconditioner_matrix.clear ();

    BlockCompressedSetSparsityPattern csp (2,2);
 
    csp.block(0,0).reinit (n_u, n_u);
    csp.block(0,1).reinit (n_u, n_p);
    csp.block(1,0).reinit (n_p, n_u);
    csp.block(1,1).reinit (n_p, n_p);
      
    csp.collect_sizes ();

    Table<2,DoFTools::Coupling> coupling (dim+1, dim+1);

				     // build the sparsity pattern. note
				     // that all dim velocities couple with
				     // each other and with the pressures,
				     // but that not all of the other
				     // components couple:
    switch (dim)
      {
	case 2:
	{
	  static const bool coupling_matrix[3][3]
	    = {{ 1, 0,   0},
	       { 0, 1,   0},
		 
	       { 0, 0,   1}};
	  for (unsigned int c=0; c<dim+1; ++c)
	    for (unsigned int d=0; d<dim+1; ++d)
	      if (coupling_matrix[c][d] == true)
		coupling[c][d] = DoFTools::always;
	      else
		coupling[c][d] = DoFTools::none;

	  break;
	}

	case 3:
	{
	  static const bool coupling_matrix[4][4]
	    = {{ 1, 0, 0,   0},
	       { 0, 1, 0,   0},
	       { 0, 0, 1,   0},
		 
	       { 0, 0, 0,   1}};
	  for (unsigned int c=0; c<dim+1; ++c)
	    for (unsigned int d=0; d<dim+1; ++d)
	      if (coupling_matrix[c][d] == true)
		coupling[c][d] = DoFTools::always;
	      else
		coupling[c][d] = DoFTools::none;

	  break;
	}

	default:
	      Assert (false, ExcNotImplemented());
      }


    DoFTools::make_sparsity_pattern (stokes_dof_handler, coupling, csp);
    stokes_constraints.condense (csp);
    stokes_preconditioner_sparsity_pattern.copy_from (csp);

    stokes_preconditioner_matrix.reinit (stokes_preconditioner_sparsity_pattern);
  }

  {
    temperature_matrix.clear ();

    CompressedSetSparsityPattern csp (n_T, n_T);      
    DoFTools::make_sparsity_pattern (temperature_dof_handler, csp);
    temperature_constraints.condense (csp);
    temperature_sparsity_pattern.copy_from (csp);

    temperature_matrix.reinit (temperature_sparsity_pattern);
  }
    
      
  
				   // As last action in this function,
				   // we need to set the vectors
				   // for the solution, the old 
				   // solution (required for 
				   // time stepping) and the system
				   // right hand side to the 
				   // three-block structure given
				   // by velocity, pressure and
				   // temperature.
  stokes_solution.reinit (2);
  stokes_solution.block(0).reinit (n_u);
  stokes_solution.block(1).reinit (n_p);
  stokes_solution.collect_sizes ();

  stokes_rhs.reinit (2);
  stokes_rhs.block(0).reinit (n_u);
  stokes_rhs.block(1).reinit (n_p);
  stokes_rhs.collect_sizes ();
  
  temperature_solution.reinit (n_T);
  old_temperature_solution.reinit (n_T);
  old_old_temperature_solution.reinit (n_T);

  temperature_rhs.reinit (n_T);
}



template <int dim>
double scalar_product (const Tensor<2,dim> &t1,
		       const Tensor<2,dim> &t2)
{
  double s = 0;
  for (unsigned int i=0; i<dim; ++i)
    for (unsigned int j=0; j<dim; ++j)
      s += t1[i][j] * t2[i][j];
  return s;
}


template <int dim>
void
BoussinesqFlowProblem<dim>::assemble_stokes_preconditioner ()
{
  stokes_preconditioner_matrix = 0;

  QGauss<dim>   quadrature_formula(degree+2);
  FEValues<dim> stokes_fe_values (stokes_fe, quadrature_formula,
				  update_JxW_values |
				  update_values |
				  update_gradients);
  const unsigned int   dofs_per_cell   = stokes_fe.dofs_per_cell;

  const unsigned int   n_q_points      = quadrature_formula.size();

  FullMatrix<double>   local_matrix (dofs_per_cell, dofs_per_cell);
  std::vector<unsigned int> local_dof_indices (dofs_per_cell);

  std::vector<Tensor<2,dim> > phi_grad_u (dofs_per_cell);
  std::vector<double>         phi_p      (dofs_per_cell);

  const FEValuesExtractors::Vector velocities (0);
  const FEValuesExtractors::Scalar pressure (dim);

  typename DoFHandler<dim>::active_cell_iterator
    cell = stokes_dof_handler.begin_active(),
    endc = stokes_dof_handler.end();
  for (; cell!=endc; ++cell)
    {
      stokes_fe_values.reinit (cell);
      local_matrix = 0;

      for (unsigned int q=0; q<n_q_points; ++q)
	{
	  for (unsigned int k=0; k<dofs_per_cell; ++k)
	    {
	      phi_grad_u[k] = stokes_fe_values[velocities].gradient(k,q);
	      phi_p[k]      = stokes_fe_values[pressure].value (k, q);
	    }
	  
	  for (unsigned int i=0; i<dofs_per_cell; ++i)
	    for (unsigned int j=0; j<dofs_per_cell; ++j)
	      local_matrix(i,j) += (scalar_product (phi_grad_u[i], phi_grad_u[j])
				    +
				    phi_p[i] * phi_p[j])
				   * stokes_fe_values.JxW(q);
	}

      cell->get_dof_indices (local_dof_indices);

      for (unsigned int i=0; i<dofs_per_cell; ++i)
	for (unsigned int j=0; j<dofs_per_cell; ++j)
	  stokes_preconditioner_matrix.add (local_dof_indices[i],
					    local_dof_indices[j],
					    local_matrix(i,j));
    }
  
  stokes_constraints.condense (stokes_preconditioner_matrix);
}



template <int dim>
void
BoussinesqFlowProblem<dim>::build_stokes_preconditioner ()
{
  std::cout << "   Rebuilding Stokes preconditioner..." << std::flush;
      

				   // This last step of the assembly
				   // function sets up the preconditioners
				   // used for the solution of the
				   // system. We are going to use an
				   // ILU preconditioner for the
				   // velocity block (to be used
				   // by BlockSchurPreconditioner class)
				   // as well as an ILU preconditioner
				   // for the inversion of the 
				   // pressure mass matrix. Recall that
				   // the velocity-velocity block sits
				   // at position (0,0) in the 
				   // global system matrix, and
				   // the pressure mass matrix in
				   // (1,1). The 
				   // storage of these objects is
				   // as in step-22, that is, we
				   // include them using a 
				   // shared pointer structure from the
				   // boost library.
				   // 
				   // When all work is done, we 
				   // change the flags 
				   // <code>rebuild_preconditioner</code>
				   // and 
				   // <code>rebuild_matrices</code>
				   // to false.
  assemble_stokes_preconditioner ();
      
  Amg_preconditioner = boost::shared_ptr<PreconditionerTrilinosAmg>
		       (new PreconditionerTrilinosAmg());
      
  const unsigned int n_u = stokes_preconditioner_matrix.block(0,0).m();
  std::vector<double> null_space (dim * n_u, 0.);
      
  std::vector<bool> precondition_dof_list (stokes_dof_handler.n_dofs(), false);
      
  for (unsigned int component=0; component < dim; ++component)
    {
      std::vector<bool> precondition_mask (dim + 1, false);
      precondition_mask[component] = true;
      DoFTools::extract_dofs (stokes_dof_handler, precondition_mask, 
			      precondition_dof_list);

				       // TODO: The current implementation 
				       // assumes that we are working on 
				       // the first components of a system when
				       // writing into the null vector.
				       // Change this to the general case,
				       // probably use something similar as
				       // for block vectors.
      unsigned int counter = 0;
      for (unsigned int i=0; i<stokes_dof_handler.n_dofs(); ++i)
	{
	  if (precondition_dof_list[i])
	    {
	      Assert(i < n_u,
		     ExcMessage("Could not correctly locate "
				"preconditioner dofs in system!"));
	      null_space [component * n_u + i] = 1.;
	      ++counter;
	    }
	}
      Assert (counter == n_u / dim,
	      ExcDimensionMismatch(counter, n_u / dim));
    }
	
  Amg_preconditioner->initialize(stokes_preconditioner_matrix.block(0,0),
				 null_space, dim, true, true, false);

				   // TODO: we could throw away the (0,0)
				   // block here since things have been
				   // copied over to Trilinos. we need to
				   // keep the (1,1) block, though
      
  Mp_preconditioner
    = boost::shared_ptr<SparseILU<double> >
    (new SparseILU<double>);
  Mp_preconditioner->initialize (stokes_preconditioner_matrix.block(1,1),
				 SparseILU<double>::AdditionalData());
      
  std::cout << std::endl;

  rebuild_preconditioner = false;
}



				 // @sect4{BoussinesqFlowProblem::assemble_stokes_system}
				 // 
				 // The assembly of the Boussinesq 
				 // system is acutally a two-step
				 // procedure. One is to create
				 // the Stokes system matrix and
				 // right hand side for the 
				 // velocity-pressure system as
				 // well as the mass matrix for
				 // temperature, and
				 // the second is to create the
				 // rhight hand side for the temperature
				 // dofs. The reason for doing this
				 // in two steps is simply that 
				 // the time stepping we have chosen
				 // needs the result from the Stokes
				 // system at the current time step
				 // for building the right hand
				 // side of the temperature equation.
				 // 
				 // This function does the 
				 // first of these two tasks.
				 // There are two different situations
				 // for calling this function. The
				 // first one is when we reset the
				 // mesh, and both the matrix and
				 // the right hand side have to
				 // be generated. The second situation
				 // only sets up the right hand
				 // side. The reason for having 
				 // two different accesses is that
				 // the matrix of the Stokes system
				 // does not change in time unless
				 // the mesh is changed, so we can
				 // save a considerable amount of
				 // work by doing the full assembly
				 // only when it is needed.
				 // 
				 // Regarding the technical details
				 // of implementation, not much has
				 // changed from step-22. We reset
				 // matrix and vector, create 
				 // a quadrature formula on the 
				 // cells and one on cell faces
				 // (for implementing Neumann 
				 // boundary conditions). Then,
				 // we create a respective
				 // FEValues object for both the 
				 // cell and the face integration.
				 // For the the update flags of
				 // the first, we perform the
				 // calculations of basis function
				 // derivatives only in
				 // case of a full assembly, since
				 // they are not needed otherwise,
				 // which makes the call of
				 // the FEValues::reinit function
				 // further down in the program 
				 // more efficient.
				 // 
				 // The declarations proceed 
				 // with some shortcuts for 
				 // array sizes, the creation of
				 // the local matrix and right 
				 // hand side as well as the
				 // vector for the indices of
				 // the local dofs compared to
				 // the global system.
template <int dim>
void BoussinesqFlowProblem<dim>::assemble_stokes_system ()
{
  std::cout << "   Assembling..." << std::flush;

  if (rebuild_matrices == true)
    stokes_matrix=0;

  stokes_rhs=0;

  QGauss<dim>   quadrature_formula(degree+2);
  QGauss<dim-1> face_quadrature_formula(degree+2);

  FEValues<dim> stokes_fe_values (stokes_fe, quadrature_formula,
				  update_values    |
				  update_quadrature_points  |
				  update_JxW_values |
				  (rebuild_matrices == true
				   ?
				   update_gradients
				   :
				   UpdateFlags(0)));

  FEValues<dim> temperature_fe_values (temperature_fe, quadrature_formula,
				       update_values);

  FEFaceValues<dim> stokes_fe_face_values (stokes_fe, face_quadrature_formula,
					   update_values    | 
					   update_normal_vectors |
					   update_quadrature_points  | 
					   update_JxW_values);

  const unsigned int   dofs_per_cell   = stokes_fe.dofs_per_cell;

  const unsigned int   n_q_points      = quadrature_formula.size();
  const unsigned int   n_face_q_points = face_quadrature_formula.size();

  FullMatrix<double>   local_matrix (dofs_per_cell, dofs_per_cell);
  Vector<double>       local_rhs (dofs_per_cell);

  std::vector<unsigned int> local_dof_indices (dofs_per_cell);

				   // These few declarations provide
				   // the structures for the evaluation
				   // of inhomogeneous Neumann boundary
				   // conditions from the function
				   // declaration made above.
				   // The vector <code>old_solution_values</code>
				   // evaluates the solution 
				   // at the old time level, since
				   // the temperature from the
				   // old time level enters the 
				   // Stokes system as a source
				   // term in the momentum equation.
				   // 
				   // Then, we create a variable
				   // to hold the Rayleigh number,
				   // the measure of buoyancy.
				   // 
				   // The set of vectors we create
				   // next hold the evaluations of
				   // the basis functions that will
				   // be used for creating the
				   // matrices. This gives faster
				   // access to that data, which
				   // increases the performance
				   // of the assembly. See step-22 
				   // for details.
				   // 
				   // The last few declarations 
				   // are used to extract the 
				   // individual blocks (velocity,
				   // pressure, temperature) from
				   // the total FE system.
  const PressureBoundaryValues<dim> pressure_boundary_values;
  std::vector<double>               boundary_values (n_face_q_points);

  std::vector<double>               old_temperature_values(n_q_points);

  const double Rayleigh_number = 10;

  std::vector<Tensor<1,dim> >          phi_u       (dofs_per_cell);
  std::vector<SymmetricTensor<2,dim> > grads_phi_u (dofs_per_cell);
  std::vector<double>                  div_phi_u   (dofs_per_cell);
  std::vector<double>                  phi_p       (dofs_per_cell);

  const FEValuesExtractors::Vector velocities (0);
  const FEValuesExtractors::Scalar pressure (dim);

				   // Now start the loop over
				   // all cells in the problem.
				   // The first commands are all
				   // very familiar, doing the
				   // evaluations of the element
				   // basis functions, resetting
				   // the local arrays and 
				   // getting the values of the
				   // old solution at the
				   // quadrature point. Then we
				   // are ready to loop over
				   // the quadrature points 
				   // on the cell.
  typename DoFHandler<dim>::active_cell_iterator
    cell = stokes_dof_handler.begin_active(),
    endc = stokes_dof_handler.end();
  typename DoFHandler<dim>::active_cell_iterator
    temperature_cell = temperature_dof_handler.begin_active();
  
  for (; cell!=endc; ++cell, ++temperature_cell)
    {
      stokes_fe_values.reinit (cell);
      temperature_fe_values.reinit (temperature_cell);
      
      local_matrix = 0;
      local_rhs = 0;

      temperature_fe_values.get_function_values (old_temperature_solution, old_temperature_values);

      for (unsigned int q=0; q<n_q_points; ++q)
	{
	  const double old_temperature = old_temperature_values[q];

					   // Extract the basis relevant
					   // terms in the inner products
					   // once in advance as shown
					   // in step-22 in order to 
					   // accelerate assembly.
					   // 
					   // Once this is done, we 
					   // start the loop over the
					   // rows and columns of the
					   // local matrix and feed
					   // the matrix with the relevant
					   // products. The right hand
					   // side is filled with the 
					   // forcing term driven by
					   // temperature in direction
					   // of gravity (which is 
					   // vertical in our example).
					   // Note that the right hand 
					   // side term is always generated,
					   // whereas the matrix 
					   // contributions are only
					   // updated when it is 
					   // requested by the
					   // <code>rebuild_matrices</code>
					   // flag.
	  for (unsigned int k=0; k<dofs_per_cell; ++k)
	    {
	      phi_u[k] = stokes_fe_values[velocities].value (k,q);
	      if (rebuild_matrices)
	        {
		  grads_phi_u[k] = stokes_fe_values[velocities].symmetric_gradient(k,q);
		  div_phi_u[k]   = stokes_fe_values[velocities].divergence (k, q);
		  phi_p[k]       = stokes_fe_values[pressure].value (k, q);
		}
	    }

					   // define viscosity
	  const double eta = 1;
	  if (rebuild_matrices)
	    for (unsigned int i=0; i<dofs_per_cell; ++i)
	      for (unsigned int j=0; j<dofs_per_cell; ++j)
		local_matrix(i,j) += (eta * grads_phi_u[i] * grads_phi_u[j]
				      - div_phi_u[i] * phi_p[j]
				      - phi_p[i] * div_phi_u[j])
				     * stokes_fe_values.JxW(q);

	  const Point<dim> gravity = ( (dim == 2) ? (Point<dim> (0,1)) : 
				       (Point<dim> (0,0,1)) );
	  for (unsigned int i=0; i<dofs_per_cell; ++i)
	    local_rhs(i) += (Rayleigh_number *
			     gravity * phi_u[i] * old_temperature)*
			    stokes_fe_values.JxW(q);
	}


				       // Next follows the assembly 
				       // of the face terms, result
				       // from Neumann boundary 
				       // conditions. Since these
				       // terms only enter the right
				       // hand side vector and not
				       // the matrix, there is no
				       // substantial benefit from
				       // extracting the data 
				       // before using it, so 
				       // we remain in the lines 
				       // of step-20 at this point.
      for (unsigned int face_no=0;
	   face_no<GeometryInfo<dim>::faces_per_cell;
	   ++face_no)
	if (cell->at_boundary(face_no))
	  {
	    stokes_fe_face_values.reinit (cell, face_no);

	    pressure_boundary_values
	      .value_list (stokes_fe_face_values.get_quadrature_points(),
			   boundary_values);

	    for (unsigned int q=0; q<n_face_q_points; ++q)
	      for (unsigned int i=0; i<dofs_per_cell; ++i)
		{
		  const Tensor<1,dim>
		    phi_i_u = stokes_fe_face_values[velocities].value (i, q);

		  local_rhs(i) += -(phi_i_u *
				    stokes_fe_face_values.normal_vector(q) *
				    boundary_values[q] *
				    stokes_fe_face_values.JxW(q));
		}
	  }      

				       // The last step in the loop 
				       // over all cells is to
				       // enter the local contributions
				       // into the global matrix and 
				       // vector structures to the
				       // positions specified in 
				       // <code>local_dof_indices</code>.
				       // Again, we only add the 
				       // matrix data when it is 
				       // requested.
      cell->get_dof_indices (local_dof_indices);

      if (rebuild_matrices == true)
	for (unsigned int i=0; i<dofs_per_cell; ++i)
	  for (unsigned int j=0; j<dofs_per_cell; ++j)
	    stokes_matrix.add (local_dof_indices[i],
			       local_dof_indices[j],
			       local_matrix(i,j));

      for (unsigned int i=0; i<dofs_per_cell; ++i)
        stokes_rhs(local_dof_indices[i]) += local_rhs(i);
    }

				   // Back at the outermost
				   // level of this function,
				   // we continue the work
				   // by condensing hanging
				   // node constraints to the
				   // right hand side and, 
				   // possibly, to the matrix.
  stokes_constraints.condense (stokes_rhs);

  if (rebuild_matrices == true)
    {
      stokes_constraints.condense (stokes_matrix);

//       std::map<unsigned int,double> boundary_values;

//       typename DoFHandler<dim>::active_cell_iterator
// 	cell = dof_handler.begin_active(),
// 	emdc = dof_handler.end();
//       for (; cell!=endc; ++cell)
// 	for (unsigned int v=0; v<GeometryInfo<dim>::vertices_per_cell; ++v)
// 	  if (cell->vertex(v).distance(dim == 2
// 				       ?
// 				       Point<dim>(0,-1)
// 				       :
// 				       Point<dim>(0,0,-1)) < 1e-6)
// 	    {
// 	      std::cout << "Found cell and vertex: " << cell << ' '
// 			<< v << std::endl;

// 	      boundary_values[cell->vertex_dof_index(v,0)] = 0;
// 	      break;
// 	    }

//      std::vector<bool> component_mask (dim+2, true);
//       component_mask[dim] = component_mask[dim+1] = false;
//       VectorTools::interpolate_boundary_values (dof_handler,
// 						0,
// 						ZeroFunction<dim>(dim+2),
// 						boundary_values,
// 						component_mask);

//       MatrixTools::apply_boundary_values (boundary_values,
// 					  stokes_matrix,
// 					  solution,
// 					  system_rhs);
    }

  rebuild_matrices = false;

  std::cout << std::endl;
}




template <int dim>
double compute_viscosity(
  const std::vector<double>          &old_temperature,
  const std::vector<double>          &old_old_temperature,
  const std::vector<Tensor<1,dim> >  &old_temperature_grads,
  const std::vector<Tensor<1,dim> >  &old_old_temperature_grads,
  const std::vector<Tensor<2,dim> >  &old_temperature_hessians,
  const std::vector<Tensor<2,dim> >  &old_old_temperature_hessians,
  const std::vector<Vector<double> > &present_stokes_values,
  const std::vector<double>          &gamma_values,
  const double                        kappa,
  const double                        global_u_infty,
  const double                        global_T_infty,
  const double                        global_Omega_diameter,
  const double                        cell_diameter,
  const double                        old_time_step
)
{
  const double beta = 0.1;
  const double alpha = 1;
  
  if (global_u_infty == 0)
    return 5e-3 * cell_diameter;
  
  const unsigned int n_q_points = old_temperature.size();
  
				   // Stage 1: calculate residual
  double max_residual = 0;
  double max_velocity = 0;
  
  for (unsigned int q=0; q < n_q_points; ++q)
    {
      Tensor<1,dim> u;
      for (unsigned int d=0; d<dim; ++d)
	u[d] = present_stokes_values[q](d);
      
      const double dT_dt = (old_temperature[q] - old_old_temperature[q])
			   / old_time_step;
      const double u_grad_T = u * (old_temperature_grads[q] +
				   old_old_temperature_grads[q]) / 2;
      
      const double kappa_Delta_T = kappa
				   * (trace(old_temperature_hessians[q]) +
				      trace(old_old_temperature_hessians[q])) / 2;

      const double residual
	= std::abs((dT_dt + u_grad_T - kappa_Delta_T - gamma_values[q]) *
		   std::pow((old_temperature[q]+old_old_temperature[q]) / 2,
			    alpha-1.));

      max_residual = std::max (residual,        max_residual);
      max_velocity = std::max (std::sqrt (u*u), max_velocity);
    }
  
  const double global_scaling = global_u_infty * global_T_infty /
				std::pow(global_Omega_diameter, alpha - 2.);

  return (beta *
	  max_velocity *
	  std::min (cell_diameter,
		    std::pow(cell_diameter,alpha) * max_residual / global_scaling));
}



				 // @sect4{BoussinesqFlowProblem::assemble_temperature_system}
				 // 
				 // This function does the second
				 // part of the assembly work, the
				 // creation of the velocity-dependent
				 // right hand side of the
				 // temperature equation. The 
				 // declarations in this function
				 // are pretty much the same as the
				 // ones used in the other 
				 // assembly routine, except that we
				 // restrict ourselves to vectors
				 // this time. Though, we need to
				 // perform more face integrals 
				 // at this point, induced by the
				 // use of discontinuous elements for 
				 // the temperature (just
				 // as it was in the first DG 
				 // example in step-12) in combination
				 // with adaptive grid refinement
				 // and subfaces. The update 
				 // flags at face level are the 
				 // same as in step-12.
template <int dim>
void BoussinesqFlowProblem<dim>::assemble_temperature_system ()
{
  const bool use_bdf2_scheme = (timestep_number != 0);

  temperature_matrix = 0;
  temperature_rhs = 0;
  
  QGauss<dim>   quadrature_formula(degree+2);
  FEValues<dim> temperature_fe_values (temperature_fe, quadrature_formula,
				       update_values    | update_gradients |
				       update_hessians |
				       update_quadrature_points  | update_JxW_values);
  FEValues<dim> stokes_fe_values (stokes_fe, quadrature_formula,
				  update_values);

  const unsigned int   dofs_per_cell   = temperature_fe.dofs_per_cell;
  const unsigned int   n_q_points      = quadrature_formula.size();

  Vector<double>       local_rhs (dofs_per_cell);
  FullMatrix<double>   local_matrix (dofs_per_cell, dofs_per_cell);

  std::vector<unsigned int> local_dof_indices (dofs_per_cell);

				   // Here comes the declaration
				   // of vectors to hold the old
				   // and present solution values
				   // and gradients
				   // for both the cell as well as faces
				   // to the cell. Next comes the
				   // declaration of an object
				   // to hold the temperature 
				   // boundary values and a
				   // well-known extractor for
				   // accessing the temperature
				   // part of the FE system.
  std::vector<Vector<double> > present_stokes_values (n_q_points, 
						      Vector<double>(dim+1));

  
  std::vector<double>         old_temperature_values (n_q_points);
  std::vector<double>         old_old_temperature_values(n_q_points);
  std::vector<Tensor<1,dim> > old_temperature_grads(n_q_points);
  std::vector<Tensor<1,dim> > old_old_temperature_grads(n_q_points);
  std::vector<Tensor<2,dim> > old_temperature_hessians(n_q_points);
  std::vector<Tensor<2,dim> > old_old_temperature_hessians(n_q_points);

  
  TemperatureBoundaryValues<dim> temperature_boundary_values;
  TemperatureRightHandSide<dim>  temperature_right_hand_side;
  std::vector<double> gamma_values (n_q_points);

  std::vector<double>                  phi_T       (dofs_per_cell);
  std::vector<Tensor<1,dim> >          grad_phi_T  (dofs_per_cell);
  
  const double global_u_infty = get_maximal_velocity();
  const double global_T_infty = get_maximal_temperature();
  const double global_Omega_diameter = GridTools::diameter (triangulation);

				   // Now, let's start the loop
				   // over all cells in the
				   // triangulation. The first
				   // actions within the loop
				   // are, 0as usual, the evaluation
				   // of the FE basis functions 
				   // and the old and present
				   // solution at the quadrature 
				   // points.
  typename DoFHandler<dim>::active_cell_iterator
    cell = temperature_dof_handler.begin_active(),
    endc = temperature_dof_handler.end();
  typename DoFHandler<dim>::active_cell_iterator
    stokes_cell = stokes_dof_handler.begin_active();

  for (; cell!=endc; ++cell, ++stokes_cell)
    {
      local_rhs = 0;
      local_matrix = 0;

      temperature_fe_values.reinit (cell);
      stokes_fe_values.reinit (stokes_cell);

      temperature_fe_values.get_function_values (old_temperature_solution, old_temperature_values);
      temperature_fe_values.get_function_values (old_old_temperature_solution, old_old_temperature_values);

      temperature_fe_values.get_function_gradients (old_temperature_solution, old_temperature_grads);
      temperature_fe_values.get_function_gradients (old_old_temperature_solution, old_old_temperature_grads);
      
      temperature_fe_values.get_function_hessians (old_temperature_solution, old_temperature_hessians);
      temperature_fe_values.get_function_hessians (old_old_temperature_solution, old_old_temperature_hessians);
      
      temperature_right_hand_side.value_list (temperature_fe_values.get_quadrature_points(),
					      gamma_values);

      stokes_fe_values.get_function_values (stokes_solution, present_stokes_values);
      
				       // build matrix contributions

				       // define diffusion. take the
				       // maximum of what we really
				       // want and the minimal amount
				       // of diffusion (determined
				       // impirically) to keep the
				       // scheme stable
      const double kappa = 1e-6;
      const double nu
	= compute_viscosity (old_temperature_values,
			     old_old_temperature_values,
			     old_temperature_grads,
			     old_old_temperature_grads,
			     old_temperature_hessians,
			     old_old_temperature_hessians,
			     present_stokes_values,
			     gamma_values,
			     kappa, global_u_infty, global_T_infty,
			     global_Omega_diameter, cell->diameter(),
			     old_time_step);
      
      for (unsigned int q=0; q<n_q_points; ++q)
	{
	  for (unsigned int k=0; k<dofs_per_cell; ++k)
	    {
	      grad_phi_T[k] = temperature_fe_values.shape_grad (k,q);
	      phi_T[k]      = temperature_fe_values.shape_value (k, q);
	    }

	  const double        old_T      = old_temperature_values[q];
	  const double        old_old_T  = old_old_temperature_values[q];

	  const Tensor<1,dim> old_grad_T     = old_temperature_grads[q];
	  const Tensor<1,dim> old_old_grad_T = old_old_temperature_grads[q];

	  
	  Tensor<1,dim> present_u;
	  for (unsigned int d=0; d<dim; ++d)
	    present_u[d] = present_stokes_values[q](d);

	  if (use_bdf2_scheme == true)
	    {
	      for (unsigned int i=0; i<dofs_per_cell; ++i)
		for (unsigned int j=0; j<dofs_per_cell; ++j)
		  local_matrix(i,j) += ((2*time_step + old_time_step) /
					(time_step + old_time_step) *
					phi_T[i] * phi_T[j]
					+
					time_step *
					kappa * grad_phi_T[i] * grad_phi_T[j])
				       * temperature_fe_values.JxW(q);

	      for (unsigned int i=0; i<dofs_per_cell; ++i)
		local_rhs(i) += ((time_step + old_time_step) / old_time_step *
				 old_T * phi_T[i]
				 -
				 (time_step * time_step) /
				 (old_time_step * (time_step + old_time_step)) *
				 old_old_T * phi_T[i]
				 -
				 time_step *
				 present_u *
				 ((1+time_step/old_time_step) * old_grad_T
				  -
				  time_step / old_time_step * old_old_grad_T) *
				 phi_T[i]
				 -
				 time_step *
				 nu *
				 ((1+time_step/old_time_step) * old_grad_T
				  -
				  time_step / old_time_step * old_old_grad_T) *
				 grad_phi_T[i]
				 +
				 time_step *
				 gamma_values[q] * phi_T[i])
				*
				temperature_fe_values.JxW(q);
	    }
	  else
	    {
	      for (unsigned int i=0; i<dofs_per_cell; ++i)
		for (unsigned int j=0; j<dofs_per_cell; ++j)
		  local_matrix(i,j) += (phi_T[i] * phi_T[j]
					+
					time_step *
					kappa * grad_phi_T[i] * grad_phi_T[j])
				       * temperature_fe_values.JxW(q);
      
	      for (unsigned int i=0; i<dofs_per_cell; ++i)
		local_rhs(i) += (old_T * phi_T[i]
				 -
				 time_step *
				 present_u * old_grad_T * phi_T[i]
				 -
				 time_step *
				 nu *
				 old_grad_T * grad_phi_T[i]
				 +
				 time_step *
				 gamma_values[q] * phi_T[i])
				*
				temperature_fe_values.JxW(q);
	    }
	}
      
      cell->get_dof_indices (local_dof_indices);

      for (unsigned int i=0; i<dofs_per_cell; ++i)
	for (unsigned int j=0; j<dofs_per_cell; ++j)
	  temperature_matrix.add (local_dof_indices[i],
				  local_dof_indices[j],
				  local_matrix(i,j));

      for (unsigned int i=0; i<dofs_per_cell; ++i)
        temperature_rhs(local_dof_indices[i]) += local_rhs(i);
    }
}




				 // @sect4{BoussinesqFlowProblem::solve}
template <int dim>
void BoussinesqFlowProblem<dim>::solve ()
{
  std::cout << "   Solving..." << std::endl;
  
				   // Use the BlockMatrixArray structure
				   // for extracting only the upper left
				   // 2x2 blocks from the matrix that will
				   // be used for the solution of the
				   // blocked system.
  {
				     // Set up inverse matrix for
				     // pressure mass matrix
    InverseMatrix<SparseMatrix<double>,SparseILU<double> >
      mp_inverse (stokes_preconditioner_matrix.block(1,1), *Mp_preconditioner);

				     // Set up block Schur preconditioner
				     /*BlockSchurPreconditioner<typename InnerPreconditioner<dim>::type,
				       SparseILU<double> >
				       preconditioner (stokes_matrix, mp_inverse, *A_preconditioner);*/
    BlockSchurPreconditioner<PreconditionerTrilinosAmg, SparseILU<double> >
      preconditioner (stokes_matrix, mp_inverse, *Amg_preconditioner);

				     // Set up GMRES solver and
				     // solve.
    SolverControl solver_control (stokes_matrix.m(),
				  1e-6*stokes_rhs.l2_norm());

    SolverGMRES<BlockVector<double> > gmres(solver_control,
					    SolverGMRES<BlockVector<double> >::AdditionalData(100));

    gmres.solve(stokes_matrix, stokes_solution, stokes_rhs, preconditioner);

    std::cout << "   "
              << solver_control.last_step()
              << " GMRES iterations for Stokes subsystem."
              << std::endl;
	      
				     // Produce a constistent solution
				     // field (we can't do this on the 'up'
				     // vector since it does not have the
				     // temperature component, but
				     // hanging_node_constraints has
				     // constraints also for the
				     // temperature vector)
    stokes_constraints.distribute (stokes_solution);
  }

				   // TODO: determine limit of stability for
				   // the time step (whether it needs to be /4
				   // or whether we could get away with a
				   // bigger time step)
  old_time_step = time_step;    
  time_step = GridTools::minimal_cell_diameter(triangulation) /
              std::max (get_maximal_velocity(), .05) / 4;
  
  temperature_solution = old_temperature_solution;


  assemble_temperature_system ();
  {

    SolverControl solver_control (temperature_matrix.m(),
				  1e-8*temperature_rhs.l2_norm());
    SolverCG<>   cg (solver_control);
    PreconditionSSOR<> preconditioner;
    preconditioner.initialize (temperature_matrix, 1.2);

    cg.solve (temperature_matrix, temperature_solution,
	      temperature_rhs, preconditioner);

				     // produce a consistent temperature field
    temperature_constraints.distribute (temperature_solution);

    std::cout << "   "
              << solver_control.last_step()
              << " CG iterations for temperature."
              << std::endl;
    std::cout << "   Max temperature: "
	      << *std::max_element (temperature_solution.begin(),
				    temperature_solution.end())
	      << std::endl;
  }
}



				 // @sect4{BoussinesqFlowProblem::output_results}
template <int dim>
void BoussinesqFlowProblem<dim>::output_results ()  const
{
  if (timestep_number % 10 != 0)
    return;

//TODO!!  
//  Assert (false, ExcNotImplemented());
  
//   std::vector<std::string> solution_names (dim, "velocity");
//   solution_names.push_back ("p");
//   solution_names.push_back ("T");

//   DataOut<dim> data_out;

//   data_out.attach_dof_handler (dof_handler);

//   std::vector<DataComponentInterpretation::DataComponentInterpretation>
//     data_component_interpretation
//     (dim+2, DataComponentInterpretation::component_is_scalar);
//   for (unsigned int i=0; i<dim; ++i)
//     data_component_interpretation[i]
//       = DataComponentInterpretation::component_is_part_of_vector;

//   data_out.add_data_vector (solution, solution_names,
// 			    DataOut<dim>::type_dof_data,
// 			    data_component_interpretation);
//   data_out.build_patches (degree);

//   std::ostringstream filename;
//   filename << "solution-" << Utilities::int_to_string(timestep_number, 4) << ".vtk";

//   std::ofstream output (filename.str().c_str());
//   data_out.write_vtk (output);
}



				 // @sect4{BoussinesqFlowProblem::refine_mesh}
template <int dim>
void BoussinesqFlowProblem<dim>::refine_mesh (const unsigned int max_grid_level)
{
  Vector<float> estimated_error_per_cell (triangulation.n_active_cells());

  KellyErrorEstimator<dim>::estimate (temperature_dof_handler,
				      QGauss<dim-1>(degree+1),
				      typename FunctionMap<dim>::type(),
				      temperature_solution,
				      estimated_error_per_cell);

  GridRefinement::refine_and_coarsen_fixed_fraction (triangulation,
						     estimated_error_per_cell,
						     0.8, 0.1);
  if (triangulation.n_levels() > max_grid_level) 
    for (typename Triangulation<dim>::active_cell_iterator
	   cell = triangulation.begin_active(max_grid_level);
	 cell != triangulation.end(); ++cell)
      if (cell->has_children() == false)
	cell->clear_refine_flag ();
  
  SolutionTransfer<dim, double> soltrans(temperature_dof_handler);

  triangulation.prepare_coarsening_and_refinement();

  std::vector<Vector<double> > x_solution (2);
  x_solution[0].reinit (temperature_dof_handler.n_dofs());
  x_solution[0] = temperature_solution;
  x_solution[1].reinit (temperature_dof_handler.n_dofs());
  x_solution[1] = old_temperature_solution;

  soltrans.prepare_for_coarsening_and_refinement(x_solution);

  triangulation.execute_coarsening_and_refinement ();
  setup_dofs ();

  std::vector<Vector<double> > tmp (2);
  tmp[0].reinit (temperature_dof_handler.n_dofs());
  tmp[1].reinit (temperature_dof_handler.n_dofs());
  soltrans.interpolate(x_solution, tmp);

  temperature_solution = tmp[0];
				   //TODO: this should be [1]!!!!!!!
  old_temperature_solution = tmp[0];

  rebuild_matrices       = true;
  rebuild_preconditioner = true;
}



				 // @sect4{BoussinesqFlowProblem::get_maximal_velocity}
template <int dim>
double BoussinesqFlowProblem<dim>::get_maximal_velocity () const
{
  QGauss<dim>   quadrature_formula(degree+2);
  const unsigned int   n_q_points
    = quadrature_formula.size();

  FEValues<dim> fe_values (stokes_fe, quadrature_formula,
                           update_values);
  std::vector<Vector<double> > stokes_values(n_q_points,
					     Vector<double>(dim+1));
  double max_velocity = 0;

  typename DoFHandler<dim>::active_cell_iterator
    cell = stokes_dof_handler.begin_active(),
    endc = stokes_dof_handler.end();
  for (; cell!=endc; ++cell)
    {
      fe_values.reinit (cell);
      fe_values.get_function_values (stokes_solution, stokes_values);

      for (unsigned int q=0; q<n_q_points; ++q)
        {
          Tensor<1,dim> velocity;
          for (unsigned int i=0; i<dim; ++i)
            velocity[i] = stokes_values[q](i);

          max_velocity = std::max (max_velocity,
                                   velocity.norm());
        }
    }

  return max_velocity;
}




				 // @sect4{BoussinesqFlowProblem::get_maximal_velocity}
template <int dim>
double BoussinesqFlowProblem<dim>::get_maximal_temperature () const
{
  QGauss<dim>   quadrature_formula(degree+2);
  const unsigned int   n_q_points = quadrature_formula.size();

  FEValues<dim> fe_values (temperature_fe, quadrature_formula,
                           update_values);
  std::vector<double> old_temperature_values(n_q_points);
  std::vector<double> old_old_temperature_values(n_q_points);
  
  double max_temperature = 0;

  typename DoFHandler<dim>::active_cell_iterator
    cell = temperature_dof_handler.begin_active(),
    endc = temperature_dof_handler.end();
  for (; cell!=endc; ++cell)
    {
      fe_values.reinit (cell);
      fe_values.get_function_values (old_temperature_solution, old_temperature_values);
      fe_values.get_function_values (old_old_temperature_solution, old_old_temperature_values);

      for (unsigned int q=0; q<n_q_points; ++q)
        {
          double temperature = 
	    (1. + time_step/old_time_step) * old_temperature_values[q]-
	    time_step/old_time_step * old_old_temperature_values[q];

          max_temperature = std::max (max_temperature,
				      temperature);
        }
    }

  return max_temperature;
}



				 // @sect4{BoussinesqFlowProblem::run}
template <int dim>
void BoussinesqFlowProblem<dim>::run ()
{
  const unsigned int initial_refinement = (dim == 2 ? 4 : 3);
  const unsigned int n_pre_refinement_steps = 4;

  
  GridGenerator::hyper_cube (triangulation);
  triangulation.refine_global (initial_refinement);

  setup_dofs();

  unsigned int       pre_refinement_step    = 0;
  
  start_time_iteration:

  VectorTools::project (temperature_dof_handler,
			temperature_constraints,
			QGauss<dim>(degree+2),
			TemperatureInitialValues<dim>(),
			old_temperature_solution);
  
  timestep_number = 0;
  double time = 0;

  do
    {
      std::cout << "Timestep " << timestep_number
		<< ":  t=" << time
		<< ", dt=" << time_step
                << std::endl;

      assemble_stokes_system ();

      if (rebuild_preconditioner == true)
	build_stokes_preconditioner ();

      solve ();

      output_results ();

      std::cout << std::endl;
      
      if ((timestep_number == 0) &&
	  (pre_refinement_step < n_pre_refinement_steps))
	{
	  refine_mesh (initial_refinement + n_pre_refinement_steps);
	  ++pre_refinement_step;
	  goto start_time_iteration;
	}
      else
	if ((timestep_number > 0) && (timestep_number % 5 == 0))
	  refine_mesh (initial_refinement + n_pre_refinement_steps);

      time += time_step;
      ++timestep_number;

      old_old_temperature_solution = old_temperature_solution;
      old_temperature_solution     = temperature_solution;      
    }
  while (time <= 10);
}



				 // @sect3{The <code>main</code> function}
int main ()
{
  try
    {
      deallog.depth_console (0);

      BoussinesqFlowProblem<2> flow_problem(1);
      flow_problem.run ();
    }
  catch (std::exception &exc)
    {
      std::cerr << std::endl << std::endl
                << "----------------------------------------------------"
                << std::endl;
      std::cerr << "Exception on processing: " << std::endl
                << exc.what() << std::endl
                << "Aborting!" << std::endl
                << "----------------------------------------------------"
                << std::endl;

      return 1;
    }
  catch (...)
    {
      std::cerr << std::endl << std::endl
                << "----------------------------------------------------"
                << std::endl;
      std::cerr << "Unknown exception!" << std::endl
                << "Aborting!" << std::endl
                << "----------------------------------------------------"
                << std::endl;
      return 1;
    }

  return 0;
}
