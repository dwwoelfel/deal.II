//----------------------------  trilinos_59.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2004, 2005, 2008 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  trilinos_59.cc  ---------------------------


// check TrilinosWrappers::Vector (const ::Vector &) copy constructor

#include "../tests.h" 
#include <base/utilities.h>
#include <lac/trilinos_vector.h>
#include <lac/vector.h>

#include <fstream>
#include <iostream>
#include <vector>


void test (TrilinosWrappers::Vector &v)
{
                                   // set only certain elements of the
                                   // vector.
  std::vector<bool> pattern (v.size(), false);
  for (unsigned int i=0; i<v.size(); i+=1+i)
    {
      v(i) += i;
      pattern[i] = true;
    }

  v.compress ();

  Vector<double> w (v);
  Vector<float>  x (v);

  TrilinosWrappers::Vector w1 (w);
  TrilinosWrappers::Vector x1 (x);
  
  for (unsigned int i=0; i<v.size(); ++i)
    {
      Assert (w1(i) == w(i), ExcInternalError());
      Assert (x1(i) == x(i), ExcInternalError());
    }
      
  deallog << "OK" << std::endl;
}



int main (int argc,char **argv) 
{
  std::ofstream logfile("59/output");
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10); 

  Utilities::System::MPI_InitFinalize mpi_initialization (argc, argv);


  try
    {
      {
        TrilinosWrappers::Vector v (100);
        test (v);
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
