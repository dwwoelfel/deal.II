/* $Id$ */
/* Author: Martin Kronbichler, Uppsala University,
           Wolfgang Bangerth, Texas A&M University 2007, 2008 */
/*                                                                */
/*    Copyright (C) 2008 by the deal.II authors */
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
#include <base/conditional_ostream.h>

#include <lac/full_matrix.h>
#include <lac/solver_gmres.h>
#include <lac/solver_cg.h>
#include <lac/trilinos_block_vector.h>
#include <lac/trilinos_sparse_matrix.h>
#include <lac/trilinos_block_sparse_matrix.h>
#include <lac/trilinos_precondition.h>

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

#include <Epetra_Map.h>

#include <fstream>
#include <iostream>
#include <sstream>


				 // Next, we import all deal.II
				 // names into global namespace
using namespace dealii;


				 // @sect3{Equation data}


				 // @sect4{Boundary values}
namespace EquationData
{
				   // define viscosity
  const double eta = 1;
  const double kappa = 1e-6;
  const double Rayleigh_number = 10;


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
  TemperatureInitialValues<dim>::value (const Point<dim>  &p,
					const unsigned int) const
  {
    return (p.norm() < 0.55+0.02*std::sin(p[0]*20) ? 1 : 0);
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
  TemperatureRightHandSide<dim>::value (const Point<dim>  &,
					const unsigned int /*component*/) const
  {
    return 0;
  }


  template <int dim>
  void
  TemperatureRightHandSide<dim>::vector_value (const Point<dim> &p,
					       Vector<double>   &values) const
  {
    for (unsigned int c=0; c<this->n_components; ++c)
      values(c) = TemperatureRightHandSide<dim>::value (p, c);
  }
}



				   // @sect3{Linear solvers and preconditioners}

namespace LinearSolvers
{
  template <class Matrix, class Preconditioner>
  class InverseMatrix : public Subscriptor
  {
    public:
      InverseMatrix (const Matrix         &m,
		     const Preconditioner &preconditioner);


      template <typename VectorType>
      void vmult (VectorType       &dst,
		  const VectorType &src) const;

    private:
      const SmartPointer<const Matrix> matrix;
      const Preconditioner &preconditioner;
  };


  template <class Matrix, class Preconditioner>
  InverseMatrix<Matrix,Preconditioner>::
  InverseMatrix (const Matrix &m,
		 const Preconditioner &preconditioner)
		  :
		  matrix (&m),
		  preconditioner (preconditioner)
  {}



  template <class Matrix, class Preconditioner>
  template <typename VectorType>
  void
  InverseMatrix<Matrix,Preconditioner>::
  vmult (VectorType       &dst,
	 const VectorType &src) const
  {
    SolverControl solver_control (src.size(), 1e-7*src.l2_norm());
    SolverCG<VectorType> cg (solver_control);

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



  template <class PreconditionerA, class PreconditionerMp>
  class BlockSchurPreconditioner : public Subscriptor
  {
    public:
      BlockSchurPreconditioner (
	const TrilinosWrappers::BlockSparseMatrix     &S,
	const InverseMatrix<TrilinosWrappers::SparseMatrix,PreconditionerMp>  &Mpinv,
	const PreconditionerA                         &Apreconditioner);

      void vmult (TrilinosWrappers::MPI::BlockVector       &dst,
		  const TrilinosWrappers::MPI::BlockVector &src) const;

    private:
      const SmartPointer<const TrilinosWrappers::BlockSparseMatrix> stokes_matrix;
      const SmartPointer<const InverseMatrix<TrilinosWrappers::SparseMatrix,
					     PreconditionerMp > > m_inverse;
      const PreconditionerA &a_preconditioner;
      mutable TrilinosWrappers::MPI::Vector tmp;

};



  template <class PreconditionerA, class PreconditionerMp>
  BlockSchurPreconditioner<PreconditionerA, PreconditionerMp>::
  BlockSchurPreconditioner(const TrilinosWrappers::BlockSparseMatrix  &S,
			   const InverseMatrix<TrilinosWrappers::SparseMatrix,PreconditionerMp> &Mpinv,
			   const PreconditionerA                      &Apreconditioner)
		  :
		  stokes_matrix           (&S),
		  m_inverse               (&Mpinv),
		  a_preconditioner        (Apreconditioner),
		  tmp                     (stokes_matrix->block(1,1).matrix->RowMap())
  {}



  template <class PreconditionerA, class PreconditionerMp>
  void BlockSchurPreconditioner<PreconditionerA, PreconditionerMp>::vmult (
    TrilinosWrappers::MPI::BlockVector       &dst,
    const TrilinosWrappers::MPI::BlockVector &src) const
  {
    a_preconditioner.vmult (dst.block(0), src.block(0));
    stokes_matrix->block(1,0).residual(tmp, dst.block(0), src.block(1));
    tmp *= -1;
    m_inverse->vmult (dst.block(1), tmp);
  }
}



				 // @sect3{The <code>BoussinesqFlowProblem</code> class template}
template <int dim>
class BoussinesqFlowProblem
{
  public:
    BoussinesqFlowProblem (Utilities::TrilinosTools &trilinos_tools);
    void run ();

  private:
    void setup_dofs ();
    void assemble_stokes_preconditioner ();
    void build_stokes_preconditioner ();
    void assemble_stokes_system ();
    void assemble_temperature_system ();
    void assemble_temperature_matrix ();
    double get_maximal_velocity () const;
    std::pair<double,double> get_extrapolated_temperature_range () const;
    void solve ();
    void output_results () const;
    void refine_mesh (const unsigned int max_grid_level);

