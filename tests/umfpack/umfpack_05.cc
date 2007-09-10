//----------------------------------------------------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2002, 2003, 2004, 2005, 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------------------------------------------------

// test the umfpack sparse direct solver on a simple 2x2 block matrix
// that equals the unit matrix

#include "../tests.h"
#include <iostream>
#include <fstream>
#include <cstdlib>

#include <lac/block_sparse_matrix.h>
#include <lac/block_sparsity_pattern.h>
#include <lac/vector.h>
#include <lac/sparse_direct.h>


void test ()
{
  std::vector<unsigned int> size (2,2U);
  
  BlockSparsityPattern b_sparsity_pattern;
  b_sparsity_pattern.reinit(size.size(),size.size());
  for (unsigned int k=0; k<size.size();++k)
    for (unsigned int l=0; l<size.size();++l)
      b_sparsity_pattern.block(k,l).reinit(size[k],
					   size[l],
					   2);
  b_sparsity_pattern.collect_sizes();
  for (unsigned int i=0; i<4; ++i)
    for (unsigned int j=0; j<4; ++j)
      b_sparsity_pattern.add (i,j);
  b_sparsity_pattern.compress();
  
  BlockSparseMatrix<double> Bb (b_sparsity_pattern);  
  for (unsigned int i=0; i<4; ++i)
    Bb.add (i,i,1);

  SparseDirectUMFPACK umfpackb;
  umfpackb.factorize(Bb);

  Vector<double> ubb(4);
  for (unsigned int i=0; i<4; ++i)
    ubb(i) = i;
  
  umfpackb.solve(ubb);

  for (unsigned int i=0; i<4; ++i)
    Assert (std::fabs (ubb(i) - i) < 1e-12,
	    ExcInternalError());

  deallog << "OK" << std::endl;
}


int main ()
{
  std::ofstream logfile("umfpack_05/output");
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  test ();
}
