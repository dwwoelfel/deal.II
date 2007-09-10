//----------------------------  project_q_04.cc  ---------------------------
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
//----------------------------  project_q_04.cc  ---------------------------


// check that VectorTools::project works for Q elements correctly

char logname[] = "project_q_04/output";


#include "project_common.cc"


template <int dim>
void test ()
{
  for (unsigned int p=1; p<6-dim; ++p)
    test_with_2d_deformed_mesh (FE_Q<dim>(p), p);
}
