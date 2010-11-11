//----------------------------  q_point_sum_2.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2003, 2004, 2005, 2010 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  q_point_sum_2.cc  ---------------------------


// integrating \vec x over the surface of the [-1,1] hypercube and
// hyperball in 2d and 3d should yield zero
//
// same as q_point_sum_1, but with different mappings


#include "../tests.h"
#include <base/logstream.h>
#include <base/quadrature_lib.h>
#include <grid/tria.h>
#include <grid/tria_boundary_lib.h>
#include <grid/tria_accessor.h>
#include <grid/tria_iterator.h>
#include <grid/grid_generator.h>
#include <dofs/dof_handler.h>
#include <fe/fe_q.h>
#include <fe/mapping_q.h>
#include <fe/fe_values.h>

#include <fstream>



template <int dim>
void check (const Triangulation<dim> &tria,
            const unsigned int        order)
{
  MappingQ<dim> mapping(order);

  FE_Q<dim> fe(1);
  DoFHandler<dim> dof_handler (tria);
  dof_handler.distribute_dofs (fe);

  QGauss<dim-1> q_face(3);
  
  FEFaceValues<dim>    fe_face_values (mapping, fe, q_face,
                                       update_q_points | update_JxW_values);
  FESubfaceValues<dim> fe_subface_values (mapping, fe, q_face,
                                          update_q_points | update_JxW_values);

  Point<dim> n1, n2;
  for (typename DoFHandler<dim>::active_cell_iterator
         cell = dof_handler.begin_active();
       cell!=dof_handler.end(); ++cell)
    {
                                       // first integrate over faces
                                       // and make sure that the
                                       // result of the integration is
                                       // close to zero
      for (unsigned int f=0; f<GeometryInfo<dim>::faces_per_cell; ++f)
        if (cell->at_boundary(f))
          {
            fe_face_values.reinit (cell, f);
            for (unsigned int q=0; q<q_face.size(); ++q)
              n1 += fe_face_values.quadrature_point(q) *
                    fe_face_values.JxW(q);
          }
      
                                       // now same for subface
                                       // integration
      for (unsigned int f=0; f<GeometryInfo<dim>::faces_per_cell; ++f)
        if (cell->at_boundary(f))
          for (unsigned int sf=0; sf<GeometryInfo<dim>::max_children_per_face; ++sf)
            {
              fe_subface_values.reinit (cell, f, sf);
              for (unsigned int q=0; q<q_face.size(); ++q)
                n2 += fe_subface_values.quadrature_point(q) *
                      fe_subface_values.JxW(q);
            }
    }
  
  Assert (n1*n1 < 1e-24, ExcInternalError());
  deallog << " face integration is ok: "
          << std::sqrt (n1*n1)
          << std::endl;
  Assert (n2*n2 < 1e-24, ExcInternalError());
  deallog << " subface integration is ok: "
          << std::sqrt (n2*n2)
          << std::endl;
}


int main () 
{
  std::ofstream logfile("q_point_sum_2/output");
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  {  
    Triangulation<2> coarse_grid;
    GridGenerator::hyper_cube (coarse_grid, -1, 1);
    check (coarse_grid, 1);
    check (coarse_grid, 2);
    check (coarse_grid, 4);
  }
  
  {  
    Triangulation<3> coarse_grid;
    GridGenerator::hyper_cube (coarse_grid, -1, 1);
    check (coarse_grid, 1);
    check (coarse_grid, 2);
    check (coarse_grid, 3);
  }

  
  {  
    Triangulation<2> coarse_grid;
    GridGenerator::hyper_ball (coarse_grid);
    static const HyperBallBoundary<2> boundary;
    coarse_grid.set_boundary (0, boundary);
    check (coarse_grid, 1);
    check (coarse_grid, 2);
    check (coarse_grid, 4);
  }
  
  {  
    Triangulation<3> coarse_grid;
    GridGenerator::hyper_ball (coarse_grid);
    static const HyperBallBoundary<3> boundary;
    coarse_grid.set_boundary (0, boundary);
    check (coarse_grid, 1);
    check (coarse_grid, 2);
    check (coarse_grid, 3);
  }
  
}

  
  
