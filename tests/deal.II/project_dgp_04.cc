//----------------------------  project_dgp_04.cc  ---------------------------
//    $Id: project_dgp_04.cc 12732 2006-03-28 23:15:45Z wolf $
//    Version: $Name$ 
//
//    Copyright (C) 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  project_dgp_04.cc  ---------------------------


// check that VectorTools::project works for DGP elements correctly

// note that for mapped DGP(k) spaces, P(k) is not in the range of the
// mapping. However, P(k/2) is. Therefore, we have a gap for
// convergence, which we have to specify in the last argument to the
// call below

char logname[] = "project_dgp_04/output";


#include "project_common.cc"


template <int dim>
void test ()
{
  for (unsigned int p=0; p<6-dim; ++p)
    test_with_2d_deformed_mesh (FE_DGP<dim>(p), p, (p+1)/2);
}
