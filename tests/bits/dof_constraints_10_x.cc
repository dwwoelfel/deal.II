//----------------------------  dof_constraints_10.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2004, 2005, 2007 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  dof_constraints_10.cc  ---------------------------


// simply check what happens when condensing compressed set sparsity
// patterns. This test was written when I changed a few things in the
// algorithm.

#include "../tests.h"
#include <lac/compressed_set_sparsity_pattern.h>
#include <lac/sparsity_pattern.h>
#include <grid/tria.h>
#include <grid/tria_accessor.h>
#include <grid/tria_iterator.h>
#include <grid/grid_tools.h>
#include <grid/grid_generator.h>
#include <dofs/dof_handler.h>
#include <dofs/dof_accessor.h>
#include <dofs/dof_tools.h>
#include <lac/constraint_matrix.h>
#include <fe/fe_q.h>
#include <fstream>
#include <iomanip>


template <int dim>
void test ()
{
  deallog << dim << "D" << std::endl;
  
  Triangulation<dim> triangulation;
  GridGenerator::hyper_cube (triangulation);

                                   // refine once, then refine first cell to
                                   // create hanging nodes
  triangulation.refine_global (1);
  triangulation.begin_active()->set_refine_flag ();
  triangulation.execute_coarsening_and_refinement ();
  deallog << "Number of cells: " << triangulation.n_active_cells() << std::endl;
  
                                   // set up a DoFHandler and compute hanging
                                   // node constraints for a Q2 element
  FE_Q<dim> fe(2);
  DoFHandler<dim> dof_handler (triangulation);
  dof_handler.distribute_dofs (fe);
  deallog << "Number of dofs: " << dof_handler.n_dofs() << std::endl;

  ConstraintMatrix constraints;
  DoFTools::make_hanging_node_constraints (dof_handler, constraints);
  constraints.close ();
  deallog << "Number of constraints: " << constraints.n_constraints() << std::endl;

                                   // then set up a sparsity pattern and a
                                   // matrix on top of it
  CompressedSetSparsityPattern sparsity (dof_handler.n_dofs(),
                                      dof_handler.n_dofs());
  DoFTools::make_sparsity_pattern (dof_handler, sparsity);
  constraints.condense (sparsity);

                                   // and output what we have
  SparsityPattern A;
  A.copy_from (sparsity);
  for (SparsityPattern::const_iterator i=A.begin(); i!=A.end(); ++i)
    deallog << i->row() << ' ' << i->column()
            << std::endl;
}



int main ()
{
  std::ofstream logfile("dof_constraints_10_x/output");
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  try
    {
      test<1> ();
      test<2> ();
      test<3> ();
    }
  catch (std::exception &exc)
    {
      deallog << std::endl << std::endl
		<< "----------------------------------------------------"
		<< std::endl;
      deallog << "Exception on processing: " << std::endl
		<< exc.what() << std::endl
		<< "Aborting!" << std::endl
		<< "----------------------------------------------------"
		<< std::endl;
      
      return 1;
    }
  catch (...) 
    {
      deallog << std::endl << std::endl
		<< "----------------------------------------------------"
		<< std::endl;
      deallog << "Unknown exception!" << std::endl
		<< "Aborting!" << std::endl
		<< "----------------------------------------------------"
		<< std::endl;
      return 1;
    };
}
