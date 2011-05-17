//----------------------------------------------------------------------
//    $Id$
//
//    Copyright (C) 2010 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------------------------------------------------

// Test the SolverSelector class.

#include "../tests.h"
#include "testmatrix.h"
#include <deal.II/base/logstream.h>
#include <deal.II/lac/sparse_matrix.h>
#include <deal.II/lac/vector.h>
#include <deal.II/lac/vector_memory.h>
#include <deal.II/lac/solver_selector.h>

#include <fstream>

template <class MATRIX, class VECTOR>
void
check(const MATRIX& A, const VECTOR& f)
{
  std::vector<std::string> names;
  names.push_back("cg");
  names.push_back("bicgstab");
  names.push_back("gmres");
  names.push_back("fgmres");

  ReductionControl cont1(100, 0., 1.e-4);
  SolverControl cont2(100, 1.e-7);
  SolverSelector<VECTOR> solver;
  PreconditionSSOR<SparseMatrix<double> > pre;
  pre.initialize(A);
  
  VECTOR u;
  u.reinit(f);

  std::vector<std::string>::const_iterator name;
  
  solver.set_control(cont1);
  for (name = names.begin(); name != names.end();++name)
    {
      solver.select(*name);
      u = 0.;
      solver.solve(A, u, f, pre);
    }
  
  solver.set_control(cont2);
  for (name = names.begin(); name != names.end();++name)
    {
      solver.select(*name);
      u = 0.;
      solver.solve(A, u, f, pre);
    }
  
}


int main()
{
  std::ofstream logfile("solver_selector_01/output");
//  logfile.setf(std::ios::fixed);
  deallog << std::setprecision(4);
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  unsigned int size=37;
  unsigned int dim = (size-1)*(size-1);
  
  deallog << "Size " << size << " Unknowns " << dim << std::endl;
  
				   // Make matrix
  FDMatrix testproblem(size, size);
  SparsityPattern structure(dim, dim, 5);
  testproblem.five_point_structure(structure);
  structure.compress();
  SparseMatrix<double>  A(structure);
  testproblem.five_point(A);
  Vector<double>  f(dim);
  f = 1.;
  
  check(A, f);
}
