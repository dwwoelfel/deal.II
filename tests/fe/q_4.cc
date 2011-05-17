//----------------------------  q_4.cc  ---------------------------
//    q_4.cc,v 1.1 2003/05/08 14:54:41 wolf Exp
//    Version:
//
//    Copyright (C) 2003, 2005 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    fuqher information on this license.
//
//----------------------------  q_4.cc  ---------------------------


// previously, the FETools::get_interpolation_matrix function would
// compute its result itself by interpolation. now, the different
// finite elements do that themselves, if they can. make sure the
// result doesn't change

#include "../tests.h"
#include <deal.II/base/logstream.h>
#include <deal.II/base/quadrature_lib.h>
#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_tools.h>

#include <fstream>
#include <string>

#define PRECISION 5



template<int dim>
void
test(const FE_Q<dim> fe1,
     const FE_Q<dim> fe2)
{
  deallog << fe1.get_name()
          << " to " << fe2.get_name()
	  << std::endl;

  FullMatrix<float> m (fe2.dofs_per_cell,
                       fe1.dofs_per_cell);
  FETools::get_interpolation_matrix (fe1, fe2, m);

  for (unsigned int i=0; i<m.m(); ++i)
    {
      for (unsigned int j=0; j<m.n(); ++j)
        deallog << m(i,j) << ' ';

      deallog << std::endl;
    }

  deallog << std::endl;
}


int
main()
{
  std::ofstream logfile ("q_4/output");
  deallog << std::setprecision(PRECISION);
  deallog << std::fixed;
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  for (unsigned int degree1=1; degree1<=4; ++degree1)
    for (unsigned int degree2=1; degree2<=4; ++degree2)
      test<1>(FE_Q<1>(degree1), FE_Q<1>(degree2));
  for (unsigned int degree1=2; degree1<=5; ++degree1)
    for (unsigned int degree2=2; degree2<=5; ++degree2)
      test<1>(FE_Q<1>(QGaussLobatto<1>(degree1)),
	      FE_Q<1>(QGaussLobatto<1>(degree2)));

  for (unsigned int degree1=1; degree1<=3; ++degree1)
    for (unsigned int degree2=1; degree2<=3; ++degree2)
      test<2>(FE_Q<2>(degree1), FE_Q<2>(degree2));
  for (unsigned int degree1=2; degree1<=4; ++degree1)
    for (unsigned int degree2=2; degree2<=4; ++degree2)
      test<2>(FE_Q<2>(QGaussLobatto<1>(degree1)),
	      FE_Q<2>(QGaussLobatto<1>(degree2)));

  for (unsigned int degree1=1; degree1<=2; ++degree1)
    for (unsigned int degree2=1; degree2<=2; ++degree2)
      test<3>(FE_Q<3>(degree1), FE_Q<3>(degree2));
  for (unsigned int degree1=2; degree1<=4; ++degree1)
    for (unsigned int degree2=2; degree2<=4; ++degree2)
      test<3>(FE_Q<3>(QGaussLobatto<1>(degree1)),
	      FE_Q<3>(QGaussLobatto<1>(degree2)));


  return 0;
}



