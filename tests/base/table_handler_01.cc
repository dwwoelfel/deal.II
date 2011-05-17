//-----------------------------------------------------------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2010 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//-----------------------------------------------------------------------------

#include "../tests.h"
#include <deal.II/base/data_out_base.h>
#include <deal.II/base/table_handler.h>
#include <deal.II/base/logstream.h>

#include <vector>
#include <iomanip>
#include <fstream>
#include <string>

// test the method set_tex_table_caption
// it creates a caption for the whole table

int main ()
{
  std::ofstream logfile("table_handler_01/output");
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  TableHandler table;

  for (unsigned int i=1; i<=10; ++i)
  {
    table.add_value("numbers", i);
    table.add_value("squares", i*i);
    table.add_value("square roots", sqrt(i));
  }

  table.set_tex_table_caption("This is a caption text with \\LaTeX{} symbols");

                                 // output
  table.write_tex(deallog.get_file_stream());
}
