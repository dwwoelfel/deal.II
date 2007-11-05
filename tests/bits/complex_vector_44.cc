//----------------------------  complex_vector_44.cc  ---------------------------
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
//----------------------------  complex_vector_44.cc  ---------------------------


// check Vector<std::complex<double> >::sadd(s,s,V,s,V,s,V)

#include "../tests.h"
#include <lac/vector.h>    
#include <fstream>
#include <iostream>
#include <vector>


void test (Vector<std::complex<double> > &v,
           Vector<std::complex<double> > &w,
           Vector<std::complex<double> > &x,
           Vector<std::complex<double> > &y)
{
  for (unsigned int i=0; i<v.size(); ++i)
    {
      v(i) = i;
      w(i) = std::complex<double> (i+1., i+2.);
      x(i) = i+2.;
      y(i) = std::complex<double> (i+3., i+4.);
    }
  
  v.compress ();
  w.compress ();
  x.compress ();
  y.compress ();

  v.sadd (1.5, 2, w, 3, x, 4, y);

                                   // make sure we get the expected result
  for (unsigned int i=0; i<v.size(); ++i)
    {
      Assert (w(i) == std::complex<double> (i+1., i+2.),
	      ExcInternalError());
      Assert (x(i) == i+2., ExcInternalError());
      Assert (y(i) == std::complex<double> (i+3., i+4.),
	      ExcInternalError());
      Assert (v(i) ==
	      1.5*i+2.*std::complex<double> (i+1., i+2.)+
	      3.*(i+2.)+4.*std::complex<double> (i+3., i+4.),
	      ExcInternalError());
    }

  deallog << "OK" << std::endl;
}



int main () 
{
  std::ofstream logfile("complex_vector_44/output");
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  try
    {
      Vector<std::complex<double> > v (100);
      Vector<std::complex<double> > w (100);
      Vector<std::complex<double> > x (100);
      Vector<std::complex<double> > y (100);
      test (v,w,x,y);
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
