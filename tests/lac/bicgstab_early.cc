//----------------------------  bicgstab_early.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2007 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  bicgstab_early.cc  ---------------------------

// adapted from a testcase by Roger Young, sent to the mailing list
// 2007-03-02, that illustrates that bicgstab can't handle early
// success

#include "../tests.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <iomanip>
#include "testmatrix.h"
#include <base/logstream.h>
#include <lac/sparse_matrix.h>
#include <lac/sparse_ilu.h>
#include <lac/solver_bicgstab.h>
#include <lac/vector.h>
#include <lac/precondition.h>


int main()
{
  std::ofstream logfile("bicgstab_early/output");
  logfile.precision(4);
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);
  
  GrowingVectorMemory<> mem;
  SolverControl control(100, 1.e-3);
  SolverBicgstab<> bicgstab(control, mem);

  SparsityPattern sparsity_pattern(4,4,4);
  for (unsigned int i=0; i<4; ++i)
    for (unsigned int j=0; j<4; ++j)
      sparsity_pattern.add(i,j);
  sparsity_pattern.compress();
  
  SparseMatrix<double> M(sparsity_pattern);
  M.set(0,0,21.1);
  M.set(0,1,0);
  M.set(0,2,0);
  M.set(0,3,0);
  M.set(1,1,7.033333333);
  M.set(1,0,0);
  M.set(1,2,0);
  M.set(1,3,3.516666667);
  M.set(2,2,21.1);
  M.set(2,0,0);
  M.set(2,1,0);
  M.set(2,3,0);
  M.set(3,3,7.033333333);
  M.set(3,0,0);
  M.set(3,1,3.516666667);
  M.set(3,2,0);

  Vector<double> rhs(4);
  rhs(0) = rhs(2) = 0;
  rhs(1) = rhs(3) = 0.0975;


  SparseILU<double> ilu (sparsity_pattern);
  ilu.decompose (M);

  Vector<double> solution (4);
				   // this would fail with elements of
				   // the solution vector being set to
				   // NaN before Roger's suggested
				   // change
  bicgstab.solve (M, solution, rhs, ilu);

  for (unsigned int i=0; i<4; ++i)
    deallog << solution(i) << std::endl;

  Vector<double> res (4);
  M.residual (res, solution, rhs);
  deallog << "residual=" << res.l2_norm()
	  << std::endl;
}

