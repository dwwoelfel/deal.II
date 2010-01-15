//----------------------------------------------------------------------------
//    transfer.cc,v 1.13 2005/12/30 16:07:03 guido Exp
//    Version: 
//
//    Copyright (C) 2000 - 2007, 2009 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------------------------------------------------------

//TODO:[GK] Add checks for RT again!

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
#include <fe/fe_raviart_thomas.h>
#include <fe/fe_q.h>
#include <fe/fe_system.h>
#include <multigrid/mg_dof_handler.h>
#include <multigrid/mg_transfer.h>
#include <multigrid/mg_tools.h>
#include <multigrid/mg_level_object.h>

#include <fstream>
#include <iomanip>
#include <iomanip>
#include <algorithm>

using namespace std;

template <int dim>
void check_simple(const FiniteElement<dim>& fe)
{
  deallog << fe.get_name() << std::endl;
  
  Triangulation<dim> tr;
  GridGenerator::hyper_cube(tr);
  tr.refine_global(2);
  
  MGDoFHandler<dim> mgdof(tr);
  mgdof.distribute_dofs(fe);

  MGTransferPrebuilt<Vector<double> > transfer;
  transfer.build_matrices(mgdof);

  MGLevelObject<Vector<double> > u(0, tr.n_levels()-1);
  reinit_vector(mgdof, u);
				   // First prolongate the constant
				   // vector.  For Lagrange elements,
				   // the values are just the number
				   // of degrees of freedom.
  u[0] = 1;
  transfer.prolongate(1,u[1],u[0]);
  transfer.prolongate(2,u[2],u[1]);
  deallog << "u0\t" <<  (u[0]*u[0]+.5) << std::endl
	  << "u1\t" <<  (u[1]*u[1]+.5) << std::endl
	  << "u2\t" <<  (u[2]*u[2]+.5) << std::endl;
				   // Now restrict the same vectors.
  u[1] = 0.;
  u[0] = 0.;
  transfer.restrict_and_add(2,u[1],u[2]);
  transfer.restrict_and_add(1,u[0],u[1]);
  deallog << "u1\t" <<  (u[1]*u[1]+.5) << std::endl
	  << "u0\t" <<  (u[0]*u[0]+.5) << std::endl;
  
				   // Now the same for a non-constant
				   // vector
  for (unsigned int i=0;i<u[0].size();++i)
    u[0](i) = i;
  transfer.prolongate(1,u[1],u[0]);
  transfer.prolongate(2,u[2],u[1]);
  deallog << "u0\t" <<  (u[0]*u[0]+.5) << std::endl
	  << "u1\t" <<  (u[1]*u[1]+.5) << std::endl
	  << "u2\t" <<  (u[2]*u[2]+.5) << std::endl;
				   // Now restrict the same vectors.
  u[1] = 0.;
  u[0] = 0.;
  transfer.restrict_and_add(2,u[1],u[2]);
  transfer.restrict_and_add(1,u[0],u[1]);
  deallog << "u1\t" <<  (u[1]*u[1]+.5) << std::endl
	  << "u0\t" <<  (u[0]*u[0]+.5) << std::endl;

				   // Fill a global vector by counting
				   // from one up
  Vector<double> v;
  v.reinit (mgdof.n_dofs());
  for (unsigned int i=0;i<v.size();++i)
    v(i) = i+1;
  
  transfer.copy_to_mg(mgdof, u, v);
  for (unsigned int i=0; i<u[2].size();++i)
    deallog << ' ' << (int) u[2](i);
  deallog << std::endl;

				   // Now do the opposite: fill a
				   // multigrid vector counting the
				   // dofs and see where the numbers go
  v = 0.;
  for (unsigned int i=0;i<u[2].size();++i)
    u[2](i) = i+1;
  transfer.copy_from_mg(mgdof, v, u);
  for (unsigned int i=0; i<v.size();++i)
    deallog << ' ' << (int) v(i);
  deallog << std::endl;  
  v.equ(-1., v);
  transfer.copy_from_mg_add(mgdof, v, u);
  deallog << "diff " << v.l2_norm() << std::endl;  
}


int main()
{
  std::ofstream logfile("transfer/output");
  deallog << std::setprecision(4);
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);
  
  check_simple (FE_DGP<2>(0));
  check_simple (FE_DGP<2>(1));
  check_simple (FE_DGQ<2>(1));
  check_simple (FE_DGQ<2>(2));
  check_simple (FE_Q<2>(1));
  check_simple (FE_Q<2>(2));
  check_simple (FESystem<2>(FE_DGQ<2>(1), 2));
  check_simple (FESystem<2>(FE_DGP<2>(1), 2, FE_DGQ<2>(1), 3));

  check_simple (FE_RaviartThomasNodal<2>(1));
//  check_simple (FESystem<2>(FE_RaviartThomas<2>(1),1,FE_DGQ<2>(0),2));
  
  check_simple (FE_DGQ<3>(1));
  check_simple (FE_Q<3>(2));
}
