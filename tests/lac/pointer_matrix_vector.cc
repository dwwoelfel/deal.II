//----------------------------------------------------------------------
//    $Id: pointer_matrix.cc 13349 2006-07-07 09:39:32Z guido $
//    Version: $Name$ 
//
//    Copyright (C) 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------------------------------------------------

// Test vmult and Tvmult of PointerMatrixVector

#include "../tests.h"
#include <base/logstream.h>
#include <lac/pointer_matrix.h>
#include <lac/vector.h>

#include <fstream>

int main()
{
  std::ofstream logfile("pointer_matrix_vector/output");
  deallog.attach(logfile);
  deallog.depth_console(0);
  
  Vector<double> u(5);
  Vector<double> v(5);
  Vector<double> w(1);
  Vector<float>  x(5);
  Vector<float>  y(5);
  Vector<float>  z(1);
  

  for (unsigned int i=0;i<u.size();++i)
    {
      u(i) = 1 << i;
      x(i) = 1 << i;
      v(i) = 6-i;
      y(i) = 6-i;
    }
  
  PointerMatrixVector<double> Mu(&u);
  Mu.vmult(w,v);
  deallog << "vmult  " << w(0) << std::endl << "Tvmult";
  w(0) = 2.;
  Mu.Tvmult(v,w);
  for (unsigned int i=0;i<v.size();++i)
    deallog << ' ' << v(i);
  deallog << std::endl;
  
  PointerMatrixVector<float> Mx(&x);
  Mx.vmult(z,y);
  deallog << "vmult  " << z(0) << std::endl << "Tvmult";
  z(0) = 2.;
  Mx.Tvmult(y,z);
  for (unsigned int i=0;i<y.size();++i)
    deallog << ' ' << y(i);
  deallog << std::endl;
}
