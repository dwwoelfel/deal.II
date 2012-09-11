//---------------------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 2009, 2010, 2011, 2012 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------


// check Utilities::MPI::max()

#include "../tests.h"
#include <deal.II/base/logstream.h>
#include <deal.II/base/utilities.h>
#include <fstream>

void test()
{
  unsigned int myid = Utilities::MPI::this_mpi_process (MPI_COMM_WORLD);
  const unsigned int numprocs = Utilities::MPI::n_mpi_processes (MPI_COMM_WORLD);

  int int_sum;
  unsigned int uint_sum;
  double double_sum;
  float float_sum;

  int_sum
    =
    Utilities::MPI::max<int>(myid+1,
			     MPI_COMM_WORLD);
  uint_sum
    =
    Utilities::MPI::max<unsigned int>(myid+1,
				      MPI_COMM_WORLD);
  float_sum
    =
    Utilities::MPI::max<float>(myid+1,
			       MPI_COMM_WORLD);
  double_sum
    =
    Utilities::MPI::max<double>(myid+1,
				MPI_COMM_WORLD);

  if (myid==0)
    deallog << int_sum << ' ' << uint_sum << ' ' << double_sum << ' ' << float_sum << std::endl;
}


int main(int argc, char *argv[])
{
#ifdef DEAL_II_COMPILER_SUPPORTS_MPI
  Utilities::MPI::MPI_InitFinalize mpi (argc, argv);
#else
  (void)argc;
  (void)argv;
  compile_time_error;

#endif

  if (Utilities::MPI::this_mpi_process (MPI_COMM_WORLD) == 0)
    {
      std::ofstream logfile(output_file_for_mpi("collective_03").c_str());
      deallog.attach(logfile);
      deallog.depth_console(0);
      deallog.threshold_double(1.e-10);

      deallog.push("mpi");
      test();
      deallog.pop();
    }
  else
    test();
}
