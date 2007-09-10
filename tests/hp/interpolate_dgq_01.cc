//----------------------------  interpolate_dgq_01.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  interpolate_dgq_01.cc  ---------------------------


// check that VectorTools::interpolate works for FE_DGQ(p) elements correctly on
// a uniformly refined mesh for functions of degree q

#include "../tests.h"
#include <base/function.h>
#include <base/logstream.h>
#include <base/quadrature_lib.h>
#include <lac/vector.h>

#include <grid/tria.h>
#include <hp/dof_handler.h>
#include <grid/grid_generator.h>
#include <grid/grid_refinement.h>
#include <grid/tria_accessor.h>
#include <grid/tria_iterator.h>
#include <grid/tria_boundary_lib.h>
#include <dofs/dof_accessor.h>
#include <dofs/dof_tools.h>
#include <numerics/vectors.h>
#include <fe/fe_dgq.h>
#include <hp/q_collection.h>

#include <fstream>
#include <vector>


template <int dim>
class F :  public Function<dim>
{
  public:
    F (const unsigned int q) : q(q) {}
    
    virtual double value (const Point<dim> &p,
			  const unsigned int) const
      {
	double v=0;
	for (unsigned int d=0; d<dim; ++d)
	  for (unsigned int i=0; i<=q; ++i)
	    v += (d+1)*(i+1)*std::pow (p[d], 1.*i);
	return v;
      }

  private:
    const unsigned int q;
};



template <int dim>
void test ()
{
  Triangulation<dim>     triangulation;
  GridGenerator::hyper_cube (triangulation);
  triangulation.refine_global (3);

  for (unsigned int p=1; p<7-dim; ++p)
    {
      FE_DGQ<dim>              fe(p);
      hp::FECollection<dim> hp_fe (fe);
      hp::DoFHandler<dim>        dof_handler(triangulation);
      dof_handler.distribute_dofs (hp_fe);

      Vector<double> interpolant (dof_handler.n_dofs());
      Vector<float>  error (triangulation.n_active_cells());
      for (unsigned int q=0; q<=p+2; ++q)
	{
					   // interpolate the function
	  VectorTools::interpolate (dof_handler,
				    F<dim> (q),
				    interpolant);
      
					   // then compute the interpolation error
	  VectorTools::integrate_difference (dof_handler,
					     interpolant,
					     F<dim> (q),
					     error,
					     hp::QCollection<dim>(QGauss<dim>(q+2)),
					     VectorTools::L2_norm);
	  if (q<=p)
	    Assert (error.l2_norm() < 1e-12*interpolant.l2_norm(),
		    ExcInternalError());

	  deallog << fe.get_name() << ", P_" << q
		  << ", rel. error=" << error.l2_norm() / interpolant.l2_norm()
		  << std::endl;
	}
    }
}



int main ()
{
  std::ofstream logfile("interpolate_dgq_01/output");
  logfile.precision (3);
  
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  test<1>();
  test<2>();
  test<3>();
}

