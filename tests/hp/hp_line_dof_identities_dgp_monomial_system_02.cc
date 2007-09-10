//----------------------------  hp_line_dof_identities_dgp_monomial_system_02.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2005, 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  hp_line_dof_identities_dgp_monomial_system_02.cc  ---------------------------


// check FESystem(FE_DGPMonomial)::hp_line_dof_identities, but with a different
// arrangement of base elements and multiplicities than in the _01 test


#include "../tests.h"
#include <base/logstream.h>
#include <hp/fe_collection.h>
#include <fe/fe_dgp_monomial.h>
#include <fe/fe_system.h>

#include <fstream>


template <int dim>
void test ()
{
  hp::FECollection<dim> fe_collection;
  for (unsigned int i=0; i<4; ++i)
    {
				       // add the system three times, with
				       // different numbers of base elements
				       // and multiplicities
      fe_collection.push_back (FESystem<dim>(FE_DGPMonomial<dim>(i),3));
      fe_collection.push_back (FESystem<dim>(FE_DGPMonomial<dim>(i),2,
					     FE_DGPMonomial<dim>(i),1));
      fe_collection.push_back (FESystem<dim>(FE_DGPMonomial<dim>(i),1,
					     FE_DGPMonomial<dim>(i),2));
    }

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

					 // make sure the identities are the
					 // same whatever the arrangement of
					 // the elements to their base
					 // elements (the operation i/3*3
					 // brings us back to the first of the
					 // three identical elements in the
					 // collection)
	Assert (identities ==
		fe_collection[i/3*3].hp_line_dof_identities (fe_collection[j/3*3]),
		ExcInternalError());
      }
}



int main ()
{
  std::ofstream logfile("hp_line_dof_identities_dgp_monomial_system_02/output");
  logfile.precision(2);
  
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);
  
  test<1> ();
  test<2> ();
  test<3> ();
  
  deallog << "OK" << std::endl;
}