    static
    double
    compute_viscosity(const std::vector<double>          &old_temperature,
		      const std::vector<double>          &old_old_temperature,
		      const std::vector<Tensor<1,dim> >  &old_temperature_grads,
		      const std::vector<Tensor<1,dim> >  &old_old_temperature_grads,
		      const std::vector<Tensor<2,dim> >  &old_temperature_hessians,
		      const std::vector<Tensor<2,dim> >  &old_old_temperature_hessians,
		      const std::vector<Vector<double> > &present_stokes_values,
		      const std::vector<double>          &gamma_values,
		      const double                        global_u_infty,
		      const double                        global_T_variation,
		      const double                        global_Omega_diameter,
		      const double                        cell_diameter,
		      const double                        old_time_step);


    Utilities::TrilinosTools            trilinos_tools;
    
    ConditionalOStream                  pcout;

    Triangulation<dim>                  triangulation;

    const unsigned int                  stokes_degree;
    FESystem<dim>                       stokes_fe;
    DoFHandler<dim>                     stokes_dof_handler;
    ConstraintMatrix                    stokes_constraints;

    std::vector<Epetra_Map>             stokes_partitioner;
    TrilinosWrappers::BlockSparseMatrix stokes_matrix;
    TrilinosWrappers::BlockSparseMatrix stokes_preconditioner_matrix;

    TrilinosWrappers::MPI::BlockVector  stokes_solution;
    TrilinosWrappers::MPI::BlockVector  stokes_rhs;


    const unsigned int                  temperature_degree;    
    FE_Q<dim>                           temperature_fe;
    DoFHandler<dim>                     temperature_dof_handler;
    ConstraintMatrix                    temperature_constraints;

    Epetra_Map                          temperature_partitioner;
    TrilinosWrappers::SparseMatrix      temperature_mass_matrix;
    TrilinosWrappers::SparseMatrix      temperature_stiffness_matrix;
    TrilinosWrappers::SparseMatrix      temperature_matrix;

    TrilinosWrappers::MPI::Vector       temperature_solution;
    TrilinosWrappers::Vector            old_temperature_solution;
    TrilinosWrappers::Vector            old_old_temperature_solution;
    TrilinosWrappers::MPI::Vector       temperature_rhs;


    double time_step;
    double old_time_step;
    unsigned int timestep_number;

    boost::shared_ptr<TrilinosWrappers::PreconditionAMG>  Amg_preconditioner;
    boost::shared_ptr<TrilinosWrappers::PreconditionIC> Mp_preconditioner;

    bool rebuild_stokes_matrix;
    bool rebuild_temperature_matrices;
    bool rebuild_stokes_preconditioner;
};


				 // @sect3{BoussinesqFlowProblem class implementation}

				 // @sect4{BoussinesqFlowProblem::BoussinesqFlowProblem}
template <int dim>
BoussinesqFlowProblem<dim>::BoussinesqFlowProblem (Utilities::TrilinosTools &trilinos_tools)
                :
                trilinos_tools (trilinos_tools),
		pcout (std::cout, trilinos_tools.comm().MyPID()==0),

		triangulation (Triangulation<dim>::maximum_smoothing),

                stokes_degree (1),
                stokes_fe (FE_Q<dim>(stokes_degree+1), dim,
			   FE_Q<dim>(stokes_degree), 1),
		stokes_dof_handler (triangulation),

		temperature_degree (2),
		temperature_fe (temperature_degree),
                temperature_dof_handler (triangulation),

		temperature_partitioner (0, 0, trilinos_tools.comm()),

                time_step (0),
		old_time_step (0),
		timestep_number (0),
		rebuild_stokes_matrix (true),
		rebuild_temperature_matrices (true),
		rebuild_stokes_preconditioner (true)
{}



				 // @sect4{BoussinesqFlowProblem::get_maximal_velocity}
template <int dim>
double BoussinesqFlowProblem<dim>::get_maximal_velocity () const
{
  const QIterated<dim> quadrature_formula (QTrapez<1>(),
					   stokes_degree+1);
  const unsigned int n_q_points = quadrature_formula.size();

  BlockVector<double> localized_stokes_solution (stokes_solution);

  FEValues<dim> fe_values (stokes_fe, quadrature_formula, update_values);
  std::vector<Vector<double> > stokes_values(n_q_points,
					     Vector<double>(dim+1));
  double max_velocity = 0;

  typename DoFHandler<dim>::active_cell_iterator
    cell = stokes_dof_handler.begin_active(),
    endc = stokes_dof_handler.end();
  for (; cell!=endc; ++cell)
    if (cell->subdomain_id() == (unsigned int)trilinos_tools.comm().MyPID())
      {
	fe_values.reinit (cell);
	fe_values.get_function_values (localized_stokes_solution, stokes_values);

	for (unsigned int q=0; q<n_q_points; ++q)
	  {
	    Tensor<1,dim> velocity;
	    for (unsigned int i=0; i<dim; ++i)
	      velocity[i] = stokes_values[q](i);

	    max_velocity = std::max (max_velocity, velocity.norm());
	  }
      }

  return max_velocity;
}




