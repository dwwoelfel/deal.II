//----------------------------  maximal_cell_diameter.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2008 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  maximal_cell_diameter.cc  ---------------------------


#include "../tests.h"
#include <base/logstream.h>
#include <grid/tria.h>
#include <grid/grid_generator.h>
#include <grid/grid_tools.h>
#include <grid/grid_out.h>

#include <fstream>
#include <iomanip>


std::ofstream logfile("maximal_cell_diameter/output");



template <int dim>
void test1 ()
{
				   // test 1: hypercube
  if (true)
    {
      Triangulation<dim> tria;
      GridGenerator::hyper_cube(tria);

      for (unsigned int i=0; i<2; ++i)
	{
	  tria.refine_global(2);
	  deallog << dim << "d, "
		  << "max diameter: "
		  << GridTools::maximal_cell_diameter (tria)
		  << std::endl;
	  Assert (GridTools::maximal_cell_diameter (tria)
		  >=
		  GridTools::minimal_cell_diameter (tria),
		  ExcInternalError());
	};
    };

				   // test 2: hyperball
  if (dim >= 2)
    {
      Triangulation<dim> tria;
      GridGenerator::hyper_ball(tria, Point<dim>(), 1);

      for (unsigned int i=0; i<2; ++i)
	{
	  tria.refine_global(2);
	  deallog << dim << "d, "
		  << "max diameter: "
		  << GridTools::maximal_cell_diameter (tria)
		  << std::endl;
	  Assert (GridTools::maximal_cell_diameter (tria)
		  >=
		  GridTools::minimal_cell_diameter (tria),
		  ExcInternalError());
	};
    };
}


int main ()
{
  deallog << std::setprecision(4);
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  test1<1> ();
  test1<2> ();
  test1<3> ();

  return 0;
}

