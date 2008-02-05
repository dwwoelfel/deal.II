//----------------------------------------------------------------------
//    system_1.cc,v 1.3 2003/06/09 21:55:00 wolf Exp
//    Version: 
//
//    Copyright (C) 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    fuqher information on this license.
//
//----------------------------------------------------------------------

// Test the various index conversion methods

#include "../tests.h"
#include <base/logstream.h>
#include <fe/fe_q.h>
#include <fe/fe_dgq.h>
#include <fe/fe_raviart_thomas.h>
#include <fe/fe_system.h>

#include <fstream>
#include <string>
#include <iomanip>

#define PRECISION 5



template<int dim>
void
check_fe(const FiniteElement<dim> &fe)
{
  deallog << fe.get_name() << std::endl;
  const unsigned int n_dofs = fe.dofs_per_cell;
  const unsigned int n_base = fe.n_base_elements();
  const unsigned int n_comp = fe.n_components();
  const unsigned int n_blocks = fe.n_blocks();
  
  deallog << "Base elements:  " << n_base
	  << std::endl
	  << "Multiplicities:";
  for (unsigned int b=0;b<n_base;++b)
    deallog << ' ' << fe.element_multiplicity(b);
  deallog << std::endl
	  << "First block   :";
  for (unsigned int b=0;b<n_base;++b)
    deallog << ' ' << fe.first_block_of_base(b);
  
  deallog << std::endl << "Blocks : " << n_blocks << std::endl;
  
  for (unsigned int i=0;i<n_dofs;++i)
    {
      deallog << std::setw(3) << i;
				       // Cehck consistency of
				       // functions and inverse
      std::pair<unsigned int, unsigned int> p;
      if (fe.is_primitive(i))
	{
	  p = fe.system_to_component_index(i);
	  Assert(fe.component_to_system_index(p.first, p.second) == i,
		 ExcInternalError());
	}
    }
  
  deallog << std::endl;
  
  deallog << "Next two lines: block index_in_block"
	  << std::endl;
  for (unsigned int i=0;i<n_dofs;++i)
    deallog << std::setw(3) << fe.system_to_block_index(i).first;
  deallog << std::endl;
  for (unsigned int i=0;i<n_dofs;++i)
    deallog << std::setw(3) << fe.system_to_block_index(i).second;
  deallog << std::endl;
  deallog << "Next three lines: base block_in_base index_in_block"
	  << std::endl;
  for (unsigned int i=0;i<n_dofs;++i)
    deallog << std::setw(3) << fe.system_to_base_index(i).first.first;
  deallog << std::endl;
  for (unsigned int i=0;i<n_dofs;++i)
    deallog << std::setw(3) << fe.system_to_base_index(i).first.second;
  deallog << std::endl;
  for (unsigned int i=0;i<n_dofs;++i)
    deallog << std::setw(3) << fe.system_to_base_index(i).second;
  deallog << std::endl;

  deallog << "Next two lines: component index_in_component"
	  << std::endl;
  for (unsigned int i=0;i<n_dofs;++i)
    if (fe.is_primitive(i))
      deallog << std::setw(3) << fe.system_to_component_index(i).first;
    else
      deallog << std::setw(3) << 'x';
  deallog << std::endl;
  for (unsigned int i=0;i<n_dofs;++i)
    if (fe.is_primitive(i))
      deallog << std::setw(3) << fe.system_to_component_index(i).second;
    else
      deallog << std::setw(3) << 'x';
  deallog << std::endl;

  if (true || fe.is_primitive())
    {
      deallog << "Next two lines: component_to_base" << std::endl;
      for (unsigned int i=0;i<n_comp;++i)
	deallog << std::setw(3) << fe.component_to_base_index(i).first;
      deallog << std::endl;
      for (unsigned int i=0;i<n_comp;++i)
	deallog << std::setw(3) << fe.component_to_base_index(i).second;
      deallog << std::endl;
      deallog << "Next line: component_to_block_index" << std::endl;
      for (unsigned int i=0;i<n_comp;++i)
	deallog << std::setw(3) << fe.component_to_block_index(i);
      deallog << std::endl;
      
    }  
}



template<int dim>
void check()
{
  FE_DGQ<dim> co(0);
  FE_Q<dim> q1(1);
  FE_Q<dim> q2(2);
  FE_DGQ<dim> dgq1(1);
  FE_RaviartThomas<dim> rt0(0);
  FE_RaviartThomas<dim> rt1(1);
  FE_RaviartThomas<dim> rt2(2);

  check_fe(FESystem<dim>(q1, 1, co, 1));
  check_fe(FESystem<dim>(q1, 2, co, 3));
  check_fe(FESystem<dim>(dgq1, 2, co, 3));
  check_fe(FESystem<dim>(rt1,1, dgq1, 1));
  check_fe(FESystem<dim>(rt0, 2, co, 3));
  check_fe(FESystem<dim>(rt1, 2, co, 3));
  check_fe(FESystem<dim>(q1, 2, q2, 1, co, 2));
  check_fe(FESystem<dim>(rt1, 2, q2, 1, co, 2));
  check_fe(FESystem<dim>(rt2,1, q2, 1));
}


int
main()
{
  std::ofstream logfile ("system_index/output");
  deallog << std::setprecision(PRECISION);
  deallog << std::fixed;  
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  check<2>();
  check<3>();
  
  return 0;
}