				 // @sect4{BoussinesqFlowProblem::get_extrapolated_temperature_range}
template <int dim>
std::pair<double,double>
BoussinesqFlowProblem<dim>::get_extrapolated_temperature_range () const
{
  const QIterated<dim> quadrature_formula (QTrapez<1>(),
					   temperature_degree);
  const unsigned int n_q_points = quadrature_formula.size();

  FEValues<dim> fe_values (temperature_fe, quadrature_formula,
                           update_values);
  std::vector<double> old_temperature_values(n_q_points);
  std::vector<double> old_old_temperature_values(n_q_points);

  if (timestep_number != 0)
    {
      double min_temperature = (1. + time_step/old_time_step) *
			       old_temperature_solution.linfty_norm()
			       +
			       time_step/old_time_step *
			       old_old_temperature_solution.linfty_norm(),
	     max_temperature = -min_temperature;

      typename DoFHandler<dim>::active_cell_iterator
	cell = temperature_dof_handler.begin_active(),
	endc = temperature_dof_handler.end();
      for (; cell!=endc; ++cell)
	{
	  fe_values.reinit (cell);
	  fe_values.get_function_values (old_temperature_solution,
					 old_temperature_values);
	  fe_values.get_function_values (old_old_temperature_solution,
					 old_old_temperature_values);

	  for (unsigned int q=0; q<n_q_points; ++q)
	    {
	      const double temperature = 
		(1. + time_step/old_time_step) * old_temperature_values[q]-
		time_step/old_time_step * old_old_temperature_values[q];

	      min_temperature = std::min (min_temperature, temperature);
	      max_temperature = std::max (max_temperature, temperature);
	    }
	}

      return std::make_pair(min_temperature, max_temperature);
    }
  else
    {
      double min_temperature = old_temperature_solution.linfty_norm(),
	     max_temperature = -min_temperature;

      typename DoFHandler<dim>::active_cell_iterator
	cell = temperature_dof_handler.begin_active(),
	endc = temperature_dof_handler.end();
      for (; cell!=endc; ++cell)
	{
	  fe_values.reinit (cell);
	  fe_values.get_function_values (old_temperature_solution,
					 old_temperature_values);

	  for (unsigned int q=0; q<n_q_points; ++q)
	    {
	      const double temperature = old_temperature_values[q];

	      min_temperature = std::min (min_temperature, temperature);
	      max_temperature = std::max (max_temperature, temperature);
	    }
	}
  
      return std::make_pair(min_temperature, max_temperature);
    }    
}



template <int dim>
double
BoussinesqFlowProblem<dim>::
compute_viscosity(const std::vector<double>          &old_temperature,
		  const std::vector<double>          &old_old_temperature,
		  const std::vector<Tensor<1,dim> >  &old_temperature_grads,
		  const std::vector<Tensor<1,dim> >  &old_old_temperature_grads,
		  const std::vector<Tensor<2,dim> >  &old_temperature_hessians,
		  const std::vector<Tensor<2,dim> >  &old_old_temperature_hessians,
		  const std::vector<Vector<double> > &present_stokes_values,
		  const std::vector<double>          &gamma_values,
		  const double                        global_u_infty,
		  const double                        global_T_variation,
		  const double                        global_Omega_diameter,
		  const double                        cell_diameter,
		  const double                        old_time_step)
{
  const double beta = 0.015 * dim;
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
      
      const double kappa_Delta_T = EquationData::kappa
				   * (trace(old_temperature_hessians[q]) +
				      trace(old_old_temperature_hessians[q])) / 2;

      const double residual
	= std::abs((dT_dt + u_grad_T - kappa_Delta_T - gamma_values[q]) *
		   std::pow((old_temperature[q]+old_old_temperature[q]) / 2,
			    alpha-1.));

      max_residual = std::max (residual,        max_residual);
      max_velocity = std::max (std::sqrt (u*u), max_velocity);
    }
  
  const double global_scaling = global_u_infty * global_T_variation /
				std::pow(global_Omega_diameter, alpha - 2.);

  return (beta *
	  max_velocity *
	  std::min (cell_diameter,
		    std::pow(cell_diameter,alpha) * max_residual / global_scaling));
}



