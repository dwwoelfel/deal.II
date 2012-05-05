//------------------  matrix_vector_03.cc  ------------------------
//    $Id$
//    Version: $Name$
//
//------------------  matrix_vector_03.cc  ------------------------


// this function tests the correctness of the implementation of matrix free
// matrix-vector products by comparing with the result of deal.II sparse
// matrix. The mesh uses a hypercube mesh with hanging nodes (created by
// randomly refining some cells) and zero Dirichlet conditions.

#include "../tests.h"
#include <deal.II/base/function.h>

std::ofstream logfile("matrix_vector_03/output");

#include "matrix_vector_common.h"

template <int dim, int fe_degree>
void test ()
{
  Triangulation<dim> tria;
  GridGenerator::hyper_cube (tria);
  typename Triangulation<dim>::active_cell_iterator
    cell = tria.begin_active (),
    endc = tria.end();
  for (; cell!=endc; ++cell)
    if (cell->center().norm()<1e-8)
      cell->set_refine_flag();
  tria.execute_coarsening_and_refinement();
  cell = tria.begin_active ();
  for (; cell!=endc; ++cell)
    if (cell->center().norm()<0.2)
      cell->set_refine_flag();
  tria.execute_coarsening_and_refinement();
  if (dim < 3 || fe_degree < 2)
    tria.refine_global(2);
  else
    tria.refine_global(1);
  tria.begin(tria.n_levels()-1)->set_refine_flag();
  tria.last()->set_refine_flag();
  tria.execute_coarsening_and_refinement();
  cell = tria.begin_active ();
  for (unsigned int i=0; i<10-3*dim; ++i)
    {
      cell = tria.begin_active ();
      unsigned int counter = 0;
      for (; cell!=endc; ++cell, ++counter)
	if (counter % (7-i) == 0)
	  cell->set_refine_flag();
      tria.execute_coarsening_and_refinement();
    }

  FE_Q<dim> fe (fe_degree);
  DoFHandler<dim> dof (tria);
  dof.distribute_dofs(fe);
  ConstraintMatrix constraints;
  DoFTools::make_hanging_node_constraints(dof, constraints);
  VectorTools::interpolate_boundary_values (dof, 0, ZeroFunction<dim>(),
					    constraints);
  constraints.close();

  do_test<dim, fe_degree, double> (dof, constraints);
}
