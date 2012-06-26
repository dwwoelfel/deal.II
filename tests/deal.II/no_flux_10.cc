//----------------------------  vectors_rhs_hp_02.cc  ---------------------------
//    $Id: vectors_rhs_hp_02.cc 23710 2011-05-17 04:50:10Z bangerth $
//    Version: $Name$
//
//    Copyright (C) 2012 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  ---------------------------


// test by Jennifer Worthen -- we get an error of the following kind:
// --------------------------------------------------------
// An error occurred in line <304> of file <.../constraint_matrix.cc> in function
//     void dealii::ConstraintMatrix::close()
// The violated condition was:
//     dof_index != line->line
// The name and call sequence of the exception was:
//     ExcMessage ("Cycle in constraints detected!")
// Additional Information:
// Cycle in constraints detected!
// --------------------------------------------------------



#include "../tests.h"

#include <grid/tria.h>
#include <grid/grid_generator.h>
#include <grid/tria_boundary_lib.h>
#include <dofs/dof_handler.h>
#include <fe/fe_system.h>
#include <fe/mapping_q1.h>
#include <numerics/vectors.h>
#include <numerics/data_out.h>
#include <base/exceptions.h>
#include <base/function.h>


template <int dim>
static void
colorize_sixty_deg_hyper_shell(Triangulation<dim> & tria,
			       const Point<dim>& center,
			       const double inner_radius,
			       const double outer_radius);

template<int dim>
static void sixty_deg_hyper_shell (Triangulation<dim>   &tria,
				   const Point<dim>     &center,
				   const double        inner_radius,
				   const double        outer_radius,
				   const unsigned int  n_cells = 0,
				   const bool colorize = false);

template <int dim>
class SixtyDegHyperShellBoundary : public HyperShellBoundary<dim>
{
  public:

    SixtyDegHyperShellBoundary (const Point<dim> &center,
				const double inner_radius,
				const double outer_radius);

  private:
    const double inner_radius;
    const double outer_radius;
};

// Implementation for 3D only
template <>
void
colorize_sixty_deg_hyper_shell(Triangulation<3> & tria,
			       const Point<3>& center,
			       const double inner_radius,
			       const double outer_radius)
{

				   //    if (tria.n_cells() != 4)
				   //      AssertThrow (false, ExcNotImplemented());

  double middle = (outer_radius-inner_radius)/2e0 + inner_radius;
  double eps = 1e-3*middle;
  Triangulation<3>::cell_iterator cell = tria.begin();

  for (;cell!=tria.end();++cell)
    for(unsigned int f=0; f<GeometryInfo<3>::faces_per_cell; ++f)
      {
        if(!cell->face(f)->at_boundary())
          continue;

        double radius = cell->face(f)->center().norm() - center.norm();
        if (std::fabs(cell->face(f)->center()(2) - sqrt(3.)*cell->face(f)->center()(0)) < eps ) // z = sqrt(3)x set boundary 2
          {
            cell->face(f)->set_boundary_indicator(2);
            for (unsigned int j=0;j<GeometryInfo<3>::lines_per_face;++j)
              if(cell->face(f)->line(j)->at_boundary())
                if (std::fabs(cell->face(f)->line(j)->vertex(0).norm() - cell->face(f)->line(j)->vertex(1).norm()) > eps)
                  cell->face(f)->line(j)->set_boundary_indicator(2);
          }
        else if (std::fabs(cell->face(f)->center()(2) + sqrt(3.)*cell->face(f)->center()(0)) < eps) // z = -sqrt(3)x set boundary 3
          {
            cell->face(f)->set_boundary_indicator(3);
            for (unsigned int j=0;j<GeometryInfo<3>::lines_per_face;++j)
              if(cell->face(f)->line(j)->at_boundary())
                if (std::fabs(cell->face(f)->line(j)->vertex(0).norm() - cell->face(f)->line(j)->vertex(1).norm()) > eps)
                  cell->face(f)->line(j)->set_boundary_indicator(3);
          }
        else if (std::fabs(cell->face(f)->center()(2) - sqrt(3.)*cell->face(f)->center()(1)) < eps ) // z = sqrt(3)y set boundary 4
          {
            cell->face(f)->set_boundary_indicator(4);
            for (unsigned int j=0;j<GeometryInfo<3>::lines_per_face;++j)
              if(cell->face(f)->line(j)->at_boundary())
                if (std::fabs(cell->face(f)->line(j)->vertex(0).norm() - cell->face(f)->line(j)->vertex(1).norm()) > eps)
                  cell->face(f)->line(j)->set_boundary_indicator(4);
          }
        else if (std::fabs(cell->face(f)->center()(2) + sqrt(3.)*cell->face(f)->center()(1)) < eps ) // z = -sqrt(3)y set boundary 5
          {
            cell->face(f)->set_boundary_indicator(5);
            for (unsigned int j=0;j<GeometryInfo<3>::lines_per_face;++j)
              if(cell->face(f)->line(j)->at_boundary())
                if (std::fabs(cell->face(f)->line(j)->vertex(0).norm() - cell->face(f)->line(j)->vertex(1).norm()) > eps)
                  cell->face(f)->line(j)->set_boundary_indicator(5);
          }
        else if (radius < middle) // inner radius set boundary 0
          {
            cell->face(f)->set_boundary_indicator(0);
            for (unsigned int j=0;j<GeometryInfo<3>::lines_per_face;++j)
              if(cell->face(f)->line(j)->at_boundary())
                if (std::fabs(cell->face(f)->line(j)->vertex(0).norm() - cell->face(f)->line(j)->vertex(1).norm()) < eps)
                  cell->face(f)->line(j)->set_boundary_indicator(0);
          }
        else if (radius > middle) // outer radius set boundary 1
          {
            cell->face(f)->set_boundary_indicator(1);
            for (unsigned int j=0;j<GeometryInfo<3>::lines_per_face;++j)
              if(cell->face(f)->line(j)->at_boundary())
                if (std::fabs(cell->face(f)->line(j)->vertex(0).norm() - cell->face(f)->line(j)->vertex(1).norm()) < eps)
		  cell->face(f)->line(j)->set_boundary_indicator(1);
          }
        else
          AssertThrow (false, ExcInternalError());
      }
}