				 // @sect4{BoussinesqFlowProblem::setup_dofs}
template <int dim>
void BoussinesqFlowProblem<dim>::setup_dofs ()
{
  std::vector<unsigned int> stokes_sub_blocks (dim+1,0);
  stokes_sub_blocks[dim] = 1;

  GridTools::partition_triangulation (trilinos_tools.comm().NumProc(), 
				      triangulation);

  {
    stokes_dof_handler.distribute_dofs (stokes_fe);
    DoFRenumbering::Cuthill_McKee (stokes_dof_handler);
    DoFRenumbering::subdomain_wise (stokes_dof_handler);
    DoFRenumbering::component_wise (stokes_dof_handler, stokes_sub_blocks);
    
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
    DoFRenumbering::subdomain_wise (temperature_dof_handler);

    temperature_constraints.clear ();
    DoFTools::make_hanging_node_constraints (temperature_dof_handler,
					     temperature_constraints);
    temperature_constraints.close ();
  }
  
  std::vector<unsigned int> stokes_dofs_per_block (2);
  DoFTools::count_dofs_per_block (stokes_dof_handler, stokes_dofs_per_block,
				  stokes_sub_blocks);
  
  const unsigned int n_u = stokes_dofs_per_block[0],
                     n_p = stokes_dofs_per_block[1],
		     n_T = temperature_dof_handler.n_dofs();

  pcout << "Number of active cells: "
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

  stokes_partitioner.clear();
  {
    std::vector<unsigned int> local_dofs (dim+1);
    DoFTools::
      count_dofs_with_subdomain_association (stokes_dof_handler,
					     trilinos_tools.comm().MyPID(),
					     local_dofs);
    unsigned int n_local_velocities = 0;
    for (unsigned int c=0; c<dim; ++c)
      n_local_velocities += local_dofs[c];

    const unsigned int n_local_pressures = local_dofs[dim];

    Epetra_Map map_u(n_u, n_local_velocities, 0, trilinos_tools.comm());
    stokes_partitioner.push_back (map_u);
    Epetra_Map map_p(n_p, n_local_pressures, 0, trilinos_tools.comm());
    stokes_partitioner.push_back (map_p);
  }
  {
    stokes_matrix.clear ();

    BlockCompressedSetSparsityPattern csp (2,2);

    csp.block(0,0).reinit (n_u, n_u);
    csp.block(0,1).reinit (n_u, n_p);
    csp.block(1,0).reinit (n_p, n_u);
    csp.block(1,1).reinit (n_p, n_p);

    csp.collect_sizes ();

    Table<2,DoFTools::Coupling> coupling (dim+1, dim+1);

    for (unsigned int c=0; c<dim+1; ++c)
      for (unsigned int d=0; d<dim+1; ++d)
	if (! ((c==dim) && (d==dim)))
	  coupling[c][d] = DoFTools::always;
	else
	  coupling[c][d] = DoFTools::none;

    DoFTools::make_sparsity_pattern (stokes_dof_handler, coupling, csp,
				     stokes_constraints, false);

    stokes_matrix.reinit (stokes_partitioner, csp);
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
    for (unsigned int c=0; c<dim+1; ++c)
      for (unsigned int d=0; d<dim+1; ++d)
	if (c == d)
	  coupling[c][d] = DoFTools::always;
	else
	  coupling[c][d] = DoFTools::none;

    DoFTools::make_sparsity_pattern (stokes_dof_handler, coupling, csp,
				     stokes_constraints, false);

    stokes_preconditioner_matrix.reinit (stokes_partitioner, csp);
  }

  temperature_partitioner
    = Epetra_Map (n_T,
		  DoFTools::count_dofs_with_subdomain_association
		  (temperature_dof_handler,
		   trilinos_tools.comm().MyPID()),
		  0,
		  trilinos_tools.comm());
  {
    temperature_mass_matrix.clear ();
    temperature_stiffness_matrix.clear ();
    temperature_matrix.clear ();

    CompressedSetSparsityPattern csp (n_T, n_T);
    DoFTools::make_sparsity_pattern (temperature_dof_handler, csp,
				     temperature_constraints, false);

    temperature_matrix.reinit (temperature_partitioner, csp);
    temperature_mass_matrix.reinit (temperature_partitioner, csp);
    temperature_stiffness_matrix.reinit (temperature_partitioner, csp);
  }

  stokes_solution.reinit (stokes_partitioner);
  stokes_rhs.reinit (stokes_partitioner);

  temperature_solution.reinit (temperature_partitioner);
  old_temperature_solution.reinit (temperature_partitioner);
  old_old_temperature_solution.reinit (temperature_partitioner);
  temperature_rhs.reinit (temperature_partitioner);
}



template <int dim>
void
BoussinesqFlowProblem<dim>::assemble_stokes_preconditioner ()
{
  stokes_preconditioner_matrix = 0;

  const QGauss<dim> quadrature_formula (stokes_degree+2);
  FEValues<dim>     stokes_fe_values (stokes_fe, quadrature_formula,
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
    if (cell->subdomain_id() == (unsigned int)trilinos_tools.comm().MyPID())
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
		local_matrix(i,j) += (EquationData::eta *
				      scalar_product (phi_grad_u[i], phi_grad_u[j])
				      +
				      (1./EquationData::eta) *
				      phi_p[i] * phi_p[j])
				    * stokes_fe_values.JxW(q);
	  }

	cell->get_dof_indices (local_dof_indices);
	stokes_constraints.distribute_local_to_global (local_matrix,
						       local_dof_indices,
						       stokes_preconditioner_matrix);
      }

  stokes_preconditioner_matrix.compress();
}



template <int dim>
void
BoussinesqFlowProblem<dim>::build_stokes_preconditioner ()
{
  if (rebuild_stokes_preconditioner == false)
    return;

  pcout << "   Rebuilding Stokes preconditioner..." << std::flush;

  assemble_stokes_preconditioner ();

  Amg_preconditioner = boost::shared_ptr<TrilinosWrappers::PreconditionAMG>
		       (new TrilinosWrappers::PreconditionAMG());

  std::vector<std::vector<bool> > null_space;
  std::vector<bool>  velocity_components (dim+1,true);
  velocity_components[dim] = false;
  DoFTools::extract_constant_modes (stokes_dof_handler, velocity_components, 
				    null_space);
  
  Amg_preconditioner->initialize(stokes_preconditioner_matrix.block(0,0),
				 TrilinosWrappers::PreconditionAMG::AdditionalData
				   (true, true, 5e-2, null_space, 0, false));

  Mp_preconditioner = boost::shared_ptr<TrilinosWrappers::PreconditionIC>
                                   (new TrilinosWrappers::PreconditionIC());

  Mp_preconditioner->initialize (stokes_preconditioner_matrix.block(1,1));

  pcout << std::endl;

  rebuild_stokes_preconditioner = false;
}



				 // @sect4{BoussinesqFlowProblem::assemble_stokes_system}
