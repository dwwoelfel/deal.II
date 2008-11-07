//----------------------------  trilinos_56.cc  ---------------------------
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
//----------------------------  trilinos_56.cc  ---------------------------


// check TrilinosWrappers::Vector::operator() in set, and later in /= mode

#include "../tests.h" 
#include <base/utilities.h>
#include <lac/trilinos_vector.h>    
#include <fstream>
#include <iostream>
#include <vector>


void test (TrilinosWrappers::Vector &v)
{
                                   // set only certain elements of the
                                   // vector. have a bit pattern of where we
                                   // actually wrote elements to
  std::vector<bool> pattern (v.size(), false);
  for (unsigned int i=0; i<v.size(); i+=1+i)
    {
      v(i) = 2*i;
      pattern[i] = true;
    }
  for (unsigned int i=0; i<v.size(); i+=1+i)
    v(i) /= 2;

  v.compress ();

                                   // check that they are ok, and this time
                                   // all of them
  for (unsigned int i=0; i<v.size(); ++i)
    Assert ((((pattern[i] == true) && (v(i) == i))
             ||
             ((pattern[i] == false) && (v(i) == 0))),
             ExcInternalError());

  deallog << "OK" << std::endl;
}



int main (int argc,char **argv) 
{
  std::ofstream logfile("56/output");
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
