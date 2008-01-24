//----------------------------  complex_vector_18.cc  ---------------------------
//    complex_vector_18.cc,v 1.4 2004/02/26 17:25:45 wolf Exp
//    Version:  
//
//    Copyright (C) 2004, 2005, 2007 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  complex_vector_18.cc  ---------------------------


// check Vector<std::complex<double> >::l2_norm()

#include "../tests.h"
#include <lac/vector.h>    
#include <fstream>
#include <iomanip>
#include <vector>


void test (Vector<std::complex<double> > &v)
{
                                   // set some elements of the vector
  double norm = 0;
  for (unsigned int i=0; i<v.size(); i+=1+i)
    {
      v(i) = std::complex<double> (i+1., i+2.);
      norm += std::abs(std::complex<double> (i+1., i+2.))*
	      std::abs(std::complex<double> (i+1., i+2.));
    }
  v.compress ();

                                   // then check the norm
  Assert (std::fabs((v.l2_norm() - std::sqrt(norm))/std::sqrt(norm)) < 1e-14, ExcInternalError());

  deallog << "OK" << std::endl;
}



int main () 
{
  std::ofstream logfile("complex_vector_18/output");
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
