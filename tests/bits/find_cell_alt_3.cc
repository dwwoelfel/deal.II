//----------------------------  find_cell_alt_3.cc  ------------------------
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
//----------------------------  find_cell_alt_3.cc  ------------------------


// like find_cell_2, but with the strange meshes from the mesh_3d_* tests

#include "../tests.h"
#include "mesh_3d.h"

#include <base/logstream.h>
#include <grid/tria.h>
#include <grid/tria_accessor.h>
#include <grid/tria_iterator.h>
#include <grid/grid_tools.h>
#include <grid/grid_generator.h>
#include <grid/tria_boundary_lib.h>

#include <fe/mapping_q.h>

#include <fstream>


void check (Triangulation<3> &tria)
{
   MappingQ<3> map(3);
   
  Point<3> p(1./3.,1./2.,-1./5.);
  
  std::pair<Triangulation<3>::active_cell_iterator, Point<3> > cell
    = GridTools::find_active_cell_around_point (map, tria, p);

  deallog << cell.first << std::endl;
  for (unsigned int v=0; v<GeometryInfo<3>::vertices_per_cell; ++v)
    deallog << "<" << cell.first->vertex(v) << "> ";
  deallog << "[ " << cell.second << "] ";
  deallog << std::endl;

  Assert (p.distance (cell.first->center()) < cell.first->diameter()/2,
	  ExcInternalError());
}


int main () 
{
  std::ofstream logfile("find_cell_alt_3/output");
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  {  
    Triangulation<3> coarse_grid;
    create_two_cubes (coarse_grid);
    coarse_grid.refine_global (1);
    check (coarse_grid);
  }
  
  {  
    Triangulation<3> coarse_grid;
    create_L_shape (coarse_grid);
    coarse_grid.refine_global (1);
    check (coarse_grid);
  }
  
  {  
    Triangulation<3> coarse_grid;
    GridGenerator::hyper_ball (coarse_grid);
    coarse_grid.refine_global (1);
    check (coarse_grid);
  }
  
}

  
  
