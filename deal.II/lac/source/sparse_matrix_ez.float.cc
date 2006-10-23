//---------------------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 2003, 2005, 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------


#include <base/logstream.h>
#include <lac/sparse_matrix_ez.templates.h>
#include <iostream>

#define TYPEMAT float

DEAL_II_NAMESPACE_OPEN
template class SparseMatrixEZ<TYPEMAT>;
DEAL_II_NAMESPACE_CLOSE

#define TYPEVEC float
#include "sparse_matrix_ez_vector.in.h"
#undef TYPEVEC

#define TYPEVEC double
#include "sparse_matrix_ez_vector.in.h"
#undef TYPEVEC

				 // a prerelease of gcc3.0 fails to
				 // compile this due to long double
//  #undef TYPE2
//  #define TYPE2 long double

//  #include <lac/sparse_matrix.2.templates>

#undef TYPEMAT
