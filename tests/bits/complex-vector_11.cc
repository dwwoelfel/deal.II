//----------------------------  vector_11.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2004, 2005, 2007 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  vector_11.cc  ---------------------------


// check Vector<std::complex<double> >::size()

#include "../tests.h"
#include <lac/vector.h>    
#include <fstream>
#include <iostream>


void test (Vector<std::complex<double> > &v)
{
                                   // set only certain elements of the vector
  for (unsigned int i=0; i<v.size(); i+=1+i)
    v(i) = std::complex<double> (i+1., i+2.);

  v.compress ();
  
  Assert (v.size() == 100, ExcInternalError());

  deallog << "OK" << std::endl;
}



int main () 
{
  std::ofstream logfile("complex-vector_11/output");
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  try
    {
      Vector<std::complex<double> > v (100);
      test (v);
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