// Implementation for 3D only
template <>
void sixty_deg_hyper_shell (Triangulation<3> & tria,
			    const Point<3>& center,
			    const double inner_radius,
			    const double outer_radius,
			    const unsigned int n,
			    const bool colorize)
{
				   //  Assert ((inner_radius > 0) && (inner_radius < outer_radius),
				   //          ExcInvalidRadii ());
  if (n == 0 || n == 2)
    {
      const double r0 = inner_radius;
      const double r1 = outer_radius;

      std::vector<Point<3> > vertices;

      vertices.push_back (center+Point<3>( 1.0/sqrt(5.)*r0, 1.0/sqrt(5.)*r0, sqrt(3./5.)*r0));   //8 -> 0
      vertices.push_back (center+Point<3>( 1.0/sqrt(5.)*r1, 1.0/sqrt(5.)*r1, sqrt(3./5.)*r1));   //9 -> 1
      vertices.push_back (center+Point<3>( 1.0/sqrt(5.)*r0, -1.0/sqrt(5.)*r0, sqrt(3./5.)*r0));  //10 -> 2
      vertices.push_back (center+Point<3>( 1.0/sqrt(5.)*r1, -1.0/sqrt(5.)*r1, sqrt(3./5.)*r1));  //11 -> 3
      vertices.push_back (center+Point<3>( -1.0/sqrt(5.)*r0, 1.0/sqrt(5.)*r0, sqrt(3./5.)*r0));  //14 -> 4
      vertices.push_back (center+Point<3>( -1.0/sqrt(5.)*r1, 1.0/sqrt(5.)*r1, sqrt(3./5.)*r1));  //15 -> 5
      vertices.push_back (center+Point<3>( -1.0/sqrt(5.)*r0, -1.0/sqrt(5.)*r0, sqrt(3./5.)*r0)); //16 -> 6
      vertices.push_back (center+Point<3>( -1.0/sqrt(5.)*r1, -1.0/sqrt(5.)*r1, sqrt(3./5.)*r1)); //17 -> 7

      const int cell_vertices[1][8] = {
	    {6, 2, 4, 0, 7, 3, 5, 1},
      };

      std::vector<CellData<3> > cells(1);

      for (unsigned int i=0; i<1; ++i)
        {
          for (unsigned int j=0; j<8; ++j)
            cells[i].vertices[j] = cell_vertices[i][j];
          cells[i].material_id = 0;
        }

      tria.create_triangulation ( vertices, cells, SubCellData());      // no boundary information
    }
  else
    {
      AssertThrow(false, ExcNotImplemented());
    }

  if (colorize)
    colorize_sixty_deg_hyper_shell(tria, center, inner_radius, outer_radius);
}

template<int dim>
SixtyDegHyperShellBoundary<dim>::SixtyDegHyperShellBoundary (const Point<dim> &center,
							     const double inner_radius,
							     const double outer_radius)
                :
                HyperShellBoundary<dim> (center),
                inner_radius (inner_radius),
                outer_radius (outer_radius)
{
  if (dim > 2)
    Assert ((inner_radius >= 0) &&
            (outer_radius > 0) &&
            (outer_radius > inner_radius),
            ExcMessage ("Inner and outer radii must be specified explicitly in 3d."));
}

template <int dim>
void run()
{
  Triangulation<dim> triangulation;
  FESystem<dim> fe(FE_Q<dim>(1), dim);
  DoFHandler<dim> dof_handler (triangulation);
  ConstraintMatrix constraints;

  sixty_deg_hyper_shell (triangulation,
			 Point<dim>(),
			 0.5,
			 1.0,
			 2,
			 true);

  static SixtyDegHyperShellBoundary<dim> boundary(Point<dim>(),
						  0.5,
						  1.0);
  triangulation.set_boundary (0, boundary);
  triangulation.set_boundary (1, boundary);

  triangulation.refine_global(2);

  dof_handler.distribute_dofs (fe);

  std::set<unsigned char> no_normal_flux_boundaries;
  no_normal_flux_boundaries.insert (0);
  no_normal_flux_boundaries.insert (2);
  VectorTools::compute_no_normal_flux_constraints
    (dof_handler, 0,
     no_normal_flux_boundaries,
     constraints);

  constraints.close();
  constraints.print(deallog.get_file_stream());


  deallog << "OK" << std::endl;
}


int main ()
{
  std::ofstream logfile ("no_flux_10/output");
  logfile.precision (4);
  logfile.setf(std::ios::fixed);
  deallog.attach(logfile);
  deallog.depth_console (0);

  run<3> ();
}
