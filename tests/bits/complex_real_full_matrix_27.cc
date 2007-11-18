//----------------------------------------------------------------------
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
//----------------------------------------------------------------------


// check FullMatrix::add. like the full_matrix_* tests, but use
// complex-valued matrices and vectors, even though we only store real values
// in them


#include "../tests.h"
#include "full_matrix_common.h"


std::string output_file_name = "complex_real_full_matrix_27/output";


template <typename number>
void
check ()
{
  FullMatrix<std::complex<number> > m(10,10),n;
  make_matrix (n);
  m.add (n, 2., 3, 4, 2, 3);
  print_matrix (m);
}

