//---------------------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------


#include <lac/full_matrix.templates.h>
#include <base/logstream.h>

DEAL_II_NAMESPACE_OPEN

#define TYPEMAT float

template class FullMatrix<TYPEMAT>;

template void FullMatrix<TYPEMAT>::print(
  LogStream&, const unsigned int, const unsigned int) const;
template void FullMatrix<TYPEMAT>::print(
  std::ostream&, const unsigned int, const unsigned int) const;

template FullMatrix<TYPEMAT>& FullMatrix<TYPEMAT>::operator =(
  const FullMatrix<double>&);

#define TYPEMAT2 float

//template FullMatrix<TYPEMAT>& FullMatrix<TYPEMAT>::operator =<>(const FullMatrix<TYPEMAT2>&);
template void FullMatrix<TYPEMAT>::fill<TYPEMAT2> (
  const FullMatrix<TYPEMAT2>&, unsigned, unsigned, unsigned, unsigned);
template void FullMatrix<TYPEMAT>::add<TYPEMAT2> (const TYPEMAT, const FullMatrix<TYPEMAT2>&);
template void FullMatrix<TYPEMAT>::add<TYPEMAT2> (const TYPEMAT, const FullMatrix<TYPEMAT2>&,
						  const TYPEMAT, const FullMatrix<TYPEMAT2>&);
template void FullMatrix<TYPEMAT>::add<TYPEMAT2> (const TYPEMAT, const FullMatrix<TYPEMAT2>&,
						  const TYPEMAT, const FullMatrix<TYPEMAT2>&,
						  const TYPEMAT, const FullMatrix<TYPEMAT2>&);
template void FullMatrix<TYPEMAT>::add<TYPEMAT2> (
  const FullMatrix<TYPEMAT2>&, TYPEMAT, unsigned, unsigned, unsigned, unsigned);
template void FullMatrix<TYPEMAT>::Tadd<TYPEMAT2> (const TYPEMAT, const FullMatrix<TYPEMAT2>&);
template void FullMatrix<TYPEMAT>::Tadd<TYPEMAT2> (
  const FullMatrix<TYPEMAT2>&, TYPEMAT, unsigned, unsigned, unsigned, unsigned);
template void FullMatrix<TYPEMAT>::equ<TYPEMAT2> (const TYPEMAT, const FullMatrix<TYPEMAT2>&);
template void FullMatrix<TYPEMAT>::equ<TYPEMAT2> (const TYPEMAT, const FullMatrix<TYPEMAT2>&,
						  const TYPEMAT, const FullMatrix<TYPEMAT2>&);
template void FullMatrix<TYPEMAT>::equ<TYPEMAT2> (const TYPEMAT, const FullMatrix<TYPEMAT2>&,
						  const TYPEMAT, const FullMatrix<TYPEMAT2>&,
						  const TYPEMAT, const FullMatrix<TYPEMAT2>&);
template void FullMatrix<TYPEMAT>::mmult<TYPEMAT2> (FullMatrix<TYPEMAT2>&, const FullMatrix<TYPEMAT2>&, const bool) const;
template void FullMatrix<TYPEMAT>::Tmmult<TYPEMAT2> (FullMatrix<TYPEMAT2>&, const FullMatrix<TYPEMAT2>&, const bool) const;
template void FullMatrix<TYPEMAT>::add_diag<TYPEMAT2> (const TYPEMAT, const FullMatrix<TYPEMAT2>&);
template void FullMatrix<TYPEMAT>::invert<TYPEMAT2> (const FullMatrix<TYPEMAT2>&);

#define TYPEVEC double
#define TYPERES double

template void FullMatrix<TYPEMAT>::fill_permutation<TYPEVEC> (
  const FullMatrix<TYPEVEC>&,
  const std::vector<unsigned int>&,
  const std::vector<unsigned int>&);
template void FullMatrix<TYPEMAT>::vmult<TYPEVEC>(
  Vector<TYPEVEC>&, const Vector<TYPEVEC>&, bool) const;
template void FullMatrix<TYPEMAT>::Tvmult<TYPEVEC>(
  Vector<TYPEVEC>&, const Vector<TYPEVEC>&, bool) const;
template TYPEMAT FullMatrix<TYPEMAT>::residual<TYPEVEC>(
  Vector<TYPEVEC>&, const Vector<TYPEVEC>&, const Vector<TYPERES>&) const;
template TYPEVEC FullMatrix<TYPEMAT>::matrix_norm_square<TYPEVEC> (
  const Vector<TYPEVEC> &) const;
template TYPEVEC FullMatrix<TYPEMAT>::matrix_scalar_product<TYPEVEC>(
  const Vector<TYPEVEC>&, const Vector<TYPEVEC>&) const;
template void FullMatrix<TYPEMAT>::forward<TYPEVEC>(
  Vector<TYPEVEC>&, const Vector<TYPEVEC>&) const;
template void FullMatrix<TYPEMAT>::backward<TYPEVEC>(
  Vector<TYPEVEC>&, const Vector<TYPEVEC>&) const;

template
void FullMatrix<TYPEMAT>::precondition_Jacobi<TYPEVEC> (
  Vector<TYPEVEC> &, const Vector<TYPEVEC> &, const TYPEMAT) const;

#undef TYPEVEC
#define TYPEVEC float

template void FullMatrix<TYPEMAT>::fill_permutation<TYPEVEC> (
  const FullMatrix<TYPEVEC>&,
  const std::vector<unsigned int>&,
  const std::vector<unsigned int>&);
template void FullMatrix<TYPEMAT>::vmult<TYPEVEC>(
  Vector<TYPEVEC>&, const Vector<TYPEVEC>&, bool) const;
template void FullMatrix<TYPEMAT>::Tvmult<TYPEVEC>(
  Vector<TYPEVEC>&, const Vector<TYPEVEC>&, bool) const;
template TYPEMAT FullMatrix<TYPEMAT>::residual<TYPEVEC>(
  Vector<TYPEVEC>&, const Vector<TYPEVEC>&, const Vector<TYPERES>&) const;
template TYPEVEC FullMatrix<TYPEMAT>::matrix_norm_square<TYPEVEC> (
  const Vector<TYPEVEC> &) const;
template TYPEVEC FullMatrix<TYPEMAT>::matrix_scalar_product<TYPEVEC>(
  const Vector<TYPEVEC>&, const Vector<TYPEVEC>&) const;
template void FullMatrix<TYPEMAT>::forward<TYPEVEC>(
  Vector<TYPEVEC>&, const Vector<TYPEVEC>&) const;
template void FullMatrix<TYPEMAT>::backward<TYPEVEC>(
  Vector<TYPEVEC>&, const Vector<TYPEVEC>&) const;
template
void FullMatrix<TYPEMAT>::precondition_Jacobi<TYPEVEC> (
  Vector<TYPEVEC> &, const Vector<TYPEVEC> &, const TYPEMAT) const;


#undef TYPERES
#define TYPERES float

template TYPEMAT FullMatrix<TYPEMAT>::residual<TYPEVEC,TYPERES>(
  Vector<TYPEVEC>&, const Vector<TYPEVEC>&, const Vector<TYPERES>&) const;

DEAL_II_NAMESPACE_CLOSE
