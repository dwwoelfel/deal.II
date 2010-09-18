//----------------------------  quadrature_selector.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  quadrature_selector.cc  ---------------------------


// make sure that the QuadratureSelector works for a selection of
// arguments


#include "../tests.h"
#include <fstream>

#include <base/logstream.h>
#include <base/quadrature_lib.h>
#include <base/quadrature_selector.h>
#include <string>
#include <cmath>


template <int dim>
void check (const std::string     &name,
	    const unsigned int     order,
	    const Quadrature<dim> &q)
{
  Assert (QuadratureSelector<dim>(name, order).get_points() ==
	  q.get_points(),
	  ExcInternalError());
  deallog << name << ' ' << order << " ok" << std::endl;
}


int main()
{
  std::ofstream logfile("quadrature_selector/output");
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  check ("gauss", 2, QGauss<1>(2));
  check ("gauss", 2, QGauss<2>(2));
  check ("gauss", 2, QGauss<3>(2));
  
  check ("gauss", 2, QGauss<3>(2));
  check ("gauss", 6, QGauss<3>(6));
  check ("gauss", 10, QGauss<3>(10));

  check ("weddle", 0, QWeddle<2>());
}


