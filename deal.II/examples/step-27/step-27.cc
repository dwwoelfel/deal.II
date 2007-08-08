/* $Id$ */
/* Author: Wolfgang Bangerth, Texas A&M University, 2006, 2007 */

/*    $Id$       */
/*    Version: $Name$                                          */
/*                                                                */
/*    Copyright (C) 2006, 2007 by the deal.II authors */
/*                                                                */
/*    This file is subject to QPL and may not be  distributed     */
/*    without copyright and license information. Please refer     */
/*    to the file deal.II/doc/license.html for the  text  and     */
/*    further information on this license.                        */

                                 // @sect3{Include files}

				 // The first few files have already
				 // been covered in previous examples
				 // and will thus not be further
				 // commented on.
#include <base/quadrature_lib.h>
#include <base/function.h>
#include <base/logstream.h>
#include <base/utilities.h>
#include <lac/vector.h>
#include <lac/full_matrix.h>
#include <lac/sparse_matrix.h>
#include <lac/solver_cg.h>
#include <lac/precondition.h>
#include <grid/tria.h>
#include <grid/grid_generator.h>
#include <grid/tria_accessor.h>
#include <grid/tria_iterator.h>
#include <grid/grid_refinement.h>
#include <dofs/dof_accessor.h>
#include <dofs/dof_tools.h>
#include <dofs/dof_constraints.h>
#include <fe/fe_q.h>
#include <numerics/vectors.h>
#include <numerics/matrices.h>
#include <numerics/data_out.h>
#include <numerics/error_estimator.h>

				 // These are the new files we need. The first
				 // one provides an alternative to the usual
				 // SparsityPattern class and the
				 // CompressedSparsityPattern class already
				 // discussed in step-11 and step-18. The last
				 // two provide $hp$ versions of the
				 // DoFHandler and FEValues classes as
				 // described in the introduction of this
				 // program.
#include <lac/compressed_set_sparsity_pattern.h>
#include <hp/dof_handler.h>
#include <hp/fe_values.h>

				 // The last set of include files are standard
				 // C++ headers. We need support for complex
				 // numbers when we compute the Fourier
				 // transform.
#include <fstream>
#include <iostream>
#include <complex>


				 // Finally, this is as in previous
				 // programs:
using namespace dealii;


				 // @sect3{The main class}

				 // The main class of this program looks very
				 // much like the one already used in the
				 // first few tutorial programs, for example
				 // the one in step-6. The main difference is
				 // that we have merged the refine_grid and
				 // output_results functions into one since we
				 // will also want to output some of the
				 // quantities used in deciding how to refine
				 // the mesh (in particular the estimated
				 // smoothness of the solution). There is also
				 // a function that computes this estimated
				 // smoothness, as discussed in the
				 // introduction.
				 //
				 // As far as member variables are concerned,
				 // we use the same structure as already used
				 // in step-6, but instead of a regular
				 // DoFHandler we use an object of type
				 // hp::DoFHandler, and we need collections
				 // instead of individual finite element,
				 // quadrature, and face quadrature
				 // objects. We will fill these collections in
				 // the constructor of the class. The last
				 // variable, <code>max_degree</code>,
				 // indicates the maximal polynomial degree of
				 // shape functions used.
template <int dim>
class LaplaceProblem 
{
  public:
    LaplaceProblem ();
    ~LaplaceProblem ();

    void run ();
    
  private:
    void setup_system ();
    void assemble_system ();
    void solve ();
    void create_coarse_grid ();
    void estimate_smoothness (Vector<float> &smoothness_indicators) const;
    void postprocess (const unsigned int cycle);

    Triangulation<dim>   triangulation;

    hp::DoFHandler<dim>      dof_handler;
    hp::FECollection<dim>    fe_collection;
    hp::QCollection<dim>     quadrature_collection;
    hp::QCollection<dim-1>   face_quadrature_collection;

