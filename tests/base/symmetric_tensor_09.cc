// ---------------------------------------------------------------------
// $Id$
//
// Copyright (C) 2005 - 2013 by the deal.II authors
//
// This file is part of the deal.II library.
//
// The deal.II library is free software; you can use it, redistribute
// it, and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full text of the license can be found in the file LICENSE at
// the top level of the deal.II distribution.
//
// ---------------------------------------------------------------------


// test the determinant code

#include "../tests.h"
#include <deal.II/base/symmetric_tensor.h>
#include <deal.II/base/logstream.h>
#include <fstream>
#include <iomanip>


template <int dim>
void test ()
{
  SymmetricTensor<2,dim> t;
  for (unsigned int i=0; i<dim; ++i)
    for (unsigned int j=i; j<dim; ++j)
      t[i][j] = (1.+(i+1)*(j*2));

  for (unsigned int i=0; i<dim; ++i)
    for (unsigned int j=0; j<dim; ++j)
      deallog << i << ' ' << j << ' ' << t[i][j] << std::endl;

  deallog << determinant(t) << std::endl;
}




int main ()
{
  std::ofstream logfile("output");
  deallog << std::setprecision(3);
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  test<2> ();
  test<3> ();

  deallog << "OK" << std::endl;
}
