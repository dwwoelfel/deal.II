//----------------------------  point_value_02.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2004, 2006, 2007 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  point_value_02.cc  ---------------------------

// check VectorTools::point_value, alternative algorithm with mapping



#include "../tests.h"
#include <base/logstream.h>
#include <base/function_lib.h>
#include <base/quadrature_lib.h>
#include <lac/vector.h>
#include <grid/tria.h>
#include <grid/tria_iterator.h>
#include <grid/tria_accessor.h>
#include <grid/grid_generator.h>
#include <dofs/dof_handler.h>
#include <fe/fe_q.h>
#include <fe/mapping_q1.h>
#include <numerics/vectors.h>

#include <fstream>
#include <cmath>
#include <iomanip>


template<int dim>
class MySquareFunction : public Function<dim>
{
  public:
    virtual double value (const Point<dim>   &p,
			  const unsigned int  component) const
      {	return (component+1)*p.square()+1; }
    
    virtual void   vector_value (const Point<dim>   &p,
				 Vector<double>     &values) const
      { values(0) = value(p,0); }
};


template<int dim>
class MyExpFunction : public Function<dim>
{
  public:
    virtual double value (const Point<dim>   &p,
			  const unsigned int  component) const
      {	return std::exp (p(0)); }
    
    virtual void   vector_value (const Point<dim>   &p,
				 Vector<double>     &values) const
      { values(0) = value(p,0); }
};



template <int dim>
void make_mesh (Triangulation<dim> &tria)
{
  
  GridGenerator::hyper_cube(tria, -1, 1);

                                   // refine the mesh in a random way so as to
                                   // generate as many cells with
                                   // hanging nodes as possible
  tria.refine_global (4-dim);
  const double steps[4] = { /*d=0*/ 0, 7, 3, 3 };
  for (unsigned int i=0; i<steps[dim]; ++i)
    {
      typename Triangulation<dim>::active_cell_iterator
        cell = tria.begin_active();
      for (unsigned int index=0; cell != tria.end(); ++cell, ++index)
        if (index % (3*dim) == 0)
          cell->set_refine_flag();
      tria.execute_coarsening_and_refinement ();
    }
}




template <int dim>
void
check ()
{
  Triangulation<dim> tria;
  make_mesh (tria);
  
  FE_Q<dim> element(3);
  DoFHandler<dim> dof(tria);
  MappingQ1<dim> mapping;
  dof.distribute_dofs(element);

                                   // test with two different functions: one
                                   // that is exactly representable on the
                                   // chosen finite element space, and one
                                   // that isn't
  for (unsigned int i=0; i<2; ++i)
    {
      static const MySquareFunction<dim>          function_1;
      static const Functions::CosineFunction<dim> function_2;

      const Function<dim> &
        function = (i==0 ?
                    static_cast<const Function<dim>&>(function_1) :
                    static_cast<const Function<dim>&>(function_2));
      
      Vector<double> v (dof.n_dofs());
      VectorTools::interpolate (dof, function, v);

                                       // for the following points, check the
                                       // function value, output it, and
                                       // verify that the value retrieved from
                                       // the interpolated function is close
                                       // enough to that of the real function
                                       //
                                       // also verify that the actual value is
                                       // roughly correct
      Point<dim> p[3];
      for (unsigned int d=0; d<dim; ++d)
        {
          p[0][d] = 0;
          p[1][d] = 0.5;
          p[2][d] = 1./3.;
        }
      Vector<double> value(1);
      for (unsigned int i=0; i<3; ++i)
        {
          VectorTools::point_value (mapping, dof, v, p[i], value);
          deallog << -value(0) << std::endl;

          Assert (std::abs(value(0) - function.value(p[i])) < 1e-4,
                  ExcInternalError());

	  const double scalar_value = VectorTools::point_value (mapping, dof, v, p[i]);
          Assert (std::abs(value(0) - scalar_value) < 1e-4,
                  ExcInternalError());
        }  
    }
  
  deallog << "OK" << std::endl;
}


int main ()
{
  std::ofstream logfile ("point_value_02/output");
  deallog << std::setprecision (4);
  deallog.attach(logfile);
  deallog.depth_console (0);

  deallog.push ("1d");
  check<1> ();
  deallog.pop ();
  deallog.push ("2d");
  check<2> ();
  deallog.pop ();
  deallog.push ("3d");
  check<3> ();
  deallog.pop ();
}
