//----------------------------  testmatrix.cc  ---------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 1998, 1999, 2000, 2002 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  testmatrix.cc  ---------------------------


//TODO: [GK] Produce some useful output!

#include "testmatrix.h"
#include <base/logstream.h>
#include <lac/sparse_matrix.h>
#include <lac/sparse_matrix_ez.h>
#include <lac/vector.h>
#include <lac/solver_richardson.h>
#include <lac/precondition.h>

#include <lac/sparse_matrix_ez.templates.h>

#include <fstream>
#include <cstdio>


#define PREC_CHECK(solver, method, precond) try \
 { solver.method (A, u, f, precond);  } catch (...) {} \
 residuals.push_back(control.last_value())

template<class MATRIX>
void
check_vmult_quadratic(std::vector<double>& residuals,
		      const MATRIX& A,
		      const char* prefix)
{
  deallog.push(prefix);
  
  Vector<double> u(A.n());
  Vector<double> f(A.m());
  GrowingVectorMemory<> mem;

  SolverControl control(10, 1.e-13, false);
  SolverRichardson<> rich(control, mem, .01);
  SolverRichardson<> prich(control, mem, 1.);
  PreconditionIdentity identity;
  PreconditionJacobi<MATRIX> jacobi;
  jacobi.initialize(A, .5);
  PreconditionSOR<MATRIX> sor;
  sor.initialize(A, 1.2);
  PreconditionSSOR<MATRIX> ssor;
  ssor.initialize(A, 1.2);
  
  u = 0.;
  f = 1.;

  PREC_CHECK(rich, solve, identity);
  PREC_CHECK(prich, solve, jacobi);
  PREC_CHECK(prich, solve, ssor);
  PREC_CHECK(prich, solve, sor);
  
  u = 0.;
  deallog << "Transpose" << std::endl;
  PREC_CHECK(rich, Tsolve, identity);
  PREC_CHECK(prich, Tsolve, jacobi);
  PREC_CHECK(prich, Tsolve, ssor);
  PREC_CHECK(prich, Tsolve, sor);
  deallog.pop();
}


template <class MATRIX>
void
check_iterator (const MATRIX& A)
{
  for (typename MATRIX::const_iterator i = A.begin(); i!= A.end(); ++i)
    deallog << '\t' << i->row()
	    << '\t' << i->column()
      	    << '\t' << i->index()
      	    << '\t' << i->value()
	    << std::endl;
  deallog << "Repeat row 2" << std::endl;
  for (typename MATRIX::const_iterator i = A.begin(2); i!= A.end(2); ++i)
    deallog << '\t' << i->row()
	    << '\t' << i->column()
      	    << '\t' << i->index()
      	    << '\t' << i->value()
	    << std::endl;
}


int main()
{
  std::ofstream logfile("sparse_matrices.output");
  logfile.setf(std::ios::fixed);
  logfile.precision(2);
  deallog.attach(logfile);

				   // Switch between regression test
				   // and benchmark
#ifdef DEBUG  
  deallog.depth_console(0);
  const unsigned int size = 5;
  const unsigned int row_length = 3;
#else
  deallog.depth_console(1000);
  deallog.log_execution_time(true);
  deallog.log_time_differences(true);
  const unsigned int size = 500;
  const unsigned int row_length = 9;
#endif
  
  FDMatrix testproblem (size, size);
  unsigned int dim = (size-1)*(size-1);

  std::vector<double> A_res;
  std::vector<double> E_res;
  
  deallog << "Structure" << std::endl;
  SparsityPattern structure(dim, dim, 5);
  testproblem.five_point_structure(structure);
  structure.compress();
  SparseMatrix<double>  A(structure);
  deallog << "Assemble" << std::endl;
  testproblem.five_point(A, true);
  check_vmult_quadratic(A_res, A, "5-SparseMatrix<double>");

  SparseMatrixEZ<double> E(dim,dim,row_length,2);
  deallog << "Assemble" << std::endl;
  testproblem.five_point(E, true);
  check_vmult_quadratic(E_res, E, "5-SparseMatrixEZ<double>");
#ifdef DEBUG
  check_iterator(E);
#endif
  
  A.clear();
  deallog << "Structure" << std::endl;
  structure.reinit(dim, dim, 9);
  testproblem.nine_point_structure(structure);
  structure.compress();
  A.reinit(structure);
  deallog << "Assemble" << std::endl;
  testproblem.nine_point(A);
  check_vmult_quadratic(A_res, A, "9-SparseMatrix<double>");

  E.clear();
  E.reinit(dim,dim,row_length,2);
  deallog << "Assemble" << std::endl;
  testproblem.nine_point(E);
  check_vmult_quadratic(E_res, E, "9-SparseMatrixEZ<double>");

  for (unsigned int i=0;i<A_res.size();++i)
    if (std::fabs(A_res[i] - E_res[i]) > 1.e-14)
      deallog << "SparseMatrix and SparseMatrixEZ differ!!!"
	      << std::endl;

                                   // dump A into a file, and re-read
                                   // it, then delete tmp file and
                                   // check equality
  std::ofstream tmp_write ("sparse_matrices.tmp");
  A.block_write (tmp_write);
  tmp_write.close ();
  
  std::ifstream tmp_read ("sparse_matrices.tmp");
  SparseMatrix<double> A_tmp;
  A_tmp.reinit (A.get_sparsity_pattern());
  A_tmp.block_read (tmp_read);
  tmp_read.close ();

  remove ("sparse_matrices.tmp");

  for (unsigned int i=0; i<A.n_nonzero_elements(); ++i)
    Assert (std::fabs(A.global_entry(i) - A_tmp.global_entry(i)) <=
            std::fabs(1e-14*A.global_entry(i)),
            ExcInternalError());
}
