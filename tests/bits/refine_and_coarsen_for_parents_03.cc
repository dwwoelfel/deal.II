//----------------------------  refine_and_coarsen_3d.cc  ---------------------------
//    $Id: refine_and_coarsen_3d.cc 15183 2007-09-10 01:10:03Z bangerth $
//    Version: $Name$ 
//
//    Copyright (C) 2010 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//-----------------------  refine_and_coarsen_for_parents_03.cc  ----------------------


// check that, if we take an locally refined mesh, refine it globally once,
// then coarsen it globally again, the parent relation holds

#include "../tests.h"

#include <base/geometry_info.h>
#include <base/logstream.h>
#include <base/quadrature_lib.h>
#include <multigrid/mg_dof_handler.h>
#include <multigrid/mg_dof_accessor.h>
#include <grid/tria.h>
#include <grid/tria_accessor.h>
#include <grid/grid_generator.h>

#include <fstream>


void do_refine (Triangulation<1> &tria)
{
  const int dim = 1;
  
  tria.refine_global (2);
  tria.begin_active()->set_refine_flag();
  tria.execute_coarsening_and_refinement ();
}


void do_refine (Triangulation<2> &tria)
{
  const int dim = 2;
  
  tria.refine_global (2);
  tria.begin_active()->set_refine_flag();
  tria.execute_coarsening_and_refinement ();
  tria.begin_active ()->set_refine_flag (RefinementPossibilities<dim>::cut_x);
  tria.execute_coarsening_and_refinement ();
  tria.begin_active ()->set_refine_flag (RefinementPossibilities<dim>::cut_y);
  tria.execute_coarsening_and_refinement ();
}


void do_refine (Triangulation<3> &tria)
{
  const int dim = 3;
  
  tria.refine_global (2);
  tria.begin_active()->set_refine_flag();
  tria.execute_coarsening_and_refinement ();
  tria.begin_active()->set_refine_flag(RefinementPossibilities<dim>::cut_x);
  tria.execute_coarsening_and_refinement ();
  tria.begin_active()->set_refine_flag(RefinementPossibilities<dim>::cut_y);
  tria.execute_coarsening_and_refinement ();
  tria.begin_active()->set_refine_flag(RefinementPossibilities<dim>::cut_z);
  tria.execute_coarsening_and_refinement ();
  tria.begin_active()->set_refine_flag(RefinementPossibilities<dim>::cut_xy);
  tria.execute_coarsening_and_refinement ();
  tria.begin_active()->set_refine_flag(RefinementPossibilities<dim>::cut_xz);
  tria.execute_coarsening_and_refinement ();
  tria.begin_active()->set_refine_flag(RefinementPossibilities<dim>::cut_yz);
  tria.execute_coarsening_and_refinement ();
}


template <int dim>
void check ()
{
  Triangulation<dim> tria;
  GridGenerator::hyper_cube (tria);
  do_refine (tria);
				   // refine the mesh globally and
				   // verify that the parent relation
				   // holds
  tria.refine_global (1);
  
  MGDoFHandler<dim> dof_handler (triangulation);

  for (typename MGDoFHandler<dim>::cell_iterator cell = dof_handler.begin();
       cell != dof_handler.end (); ++cell)
    for (unsigned int child = 0; child < cell->n_children (); ++child)
      Assert (cell->child (child)->parent () == cell,
              ExcInternalError ());
  
				   // coarsen the mesh globally and
				   // verify that the parent relation
				   // holds
  for (typename Triangulation<dim>::active_cell_iterator cell = tria.begin_active ();
       cell != tria.end (); ++cell)
    cell->set_coarsen_flag ();
  
  tria.execute_coarsening_and_refinement ();
  
  for (typename MGDoFHandler<dim>::cell_iterator cell = dof_handler.begin ();
       cell != dof_handler.end(); ++cell)
    for (unsigned int child = 0; child < cell->n_children (); ++child)
      Assert (cell->child (child)->parent () == cell,
	          ExcInternalError());
  
  deallog << "OK for " << dim << "d" << std::endl;
}


int main () 
{
  std::ofstream logfile("refine_and_coarsen_for_parents_03/output");
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  check<1> ();
  check<2> ();
  check<3> ();
}

  
  
