//-----------------------------------------------------------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2006, 2007 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//-----------------------------------------------------------------------------

#include "../tests.h"
#include <base/data_out_base.h>
#include <base/logstream.h>
#include <base/quadrature_lib.h>
#include <base/function_lib.h>

#include <vector>
#include <iomanip>
#include <fstream>
#include <string>

#include "patches.h"

// Output data on repetitions of the unit hypercube

// define this as 1 to get output into a separate file for each testcase
#define SEPARATE_FILES 0


template <int dim, int spacedim>
void check(DataOutBase::PovrayFlags flags,
	   std::ostream& out)
{
  const unsigned int np = 4;
  
  std::vector<DataOutBase::Patch<dim, spacedim> > patches(np);
  
  create_patches(patches);
  
  std::vector<std::string> names(5);
  names[0] = "x1";
  names[1] = "x2";
  names[2] = "x3";
  names[3] = "x4";
  names[4] = "i";
  std::vector<boost::tuple<unsigned int, unsigned int, std::string> > vectors;
  DataOutBase::write_povray(patches, names, vectors, flags, out);
}


template <int dim>
void check_cont(unsigned int ncells,
		unsigned int nsub,
		DataOutBase::PovrayFlags flags,
		std::ostream& out)
{
  std::vector<DataOutBase::Patch<dim, dim> > patches;
  
  create_continuous_patches(patches, ncells, nsub);
  
  std::vector<std::string> names(1);
  names[0] = "CutOff";
  std::vector<boost::tuple<unsigned int, unsigned int, std::string> > vectors;
  DataOutBase::write_povray(patches, names, vectors, flags, out);
}


template<int dim, int spacedim>
void check_all(std::ostream& log)
{
#if SEPARATE_FILES == 0
  std::ostream& out = log;
#endif
  
  char name[100];
//  const char* format = "data_out_base_povray/%d%d%d%s.pov";
  DataOutBase::PovrayFlags flags;

  if (true)
    {
      sprintf(name, "data_out_base_povray/cont%d%d%d.pov", dim, 4, 4);
#if SEPARATE_FILES==1
      std::ofstream out(name);
#else
      out << "==============================\n"
	  << name
	  << "\n==============================\n";
#endif
      check_cont<dim>(4, 4, flags, out);
    }
  
  flags.external_data = true;
  if (true)
    {
      sprintf(name, "data_out_base_povray/cont%d%d%dtri.pov", dim, 4, 4);
#if SEPARATE_FILES==1
      std::ofstream out(name);
#else
      out << "==============================\n"
	  << name
	  << "\n==============================\n";
#endif
      check_cont<dim>(4, 4, flags, out);
    }

  flags.smooth = true;
  if (true)
    {
      sprintf(name, "data_out_base_povray/cont%d%d%dsmooth.pov", dim, 4, 4);
#if SEPARATE_FILES==1
      std::ofstream out(name);
#else
      out << "==============================\n"
	  << name
	  << "\n==============================\n";
#endif
      check_cont<dim>(4, 4, flags, out);
    }
  
  flags.bicubic_patch = true;
  if (true)
    {
      sprintf(name, "data_out_base_povray/cont%d%d%dbic.pov", dim, 4, 3);
#if SEPARATE_FILES==1
      std::ofstream out(name);
#else
      out << "==============================\n"
	  << name
	  << "\n==============================\n";
#endif
      check_cont<dim>(4, 3, flags, out);
    }
}

int main()
{
  std::ofstream logfile("data_out_base_povray/output");
//  check_all<1,1>(logfile);
//  check_all<1,2>(logfile);
  check_all<2,2>(logfile);
  check_all<2,3>(logfile);
//  check_all<3,3>(logfile);  
}
