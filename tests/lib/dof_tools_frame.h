//----------------------------------------------------------------------
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
//----------------------------------------------------------------------


#include "../tests.h"
#include <base/logstream.h>
#include <grid/tria.h>
#include <grid/tria_iterator.h>
#include <grid/grid_generator.h>
#include <dofs/dof_accessor.h>
#include <dofs/dof_handler.h>
#include <dofs/dof_tools.h>
#include <fe/fe_q.h>
#include <fe/fe_dgq.h>
#include <fe/fe_dgp.h>
#include <fe/fe_nedelec.h>
#include <fe/fe_raviart_thomas.h>
#include <fe/fe_system.h>

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>


// forward declaration of the function that must be provided in the
// .cc files
template <int dim>
void
check_this (const DoFHandler<dim> &dof_handler);


void
output_vector (std::vector<bool> &v)
{
for (unsigned int i=0; i<v.size(); ++i)
    deallog << (v[i] ? '1' : '0');
  deallog << std::endl;
}


void
output_vector (std::vector<unsigned int> &v)
{
for (unsigned int i=0; i<v.size(); ++i)
  deallog << ' ' << v[i];
  deallog << std::endl;
}



template <int dim>
void
check (const Triangulation<dim>& tria,
       const FiniteElement<dim> &fe)
{
  deallog << fe.get_name() << std::endl;
  
  DoFHandler<dim> dof_handler (tria);
  dof_handler.distribute_dofs (fe);

                                   // call main function in .cc files
  check_this (dof_handler);
}


template <int dim>
void check_grid(const Triangulation<dim>& tr)
{
  FE_Q<dim> q1(1);
  check(tr, q1);
  FE_Q<dim> q2(2);
  check(tr, q2);
  FE_Q<dim> q3(3);
  check(tr, q3);
  
  FE_DGQ<dim> dgq0(0);
  check(tr, dgq0);
  FE_DGQ<dim> dgq1(1);
  check(tr, dgq1);
  FE_DGQ<dim> dgq2(2);
  check(tr, dgq2);

  FE_DGP<dim> dgp1(1);
  check(tr, dgp1);
  FE_DGP<dim> dgp2(2);
  check(tr, dgp2);
  
  FE_Nedelec<dim> nedelec1(1);
  check(tr, nedelec1);

  FE_RaviartThomas<dim> rt0(0);
  check(tr, rt0);
  FE_RaviartThomas<dim> rt1(1);
  check(tr, rt1);
  FE_RaviartThomas<dim> rt2(2);
  check(tr, rt2);

  FESystem<dim> s1(q1, 3);
  check(tr, s1);
  FESystem<dim> s2(dgq1, 2, q1, 1);
  check(tr, s2);
  FESystem<dim> s3(q1, 2, dgq0, 3);
  check(tr, s3);
  FESystem<dim> s4(q1, 3, dgq0, 2, dgp1, 1);
  check(tr, s4);

  FESystem<dim> s10(rt1, 1, dgq1, 1);
  check(tr, s10);
  FESystem<dim> s11(rt0, 2, rt1, 1);
  check(tr, s11);

  FESystem<dim> ss1(s1, 2, s3, 1);
  check(tr, ss1);
}


