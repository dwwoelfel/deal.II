//---------------------------------------------------------------------------
//    $Id: ghost_01.cc 28440 2013-02-17 14:35:08Z heister $
//    Version: $Name$ 
//
//    Copyright (C) 2004, 2005, 2010 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------


// create, size, and reinit of LA::MPI::SparseMatrix

#include "../tests.h"
#include <deal.II/lac/generic_linear_algebra.h>
#include <deal.II/base/index_set.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>

#include "gla.h"

template <class LA> 
void test ()
{
  unsigned int myid = Utilities::MPI::this_mpi_process (MPI_COMM_WORLD);
  unsigned int numproc = Utilities::MPI::n_mpi_processes (MPI_COMM_WORLD);
  
  if (myid==0)
    deallog << "numproc=" << numproc << std::endl;

				   // each processor owns 2 indices and all
				   // are ghosting Element 1 (the second)

  IndexSet local_active(numproc*2);
  local_active.add_range(myid*2,myid*2+2);
  IndexSet local_relevant= local_active;
  local_relevant.add_range(1,2);

  CompressedSimpleSparsityPattern csp (local_relevant);
  
  for (unsigned int i=0;i<2*numproc;++i)
    if (local_relevant.is_element(i))
      csp.add(i,i);

  if (myid==0)
    csp.add(0,1);

 
  typename LA::MPI::SparseMatrix mat;
  mat.reinit (local_active, local_active, csp, MPI_COMM_WORLD);

  Assert(mat.n()==numproc*2, ExcInternalError());
  Assert(mat.m()==numproc*2, ExcInternalError());

				   // set local values
  mat.set(myid*2,myid*2, myid*2.0);
  mat.set(myid*2+1,myid*2+1, myid*2.0+1.0);

  mat.compress(VectorOperation::insert);

  mat.add(0,1,1.0);
  
  mat.compress(VectorOperation::add);
  
				   // check local values
  if (myid==0)
    {
      deallog << myid*2 << ": " << mat(myid*2,myid*2) << std::endl;
      deallog << myid*2+1 << ": " << mat(myid*2+1,myid*2+1) << std::endl;
      deallog << "0,1 : " << mat(0,1) << std::endl;
    }  

				   // done
  if (myid==0)
    deallog << "OK" << std::endl;
}



int main (int argc, char **argv)
{
  Utilities::MPI::MPI_InitFinalize mpi_initialization(argc, argv, 1);
  MPILogInitAll log(__FILE__);
  {	
    deallog.push("PETSc");
    test<LA_PETSc>();
    deallog.pop();	
    deallog.push("Trilinos");
    test<LA_Trilinos>();
    deallog.pop();	
  }
  
  // compile, don't run
  //if (myid==9999)
    //  test<LA_Dummy>();
  

}
