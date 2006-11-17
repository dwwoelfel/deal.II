//---------------------------------------------------------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2005, 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------


// Test GridGenerator::subdivided_hyper_rectangle with vector of step
// sizes.

#include "../tests.h"
#include <base/logstream.h>
#include <base/tensor.h>
#include <grid/tria.h>
#include <grid/grid_generator.h>
#include <grid/grid_out.h>

#include <fstream>
#include <iostream>


template<int dim>
void test(std::ostream& out)
{
  Point<dim> p1;
  p1[0] = 2.;
  if (dim>1) p1[1] = -1.;
  if (dim>2) p1[2] = 0.;
  Point<dim> p2;
  p2[0] = 3.;
  if (dim>1) p2[1] = 2.;
  if (dim>2) p2[2] = 4.;
  Point<dim> p3;
  p3[0] = 2.;
  if (dim>1) p3[1] = 1.;
  if (dim>2) p3[2] = 4.;
  
  GridOut go;

                                   // uniformly subdivided mesh
  if (true)
    {
      deallog << "subdivided_hyper_rectangle" << std::endl;
      Triangulation<dim> tr;
      std::vector<std::vector<double> > sub(dim);
      for (unsigned int i=0; i<dim; ++i)
        sub[i] = std::vector<double> (i+2, (p2[i]-p1[i])/(i+2));

      GridGenerator::subdivided_hyper_rectangle(tr, sub, p1, p2, true);
      if (tr.n_cells() > 0)
	go.write_gnuplot(tr, out);
    }


                                   // non-uniformly subdivided mesh
  if (true)
    {
      deallog << "subdivided_hyper_rectangle" << std::endl;
      Triangulation<dim> tr;
      std::vector<std::vector<double> > sub(dim);
      for (unsigned int i=0; i<dim; ++i)
        {
          sub[i] = std::vector<double> (i+2, (p2[i]-p1[i])/(i+2));
          sub[i][0] /= 2;
          sub[i].back() *= 1.5;
        }

      GridGenerator::subdivided_hyper_rectangle(tr, sub, p1, p2, true);
      if (tr.n_cells() > 0)
	go.write_gnuplot(tr, out);
    }
}


int main()
{
  std::ofstream logfile("grid_generator_02/output");
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);
  
  deallog.push("1d");
  test<1>(logfile);
  deallog.pop();
  deallog.push("2d");
  test<2>(logfile);
  deallog.pop();
  deallog.push("3d");
  test<3>(logfile);
  deallog.pop();
}
