//----------------------------------------------------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------------------------------------------------

// plot PolynomialsRaviartThomas on the reference cell

#include "../tests.h"
#include <base/logstream.h>
#include <base/tensor.h>
#include <base/polynomials_raviart_thomas.h>
#include <base/quadrature_lib.h>

#include <vector>
#include <iomanip>
#include <fstream>

using namespace std;

template <int dim>
void plot(const PolynomialsRaviartThomas<dim>& poly)
{
  QTrapez<1> base_quadrature;
  QIterated<dim> quadrature(base_quadrature, poly.degree()+3);
  std::vector<Tensor<1,dim> > values(poly.n());
  std::vector<Tensor<2,dim> > grads;
  std::vector<Tensor<3,dim> > grads2;

  
  for (unsigned int k=0;k<quadrature.n_quadrature_points;++k)
    {
      if (k%(poly.degree()+4) == 0)
	deallog << "RT" << poly.degree() << '<' << dim << '>' << std::endl;
      
      deallog << "RT" << poly.degree() << '<' << dim << '>'
	      << '\t' << quadrature.point(k);
      poly.compute(quadrature.point(k), values, grads, grads2);
      
      for (unsigned int i=0;i<poly.n();++i)
	for (unsigned int d=0;d<dim;++d)
	  deallog << '\t' << values[i][d];
      deallog << std::endl;
    }
}

int main()
{
  std::ofstream logfile("polynomials_rt/output");
  deallog << std::setprecision(3);
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  PolynomialsRaviartThomas<2> p20(0);
  PolynomialsRaviartThomas<2> p21(1);
  PolynomialsRaviartThomas<2> p22(2);

  plot(p20);
  plot(p21);
  plot(p22);
}
