//----------------------------  work_stream_03.cc  ---------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 2006, 2013 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  work_stream_03.cc  ---------------------------


// Moritz originally implemented thread local scratch objects for
// WorkStream in r24748 but it led to failures in the testsuite. what
// exactly went on was a mystery and this test is a first step in
// figuring out what happens by running a simplified version of one of
// the failing tests (deal.II/project_q_01) multiple times and
// verifying that it indeed works

#include "../tests.h"
#include <deal.II/base/logstream.h>
#include <deal.II/base/quadrature_lib.h>
#include <deal.II/base/work_stream.h>
#include <deal.II/lac/vector.h>

#include <deal.II/grid/tria.h>
#include <deal.II/dofs/dof_handler.h>
#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/tria_accessor.h>
#include <deal.II/grid/tria_iterator.h>
#include <deal.II/dofs/dof_accessor.h>
#include <deal.II/fe/fe_nothing.h>
#include <deal.II/fe/fe_values.h>

#include <fstream>
#include <vector>

char logname[] = "work_stream_03/output";



template <int dim>
double value (const Point<dim> &p)
{
  double val = 0;
  for (unsigned int d=0; d<dim; ++d)
    for (unsigned int i=0; i<=1; ++i)
      val += std::pow (p[d], 1.*i);
  return val;
}

namespace
{
  template <int dim>
  struct Scratch
  {
    Scratch (const FiniteElement<dim> &fe,
             const Quadrature<dim> &quadrature)
      :
      fe_collection (fe),
      quadrature_collection (quadrature),
      x_fe_values (fe_collection,
                   quadrature_collection,
                   update_q_points),
      rhs_values(quadrature_collection.size())
    {}

    Scratch (const Scratch &data)
      :
      fe_collection (data.fe_collection),
      quadrature_collection (data.quadrature_collection),
      x_fe_values (fe_collection,
                   quadrature_collection,
                   update_q_points),
      rhs_values (data.rhs_values)
    {}

    const FiniteElement<dim>      &fe_collection;
    const Quadrature<dim>                &quadrature_collection;

    FEValues<dim> x_fe_values;

    std::vector<double>                  rhs_values;
  };


  struct CopyData
  {
    Vector<double>    cell_rhs;
  };
}

template<int dim, typename CellIterator>
  void
  mass_assembler(const CellIterator &cell, Scratch<dim> &data,
      CopyData &copy_data)
  {
    data.x_fe_values.reinit(cell);

    // this appears to be the key: the following line overwrites some of the memory
    // in which we store the quadrature point location. if the line is moved down,
    // the comparison in the if() always succeeds...
    copy_data.cell_rhs = 0;
    if (cell->center().distance (data.x_fe_values.quadrature_point(0)) >= 1e-6 *cell->diameter())
      std::cout << '.' << std::flush;
    else
      std::cout << '*' << std::flush;

    copy_data.cell_rhs[0] = value(data.x_fe_values.quadrature_point(0));
  }


void copy_local_to_global (const CopyData &data,
                           double *sum)
{
  *sum += data.cell_rhs[0];
}


void do_project ()
{
  static const int dim = 3;

  Triangulation<dim>     triangulation;
  GridGenerator::hyper_cube (triangulation);
  triangulation.refine_global (2);

  FE_Nothing<3> fe;
  DoFHandler<dim>        dof_handler(triangulation);
  dof_handler.distribute_dofs (fe);

  QMidpoint<dim> q;

  for (unsigned int i=0; i<12; ++i)
    {
      std::vector<double> tmp;

      double sum = 0;
      Scratch<dim> assembler_data (dof_handler.get_fe(), q);
      CopyData copy_data;
      copy_data.cell_rhs.reinit (8);
      dealii::WorkStream::run (dof_handler.begin_active(),
                       dof_handler.end(),
                       &mass_assembler<dim, typename DoFHandler<dim>::active_cell_iterator>,
                       std_cxx1x::bind(&
                                       copy_local_to_global,
                                       std_cxx1x::_1, &sum),
                       assembler_data,
                       copy_data,
                       8,
                       1);
      printf ("\nCheck: %5.3f\n", sum);
    }
}


int main ()
{
  std::ofstream logfile(logname);
  deallog << std::setprecision (3);

  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  do_project ();
}
