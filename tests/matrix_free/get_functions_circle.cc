//------------------  get_functions_circle.cc  ------------------------
//    $Id$
//    Version: $Name$
//
//------------------  get_functions_circle.cc  ------------------------


// this function tests the correctness of the implementation of matrix free
// operations in getting the function values, the function gradients, and the
// function Laplacians on a hyperball mesh. This tests whether general Q1
// transformations work correctly. The test case includes hanging node
// constraints, but no constraints from boundary conditions

#include "../tests.h"

std::ofstream logfile("get_functions_circle/output");

#include "get_functions_common.h"


template <int dim, int fe_degree>
void test ()
{
  Triangulation<dim> tria;
  GridGenerator::hyper_ball (tria);
  static const HyperBallBoundary<dim> boundary;
  tria.set_boundary (0, boundary);
				// refine first and last cell
  tria.begin(tria.n_levels()-1)->set_refine_flag();
  tria.last()->set_refine_flag();
  tria.execute_coarsening_and_refinement();
  tria.refine_global (4-dim);

  FE_Q<dim> fe (fe_degree);
  DoFHandler<dim> dof (tria);
  dof.distribute_dofs(fe);

  ConstraintMatrix constraints;
  DoFTools::make_hanging_node_constraints (dof, constraints);
  constraints.close();

  do_test <dim, fe_degree, double> (dof, constraints);
}
