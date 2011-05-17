//----------------------------  sparse_matrix_vector_07.cc  ---------------------------
//    sparse_matrix_vector_07.cc,v 1.5 2004/02/26 17:25:45 wolf Exp
//    Version:  
//
//    Copyright (C) 2004, 2005, 2008 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  sparse_matrix_vector_07.cc  ---------------------------


// check SparseMatrix::residual

#include "../tests.h"
#include <deal.II/lac/vector.h>
#include <deal.II/lac/sparse_matrix.h>
#include <fstream>
#include <iomanip>
#include <vector>


void test (Vector<double> &v,
           Vector<double> &w,
           Vector<double> &x)
{
                                   // set some entries in the
                                   // matrix. actually, set them all
  SparsityPattern sp (v.size(),v.size(),v.size());
  for (unsigned int i=0; i<v.size(); ++i)
    for (unsigned int j=0; j<v.size(); ++j)
      sp.add (i,j);
  sp.compress ();

                                   // then create a matrix from that
  SparseMatrix<double> m(sp);
  for (unsigned int i=0; i<m.m(); ++i)
    for (unsigned int j=0; j<m.n(); ++j)
        m.set (i,j, i+2*j);

  for (unsigned int i=0; i<v.size(); ++i)
    {
      v(i) = i;
      w(i) = i+1;
    }
      
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
      for (unsigned int j=0; j<m.n(); ++j)
        result -= (i+2*j)*j;

      Assert (x(i) == result, ExcInternalError());
    }

  Assert (std::fabs((s - x.l2_norm())/s) < 1e-14, ExcInternalError());

  deallog << "OK" << std::endl;
}



int main () 
{
  std::ofstream logfile("sparse_matrix_vector_07/output");
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  try
    {
      Vector<double> v (100);
      Vector<double> w (100);
      Vector<double> x (100);
      test (v,w,x);
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
