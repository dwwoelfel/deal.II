//------------------------  grid_in_tecplot.cc  -----------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2002, 2003, 2004, 2005, 2007 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//------------------------  grid_in_tecplot.cc  -----------------------

#include "../tests.h"
#include <dofs/dof_handler.h>
#include <grid/tria.h>
#include <grid/grid_out.h>
#include <grid/grid_in.h>
#include <base/logstream.h>

#include <fstream>
#include <string>

std::ofstream logfile("grid_in_tecplot/output");


template <int dim>
void test (const std::string &infilename)
{  
  Triangulation<dim> tria;
  GridIn<dim> gi;
  gi.attach_triangulation (tria);
  gi.read (infilename);

  logfile<<"------------------------------------------"<<std::endl
	 <<"output for grid in "<<infilename<<std::endl;
  
  GridOut grid_out;
  grid_out.set_flags (GridOutFlags::Ucd(true));
  grid_out.write_ucd (tria, logfile);
}

int main ()
{
  test<2> ("grid_tec_1.dat");
  test<2> ("grid_tec_2.dat");
  test<2> ("grid_tec_3.dat");
  test<2> ("grid_tec_4.dat");
}

