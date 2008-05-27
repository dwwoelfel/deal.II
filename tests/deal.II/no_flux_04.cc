//----------------------------------------------------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2007, 2008 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------------------------------------------------


// check the creation of no-flux boundary conditions for a finite
// element that consists of only a single set of vector components
// (i.e. it has dim components)
//
// like no_flux_04 but apply the constraints to a vector field to see
// whether the result looks alright

#include "../tests.h"
#include <base/logstream.h>
#include <base/function.h>
#include <base/quadrature_lib.h>
#include <lac/vector.h>
#include <grid/grid_generator.h>
#include <grid/tria_boundary_lib.h>
#include <dofs/dof_handler.h>
#include <dofs/dof_constraints.h>
#include <fe/fe_q.h>
#include <fe/fe_system.h>
#include <fe/mapping_q1.h>
#include <numerics/vectors.h>
#include <numerics/data_out.h>

#include <fstream>


// a function that shows something useful on the surface of a sphere
template <int dim>
class RadialFunction : public Function<dim>
{
  public:
    RadialFunction() : Function<dim> (dim) {}
    
    virtual void vector_value (const Point<dim> &p,
			       Vector<double> &v) const
      {
	Assert (v.size() == dim, ExcInternalError());

	switch (dim)
	  {
	    case 2:
		  v(0) = p[0] + p[1];
		  v(1) = p[1] - p[0];
		  break;
	    case 3:
		  v(0) = p[0] + p[1];
		  v(1) = p[1] - p[0];
		  v(2) = p[2];
		  break;
	    default:
		  Assert (false, ExcNotImplemented());
	  }
      }
};



template<int dim>
void test (const Triangulation<dim>& tr,
	   const FiniteElement<dim>& fe)
{
  DoFHandler<dim> dof(tr);
  dof.distribute_dofs(fe);

  deallog << "FE=" << fe.get_name()
	  << std::endl;

  std::set<unsigned char> boundary_ids;
  boundary_ids.insert (0);
      
  ConstraintMatrix cm;
  VectorTools::compute_no_normal_flux_constraints (dof, 0, boundary_ids, cm);
  cm.close ();

  DoFHandler<dim> dh (tr);
  dh.distribute_dofs (fe);

  Vector<double> v(dh.n_dofs());
  VectorTools::interpolate (dh, RadialFunction<dim>(), v);

  cm.distribute (v);

  DataOut<dim> data_out;
  data_out.attach_dof_handler (dh);

  std::vector<DataComponentInterpretation::DataComponentInterpretation>
    data_component_interpretation
    (dim, DataComponentInterpretation::component_is_part_of_vector);
  
  data_out.add_data_vector (v, "x",
			    DataOut<dim>::type_dof_data,
			    data_component_interpretation);
  data_out.build_patches (fe.degree);

  data_out.write_vtk (deallog.get_file_stream());
}



template<int dim>
void test_hyper_sphere()
{
  Triangulation<dim> tr;
  GridGenerator::hyper_ball(tr);

  static const HyperBallBoundary<dim> boundary;
  tr.set_boundary (0, boundary);
  
  tr.refine_global(2);

  for (unsigned int degree=1; degree<6-dim; ++degree)
    {
      FESystem<dim> fe (FE_Q<dim>(degree), dim);
      test(tr, fe);
    }
}


int main()
{
  std::ofstream logfile ("no_flux_04/output");
  deallog << std::setprecision (2);
  deallog << std::fixed;  
  deallog.attach(logfile);
  deallog.depth_console (0);
  deallog.threshold_double(1.e-12);

  test_hyper_sphere<2>();
  test_hyper_sphere<3>();
}
