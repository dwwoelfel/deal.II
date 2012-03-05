//----------------------------  vectors_rhs_hp_02.cc  ---------------------------
//    $Id: vectors_rhs_hp_02.cc 23710 2011-05-17 04:50:10Z bangerth $
//    Version: $Name$
//
//    Copyright (C) 2000, 2001, 2003, 2004, 2006, 2007, 2011 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  ---------------------------


// Currently fails. We get this crash in 3d with the quarter shell and no normal flux conditions:
//
//An error occurred in line <2571> of file </w/heister/deal-trunk/deal.II/include/deal.II/numerics/vectors.templates.h> in function
//    void dealii::VectorTools::internal::compute_orthonormal_vectors(const dealii::Tensor<1, dim>&, dealii::Tensor<1, dim> (&)[(dim - 1)]) [with int dim = 3]
//The violated condition was:
//    std::fabs(vector * tmp) < 1e-12
//The name and call sequence of the exception was:
//    ExcInternalError()
//Additional Information:
//(none)


#include "../tests.h"

#include <deal.II/grid/tria.h>
#include <deal.II/grid/grid_generator.h>
#include <deal.II/hp/dof_handler.h>
#include <deal.II/lac/constraint_matrix.h>
#include <deal.II/fe/fe_system.h>
#include <deal.II/fe/mapping_q.h>
#include <deal.II/numerics/vectors.h>


template <int dim>
void
check ()
{
  Triangulation<dim> tr;
  GridGenerator::quarter_hyper_shell (tr,
                                      Point<dim>(),
                                      0.5, 1.0,
                                      3, true);

  ConstraintMatrix cm;
  MappingQ<dim> mapping(1);

  FESystem<dim> fe(FE_Q<dim>(1),dim);
  DoFHandler<dim> dofh(tr);

  dofh.distribute_dofs (fe);

  std::set<unsigned char> no_normal_flux_boundaries;
  no_normal_flux_boundaries.insert (1);
  //  no_normal_flux_boundaries.insert (2); // not required for the crash for now, please test with it later!
  no_normal_flux_boundaries.insert (3);
  no_normal_flux_boundaries.insert (4);
  VectorTools::compute_no_normal_flux_constraints (dofh, 0, no_normal_flux_boundaries, cm, mapping);

  cm.print (deallog.get_file_stream ());
}



int main ()
{
  std::ofstream logfile ("no_flux_09/output");
  logfile.precision (4);
  logfile.setf(std::ios::fixed);
  deallog.attach(logfile);
  deallog.depth_console (0);

  check<3> ();
}
