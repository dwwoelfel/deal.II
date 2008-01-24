//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------

// check MGDoFAccessor::get_mg_dof_indices

#include "../tests.h"
#include <base/logstream.h>
#include <base/function.h>
#include <lac/vector.h>
#include <lac/block_vector.h>
#include <grid/tria.h>
#include <grid/tria_iterator.h>
#include <grid/tria_accessor.h>
#include <grid/grid_generator.h>
#include <dofs/function_map.h>
#include <fe/fe_dgq.h>
#include <fe/fe_q.h>
#include <fe/fe_raviart_thomas.h>
#include <fe/fe_system.h>
#include <multigrid/mg_dof_accessor.h>
#include <multigrid/mg_dof_handler.h>
#include <multigrid/mg_tools.h>

#include <fstream>
#include <iomanip>
#include <iomanip>
#include <algorithm>

using namespace std;


template <int dim>
void dofs(const MGDoFHandler<dim>& dof)
{
  typename MGDoFHandler<dim>::cell_iterator cell;
  const typename MGDoFHandler<dim>::cell_iterator end = dof.end();

  std::vector<unsigned int> indices;
  
  for (cell = dof.begin(); cell != end; ++cell)
    {
      indices.resize(cell->get_fe().dofs_per_cell);
      cell->get_mg_dof_indices(indices);
      
      deallog << "Level " << cell->level();
      for (unsigned int i=0;i<GeometryInfo<dim>::vertices_per_cell;++i)
	deallog << " v" << cell->vertex(i);
      deallog << " dofs ";
      for (unsigned int i=0;i<indices.size();++i)
	deallog << ' ' << indices[i];
      deallog << std::endl;
    }
}


template <int dim>
void check_fe(FiniteElement<dim>& fe)
{
  deallog << fe.get_name() << std::endl;
  
  Triangulation<dim> tr;
  GridGenerator::hyper_cube(tr);
  tr.refine_global(2);
  ZeroFunction<dim> zero;
  typename FunctionMap<dim>::type fmap;
  fmap.insert(std::make_pair(0, &zero));
  
  MGDoFHandler<dim> mgdof(tr);
  mgdof.distribute_dofs(fe);
  dofs(mgdof);
}


template <int dim>
void check()
{
  FE_Q<dim> q1(1);
  FE_Q<dim> q2(2);
//  FE_DGQ<dim> dq1(1);
  
  FESystem<dim> s1(q1, 2, q2,1);

  check_fe(q1);
  check_fe(q2);
  check_fe(s1);
}

int main()
{
  std::ofstream logfile("dof_01/output");
  deallog << std::setprecision(3);
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  check<1> ();
  check<2> ();
  check<3> ();
}