    ConstraintMatrix     hanging_node_constraints;

    SparsityPattern      sparsity_pattern;
    SparseMatrix<double> system_matrix;

    Vector<double>       solution;
    Vector<double>       system_rhs;

    const unsigned int max_degree;
};



				 // @sect3{Equation data}
				 //
				 // Next, let us define the right hand side
				 // function for this problem. It is $x+1$ in
				 // 1d, $(x+1)(y+1)$ in 2d, and so on.
template <int dim>
class RightHandSide : public Function<dim>
{
  public:
    RightHandSide () : Function<dim> () {}
    
    virtual double value (const Point<dim>   &p,
			  const unsigned int  component) const;
};


template <int dim>
double
RightHandSide<dim>::value (const Point<dim>   &p,
			   const unsigned int  /*component*/) const
{
  double product = 1;
  for (unsigned int d=0; d<dim; ++d)
    product *= (p[d]+1);
  return product;
}




				 // @sect3{Implementation of the main class}

				 // @sect4{LaplaceProblem::LaplaceProblem}

				 // The constructor of this class is fairly
				 // straightforward. It associates the
				 // hp::DoFHandler object with the
				 // triangulation, and then sets the maximal
				 // polynomial degree to 7 (in 1d and 2d) or 5
				 // (in 3d and higher). We do so because using
				 // higher order polynomial degrees becomes
				 // prohibitively expensive, especially in
				 // higher space dimensions.
				 //
				 // Following this, we fill the collections of
				 // finite element, and cell and face
				 // quadrature objects. We start with
				 // quadratic elements, and each quadrature
				 // formula is chosen so that it is
				 // appropriate for the matching finite
				 // element in the hp::FECollection object.
template <int dim>
LaplaceProblem<dim>::LaplaceProblem ()
		:
		dof_handler (triangulation),
		max_degree (dim <= 2 ? 7 : 5)
{
  for (unsigned int degree=2; degree<=max_degree; ++degree)
    {
      fe_collection.push_back (FE_Q<dim>(degree));
      quadrature_collection.push_back (QGauss<dim>(degree+1));
      face_quadrature_collection.push_back (QGauss<dim-1>(degree+1));
    }
}


				 // @sect4{LaplaceProblem::~LaplaceProblem}

				 // The destructor is unchanged from what we
				 // already did in step-6:
template <int dim>
LaplaceProblem<dim>::~LaplaceProblem () 
{
  dof_handler.clear ();
}


				 // @sect4{LaplaceProblem::setup_system}
				 //
				 // This function is again an almost verbatim
				 // copy of what we already did in step-6,
				 // with the main difference that we don't
				 // directly build the sparsity pattern, but
				 // first create an intermediate object that
				 // we later copy into the right data
				 // structure. In another slight deviation, we
				 // do not first build the sparsity pattern
				 // then condense away constrained degrees of
				 // freedom, but pass the constraint matrix
				 // object directly to the function that
				 // builds the sparsity pattern. Both of these
				 // steps are explained in the introduction of
				 // this program.
				 //
				 // The second change, maybe hidden in plain
				 // sight, is that the dof_handler variable
				 // here is an hp object -- nevertheless all
				 // the function calls we had before still
				 // work in exactly the same way as they
				 // always did.