template <int dim>
void BoussinesqFlowProblem<dim>::assemble_stokes_system ()
{
  pcout << "   Assembling..." << std::flush;

  if (rebuild_stokes_matrix == true)
    stokes_matrix=0;

  stokes_rhs=0;

  const QGauss<dim> quadrature_formula(stokes_degree+2);
  FEValues<dim>     stokes_fe_values (stokes_fe, quadrature_formula,
				      update_values    |
				      update_quadrature_points  |
				      update_JxW_values |
				      (rebuild_stokes_matrix == true
				       ?
				       update_gradients
				       :
				       UpdateFlags(0)));
  
  FEValues<dim>     temperature_fe_values (temperature_fe, quadrature_formula,
					   update_values);

  const unsigned int   dofs_per_cell   = stokes_fe.dofs_per_cell;

  const unsigned int   n_q_points      = quadrature_formula.size();

  FullMatrix<double>   local_matrix (dofs_per_cell, dofs_per_cell);
  Vector<double>       local_rhs (dofs_per_cell);

  std::vector<unsigned int> local_dof_indices (dofs_per_cell);

  std::vector<double>               old_temperature_values(n_q_points);

  std::vector<Tensor<1,dim> >          phi_u       (dofs_per_cell);
  std::vector<SymmetricTensor<2,dim> > grads_phi_u (dofs_per_cell);
  std::vector<double>                  div_phi_u   (dofs_per_cell);
  std::vector<double>                  phi_p       (dofs_per_cell);
  
  const FEValuesExtractors::Vector velocities (0);
  const FEValuesExtractors::Scalar pressure (dim);

  typename DoFHandler<dim>::active_cell_iterator
    cell = stokes_dof_handler.begin_active(),
    endc = stokes_dof_handler.end();
  typename DoFHandler<dim>::active_cell_iterator
    temperature_cell = temperature_dof_handler.begin_active();
  
  for (; cell!=endc; ++cell, ++temperature_cell)
    if (cell->subdomain_id() == (unsigned int)trilinos_tools.comm().MyPID())
      {
	stokes_fe_values.reinit (cell);
	temperature_fe_values.reinit (temperature_cell);
	
	local_matrix = 0;
	local_rhs = 0;
  
	temperature_fe_values.get_function_values (old_temperature_solution, 
						   old_temperature_values);
  
	for (unsigned int q=0; q<n_q_points; ++q)
	  {
	    const double old_temperature = old_temperature_values[q];
  
	    for (unsigned int k=0; k<dofs_per_cell; ++k)
	      {
		phi_u[k] = stokes_fe_values[velocities].value (k,q);
		if (rebuild_stokes_matrix)
		  {
		    grads_phi_u[k] = stokes_fe_values[velocities].symmetric_gradient(k,q);
		    div_phi_u[k]   = stokes_fe_values[velocities].divergence (k, q);
		    phi_p[k]       = stokes_fe_values[pressure].value (k, q);
		  }
	      }
  
	    if (rebuild_stokes_matrix)
	      for (unsigned int i=0; i<dofs_per_cell; ++i)
		for (unsigned int j=0; j<dofs_per_cell; ++j)
		  local_matrix(i,j) += (EquationData::eta *
					grads_phi_u[i] * grads_phi_u[j]
					- div_phi_u[i] * phi_p[j]
					- phi_p[i] * div_phi_u[j])
				      * stokes_fe_values.JxW(q);

					     // use gravity radially outward
	    const Point<dim> gravity = stokes_fe_values.quadrature_point(q) /
				       stokes_fe_values.quadrature_point(q).norm();
	    for (unsigned int i=0; i<dofs_per_cell; ++i)
	      local_rhs(i) += (EquationData::Rayleigh_number *
			      gravity * phi_u[i] * old_temperature)*
			      stokes_fe_values.JxW(q);
	  }
  
	cell->get_dof_indices (local_dof_indices);
  
	if (rebuild_stokes_matrix == true)
	  stokes_constraints.distribute_local_to_global (local_matrix,
							 local_dof_indices,
							 stokes_matrix);
  
	stokes_constraints.distribute_local_to_global (local_rhs,
						       local_dof_indices,
						       stokes_rhs);
      }

  stokes_matrix.compress();
  stokes_rhs.compress();

  rebuild_stokes_matrix = false;

  pcout << std::endl;
}






				 // @sect4{BoussinesqFlowProblem::assemble_temperature_system}
