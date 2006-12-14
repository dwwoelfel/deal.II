//----------------------------  grid_invert.cc  ---------------------------
//    $Id: grid_in.cc 11749 2005-11-09 19:11:20Z wolf $
//    Version: $Name$ 
//
//    Copyright (C) 2002, 2003, 2004, 2005 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  grid_invert.cc  ---------------------------


#include "../tests.h"
#include <dofs/dof_handler.h>
#include <grid/grid_out.h>
#include <grid/grid_reordering.h>
#include <base/logstream.h>

#include <fstream>
#include <string>

std::ofstream logfile("grid_invert/output");

template <int dim>
void test (bool second_case=false)
{
  std::vector<Point<dim> > vertices(GeometryInfo<dim>::vertices_per_cell);
  vertices[1](1)=1;
  vertices[2](0)=1;
  vertices[2](1)=1;
  vertices[3](0)=1;
  if (dim==3)
    {
      for (unsigned int i=4; i<GeometryInfo<dim>::vertices_per_cell; ++i)
	vertices[i](2)=-1;
      vertices[5](1)=1;
      vertices[6](0)=1;
      vertices[6](1)=1;
      vertices[7](0)=1;
    }
  std::vector<CellData<dim> > cells(1);
  for (unsigned int i=0;i<GeometryInfo<dim>::vertices_per_cell; ++i)
    cells[0].vertices[i]=i;

  if (dim==3 && second_case)
    {
      std::swap(cells[0].vertices[1],cells[0].vertices[3]);
      std::swap(cells[0].vertices[5],cells[0].vertices[7]);
      for (unsigned int i=4; i<GeometryInfo<dim>::vertices_per_cell; ++i)
	vertices[i](2)=1;
    }

  SubCellData subcelldata;
  GridReordering<dim>::invert_all_cells_of_negative_grid(vertices,cells);

  Triangulation<dim> tria;
  tria.create_triangulation_compatibility(vertices,cells,subcelldata);

  logfile<<"---------------------------------------------"
	 <<std::endl<<"dim="<<dim
	 <<(second_case ? ", second case" : ", first case")
	 <<std::endl<<std::endl;

  GridOut grid_out;
  grid_out.write_ucd (tria, logfile);
}

int main ()
{
  test<2> ();
  test<3> (false);
  test<3> (true);
}

