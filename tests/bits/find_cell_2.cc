//----------------------------  find_cell_2.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2003, 2004, 2005 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  find_cell_2.cc  ---------------------------


// same as find_cell_2_1, but in 3d

#include "../tests.h"
#include <deal.II/base/logstream.h>
#include <deal.II/grid/tria.h>
#include <deal.II/grid/tria_accessor.h>
#include <deal.II/grid/tria_iterator.h>
#include <deal.II/grid/grid_tools.h>
#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/tria_boundary_lib.h>

#include <fstream>




void check (Triangulation<3> &tria)
{
  Point<3> p(1./3.,1./2.,1./5.);
  
  Triangulation<3>::active_cell_iterator cell
    = GridTools::find_active_cell_around_point (tria, p);

  deallog << cell << std::endl;
  for (unsigned int v=0; v<GeometryInfo<3>::vertices_per_cell; ++v)
    deallog << "<" << cell->vertex(v) << "> ";
  deallog << std::endl;

  Assert (p.distance (cell->center()) < cell->diameter()/2,
	  ExcInternalError());
}


int main () 
{
  std::ofstream logfile("find_cell_2/output");
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  {  
    Triangulation<3> coarse_grid;
    GridGenerator::hyper_cube (coarse_grid);
    coarse_grid.refine_global (2);
    check (coarse_grid);
  }
  
  {  
    Triangulation<3> coarse_grid;
    GridGenerator::hyper_ball (coarse_grid);
    static const HyperBallBoundary<3> boundary;
    coarse_grid.set_boundary (0, boundary);
    coarse_grid.refine_global (2);
    check (coarse_grid);
  }
}

  
  
