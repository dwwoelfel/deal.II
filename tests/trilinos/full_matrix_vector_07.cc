//----------------------------  trilinos_full_matrix_vector_07.cc  ---------------------------
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
//----------------------------  trilinos_full_matrix_vector_07.cc  ---------------------------


// check FullMatrix::matrix_norm_square

#include "../tests.h"
#include <lac/trilinos_vector.h>
#include <lac/trilinos_full_matrix.h>
#include <fstream>
#include <iostream>
#include <vector>


void test (TrilinosWrappers::Vector &v,
           TrilinosWrappers::Vector &w,
           TrilinosWrappers::Vector &x)
{
  TrilinosWrappers::FullMatrix m(v.size(),v.size());
  for (unsigned int i=0; i<m.m(); ++i)
    for (unsigned int j=0; j<m.m(); ++j)
        m.set (i,j, i+2*j);

  for (unsigned int i=0; i<v.size(); ++i)
    {
      v(i) = i;
      w(i) = i+1;
    }
      
  m.compress ();
  v.compress ();
  w.compress ();

                                   // x=w-Mv
  const double s = m.residual (x, v, w);

                                   // make sure we get the expected result
  for (unsigned int i=0; i<v.size(); ++i)
    {
      Assert (v(i) == i, ExcInternalError());
      Assert (w(i) == i+1, ExcInternalError());

      double result = i+1;
      for (unsigned int j=0; j<m.m(); ++j)
        result -= (i+2*j)*j;

      Assert (x(i) == result, ExcInternalError());
    }

  Assert (s == x.l2_norm(), ExcInternalError());

  deallog << "OK" << std::endl;
}



int main (int argc, char **argv)
{
  std::ofstream logfile("full_matrix_vector_07/output");
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  try
    {
      {
        TrilinosWrappers::Vector v (100);
        TrilinosWrappers::Vector w (100);
        TrilinosWrappers::Vector x (100);
        test (v,w,x);
      }
    }
  catch (std::exception &exc)
    {
      std::cerr << std::endl << std::endl
		<< "----------------------------------------------------"
		<< std::endl;
      std::cerr << "Exception on processing: " << std::endl
		<< exc.what() << std::endl
		<< "Aborting!" << std::endl
		<< "----------------------------------------------------"
		<< std::endl;
      
      return 1;
    }
  catch (...) 
    {
      std::cerr << std::endl << std::endl
		<< "----------------------------------------------------"
		<< std::endl;
      std::cerr << "Unknown exception!" << std::endl
		<< "Aborting!" << std::endl
		<< "----------------------------------------------------"
		<< std::endl;
      return 1;
    };
}
