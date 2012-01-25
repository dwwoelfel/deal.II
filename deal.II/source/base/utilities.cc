//---------------------------------------------------------------------------
//      $Id$
//    Version: $Name$
//
//    Copyright (C) 2005, 2006, 2008, 2009, 2010, 2011 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------

#include <deal.II/base/utilities.h>
#include <deal.II/base/exceptions.h>

#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <cstddef>
#include <ctime>
#include <cerrno>
#include <cmath>
#include <unistd.h>
#include <sys/types.h>
#include <sstream>
#include <iostream>

#ifdef HAVE_STD_NUMERIC_LIMITS
#  include <limits>
#else
#  include <limits.h>
#endif


#ifdef DEAL_II_USE_TRILINOS
#  ifdef DEAL_II_COMPILER_SUPPORTS_MPI
#    include <Epetra_MpiComm.h>
#    include <deal.II/lac/vector_memory.h>
#    include <deal.II/lac/trilinos_vector.h>
#    include <deal.II/lac/trilinos_block_vector.h>
#  endif
#  include "Teuchos_RCP.hpp"
#  include "Epetra_SerialComm.h"
#endif



DEAL_II_NAMESPACE_OPEN


namespace Utilities
{


  DeclException2 (ExcInvalidNumber2StringConversersion,
		  unsigned int, unsigned int,
		  << "When trying to convert " << arg1
		  << " to a string with " << arg2 << " digits");
  DeclException1 (ExcInvalidNumber,
		  unsigned int,
		  << "Invalid number " << arg1);
  DeclException1 (ExcCantConvertString,
		  std::string,
		  << "Can't convert the string " << arg1
                  << " to the desired type");


  std::string
  int_to_string (const unsigned int i,
		 const unsigned int digits)
  {
				     // if second argument is invalid, then do
				     // not pad the resulting string at all
    if (digits == numbers::invalid_unsigned_int)
      return int_to_string (i, needed_digits(i));


    AssertThrow ( ! ((digits==1 && i>=10)   ||
		     (digits==2 && i>=100)  ||
		     (digits==3 && i>=1000) ||
		     (digits==4 && i>=10000)||
		     (digits==5 && i>=100000)||
		     (i>=1000000)),
		  ExcInvalidNumber2StringConversersion(i, digits));

    std::string s;
    switch (digits)
      {
	case 6:
	      s += '0' + i/100000;
	case 5:
	      s += '0' + (i%100000)/10000;
	case 4:
	      s += '0' + (i%10000)/1000;
	case 3:
	      s += '0' + (i%1000)/100;
	case 2:
	      s += '0' + (i%100)/10;
	case 1:
	      s += '0' + i%10;
	      break;
	default:
	      s += "invalid digits information";
      };
    return s;
  }



  unsigned int
  needed_digits (const unsigned int max_number)
  {
    if (max_number < 10)
      return 1;
    if (max_number < 100)
      return 2;
    if (max_number < 1000)
      return 3;
    if (max_number < 10000)
      return 4;
    if (max_number < 100000)
      return 5;
    if (max_number < 1000000)
      return 6;
    AssertThrow (false, ExcInvalidNumber(max_number));
    return 0;
  }



  int
  string_to_int (const std::string &s)
  {
    std::istringstream ss(s);

#ifdef HAVE_STD_NUMERIC_LIMITS
    static const int max_int = std::numeric_limits<int>::max();
#else
    static const int max_int = INT_MAX;
#endif

    int i = max_int;
    ss >> i;
                                     // check for errors
    AssertThrow (i != max_int, ExcCantConvertString (s));

//TODO: The test for errors above doesn't work, as can easily be
//verified. furthermore, it doesn't catch cases like when calling
//string_to_int("1.23.4") since it just reads in however much it can, without
//realizing that there is more

    return i;
  }



  std::vector<int>
  string_to_int (const std::vector<std::string> &s)
  {
    std::vector<int> tmp (s.size());
    for (unsigned int i=0; i<s.size(); ++i)
      tmp[i] = string_to_int (s[i]);
    return tmp;
  }



  double
  string_to_double (const std::string &s)
  {
    std::istringstream ss(s);

#ifdef HAVE_STD_NUMERIC_LIMITS
    static const double max_double = std::numeric_limits<double>::max();
#else
    static const double max_double = DBL_MAX;
#endif

    double i = max_double;
    ss >> i;

                                     // check for errors
    AssertThrow (i != max_double, ExcCantConvertString (s));

//TODO: The test for errors above doesn't work, as can easily be
//verified. furthermore, it doesn't catch cases like when calling
//string_to_int("1.23.4") since it just reads in however much it can, without
//realizing that there is more

    return i;
  }



