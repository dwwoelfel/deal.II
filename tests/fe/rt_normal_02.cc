//----------------------------  rt_normal_02.cc  ---------------------------
//    rt_normal_02.cc,v 1.3 2003/06/09 16:00:38 wolf Exp
//    Version: 
//
//    Copyright (C) 2003, 2005, 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  rt_normal_02.cc  ---------------------------

/*
 * Small test to analyse the equivalence of the normal component
 * on the element edges for the Raviart-Thomas elements.
 */



#include "../tests.h"
#include <base/logstream.h>

#define PRECISION 2

#include <fstream>

#include <grid/grid_generator.h>
#include <grid/grid_out.h>
#include <fe/mapping_q.h>
#include <fe/mapping_q1_eulerian.h>

#include <numerics/vectors.h>
#include <numerics/data_out.h>
#include <lac/vector.h>
#include <base/function.h>
#include <base/quadrature.h>
#include <base/quadrature_lib.h>
#include <dofs/dof_constraints.h>
#include <dofs/dof_tools.h>

#include <fe/fe_system.h>
#include <fe/fe.h>
#include <fe/fe_q.h>
#include <fe/fe_values.h>
#include <fe/fe_raviart_thomas.h>
#include <fe/fe_dgq.h>

std::ofstream logfile ("rt_normal_02/output");


/*
 * Check if the normal component is continuous over element edges.
 */

void EvaluateNormal2 (DoFHandler<2> *dof_handler,
		      Vector<double> &solution)
{
    // This quadrature rule determines the points, where the
    // continuity will be tested.
    QGauss<1> quad (6);
    Quadrature<2> qproject = QProjector<2>::project_to_all_faces (quad);

    FEFaceValues<2> fe_v_face (dof_handler->get_fe (), quad, 
			       UpdateFlags(update_values    |
					   update_q_points  |
					   update_gradients |
					   update_normal_vectors |
					   update_JxW_values));

    FEValues<2> fe_v (dof_handler->get_fe (), qproject, 
		      UpdateFlags(update_values    |
				  update_q_points  |
				  update_gradients |
				  update_JxW_values));

    FEValues<2> fe_v_n (dof_handler->get_fe (), qproject, 
			UpdateFlags(update_values    |
				    update_q_points  |
				    update_gradients |
				    update_JxW_values));

    const unsigned int   n_q_face    = quad.n_quadrature_points;
    const unsigned int   n_q_proj    = qproject.n_quadrature_points;
    const unsigned int   n_components   = dof_handler->get_fe().n_components();
    const unsigned int   dofs_per_cell = dof_handler->get_fe().dofs_per_cell;

    deallog << "Quad Points Face " << n_q_face
	    << ", Quad Points Proj. " << n_q_proj
	    << std::endl;

    // Cell iterators
    DoFHandler<2>::active_cell_iterator cell = dof_handler->begin_active(),
	endc = dof_handler->end();

    std::vector<unsigned int> local_dof_indices (dofs_per_cell);

    for (; cell!=endc; ++cell)
    {
	cell->get_dof_indices (local_dof_indices);
	fe_v.reinit (cell);
	
	for (unsigned int f = 0; f < GeometryInfo<2>::faces_per_cell; ++f)
	{
	    if (!cell->face(f)->at_boundary ())
	    {
		const QProjector<2>::DataSetDescriptor offset
		    = (QProjector<2>::DataSetDescriptor::
		       face (f,
			     cell->face_orientation(f),
			     cell->face_flip(f),
			     cell->face_rotation(f),
			     quad.n_quadrature_points));
		fe_v_face.reinit (cell, f);
		
		DoFHandler<2>::active_cell_iterator cell_n = cell->neighbor (f);
		const unsigned int neighbor=cell->neighbor_of_neighbor(f);
		fe_v_n.reinit (cell_n); 

		const QProjector<2>::DataSetDescriptor offset_n
		    = (QProjector<2>::DataSetDescriptor::
		       face (neighbor,
			     cell_n->face_orientation(neighbor),
			     cell_n->face_flip(neighbor),
			     cell_n->face_rotation(neighbor),
			     quad.n_quadrature_points));

		// Get values from solution vector (For Trap.Rule)
		std::vector<Vector<double> > this_value
		    (n_q_proj, Vector<double>(n_components));
		fe_v.get_function_values (solution, this_value);

		// Same for neighbor cell
		std::vector<Vector<double> > this_value_n
		    (n_q_proj, Vector<double>(n_components));
		fe_v_n.get_function_values (solution, this_value_n);

		for (unsigned int q_point=0; q_point<n_q_face; ++q_point)
		{
		    Point<2> vn = fe_v_face.normal_vector (q_point);
		    double nx = vn(0);
		    double ny = vn(1);
		    
		    double u = this_value[q_point + offset](0);
		    double v = this_value[q_point + offset](1);

		    double u_n = this_value_n[q_point + offset_n](0);
		    double v_n = this_value_n[q_point + offset_n](1);
		    double un1 = u * nx + v * ny,
			un2 = u_n * nx + v_n * ny;

		    deallog << "QP " << q_point
			    << ", Error: "
			    << (u-u_n) * nx + (v-v_n) * ny
			    << ", u " << un1
			    << ", un " << un2
			    << ", Rat " << un2 / un1
			    << std::endl;

		    Assert (std::fabs((u-u_n) * nx + (v-v_n) * ny) < 1e-12,
			    ExcInternalError());
		}
	    }
	}
    }
}



