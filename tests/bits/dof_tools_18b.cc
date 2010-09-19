//----------------------------  dof_tools_1b.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2003, 2004, 2005, 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  dof_tools_1b.cc  ---------------------------

#include "../tests.h"
#include "dof_tools_common.h"
#include <lac/compressed_sparsity_pattern.h>

// check
//   DoFTools::
//   make_flux_sparsity_pattern (const DoFHandler<dim>     &,
//	                         CompressedSparsityPattern &);

std::string output_file_name = "dof_tools_18b/output";

                                   // set up masks. choose X-shaped
                                   // masks with full first row and
                                   // column (well, we had to invent
                                   // something)
void
make_masks (const unsigned int n,
            Table<2, DoFTools::Coupling> &m1,
            Table<2, DoFTools::Coupling> &m2)
{
  m1.reinit (n,n);
  m2.reinit (n,n);
  for (unsigned int i=0; i<n; ++i)
    m1(i,0) = m1(0,i) = m2(i,0) = m2(0,i) = m1(i,i) = m2(i,i) = DoFTools::nonzero;
}

  
void
my_check_this (const DoFHandler<1> &) 
{}



template <int dim>
void
my_check_this (const DoFHandler<dim> &dof_handler)
{
  Table<2, DoFTools::Coupling> mask_int;
  Table<2, DoFTools::Coupling> mask_ext;
  make_masks (dof_handler.get_fe().n_components(),
              mask_int, mask_ext);
  
                                   // create sparsity pattern
  CompressedSparsityPattern sp (dof_handler.n_dofs());
  DoFTools::make_flux_sparsity_pattern (dof_handler, sp,
                                        mask_int, mask_ext);
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

template <int dim>
void
check_this (const DoFHandler<dim> &dof_handler)
{
				   // since we can't forward declare
				   // check_this in this file (it is forward
				   // declared in dof_tools_common.h), we
				   // also can't make the driver file aware of
				   // the overload for 1d. to avoid linker
				   // errors, we can consequently not overload
				   // check_this, and need this forwarder
				   // function
  my_check_this (dof_handler);
}
