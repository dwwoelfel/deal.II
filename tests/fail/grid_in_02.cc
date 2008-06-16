//----------------------------  grid_in_02.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2002, 2003, 2004, 2005, 2007, 2008 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  grid_in_02.cc  ---------------------------

// when reading a particular input file with a mesh, it turns out that
// there's a cell where we list the same cell as neighbor through two
// different faces. this of course can't be

#include "../tests.h"
#include <dofs/dof_handler.h>
#include <grid/tria.h>
#include <grid/tria_boundary.h>
#include <grid/tria_boundary_lib.h>
#include <grid/tria_accessor.h>
#include <grid/tria_iterator.h>
#include <grid/grid_out.h>
#include <grid/grid_in.h>
#include <grid/grid_generator.h>
#include <base/logstream.h>

#include <fstream>
#include <iomanip>
#include <string>

std::ofstream logfile("grid_in_02/output");


template <int dim>
void test2 ()
{
                                   // read a much larger grid (30k
                                   // cells). with the old grid
                                   // reordering scheme, this took >90
                                   // minutes (exact timing not
                                   // available, program was killed
                                   // before), with the new one it
                                   // takes less than 8 seconds
  Triangulation<dim> tria;
  GridIn<dim> gi;
  gi.attach_triangulation (tria);
  std::ifstream in ("grid_in_02/2d.xda");
  gi.read_xda (in);

  Triangulation<2>::active_cell_iterator
    cell = tria.begin_active(),
    endc = tria.end();
  for (; cell != endc; ++cell)
    for (unsigned int f=0; f<GeometryInfo<2>::faces_per_cell; ++f)
      for (unsigned int e=0; e<GeometryInfo<2>::faces_per_cell; ++e)
	if (f != e)
	  if (!cell->at_boundary(e) && !cell->at_boundary(f))
	    Assert (cell->neighbor(e) !=
		    cell->neighbor(f),
		    ExcInternalError());
}




int main ()
{
  deallog << std::setprecision (2);
  logfile << std::setprecision (2);
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  test2<2> ();
}

