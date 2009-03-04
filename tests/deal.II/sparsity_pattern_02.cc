//----------------------------  sparsity_pattern_02.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2000, 2001, 2003, 2004, 2007 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  sparsity_pattern_02.cc  ---------------------------


// use the version of DoFTools::make_sparsity_pattern that takes two
// DoFHandler arguments for two DoFHandlers that are actually from different
// meshes (though with the same base)


#include "../tests.h"
#include <base/logstream.h>
#include <lac/sparsity_pattern.h>
#include <lac/block_sparsity_pattern.h>
#include <lac/compressed_sparsity_pattern.h>
#include <grid/tria.h>
#include <grid/tria_iterator.h>
#include <grid/tria_accessor.h>
#include <grid/grid_generator.h>
#include <dofs/dof_handler.h>
#include <dofs/dof_tools.h>
#include <lac/constraint_matrix.h>
#include <fe/fe_q.h>
#include <fe/fe_system.h>

#include <fstream>




template <int dim>
void
check ()
{
				   // create two different triangulations
  Triangulation<dim> triangulation_1;  
  if (dim==2)
    GridGenerator::hyper_ball(triangulation_1, Point<dim>(), 1);
  else
    GridGenerator::hyper_cube(triangulation_1, -1,1);
  triangulation_1.refine_global (1);
  triangulation_1.begin_active()->set_refine_flag ();
  triangulation_1.execute_coarsening_and_refinement ();
  triangulation_1.begin_active(2)->set_refine_flag ();
  triangulation_1.execute_coarsening_and_refinement ();
  if (dim==1)
    triangulation_1.refine_global(2);


  Triangulation<dim> triangulation_2;  
  if (dim==2)
    GridGenerator::hyper_ball(triangulation_2, Point<dim>(), 1);
  else
    GridGenerator::hyper_cube(triangulation_2, -1,1);
  triangulation_2.refine_global (1);
  (++triangulation_2.begin_active())->set_refine_flag ();
  triangulation_2.execute_coarsening_and_refinement ();
  (++triangulation_2.begin_active(2))->set_refine_flag ();
  triangulation_2.execute_coarsening_and_refinement ();
  if (dim==1)
    triangulation_2.refine_global(2);


  
				   // create a system element composed
				   // of one Q1 and one Q2 element
  FESystem<dim> element(FE_Q<dim>(1), 1,
			FE_Q<dim>(2), 1);
  DoFHandler<dim> dof_1(triangulation_1);
  DoFHandler<dim> dof_2(triangulation_2);
  dof_1.distribute_dofs(element);
  dof_2.distribute_dofs(element);

  SparsityPattern sparsity (dof_1.n_dofs(), dof_2.n_dofs(),
			    std::max(dof_1.n_dofs(), dof_2.n_dofs()));
  DoFTools::make_sparsity_pattern (dof_1, dof_2, sparsity);
  sparsity.compress ();

  sparsity.print (deallog.get_file_stream());
}



int main ()
{
  std::ofstream logfile ("sparsity_pattern_02/output");
  deallog << std::setprecision (2);
  deallog << std::fixed;  
  deallog.attach(logfile);
  deallog.depth_console (0);

  deallog.push ("1d");
  check<1> ();
  deallog.pop ();
  deallog.push ("2d");
  check<2> ();
  deallog.pop ();
  deallog.push ("3d");
  check<3> ();
  deallog.pop ();
}
