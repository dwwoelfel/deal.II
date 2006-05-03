//----------------------------  step-10.cc  ---------------------------
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
//----------------------------  step-10.cc  ---------------------------


// a hp-ified version of step-10


#include <base/logstream.h>
#include <fstream>
std::ofstream logfile("step-10/output");


#include <base/quadrature_lib.h>
#include <base/convergence_table.h>
#include <grid/grid_generator.h>
#include <grid/tria_accessor.h>
#include <grid/tria_iterator.h>
#include <grid/tria_boundary_lib.h>
#include <grid/tria.h>
#include <grid/grid_out.h>
#include <dofs/hp_dof_handler.h>
#include <dofs/dof_accessor.h>
#include <fe/fe_q.h>
#include <fe/hp_fe_values.h>

#include <fe/mapping_q.h>

#include <iostream>
#include <fstream>
#include <cmath>

const long double pi = 3.141592653589793238462643;



template <int dim>
void gnuplot_output()
{
  deallog << "Output of grids into gnuplot files:" << std::endl
	    << "===================================" << std::endl;

  Triangulation<dim> triangulation;
  GridGenerator::hyper_ball (triangulation);
  static const HyperBallBoundary<dim> boundary;
  triangulation.set_boundary (0, boundary);

  for (unsigned int refinement=0; refinement<2;
       ++refinement, triangulation.refine_global(1))
    {
      deallog << "Refinement level: " << refinement << std::endl;

      std::string filename_base = "ball";
      filename_base += '0'+refinement;

      for (unsigned int degree=1; degree<4; ++degree)
	{
	  deallog << "Degree = " << degree << std::endl;

	  const MappingQ<dim> mapping (degree);


	  GridOut grid_out;
	  GridOutFlags::Gnuplot gnuplot_flags(false, 30);
	  grid_out.set_flags(gnuplot_flags);
  
	  std::string filename = filename_base+"_mapping_q";
	  filename += ('0'+degree);
	  filename += ".dat";
	  std::ofstream gnuplot_file (filename.c_str());

	  grid_out.write_gnuplot (triangulation, gnuplot_file, &mapping);
	}
      deallog << std::endl;
    }
}

template <int dim>
void compute_pi_by_area ()
{
  deallog << "Computation of Pi by the area:" << std::endl
	    << "==============================" << std::endl;

  const hp::QCollection<dim> quadrature(QGauss<dim>(4));

  for (unsigned int degree=1; degree<5; ++degree)
    {
      deallog << "Degree = " << degree << std::endl;

      Triangulation<dim> triangulation;
      GridGenerator::hyper_ball (triangulation);
  
      static const HyperBallBoundary<dim> boundary;
      triangulation.set_boundary (0, boundary);

      MappingQ<dim> m(degree);
      const hp::MappingCollection<dim> mapping (m);

      const hp::FECollection<dim>     dummy_fe (FE_Q<dim>(1));

      hp::DoFHandler<dim> dof_handler (triangulation);

      hp::FEValues<dim> x_fe_values (mapping, dummy_fe, quadrature,
                               update_JxW_values);

      ConvergenceTable table;

      for (unsigned int refinement=0; refinement<6;
	   ++refinement, triangulation.refine_global (1))
	{
	  table.add_value("cells", triangulation.n_active_cells());

	  dof_handler.distribute_dofs (dummy_fe);

	  long double area = 0;

	  typename hp::DoFHandler<dim>::active_cell_iterator
	    cell = dof_handler.begin_active(),
	    endc = dof_handler.end();
	  for (; cell!=endc; ++cell)
	    {
	      x_fe_values.reinit (cell);
	      const FEValues<dim> &fe_values = x_fe_values.get_present_fe_values();
	      for (unsigned int i=0; i<fe_values.n_quadrature_points; ++i)
		area += fe_values.JxW (i);
	    };

	  table.add_value("eval.pi", static_cast<double> (area));
	  table.add_value("error",   static_cast<double> (std::fabs(area-pi)));
	};

      table.omit_column_from_convergence_rate_evaluation("cells");
      table.omit_column_from_convergence_rate_evaluation("eval.pi");
      table.evaluate_all_convergence_rates(ConvergenceTable::reduction_rate_log2);

      table.set_precision("eval.pi", 16);
      table.set_scientific("error", true);

      table.write_text(deallog.get_file_stream());

      deallog << std::endl;
    };
}


template <int dim>
void compute_pi_by_perimeter ()
{
  deallog << "Computation of Pi by the perimeter:" << std::endl
	    << "===================================" << std::endl;

  const hp::QCollection<dim-1> quadrature(4);

  for (unsigned int degree=1; degree<5; ++degree)
    {
      deallog << "Degree = " << degree << std::endl;
      Triangulation<dim> triangulation;
      GridGenerator::hyper_ball (triangulation);
  
      static const HyperBallBoundary<dim> boundary;
      triangulation.set_boundary (0, boundary);

      const MappingQ<dim> m (degree);
      const hp::MappingCollection<dim> mapping(m);
      const hp::FECollection<dim>     fe (FE_Q<dim>(1));

      hp::DoFHandler<dim> dof_handler (triangulation);

      hp::FEFaceValues<dim> x_fe_face_values (mapping, fe, quadrature,
                                        update_JxW_values);
      ConvergenceTable table;

      for (unsigned int refinement=0; refinement<6;
	   ++refinement, triangulation.refine_global (1))
	{
	  table.add_value("cells", triangulation.n_active_cells());

	  dof_handler.distribute_dofs (fe);

	  typename hp::DoFHandler<dim>::active_cell_iterator
	    cell = dof_handler.begin_active(),
	    endc = dof_handler.end();
	  long double perimeter = 0;
	  for (; cell!=endc; ++cell)
	    for (unsigned int face_no=0; face_no<GeometryInfo<dim>::faces_per_cell; ++face_no)
	      if (cell->face(face_no)->at_boundary())
		{
		  x_fe_face_values.reinit (cell, face_no);
		  const FEFaceValues<dim> &fe_face_values
		    = x_fe_face_values.get_present_fe_values ();
		  
		  for (unsigned int i=0; i<fe_face_values.n_quadrature_points; ++i)
		    perimeter += fe_face_values.JxW (i);
		};
	  table.add_value("eval.pi", static_cast<double> (perimeter/2.));
	  table.add_value("error",   static_cast<double> (std::fabs(perimeter/2.-pi)));
	};

      table.omit_column_from_convergence_rate_evaluation("cells");
      table.omit_column_from_convergence_rate_evaluation("eval.pi");
      table.evaluate_all_convergence_rates(ConvergenceTable::reduction_rate_log2);

      table.set_precision("eval.pi", 16);
      table.set_scientific("error", true);

      table.write_text(deallog.get_file_stream());

      deallog << std::endl;
    };
}


int main () 
{
  logfile.precision(2);
  
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);  

  gnuplot_output<2>();

  compute_pi_by_area<2> ();
  compute_pi_by_perimeter<2> ();
  
  return 0;
}
