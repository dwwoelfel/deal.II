//----------------------------  rt_3.cc  ---------------------------
//    rt_3.cc,v 1.3 2003/06/09 16:00:38 wolf Exp
//    Version: 
//
//    Copyright (C) 2003, 2005 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  rt_3.cc  ---------------------------


// Just output the constraint matrices of the RT element

#include "../tests.h"
#include <base/logstream.h>
#include <fe/fe_raviart_thomas.h>

#include <fstream>
#include <string>

#define PRECISION 4



template<int dim>
void
test(const unsigned int degree)
{
  deallog << "FE_RaviartThomas<" << dim << "> (" << degree << ")"
	  << std::endl;
  
  FE_RaviartThomas<dim> fe_rt(degree);
  const FullMatrix<double> & constraints = fe_rt.constraints();

  for (unsigned int i=0; i<constraints.m(); ++i)
    {
      for (unsigned int j=0; j<constraints.n(); ++j)
	deallog << constraints(i,j) << ' ';
      deallog << std::endl;
    }
  
  deallog << std::endl;
}


int
main()
{
  std::ofstream logfile ("rt_3/output");
  logfile.precision (PRECISION);
  logfile.setf(std::ios::fixed);  
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  for (unsigned int degree=0; degree<4; ++degree)
    test<2>(degree);
  
  return 0;
}



