//----------------------------  number_cache.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2008 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  number_cache.cc  ---------------------------


// test that we correctly compute the number of lines, quads, hexes, etc and
// store them in the number cache of the Triangulation class. use the meshes
// from grid_in (2d) and grid_in_3d (3d) for this

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

std::ofstream logfile("number_cache/output");



template <int dim>
void output (const Triangulation<dim> &tria)
{
  
  deallog << "  " << tria.n_active_cells() << std::endl;
  deallog << "  " << tria.n_cells() << std::endl;
  deallog << "  " << tria.n_active_lines() << std::endl;
  deallog << "  " << tria.n_lines() << std::endl;
  deallog << "  " << tria.n_active_quads() << std::endl;
  deallog << "  " << tria.n_quads() << std::endl;
  deallog << "  " << tria.n_active_hexs() << std::endl;
  deallog << "  " << tria.n_hexs() << std::endl;

  for (unsigned int i=0; i<tria.n_levels(); ++i)
    {
      deallog << "  " << tria.n_active_cells(i) << std::endl;
      deallog << "  " << tria.n_cells(i) << std::endl;
      if (dim == 1)
	{
	  deallog << "  " << tria.n_active_lines(i) << std::endl;
	  deallog << "  " << tria.n_lines(i) << std::endl;
	}
      
      if (dim == 2)
	{
	  deallog << "  " << tria.n_active_quads(i) << std::endl;
	  deallog << "  " << tria.n_quads(i) << std::endl;
	}
      
      if (dim == 3)
	{
	  deallog << "  " << tria.n_active_hexs(i) << std::endl;
	  deallog << "  " << tria.n_hexs(i) << std::endl;
	}
    }
}


template <int dim>
void test (const char *filename)
{
  deallog << "Reading " << filename << std::endl;
  
  Triangulation<dim> tria;
  GridIn<dim> gi;
  gi.attach_triangulation (tria);
  std::ifstream in (filename);

  try
    {
      gi.read_xda (in);
    }
  catch (std::exception &exc)
    {
      deallog << "  caught exception:" << std::endl
	      << exc.what()
	      << std::endl;
      return;
    }

  output (tria);
  
				   // now refine a few cells and output again
  deallog << "  Refining..." << std::endl;
  typename Triangulation<dim>::active_cell_iterator
    cell = tria.begin_active();
  for (unsigned int i=0; i<=std::min (tria.n_active_cells() / 3,
				      10U); ++i, ++cell)
    cell->set_refine_flag ();
  tria.execute_coarsening_and_refinement ();

  output (tria);
}


int main ()
{
  deallog << std::setprecision (2);
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  test<2> ("grid_in/2d.xda");
  
  test<3> ("grid_in_3d/1.in");
  test<3> ("grid_in_3d/2.in");
  test<3> ("grid_in_3d/3.in");
  test<3> ("grid_in_3d/4.in");

  test<3> ("grid_in_3d/evil_0.in");
  test<3> ("grid_in_3d/evil_1.in");
  test<3> ("grid_in_3d/evil_2.in");
  test<3> ("grid_in_3d/evil_3.in");
  test<3> ("grid_in_3d/evil_4.in");
}

