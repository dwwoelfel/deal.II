//---------------------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 2005, 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------
#ifndef __deal2__lapack_support_h
#define __deal2__lapack_support_h

namespace LAPACKSupport
{
/**
 * Most LAPACK functions change the contents of the matrix applied to
 * to something which is not a matrix anymore. Therefore, LAPACK
 * matrix classes in <tt>deal.II</tt> have a state flag indicating
 * what happened to them.
 *
 * @author Guido Kanschat, 2005
 */
  enum State
  {
					 /// Contents is actually a matrix.
	matrix,
					 /// Contents is an LU decomposition.
	lu,
					 /// Eigenvalue vector is filled
	eigenvalues,
					 /// Contents is something useless.
	unusable = 0x8000
  };
  
				   /**
				    * Function printing the name of a State.
				    */
  inline const char* state_name(State s)
  {
    switch (s)
      {
	case matrix:
	      return "matrix";
	case lu:
	      return "lu decomposition";
	case eigenvalues:
	      return "eigenvalues";
	case unusable:
	      return "unusable";
	default:
	      return "unknown";
      }
    return "internal error";
  }
  
/**
 * A matrix can have certain features allowing for optimization, but
 * hard to test. These are listed here.
 */
  enum Properties
  {
					 /// No special properties
	general = 0,
					 /// Matrix is symmetric
	symmetric = 1,
					 /// Matrix is upper triangular
	upper_triangle = 2,
					 /// Matrix is lower triangular
	lower_triangle = 4,
					 /// Matrix is diagonal
	diagonal = 6,
					 /// Matrix is in upper Hessenberg form
	hessenberg = 8
  };

				   /**
				    * Character constant.
				    */
  static const char V = 'V';
				   /**
				    * Character constant.
				    */
  static const char T = 'T';
				   /**
				    * Character constant.
				    */
  static const char N = 'N';
				   /**
				    * Integer constant.
				    */
  static const int zero = 0;
				   /**
				    * Integer constant.
				    */
  static const int one = 1;

				   /**
				    * Exception thrown when a matrix
				    * is not in a suitable state for
				    * an operation. For instance, a
				    * LAPACK routine may have left the
				    * matrix in an unusable state,
				    * then vmult does not make sense
				    * anymore.
				    */
  DeclException1(ExcState, State,
		 << "The function cannot be called while the matrix is in state "
		 << state_name(arg1));
  
}


#endif
