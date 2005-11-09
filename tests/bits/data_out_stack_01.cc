//----------------------------  data_out_stack_01.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2003, 2004 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  data_out_stack_01.cc  ---------------------------

#include "../tests.h"
#include "data_out_common.cc"
#include <lac/sparsity_pattern.h>
#include <numerics/data_out_stack.h>


std::string output_file_name = "data_out_stack_01/output";


template <int dim>
void
check_this (const DoFHandler<dim> &dof_handler,
            const Vector<double>  &v_node,
            const Vector<double>  &v_cell)
{
                                   // 3d would generate 4d data, which
                                   // we don't presently support
                                   //
                                   // output for 2d+time is not
                                   // presently implemented
  if (dim > 1)
    return;
  
  DataOutStack<dim> data_out_stack;
  data_out_stack.declare_data_vector ("node_data",
                                      DataOutStack<dim>::dof_vector);
  data_out_stack.declare_data_vector ("cell_data",
                                      DataOutStack<dim>::cell_vector);
  
  
  data_out_stack.new_parameter_value (1., 1.);
  data_out_stack.attach_dof_handler (dof_handler);
  data_out_stack.add_data_vector (v_node, "node_data");
  data_out_stack.add_data_vector (v_cell, "cell_data");
  data_out_stack.build_patches ();
  data_out_stack.finish_parameter_value ();

  Vector<double> vn1 (v_node.size());
  vn1 = v_node;
  vn1 *= 2.;
  Vector<double> vc1 (v_cell.size());
  vc1 = v_cell;
  vc1 *= 3.;
  data_out_stack.new_parameter_value (1., 1.);
  data_out_stack.attach_dof_handler (dof_handler);
  data_out_stack.add_data_vector (vn1, "node_data");
  data_out_stack.add_data_vector (vc1, "cell_data");
  data_out_stack.build_patches ();
  data_out_stack.finish_parameter_value ();
  
  data_out_stack.write_dx (deallog.get_file_stream());
  data_out_stack.write_ucd (deallog.get_file_stream());  
  data_out_stack.write_gmv (deallog.get_file_stream());
  data_out_stack.write_tecplot (deallog.get_file_stream());
  data_out_stack.write_vtk (deallog.get_file_stream());
  data_out_stack.write_gnuplot (deallog.get_file_stream());
  data_out_stack.write_deal_II_intermediate (deallog.get_file_stream());

                                   // the following is only
                                   // implemented for 2d (=1d+time)
  if (dim == 1)
    {
      data_out_stack.write_povray (deallog.get_file_stream());
      data_out_stack.write_eps (deallog.get_file_stream());
    }  
}


