//----------------------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 2000 - 2007 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------------------------------------------------------


#include "../tests.h"
#include <base/logstream.h>
#include <lac/vector.h>
#include <lac/block_vector.h>
#include <grid/tria.h>
#include <grid/grid_generator.h>
#include <dofs/dof_renumbering.h>
#include <dofs/dof_tools.h>
#include <fe/fe_dgp.h>
#include <fe/fe_dgq.h>
#include <fe/fe_q.h>
#include <fe/fe_raviart_thomas.h>
#include <fe/fe_system.h>
#include <multigrid/mg_dof_handler.h>
#include <multigrid/mg_transfer_block.h>
#include <multigrid/mg_tools.h>
#include <multigrid/mg_level_object.h>

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace std;

template <int dim>
void check_block(const FiniteElement<dim>& fe,
		 const vector<bool>& selected,
		 const vector<double>& factors)
{
  deallog << fe.get_name() << std::endl << "selected ";
  for (unsigned int i=0;i<selected.size();++i)
    if (selected[i])
      deallog << ' ' << i;
  deallog << std::endl;
  
  Triangulation<dim> tr;
  GridGenerator::hyper_cube(tr);
  tr.refine_global(2);
  
  MGDoFHandler<dim> mgdof(tr);
  DoFHandler<dim>& dof=mgdof;
  mgdof.distribute_dofs(fe);
  DoFRenumbering::component_wise(mgdof);
  vector<unsigned int> ndofs(fe.n_blocks());
  DoFTools::count_dofs_per_block(mgdof, ndofs);
  
  for (unsigned int l=0;l<tr.n_levels();++l)
    DoFRenumbering::component_wise(mgdof, l);
  std::vector<std::vector<unsigned int> > mg_ndofs(mgdof.get_tria().n_levels());
  MGTools::count_dofs_per_block(mgdof, mg_ndofs);

  deallog << "Global  dofs:";
  for (unsigned int i=0;i<ndofs.size();++i)
    deallog << ' ' << ndofs[i];
  deallog << std::endl;
  for (unsigned int l=0;l<mg_ndofs.size();++l)
    {
      deallog << "Level " << l << " dofs:";
      for (unsigned int i=0;i<mg_ndofs[l].size();++i)
	deallog << ' ' << mg_ndofs[l][i];
      deallog << std::endl;
    }  
  
  PrimitiveVectorMemory<Vector<double> > mem;
  MGTransferBlock<double> transfer;
  transfer.build_matrices(dof, mgdof, selected);
  if (factors.size()>0)
    transfer.initialize(factors, mem);

  BlockVector<double> u2(mg_ndofs[2]);
  BlockVector<double> u1(mg_ndofs[1]);
  BlockVector<double> u0(mg_ndofs[0]);

				   // Prolongate a constant function
				   // twice
  u0 = 1;
  transfer.prolongate(1,u1,u0);
  transfer.prolongate(2,u2,u1);
				   // These outputs are just the
				   // number of dofs on each level
  deallog << "u0";
  for (unsigned int b=0;b<u0.n_blocks();++b)
    deallog << '\t' << (int) (u0.block(b)*u0.block(b)+.4);
  deallog << std::endl << "u1";
  for (unsigned int b=0;b<u1.n_blocks();++b)
    deallog << '\t' << (int) (u1.block(b)*u1.block(b)+.4);
  deallog << std::endl << "u2";
  for (unsigned int b=0;b<u2.n_blocks();++b)
    deallog << '\t' << (int) (u2.block(b)*u2.block(b)+.4);
  deallog << std::endl;
  
  u1 = 0.;
  u0 = 0.;
  transfer.restrict_and_add(2,u1,u2);
  transfer.restrict_and_add(1,u0,u1);
				   // After adding the restrictions,
				   // things get bigger.
  deallog << "u1";
  for (unsigned int b=0;b<u1.n_blocks();++b)
    deallog << '\t' << (int) (u1.block(b)*u1.block(b)+.5);
  deallog << std::endl << "u0";
  for (unsigned int b=0;b<u0.n_blocks();++b)
    deallog << '\t' << (int) (u0.block(b)*u0.block(b)+.5);
  deallog << std::endl;
  				   // Check copy to mg and back
  BlockVector<double> u;
  u.reinit (ndofs);
  for (unsigned int i=0;i<u.size();++i)
    u(i) = i+1;

  std::vector<std::vector<unsigned int> > cached_sizes;
  MGLevelObject<BlockVector<double> > v;
  v.resize(0, tr.n_levels()-1);
  MGTools::reinit_vector_by_blocks(mgdof, v, selected, cached_sizes);
  
  transfer.copy_to_mg(mgdof, v, u);
  for (unsigned int i=0; i<v[2].size();++i)
    deallog << ' ' << (int) v[2](i);
  deallog << std::endl;

				   // Now do the opposite: fill a
				   // multigrid vector counting the
				   // dofs and see where the numbers go
  u = 0.;
  for (unsigned int i=0;i<v[2].size();++i)
    v[2](i) = i+1;
  transfer.copy_from_mg_add(mgdof, u, v);
  for (unsigned int i=0; i<u.size();++i)
    deallog << ' ' << (int) u(i);
  deallog << std::endl;
  
}


int main()
{
  std::ofstream logfile("transfer_block/output");
  logfile.precision(3);
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);
  
  std::vector<double> factors;

  FE_DGQ<2> q0(0);
  FE_DGQ<2> q1(1);
  FE_RaviartThomasNodal<2> rt0(0);
  FE_RaviartThomasNodal<2> rt1(1);
  
  FESystem<2> fe0(rt1, 1, q1, 1);
  FESystem<2> fe1(rt0, 2, q0, 2);
  
  vector<bool> s1(2, true);
  deallog << "All" << std::endl;
  check_block(fe0, s1, factors);
 
  s1[1] = false;
  deallog << "Velocity" << std::endl;
  check_block(fe0, s1, factors);
  
  s1[1] = true;
  s1[0] = false;
  deallog << "Pressure" << std::endl;
  check_block(fe0, s1, factors);

  s1.resize(4,true);
  s1[0] = false;
  s1[2] = false;
  check_block(fe1, s1, factors);
}