template <int dim>
void BoussinesqFlowProblem<dim>::assemble_temperature_matrix ()
{
  if (rebuild_temperature_matrices == false)
    return;
  
  temperature_mass_matrix = 0;
  temperature_stiffness_matrix = 0;
  
  const QGauss<dim> quadrature_formula(temperature_degree+2);
  FEValues<dim>     temperature_fe_values (temperature_fe, quadrature_formula,
					   update_values    | update_gradients |
					   update_JxW_values);

  const unsigned int   dofs_per_cell   = temperature_fe.dofs_per_cell;
  const unsigned int   n_q_points      = quadrature_formula.size();

  FullMatrix<double>   local_mass_matrix (dofs_per_cell, dofs_per_cell);
  FullMatrix<double>   local_stiffness_matrix (dofs_per_cell, dofs_per_cell);

  std::vector<unsigned int> local_dof_indices (dofs_per_cell);

  std::vector<double>                  phi_T       (dofs_per_cell);
  std::vector<Tensor<1,dim> >          grad_phi_T  (dofs_per_cell);

  typename DoFHandler<dim>::active_cell_iterator
    cell = temperature_dof_handler.begin_active(),
    endc = temperature_dof_handler.end();
  for (; cell!=endc; ++cell)
    if (cell->subdomain_id() == (unsigned int)trilinos_tools.comm().MyPID())
      {
	local_mass_matrix = 0;
	local_stiffness_matrix = 0;
  
	temperature_fe_values.reinit (cell);
	
	for (unsigned int q=0; q<n_q_points; ++q)
	  {
	    for (unsigned int k=0; k<dofs_per_cell; ++k)
	      {
		grad_phi_T[k] = temperature_fe_values.shape_grad (k,q);
		phi_T[k]      = temperature_fe_values.shape_value (k, q);
	      }
	    
	    for (unsigned int i=0; i<dofs_per_cell; ++i)
	      for (unsigned int j=0; j<dofs_per_cell; ++j)
		{
		  local_mass_matrix(i,j)
		    += (phi_T[i] * phi_T[j]
			*
			temperature_fe_values.JxW(q));
		  local_stiffness_matrix(i,j)
		    += (EquationData::kappa * grad_phi_T[i] * grad_phi_T[j]
			*
			temperature_fe_values.JxW(q));
		}
	  }
	
	cell->get_dof_indices (local_dof_indices);
  
	temperature_constraints.distribute_local_to_global (local_mass_matrix,
							    local_dof_indices,
							    temperature_mass_matrix);
	temperature_constraints.distribute_local_to_global (local_stiffness_matrix,
							    local_dof_indices,
							    temperature_stiffness_matrix);
      }

  temperature_mass_matrix.compress();
  temperature_stiffness_matrix.compress();

  rebuild_temperature_matrices = false;
}