  std::vector<double>
  string_to_double (const std::vector<std::string> &s)
  {
    std::vector<double> tmp (s.size());
    for (unsigned int i=0; i<s.size(); ++i)
      tmp[i] = string_to_double (s[i]);
    return tmp;
  }



  std::vector<std::string>
  split_string_list (const std::string &s,
                     const char         delimiter)
  {
    std::string tmp = s;
    std::vector<std::string> split_list;
    split_list.reserve (std::count (tmp.begin(), tmp.end(), delimiter)+1);

				     // split the input list
    while (tmp.length() != 0)
      {
        std::string name;
	name = tmp;

	if (name.find(delimiter) != std::string::npos)
	  {
	    name.erase (name.find(delimiter), std::string::npos);
	    tmp.erase (0, tmp.find(delimiter)+1);
	  }
	else
	  tmp = "";

	while ((name.length() != 0) &&
	       (name[0] == ' '))
	  name.erase (0,1);

	while (name[name.length()-1] == ' ')
	  name.erase (name.length()-1, 1);

	split_list.push_back (name);
      }

    return split_list;
  }



  std::vector<std::string>
  break_text_into_lines (const std::string &original_text,
                         const unsigned int width,
                         const char delimiter)
  {
    std::string              text = original_text;
    std::vector<std::string> lines;

                                     // remove trailing spaces
    while ((text.length() != 0) && (text[text.length()-1] == delimiter))
      text.erase(text.length()-1,1);

                                     // then split the text into lines
    while (text.length() != 0)
      {
                                         // in each iteration, first remove
                                         // leading spaces
        while ((text.length() != 0) && (text[0] == delimiter))
          text.erase(0, 1);

	std::size_t pos_newline = text.find_first_of("\n", 0);
	if (pos_newline != std::string::npos && pos_newline <= width)
	  {
	    std::string line (text, 0, pos_newline);
	    while ((line.length() != 0) && (line[line.length()-1] == delimiter))
	      line.erase(line.length()-1,1);
	    lines.push_back (line);
            text.erase (0, pos_newline+1);
	    continue;
	  }
	  
                                         // if we can fit everything into one
                                         // line, then do so. otherwise, we have
                                         // to keep breaking
        if (text.length() < width)
          {
					     // remove trailing spaces
	    while ((text.length() != 0) && (text[text.length()-1] == delimiter))
	      text.erase(text.length()-1,1);
            lines.push_back (text);
            text = "";
          }
        else
          {
                                             // starting at position width, find the
                                             // location of the previous space, so
                                             // that we can break around there
            int location = std::min<int>(width,text.length()-1);
            for (; location>0; --location)
              if (text[location] == delimiter)
                break;

                                             // if there are no spaces, then try if
                                             // there are spaces coming up
            if (location == 0)
              for (location = std::min<int>(width,text.length()-1);
                   location<static_cast<int>(text.length());
                   ++location)
                if (text[location] == delimiter)
                  break;

                                             // now take the text up to the found
                                             // location and put it into a single
                                             // line, and remove it from 'text'
	    std::string line (text, 0, location);
	    while ((line.length() != 0) && (line[line.length()-1] == delimiter))
	      line.erase(line.length()-1,1);
            lines.push_back (line);
            text.erase (0, location);
          }
      }

    return lines;
  }



  bool
  match_at_string_start (const std::string &name,
			 const std::string &pattern)
  {
    if (pattern.size() > name.size())
      return false;

    for (unsigned int i=0; i<pattern.size(); ++i)
      if (pattern[i] != name[i])
	return false;

    return true;
  }



  std::pair<int, unsigned int>
  get_integer_at_position (const std::string &name,
			   const unsigned int position)
  {
    Assert (position < name.size(), ExcInternalError());

    const std::string test_string (name.begin()+position,
				   name.end());

    std::istringstream str(test_string);

    int i;
    if (str >> i)
      {
					 // compute the number of
					 // digits of i. assuming it
					 // is less than 8 is likely
					 // ok
	if (i<10)
	  return std::make_pair (i, 1U);
	else if (i<100)
	  return std::make_pair (i, 2U);
	else if (i<1000)
	  return std::make_pair (i, 3U);
	else if (i<10000)
	  return std::make_pair (i, 4U);
	else if (i<100000)
	  return std::make_pair (i, 5U);
	else if (i<1000000)
	  return std::make_pair (i, 6U);
	else if (i<10000000)
	  return std::make_pair (i, 7U);
	else
	  {
	    Assert (false, ExcNotImplemented());
	    return std::make_pair (-1, numbers::invalid_unsigned_int);
	  }
      }
    else
      return std::make_pair (-1, numbers::invalid_unsigned_int);
  }



