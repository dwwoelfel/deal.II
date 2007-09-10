//----------------------------  hp_constraints_dgp_monomial_04.cc  ---------------------------
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
//----------------------------  hp_constraints_dgp_monomial_04.cc  ---------------------------


// check that computation of hp constraints works for DGP_Monomial elements correctly

// note that for mapped DGP(k) spaces, P(k) is not in the range of the
// mapping. However, P(k/2) is. Therefore, we have a gap for
// convergence, which we have to specify in the last argument to the
// call below

char logname[] = "hp_constraints_dgp_monomial_04/output";


#include "hp_constraints_common.h"


template <int dim>
void test ()
{
  hp::FECollection<dim> fe;
  for (unsigned int i=0; i<4; ++i)
    fe.push_back (FE_DGPMonomial<dim>(i));
  
  test_with_2d_deformed_mesh (fe);
}
