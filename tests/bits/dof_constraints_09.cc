//----------------------------  dof_constraints_09.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2004, 2005 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  dof_constraints_09.cc  ---------------------------


// simply check what happens when condensing block matrices. This test was
// written when I changed a few things in the algorithm. By simply looping
// over all entries of the sparse matrix, we also check that things went right
// during compression of the sparsity pattern.

#include "../tests.h"
#include <lac/block_sparsity_pattern.h>
#include <lac/block_sparse_matrix.h>
#include <lac/block_vector.h>
#include <grid/tria.h>
#include <grid/tria_accessor.h>
#include <grid/tria_iterator.h>
#include <grid/grid_tools.h>
#include <grid/grid_generator.h>
#include <dofs/dof_handler.h>
#include <dofs/dof_accessor.h>
#include <dofs/dof_tools.h>
#include <dofs/dof_constraints.h>
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
  std::vector<unsigned int> block_sizes(2);
  block_sizes[0] = dof_handler.n_dofs()/3;
  block_sizes[1] = dof_handler.n_dofs() - block_sizes[0];

  BlockSparsityPattern sparsity(2,2);
  for (unsigned int i=0; i<2; ++i)
    for (unsigned int j=0; j<2; ++j)
      sparsity.block(i,j).reinit (block_sizes[i], block_sizes[j],
                                  dof_handler.max_couplings_between_dofs());
  sparsity.collect_sizes();
  
  DoFTools::make_sparsity_pattern (dof_handler, sparsity);
  constraints.condense (sparsity);
  BlockSparseMatrix<double> A(sparsity);

                                   // then fill the matrix by setting up
                                   // bogus matrix entries
  std::vector<unsigned int> local_dofs (fe.dofs_per_cell);
  FullMatrix<double> local_matrix (fe.dofs_per_cell, fe.dofs_per_cell);
  for (typename DoFHandler<dim>::active_cell_iterator
         cell = dof_handler.begin_active();
       cell != dof_handler.end(); ++cell)
    {
      cell->get_dof_indices (local_dofs);
      local_matrix = 0;
      for (unsigned int i=0; i<fe.dofs_per_cell; ++i)
        for (unsigned int j=0; j<fe.dofs_per_cell; ++j)
          local_matrix(i,j) = (i+1.)*(j+1.)*(local_dofs[i]+1.)*(local_dofs[j]+1.);

                                       // copy local to global
      for (unsigned int i=0; i<fe.dofs_per_cell; ++i)
        for (unsigned int j=0; j<fe.dofs_per_cell; ++j)
          A.add (local_dofs[i], local_dofs[j], local_matrix(i,j));
    }

                                   // now condense away constraints from A
  constraints.condense (A);

                                   // and output what we have
  for (BlockSparseMatrix<double>::const_iterator i=A.begin(); i!=A.end(); ++i)
    deallog << i->block_row() << ' ' << i->block_column() << ' '
            << i->row() << ' ' << i->column() << ' ' << i->value()
            << std::endl;
}



int main ()
{
  std::ofstream logfile("dof_constraints_09/output");
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
