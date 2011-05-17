//----------------------------  parameter_handler_1.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2002, 2003, 2004, 2005, 2010 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  parameter_handler_1.cc  ---------------------------


// check the Patterns::List pattern

#include "../tests.h"
#include <deal.II/base/logstream.h>
#include <deal.II/base/parameter_handler.h>
#include <fstream>

void check (const char *p)
{
  ParameterHandler prm;
  prm.declare_entry ("test_1", "-1,0",
                     Patterns::List(Patterns::Integer(-1,1),2,3));

  std::ifstream in(p);
  prm.read_input (in);

  deallog << "test_1=" << prm.get ("test_1") << std::endl;
}


int main () 
{
  std::ofstream logfile("parameter_handler_1/output");
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  check ("parameter_handler_1/prm");
  
  return 0;
}
