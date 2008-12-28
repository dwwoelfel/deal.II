//----------------------------  kelly_crash_02.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2006, 2007, 2008 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  kelly_crash_02.cc  ---------------------------


// like kelly_crash_01 but much reduced. it turned out that
// cell->face(f)->at_boundary() and cell->at_boundary(f) did not always return
// the same thing, although they of course should. as a result, the
// KellyErrorEstimator forgot to work on certain faces

char logname[] = "kelly_crash_02/output";


#include "../tests.h"


#include <base/logstream.h>
#include <base/utilities.h>
#include <grid/tria.h>
#include <grid/grid_generator.h>
#include <grid/tria_accessor.h>
#include <grid/tria_iterator.h>
#include <grid/tria_boundary_lib.h>

#include <fstream>


void test ()
{
  const unsigned int dim = 3;
  
  static const Point<3> vertices_1[]
    = 
    {
					   // points on the lower surface
	  Point<dim>(0,  0, -4),
	  Point<dim>(std::cos(0*numbers::PI/6),
		     std::sin(0*numbers::PI/6),
		     -4),
	  Point<dim>(std::cos(2*numbers::PI/6),
		     std::sin(2*numbers::PI/6),
		     -4),
	  Point<dim>(std::cos(4*numbers::PI/6),
		     std::sin(4*numbers::PI/6),
		     -4),
	  Point<dim>(std::cos(6*numbers::PI/6),
		     std::sin(6*numbers::PI/6),
		     -4),
	  Point<dim>(std::cos(8*numbers::PI/6),
		     std::sin(8*numbers::PI/6),
		     -4),
	  Point<dim>(std::cos(10*numbers::PI/6),
		     std::sin(10*numbers::PI/6),
		     -4),

					   // same points on the top
					   // of the stem, with
					   // indentation in the middle
	  Point<dim>(0,  0, 4-std::sqrt(2.)/2),
	  Point<dim>(std::cos(0*numbers::PI/6),
		     std::sin(0*numbers::PI/6),
		     4),
	  Point<dim>(std::cos(2*numbers::PI/6),
		     std::sin(2*numbers::PI/6),
		     4),
	  Point<dim>(std::cos(4*numbers::PI/6),
		     std::sin(4*numbers::PI/6),
		     4),
	  Point<dim>(std::cos(6*numbers::PI/6),
		     std::sin(6*numbers::PI/6),
		     4),
	  Point<dim>(std::cos(8*numbers::PI/6),
		     std::sin(8*numbers::PI/6),
		     4),
	  Point<dim>(std::cos(10*numbers::PI/6),
		     std::sin(10*numbers::PI/6),
		     4),

					   // point at top of chevron
	  Point<dim>(0,0,4+std::sqrt(2.)/2),

					   // points at the top of the
					   // first extension
					   // points 15-18
	  Point<dim>(0,  0, 7) + Point<dim> (std::cos(2*numbers::PI/6),
					     std::sin(2*numbers::PI/6),
					     0) * 4,
	  Point<dim>(std::cos(0*numbers::PI/6),
		     std::sin(0*numbers::PI/6),
		     7) + Point<dim> (std::cos(2*numbers::PI/6),
				      std::sin(2*numbers::PI/6),
				      0) * 4,
	  Point<dim>(std::cos(2*numbers::PI/6),
		     std::sin(2*numbers::PI/6),
		     7) + Point<dim> (std::cos(2*numbers::PI/6),
				      std::sin(2*numbers::PI/6),
				      0) * 4,
	  Point<dim>(std::cos(4*numbers::PI/6),
		     std::sin(4*numbers::PI/6),
		     7) + Point<dim> (std::cos(2*numbers::PI/6),
				      std::sin(2*numbers::PI/6),
				      0) * 4,

					   // points at the top of the
					   // second extension
					   // points 19-22
	  Point<dim>(0,  0, 7) + Point<dim> (std::cos(6*numbers::PI/6),
					     std::sin(6*numbers::PI/6),
					     0) * 4,
	  Point<dim>(std::cos(4*numbers::PI/6),
		     std::sin(4*numbers::PI/6),
		     7) + Point<dim> (std::cos(6*numbers::PI/6),
				      std::sin(6*numbers::PI/6),
				      0) * 4,
	  Point<dim>(std::cos(6*numbers::PI/6),
		     std::sin(6*numbers::PI/6),
		     7) + Point<dim> (std::cos(6*numbers::PI/6),
				      std::sin(6*numbers::PI/6),
				      0) * 4,
	  Point<dim>(std::cos(8*numbers::PI/6),
		     std::sin(8*numbers::PI/6),
		     7) + Point<dim> (std::cos(6*numbers::PI/6),
				      std::sin(6*numbers::PI/6),
				      0) * 4,

					   // points at the top of the
					   // third extension
					   // points 23-26
	  Point<dim>(0,  0, 7) + Point<dim> (std::cos(10*numbers::PI/6),
					     std::sin(10*numbers::PI/6),
					     0) * 4,
	  Point<dim>(std::cos(8*numbers::PI/6),
		     std::sin(8*numbers::PI/6),
		     7) + Point<dim> (std::cos(10*numbers::PI/6),
				      std::sin(10*numbers::PI/6),
				      0) * 4,
	  Point<dim>(std::cos(10*numbers::PI/6),
		     std::sin(10*numbers::PI/6),
		     7) + Point<dim> (std::cos(10*numbers::PI/6),
				      std::sin(10*numbers::PI/6),
				      0) * 4,
	  Point<dim>(std::cos(0*numbers::PI/6),
		     std::sin(0*numbers::PI/6),
		     7) + Point<dim> (std::cos(10*numbers::PI/6),
				      std::sin(10*numbers::PI/6),
				      0) * 4,
	  
    };
  
  const unsigned int
    n_vertices = sizeof(vertices_1) / sizeof(vertices_1[0]);
  const std::vector<Point<dim> > vertices (&vertices_1[0],
                                           &vertices_1[n_vertices]);
  static const int cell_vertices[][GeometryInfo<dim>::vertices_per_cell]
    = {
					   // the three cells in the stem
	  {0, 2, 4, 3, 7, 9, 11, 10},
	  {6, 0, 5, 4, 13, 7, 12, 11},
	  {6, 1, 0, 2, 13, 8, 7, 9},
					   // the chevron at the center
	  {13, 8, 7, 9, 12, 14, 11, 10},
					   // first extension
	  {14, 8, 10, 9, 15, 16, 18, 17},
					   // second extension
	  {11, 12, 10, 14, 21, 22, 20, 19},
					   // third extension
	  {12, 13, 14, 8, 24, 25, 23, 26},
    };
  const unsigned int
    n_cells = sizeof(cell_vertices) / sizeof(cell_vertices[0]);

  std::vector<CellData<dim> > cells (n_cells, CellData<dim>());
  for (unsigned int i=0; i<n_cells; ++i) 
    {
      for (unsigned int j=0;
           j<GeometryInfo<dim>::vertices_per_cell;
           ++j)
        cells[i].vertices[j] = cell_vertices[i][j];
      cells[i].material_id = 0;
    }

  Triangulation<3> triangulation;
  triangulation.create_triangulation (vertices,
				      cells,
				      SubCellData());

  for (Triangulation<dim>::active_cell_iterator
	 cell = triangulation.begin_active();
       cell != triangulation.end(); ++cell)
    for (unsigned int f=0; f<GeometryInfo<dim>::faces_per_cell; ++f)
      if ((cell->face(f)->center()[2] != -4)
	  &&
	  (cell->face(f)->center()[2] != 7))
	cell->face(f)->set_boundary_indicator (1);
  
  triangulation.refine_global (1);

  for (Triangulation<dim>::active_cell_iterator
	 cell = triangulation.begin_active();
       cell != triangulation.end(); ++cell)
    for (unsigned int face_no=0;
	 face_no<GeometryInfo<dim>::faces_per_cell; ++face_no)
      Assert (cell->at_boundary(face_no)
	      ==
	      cell->face(face_no)->at_boundary(),
	      ExcInternalError());
}


int main () 
{
  std::ofstream logfile(logname);
  logfile.precision (3);
  
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);
  test ();
  return 0;
}
