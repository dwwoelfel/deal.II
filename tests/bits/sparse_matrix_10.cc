//----------------------------  sparse_matrix_10.cc  ---------------------------
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
//----------------------------  sparse_matrix_10.cc  ---------------------------


// check SparseMatrix::operator /=

#include "../tests.h"
#include <lac/sparse_matrix.h>    
#include <fstream>


void test ()
{
  SparsityPattern sp (5,5,3);
  for (unsigned int i=0; i<5; ++i)
    for (unsigned int j=0; j<5; ++j)
      if ((i+2*j+1) % 3 == 0)
        sp.add (i,j);
  sp.compress ();

  SparseMatrix<double> m(sp);
  
                                   // first set a few entries
  for (unsigned int i=0; i<m.m(); ++i)
    for (unsigned int j=0; j<m.m(); ++j)
      if ((i+2*j+1) % 3 == 0)
        m.set (i,j, i*j*.5+.5);

                                   // then divide everything by 4/3 and
                                   // make sure we retrieve the values we
                                   // expect
  m /= 4./3.;
  
  for (unsigned int i=0; i<m.m(); ++i)
    for (unsigned int j=0; j<m.m(); ++j)
      if ((i+2*j+1) % 3 == 0)
        {
          Assert (m(i,j) == (i*j*.5+.5)/4*3, ExcInternalError());
          Assert (m.el(i,j) == (i*j*.5+.5)/4*3, ExcInternalError());
        }
      else
        {
          Assert (m.el(i,j) == 0, ExcInternalError());
        }

  deallog << "OK" << std::endl;
}



int main ()
{
  std::ofstream logfile("sparse_matrix_10/output");
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  try
    {
      test ();
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
