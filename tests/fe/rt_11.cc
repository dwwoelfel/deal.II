//----------------------------  rt_11.cc  ---------------------------
//    rt_11.cc,v 1.3 2003/06/09 16:00:38 wolf Exp
//    Version: 
//
//    Copyright (C) 2003, 2005, 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  rt_11.cc  ---------------------------

// Observe how the values of the shape functions change as we make a
// cell smaller and smaller. Evaluate the values with FEFaceValues, to
// make sure the values scale as in rt_10 where we used FEValues
//
// the test presently fails because of the issue with computing the
// normals using FEFaceValue, where FEFaceValue by accident uses the
// *face* mapping, not the *cell* mapping to compute the Piola
// transform (leading to a missing power of h in the determinant)

#include "../tests.h"
#include <base/quadrature_lib.h>
#include <base/logstream.h>
#include <lac/vector.h>
#include <lac/solver_cg.h>
#include <lac/precondition.h>
#include <lac/vector_memory.h>
#include <lac/sparse_matrix.h>
#include <lac/sparsity_pattern.h>
#include <grid/tria.h>
#include <grid/tria_iterator.h>
#include <dofs/dof_accessor.h>
#include <grid/grid_generator.h>
#include <grid/grid_tools.h>
#include <fe/fe_raviart_thomas.h>
#include <fe/fe_values.h>

#include <vector>
#include <fstream>
#include <string>

#define PRECISION 2


std::ofstream logfile ("rt_11/output");

template<int dim>
void
test (const unsigned int degree)
{
  FE_RaviartThomas<dim> fe_rt(degree);

  deallog << "Degree=" << degree
	  << std::endl;
  
  for (double h=1; h>1./128; h/=2)
    {      
      deallog << "  h=" << h
	      << std::endl;

      Triangulation<dim> tr;
      GridGenerator::hyper_cube(tr, 0., h);

      DoFHandler<dim> dof(tr);
      dof.distribute_dofs(fe_rt);

      QTrapez<dim-1> quadrature;

      FEFaceValues<dim> fe_values (fe_rt, quadrature, update_values);
      fe_values.reinit (dof.begin_active(), 0);
      for (unsigned int q=0; q<quadrature.n_quadrature_points; ++q)
	{
	  deallog << "    Quadrature point " << q << ": ";
	  for (unsigned int i=0; i<fe_rt.dofs_per_cell; ++i)
	    {
	      deallog << '[';
	      for (unsigned int c=0; c<fe_rt.n_components(); ++c)
		deallog << fe_values.shape_value_component(i,q,c) << ' ';
	      deallog << ']';
	    }
	  deallog << std::endl;
	}
    }
}


int
main()
{
  logfile.precision (PRECISION);
  logfile.setf(std::ios::fixed);  
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  for (unsigned int i=0; i<4; ++i)
    test<2>(i);
  
  return 0;
}