  double
  generate_normal_random_number (const double a,
				 const double sigma)
  {
				     // if no noise: return now
    if (sigma == 0)
      return a;

#ifdef HAVE_RAND_R
    static unsigned int seed = 0xabcd1234;
    const double y = 1.0*rand_r(&seed)/RAND_MAX;
#else
    const double y = 1.0*rand()/RAND_MAX;
#endif

				     // find x such that y=erf(x). do so
				     // using a Newton method to find
				     // the zero of F(x)=erf(x)-y. start
				     // at x=0
    double x = 0;
    unsigned int iteration = 0;
    while (true)
      {
	const double residual = 0.5+erf(x/std::sqrt(2.)/sigma)/2-y;
	if (std::fabs(residual) < 1e-7)
	  break;
	const double F_prime = 1./std::sqrt(2*3.1415926536)/sigma *
			       std::exp(-x*x/sigma/sigma/2);
	x += -residual / F_prime;

					 // make sure that we don't
					 // recurse endlessly
	++iteration;
	Assert (iteration < 20, ExcInternalError());
      };
    return x+a;
  }



  std::vector<unsigned int>
  reverse_permutation (const std::vector<unsigned int> &permutation)
  {
    const unsigned int n = permutation.size();

    std::vector<unsigned int> out (n);
    for (unsigned int i=0; i<n; ++i)
      out[i] = n - 1 - permutation[i];

    return out;
  }



  std::vector<unsigned int>
  invert_permutation (const std::vector<unsigned int> &permutation)
  {
    const unsigned int n = permutation.size();

    std::vector<unsigned int> out (n, numbers::invalid_unsigned_int);

    for (unsigned int i=0; i<n; ++i)
      {
	Assert (permutation[i] < n, ExcIndexRange (permutation[i], 0, n));
	out[permutation[i]] = i;
      }

				     // check that we have actually reached
				     // all indices
    for (unsigned int i=0; i<n; ++i)
      Assert (out[i] != numbers::invalid_unsigned_int,
	      ExcMessage ("The given input permutation had duplicate entries!"));

    return out;
  }



  namespace System
  {
#if defined(__linux__)

    double get_cpu_load ()
    {
      std::ifstream cpuinfo;
      cpuinfo.open("/proc/loadavg");

      AssertThrow(cpuinfo, ExcIO());

      double load;
      cpuinfo >> load;

      return load;
    }

#else

    double get_cpu_load ()
    {
      return 0.;
    }

#endif



    void get_memory_stats (MemoryStats & stats)
    {
      stats.VmPeak = stats.VmSize = stats.VmHWM = stats.VmRSS = 0;

				       // parsing /proc/self/stat would be a
				       // lot easier, but it does not contain
				       // VmHWM, so we use /status instead.
#if defined(__linux__)
      std::ifstream file("/proc/self/status");
      std::string line;
      std::string name;
      while (!file.eof())
	{
	  file >> name;
	  if (name == "VmPeak:")
	    file >> stats.VmPeak;
	  else if (name == "VmSize:")
	    file >> stats.VmSize;
	  else if (name == "VmHWM:")
	    file >> stats.VmHWM;
	  else if (name == "VmRSS:")
	    {
	      file >> stats.VmRSS;
	      break; //this is always the last entry
	    }

	  getline(file, line);
	}
#endif
    }



    std::string get_hostname ()
    {
      const unsigned int N=1024;
      char hostname[N];
      gethostname (&(hostname[0]), N-1);
      return hostname;
    }



    std::string get_time ()
    {
      std::time_t  time1= std::time (0);
      std::tm     *time = std::localtime(&time1);

      std::ostringstream o;
      o << time->tm_hour << ":"
        << (time->tm_min < 10 ? "0" : "") << time->tm_min << ":"
        << (time->tm_sec < 10 ? "0" : "") << time->tm_sec;

      return o.str();
    }


    bool job_supports_mpi ()
    {
      return program_uses_mpi();
    }


    bool
    program_uses_mpi ()
    {
#ifdef DEAL_II_COMPILER_SUPPORTS_MPI
      int MPI_has_been_started = 0;
      MPI_Initialized(&MPI_has_been_started);

      return true && (MPI_has_been_started > 0);
#else
      return false;
#endif
    }


