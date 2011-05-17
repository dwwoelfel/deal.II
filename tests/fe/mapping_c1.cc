// mapping_c1.cc,v 1.5 2003/04/09 15:49:55 wolf Exp
// Copyright (C) 2001, 2003, 2005, 2008 Wolfgang Bangerth
//
// Test the continuity of normal vectors at vertices, and thus the
// C1-ness of the C1-mapping

#include "../tests.h"
#include <deal.II/base/logstream.h>
#include <deal.II/base/quadrature_lib.h>
#include <deal.II/grid/tria.h>
#include <deal.II/grid/tria_boundary_lib.h>
#include <deal.II/grid/grid_generator.h>
#include <deal.II/dofs/dof_handler.h>
#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/mapping_c1.h>
#include <deal.II/fe/fe_values.h>

#include <algorithm>
#include <fstream>
#include <iomanip>

#define PRECISION 2

int main ()
{
  std::ofstream logfile ("mapping_c1/output");
  deallog << std::fixed;  
  deallog << std::setprecision(PRECISION) << std::fixed;
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);
  
				   // create grid of circle, somehow
				   // arbitrarily from only one cell,
				   // but since we are not interested
				   // in the quality of the mesh, this
				   // is ok
  Point<2> center;
  HyperBallBoundary<2> circle(center, std::sqrt(2.0));
  Triangulation<2> tria;
  GridGenerator::hyper_cube(tria, -1, 1);
  tria.set_boundary (0, circle);
  

				   // refine it more or less
				   // arbitrarily
  tria.refine_global (1);
  if (true)
    {
      Triangulation<2>::active_cell_iterator cell = tria.begin_active();
      ++cell;
      cell->set_refine_flag();
      tria.execute_coarsening_and_refinement ();
    };

				   // attach a dof handler to it
  const FE_Q<2>       fe(2);
  DoFHandler<2> dof_handler (tria);
  dof_handler.distribute_dofs (fe);

				   // and loop over all exterior faces
				   // to see whether the normal
				   // vectors are indeed continuous
				   // and pointing radially outward at
				   // the vertices
  const QTrapez<1>    quadrature;
  const MappingC1<2>  c1_mapping;
  FEFaceValues<2> c1_values (c1_mapping, fe, quadrature,
			     update_q_points | update_normal_vectors);

				   // to compare with, also print the
				   // normal vectors as generated by a
				   // cubic mapping
  const MappingQ<2> q3_mapping(3);
  FEFaceValues<2> q3_values (q3_mapping, fe, quadrature,
			     update_q_points | update_normal_vectors);
  
  for (DoFHandler<2>::active_cell_iterator cell=dof_handler.begin_active();
       cell!=dof_handler.end(); ++cell)
    for (unsigned int f=0; f<GeometryInfo<2>::faces_per_cell; ++f)
      if (cell->face(f)->at_boundary())
	{
	  c1_values.reinit (cell, f);
	  q3_values.reinit (cell, f);

					   // there should now be two
					   // normal vectors, one for
					   // each vertex of the face
	  Assert (c1_values.get_normal_vectors().size() == 2,
		  ExcInternalError());

					   // check that these two
					   // normal vectors have
					   // length approximately 1
					   // and point radially
					   // outward
	  for (unsigned int i=0; i<2; ++i)
	    {
	      Point<2> radius = c1_values.quadrature_point(i);
  	      radius /= std::sqrt(radius.square());
	      deallog << "Normalized radius=" << radius << std::endl;

	      deallog << "C1 normal vector " << i << ": " << c1_values.normal_vector(i) << std::endl;
	      deallog << "Q3 normal vector " << i << ": " << q3_values.normal_vector(i) << std::endl;
	    };
	  

					   // some numerical checks for correctness
	  for (unsigned int i=0; i<2; ++i)
	    {
	      Assert (std::fabs(c1_values.normal_vector(i) *
				c1_values.normal_vector(i) - 1) < 1e-14,
		      ExcInternalError());
	      Point<2> radius = c1_values.quadrature_point(i);
	      radius /= std::sqrt(radius.square());

	      Assert ((radius-c1_values.normal_vector(i)).square() < 1e-14,
		      ExcInternalError());
	    };
	};  
}
