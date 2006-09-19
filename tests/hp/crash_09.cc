//----------------------------  crash_09.cc  ---------------------------
//    $Id: crash_09.cc 12732 2006-03-28 23:15:45Z wolf $
//    Version: $Name$ 
//
//    Copyright (C) 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  crash_09.cc  ---------------------------


// a test where a degree of freedom was constrained multiple times,
// but with different weights. see the hp paper for more on this

char logname[] = "crash_09/output";


#include "../tests.h"
#include <base/function.h>
#include <base/logstream.h>
#include <base/quadrature_lib.h>
#include <lac/vector.h>

#include <grid/tria.h>
#include <dofs/hp_dof_handler.h>
#include <dofs/dof_constraints.h>
#include <grid/grid_generator.h>
#include <grid/grid_refinement.h>
#include <grid/tria_accessor.h>
#include <grid/tria_iterator.h>
#include <grid/tria_boundary_lib.h>
#include <dofs/dof_accessor.h>
#include <dofs/dof_tools.h>
#include <numerics/vectors.h>
#include <fe/fe_q.h>

#include <fstream>
#include <vector>



int main ()
{
  std::ofstream logfile(logname);
  logfile.precision (3);
  
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);


				   // create a mesh like this:
				   //
				   // *---*---*---*
				   // | 6 | 7 | 8 |
				   // *---*---*---*
				   // | 3 | 4 | 5 |
				   // *---*---*---*
				   // | 0 | 1 | 2 |
				   // *---*---*---*
  Triangulation<3>     triangulation;
  std::vector<unsigned int> subdivisions (3, 2);
  subdivisions[2] = 1;
  GridGenerator::subdivided_hyper_rectangle (triangulation, subdivisions,
                                             Point<3>(), Point<3>(2,2,1));

  hp::FECollection<3> fe;
  fe.push_back (FE_Q<3>(1));
  fe.push_back (FE_Q<3>(2));
  fe.push_back (FE_Q<3>(3));

  hp::DoFHandler<3>        dof_handler(triangulation);

  hp::DoFHandler<3>::active_cell_iterator
    cell = dof_handler.begin_active();
  cell->set_active_fe_index (0);
  ++cell;
  cell->set_active_fe_index (1);
  ++cell;
  cell->set_active_fe_index (0);
  ++cell;
  cell->set_active_fe_index (2);

  dof_handler.distribute_dofs (fe);
  
  ConstraintMatrix constraints;
  DoFTools::make_hanging_node_constraints (dof_handler,
					   constraints);
  constraints.close ();

  constraints.print (deallog.get_file_stream());
}

