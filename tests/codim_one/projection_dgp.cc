//----------------------------  template.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2005, 2008 by the deal.II authors 
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  template.cc  ---------------------------


// projection of a function on the surface of a hypersphere
// with discontinous elements

#include "../tests.h"
#include <fstream>
#include <deal.II/base/logstream.h>

// all include files needed for the program

#include <deal.II/grid/tria.h>
#include <deal.II/grid/grid_in.h>
#include <deal.II/grid/grid_out.h>
#include <deal.II/fe/mapping.h>
#include <deal.II/fe/mapping_q1.h>
#include <deal.II/fe/fe_dgp.h>
#include <deal.II/fe/fe_values.h>
#include <deal.II/base/quadrature_lib.h>
#include <deal.II/dofs/dof_handler.h>
#include <deal.II/dofs/dof_accessor.h>
#include <deal.II/lac/constraint_matrix.h>
#include <deal.II/numerics/vector_tools.h>
#include <deal.II/numerics/data_out.h>
#include <deal.II/base/function.h>
#include <deal.II/base/function_lib.h>

#include <deal.II/base/quadrature_lib.h>

#include <fstream>
#include <string>


std::ofstream logfile("projection_dgp/output");


template <int dim, int spacedim>
void test(std::string filename, unsigned int n) {

  Triangulation<dim, spacedim> triangulation;
  GridIn<dim, spacedim> gi;

  gi.attach_triangulation (triangulation);
  std::ifstream in (filename.c_str());
  gi.read_ucd (in);

  FE_DGP<dim,spacedim>     fe (n);
  DoFHandler<dim,spacedim> dof_handler (triangulation);

  dof_handler.distribute_dofs (fe);
  
  // Now we interpolate the constant function on the mesh, and check
  // that this is consistent with what we expect.
  Vector<double> interpolated_one(dof_handler.n_dofs());
  Vector<double> error(dof_handler.n_dofs());

  Functions::CosineFunction<spacedim> cosine;
  QGauss<dim> quad(5);
  ConstraintMatrix constraints;
  constraints.close();
  VectorTools::project(dof_handler, constraints, quad, cosine, interpolated_one);
 
  DataOut<dim, DoFHandler<dim,spacedim> > dataout;
  dataout.attach_dof_handler(dof_handler);
  dataout.add_data_vector(interpolated_one, "numbering");
  dataout.build_patches();
  dataout.write_vtk(logfile);
}



int main () 
{
  deallog.attach(logfile);
  deallog.depth_console(0);
  
  for (unsigned int n=1; n<5; n++)
    {
      deallog << "Test<1,2>, continous finite element q_" << n << std::endl;
      test<1,2>("grids/circle_2.inp",n);
      
     deallog << "Test<2,3>, continous finite element q_" << n << std::endl;
     test<2,3>("grids/sphere_2.inp",n);
    }
  return 0;
}
