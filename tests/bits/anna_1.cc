//----------------------------  anna_1.cc  ---------------------------
//    $Id$
//    Version: 
//
//    Copyright (C) 2002, 2003, 2004 by the deal.II authors and Anna Schneebeli
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  anna_1.cc  ---------------------------


// check some things about Nedelec elements, basically also that the
// DoFRenumbering::component_wise function also works for
// non_primitive elements, for which it did not work previously since
// there is no component to associate a non-primitive shape function
// with
//
// this program is a modified version of one by Anna Schneebeli,
// University of Basel

#include "../tests.h"
#include <base/logstream.h>
#include <grid/tria.h>
#include <dofs/dof_handler.h>
#include <grid/grid_generator.h>
#include <grid/tria_accessor.h>
#include <grid/tria_iterator.h>
#include <dofs/dof_accessor.h>
#include <dofs/dof_tools.h>
#include <fe/fe_system.h>		
#include <fe/fe_q.h>
#include <fe/fe_nedelec.h>
#include <fe/fe_base.h>
#include <dofs/dof_renumbering.h>
#include <iostream>
#include <fstream>


template <int dim>
class SystemTest 
{
  public:
    SystemTest ();
    void run ();    
				    
  private:
    void make_grid_and_dofs ();
    void shape_to_components ();
    void check_numbering ();

				    
    Triangulation<dim>     triangulation;
    FESystem<dim>          fe;
    DoFHandler<dim>        dof_handler;

				   
};

template <int dim>
SystemTest<dim>::SystemTest () :
                fe (FE_Nedelec<dim>(1), 2,
                    FE_Q<dim>(1), 1),
		dof_handler (triangulation)
{}


template <int dim>
void SystemTest<dim>::make_grid_and_dofs ()
{
				  
  GridGenerator::hyper_cube (triangulation, -1, 1);
  triangulation.refine_global (0);
  deallog << "Number of active cells: " << triangulation.n_active_cells()
          << std::endl;
  deallog << "Total number of cells: " << triangulation.n_cells()
          << std::endl;
				  
  dof_handler.distribute_dofs (fe);
  deallog << "Number of degrees of freedom: " << dof_handler.n_dofs()
          << std::endl;
				  
}

template <int dim>
void SystemTest<dim>::shape_to_components () 
{
                                   // testing, if the shape function
                                   // with index i is of type Nedelec:
                                   // (i.e. the first component of the FESystem)
                                   // 1 for yes, 0 for no.
   
  for(unsigned int i = 0; i<fe.dofs_per_cell; i++)
    deallog <<"  shapefunction "<< i << " is Nedelec:  "
            << (fe.is_primitive(i) ? "false" : "true") << std::endl;
}



template <int dim>
void SystemTest<dim>::check_numbering () 
{
  typename DoFHandler<dim>::active_cell_iterator
    cell = dof_handler.begin_active(),
    endc = dof_handler.end();
  std::vector<unsigned int>	local_dof_indices(fe.dofs_per_cell);
	
  for (; cell!=endc; ++cell)
    {
      cell->get_dof_indices (local_dof_indices);
      for (unsigned int i=0; i<fe.dofs_per_cell; i++)
        deallog <<"  DoF "<< local_dof_indices[i] << " belongs to base element " 
                << fe.system_to_base_index(i).first.first
                << ", instance " << fe.system_to_base_index(i).first.second
                << std::endl;
      deallog<< std::endl;
    };
	 
	 
                                   //Now: Componentwise reodering of the dofs
	
  deallog << "  Now we renumber the DoFs component-wise:" << std::endl;
  deallog << "  ****************************************" << std::endl;
  DoFRenumbering::component_wise (dof_handler);
	
  cell = dof_handler.begin_active();
  endc = dof_handler.end();
	
  for (; cell!=endc; ++cell)
    {
      cell->get_dof_indices (local_dof_indices);
      for(unsigned int i=0; i<fe.dofs_per_cell; i++)
        deallog <<"  DoF "<< local_dof_indices[i] << " belongs to base " 
                << fe.system_to_base_index(i).first.first
                << ", instance " << fe.system_to_base_index(i).first.second
                << std::endl;
      deallog << std::endl;
    };
}


template <int dim>
void SystemTest<dim>::run () 
{
  make_grid_and_dofs ();
  shape_to_components ();
  check_numbering();
}

    

int main () 
{
  std::ofstream logfile("anna_1.output");
  deallog.attach(logfile);
  deallog.depth_console(0);

  SystemTest<2>().run();
  SystemTest<3>().run();  
  return 0;
}
