//----------------------------  union_triangulation.cc  ---------------------------
//    $Id: union_triangulation.cc 11749 2005-11-09 19:11:20Z wolf $
//    Version: $Name$ 
//
//    Copyright (C) 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  union_triangulation.cc  ---------------------------


#include "../tests.h"
#include <grid/tria_boundary.h>
#include <grid/tria_accessor.h>
#include <grid/tria_iterator.h>
#include <grid/tria.h>
#include <grid/grid_generator.h>
#include <grid/grid_tools.h>
#include <grid/grid_out.h>
#include <base/logstream.h>
#include <cmath>
#include <cstdlib>

#include <fstream>
#include <cstdio>

std::ofstream logfile("union_triangulation/output");



template <int dim>
void test ()
{
  Triangulation<dim> tria_1, tria_2, tria_3;
  GridGenerator::hyper_cube(tria_1);
  GridGenerator::hyper_cube(tria_2);

				   // fill tria_3 with something, to
				   // make sure that the function we
				   // call later can deal with prior
				   // content
  GridGenerator::hyper_cube(tria_3);

				   // refine once, then refine first
				   // cell
  tria_1.refine_global (1);
  tria_1.begin_active()->set_refine_flag();
  tria_1.execute_coarsening_and_refinement ();

				   // similar for second grid, but
				   // different cell
  tria_2.refine_global (1);
  (++tria_2.begin_active())->set_refine_flag();
  tria_2.execute_coarsening_and_refinement ();

  GridTools::create_union_triangulation (tria_1, tria_2, tria_3);

  GridOut().write_gnuplot (tria_3, logfile);

  deallog << "     Total number of cells        = " << tria_3.n_cells() << std::endl
	  << "     Total number of active cells = " << tria_3.n_active_cells() << std::endl;
}


int main ()
{
  logfile.precision(2);
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);
  
  test<1> ();
  test<2> ();
  test<3> ();
  
  return 0;
}
