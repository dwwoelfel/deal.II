//----------------------------  gradients.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 1998, 1999, 2000, 2003, 2004, 2005 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  gradients.cc  ---------------------------


#include "../tests.h"
#include <grid/tria.h>
#include <grid/tria_boundary.h>
#include <dofs/dof_handler.h>
#include <grid/grid_generator.h>
#include <fe/fe_values.h>
#include <fe/fe_q.h>
#include <fe/mapping_q1.h>
#include <base/quadrature_lib.h>
#include <grid/tria_iterator.h>
#include <dofs/dof_accessor.h>
#include <lac/vector.h>
#include <base/logstream.h>

#include <fstream>


int main ()
{
  std::ofstream logfile("gradients/output");
				   // limit output a bit
  logfile.precision (3);
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  Triangulation<2> tria;
  GridGenerator::hyper_cube (tria,0,1);
  tria.begin_active()->vertex(3)(0) = 2;

  FE_Q<2> fe(1);
  DoFHandler<2> dof(tria);
  dof.distribute_dofs(fe);

  QTrapez<2> q;
  MappingQ1<2> mapping;
  FEValues<2> fevalues(mapping,fe,q,update_gradients);
  fevalues.reinit (dof.begin_active());


  Vector<double> val(4);

  deallog << "Testing transformation of gradients of shape function:" << std::endl;
  
				   // test for each of the four
				   // shape functions
  bool testcase_succeeded = true;
  for (unsigned int vertex=0; vertex<4; ++vertex)
    {
      val = 0;
      val(vertex) = 1;

      std::vector<Tensor<1,2> > grads(4);
      fevalues.get_function_grads (val, grads);


      bool ok;
      switch (vertex) 
	{
	  case 0:
		ok = ((grads[0] == Point<2>(-1,-1)) &&
		      (grads[1] == Point<2>(0,-1)) &&
		      (grads[2] == Point<2>(-1,1)) &&
		      (grads[3] == Point<2>(0,0)));
		break;
	  case 1:
		ok = ((grads[0] == Point<2>(1,0)) &&
		      (grads[1] == Point<2>(0,0)) &&
		      (grads[2] == Point<2>(1,-2)) &&
		      (grads[3] == Point<2>(0,-1)));
		break;
	  case 2:
		ok = ((grads[0] == Point<2>(0,1)) &&
		      (grads[1] == Point<2>(-0.5,1)) &&
		      (grads[2] == Point<2>(0,0)) &&
		      (grads[3] == Point<2>(-0.5,0.5)));
		break;
	  case 3:
		ok = ((grads[0] == Point<2>(0,0)) &&
		      (grads[1] == Point<2>(0.5,0)) &&
		      (grads[2] == Point<2>(0,1)) &&
		      (grads[3] == Point<2>(0.5,0.5)));
		break;
	};

      deallog << "  Shape function " << vertex
	   << ": "
	   << (ok ? "OK" : "WRONG!")
	   << std::endl;

      if (!ok)
	testcase_succeeded = false;
    };

  if (testcase_succeeded)
    return 0;
  else
    return 1;
}
