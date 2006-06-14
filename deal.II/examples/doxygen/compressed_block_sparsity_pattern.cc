//---------------------------------------------------------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------

// See documentation of CompressedBlockSparsityPattern for documentation of this example

#include <lac/block_sparsity_pattern.h>
#include <grid/tria.h>
#include <grid/tria_accessor.h>
#include <grid/tria_iterator.h>
#include <grid/grid_generator.h>
#include <fe/fe_q.h>
#include <fe/fe_system.h>
#include <dofs/dof_handler.h>
#include <dofs/dof_constraints.h>
#include <dofs/dof_tools.h>

#include <iostream>

int main()
{
  Triangulation<2> tr;
  GridGenerator::subdivided_hyper_cube(tr, 3);
  tr.begin_active()->set_refine_flag();
  tr.execute_coarsening_and_refinement();

  FE_Q<2> fe1(1);
  FE_Q<2> fe2(2);
  FESystem<2> fe(fe1, 2, fe2, 1);
  
  DoFHandler<2> dof(tr);
  dof.distribute_dofs(fe);
  ConstraintMatrix constraints;
  DoFTools::make_hanging_node_constraints(dof, constraints);
  constraints.close();
  
  std::vector<unsigned int> dofs_per_block(fe.n_blocks());
  DoFTools::count_dofs_per_block(dof, dofs_per_block);
  
  CompressedBlockSparsityPattern c_sparsity(fe.n_blocks(), fe.n_blocks());
  for (unsigned int i=0;i<fe.n_blocks();++i)
    for (unsigned int j=0;j<fe.n_blocks();++j)
      c_sparsity.block(i,j).reinit(dofs_per_block[i],dofs_per_block[j]);
  c_sparsity.collect_sizes();

  DoFTools::make_sparsity_pattern(dof, c_sparsity);
  constraints.condense(c_sparsity);

  BlockSparsityPattern sparsity;
  sparsity.copy_from(c_sparsity);
  
  for (unsigned int i=0;i<fe.n_blocks();++i)
    for (unsigned int j=0;j<fe.n_blocks();++j)
      {
	std::cout << "   Block " << i << ' ' << j << std::endl;
	sparsity.block(i,j).print(std::cout);
      }
}