template <int dim>
void LaplaceProblem<dim>::setup_system ()
{
  dof_handler.distribute_dofs (fe_collection);

  solution.reinit (dof_handler.n_dofs());
  system_rhs.reinit (dof_handler.n_dofs());

  hanging_node_constraints.clear ();
  DoFTools::make_hanging_node_constraints (dof_handler,
					   hanging_node_constraints);
  hanging_node_constraints.close ();

  CompressedSetSparsityPattern csp (dof_handler.n_dofs(),
				    dof_handler.n_dofs());
  DoFTools::make_sparsity_pattern (dof_handler, csp,
				   hanging_node_constraints);
  sparsity_pattern.copy_from (csp);

  system_matrix.reinit (sparsity_pattern);
}



				 // @sect4{LaplaceProblem::assemble_system}

				 // This is the function that assembles the
				 // global matrix and right hand side vector
				 // from the local contributions of each
				 // cell. Its main working is as has been
				 // described in many of the tutorial programs
				 // before. The significant deviations are the
				 // ones necessary for $hp$ finite element
				 // methods. In particular, that we need to
				 // use a collection of FEValues object
				 // (implemented through the hp::FEValues
				 // class), and that we have to eliminate
				 // constrained degrees of freedom already
				 // when copying local contributions into
				 // global objects. Both of these are
				 // explained in detail in the introduction of
				 // this program.
				 //
				 // One other slight complication is the fact
				 // that because we use different polynomial
				 // degrees on different cells, the matrices
				 // and vectors holding local contributions do
				 // not have the same size on all cells. At
				 // the beginning of the loop over all cells,
				 // we therefore each time have to resize them
				 // to the correct size (given by
				 // <code>dofs_per_cell</code>). Because these
				 // classes are implement in such a way that
				 // reducing the size of a matrix or vector
				 // does not release the currently allocated
				 // memory (unless the new size is zero), the
				 // process of resizing at the beginning of
				 // the loop will only require re-allocation
				 // of memory during the first few
				 // iterations. Once we have found in a cell
				 // with the maximal finite element degree, no
				 // more re-allocations will happen because
				 // all subsequent <code>reinit</code> calls
				 // will only set the size to something that
				 // fits the currently allocated memory. This
				 // is important since allocating memory is
				 // expensive, and doing so every time we
				 // visit a new cell would take significant
				 // compute time.
template <int dim>
void LaplaceProblem<dim>::assemble_system () 
{
  hp::FEValues<dim> hp_fe_values (fe_collection,
				  quadrature_collection, 
				  update_values    |  update_gradients |
				  update_q_points  |  update_JxW_values);

  const RightHandSide<dim> rhs_function;
  
  FullMatrix<double>   cell_matrix;
  Vector<double>       cell_rhs;

  std::vector<unsigned int> local_dof_indices;
  
  typename hp::DoFHandler<dim>::active_cell_iterator
    cell = dof_handler.begin_active(),
    endc = dof_handler.end();
  for (; cell!=endc; ++cell)
    {
      const unsigned int   dofs_per_cell = cell->get_fe().dofs_per_cell;

      cell_matrix.reinit (dofs_per_cell, dofs_per_cell);
      cell_matrix = 0;

      cell_rhs.reinit (dofs_per_cell);
      cell_rhs = 0;

      hp_fe_values.reinit (cell);
      
      const FEValues<dim> &fe_values = hp_fe_values.get_present_fe_values ();

      std::vector<double>  rhs_values (fe_values.n_quadrature_points);
      rhs_function.value_list (fe_values.get_quadrature_points(),
			       rhs_values);
      
      for (unsigned int q_point=0;
	   q_point<fe_values.n_quadrature_points;
	   ++q_point)
	for (unsigned int i=0; i<dofs_per_cell; ++i)
	  {
	    for (unsigned int j=0; j<dofs_per_cell; ++j)
	      cell_matrix(i,j) += (fe_values.shape_grad(i,q_point) *
				   fe_values.shape_grad(j,q_point) *
				   fe_values.JxW(q_point));

	    cell_rhs(i) += (fe_values.shape_value(i,q_point) *
			    rhs_values[q_point] *
			    fe_values.JxW(q_point));
	  }

      local_dof_indices.resize (dofs_per_cell);
      cell->get_dof_indices (local_dof_indices);

      hanging_node_constraints
	.distribute_local_to_global (cell_matrix,
				     local_dof_indices,
				     system_matrix);
	  
      hanging_node_constraints
	.distribute_local_to_global (cell_rhs,
				     local_dof_indices,
				     system_rhs);
    }

				   // After the steps already discussed above,
				   // all we still have to do is to treat
				   // Dirichlet boundary values
				   // correctly. This works in exactly the
				   // same way as for non-$hp$ objects:
  std::map<unsigned int,double> boundary_values;
  VectorTools::interpolate_boundary_values (dof_handler,
					    0,
					    ZeroFunction<dim>(),
					    boundary_values);
  MatrixTools::apply_boundary_values (boundary_values,
				      system_matrix,
				      solution,
				      system_rhs);
}



				 // @sect4{LaplaceProblem::solve}

				 // The function solving the linear system is
				 // entirely unchanged from previous
				 // examples. We simply try to reduce the
				 // initial residual (which equals the $l_2$
				 // norm of the right hand side) by a certain
				 // factor:
template <int dim>
void LaplaceProblem<dim>::solve () 
{
  SolverControl           solver_control (system_rhs.size(),
					  1e-8*system_rhs.l2_norm());
  SolverCG<>              cg (solver_control);

  PreconditionSSOR<> preconditioner;
  preconditioner.initialize(system_matrix, 1.2);

  cg.solve (system_matrix, solution, system_rhs,
	    preconditioner);

  hanging_node_constraints.distribute (solution);
}



				 // @sect4{LaplaceProblem::postprocess}

				 // After solving the linear system, we will
				 // want to postprocess the solution. Here,
				 // all we do is to estimate the error,
				 // estimate the local smoothness of the
				 // solution as described in the introduction,
				 // then write graphical output, and finally
				 // refine the mesh in both $h$ and $p$
				 // according to the indicators computed
				 // before. We do all this in the same
				 // function because we want the estimated
				 // error and smoothness indicators not only
				 // for refinement, but also include them in
				 // the graphical output.
template <int dim>
void LaplaceProblem<dim>::postprocess (const unsigned int cycle)
{
				   // Let us start with computing estimated
				   // error and smoothness indicators, which
				   // each are one number for each active cell
				   // of our triangulation. For the error
				   // indicator, we use the
				   // KellyErrorEstimator class as
				   // always. Estimating the smoothness is
				   // done in the respective function of this
				   // class; that function is discussed
				   // further down below:
  Vector<float> estimated_error_per_cell (triangulation.n_active_cells());
  KellyErrorEstimator<dim>::estimate (dof_handler,
				      face_quadrature_collection,
				      typename FunctionMap<dim>::type(),
				      solution,
				      estimated_error_per_cell);

  Vector<float> smoothness_indicators (triangulation.n_active_cells());
  estimate_smoothness (smoothness_indicators);

				   // Next we want to generate graphical
				   // output. In addition to the two estimated
				   // quantities derived above, we would also
				   // like to output the polynomial degree of
				   // the finite elements used on each of the
				   // elements on the mesh.
				   //
				   // The way to do that requires that we loop
				   // over all cells and poll the active
				   // finite element index of them using
				   // <code>cell-@>active_fe_index()</code>. We
				   // then use the result of this operation
				   // and query the finite element collection
				   // for the finite element with that index,
				   // and finally determine the polynomial
				   // degree of that element. The result we
				   // put into a vector with one element per
				   // cell. The DataOut class requires this to
				   // be a vector of <code>float</code> or
				   // <code>double</code>, even though our
				   // values are all integers, so that it what
				   // we use:
  {
    Vector<float> fe_degrees (triangulation.n_active_cells());
    {
      typename hp::DoFHandler<dim>::active_cell_iterator
	cell = dof_handler.begin_active(),
	endc = dof_handler.end();
      for (unsigned int index=0; cell!=endc; ++cell, ++index)
	fe_degrees(index)
	  = fe_collection[cell->active_fe_index()].degree;
    }

				     // With now all data vectors available --
				     // solution, estimated errors and
				     // smoothness indicators, and finite
				     // element degrees --, we create a
				     // DataOut object for graphical output
				     // and attach all data. Note that the
				     // DataOut class has a second template
				     // argument (which defaults to
				     // DoFHandler@<dim@>, which is why we
				     // have never seen it in previous
				     // tutorial programs) that indicates the
				     // type of DoF handler to be used. Here,
				     // we have to use the hp::DoFHandler
				     // class:
    DataOut<dim,hp::DoFHandler<dim> > data_out;

    data_out.attach_dof_handler (dof_handler);
    data_out.add_data_vector (solution, "solution");
    data_out.add_data_vector (estimated_error_per_cell, "error");
    data_out.add_data_vector (smoothness_indicators, "smoothness");
    data_out.add_data_vector (fe_degrees, "fe_degree");
    data_out.build_patches ();

				     // The final step in generating
				     // output is to determine a file
				     // name, open the file, and write
				     // the data into it (here, we use
				     // VTK format):
    const std::string filename = "solution-" +
				 Utilities::int_to_string (cycle, 2) +
				 ".vtk";
    std::ofstream output (filename.c_str());
    data_out.write_vtk (output);
  }

				   // After this, we would like to actually
				   // refine the mesh, in both $h$ and
				   // $p$. The way we are going to do this is
				   // as follows: first, we use the estimated
				   // error to flag those cells for refinement
				   // that have the largest error. This is
				   // what we have always done:
  {
    GridRefinement::refine_and_coarsen_fixed_number (triangulation,
						     estimated_error_per_cell,
						     0.3, 0.03);

				     // Next we would like to figure out which
				     // of the cells that have been flagged
				     // for refinement should actually have
				     // $p$ increased instead of $h$
				     // decreased. The strategy we choose here
				     // is that we look at the smoothness
				     // indicators of those cells that are
				     // flagged for refinement, and increase
				     // $p$ for those with a smoothness larger
				     // than a certain threshold. For this, we
				     // first have to determine the maximal
				     // and minimal values of the smoothness
				     // indicators of all flagged cells, which
				     // we do using a loop over all cells and
				     // comparing current minimal and maximal
				     // values. (We start with the minimal and
				     // maximal values of <i>all</i> cells, a
				     // range within which the minimal and
				     // maximal values on cells flagged for
				     // refinement must surely lie.) Absent
				     // any better strategies, we will then
				     // set the threshold above which will
				     // increase $p$ instead of reducing $h$
				     // as the mean value between minimal and
				     // maximal smoothness indicators on cells
				     // flagged for refinement:
    float max_smoothness = *std::min_element (smoothness_indicators.begin(),
					      smoothness_indicators.end()),
	  min_smoothness = *std::max_element (smoothness_indicators.begin(),
					      smoothness_indicators.end());
    {
      typename hp::DoFHandler<dim>::active_cell_iterator
	cell = dof_handler.begin_active(),
	endc = dof_handler.end();
      for (unsigned int index=0; cell!=endc; ++cell, ++index)
	if (cell->refine_flag_set())
	  {
	    max_smoothness = std::max (max_smoothness,
				       smoothness_indicators(index));
	    min_smoothness = std::min (min_smoothness,
				       smoothness_indicators(index));
	  }
    }
    const float threshold_smoothness = (max_smoothness + min_smoothness) / 2;

				     // With this, we can go back, loop over
				     // all cells again, and for those cells
				     // for which (i) the refinement flag is
				     // set, (ii) the smoothness indicator is
				     // larger than the threshold, and (iii)
				     // we still have a finite element with a
				     // polynomial degree higher than the
				     // current one in the finite element
				     // collection, we then increase the
				     // polynomial degree and in return remove
				     // the flag indicating that the cell
				     // should undergo bisection. For all
				     // other cells, the refinement flags
				     // remain untouched:
    {
      typename hp::DoFHandler<dim>::active_cell_iterator
	cell = dof_handler.begin_active(),
	endc = dof_handler.end();
      for (unsigned int index=0; cell!=endc; ++cell, ++index)
	if (cell->refine_flag_set()
	    &&
	    (smoothness_indicators(index) > threshold_smoothness)
	    &&
	    (cell->active_fe_index()+1 < fe_collection.size()))
	  {
	    cell->clear_refine_flag();
	    cell->set_active_fe_index (cell->active_fe_index() + 1);
	  }
    } 

				     // At the end of this procedure, we then
				     // refine the mesh. During this process,
				     // children of cells undergoing bisection
				     // inherit their mother cell's finite
				     // element index:
    triangulation.execute_coarsening_and_refinement ();
  }
}


				 // @sect4{LaplaceProblem::create_coarse_grid}

				 // The following function is used when
				 // creating the initial grid. It is a
				 // specialization for the 2d case, i.e. a
				 // corresponding function needs to be
				 // implemented if the program is run in
				 // anything other then 2d. The function is
				 // actually stolen from step-14 and generates
				 // the same mesh used already there, i.e. the
				 // square domain with the square hole in the
				 // middle. The meaning of the different parts
				 // of this function are explained in the
				 // documentation of step-14:
