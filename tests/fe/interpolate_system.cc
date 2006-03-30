//----------------------------------------------------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2005, 2006 by the deal.II authors
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
#include <fe/fe_system.h>

#include <fstream>

// FE_Q<dim>::interpolate(...)

template <int dim>
void check1(const Function<dim>& f,
	    const unsigned int degree,
	    const unsigned int comp)
{
  FE_Q<dim> feq(degree);
  FESystem<dim> fe(feq, comp);
  deallog << fe.get_name() << ' ';
  
  std::vector<double> dofs(fe.dofs_per_cell);
  
  std::vector<std::vector<double> >
    values(comp, std::vector<double>(fe.get_unit_support_points().size()));
  std::vector<Vector<double> >
    vectors(fe.get_unit_support_points().size(),
	    Vector<double>(f.n_components));
  f.vector_value_list(fe.get_unit_support_points(), vectors);
  for (unsigned int c=0;c<values.size();++c)
    for (unsigned int k=0;k<values[c].size();++k)
      values[c][k] = vectors[k](c);
  
  fe.interpolate(dofs, values);
  deallog << " vector " << vector_difference(fe,dofs,f,0);
  
  fe.interpolate(dofs, vectors, 0);
  deallog << " Vector " << vector_difference(fe,dofs,f,0) << std::endl;
}

template <int dim>
void check3(const Function<dim>& f,
	    const unsigned int degree,
	    const unsigned int comp1,
	    const unsigned int comp2,
	    const unsigned int comp3)
{
  FE_Q<dim> feq1(degree);
  FE_Q<dim> feq2(degree+1);
  FE_Q<dim> feq3(degree+2);
  FESystem<dim> fe(feq1, comp1, feq2, comp2, feq3, comp3);
  deallog << fe.get_name() << ' ';
  
  std::vector<double> dofs(fe.dofs_per_cell);
  
  std::vector<std::vector<double> >
    values(f.n_components,
	   std::vector<double>(fe.get_unit_support_points().size()));
  std::vector<Vector<double> >
    vectors(fe.get_unit_support_points().size(),
	    Vector<double>(f.n_components));
  f.vector_value_list(fe.get_unit_support_points(), vectors);
  for (unsigned int c=0;c<values.size();++c)
    for (unsigned int k=0;k<values[c].size();++k)
      values[c][k] = vectors[k](c);
  
  fe.interpolate(dofs, values);
  deallog << " vector " << vector_difference(fe,dofs,f,0);
  
  fe.interpolate(dofs, vectors, 0);
  deallog << " Vector " << vector_difference(fe,dofs,f,0) << std::endl;
}

int main()
{
  std::ofstream logfile ("interpolate_system/output");
  deallog.attach(logfile);
//  deallog.depth_console(0);
  deallog.threshold_double(1.e-15);

  Q1WedgeFunction<1,1,2> w1;
  check1(w1,1,2);
  check1(w1,2,2);
  check1(w1,3,2);
  Q1WedgeFunction<2,2,3> w2;
  check1(w2,2,3);
  check1(w2,3,3);
  Q1WedgeFunction<3,1,3> w3;
  check1(w3,1,3);
  check1(w3,2,3);
  Q1WedgeFunction<2,1,9> www2;
  check3(www2,1,2,3,4);
  Q1WedgeFunction<3,1,9> www3;
  check3(www3,1,2,3,4);
}
