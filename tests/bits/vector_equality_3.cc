//----------------------------  vector_equality_3.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2004 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  vector_equality_3.cc  ---------------------------


// check Vector<double>::operator!=(Vector<double>) for vectors that are not
// equal

#include "../tests.h"
#include <lac/vector.h>    
#include <fstream>
#include <iostream>
#include <vector>


void test (Vector<double> &v,
           Vector<double> &w)
{
                                   // set only certain elements of each
                                   // vector
  for (unsigned int i=0; i<v.size(); ++i)
    {
      v(i) = i;
      if (i%3 == 0)
        w(i) = i+1.;
    }

  Assert (v.operator != (w), ExcInternalError());

  deallog << "OK" << std::endl;
}



int main ()
{
  std::ofstream logfile("vector_equality_3.output");
  deallog.attach(logfile);
  deallog.depth_console(0);

  try
    {
      Vector<double> v (100);
      Vector<double> w (100);
      test (v,w);
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
