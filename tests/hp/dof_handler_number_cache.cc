//---------------------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 2008, 2010 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------


// Check the consistency of the number cache of DoFHandler for a sequential
// object. Like deal.II/dof_handler_number_cache but for an hp object


#include "../tests.h"
#include <base/logstream.h>
#include <base/tensor.h>
#include <grid/tria.h>
#include <grid/tria_accessor.h>
#include <grid/tria_iterator.h>
#include <grid/grid_generator.h>
#include <grid/intergrid_map.h>
#include <base/utilities.h>
#include <hp/dof_handler.h>
#include <fe/fe_system.h>
#include <fe/fe_q.h>
#include <fe/fe_dgq.h>
#include <hp/fe_collection.h>

#include <fstream>
#include <cstdlib>


template<int dim>
void test()
{
  Triangulation<dim> triangulation (Triangulation<dim>::limit_level_difference_at_vertices);

  hp::FECollection<dim> fe;
  for (unsigned int i=0; i<4; ++i)
    fe.push_back (FESystem<dim>  (FE_Q<dim>(i+1),2,
				  FE_DGQ<dim>(i),1));

  hp::DoFHandler<dim> dof_handler (triangulation);

  GridGenerator::hyper_cube(triangulation);
  triangulation.refine_global (2);

  const unsigned int n_refinements[] = { 0, 4, 3, 2 };
  for (unsigned int i=0; i<n_refinements[dim]; ++i)
    {
				       // refine one-fifth of cells randomly
      std::vector<bool> flags (triangulation.n_active_cells(), false);
      for (unsigned int k=0; k<flags.size()/5 + 1; ++k)
	flags[rand() % flags.size()] = true;
				       // make sure there's at least one that
				       // will be refined
      flags[0] = true;

				       // refine triangulation
      unsigned int index=0;
      for (typename Triangulation<dim>::active_cell_iterator
	     cell = triangulation.begin_active();
	   cell != triangulation.end(); ++cell, ++index)
	if (flags[index])
	  cell->set_refine_flag();
      Assert (index == triangulation.n_active_cells(), ExcInternalError());

				       // flag all other cells for coarsening
				       // (this should ensure that at least
				       // some of them will actually be
				       // coarsened)
      index=0;
      for (typename Triangulation<dim>::active_cell_iterator
	     cell = triangulation.begin_active();
	   cell != triangulation.end(); ++cell, ++index)
	if (!flags[index])
	  cell->set_coarsen_flag();

      triangulation.execute_coarsening_and_refinement ();

      index=0;
      for (typename hp::DoFHandler<dim>::active_cell_iterator
	     cell = dof_handler.begin_active();
	   cell != dof_handler.end(); ++cell, ++index)
	cell->set_active_fe_index (index % fe.size());
      
      dof_handler.distribute_dofs (fe);

      const unsigned int N = dof_handler.n_dofs();
      deallog << N << std::endl;

      IndexSet all (N);
      all.add_range (0, N);

      Assert (dof_handler.n_locally_owned_dofs() == N,
	      ExcInternalError());
      Assert (dof_handler.locally_owned_dofs() == all,
	      ExcInternalError());
      Assert (dof_handler.n_locally_owned_dofs_per_processor() ==
	      std::vector<unsigned int> (1,N),
	      ExcInternalError());
      Assert (dof_handler.locally_owned_dofs_per_processor() ==
	      std::vector<IndexSet>(1,all),
	      ExcInternalError());
    }
}


int main()
{
  std::ofstream logfile("dof_handler_number_cache/output");
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  deallog.push("1d");
  test<1>();
  deallog.pop();

  deallog.push("2d");
  test<2>();
  deallog.pop();

  deallog.push("3d");
  test<3>();
  deallog.pop();
}
