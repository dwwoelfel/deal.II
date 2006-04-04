//----------------------------  data_out_stack_01.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  data_out_stack_04.cc  ---------------------------

// same as data_out_stack_04, but test for 2d

#include "../tests.h"
#include <grid/tria.h>
#include <dofs/dof_handler.h>
#include <grid/grid_generator.h>
#include <fe/fe_q.h>
#include <lac/vector.h>

#include <numerics/data_out_stack.h>
#include <fstream>
#include <iostream>

#include <base/logstream.h>




template <int dim>
void run () 
{
  Triangulation<dim> triangulation;
  GridGenerator::hyper_cube (triangulation, -1, 1);
  triangulation.refine_global (1);
  
  FE_Q<dim> fe(1);
  DoFHandler<dim> dof_handler(triangulation);
  dof_handler.distribute_dofs (fe);

				   // create a continuous field over
				   // this DoFHandler
  Vector<double> v(dof_handler.n_dofs());
  v(v.size()/2) = 1;

				   // output this field using
				   // DataOutStack. the result should
				   // be a continuous field again
  DataOutStack<dim> data_out_stack;
  data_out_stack.declare_data_vector ("solution",
				      DataOutStack<dim>::dof_vector);
  data_out_stack.new_parameter_value (1,1);
  data_out_stack.attach_dof_handler (dof_handler);
  data_out_stack.add_data_vector (v, "solution");
  data_out_stack.build_patches (1);
  data_out_stack.finish_parameter_value ();

  data_out_stack.write_gnuplot (deallog.get_file_stream());
}



int main () 
{
  std::ofstream logfile("data_out_stack_04/output");
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  run<2> ();
  
  return 0;
}
