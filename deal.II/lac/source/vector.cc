//----------------------------  vector.cc  ---------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 1998, 1999, 2000, 2001 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  vector.cc  ---------------------------


#include <lac/vector.templates.h>

// explicit instantiations
template class Vector<double>;
template Vector<double>& Vector<double>::operator=(const Vector<float>&);
template double Vector<double>::operator*(const Vector<float>&) const;
template double Vector<double>::operator*(const Vector<double>&) const;

template class Vector<float>;
template Vector<float>& Vector<float>::operator=(const Vector<double>&);
template float Vector<float>::operator*(const Vector<float>&) const;
template float Vector<float>::operator*(const Vector<double>&) const;

// see the .h file for why these functions are disabled.
// template Vector<float>::Vector (const Vector<double>& v);
// template Vector<double>::Vector (const Vector<float>& v);
