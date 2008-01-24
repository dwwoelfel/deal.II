//----------------------------  cylinder_shell_01.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2003, 2004, 2005, 2008 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  cylinder_shell_01.cc  ---------------------------

// test that the grid generated by GridGenerator::cylinder_shell<3> works as
// expected


#include "../tests.h"
#include <base/logstream.h>
#include <grid/tria.h>
#include <grid/tria_iterator.h>
#include <grid/tria_accessor.h>
#include <grid/grid_generator.h>
#include <fstream>
#include <iomanip>

    

int main () 
{
  std::ofstream logfile("cylinder_shell_01/output");
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);
  deallog << std::setprecision (2);

                                   // generate a hyperball in 3d
  Triangulation<3> tria;
  GridGenerator::cylinder_shell (tria, 1, .8, 1);

                                   // make sure that all cells have positive
                                   // volume
  for (Triangulation<3>::active_cell_iterator cell=tria.begin_active();
       cell!=tria.end(); ++cell)
    deallog << cell << ' ' << cell->measure () << std::endl;
}
