//----------------------------  full_tensor_04.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2005 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  full_tensor_04.cc  ---------------------------

// test full 3x3x3x3 tensors

#include "../tests.h"
#include <deal.II/base/tensor.h>
#include <deal.II/base/logstream.h>
#include <fstream>
#include <iomanip>

int main ()
{
  std::ofstream logfile("full_tensor_04/output");
  deallog << std::setprecision(3);
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);
  
  Tensor<4,3> t;
  t[0][0][0][0] = 1;
  t[1][1][1][1] = 2;
  t[0][1][0][1] = 3;
  t[1][0][1][0] = 3;

  Assert (t[0][1][0][1] == t[1][0][1][0], ExcInternalError());

                                   // check norm of tensor
  deallog << t.norm() << std::endl;

                                   // make sure norm is induced by scalar
                                   // product
  double norm_sqr = 0;
  for (unsigned int i=0; i<2; ++i)
    for (unsigned int j=0; j<2; ++j)
      for (unsigned int k=0; k<2; ++k)
	for (unsigned int l=0; l<2; ++l)
	  norm_sqr += t[i][j][k][l] * t[i][j][k][l];
  Assert (std::fabs (t.norm()*t.norm() - norm_sqr) < 1e-14,
          ExcInternalError());

  deallog << "OK" << std::endl;
}