/*
 * Check if the normal component is continuous over element edges.
 */

void EvaluateNormal (DoFHandler<2> *dof_handler,
		     Vector<double> &solution)
{
    // This quadrature rule determines the points, where the
    // continuity will be tested.
    QGauss<1> quad (6);
    FEFaceValues<2> fe_v_face (dof_handler->get_fe (), quad, 
			       UpdateFlags(update_values    |
					   update_q_points  |
					   update_gradients |
					   update_normal_vectors |
					   update_JxW_values));

    FEFaceValues<2> fe_v_face_n (dof_handler->get_fe (), quad, 
				 UpdateFlags(update_values    |
					     update_q_points  |
					     update_gradients |
					     update_normal_vectors |
					     update_JxW_values));

    const unsigned int   n_q_face    = quad.n_quadrature_points;
    const unsigned int   n_components   = dof_handler->get_fe().n_components();
    const unsigned int   dofs_per_cell = dof_handler->get_fe().dofs_per_cell;

    // Cell iterators
    DoFHandler<2>::active_cell_iterator cell = dof_handler->begin_active(),
	endc = dof_handler->end();

    std::vector<unsigned int> local_dof_indices (dofs_per_cell);

    for (; cell!=endc; ++cell)
    {
	cell->get_dof_indices (local_dof_indices);

	for (unsigned int f = 0; f < GeometryInfo<2>::faces_per_cell; ++f)
	{
	    if (!cell->face(f)->at_boundary ())
	    {
		fe_v_face.reinit (cell, f);
		
		const unsigned int neighbor=cell->neighbor_of_neighbor(f);
		fe_v_face_n.reinit (cell->neighbor (f), neighbor); 

		// Get values from solution vector (For Trap.Rule)
		std::vector<Vector<double> > this_value
		    (n_q_face, Vector<double>(n_components));
		fe_v_face.get_function_values (solution, this_value);

		// Same for neighbor cell
		std::vector<Vector<double> > this_value_n
		    (n_q_face, Vector<double>(n_components));
		fe_v_face_n.get_function_values (solution, this_value_n);

		for (unsigned int q_point=0; q_point<n_q_face; ++q_point)
		{
		    Point<2> vn = fe_v_face.normal_vector (q_point);
		    double nx = vn(0);
		    double ny = vn(1);
		    
		    double u = this_value[q_point](0);
		    double v = this_value[q_point](1);

		    double u_n = this_value_n[q_point](0);
		    double v_n = this_value_n[q_point](1);
		    double un1 = u * nx + v * ny,
			un2 = u_n * nx + v_n * ny;

		    deallog << "QP " << q_point
			    << ", Error: "
			    << (u-u_n) * nx + (v-v_n) * ny
			    << ", u " << un1
			    << ", un " << un2
			    << ", Rat " << un2 / un1
			    << std::endl;

		    Assert (std::fabs((u-u_n) * nx + (v-v_n) * ny) < 1e-12,
			    ExcInternalError());
		}
	    }
	}
    }
}


int main (int /*argc*/, char **/*argv*/)
{
  deallog << std::setprecision(PRECISION);
  deallog << std::fixed;  
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  Triangulation<2> tria_test;
  DoFHandler<2> *dof_handler;
  Point<2> p1 (0,0),
    p2 (1, 1);
  std::vector<unsigned int> sub_div;

  sub_div.push_back (1);
  sub_div.push_back (4);

  GridGenerator::subdivided_hyper_rectangle (tria_test, sub_div, p1, p2);
  tria_test.refine_global (2);
  tria_test.distort_random (0.05);

  // Create a DoFHandler
  FE_RaviartThomas<2> fe (1);
  dof_handler = new DoFHandler<2> (tria_test);
  dof_handler->distribute_dofs (fe);

  // Alloc some DoFs
  Vector<double> solution;
  solution.reinit (dof_handler->n_dofs ());

  // Fill solution vector with random values between 0 and 1.
  for (unsigned int i = 0; i < dof_handler->n_dofs (); ++i)
      solution(i) = (double) rand () / (double) RAND_MAX;

  // Now check if the function is continuous in normal
  // direction.
  EvaluateNormal2 (dof_handler, solution);

  delete (dof_handler);
  return (0);
}
