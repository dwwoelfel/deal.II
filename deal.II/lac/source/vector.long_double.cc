//----------------------------  vector.long_double.cc  ---------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 1998, 1999, 2000, 2001, 2002 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  vector.long_double.cc  ---------------------------


#include <lac/vector.templates.h>

// explicit instantiations
template class Vector<long double>;

template Vector<long double>& Vector<long double>::template operator=<>(const Vector<double>&);
template Vector<long double>& Vector<long double>::template operator=<>(const Vector<float>&);
template long double Vector<long double>::template operator *<> (const Vector<long double> &) const;
template long double Vector<long double>::template operator *<> (const Vector<double> &) const;
template long double Vector<long double>::template operator *<> (const Vector<float> &) const;
template void Vector<long double>::reinit<>(const Vector<long double>&, const bool);
template void Vector<long double>::reinit<>(const Vector<double>&, const bool);
template void Vector<long double>::reinit<>(const Vector<float>&, const bool);
template void Vector<long double>::equ<>(const long double, const Vector<long double>&);
template void Vector<long double>::equ<>(const long double, const Vector<double>&);
template void Vector<long double>::equ<>(const long double, const Vector<float>&);

template Vector<double>& Vector<double>::template operator=<>(const Vector<long double>&);
template double Vector<double>::template operator *<> (const Vector<long double> &) const;
template void Vector<double>::reinit<>(const Vector<long double>&, const bool);
template void Vector<double>::equ<>(const double, const Vector<long double>&);

template Vector<float>& Vector<float>::template operator=<>(const Vector<long double>&);
template float Vector<float>::template operator *<> (const Vector<long double> &) const;
template void Vector<float>::reinit<>(const Vector<long double>&, const bool);
template void Vector<float>::equ<>(const float, const Vector<long double>&);

// see the .h file for why these functions are disabled.
// template Vector<float>::Vector (const Vector<double>& v);
// template Vector<double>::Vector (const Vector<float>& v);
