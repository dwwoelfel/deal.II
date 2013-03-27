//---------------------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 2008, 2009, 2010 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------


// test distributed solution_transfer with one processor

#include "../tests.h"
#include "coarse_grid_common.h"
#include <deal.II/base/logstream.h>
#include <deal.II/base/tensor.h>
#include <deal.II/grid/tria.h>
#include <deal.II/distributed/tria.h>
#include <deal.II/distributed/solution_transfer.h>
#include <deal.II/grid/tria_accessor.h>
#include <deal.II/grid/tria_iterator.h>
#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/grid_out.h>
#include <deal.II/grid/grid_in.h>
#include <deal.II/grid/intergrid_map.h>

#include <deal.II/lac/petsc_vector.h>
#include <deal.II/fe/fe_q.h>

#include <fstream>
#include <cstdlib>


template<int dim>
void test(std::ostream& /*out*/)
{
  parallel::distributed::Triangulation<dim> tr(MPI_COMM_WORLD);

  GridGenerator::hyper_cube(tr);
  tr.refine_global (1);
  DoFHandler<dim> dofh(tr);
  static const FE_Q<dim> fe(2);
  dofh.distribute_dofs (fe);

  parallel::distributed::SolutionTransfer<dim, Vector<double> > soltrans(dofh);

  for (typename Triangulation<dim>::active_cell_iterator
	     cell = tr.begin_active();
	   cell != tr.end(); ++cell)
    {
      cell->set_refine_flag();
    }

  tr.prepare_coarsening_and_refinement();

  Vector<double> solution(dofh.n_dofs());

  soltrans.prepare_for_coarsening_and_refinement(solution);

  tr.execute_coarsening_and_refinement ();

  dofh.distribute_dofs (fe);

  Vector<double> interpolated_solution(dofh.n_dofs());
  soltrans.interpolate(interpolated_solution);

  deallog << "norm: " << interpolated_solution.l2_norm() << std::endl;

}


int main(int argc, char *argv[])
{
#ifdef DEAL_II_WITH_MPI
  Utilities::MPI::MPI_InitFinalize mpi_initialization(argc, argv, 1);
#else
  (void)argc;
  (void)argv;
#endif

  std::ofstream logfile("solution_transfer_01/output");
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  deallog.push("2d");
  test<2>(logfile);
  deallog.pop();
  deallog.push("3d");
  test<3>(logfile);
  deallog.pop();


}
