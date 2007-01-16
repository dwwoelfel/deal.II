//----------------------------  injection_dgq.cc  ---------------------------
//    $Id: injection_dgq.cc 12732 2006-03-28 23:15:45Z wolf $
//    Version: $Name$ 
//
//    Copyright (C) 2006, 2007 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  injection_dgq.cc  ---------------------------


char logname[] = "injection_dgq/output";


#include "injection_common.h"


template <int dim>
void test ()
{
  for (unsigned int i=0; i<4; ++i)
    for (unsigned int j=i; j<4; ++j)
      do_check (FE_DGQ<dim>(i), FE_DGQ<dim>(j));
}
