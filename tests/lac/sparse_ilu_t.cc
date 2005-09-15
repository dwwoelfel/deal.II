//----------------------------  sparse_ilu_t.cc  -------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2001, 2002, 2003, 2004, 2005 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  sparse_ilu_t.cc  -------------------------


// make sure that the SparseILU applied with infinite fill-in
// generates the exact inverse matrix

#include "../tests.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include "testmatrix.h"
#include <base/logstream.h>
#include <lac/sparse_matrix.h>
#include <lac/sparse_ilu.h>
#include <lac/vector.h>

//TODO:[WB] find test that is less sensitive to floating point accuracy

int main()
{
  std::ofstream logfile("sparse_ilu_t.output");
  logfile.setf(std::ios::fixed);
  logfile.precision(3);
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);
  

  for (unsigned int size=4; size <= 16; size *= 2)
    {
      unsigned int dim = (size-1)*(size-1);

      deallog << "Size " << size << " Unknowns " << dim << std::endl;
      
				       // Make matrix
      FDMatrix testproblem(size, size);
      SparsityPattern structure(dim, dim, 5);
      testproblem.five_point_structure(structure);
      structure.compress();
      SparseMatrix<double>  A(structure);
      testproblem.five_point(A);

      
      for (unsigned int test=0; test<2; ++test)
	{
	  deallog << "Test " << test << std::endl;
	  
					   // generate sparse ILU.
					   //
					   // for test 1, test with
					   // full pattern.  for test
					   // 2, test with same
					   // pattern as A
	  SparsityPattern ilu_pattern (dim, dim,
				       (test==0 ? dim : 5));
	  switch (test)
	    {
	      case 0:
		    for (unsigned int i=0; i<dim; ++i)
		      for (unsigned int j=0; j<dim; ++j)
			ilu_pattern.add(i,j);
		    break;

	      case 1:
		    for (unsigned int i=0; i<dim; ++i)
		      for (unsigned int j=0; j<dim; ++j)
			if (structure(i,j) != SparsityPattern::invalid_entry)
			  ilu_pattern.add(i,j);
		    break;

	      default:
		    Assert (false, ExcNotImplemented());
	    };
	  ilu_pattern.compress();
	  SparseILU<double> ilu (ilu_pattern);
	  ilu.decompose (A);
	  
					   // now for three test vectors v
					   // determine norm of
					   // (I-B'A')v, where B' is the ILU 
	                                   // of A transposed.
					   // Since matrix is symmetric,
					   // likewise test for right
					   // preconditioner
	  Vector<double> v(dim);
	  Vector<double> tmp1(dim), tmp2(dim);
	  for (unsigned int i=0; i<3; ++i)
	    {
	      for (unsigned int j=0; j<dim; ++j)
		v(j) = 1. * std::rand()/RAND_MAX;
	      
	      A.Tvmult (tmp1, v);
	      ilu.Tvmult (tmp2, tmp1);
	      tmp2 -= v;
	      const double left_residual = tmp2.l2_norm();
	      
	      ilu.Tvmult (tmp1, v);
	      A.Tvmult (tmp2, tmp1);
	      tmp2 -= v;
	      const double right_residual = tmp2.l2_norm();
	      
	      
	      deallog << "Residual with test vector " << i << ":  "
		      << " left=" << left_residual
		      << ", right=" << right_residual
		      << std::endl;
	    };
	};
      
    };
}

