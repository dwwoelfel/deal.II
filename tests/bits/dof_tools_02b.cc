//----------------------------  dof_tools_2b.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2003, 2004 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  dof_tools_2b.cc  ---------------------------

#include "../tests.h"
#include "dof_tools_common.h"
#include <lac/compressed_sparsity_pattern.h>

// check
//   DoFTools::
//   make_sparsity_pattern (const DoFHandler<dim>     &,
//                          std::vector<std::vector<bool> > &,
//	                    CompressedSparsityPattern &);

std::string output_file_name = "dof_tools_02b/output";


template <int dim>
void
check_this (const DoFHandler<dim> &dof_handler)
{
                                   // set up X-shape mask
  const unsigned int n_components = dof_handler.get_fe().n_components();
  std::vector<std::vector<bool> > mask (n_components,
                                        std::vector<bool>(n_components,false));
  for (unsigned int i=0; i<n_components; ++i)
    mask[i][i] = mask[i][n_components-i-1] = true;
  
                                   // create sparsity pattern
  CompressedSparsityPattern sp (dof_handler.n_dofs());
  DoFTools::make_sparsity_pattern (dof_handler, mask, sp);
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
    hash += l*sp.row_length(l);
  deallog << hash << std::endl;
}
