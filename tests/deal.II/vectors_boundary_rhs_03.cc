//----------------------------  vectors_boundary_rhs_03.cc  ---------------------------
//    $Id: vectors_boundary_rhs_03.cc 14279 2006-12-28 04:53:24Z bangerth $
//    Version: $Name$ 
//
//    Copyright (C) 2000, 2001, 2003, 2004, 2006, 2007 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  vectors_boundary_rhs_03.cc  ---------------------------


// check VectorTools::create_boundary_right_hand_side, but this time for a scalar
// primitive element


#include "../tests.h"
#include <base/quadrature_lib.h>
#include <base/logstream.h>
#include <base/function_lib.h>
#include <lac/sparse_matrix.h>
#include <lac/vector.h>
#include <grid/tria.h>
#include <grid/tria_iterator.h>
#include <grid/tria_accessor.h>
#include <grid/grid_generator.h>
#include <dofs/dof_handler.h>
#include <dofs/dof_tools.h>
#include <dofs/dof_constraints.h>
#include <fe/fe_q.h>
#include <fe/fe_system.h>
#include <fe/fe_collection.h>
#include <fe/q_collection.h>
#include <fe/mapping_q.h>
#include <fe/mapping_collection.h>
#include <numerics/vectors.h>

#include <fstream>


template<int dim>
class MySquareFunction : public Function<dim>
{
  public:
    MySquareFunction () : Function<dim>(1) {};
    
    virtual double value (const Point<dim>   &p,
			  const unsigned int  component) const
      {	return (component+1)*p.square(); };
    
    virtual void   vector_value (const Point<dim>   &p,
				 Vector<double>     &values) const
      {
	  values(0) = value(p,0);
      };
};



template <int dim>
void
check ()
{
  Triangulation<dim> tr;  
  if (dim==2)
    GridGenerator::hyper_ball(tr, Point<dim>(), 1);
  else
    GridGenerator::hyper_cube(tr, -1,1);
  tr.refine_global (1);
  tr.begin_active()->set_refine_flag ();
  tr.execute_coarsening_and_refinement ();
  if (dim==1)
    tr.refine_global(2);

				   // create a system element composed
				   // of one Q1 and one Q2 element
  FE_Q<dim> element(1);
  DoFHandler<dim> dof(tr);
  dof.distribute_dofs(element);

				   // use a more complicated mapping
				   // of the domain and a quadrature
				   // formula suited to the elements
				   // we have here
  MappingQ<dim> mapping(3);

  QGauss<dim-1> quadrature(3);

  Vector<double> rhs (dof.n_dofs());
  VectorTools::create_boundary_right_hand_side (dof, quadrature,
				       MySquareFunction<dim>(),
				       rhs);
  for (unsigned int i=0; i<rhs.size(); ++i)
    deallog << rhs(i) << std::endl;
}



int main ()
{
  std::ofstream logfile ("vectors_boundary_rhs_03/output");
  logfile.precision (4);
  logfile.setf(std::ios::fixed);  
  deallog.attach(logfile);
  deallog.depth_console (0);

  deallog.push ("2d");
  check<2> ();
  deallog.pop ();
  deallog.push ("3d");
  check<3> ();
  deallog.pop ();
}
