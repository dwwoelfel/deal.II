//---------------------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 2004 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------


#include <lac/petsc_full_matrix.h>
#include <lac/petsc_vector.h>

#ifdef DEAL_II_USE_PETSC


namespace PETScWrappers
{
  FullMatrix::FullMatrix (const unsigned int m,
                          const unsigned int n)
  {
    const int ierr
      = MatCreateSeqDense(PETSC_COMM_SELF, m, n, PETSC_NULL,
                          &matrix);

    AssertThrow (ierr == 0, ExcPETScError(ierr));
  }
}


#endif // DEAL_II_USE_PETSC
