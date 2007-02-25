//----------------------------  hp_line_dof_identities_q_system_03.cc  ---------------------------
//    $Id: hp_line_dof_identities_q_system_03.cc 12464 2006-02-23 01:13:17Z wolf $
//    Version: $Name$ 
//
//    Copyright (C) 2005, 2006, 2007 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  hp_line_dof_identities_q_system_03.cc  ---------------------------


// check FESystem(FE_Q,FE_DGQ)::hp_line_dof_identities. i thought this was
// what would show what happened crash_14, but it turned out to be a wrong
// lead. doesn't matter, keep it anyway :-)


#include "../tests.h"
#include <base/logstream.h>
#include <fe/fe_collection.h>
#include <fe/fe_q.h>
#include <fe/fe_dgq.h>
#include <fe/fe_system.h>

#include <fstream>


template <int dim>
void test ()
{
  hp::FECollection<dim> fe_collection;
  for (unsigned int k=1; k<4; ++k)
    {
      for (unsigned int i=1; i<4; ++i)
	fe_collection.push_back (FESystem<dim>(FE_Q<dim>(k),1,
					       FE_DGQ<dim>(i),1));

      for (unsigned int i=0; i<fe_collection.size(); ++i)
	for (unsigned int j=0; j<fe_collection.size(); ++j)
	  {
	    const std::vector<std::pair<unsigned int, unsigned int> >
	      identities = fe_collection[i].hp_line_dof_identities (fe_collection[j]);

	    deallog << "Identities for "
		    << fe_collection[i].get_name() << " and "
		    << fe_collection[j].get_name() << ": "
		    << identities.size()
		    << std::endl;
	
	    for (unsigned int k=0; k<identities.size(); ++k)
	      {
		Assert (identities[k].first < fe_collection[i].dofs_per_line,
			ExcInternalError());
		Assert (identities[k].second < fe_collection[j].dofs_per_line,
			ExcInternalError());
	    
		deallog << identities[k].first << ' '
			<< identities[k].second
			<< std::endl;
	      }
	  }
    }
}



int main ()
{
  std::ofstream logfile("hp_line_dof_identities_q_system_03/output");
  logfile.precision(2);
  
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);
  
  test<1> ();
  test<2> ();
  test<3> ();
  
  deallog << "OK" << std::endl;
}
