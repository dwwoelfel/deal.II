//----------------------------  have_same_coarse_mesh_02.cc  ---------------------------
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
//----------------------------  have_same_coarse_mesh_02.cc  ---------------------------
// check GridTools::have_same_coarse_mesh for DoFHandler arguments


#include "../tests.h"
#include <grid/tria.h>
#include <grid/grid_generator.h>
#include <grid/grid_tools.h>
#include <dofs/dof_handler.h>
#include <hp/dof_handler.h>
#include <multigrid/mg_dof_handler.h>

#include <fstream>


template<int dim>
void test()
{
                                   // create 3 triangulations
  Triangulation<dim> tria[3];

  GridGenerator::hyper_cube (tria[0]);
  tria[0].refine_global (1);
  
  GridGenerator::hyper_cube (tria[1]);
  GridTools::scale (2, tria[1]);
  tria[1].refine_global (2);

  if (dim != 1)
    GridGenerator::hyper_ball (tria[2]);
  else
    {
      GridGenerator::hyper_cube (tria[2]);
      GridTools::shift (Point<dim>(2.), tria[2]);
    }
      
  tria[2].refine_global (3);

  DoFHandler<dim> dh0 (tria[0]);
  DoFHandler<dim> dh1 (tria[1]);
  DoFHandler<dim> dh2 (tria[2]);
  
  DoFHandler<dim> *dof_handler[3] = { &dh0, &dh1, &dh2 };
  
  for (unsigned int i=0; i<3; ++i)
    for (unsigned int j=0; j<3; ++j)
      {
        Assert (GridTools::have_same_coarse_mesh (*dof_handler[i], *dof_handler[j])
                ==
                (i == j),
                ExcInternalError());
        
        deallog << "meshes " << i << " and " << j << ": "
                << (GridTools::have_same_coarse_mesh (*dof_handler[i], *dof_handler[j])
                    ?
                    "true"
                    :
                    "false")
                << std::endl;
      }
}


int main()
{
  std::ofstream logfile ("have_same_coarse_mesh_02/output");
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  test<1>();
  test<2>();
  test<3>();
}

