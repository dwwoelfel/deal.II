//----------------------------  metis_01.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2004, 2005 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  metis_01.cc  ---------------------------


// check GridTools::partition_triangulation

#include "../tests.h"
#include <deal.II/grid/tria.h>
#include <deal.II/grid/tria_accessor.h>
#include <deal.II/grid/tria_iterator.h>
#include <deal.II/grid/grid_tools.h>
#include <deal.II/grid/grid_generator.h>
#include <fstream>
#include <iomanip>


template <int dim>
void test ()
{
  Triangulation<dim> triangulation;
  GridGenerator::hyper_cube (triangulation);
  triangulation.refine_global (5-dim);

                                   // subdivide into 5 subdomains
  deallog << "RECURSIVE" << std::endl;
  GridTools::partition_triangulation (5, triangulation);
  for (typename Triangulation<dim>::active_cell_iterator
         cell = triangulation.begin_active();
       cell != triangulation.end(); ++cell)
    deallog << cell << ' ' << cell->subdomain_id() << std::endl;

                                   // subdivide into 9 subdomains. note that
                                   // this uses the k-way subdivision (for
                                   // more than 8 subdomains) rather than the
                                   // recursive one
  deallog << "K-WAY" << std::endl;
  GridTools::partition_triangulation (9, triangulation);
  for (typename Triangulation<dim>::active_cell_iterator
         cell = triangulation.begin_active();
       cell != triangulation.end(); ++cell)
    deallog << cell << ' ' << cell->subdomain_id() << std::endl;
}



int main ()
{
  std::ofstream logfile("metis_01/output");
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  try
    {
      test<1> ();
      test<2> ();
      test<3> ();
    }
  catch (std::exception &exc)
    {
      deallog << std::endl << std::endl
		<< "----------------------------------------------------"
		<< std::endl;
      deallog << "Exception on processing: " << std::endl
		<< exc.what() << std::endl
		<< "Aborting!" << std::endl
		<< "----------------------------------------------------"
		<< std::endl;
      
      return 1;
    }
  catch (...) 
    {
      deallog << std::endl << std::endl
		<< "----------------------------------------------------"
		<< std::endl;
      deallog << "Unknown exception!" << std::endl
		<< "Aborting!" << std::endl
		<< "----------------------------------------------------"
		<< std::endl;
      return 1;
    };
}