template <int dim>
void BoussinesqFlowProblem<dim>::assemble_temperature_system ()
{
  const bool use_bdf2_scheme = (timestep_number != 0);

  if (use_bdf2_scheme == true)
    {
      temperature_matrix.copy_from (temperature_mass_matrix);
      temperature_matrix *= (2*time_step + old_time_step) /
			    (time_step + old_time_step);
      temperature_matrix.add (time_step, temperature_stiffness_matrix);
    }
  else
    {
      temperature_matrix.copy_from (temperature_mass_matrix);
      temperature_matrix.add (time_step, temperature_stiffness_matrix);
    }
  temperature_matrix.compress();
  
  temperature_rhs = 0;
  
  const QGauss<dim> quadrature_formula(temperature_degree+2);
  FEValues<dim>     temperature_fe_values (temperature_fe, quadrature_formula,
					   update_values    |
					   update_gradients |
					   update_hessians  |
					   update_quadrature_points |
					   update_JxW_values);
  FEValues<dim> stokes_fe_values (stokes_fe, quadrature_formula,
				  update_values);

  const unsigned int   dofs_per_cell   = temperature_fe.dofs_per_cell;
  const unsigned int   n_q_points      = quadrature_formula.size();

  Vector<double>       local_rhs (dofs_per_cell);
  FullMatrix<double>   local_matrix (dofs_per_cell, dofs_per_cell);

  std::vector<unsigned int> local_dof_indices (dofs_per_cell);

  std::vector<Vector<double> > present_stokes_values (n_q_points, 
						      Vector<double>(dim+1));

  
  std::vector<double>         old_temperature_values (n_q_points);
  std::vector<double>         old_old_temperature_values(n_q_points);
  std::vector<Tensor<1,dim> > old_temperature_grads(n_q_points);
  std::vector<Tensor<1,dim> > old_old_temperature_grads(n_q_points);
  std::vector<Tensor<2,dim> > old_temperature_hessians(n_q_points);
  std::vector<Tensor<2,dim> > old_old_temperature_hessians(n_q_points);

  
  EquationData::TemperatureRightHandSide<dim>  temperature_right_hand_side;
  std::vector<double> gamma_values (n_q_points);

  std::vector<double>                  phi_T       (dofs_per_cell);
  std::vector<Tensor<1,dim> >          grad_phi_T  (dofs_per_cell);
  
  const double global_u_infty = get_maximal_velocity();
  const std::pair<double,double>
    global_T_range = get_extrapolated_temperature_range();
  const double global_Omega_diameter = GridTools::diameter (triangulation);

  const TrilinosWrappers::BlockVector localized_stokes_solution (stokes_solution);

  typename DoFHandler<dim>::active_cell_iterator
    cell = temperature_dof_handler.begin_active(),
    endc = temperature_dof_handler.end();
  typename DoFHandler<dim>::active_cell_iterator
    stokes_cell = stokes_dof_handler.begin_active();

  for (; cell!=endc; ++cell, ++stokes_cell)
    if (cell->subdomain_id() == (unsigned int)trilinos_tools.comm().MyPID() )
      {
	local_rhs = 0;
  
	temperature_fe_values.reinit (cell);
	stokes_fe_values.reinit (stokes_cell);
  
	temperature_fe_values.get_function_values (old_temperature_solution,
						   old_temperature_values);
	temperature_fe_values.get_function_values (old_old_temperature_solution,
						   old_old_temperature_values);
  
	temperature_fe_values.get_function_gradients (old_temperature_solution,
						      old_temperature_grads);
	temperature_fe_values.get_function_gradients (old_old_temperature_solution,
						      old_old_temperature_grads);
	
	temperature_fe_values.get_function_hessians (old_temperature_solution,
						     old_temperature_hessians);
	temperature_fe_values.get_function_hessians (old_old_temperature_solution,
						     old_old_temperature_hessians);
	
	temperature_right_hand_side.value_list (temperature_fe_values.get_quadrature_points(),
						gamma_values);
  
	stokes_fe_values.get_function_values (localized_stokes_solution,
					      present_stokes_values);
	
	const double nu
	  = compute_viscosity (old_temperature_values,
			       old_old_temperature_values,
			       old_temperature_grads,
			       old_old_temperature_grads,
			       old_temperature_hessians,
			       old_old_temperature_hessians,
			       present_stokes_values,
			       gamma_values,
			       global_u_infty,
			       global_T_range.second - global_T_range.first,
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
	temperature_constraints.distribute_local_to_global (local_rhs,
							    local_dof_indices,
							    temperature_rhs);
      }

  temperature_rhs.compress();
}




				 // @sect4{BoussinesqFlowProblem::solve}
template <int dim>
void BoussinesqFlowProblem<dim>::solve ()
{
  pcout << "   Solving..." << std::endl;

  {
    LinearSolvers::InverseMatrix<TrilinosWrappers::SparseMatrix,
				 TrilinosWrappers::PreconditionIC>
      mp_inverse (stokes_preconditioner_matrix.block(1,1), *Mp_preconditioner);

    LinearSolvers::BlockSchurPreconditioner<TrilinosWrappers::PreconditionAMG,
                                            TrilinosWrappers::PreconditionIC>
      preconditioner (stokes_matrix, mp_inverse, *Amg_preconditioner);

    SolverControl solver_control (stokes_matrix.m(),
				  1e-6*stokes_rhs.l2_norm());

    SolverGMRES<TrilinosWrappers::MPI::BlockVector> gmres(solver_control,
      SolverGMRES<TrilinosWrappers::MPI::BlockVector >::AdditionalData(100));

    gmres.solve(stokes_matrix, stokes_solution, stokes_rhs, preconditioner);

    pcout << "   "
	  << solver_control.last_step()
	  << " GMRES iterations for Stokes subsystem."
	  << std::endl;

    TrilinosWrappers::BlockVector localized_stokes_solution (stokes_solution);
    stokes_constraints.distribute (localized_stokes_solution);
    stokes_solution = localized_stokes_solution;
  }

  old_time_step = time_step;    
  time_step = 1./(1.6*dim*std::sqrt(1.*dim)) /
	      temperature_degree *
	      GridTools::minimal_cell_diameter(triangulation) /
              std::max (get_maximal_velocity(), 1.e-5);
  
  temperature_solution = old_temperature_solution;


  assemble_temperature_system ();
  {

    SolverControl solver_control (temperature_matrix.m(),
				  1e-8*temperature_rhs.l2_norm());
    SolverCG<TrilinosWrappers::MPI::Vector>   cg (solver_control);

    TrilinosWrappers::PreconditionSSOR preconditioner;
    preconditioner.initialize (temperature_matrix, 1.2);

    cg.solve (temperature_matrix, temperature_solution,
	      temperature_rhs, preconditioner);

    TrilinosWrappers::Vector localized_temperature_solution (temperature_solution);
    temperature_constraints.distribute (localized_temperature_solution);
    temperature_solution = localized_temperature_solution;

    pcout << "   "
	  << solver_control.last_step()
	  << " CG iterations for temperature."
	  << std::endl;

    double min_temperature = localized_temperature_solution(0),
	   max_temperature = localized_temperature_solution(0);
    for (unsigned int i=0; i<temperature_solution.size(); ++i)
      {
	min_temperature = std::min<double> (min_temperature,
					    localized_temperature_solution(i));
	max_temperature = std::max<double> (max_temperature,
					    localized_temperature_solution(i));
      }
    
    pcout << "   Temperature range: "
	  << min_temperature << ' ' << max_temperature
	  << std::endl;
  }
}



				 // @sect4{BoussinesqFlowProblem::output_results}
template <int dim>
void BoussinesqFlowProblem<dim>::output_results ()  const
{
  if (timestep_number % 10 != 0)
    return;

  const FESystem<dim> joint_fe (stokes_fe, 1,
				temperature_fe, 1);
  DoFHandler<dim> joint_dof_handler (triangulation);
  joint_dof_handler.distribute_dofs (joint_fe);
  Assert (joint_dof_handler.n_dofs() ==
	  stokes_dof_handler.n_dofs() + temperature_dof_handler.n_dofs(),
	  ExcInternalError());
  
  Vector<double> joint_solution (joint_dof_handler.n_dofs());
  TrilinosWrappers::BlockVector localized_stokes_solution (stokes_solution);
  TrilinosWrappers::Vector localized_temperature_solution (temperature_solution);

  {
    std::vector<unsigned int> local_joint_dof_indices (joint_fe.dofs_per_cell);
    std::vector<unsigned int> local_stokes_dof_indices (stokes_fe.dofs_per_cell);
    std::vector<unsigned int> local_temperature_dof_indices (temperature_fe.dofs_per_cell);
    
    typename DoFHandler<dim>::active_cell_iterator
      joint_cell       = joint_dof_handler.begin_active(),
      joint_endc       = joint_dof_handler.end(),
      stokes_cell      = stokes_dof_handler.begin_active(),
      temperature_cell = temperature_dof_handler.begin_active();
    for (; joint_cell!=joint_endc; ++joint_cell, ++stokes_cell, ++temperature_cell)
      {
	joint_cell->get_dof_indices (local_joint_dof_indices);
	stokes_cell->get_dof_indices (local_stokes_dof_indices);
	temperature_cell->get_dof_indices (local_temperature_dof_indices);

	for (unsigned int i=0; i<joint_fe.dofs_per_cell; ++i)
	  if (joint_fe.system_to_base_index(i).first.first == 0)
	    {
	      Assert (joint_fe.system_to_base_index(i).second
		      <
		      local_stokes_dof_indices.size(),
		      ExcInternalError());
	      joint_solution(local_joint_dof_indices[i])
		= localized_stokes_solution(local_stokes_dof_indices[joint_fe.system_to_base_index(i).second]);
	    }
	  else
	    {
	      Assert (joint_fe.system_to_base_index(i).first.first == 1,
		      ExcInternalError());
	      Assert (joint_fe.system_to_base_index(i).second
		      <
		      local_stokes_dof_indices.size(),
		      ExcInternalError());
	      joint_solution(local_joint_dof_indices[i])
		= localized_temperature_solution(local_temperature_dof_indices[joint_fe.system_to_base_index(i).second]);
	    }
      }
  }
  
  
  std::vector<std::string> joint_solution_names (dim, "velocity");
  joint_solution_names.push_back ("p");
  joint_solution_names.push_back ("T");

  DataOut<dim> data_out;

  if (trilinos_tools.comm().MyPID() == 0)
    {
      data_out.attach_dof_handler (joint_dof_handler);

      std::vector<DataComponentInterpretation::DataComponentInterpretation>
	data_component_interpretation
	(dim+2, DataComponentInterpretation::component_is_scalar);
      for (unsigned int i=0; i<dim; ++i)
	data_component_interpretation[i]
	  = DataComponentInterpretation::component_is_part_of_vector;

      data_out.add_data_vector (joint_solution, joint_solution_names,
				DataOut<dim>::type_dof_data,
				data_component_interpretation);
      data_out.build_patches (std::min(stokes_degree, temperature_degree));

      std::ostringstream filename;
      filename << "solution-" << Utilities::int_to_string(timestep_number, 4) << ".vtk";

      std::ofstream output (filename.str().c_str());
      data_out.write_vtk (output);
    }
}



				 // @sect4{BoussinesqFlowProblem::refine_mesh}
template <int dim>
void BoussinesqFlowProblem<dim>::refine_mesh (const unsigned int max_grid_level)
{
  Vector<float> estimated_error_per_cell (triangulation.n_active_cells());

  TrilinosWrappers::Vector localized_temperature_solution (temperature_solution);

  KellyErrorEstimator<dim>::estimate (temperature_dof_handler,
				      QGauss<dim-1>(temperature_degree+1),
				      typename FunctionMap<dim>::type(),
				      localized_temperature_solution,
				      estimated_error_per_cell);

  GridRefinement::refine_and_coarsen_fixed_fraction (triangulation,
						     estimated_error_per_cell,
						     0.8, 0.1);
  if (triangulation.n_levels() > max_grid_level) 
    for (typename Triangulation<dim>::active_cell_iterator
	   cell = triangulation.begin_active(max_grid_level);
	 cell != triangulation.end(); ++cell)
      cell->clear_refine_flag ();
  
  std::vector<TrilinosWrappers::Vector > x_solution (2);
  x_solution[0] = temperature_solution;
  x_solution[1] = old_temperature_solution;

  SolutionTransfer<dim,TrilinosWrappers::Vector > soltrans(temperature_dof_handler);

  triangulation.prepare_coarsening_and_refinement();
  soltrans.prepare_for_coarsening_and_refinement(x_solution);

  triangulation.execute_coarsening_and_refinement ();
  setup_dofs ();

  std::vector<TrilinosWrappers::Vector > tmp (2);
  tmp[0] = temperature_solution;
  tmp[1] = temperature_solution;
  soltrans.interpolate(x_solution, tmp);

  temperature_solution = tmp[0];
  old_temperature_solution = tmp[1];

  rebuild_stokes_matrix         = true;
  rebuild_temperature_matrices  = true;
  rebuild_stokes_preconditioner = true;
}



				 // @sect4{BoussinesqFlowProblem::run}
template <int dim>
void BoussinesqFlowProblem<dim>::run ()
{
  const unsigned int initial_refinement = (dim == 2 ? 3 : 2);
  const unsigned int n_pre_refinement_steps = (dim == 2 ? 4 : 3);

  GridGenerator::half_hyper_shell (triangulation,
				   Point<dim>(), 0.5, 1.0);

  static HalfHyperShellBoundary<dim> boundary;
  triangulation.set_boundary (0, boundary);

  triangulation.refine_global (initial_refinement);

  setup_dofs();

  unsigned int       pre_refinement_step    = 0;
  
  start_time_iteration:

  VectorTools::project (temperature_dof_handler,
			temperature_constraints,
			QGauss<dim>(temperature_degree+2),
			EquationData::TemperatureInitialValues<dim>(),
			old_temperature_solution);
  
  timestep_number           = 0;
  time_step = old_time_step = 0;

  double time = 0;

  do
    {
      pcout << "Timestep " << timestep_number
	    << ":  t=" << time
	    << ", dt=" << time_step
	    << std::endl;

      assemble_stokes_system ();
      build_stokes_preconditioner ();
      assemble_temperature_matrix ();

      solve ();

      output_results ();

      pcout << std::endl;
      
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
  while (time <= 100);
}



				 // @sect3{The <code>main</code> function}
int main (int argc, char *argv[])
{
  try
    {
      deallog.depth_console (0);
      
      Utilities::TrilinosTools trilinos(&argc, &argv);

      BoussinesqFlowProblem<2> flow_problem (trilinos);
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
