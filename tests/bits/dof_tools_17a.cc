//----------------------------  dof_tools_1a.cc  ---------------------------
//    $Id$
//    Version: 
//
//    Copyright (C) 2003, 2004 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  dof_tools_1a.cc  ---------------------------

#include "../tests.h"
#include "dof_tools_common.cc"
#include <lac/sparsity_pattern.h>

// check
//   DoFTools::
//   make_flux_sparsity_pattern (const DoFHandler<dim> &,
//	                         SparsityPattern       &);


std::string output_file_name = "dof_tools_17a.output";


template <int dim>
void
check_this (const DoFHandler<dim> &dof_handler)
{
                                   // create sparsity pattern
  SparsityPattern sp (dof_handler.n_dofs(),
                      dof_handler.max_couplings_between_dofs()*2);
  DoFTools::make_flux_sparsity_pattern (dof_handler, sp);
  sp.compress ();
  
                                   // write out 20 lines of this
                                   // pattern (if we write out the
                                   // whole pattern, the output file
                                   // would be in the range of 40 MB)
  for (unsigned int l=0; l<20; ++l)
    {
      const unsigned int line = l*(sp.n_rows()/20);
      for (unsigned int c=0; c<sp.row_length(line); ++c)
        deallog << sp.column_number(line,c) << " ";
      deallog << std::endl;
    }

                                   // write out some other indicators
  deallog << sp.bandwidth () << std::endl
          << sp.max_entries_per_row () << std::endl
          << sp.n_nonzero_elements () << std::endl;

  unsigned int hash = 0;
  for (unsigned int l=0; l<sp.n_rows(); ++l)
    hash += l*(sp.row_length(l) +
               sp.get_rowstart_indices()[l] +
               sp.get_column_numbers()[sp.get_rowstart_indices()[l]
                                       +
                                       (sp.row_length(l)>1 ? 1 : 0)]);
  deallog << hash << std::endl;
}