template <>
void LaplaceProblem<2>::create_coarse_grid ()
{
  const unsigned int dim = 2;
  
  static const Point<2> vertices_1[]
    = {  Point<2> (-1.,   -1.),
         Point<2> (-1./2, -1.),
         Point<2> (0.,    -1.),
         Point<2> (+1./2, -1.),
         Point<2> (+1,    -1.),
	     
         Point<2> (-1.,   -1./2.),
         Point<2> (-1./2, -1./2.),
         Point<2> (0.,    -1./2.),
         Point<2> (+1./2, -1./2.),
         Point<2> (+1,    -1./2.),
	     
         Point<2> (-1.,   0.),
         Point<2> (-1./2, 0.),
         Point<2> (+1./2, 0.),
         Point<2> (+1,    0.),
	     
         Point<2> (-1.,   1./2.),
         Point<2> (-1./2, 1./2.),
         Point<2> (0.,    1./2.),
         Point<2> (+1./2, 1./2.),
         Point<2> (+1,    1./2.),
	     
         Point<2> (-1.,   1.),
         Point<2> (-1./2, 1.),
         Point<2> (0.,    1.),			  
         Point<2> (+1./2, 1.),
         Point<2> (+1,    1.)    };
  const unsigned int
    n_vertices = sizeof(vertices_1) / sizeof(vertices_1[0]);
  const std::vector<Point<dim> > vertices (&vertices_1[0],
                                           &vertices_1[n_vertices]);
  static const int cell_vertices[][GeometryInfo<dim>::vertices_per_cell]
    = {{0, 1, 5, 6},
       {1, 2, 6, 7},
       {2, 3, 7, 8},
       {3, 4, 8, 9},
       {5, 6, 10, 11},
       {8, 9, 12, 13},
       {10, 11, 14, 15},
       {12, 13, 17, 18},
       {14, 15, 19, 20},
       {15, 16, 20, 21},
       {16, 17, 21, 22},
       {17, 18, 22, 23}};
  const unsigned int
    n_cells = sizeof(cell_vertices) / sizeof(cell_vertices[0]);

  std::vector<CellData<dim> > cells (n_cells, CellData<dim>());
  for (unsigned int i=0; i<n_cells; ++i) 
    {
      for (unsigned int j=0;
           j<GeometryInfo<dim>::vertices_per_cell;
           ++j)
        cells[i].vertices[j] = cell_vertices[i][j];
      cells[i].material_id = 0;
    }

  triangulation.create_triangulation (vertices,
                                    cells,
                                    SubCellData());
  triangulation.refine_global (3);
}




				 // @sect4{LaplaceProblem::run}

				 // This function implements the logic of the
				 // program, as did the respective function in
				 // most of the previous programs already, see
				 // for example step-6.
				 //
				 // Basically, it contains the adaptive loop:
				 // in the first iteration create a coarse
				 // grid, and then set up the linear system,
				 // assemble it, solve, and postprocess the
				 // solution including mesh refinement. Then
				 // start over again. In the meantime, also
				 // output some information for those staring
				 // at the screen trying to figure out what
				 // the program does:
