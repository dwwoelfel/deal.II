//----------------------------  find_cell_4.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2003, 2004, 2005, 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  find_cell_4.cc  ---------------------------


// take a 3d mesh and check that we can find an arbitrary point's cell
// in it. this presently fails since the point sits right on the edge
// of the domain, but for different reasons that find_cell_5

#include "../tests.h"
#include <base/logstream.h>
#include <grid/tria.h>
#include <grid/tria_accessor.h>
#include <grid/tria_iterator.h>
#include <grid/grid_tools.h>
#include <grid/grid_generator.h>
#include <grid/tria_boundary_lib.h>

#include <fstream>

#include <fe/mapping_q1.h>

void check (Triangulation<3> &tria)
{
  Point<3> p (0.75,0,0);
  
  Triangulation<3>::active_cell_iterator cell
    = GridTools::find_active_cell_around_point (tria, p);

  deallog << cell << std::endl;
  for (unsigned int v=0; v<GeometryInfo<3>::vertices_per_cell; ++v)
    deallog << "<" << cell->vertex(v) << "> ";
  deallog << std::endl;

  // Transform back and forth
  Point<3> pp =
     StaticMappingQ1<3>::mapping.transform_unit_to_real_cell
     ( cell,
       GeometryInfo<3>::project_to_unit_cell
       (
         StaticMappingQ1<3>::mapping.transform_real_to_unit_cell
         ( cell,
           p
           )
         )
       );
                                                                       
  Assert (p.distance (pp) < 1e-15,  ExcInternalError());
}


int main () 
{
  std::ofstream logfile("find_cell_4/output");
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  try
    {
      Triangulation<3> coarse_grid;
      GridGenerator::hyper_cube (coarse_grid);
      coarse_grid.refine_global (3);
      check (coarse_grid);
    }
  catch (const std::exception &exc)
    {
				       // we shouldn't get here...
      deallog << "Caught an error..." << std::endl;
      deallog << exc.what() << std::endl;
    }
}

  
  
