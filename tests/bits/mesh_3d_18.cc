//----------------------------  mesh_3d_18.cc  ---------------------------
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
//----------------------------  mesh_3d_18.cc  ---------------------------


// adapted from hp/crash_06, which itself is from
// make_hanging_node_constraints for hp elements. used to crash. triggers the
// crash that at the time of writing the test afflicts all
// hp/hp_constraints_*_03 tests

char logname[] = "mesh_3d_18/output";


#include "../tests.h"
#include <base/function.h>
#include <base/logstream.h>

#include <grid/tria.h>
#include <grid/grid_generator.h>
#include <grid/grid_refinement.h>
#include <grid/tria_accessor.h>
#include <grid/tria_iterator.h>

#include <fstream>
#include <vector>




void test_with_wrong_face_orientation ()
{
  Triangulation<3>     triangulation;
  GridGenerator::hyper_ball (triangulation);
  triangulation.begin_active()->set_refine_flag ();
  triangulation.execute_coarsening_and_refinement ();

  Triangulation<3>::active_cell_iterator cell = triangulation.begin_active();
  ++cell;
  ++cell;

  deallog << "cell=" << cell << std::endl;
  deallog << "cell->neighbor(3)=" << cell->neighbor(3) << std::endl;
  deallog << "cell->face_orientation(3)="
	  << (cell->face_orientation(3) ? "true" : "false")
	  << std::endl;
  
  const Triangulation<3>::active_cell_iterator neighbor_child
    = cell->neighbor_child_on_subface (3, 1);

  deallog << "cell->neighbor_child_on_subface(3,1)=" << neighbor_child << std::endl;
  deallog << "cell->neighbor_child_on_subface(3,1)->neighbor(5)="
	  << neighbor_child->neighbor(5) << std::endl;

  deallog << "cell->neighbor_child_on_subface(3,1)->face_orientation(5)="
	  << (neighbor_child->face_orientation(5) ? "true" : "false")
	  << std::endl;
  
  deallog << "cell->face(3)=" << cell->face(3) << std::endl;
  for (unsigned int i=0; i<4; ++i)
    deallog << "cell->face(3)->child(" << i << ")="
	    << cell->face(3)->child(i) << std::endl;
  
  for (unsigned int i=0; i<6; ++i)
    deallog << "cell->neighbor(3)->face(" << i << ")="
	    << cell->neighbor(3)->face(i) << std::endl;

  for (unsigned int i=0; i<6; ++i)
    deallog << "cell->neighbor_child_on_subface(3,1)->face(" << i << ")="
	    << cell->neighbor_child_on_subface(3,1)->face(i) << std::endl;

				   // The following assertion was originally
				   // in make_hanging_node_constraints with an
				   // equality sign and triggered for the mesh
				   // and cell here at hand. however, if one
				   // carefully reads the comment for
				   // CellAccessor::neighbor_child_on_subface,
				   // one realizes that it isn't true if
				   // cell->face_orientation()==false for the
				   // face we are presently considering. the
				   // original assertion therefore
				   // failed. let's make sure we get the
				   // status we expect.
  Assert (cell->face(3)->child(1) !=
	  neighbor_child->face(cell->neighbor_of_neighbor(3)),
	  ExcInternalError());
}



int main ()
{
  std::ofstream logfile(logname);
  logfile.precision (3);
  
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  test_with_wrong_face_orientation ();

  deallog << "OK" << std::endl;
}

