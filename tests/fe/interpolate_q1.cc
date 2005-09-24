//----------------------------------------------------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2005 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------------------------------------------------

#include "interpolate_common.h"
#include <base/logstream.h>
#include <base/quadrature_lib.h>

#include <fe/fe_q.h>
#include <fe/fe_dgq.h>

#include <fstream>

// FE_Q<dim>::interpolate(...)
// FE_DGQ<dim>::interpolate(...)

template <int dim>
void check(const Function<dim>& f,
	   const unsigned int degree)
{
  FE_Q<dim> fe(degree);
  deallog << fe.get_name() << ' ';
  
  std::vector<double> dofs(fe.dofs_per_cell);
  
  std::vector<std::vector<double> >
    values(1, std::vector<double>(fe.get_unit_support_points().size()));
  f.value_list(fe.get_unit_support_points(), values[0]);
  fe.interpolate(dofs, values[0]);
  deallog << " value " << difference(fe,dofs,f);
  fe.interpolate(dofs, values);
  deallog << " vector " << difference(fe,dofs,f);

  std::vector<Vector<double> >
    vectors(fe.get_unit_support_points().size(), Vector<double>(1));
  f.vector_value_list(fe.get_unit_support_points(), vectors);
  fe.interpolate(dofs, vectors, 0);
  deallog << " Vector " << difference(fe,dofs,f) << std::endl;
}

template <int dim>
void check_dg(const Function<dim>& f,
	      const unsigned int degree)
{
  FE_DGQ<dim> fe(degree);
  deallog << fe.get_name() << ' ';
  
  std::vector<double> dofs(fe.dofs_per_cell);
  
  std::vector<std::vector<double> >
    values(1, std::vector<double>(fe.get_unit_support_points().size()));
  f.value_list(fe.get_unit_support_points(), values[0]);
  fe.interpolate(dofs, values[0]);
  deallog << " value " << difference(fe,dofs,f);
  fe.interpolate(dofs, values);
  deallog << " vector " << difference(fe,dofs,f);

  std::vector<Vector<double> >
    vectors(fe.get_unit_support_points().size(), Vector<double>(1));
  f.vector_value_list(fe.get_unit_support_points(), vectors);
  fe.interpolate(dofs, vectors, 0);
  deallog << " Vector " << difference(fe,dofs,f) << std::endl;
}

template <int dim>
void check_dg_lobatto(const Function<dim>& f,
		      const unsigned int degree)
{
  QGaussLobatto<1> fe_quadrature(degree);
  FE_DGQ<dim> fe(fe_quadrature);
  deallog << fe.get_name() << ' ';
  
  std::vector<double> dofs(fe.dofs_per_cell);
  
  std::vector<std::vector<double> >
    values(1, std::vector<double>(fe.get_unit_support_points().size()));
  f.value_list(fe.get_unit_support_points(), values[0]);
  fe.interpolate(dofs, values[0]);
  deallog << " value " << difference(fe,dofs,f);
  fe.interpolate(dofs, values);
  deallog << " vector " << difference(fe,dofs,f);

  std::vector<Vector<double> >
    vectors(fe.get_unit_support_points().size(), Vector<double>(1));
  f.vector_value_list(fe.get_unit_support_points(), vectors);
  fe.interpolate(dofs, vectors, 0);
  deallog << " Vector " << difference(fe,dofs,f) << std::endl;
}

int main()
{
  std::ofstream logfile ("interpolate_q1.output");
  deallog.attach(logfile);
  deallog.depth_console(10);
  deallog.threshold_double(2.e-15);

  Q1WedgeFunction<1,1> w1;
  check(w1,1);
  check(w1,2);
  check(w1,3);
  check_dg(w1,1);
  check_dg(w1,2);
  check_dg(w1,3);
  Q1WedgeFunction<2,1> w2;
  check(w2,1);
  check(w2,2);
  check(w2,3);
  check_dg(w2,2);
  check_dg(w2,3);
  Q1WedgeFunction<2,2> w22;
  check(w22,2);
  check(w22,3);
  check_dg(w22,2);
  check_dg(w22,3);
  check_dg_lobatto(w22,4);
  Q1WedgeFunction<2,3> w23;
  check(w23,3);
  Q1WedgeFunction<3,1> w3;
  check_dg(w3,1);
  check(w3,1);
  check(w3,2);
  check(w3,3);
}
