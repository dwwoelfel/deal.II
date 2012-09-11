//----------------------------  dof_tools_5.cc  ---------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 2003, 2004, 2010 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  dof_tools_5.cc  ---------------------------

#include "../tests.h"
#include "dof_tools_common.h"

// check
//   DoFTools::extract_dofs_with_support_on_boundary


std::string output_file_name = "dof_tools_20/output";


template <int dim>
void
check_this (const DoFHandler<dim> &dof_handler)
{
  std::vector<bool> component_select (dof_handler.get_fe().n_components(),
                                      true);
  std::vector<bool> boundary_dofs (dof_handler.n_dofs());

                                   // first with all components
  {
    DoFTools::extract_dofs_with_support_on_boundary (dof_handler,
                                     component_select,
                                     boundary_dofs);
    output_bool_vector (boundary_dofs);
  }

                                   // next with only every second
                                   // component
  for (unsigned int i=1; i<component_select.size(); i+=2)
    component_select[i] = false;
  {
    DoFTools::extract_dofs_with_support_on_boundary (dof_handler,
                                     component_select,
                                     boundary_dofs);
    output_bool_vector (boundary_dofs);
  }

                                   // third further restrict to
                                   // boundary indicator 0
  {
    std::set<types::boundary_id> boundary_ids;
    boundary_ids.insert (0);
    DoFTools::extract_dofs_with_support_on_boundary (dof_handler,
                                     component_select,
                                     boundary_dofs,
                                     boundary_ids);
    output_bool_vector (boundary_dofs);
  }
}
