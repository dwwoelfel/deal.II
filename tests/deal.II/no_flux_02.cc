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
// element that consists of more than dim components and where
// therefore we have to pick the vector components from somewhere in
// the middle


#include "../tests.h"
#include <deal.II/base/logstream.h>
#include <deal.II/base/function.h>
#include <deal.II/base/quadrature_lib.h>
#include <deal.II/lac/vector.h>
#include <deal.II/grid/grid_generator.h>
#include <deal.II/dofs/dof_handler.h>
#include <deal.II/lac/constraint_matrix.h>
#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_system.h>
#include <deal.II/fe/mapping_q1.h>
#include <deal.II/numerics/vector_tools.h>

#include <fstream>



template<int dim>
void test (const Triangulation<dim>& tr,
		      const FiniteElement<dim>& fe)
{
  DoFHandler<dim> dof(tr);
  dof.distribute_dofs(fe);

  for (unsigned int i=0; i<GeometryInfo<dim>::faces_per_cell; ++i)
    {
      deallog << "FE=" << fe.get_name()
	      << ", case=" << i
	      << std::endl;

      std::set<types::boundary_id_t> boundary_ids;
      for (unsigned int j=0; j<=i; ++j)
	boundary_ids.insert (j);
      
      ConstraintMatrix cm;
      VectorTools::compute_no_normal_flux_constraints (dof, 1, boundary_ids, cm);

      cm.print (deallog.get_file_stream ());
    }
}


template<int dim>
void test_hyper_cube()
{
  Triangulation<dim> tr;
  GridGenerator::hyper_cube(tr);

  for (unsigned int i=0; i<GeometryInfo<dim>::faces_per_cell; ++i)
    tr.begin_active()->face(i)->set_boundary_indicator (i);
  
  tr.refine_global(2);

  for (unsigned int degree=1; degree<4; ++degree)
    {
      FESystem<dim> fe (FE_Q<dim>(degree+1), 1,
			FE_Q<dim>(degree), dim,
			FE_Q<dim>(degree+1), 1);
      test(tr, fe);
    }
}


int main()
{
  std::ofstream logfile ("no_flux_02/output");
  deallog << std::setprecision (2);
  deallog << std::fixed;  
  deallog.attach(logfile);
  deallog.depth_console (0);
  deallog.threshold_double(1.e-12);

  test_hyper_cube<2>();
  test_hyper_cube<3>();
}