template <int dim>
void LaplaceProblem<dim>::run () 
{
  for (unsigned int cycle=0; cycle<6; ++cycle)
    {
      std::cout << "Cycle " << cycle << ':' << std::endl;

      if (cycle == 0)
	create_coarse_grid ();

      setup_system ();

      std::cout << "   Number of active cells:       "
		<< triangulation.n_active_cells()
		<< std::endl
		<< "   Number of degrees of freedom: "
		<< dof_handler.n_dofs()
		<< std::endl
		<< "   Number of constraints       : "
		<< hanging_node_constraints.n_constraints()
		<< std::endl;

      assemble_system ();
      solve ();
      postprocess (cycle);
    }
}



template <int dim>
void
LaplaceProblem<dim>::
estimate_smoothness (Vector<float> &smoothness_indicators) const
{
  const unsigned int N = max_degree;

				   // form all the Fourier vectors
				   // that we want to
				   // consider. exclude k=0 to avoid
				   // problems with |k|^{-mu} and also
				   // logarithms of |k|
  std::vector<Tensor<1,dim> > k_vectors;
  std::vector<unsigned int>   k_vectors_magnitude;
  switch (dim)
    {
      case 2:
      {
	for (unsigned int i=0; i<N; ++i)
	  for (unsigned int j=0; j<N; ++j)
	    if (!((i==0) && (j==0))
		&&
		(i*i + j*j < N*N))
	      {
		k_vectors.push_back (Point<dim>(deal_II_numbers::PI * i,
						deal_II_numbers::PI * j));
		k_vectors_magnitude.push_back (i*i+j*j);
	      }
	
	break;
      }

      case 3:
      {
	for (unsigned int i=0; i<N; ++i)
	  for (unsigned int j=0; j<N; ++j)
	    for (unsigned int k=0; k<N; ++k)
	      if (!((i==0) && (j==0) && (k==0))
		  &&
		  (i*i + j*j + k*k < N*N))
		{
		  k_vectors.push_back (Point<dim>(deal_II_numbers::PI * i,
						  deal_II_numbers::PI * j,
						  deal_II_numbers::PI * k));
		  k_vectors_magnitude.push_back (i*i+j*j+k*k);
	      }
	
	break;
      }
      
      default:
	    Assert (false, ExcNotImplemented());
    }

  const unsigned n_fourier_modes = k_vectors.size();
  std::vector<double> ln_k (n_fourier_modes);
  for (unsigned int i=0; i<n_fourier_modes; ++i)
    ln_k[i] = std::log (k_vectors[i].norm());
  

				   // assemble the matrices that do
				   // the Fourier transforms for each
				   // of the finite elements we deal
				   // with. note that these matrices
				   // are complex-valued, so we can't
				   // use FullMatrix
  QGauss<1>      base_quadrature (2);
  QIterated<dim> quadrature (base_quadrature, N);
  
  std::vector<Table<2,std::complex<double> > >
    fourier_transform_matrices (fe_collection.size());
  for (unsigned int fe=0; fe<fe_collection.size(); ++fe)
    {
      fourier_transform_matrices[fe].reinit (n_fourier_modes,
					     fe_collection[fe].dofs_per_cell);

      for (unsigned int k=0; k<n_fourier_modes; ++k)
	for (unsigned int i=0; i<fe_collection[fe].dofs_per_cell; ++i)
	  {
	    std::complex<double> sum = 0;
	    for (unsigned int q=0; q<quadrature.n_quadrature_points; ++q)
	      {
		const Point<dim> x_q = quadrature.point(q);
		sum += std::exp(std::complex<double>(0,1) *
				(k_vectors[k] * x_q)) *
		       fe_collection[fe].shape_value(i,x_q) *
		       quadrature.weight(q);
	      }
	    fourier_transform_matrices[fe](k,i)
	      = sum / std::pow(2*deal_II_numbers::PI, 1.*dim/2);
	  }
    }

				   // the next thing is to loop over
				   // all cells and do our work there,
				   // i.e. to locally do the Fourier
				   // transform and estimate the decay
				   // coefficient
  std::vector<std::complex<double> > fourier_coefficients (n_fourier_modes);
  Vector<double>                     local_dof_values;
  
  typename hp::DoFHandler<dim>::active_cell_iterator
    cell = dof_handler.begin_active(),
    endc = dof_handler.end();
  for (unsigned int index=0; cell!=endc; ++cell, ++index)
    {
      local_dof_values.reinit (cell->get_fe().dofs_per_cell);
      cell->get_dof_values (solution, local_dof_values);

				       // first compute the Fourier
				       // transform of the local
				       // solution
      std::fill (fourier_coefficients.begin(), fourier_coefficients.end(), 0);
      for (unsigned int f=0; f<n_fourier_modes; ++f)
	for (unsigned int i=0; i<cell->get_fe().dofs_per_cell; ++i)
	  fourier_coefficients[f] += 
	    fourier_transform_matrices[cell->active_fe_index()](f,i)
	    *
	    local_dof_values(i);

				       // enter the Fourier
				       // coefficients into a map with
				       // the k-magnitudes, to make
				       // sure that we get only the
				       // largest magnitude for each
				       // value of |k|
      std::map<unsigned int, double> k_to_max_U_map;
      for (unsigned int f=0; f<n_fourier_modes; ++f)
	if ((k_to_max_U_map.find (k_vectors_magnitude[f]) ==
	     k_to_max_U_map.end())
	    ||
	    (k_to_max_U_map[k_vectors_magnitude[f]] <
	     std::abs (fourier_coefficients[f])))
	  k_to_max_U_map[k_vectors_magnitude[f]]
	    = std::abs (fourier_coefficients[f]);
      
				       // now we have to calculate the
				       // various contributions to the
				       // formula for mu. we'll only
				       // take those fourier
				       // coefficients with the
				       // largest value for a given
				       // |k|
      double  sum_1           = 0,
	      sum_ln_k        = 0,
	      sum_ln_k_square = 0,
	      sum_ln_U        = 0,
	      sum_ln_U_ln_k   = 0;
      for (unsigned int f=0; f<n_fourier_modes; ++f)
	if (k_to_max_U_map[k_vectors_magnitude[f]] ==
	    std::abs (fourier_coefficients[f]))
	  {
	    sum_1 += 1;
	    sum_ln_k += ln_k[f];
	    sum_ln_k_square += ln_k[f]*ln_k[f];
	    sum_ln_U += std::log (std::abs (fourier_coefficients[f]));
	    sum_ln_U_ln_k += std::log (std::abs (fourier_coefficients[f])) * ln_k[f];
	  }

      const double mu
	= (1./(sum_1*sum_ln_k_square - sum_ln_k*sum_ln_k)
	   *
	   (sum_ln_k*sum_ln_U - sum_1*sum_ln_U_ln_k));
      
      smoothness_indicators(index) = mu - 1.*dim/2;
    }
}


				 // @sect3{The main function}

				 // The main function is again verbatim what
				 // we had before: wrap creating and running
				 // an object of the main class into a
				 // <code>try</code> block and catch whatever
				 // exceptions are thrown, thereby producing
				 // meaningful output if anything should go
				 // wrong:
int main () 
{
  try
    {
      deallog.depth_console (0);

      LaplaceProblem<2> laplace_problem;
      laplace_problem.run ();
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
