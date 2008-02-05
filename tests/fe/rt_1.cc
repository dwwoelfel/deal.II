//----------------------------  rt_1.cc  ---------------------------
//    rt_1.cc,v 1.3 2003/06/09 16:00:38 wolf Exp
//    Version: 
//
//    Copyright (C) 2003, 2005, 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  rt_1.cc  ---------------------------


// Show the shape functions of the Raviart-Thomas element on the unit cell
// Plots are gnuplot compatible if lines with desired prefix are selected.

#include "../tests.h"
#include <base/logstream.h>
#include <fe/fe_raviart_thomas.h>

#include <vector>
#include <fstream>
#include <sstream>
#include <string>

#define PRECISION 2



template<int dim>
inline void
plot_shape_functions(const unsigned int degree)
{
  std::ostringstream prefix;
  prefix << "RT<" << dim << ">(" << degree << ')';
  
  deallog.push(prefix.str());
  FE_RaviartThomas<dim> fe_rt(degree);

  const unsigned int div=2;
  for (unsigned int mz=0;mz<=((dim>2) ? div : 0) ;++mz)
    for (unsigned int my=0;my<=((dim>1) ? div : 0) ;++my)
      {
	for (unsigned int mx=0;mx<=div;++mx)
	  {
	    const Point<dim> p = (dim==2 ?
				  Point<dim>(1.*mx/div, 1.*my/div) :
				  Point<dim>(1.*mx/div, 1.*my/div, 1.*mz/div));
	    
					     // Lines with function
					     // values contain
					     // quadrature point and one
					     // vector of dim entries
					     // for each chape function
	    deallog << "value " << p;
	    for (unsigned int i=0;i<fe_rt.dofs_per_cell;++i)
	      {
		for (unsigned int c=0; c<dim; ++c)
		  deallog << " " << fe_rt.shape_value_component(i,p,c);
		deallog << "  ";
	      }
	    deallog << std::endl << "grad "  << p;
	    for (unsigned int i=0;i<fe_rt.dofs_per_cell;++i)
	      {
		for (unsigned int c=0; c<dim; ++c)
		  {
		    deallog << ' ';
		    for (unsigned int d=0; d<dim; ++d)
		      deallog << ' ' << fe_rt.shape_grad_component(i,p,c)[d];
		  }
	      }
	    deallog << std::endl;
	  }
      deallog << "value " << std::endl;
      deallog << "grad " << std::endl;
    }
  
  deallog.pop();  
}


int
main()
{
  std::ofstream logfile ("rt_1/output");
  deallog << std::setprecision(PRECISION);
  deallog << std::fixed;  
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  for (unsigned int degree=0; degree<4; ++degree)
    plot_shape_functions<2>(degree);
//  plot_shape_functions<3>(degree);
  
  return 0;
}



