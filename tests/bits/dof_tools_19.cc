//----------------------------  dof_tools_19.cc  ---------------------------
//    $Id: dof_tools_03.cc 11749 2005-11-09 19:11:20Z wolf $
//    Version: $Name$ 
//
//    Copyright (C) 2003, 2004, 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  dof_tools_19.cc  ---------------------------

#include "../tests.h"
#include "dof_tools_common.cc"
#include <dofs/dof_constraints.h>
#include <base/function.h>
#include <base/quadrature_lib.h>
#include <numerics/vectors.h>
#include <lac/vector.h>
#include <dofs/dof_tools.h>

// check
//   DoFTools::
//   make_hanging_node_constraints (const DoFHandler<dim> &,
//	                            ConstraintMatrix      &);
//
// As the results of dof_tools_03 seem unclear, this test aims
// at verifying the constraints by projecting a constant function
// onto the FE space and afterwards evaluating the L_2 error.
// This should reveal errors in the constraint matrix.

// as a side effect, this test used to crash because a)
// VectorTools::project instantiated QGauss<0>(6) when in 1d, and b)
// because VectorTools::project wasn't implemented at all in 1d. It
// required fixing both bugs to get to the actual point of this test.

std::string output_file_name = "dof_tools_19/output";


template <int dim>
void
check_this (const DoFHandler<dim> &dof_handler)
{
                                   // there's presently a crash in the
                                   // Raviart-Thomas element. don't
                                   // check this element for that
                                   // reason. this case is covered in
                                   // rt_crash_01, however
  if (dof_handler.get_fe().get_name().find ("RaviartThomas") !=
      std::string::npos)
    return;
  
  ConstantFunction<dim> test_func (1, dof_handler.get_fe().n_components ()); 

                                   // don't run this test if hanging
                                   // nodes are not implemented
  if (dof_handler.get_fe().constraints_are_implemented() == false)
    return;

  ConstraintMatrix cm;
  DoFTools::make_hanging_node_constraints (dof_handler, cm);
  cm.close ();

  deallog << cm.n_constraints () << std::endl;
  deallog << cm.max_constraint_indirections () << std::endl;

  // L_2 project constant function onto field
  QGauss<dim> quadrature (6);
  Vector<double> solution (dof_handler.n_dofs ());

  VectorTools::project (dof_handler, cm,
			quadrature, test_func,
			solution);
  cm.distribute (solution);

  // Evaluate error
  Vector<double> cellwise_errors (dof_handler.get_tria ().n_active_cells());
  VectorTools::integrate_difference (dof_handler, solution, test_func,
                                     cellwise_errors, quadrature,
                                     VectorTools::L2_norm);
  const double p_l2_error = cellwise_errors.l2_norm();

  Assert (p_l2_error < 1e-11, ExcInternalError());
  
  deallog << "L2_Error : " << p_l2_error << std::endl;
}