    unsigned int get_n_mpi_processes (const MPI_Comm &mpi_communicator)
    {
      return MPI::n_mpi_processes (mpi_communicator);
    }

    unsigned int get_this_mpi_process (const MPI_Comm &mpi_communicator)
    {
      return MPI::this_mpi_process (mpi_communicator);
    }



    void calculate_collective_mpi_min_max_avg(const MPI_Comm &mpi_communicator,
					      double my_value,
					      MinMaxAvg & result)
    {
      result = Utilities::MPI::min_max_avg (my_value,
					    mpi_communicator);
    }


  }


#ifdef DEAL_II_USE_TRILINOS

  namespace Trilinos
  {
    const Epetra_Comm&
    comm_world()
    {
#ifdef DEAL_II_COMPILER_SUPPORTS_MPI
      static Teuchos::RCP<Epetra_MpiComm>
	communicator = Teuchos::rcp (new Epetra_MpiComm (MPI_COMM_WORLD), true);
#else
      static Teuchos::RCP<Epetra_SerialComm>
	communicator = Teuchos::rcp (new Epetra_SerialComm (), true);
#endif

      return *communicator;
    }



    const Epetra_Comm&
    comm_self()
    {
#ifdef DEAL_II_COMPILER_SUPPORTS_MPI
      static Teuchos::RCP<Epetra_MpiComm>
	communicator = Teuchos::rcp (new Epetra_MpiComm (MPI_COMM_SELF), true);
#else
      static Teuchos::RCP<Epetra_SerialComm>
	communicator = Teuchos::rcp (new Epetra_SerialComm (), true);
#endif

      return *communicator;
    }



    Epetra_Comm *
    duplicate_communicator (const Epetra_Comm &communicator)
    {
#ifdef DEAL_II_COMPILER_SUPPORTS_MPI

				       // see if the communicator is in fact a
				       // parallel MPI communicator; if so,
				       // return a duplicate of it
      const Epetra_MpiComm
	*mpi_comm = dynamic_cast<const Epetra_MpiComm *>(&communicator);
      if (mpi_comm != 0)
	return new Epetra_MpiComm(Utilities::System::
				  duplicate_communicator(mpi_comm->GetMpiComm()));
#endif

				       // if we don't support MPI, or if the
				       // communicator in question was in fact
				       // not an MPI communicator, return a
				       // copy of the same object again
      Assert (dynamic_cast<const Epetra_SerialComm*>(&communicator)
	      != 0,
	      ExcInternalError());
      return new Epetra_SerialComm(dynamic_cast<const Epetra_SerialComm&>(communicator));
    }



    void destroy_communicator (Epetra_Comm &communicator)
    {
      Assert (&communicator != 0, ExcInternalError());

				       // save the communicator, reset
				       // the map, and delete the
				       // communicator if this whole
				       // thing was created as an MPI
				       // communicator
#ifdef DEAL_II_COMPILER_SUPPORTS_MPI
      Epetra_MpiComm
	*mpi_comm = dynamic_cast<Epetra_MpiComm *>(&communicator);
      if (mpi_comm != 0)
	{
	  MPI_Comm comm = mpi_comm->GetMpiComm();
	  *mpi_comm = Epetra_MpiComm(MPI_COMM_SELF);
	  MPI_Comm_free (&comm);
	}
#endif
    }



    unsigned int get_n_mpi_processes (const Epetra_Comm &mpi_communicator)
    {
      return mpi_communicator.NumProc();
    }


    unsigned int get_this_mpi_process (const Epetra_Comm &mpi_communicator)
    {
      return (unsigned int)mpi_communicator.MyPID();
    }



    Epetra_Map
    duplicate_map (const Epetra_BlockMap &map,
		   const Epetra_Comm     &comm)
    {
      if (map.LinearMap() == true)
	{
					   // each processor stores a
					   // contiguous range of
					   // elements in the
					   // following constructor
					   // call
	  return Epetra_Map (map.NumGlobalElements(),
			     map.NumMyElements(),
			     map.IndexBase(),
			     comm);
	}
      else
	{
					   // the range is not
					   // contiguous
	  return Epetra_Map (map.NumGlobalElements(),
			     map.NumMyElements(),
			     map.MyGlobalElements (),
			     0,
			     comm);
	}
    }
  }

#endif

}

DEAL_II_NAMESPACE_CLOSE
