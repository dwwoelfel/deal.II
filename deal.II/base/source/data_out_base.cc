//---------------------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------


#include <base/data_out_base.h>
#include <base/parameter_handler.h>
#include <base/thread_management.h>
#include <base/memory_consumption.h>

#include <algorithm>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <set>

#ifdef HAVE_STD_STRINGSTREAM
#  include <sstream>
#else
#  include <strstream>
#endif


DeclException2 (ExcUnexpectedInput,
                std::string, std::string,
                << "Unexpected input: expected line\n  <"
                << arg1
                << ">\nbut got\n  <"
                << arg2 << ">");

DeclException4 (ExcIncompatibleDimensions,
                int, int, int, int,
                << "Either the dimensions <" << arg1 << "> and <"
                << arg2 << "> or the space dimensions <"
                << arg3 << "> and <" << arg4
                << "> do not match!");


template <int dim, int spacedim>
const unsigned int DataOutBase::Patch<dim,spacedim>::no_neighbor;


template <int dim, int spacedim>
DataOutBase::Patch<dim,spacedim>::Patch ()
                :
		patch_index(no_neighbor),
		n_subdivisions (1)
				   // all the other data has a
				   // constructor of its own, except
				   // for the "neighbors" field, which
				   // we set to invalid values.
{
  for (unsigned int i=0;i<GeometryInfo<dim>::faces_per_cell;++i)
    neighbors[i] = no_neighbor;
  
  Assert (dim<=spacedim, ExcIndexRange(dim,0,spacedim));
  Assert (spacedim<=3, ExcNotImplemented());
}



template <int dim, int spacedim>
bool
DataOutBase::Patch<dim,spacedim>::operator == (const Patch &patch) const
{
  for (unsigned int i=0; i<GeometryInfo<dim>::vertices_per_cell; ++i)
    if (vertices[i] != patch.vertices[i])
      return false;

  for (unsigned int i=0; i<GeometryInfo<dim>::faces_per_cell; ++i)
    if (neighbors[i] != patch.neighbors[i])
      return false;

  if (patch_index != patch.patch_index)
    return false;

  if (n_subdivisions != patch.n_subdivisions)
    return false;

  if (data.n_rows() != patch.data.n_rows())
    return false;

  if (data.n_cols() != patch.data.n_cols())
    return false;

  for (unsigned int i=0; i<data.n_rows(); ++i)
    for (unsigned int j=0; j<data.n_cols(); ++j)
      if (data[i][j] != patch.data[i][j])
	return false;
  
  return true;
}



template <int dim, int spacedim>
unsigned int
DataOutBase::Patch<dim,spacedim>::memory_consumption () const
{
  return (sizeof(vertices) / sizeof(vertices[0]) * 
	  MemoryConsumption::memory_consumption(vertices[0])
	  +
	  MemoryConsumption::memory_consumption(n_subdivisions)
	  +
	  MemoryConsumption::memory_consumption(data));
}



DataOutBase::UcdFlags::UcdFlags (const bool write_preamble)
                :
		write_preamble (write_preamble)
{}



DataOutBase::PovrayFlags::PovrayFlags (const bool smooth,
				       const bool bicubic_patch,
				       const bool external_data)
                :
		smooth (smooth),
		bicubic_patch(bicubic_patch),
		external_data(external_data)
{}


DataOutBase::DXFlags::DXFlags (const bool write_neighbors)
                :
		write_neighbors(write_neighbors)
{}


void DataOutBase::DXFlags::declare_parameters (ParameterHandler &prm)
{
  prm.declare_entry ("Write neighbors", "true",
                     Patterns::Bool(),
                     "A boolean field indicating whether neighborship "
                     "information between cells is to be written to the "
                     "OpenDX output file");
}



void DataOutBase::DXFlags::parse_parameters (ParameterHandler &prm)
{
  write_neighbors = prm.get_bool ("Write neighbors");
}



unsigned int
DataOutBase::DXFlags::memory_consumption () const
{
				   // only simple data elements, so
				   // use sizeof operator
  return sizeof (*this);
}




void DataOutBase::UcdFlags::declare_parameters (ParameterHandler &prm)
{
  prm.declare_entry ("Write preamble", "true",
                     Patterns::Bool(),
                     "A flag indicating whether a comment should be "
                     "written to the beginning of the output file "
                     "indicating date and time of creation as well "
                     "as the creating program");
}



void DataOutBase::UcdFlags::parse_parameters (ParameterHandler &prm)
{
  write_preamble = prm.get_bool ("Write preamble");
}


unsigned int
DataOutBase::UcdFlags::memory_consumption () const
{
				   // only simple data elements, so
				   // use sizeof operator
  return sizeof (*this);
}



void DataOutBase::GnuplotFlags::declare_parameters (ParameterHandler &/*prm*/)
{}



void DataOutBase::GnuplotFlags::parse_parameters (ParameterHandler &/*prm*/)
{}



unsigned int
DataOutBase::GnuplotFlags::memory_consumption () const
{
				   // only simple data elements, so
				   // use sizeof operator
  return sizeof (*this);
}




void DataOutBase::PovrayFlags::declare_parameters (ParameterHandler &prm)
{
  prm.declare_entry ("Use smooth triangles", "false",
		     Patterns::Bool(),
                     "A flag indicating whether POVRAY should use smoothed "
                     "triangles instead of the usual ones");
  prm.declare_entry ("Use bicubic patches", "false",
		     Patterns::Bool(),
                     "Whether POVRAY should use bicubic patches");
  prm.declare_entry ("Include external file", "true",
		     Patterns::Bool (),
                     "Whether camera and lightling information should "
                     "be put into an external file \"data.inc\" or into "
                     "the POVRAY input file");
}



void DataOutBase::PovrayFlags::parse_parameters (ParameterHandler &prm)
{
  smooth        = prm.get_bool ("Use smooth triangles");
  bicubic_patch = prm.get_bool ("Use bicubic patches");
  external_data = prm.get_bool ("Include external file");
}



unsigned int
DataOutBase::PovrayFlags::memory_consumption () const
{
				   // only simple data elements, so
				   // use sizeof operator
  return sizeof (*this);
}



DataOutBase::EpsFlags::EpsFlags (const unsigned int  height_vector,
				 const unsigned int  color_vector,
				 const SizeType      size_type,
				 const unsigned int  size,
				 const double        line_width,
				 const double        azimut_angle,
				 const double        turn_angle,
				 const double        z_scaling,
				 const bool          draw_mesh,
				 const bool          draw_cells,
				 const bool          shade_cells,
				 const ColorFunction color_function)
                :
		height_vector(height_vector),
		color_vector(color_vector),
		size_type(size_type),
		size(size),
		line_width(line_width),
		azimut_angle(azimut_angle),
		turn_angle(turn_angle),
		z_scaling(z_scaling),
		draw_mesh(draw_mesh),
		draw_cells(draw_cells),
		shade_cells(shade_cells),
		color_function(color_function)
{}



DataOutBase::EpsFlags::RgbValues
DataOutBase::EpsFlags::default_color_function (const double x,
					       const double xmin,
					       const double xmax)
{
  RgbValues rgb_values = { 0,0,0 };
  
// A difficult color scale:
//     xmin          = black  (1)
// 3/4*xmin+1/4*xmax = blue   (2)
// 1/2*xmin+1/2*xmax = green  (3)
// 1/4*xmin+3/4*xmax = red    (4)
//              xmax = white  (5)
// Makes the following color functions:
//
// red      green    blue
//       __
//      /      /\  /  /\    /
// ____/    __/  \/  /  \__/

//     { 0                                (1) - (3)
// r = { ( 4*x-2*xmin+2*xmax)/(xmax-xmin) (3) - (4)
//     { 1                                (4) - (5)
//
//     { 0                                (1) - (2)
// g = { ( 4*x-3*xmin-  xmax)/(xmax-xmin) (2) - (3)
//     { (-4*x+  xmin+3*xmax)/(xmax-xmin) (3) - (4)
//     { ( 4*x-  xmin-3*xmax)/(xmax-xmin) (4) - (5)
//
//     { ( 4*x-4*xmin       )/(xmax-xmin) (1) - (2)
// b = { (-4*x+2*xmin+2*xmax)/(xmax-xmin) (2) - (3)
//     { 0                                (3) - (4)
//     { ( 4*x-  xmin-3*xmax)/(xmax-xmin) (4) - (5)

  double sum   =   xmax+  xmin;
  double sum13 =   xmin+3*xmax;
  double sum22 = 2*xmin+2*xmax;
  double sum31 = 3*xmin+  xmax;
  double dif = xmax-xmin;
  double rezdif = 1.0/dif;

  int where;

  if (x<(sum31)/4)
    where = 0;
  else if (x<(sum22)/4)
    where = 1;
  else if (x<(sum13)/4)
    where = 2;
  else
    where = 3;

  if (dif!=0)
    {
      switch (where)
	{
	  case 0:
		rgb_values.red   = 0;
		rgb_values.green = 0;
		rgb_values.blue  = (x-xmin)*4.*rezdif;
		break;
	  case 1:
		rgb_values.red   = 0;
		rgb_values.green = (4*x-3*xmin-xmax)*rezdif;
		rgb_values.blue  = (sum22-4.*x)*rezdif;
		break;
	  case 2:
		rgb_values.red   = (4*x-2*sum)*rezdif;
		rgb_values.green = (xmin+3*xmax-4*x)*rezdif;
		rgb_values.blue  = 0;
		break;
	  case 3:
		rgb_values.red   = 1;
		rgb_values.green = (4*x-xmin-3*xmax)*rezdif;
		rgb_values.blue  = (4.*x-sum13)*rezdif;
	  default:
		break;
	}
    }
  else // White 
    rgb_values.red = rgb_values.green = rgb_values.blue = 1;

  return rgb_values;
}



DataOutBase::EpsFlags::RgbValues
DataOutBase::EpsFlags::grey_scale_color_function (const double x,
						  const double xmin,
						  const double xmax)
{
  DataOutBase::EpsFlags::RgbValues rgb_values;
  rgb_values.red = rgb_values.blue = rgb_values.green
		 = (x-xmin)/(xmax-xmin);
  return rgb_values;
}



DataOutBase::EpsFlags::RgbValues
DataOutBase::EpsFlags::reverse_grey_scale_color_function (const double x,
							  const double xmin,
							  const double xmax)
{
  DataOutBase::EpsFlags::RgbValues rgb_values;
  rgb_values.red = rgb_values.blue = rgb_values.green
		 = 1-(x-xmin)/(xmax-xmin);
  return rgb_values;
}



bool DataOutBase::EpsCell2d::operator < (const EpsCell2d &e) const
{
				   // note the "wrong" order in
				   // which we sort the elements
  return depth > e.depth;
}



void DataOutBase::EpsFlags::declare_parameters (ParameterHandler &prm)
{
  prm.declare_entry ("Index of vector for height", "0",
		     Patterns::Integer(),
                     "Number of the input vector that is to be used to "
                     "generate height information");
  prm.declare_entry ("Index of vector for color", "0",
		     Patterns::Integer(),
                     "Number of the input vector that is to be used to "
                     "generate color information");
  prm.declare_entry ("Scale to width or height", "width",
		     Patterns::Selection ("width|height"),
                     "Whether width or height should be scaled to match "
                     "the given size");
  prm.declare_entry ("Size (width or height) in eps units", "300",
		     Patterns::Integer(),
                     "The size (width or height) to which the eps output "
                     "file is to be scaled");
  prm.declare_entry ("Line widths in eps units", "0.5",
		     Patterns::Double(),
                     "The width in which the postscript renderer is to "
                     "plot lines");
  prm.declare_entry ("Azimut angle", "60",
		     Patterns::Double(0,180),
                     "Angle of the viewing position against the vertical "
                     "axis");
  prm.declare_entry ("Turn angle", "30",
		     Patterns::Double(0,360),
                     "Angle of the viewing direction against the y-axis");
  prm.declare_entry ("Scaling for z-axis", "1",
		     Patterns::Double (),
                     "Scaling for the z-direction relative to the scaling "
                     "used in x- and y-directions");
  prm.declare_entry ("Draw mesh lines", "true",
		     Patterns::Bool(),
                     "Whether the mesh lines, or only the surface should be "
                     "drawn");
  prm.declare_entry ("Fill interior of cells", "true",
		     Patterns::Bool(),
                     "Whether only the mesh lines, or also the interior of "
                     "cells should be plotted. If this flag is false, then "
                     "one can see through the mesh");
  prm.declare_entry ("Color shading of interior of cells", "true",
		     Patterns::Bool(),
                     "Whether the interior of cells shall be shaded");
  prm.declare_entry ("Color function", "default",
		     Patterns::Selection ("default|grey scale|reverse grey scale"),
                     "Name of a color function used to colorize mesh lines "
                     "and/or cell interiors");
}



void DataOutBase::EpsFlags::parse_parameters (ParameterHandler &prm)
{
  height_vector = prm.get_integer ("Index of vector for height");
  color_vector  = prm.get_integer ("Index of vector for color");
  if (prm.get ("Scale to width or height") == "width")
    size_type   = width;
  else
    size_type   = height;
  size          = prm.get_integer ("Size (width or height) in eps units");
  line_width    = prm.get_double ("Line widths in eps units");
  azimut_angle  = prm.get_double ("Azimut angle");
  turn_angle    = prm.get_double ("Turn angle");
  z_scaling     = prm.get_double ("Scaling for z-axis");
  draw_mesh     = prm.get_bool ("Draw mesh lines");
  draw_cells    = prm.get_bool ("Fill interior of cells");
  shade_cells   = prm.get_bool ("Color shading of interior of cells");
  if (prm.get("Color function") == "default")
    color_function = &default_color_function;
  else if (prm.get("Color function") == "grey scale")
    color_function = &grey_scale_color_function;
  else if (prm.get("Color function") == "reverse grey scale")
    color_function = &reverse_grey_scale_color_function;
  else
				     // we shouldn't get here, since
				     // the parameter object should
				     // already have checked that the
				     // given value is valid
    Assert (false, ExcInternalError());
}



unsigned int
DataOutBase::EpsFlags::memory_consumption () const
{
				   // only simple data elements, so
				   // use sizeof operator
  return sizeof (*this);
}



void DataOutBase::GmvFlags::declare_parameters (ParameterHandler &/*prm*/)
{}



void DataOutBase::GmvFlags::parse_parameters (ParameterHandler &/*prm*/)
{}


unsigned int
DataOutBase::GmvFlags::memory_consumption () const
{
				   // only simple data elements, so
				   // use sizeof operator
  return sizeof (*this);
}



DataOutBase::TecplotFlags::
TecplotFlags (const char* tecplot_binary_file_name)
                :
                tecplot_binary_file_name(tecplot_binary_file_name)
{}



void DataOutBase::TecplotFlags::declare_parameters (ParameterHandler &/*prm*/)
{}



void DataOutBase::TecplotFlags::parse_parameters (ParameterHandler &/*prm*/)
{}


unsigned int
DataOutBase::TecplotFlags::memory_consumption () const
{
				   // only simple data elements, so
				   // use sizeof operator
  return sizeof (*this);
}



void DataOutBase::VtkFlags::declare_parameters (ParameterHandler &/*prm*/)
{}



void DataOutBase::VtkFlags::parse_parameters (ParameterHandler &/*prm*/)
{}


unsigned int
DataOutBase::VtkFlags::memory_consumption () const
{
				   // only simple data elements, so
				   // use sizeof operator
  return sizeof (*this);
}



void DataOutBase::Deal_II_IntermediateFlags::declare_parameters (ParameterHandler &/*prm*/)
{}



void DataOutBase::Deal_II_IntermediateFlags::parse_parameters (ParameterHandler &/*prm*/)
{}


unsigned int
DataOutBase::Deal_II_IntermediateFlags::memory_consumption () const
{
				   // only simple data elements, so
				   // use sizeof operator
  return sizeof (*this);
}



unsigned int DataOutBase::memory_consumption ()
{
  return 0;
}




DataOutBase::OutputFormat
DataOutBase::
parse_output_format (const std::string &format_name)
{
  if (format_name == "dx")
    return dx;

  if (format_name == "ucd")
    return ucd;

  if (format_name == "gnuplot")
    return gnuplot;

  if (format_name == "povray")
    return povray;

  if (format_name == "eps")
    return eps;

  if (format_name == "gmv")
    return gmv;

  if (format_name == "tecplot")
    return tecplot;
  
  if (format_name == "tecplot_binary")
    return tecplot_binary;
  
  if (format_name == "vtk")
    return vtk;
  
  if (format_name == "deal.II intermediate")
    return deal_II_intermediate;
  
  AssertThrow (false,
               ExcMessage ((std::string("The format <") + format_name +
                            std::string("> is not recognized")).c_str()));

				   // return something invalid
  return OutputFormat(-1);
}



std::string
DataOutBase::get_output_format_names ()
{
  return "dx|ucd|gnuplot|povray|eps|gmv|tecplot|tecplot_binary|vtk|deal.II intermediate";
}



std::string
DataOutBase::
default_suffix (const OutputFormat output_format)
{
  switch (output_format) 
    {
      case dx:
	    return ".dx";
	    
      case ucd:
	    return ".inp";
	    
      case gnuplot: 
	    return ".gnuplot";
	    
      case povray: 
	    return ".pov";
	    
      case eps: 
	    return ".eps";

      case gmv:
	    return ".gmv";

      case tecplot:
	    return ".dat";
	    
      case tecplot_binary:
	    return ".plt";
	    
      case vtk:
	    return ".vtk";
	    
      case deal_II_intermediate:
	    return ".d2";
	    
      default: 
	    Assert (false, ExcNotImplemented()); 
	    return "";
    }
}



template <int dim, int spacedim>
void DataOutBase::write_ucd (const std::vector<Patch<dim,spacedim> > &patches,
			     const std::vector<std::string>          &data_names,
			     const UcdFlags                          &flags,
			     std::ostream                            &out) 
{
  AssertThrow (out, ExcIO());

  Assert (patches.size() > 0, ExcNoPatches());
  
  const unsigned int n_data_sets = data_names.size();

				   // first count the number of cells
				   // and cells for later use
  unsigned int n_cells = 0,
	       n_nodes = 0;
  for (typename std::vector<Patch<dim,spacedim> >::const_iterator patch=patches.begin();
       patch!=patches.end(); ++patch)
    switch (dim)
      {
	case 1:
	      n_cells += patch->n_subdivisions;
	      n_nodes += patch->n_subdivisions+1;
	      break;
	case 2:
	      n_cells += patch->n_subdivisions *
			 patch->n_subdivisions;
	      n_nodes += (patch->n_subdivisions+1) *
			 (patch->n_subdivisions+1);
	      break;
	case 3:
	      n_cells += patch->n_subdivisions *
			 patch->n_subdivisions *
			 patch->n_subdivisions;
	      n_nodes += (patch->n_subdivisions+1) *
			 (patch->n_subdivisions+1) *
			 (patch->n_subdivisions+1);
	      break;
	default:
	      Assert (false, ExcNotImplemented());
      }

				   ///////////////////////
				   // preamble
  if (flags.write_preamble)
    {
      std::time_t  time1= std::time (0);
      std::tm     *time = std::localtime(&time1); 
      out << "# This file was generated by the deal.II library." << std::endl
	  << "# Date =  "
	  << time->tm_year+1900 << "/"
	  << time->tm_mon+1 << "/"
	  << time->tm_mday << std::endl
	  << "# Time =  "
	  << time->tm_hour << ":"
	  << std::setw(2) << time->tm_min << ":"
	  << std::setw(2) << time->tm_sec << std::endl
	  << "#" << std::endl
	  << "# For a description of the UCD format see the AVS Developer's guide."
	  << std::endl
	  << "#" << std::endl;
    }

				   // start with ucd data
  out << n_nodes << ' '
      << n_cells << ' '
      << n_data_sets << ' '
      << 0 << ' '                  // no cell data at present
      << 0                         // no model data
      << std::endl;

				   ///////////////////////////////
				   // first make up the list of used
				   // nodes along with their
				   // coordinates. number them
				   // consecutively starting with 1
				   //
				   // note that we have to print
				   // d=1..3 dimensions
  if (true)
    {
      unsigned int present_node = 1;
      
      for (typename std::vector<Patch<dim,spacedim> >::const_iterator patch=patches.begin();
	   patch!=patches.end(); ++patch)
	{
	  const unsigned int n_subdivisions = patch->n_subdivisions;
	  
					   // if we have nonzero values for
					   // this coordinate
	  switch (dim)
	    {
	      case 1:
	      {
		for (unsigned int i=0; i<n_subdivisions+1; ++i, ++present_node)
		  {
		    out << present_node
			<< "   ";

		    const Point<spacedim>
		      node = ((patch->vertices[1] * i / n_subdivisions) +
			      (patch->vertices[0] * (n_subdivisions-i) / n_subdivisions));

						     // write out coordinates
		    for (unsigned int i=0; i<spacedim; ++i)
		      out << node(i) << ' ';
						     // fill with zeroes
		    for (unsigned int i=spacedim; i<3; ++i)
		      out << "0 ";
		    out << std::endl;
		  }
		
		break;
	      }
	       
	      case 2:
	      {
		for (unsigned int i=0; i<n_subdivisions+1; ++i)
		  for (unsigned int j=0; j<n_subdivisions+1; ++j)
		    {
		      const double x_frac = i * 1./n_subdivisions,
				   y_frac = j * 1./n_subdivisions;
		      
		      out << present_node
			  << "   ";
		      
						       // compute coordinates for
						       // this patch point
		      const Point<spacedim>
			node = (((patch->vertices[1] * x_frac) +
				 (patch->vertices[0] * (1-x_frac))) * (1-y_frac) +
				((patch->vertices[2] * x_frac) +
				 (patch->vertices[3] * (1-x_frac))) * y_frac);
		      
						       // write out coordinates
		      for (unsigned int i=0; i<spacedim; ++i)
			out << node(i) << ' ';
						       // fill with zeroes
		      for (unsigned int i=spacedim; i<3; ++i)
			out << "0 ";
		      out << std::endl;

		      ++present_node;
		    }
	      
		break;
	      }
	       
	      case 3:
	      {
		for (unsigned int i=0; i<n_subdivisions+1; ++i)
		  for (unsigned int j=0; j<n_subdivisions+1; ++j)
		    for (unsigned int k=0; k<n_subdivisions+1; ++k)
		      {
							 // note the broken
							 // design of hexahedra
							 // in deal.II, where
							 // first the z-component
							 // is counted up, before
							 // increasing the y-
							 // coordinate
			const double x_frac = i * 1./n_subdivisions,
				     y_frac = k * 1./n_subdivisions,
				     z_frac = j * 1./n_subdivisions;
			
							 // compute coordinates for
							 // this patch point
			out << present_node
			    << "   ";
			
							 // compute coordinates for
							 // this patch point
			const Point<spacedim>
			  node = ((((patch->vertices[1] * x_frac) +
				    (patch->vertices[0] * (1-x_frac))) * (1-y_frac) +
				   ((patch->vertices[2] * x_frac) +
				    (patch->vertices[3] * (1-x_frac))) * y_frac)   * (1-z_frac) +
				  (((patch->vertices[5] * x_frac) +
				    (patch->vertices[4] * (1-x_frac))) * (1-y_frac) +
				   ((patch->vertices[6] * x_frac) +
				    (patch->vertices[7] * (1-x_frac))) * y_frac)   * z_frac);
			
							 // write out coordinates
			for (unsigned int i=0; i<spacedim; ++i)
			  out << node(i) << ' ';
							 // fill with zeroes unnecessary here
			for (unsigned int i=spacedim; i<3; ++i)
			  out << "0 ";
			out << std::endl;
			
			++present_node;
		      }
	      
		break;
	      }
	       
	      default:
		    Assert (false, ExcNotImplemented());
	    }
	}

				       // note that we number starting with 1!
      Assert (present_node == n_nodes+1,
	      ExcInternalError());
    }

				   /////////////////////////////////////////
				   // write cell. number them consecutively,
				   // starting with 1
  if (true)
    {
      unsigned int present_cell = 1;      
      unsigned int first_vertex_of_patch = 0;
      
      for (typename std::vector<Patch<dim,spacedim> >::const_iterator patch=patches.begin();
	   patch!=patches.end(); ++patch)
	{
	  const unsigned int n_subdivisions = patch->n_subdivisions;
	  
					   // write out the cells making
					   // up this patch
	  switch (dim)
	    {
	      case 1:
	      {
		for (unsigned int i=0; i<n_subdivisions; ++i, ++present_cell)
		  out << present_cell
		      << " 0  line  "        // set material id to 0
		      << first_vertex_of_patch+i+1 << ' '
		      << first_vertex_of_patch+i+1+1 << std::endl;
		break;
	      }
	       
	      case 2:
	      {
		for (unsigned int i=0; i<n_subdivisions; ++i)
		  for (unsigned int j=0; j<n_subdivisions; ++j)
		    {
		      out << present_cell
			  << " 0  quad  "    // set material id to 0
			  << first_vertex_of_patch+i*(n_subdivisions+1)+j+1 << ' '
			  << first_vertex_of_patch+(i+1)*(n_subdivisions+1)+j+1 << ' '
			  << first_vertex_of_patch+(i+1)*(n_subdivisions+1)+j+1+1 << ' '
			  << first_vertex_of_patch+i*(n_subdivisions+1)+j+1+1
			  << std::endl;
		      ++present_cell;
		    }
		break;
	      }
	       
	      case 3:
	      {
		for (unsigned int i=0; i<n_subdivisions; ++i)
		  for (unsigned int j=0; j<n_subdivisions; ++j)
		    for (unsigned int k=0; k<n_subdivisions; ++k)
		      {
			out << present_cell
			    << " 0  hex  "    // set material id to 0
							   // note: vertex indices start with 1!
			    << first_vertex_of_patch+(i*(n_subdivisions+1)+j      )*(n_subdivisions+1)+k  +1 << ' '
			    << first_vertex_of_patch+((i+1)*(n_subdivisions+1)+j  )*(n_subdivisions+1)+k  +1 << ' '
			    << first_vertex_of_patch+((i+1)*(n_subdivisions+1)+j+1)*(n_subdivisions+1)+k  +1 << ' '
			    << first_vertex_of_patch+(i*(n_subdivisions+1)+j+1    )*(n_subdivisions+1)+k  +1 << ' '
			    << first_vertex_of_patch+(i*(n_subdivisions+1)+j      )*(n_subdivisions+1)+k+1+1 << ' '
			    << first_vertex_of_patch+((i+1)*(n_subdivisions+1)+j  )*(n_subdivisions+1)+k+1+1 << ' '
			    << first_vertex_of_patch+((i+1)*(n_subdivisions+1)+j+1)*(n_subdivisions+1)+k+1+1 << ' '
			    << first_vertex_of_patch+(i*(n_subdivisions+1)+j+1    )*(n_subdivisions+1)+k+1+1 << ' '
			    << std::endl;
			++present_cell;
		      }
		break;
	      }

	      default:
		    Assert (false, ExcNotImplemented());
	    }


					   // finally update the number
					   // of the first vertex of this patch
	  switch (dim)
	    {
	      case 1:
		    first_vertex_of_patch += n_subdivisions+1;
		    break;
	      case 2:
		    first_vertex_of_patch += (n_subdivisions+1) *
					     (n_subdivisions+1);
		    break;
	      case 3:
		    first_vertex_of_patch += (n_subdivisions+1) *
					     (n_subdivisions+1) *
					     (n_subdivisions+1);
		    break;
	      default:
		    Assert (false, ExcNotImplemented());
	    }
	}
      out << std::endl;

				       // note that we number starting with 1!
      Assert (present_cell == n_cells+1,
	      ExcInternalError());
    }


				   /////////////////////////////
				   // now write data
  if (n_data_sets != 0)
    {      
      out << n_data_sets << "    ";    // number of vectors
      for (unsigned int i=0; i<n_data_sets; ++i)
	out << 1 << ' ';               // number of components;
				       // only 1 supported presently
      out << std::endl;

      for (unsigned int data_set=0; data_set<n_data_sets; ++data_set)
	out << data_names[data_set]
	    << ",dimensionless"      // no units supported at present
	    << std::endl;


				       // loop over all patches
      unsigned int present_node = 1;
      for (typename std::vector<Patch<dim,spacedim> >::const_iterator patch=patches.begin();
	   patch != patches.end(); ++patch)
	{
	  const unsigned int n_subdivisions = patch->n_subdivisions;
      
	  Assert (patch->data.n_rows() == n_data_sets,
		  ExcDimensionMismatch (patch->data.n_rows(), n_data_sets));
	  Assert (patch->data.n_cols() == (dim==1 ?
				      n_subdivisions+1 :
				      (dim==2 ?
				       (n_subdivisions+1)*(n_subdivisions+1) :
				       (dim==3 ?
					(n_subdivisions+1)*(n_subdivisions+1)*(n_subdivisions+1) :
					0))),
		  ExcInvalidDatasetSize (patch->data.n_cols(), n_subdivisions+1));

	  switch (dim)
	    {
	      case 1:
	      {      
		for (unsigned int i=0; i<n_subdivisions+1; ++i, ++present_node) 
		  {
		    out << present_node
			<< "  ";
		    for (unsigned int data_set=0; data_set<n_data_sets; ++data_set)
		      out << patch->data(data_set,i) << ' ';

		    out << std::endl;
		  }
	    
		break;
	      }
	   
	      case 2:
	      {
		for (unsigned int i=0; i<n_subdivisions+1; ++i)
		  for (unsigned int j=0; j<n_subdivisions+1; ++j)
		    {
		      out << present_node
			  << "  ";
		      for (unsigned int data_set=0; data_set<n_data_sets; ++data_set)
			out << patch->data(data_set,i*(n_subdivisions+1) + j) << ' ';

		      out << std::endl;

		      ++present_node;
		    }

		break;
	      }

	      case 3:
	      {
		for (unsigned int i=0; i<n_subdivisions+1; ++i)
		  for (unsigned int j=0; j<n_subdivisions+1; ++j)
		    for (unsigned int k=0; k<n_subdivisions+1; ++k)
		      {
			out << present_node
			    << "  ";
			for (unsigned int data_set=0; data_set<n_data_sets; ++data_set)
			  out << patch->data(data_set,
					     (i*(n_subdivisions+1)+j)*(n_subdivisions+1)+k)
			      << ' ';
			
			out << std::endl;
			
			++present_node;
		      }

		break;
	      }

	      default:
		    Assert (false, ExcNotImplemented());
	    }
	}  
    }

				   // no model data

				   // assert the stream is still ok
  AssertThrow (out, ExcIO());
}



template <int dim, int spacedim>
void DataOutBase::write_dx (const std::vector<Patch<dim,spacedim> > &patches,
			    const std::vector<std::string>          &data_names,
			    const DXFlags                           &flags,
			    std::ostream                            &out) 
{
  AssertThrow (out, ExcIO());

  Assert (patches.size() > 0, ExcNoPatches());
  
  const unsigned int n_data_sets = data_names.size();

				   // first count the number of cells
				   // and cells for later use
  unsigned int n_cells = 0,
	       n_nodes = 0;
  for (typename std::vector<Patch<dim,spacedim> >::const_iterator patch=patches.begin();
       patch!=patches.end(); ++patch)
    switch (dim)
      {
	case 1:
	      n_cells += patch->n_subdivisions;
	      n_nodes += patch->n_subdivisions+1;
	      break;
	case 2:
	      n_cells += patch->n_subdivisions *
			 patch->n_subdivisions;
	      n_nodes += (patch->n_subdivisions+1) *
			 (patch->n_subdivisions+1);
	      break;
	case 3:
	      n_cells += patch->n_subdivisions *
			 patch->n_subdivisions *
			 patch->n_subdivisions;
	      n_nodes += (patch->n_subdivisions+1) *
			 (patch->n_subdivisions+1) *
			 (patch->n_subdivisions+1);
	      break;
	default:
	      Assert (false, ExcNotImplemented());
      }

				   // start with vertices order is
				   // lexicographical, x varying
				   // fastest
  out << "object \"vertices\" class array type float rank 1 shape " << spacedim
      << " items " << n_nodes << " data follows" << std::endl;

				   ///////////////////////////////
				   // first write the coordinates of all vertices
  if (true)
    {
      for (typename std::vector<Patch<dim,spacedim> >::const_iterator patch=patches.begin();
	   patch!=patches.end(); ++patch)
	{
	  const unsigned int n_subdivisions = patch->n_subdivisions;
	  
					   // if we have nonzero values for
					   // this coordinate
	  switch (dim)
	    {
	      case 1:
	      {
		for (unsigned int i=0; i<n_subdivisions+1; ++i)
		  {
		    const Point<spacedim>
		      node = ((patch->vertices[1] * i / n_subdivisions) +
			      (patch->vertices[0] * (n_subdivisions-i) / n_subdivisions));

						     // write out coordinates
		    for (unsigned int i=0; i<spacedim; ++i)
		      out << node(i) << '\t';
		    out << std::endl;
		  }
		
		break;
	      }
	       
	      case 2:
	      {
		for (unsigned int i=0; i<n_subdivisions+1; ++i)
		  for (unsigned int j=0; j<n_subdivisions+1; ++j)
		    {
		      const double x_frac = i * 1./n_subdivisions,
				   y_frac = j * 1./n_subdivisions;
		      
						       // compute coordinates for
						       // this patch point
		      const Point<spacedim>
			node = (((patch->vertices[1] * x_frac) +
				 (patch->vertices[0] * (1-x_frac))) * (1-y_frac) +
				((patch->vertices[2] * x_frac) +
				 (patch->vertices[3] * (1-x_frac))) * y_frac);
		      
						       // write out coordinates
		      for (unsigned int i=0; i<spacedim; ++i)
			out << node(i) << '\t';
		      out << std::endl;
		    }
	      
		break;
	      }
	       
	      case 3:
	      {
		for (unsigned int i=0; i<n_subdivisions+1; ++i)
		  for (unsigned int j=0; j<n_subdivisions+1; ++j)
		    for (unsigned int k=0; k<n_subdivisions+1; ++k)
		      {
							 // note the broken
							 // design of hexahedra
							 // in deal.II, where
							 // first the z-component
							 // is counted up, before
							 // increasing the y-
							 // coordinate
			const double x_frac = i * 1./n_subdivisions,
				     y_frac = k * 1./n_subdivisions,
				     z_frac = j * 1./n_subdivisions;
			
							 // compute coordinates for
							 // this patch point
			const Point<spacedim>
			  node = ((((patch->vertices[1] * x_frac) +
				    (patch->vertices[0] * (1-x_frac))) * (1-y_frac) +
				   ((patch->vertices[2] * x_frac) +
				    (patch->vertices[3] * (1-x_frac))) * y_frac)   * (1-z_frac) +
				  (((patch->vertices[5] * x_frac) +
				    (patch->vertices[4] * (1-x_frac))) * (1-y_frac) +
				   ((patch->vertices[6] * x_frac) +
				    (patch->vertices[7] * (1-x_frac))) * y_frac)   * z_frac);
			
							 // write out coordinates
			for (unsigned int i=0; i<spacedim; ++i)
			  out << node(i) << '\t';
			out << std::endl;
		      }
	      
		break;
	      }
	       
	      default:
		    Assert (false, ExcNotImplemented());
	    }
	}
    }

				   /////////////////////////////////////////
				   // write cells
  out << "object \"cells\" class array type int rank 1 shape " << GeometryInfo<dim>::vertices_per_cell
      << " items " << n_cells << " data follows"
      << std::endl;
  
  if (true)
    {
      unsigned int first_vertex_of_patch = 0;
      
      for (typename std::vector<Patch<dim,spacedim> >::const_iterator patch=patches.begin();
	   patch!=patches.end(); ++patch)
	{
	  const unsigned int n_subdivisions = patch->n_subdivisions;
	  
					   // write out the cells making
					   // up this patch
	  switch (dim)
	    {
	      case 1:
		if (true)
		  {
		    for (unsigned int i=0; i<n_subdivisions; ++i)
		      out << first_vertex_of_patch+i << '\t'
			  << first_vertex_of_patch+i+1 << std::endl;
		  }
		break;
	      case 2:
		if (true)
		  {
		    for (unsigned int i=0; i<n_subdivisions; ++i)
		      for (unsigned int j=0; j<n_subdivisions; ++j)
			{
			  out << first_vertex_of_patch+i*(n_subdivisions+1)+j << '\t'
			      << first_vertex_of_patch+i*(n_subdivisions+1)+j+1 << '\t'
			      << first_vertex_of_patch+(i+1)*(n_subdivisions+1)+j << '\t'
			      << first_vertex_of_patch+(i+1)*(n_subdivisions+1)+j+1
			      << std::endl;
			}
		  }
		break;
	      case 3:
		if (true)
		  {
		    const unsigned int nvt = n_subdivisions+1;
		    for (unsigned int i=0; i<n_subdivisions; ++i)
		      for (unsigned int j=0; j<n_subdivisions; ++j)
			for (unsigned int k=0; k<n_subdivisions; ++k)
			  {
			    out
			      << first_vertex_of_patch+((i  )*nvt+j  )*nvt+k   << '\t'
			      << first_vertex_of_patch+((i  )*nvt+j  )*nvt+k+1 << '\t'
			      << first_vertex_of_patch+((i  )*nvt+j+1)*nvt+k   << '\t'
			      << first_vertex_of_patch+((i  )*nvt+j+1)*nvt+k+1 << '\t'
			      << first_vertex_of_patch+((i+1)*nvt+j  )*nvt+k   << '\t'
			      << first_vertex_of_patch+((i+1)*nvt+j  )*nvt+k+1 << '\t'
			      << first_vertex_of_patch+((i+1)*nvt+j+1)*nvt+k   << '\t'
			      << first_vertex_of_patch+((i+1)*nvt+j+1)*nvt+k+1 << '\t'
			      ;
			    out << std::endl;
			  }
		  }
		break;
	      default:
		    Assert (false, ExcNotImplemented());
	    }


					   // finally update the number
					   // of the first vertex of this patch
	  switch (dim)
	    {
	      case 1:
		    first_vertex_of_patch += n_subdivisions+1;
		    break;
	      case 2:
		    first_vertex_of_patch += (n_subdivisions+1) *
					     (n_subdivisions+1);
		    break;
	      case 3:
		    first_vertex_of_patch += (n_subdivisions+1) *
					     (n_subdivisions+1) *
					     (n_subdivisions+1);
		    break;
	      default:
		    Assert (false, ExcNotImplemented());
	    }
	}
      out << std::endl;
    }

  out << "attribute \"element type\" string \"";
  if (dim==1) out << "lines";
  if (dim==2) out << "quads";
  if (dim==3) out << "cubes";
  out << "\"" << std::endl
      << "attribute \"ref\" string \"positions\"" << std::endl;

//TODO:[GK] Patches must be of same size!
				   /////////////////////////////
				   // write neighbor information
  if (flags.write_neighbors)
    {
      out << "object \"neighbors\" class array type int rank 1 shape "
	  << GeometryInfo<dim>::faces_per_cell
	  << " items " << n_cells
	  << " data follows";

      for (typename std::vector<Patch<dim,spacedim> >::const_iterator
	     patch=patches.begin();
	   patch!=patches.end(); ++patch)
	{
	  const unsigned int n = patch->n_subdivisions;
	  unsigned int cells_per_patch = 1;
	  unsigned int dx = 1;
	  unsigned int dy = 1;
	  unsigned int dz = 1;
	  switch(dim)
	    {
	      case 3:
		dx *= n;
		dy *= n;
		cells_per_patch *= n;
	      case 2:
		dx *= n;
		cells_per_patch *= n;
	      case 1:
		cells_per_patch *= n;
	    }

	  const unsigned int patch_start = patch->patch_index * cells_per_patch;

	  for (unsigned int ix=0;ix<n;++ix)
	    for (unsigned int iy=0;iy<((dim>1) ? n : 1);++iy)
	      for (unsigned int iz=0;iz<((dim>2) ? n : 1);++iz)
		{
		  const unsigned int nx = ix*dx;
		  const unsigned int ny = iy*dy;
		  const unsigned int nz = iz*dz;

		  out << std::endl;
						   // Direction -x
						   // Last cell in row
						   // of other patch
		  if (ix==0)
		    {
		      const unsigned int nn = patch->neighbors[0];
		      out << '\t';
		      if (nn != patch->no_neighbor)
			out << (nn*cells_per_patch+ny+nz+dx*(n-1));
		      else
			out << "-1";
		    } else {      
		      out << '\t'
			  << patch_start+nx-dx+ny+nz;
		    }
						       // Direction +x
						       // First cell in row
						       // of other patch
		  if (ix == n-1)
		    {
		      const unsigned int nn = patch->neighbors[1];
		      out << '\t';
		      if (nn != patch->no_neighbor)
			out << (nn*cells_per_patch+ny+nz);
		      else
			out << "-1";
		    } else {
		      out << '\t'
			  << patch_start+nx+dx+ny+nz;
		    }
		  if (dim<2)
		    continue;
						   // Direction -y
		  if (iy==0)
		    {
		      const unsigned int nn = patch->neighbors[2];
		      out << '\t';
		      if (nn != patch->no_neighbor)
			out << (nn*cells_per_patch+nx+nz+dy*(n-1));
		      else
			out << "-1";
		    } else {      
		      out << '\t'
			  << patch_start+nx+ny-dy+nz;
		    }
						   // Direction +y
		  if (iy == n-1)
		    {
		      const unsigned int nn = patch->neighbors[3];
		      out << '\t';
		      if (nn != patch->no_neighbor)
			out << (nn*cells_per_patch+nx+nz);
		      else
			out << "-1";
		    } else {
		      out << '\t'
			  << patch_start+nx+ny+dy+nz;
		    }
		  if (dim<3)
		    continue;
		  
						   // Direction -z
		  if (iz==0)
		    {
		      const unsigned int nn = patch->neighbors[4];
		      out << '\t';
		      if (nn != patch->no_neighbor)
			out << (nn*cells_per_patch+nx+ny+dz*(n-1));
		      else
			out << "-1";
		    } else {      
		      out << '\t'
			  << patch_start+nx+ny+nz-dz;
		    }
						   // Direction +z
		  if (iz == n-1)
		    {
		      const unsigned int nn = patch->neighbors[5];
		      out << '\t';
		      if (nn != patch->no_neighbor)
			out << (nn*cells_per_patch+nx+ny);
		      else
			out << "-1";
		    } else {
		      out << '\t'
			  << patch_start+nx+ny+nz+dz;
		    }
		}
	  out << std::endl;	  
	}
    }
				   /////////////////////////////
				   // now write data
  if (n_data_sets != 0)
    {      
      out << "object \"data\" class array type float rank 1 shape "
	  << n_data_sets
	  << " items " << n_nodes << " data follows" << std::endl;

				       // loop over all patches
      for (typename std::vector<Patch<dim,spacedim> >::const_iterator patch=patches.begin();
	   patch != patches.end(); ++patch)
	{
	  const unsigned int n_subdivisions = patch->n_subdivisions;
      
	  Assert (patch->data.n_rows() == n_data_sets,
		  ExcDimensionMismatch (patch->data.n_rows(), n_data_sets));
	  Assert (patch->data.n_cols() == (dim==1 ?
				      n_subdivisions+1 :
				      (dim==2 ?
				       (n_subdivisions+1)*(n_subdivisions+1) :
				       (dim==3 ?
					(n_subdivisions+1)*(n_subdivisions+1)*(n_subdivisions+1) :
					0))),
		  ExcInvalidDatasetSize (patch->data.n_cols(), n_subdivisions+1));

	  switch (dim)
	    {
	      case 1:
	      {      
		for (unsigned int i=0; i<n_subdivisions+1; ++i) 
		  {
		    for (unsigned int data_set=0; data_set<n_data_sets; ++data_set)
		      out << patch->data(data_set,i) << '\t';
		    out << std::endl;
		  }
	    
		break;
	      }
	   
	      case 2:
	      {
		for (unsigned int i=0; i<n_subdivisions+1; ++i)
		  for (unsigned int j=0; j<n_subdivisions+1; ++j)
		    {
		      for (unsigned int data_set=0; data_set<n_data_sets; ++data_set)
			out << patch->data(data_set,i*(n_subdivisions+1) + j) << '\t';
		      out << std::endl;
		    }

		break;
	      }

	      case 3:
	      {
		for (unsigned int i=0; i<n_subdivisions+1; ++i)
		  for (unsigned int j=0; j<n_subdivisions+1; ++j)
		    for (unsigned int k=0; k<n_subdivisions+1; ++k)
		      {
			for (unsigned int data_set=0; data_set<n_data_sets; ++data_set)
			  out << patch->data(data_set,
					     (i*(n_subdivisions+1)+j)*(n_subdivisions+1)+k)
			      << '\t';
			out << std::endl;
		      }

		break;
	      }

	      default:
		    Assert (false, ExcNotImplemented());
	    }
	}
      out << "attribute \"dep\" string \"positions\"" << std::endl;
    } else {
      out << "object \"data\" class constantarray type float rank 0 items " << n_nodes << " data follows"
	  << std::endl << '0' << std::endl;
    }
  
				   // no model data
  
  out << "object \"deal data\" class field" << std::endl
      << "component \"positions\" value \"vertices\"" << std::endl
      << "component \"connections\" value \"cells\"" << std::endl
      << "component \"data\" value \"data\"" << std::endl;

  if (flags.write_neighbors)
    out << "component \"neighbors\" value \"neighbors\"" << std::endl;

  if (true)
    {
      std::time_t  time1= std::time (0);
      std::tm     *time = std::localtime(&time1); 
      out << "attribute \"created\" string \""
	  << time->tm_year+1900 << "/"
	  << time->tm_mon+1 << "/"
	  << time->tm_mday
	  << ' '
	  << time->tm_hour << ":"
	  << std::setw(2) << time->tm_min << ":"
	  << std::setw(2) << time->tm_sec << '"' << std::endl;
    }

  out << "end" << std::endl;
				   // assert the stream is still ok
  AssertThrow (out, ExcIO());
}



template <int dim, int spacedim>
void DataOutBase::write_gnuplot (const std::vector<Patch<dim,spacedim> > &patches,
				 const std::vector<std::string>          &data_names,
				 const GnuplotFlags                      &/*flags*/,
				 std::ostream                            &out) 
{
  AssertThrow (out, ExcIO());
  
  Assert (patches.size() > 0, ExcNoPatches());

  const unsigned int n_data_sets = data_names.size();
  
  				   // write preamble
  if (true) 
    {
				       // block this to have local
				       // variables destroyed after
				       // use
      const std::time_t  time1= std::time (0);
      const std::tm     *time = std::localtime(&time1); 
      out << "# This file was generated by the deal.II library." << std::endl
	  << "# Date =  "
	  << time->tm_year+1900 << "/"
	  << time->tm_mon+1 << "/"
	  << time->tm_mday << std::endl
	  << "# Time =  "
	  << time->tm_hour << ":"
	  << std::setw(2) << time->tm_min << ":"
	  << std::setw(2) << time->tm_sec << std::endl
	  << "#" << std::endl
	  << "# For a description of the GNUPLOT format see the GNUPLOT manual."
	  << std::endl
	  << "#" << std::endl
	  << "# ";
      
      switch (spacedim)
	{
	  case 1:
		out << "<x> ";
		break;
	  case 2:
		out << "<x> <y> ";
		break;
	  case 3:
		out << "<x> <y> <z> ";
		break;
		
	  default:
		Assert (false, ExcNotImplemented());
	}

      for (unsigned int i=0; i<data_names.size(); ++i)
	out << '<'
	    << data_names[i]
	    << "> ";
      out << std::endl;      
    }


				   // loop over all patches
  for (typename std::vector<Patch<dim,spacedim> >::const_iterator patch=patches.begin();
       patch != patches.end(); ++patch)
    {
      const unsigned int n_subdivisions = patch->n_subdivisions;
      
      Assert (patch->data.n_rows() == n_data_sets,
	      ExcDimensionMismatch (patch->data.n_rows(), n_data_sets));
      Assert (patch->data.n_cols() == (dim==1 ?
				  n_subdivisions+1 :
				  (dim==2 ?
				   (n_subdivisions+1)*(n_subdivisions+1) :
				   (dim==3 ?
				    (n_subdivisions+1)*(n_subdivisions+1)*(n_subdivisions+1) :
				    0))),
	      ExcInvalidDatasetSize (patch->data.n_cols(), n_subdivisions+1));

      switch (dim)
	{
	  case 1:
	  {      
	    for (unsigned int i=0; i<n_subdivisions+1; ++i) 
	      {
						 // compute coordinates for
						 // this patch point
		out << ((patch->vertices[1] * i / n_subdivisions) +
			(patch->vertices[0] * (n_subdivisions-i) / n_subdivisions))
		    << ' ';

		for (unsigned int data_set=0; data_set<n_data_sets; ++data_set)
		  out << patch->data(data_set,i) << ' ';

		out << std::endl;
	      }

					     // end of patch
	    out << std::endl
		<< std::endl;
	    
	    break;
	  }
	   
	  case 2:
	  {
	    for (unsigned int i=0; i<n_subdivisions+1; ++i)
	      {
		for (unsigned int j=0; j<n_subdivisions+1; ++j)
		  {
		    const double x_frac = i * 1./n_subdivisions,
				 y_frac = j * 1./n_subdivisions;
		    
						     // compute coordinates for
						     // this patch point
		    out << (((patch->vertices[1] * x_frac) +
			     (patch->vertices[0] * (1-x_frac))) * (1-y_frac) +
			    ((patch->vertices[2] * x_frac) +
			     (patch->vertices[3] * (1-x_frac))) * y_frac)
			<< ' ';
		    
		    for (unsigned int data_set=0; data_set<n_data_sets; ++data_set)
		      out << patch->data(data_set,i*(n_subdivisions+1) + j) << ' ';

		    out << std::endl;
		  }

						 // end of row in patch
		out << std::endl;
	      }

					     // end of patch
	    out << std::endl;

	    break;
	  }

	  case 3:
	  {
					     // for all grid points: draw
					     // lines into all positive
					     // coordinate directions if
					     // there is another grid point
					     // there
	    for (unsigned int i=0; i<n_subdivisions+1; ++i)
	      for (unsigned int j=0; j<n_subdivisions+1; ++j)
		for (unsigned int k=0; k<n_subdivisions+1; ++k)
		  {
						     // note the broken
						     // design of hexahedra
						     // in deal.II, where
						     // first the z-component
						     // is counted up, before
						     // increasing the y-
						     // coordinate
		    const double x_frac = i * 1./n_subdivisions,
				 y_frac = k * 1./n_subdivisions,
				 z_frac = j * 1./n_subdivisions;

						     // compute coordinates for
						     // this patch point
		    const Point<spacedim> this_point
		      = ((((patch->vertices[1] * x_frac) +
			   (patch->vertices[0] * (1-x_frac))) * (1-y_frac) +
			  ((patch->vertices[2] * x_frac) +
			   (patch->vertices[3] * (1-x_frac))) * y_frac)   * (1-z_frac) +
			 (((patch->vertices[5] * x_frac) +
			   (patch->vertices[4] * (1-x_frac))) * (1-y_frac) +
			  ((patch->vertices[6] * x_frac) +
			   (patch->vertices[7] * (1-x_frac))) * y_frac)   * z_frac);

						     // line into positive x-direction
						     // if possible
		    if (i < n_subdivisions)
		      {
							 // write point here
							 // and its data
			out << this_point;
			for (unsigned int data_set=0; data_set<n_data_sets; ++data_set)
			  out  << ' '
			       << patch->data(data_set,
					      (i*(n_subdivisions+1) + j)*(n_subdivisions+1)+k);
			out << std::endl;
			
							 // write point there
							 // and its data
			const double x_frac_new = x_frac + 1./n_subdivisions;
			out << ((((patch->vertices[1] * x_frac_new) +
				  (patch->vertices[0] * (1-x_frac_new))) * (1-y_frac) +
				 ((patch->vertices[2] * x_frac_new) +
				  (patch->vertices[3] * (1-x_frac_new))) * y_frac)   * (1-z_frac) +
				(((patch->vertices[5] * x_frac_new) +
				  (patch->vertices[4] * (1-x_frac_new))) * (1-y_frac) +
				 ((patch->vertices[6] * x_frac_new) +
				  (patch->vertices[7] * (1-x_frac_new))) * y_frac)   * z_frac);
			for (unsigned int data_set=0; data_set<n_data_sets; ++data_set)
			  out  << ' '
			       << patch->data(data_set,
					      ((i+1)*(n_subdivisions+1) + j)*(n_subdivisions+1)+k);
			out << std::endl;

							 // end of line
			out << std::endl
			    << std::endl;
		      }
		    
						     // line into positive y-direction
						     // if possible
		    if (j < n_subdivisions)
		      {
							 // write point here
							 // and its data
			out << this_point;
			for (unsigned int data_set=0; data_set<n_data_sets; ++data_set)
			  out  << ' '
			       << patch->data(data_set,
					      (i*(n_subdivisions+1) + j)*(n_subdivisions+1)+k);
			out << std::endl;
			
							 // write point there
							 // and its data
			const double z_frac_new = z_frac + 1./n_subdivisions;
			out << ((((patch->vertices[1] * x_frac) +
				  (patch->vertices[0] * (1-x_frac))) * (1-y_frac) +
				 ((patch->vertices[2] * x_frac) +
				  (patch->vertices[3] * (1-x_frac))) * y_frac)   * (1-z_frac_new) +
				(((patch->vertices[5] * x_frac) +
				  (patch->vertices[4] * (1-x_frac))) * (1-y_frac) +
				 ((patch->vertices[6] * x_frac) +
				  (patch->vertices[7] * (1-x_frac))) * y_frac)   * z_frac_new);
			for (unsigned int data_set=0; data_set<n_data_sets; ++data_set)
			  out  << ' '
			       << patch->data(data_set,
					      (i*(n_subdivisions+1) + (j+1))*(n_subdivisions+1)+k);
			out << std::endl;

							 // end of line
			out << std::endl
			    << std::endl;
		      }

						     // line into positive z-direction
						     // if possible
		    if (k < n_subdivisions)
		      {
							 // write point here
							 // and its data
			out << this_point;
			for (unsigned int data_set=0; data_set<n_data_sets; ++data_set)
			  out  << ' '
			       << patch->data(data_set,
					      (i*(n_subdivisions+1) + j)*(n_subdivisions+1)+k);
			out << std::endl;
			
							 // write point there
							 // and its data
			const double y_frac_new = y_frac + 1./n_subdivisions;
			out << ((((patch->vertices[1] * x_frac) +
				  (patch->vertices[0] * (1-x_frac))) * (1-y_frac_new) +
				 ((patch->vertices[2] * x_frac) +
				  (patch->vertices[3] * (1-x_frac))) * y_frac_new)   * (1-z_frac) +
				(((patch->vertices[5] * x_frac) +
				  (patch->vertices[4] * (1-x_frac))) * (1-y_frac_new) +
				 ((patch->vertices[6] * x_frac) +
				  (patch->vertices[7] * (1-x_frac))) * y_frac_new)   * z_frac);
			for (unsigned int data_set=0; data_set<n_data_sets; ++data_set)
			  out  << ' '
			       << patch->data(data_set,
					      (i*(n_subdivisions+1) + j)*(n_subdivisions+1)+k+1);
			out << std::endl;

							 // end of line
			out << std::endl
			    << std::endl;
		      }		    
			
		  }

	    break;
	  }

	  default:
		Assert (false, ExcNotImplemented());
	}
    }

  AssertThrow (out, ExcIO());
}



template <int dim, int spacedim>
void DataOutBase::write_povray (const std::vector<Patch<dim,spacedim> > &patches,
				const std::vector<std::string>          &data_names,
				const PovrayFlags                       &flags,
				std::ostream                            &out) 
{
  AssertThrow (out, ExcIO());
  
  Assert (patches.size() > 0, ExcNoPatches());
  Assert (dim==2, ExcNotImplemented());        // only for 2-D surfaces on a 2-D plane
  Assert (spacedim==2, ExcNotImplemented());

  const unsigned int n_data_sets = data_names.size();
  
				   // write preamble
  if (true) 
    {
				       // block this to have local
				       // variables destroyed after use
      const std::time_t  time1= std::time (0);
      const std::tm     *time = std::localtime(&time1); 
      out << "/* This file was generated by the deal.II library." << std::endl
	  << "   Date =  "
	  << time->tm_year+1900 << "/"
	  << time->tm_mon+1 << "/"
	  << time->tm_mday << std::endl
	  << "   Time =  "
	  << time->tm_hour << ":"
	  << std::setw(2) << time->tm_min << ":"
	  << std::setw(2) << time->tm_sec << std::endl
	  << std::endl
	  << "   For a description of the POVRAY format see the POVRAY manual."
	  << std::endl
	  << "*/ " << std::endl;
      
				       // include files
      out << "#include \"colors.inc\" " << std::endl
	  << "#include \"textures.inc\" " << std::endl;

      
				       // use external include file for textures,
				       // camera and light
      if (flags.external_data)
	out << "#include \"data.inc\" " << std::endl;
      else                          // all definitions in data file
	{  
					   // camera
	  out << std::endl << std::endl
	      << "camera {"            << std::endl
	      << "  location <1,4,-7>" << std::endl
	      << "  look_at <0,0,0>"   << std::endl
	      << "  angle 30"          << std::endl
	      << "}"                   << std::endl;
	  
					   // light
	  out << std::endl 
	      << "light_source {"      << std::endl
	      << "  <1,4,-7>"	   << std::endl
	      << "  color Grey"        << std::endl
	      << "}"                   << std::endl;
	  out << std::endl 
	      << "light_source {"      << std::endl
	      << "  <0,20,0>"	   << std::endl
	      << "  color White"       << std::endl
	      << "}"                   << std::endl;
	}
    }
  
				   // max. and min. heigth of solution 
  double hmin=0, hmax=0;

  for (typename std::vector<Patch<dim,spacedim> >::const_iterator patch=patches.begin();
       patch != patches.end(); ++patch)
    {
      const unsigned int n_subdivisions = patch->n_subdivisions;
      
      Assert (patch->data.n_rows() == n_data_sets,
	      ExcDimensionMismatch (patch->data.n_rows(), n_data_sets));
      Assert (patch->data.n_cols() == (dim==1 ?
				  n_subdivisions+1 :
				  (dim==2 ?
				   (n_subdivisions+1)*(n_subdivisions+1) :
				   (dim==3 ?
				    (n_subdivisions+1)*(n_subdivisions+1)*(n_subdivisions+1) :
				    0))),
	      ExcInvalidDatasetSize (patch->data.n_cols(), n_subdivisions+1));
      
      for (unsigned int i=0; i<n_subdivisions; ++i)
	for (unsigned int j=0; j<n_subdivisions; ++j)
	  {
	    const int dl = i*(n_subdivisions+1)+j;
	    if ((hmin==0)||(patch->data(0,dl)<hmin)) hmin=patch->data(0,dl);
	    if ((hmax==0)||(patch->data(0,dl)>hmax)) hmax=patch->data(0,dl);
	  }
    }

  out << "#declare HMIN=" << hmin << ";" << std::endl
      << "#declare HMAX=" << hmax << ";" << std::endl << std::endl;

  if (!flags.external_data)
    {
				       // texture with scaled niveau lines
				       // 10 lines in the surface
      out << "#declare Tex=texture{" << std::endl
	  << "  pigment {" << std::endl
	  << "    gradient y" << std::endl
	  << "    scale y*(HMAX-HMIN)*" << 0.1 << std::endl
	  << "    color_map {" << std::endl
	  << "      [0.00 color Light_Purple] " << std::endl
	  << "      [0.95 color Light_Purple] " << std::endl
	  << "      [1.00 color White]    " << std::endl
	  << "} } }" << std::endl << std::endl;
    }

  if (!flags.bicubic_patch)
    {                                  // start of mesh header
      out << std::endl
	  << "mesh {" << std::endl;
    }

				   // loop over all patches
  for (typename std::vector<Patch<dim,spacedim> >::const_iterator patch=patches.begin();
       patch != patches.end(); ++patch)
    {
      const unsigned int n_subdivisions = patch->n_subdivisions;
      
      Assert (patch->data.n_rows() == n_data_sets,
	      ExcDimensionMismatch (patch->data.n_rows(), n_data_sets));
      Assert (patch->data.n_cols() == (dim==1 ?
				  n_subdivisions+1 :
				  (dim==2 ?
				   (n_subdivisions+1)*(n_subdivisions+1) :
				   (dim==3 ?
				    (n_subdivisions+1)*(n_subdivisions+1)*(n_subdivisions+1) :
				    0))),
	      ExcInvalidDatasetSize (patch->data.n_cols(), n_subdivisions+1));


      std::vector<Point<spacedim> > ver((n_subdivisions+1)*
					(n_subdivisions+1));
      
      for (unsigned int i=0; i<n_subdivisions+1; ++i)
	{
	  for (unsigned int j=0; j<n_subdivisions+1; ++j)
	    {
	      const double x_frac = i * 1./n_subdivisions,
			   y_frac = j * 1./n_subdivisions;
					       // compute coordinates for
					       // this patch point, storing in ver
	      ver[i*(n_subdivisions+1)+j]= (((patch->vertices[1] * x_frac) +
					     (patch->vertices[0] * (1-x_frac))) * (1-y_frac) +
					    ((patch->vertices[2] * x_frac) +
					     (patch->vertices[3] * (1-x_frac))) * y_frac);
	    }
	}
      
      if (!flags.bicubic_patch)
	{                                    // setting up triangles
	  for (unsigned int i=0; i<n_subdivisions; ++i)
	    {
	      for (unsigned int j=0; j<n_subdivisions; ++j)
		{
						   // down/left vertex of triangle
		  const int dl = i*(n_subdivisions+1)+j;
		  if (flags.smooth)               // only if smooth triangles are used
		    {
						       // aproximate normal
						       // vectors in patch
		      std::vector<Point<3> > nrml((n_subdivisions+1)*
						  (n_subdivisions+1));
		      Point<3> h1,h2;
		      for (unsigned int i=0; i<n_subdivisions+1;++i)
			{
			  for (unsigned int j=0; j<n_subdivisions+1;++j)
			    {
			      if (i==0)
				{
				  h1(0)=ver[(i+1)*(n_subdivisions+1)+j](0)-
					ver[i*(n_subdivisions+1)+j](0);
				  h1(1)=patch->data(0,(i+1)*(n_subdivisions+1)+j)-
					patch->data(0,i*(n_subdivisions+1)+j);
				  h1(2)=ver[(i+1)*(n_subdivisions+1)+j](1)-
					ver[i*(n_subdivisions+1)+j](1);
				}
			      else
				if (i==n_subdivisions)
				  {
				    h1(0)=ver[i*(n_subdivisions+1)+j](0)-
					  ver[(i-1)*(n_subdivisions+1)+j](0);
				    h1(1)=patch->data(0,i*(n_subdivisions+1)+j)-
					  patch->data(0,(i-1)*(n_subdivisions+1)+j);
				    h1(2)=ver[i*(n_subdivisions+1)+j](1)-
					  ver[(i-1)*(n_subdivisions+1)+j](1);
				  }
				else
				  {
				    h1(0)=ver[(i+1)*(n_subdivisions+1)+j](0)-
					  ver[(i-1)*(n_subdivisions+1)+j](0);
				    h1(1)=patch->data(0,(i+1)*(n_subdivisions+1)+j)-
					  patch->data(0,(i-1)*(n_subdivisions+1)+j);
				    h1(2)=ver[(i+1)*(n_subdivisions+1)+j](1)-
					  ver[(i-1)*(n_subdivisions+1)+j](1);
				  }
			      if (j==0)
				{
				  h2(0)=ver[i*(n_subdivisions+1)+j+1](0)-
					ver[i*(n_subdivisions+1)+j](0);
				  h2(1)=patch->data(0,i*(n_subdivisions+1)+j+1)-
					patch->data(0,i*(n_subdivisions+1)+j);
				  h2(2)=ver[i*(n_subdivisions+1)+j+1](1)-
					ver[i*(n_subdivisions+1)+j](1);
				}
			      else
				if (j==n_subdivisions)
				  {
				    h2(0)=ver[i*(n_subdivisions+1)+j](0)-
					  ver[i*(n_subdivisions+1)+j-1](0);
				    h2(1)=patch->data(0,i*(n_subdivisions+1)+j)-
					  patch->data(0,i*(n_subdivisions+1)+j-1);
				    h2(2)=ver[i*(n_subdivisions+1)+j](1)-
					  ver[i*(n_subdivisions+1)+j-1](1);
				  }
				else
				  {
				    h2(0)=ver[i*(n_subdivisions+1)+j+1](0)-
					  ver[i*(n_subdivisions+1)+j-1](0);
				    h2(1)=patch->data(0,i*(n_subdivisions+1)+j+1)-
					  patch->data(0,i*(n_subdivisions+1)+j-1);
				    h2(2)=ver[i*(n_subdivisions+1)+j+1](1)-
					  ver[i*(n_subdivisions+1)+j-1](1);
				  }
			      nrml[i*(n_subdivisions+1)+j](0)=h1(1)*h2(2)-h1(2)*h2(1);
			      nrml[i*(n_subdivisions+1)+j](1)=h1(2)*h2(0)-h1(0)*h2(2);
			      nrml[i*(n_subdivisions+1)+j](2)=h1(0)*h2(1)-h1(1)*h2(0);

							       // normalize Vector
			      double norm=std::sqrt(
				std::pow(nrml[i*(n_subdivisions+1)+j](0),2.)+
				std::pow(nrml[i*(n_subdivisions+1)+j](1),2.)+
				std::pow(nrml[i*(n_subdivisions+1)+j](2),2.));
			      
			      if (nrml[i*(n_subdivisions+1)+j](1)<0)
				norm*=-1.;
			      
			      for (unsigned int k=0;k<3;++k)
				nrml[i*(n_subdivisions+1)+j](k)/=norm;
			    }
			}
						       // writing smooth_triangles
		      
						       // down/right triangle
		      out << "smooth_triangle {" << std::endl << "\t<" 
			  << ver[dl](0) << ","   
			  << patch->data(0,dl) << ","
			  << ver[dl](1) << ">, <"
			  << nrml[dl](0) << ", " << nrml[dl](1) << ", " << nrml[dl](2)
			  << ">," << std::endl;
		      out << " \t<" 
			  << ver[dl+n_subdivisions+1](0) << "," 
			  << patch->data(0,dl+n_subdivisions+1)  << ","
			  << ver[dl+n_subdivisions+1](1) << ">, <"
			  << nrml[dl+n_subdivisions+1](0) << ", "
			  << nrml[dl+n_subdivisions+1](1) << ", "
			  << nrml[dl+n_subdivisions+1](2)
			  << ">," << std::endl;
		      out << "\t<" 
			  << ver[dl+n_subdivisions+2](0) << "," 
			  << patch->data(0,dl+n_subdivisions+2)  << ","
			  << ver[dl+n_subdivisions+2](1) << ">, <"
			  << nrml[dl+n_subdivisions+2](0) << ", "
			  << nrml[dl+n_subdivisions+2](1) << ", "
			  << nrml[dl+n_subdivisions+2](2) 
			  << ">}" << std::endl; 
		      
						       // upper/left triangle
		      out << "smooth_triangle {" << std::endl << "\t<" 
			  << ver[dl](0) << "," 
			  << patch->data(0,dl) << ","
			  << ver[dl](1) << ">, <"
			  << nrml[dl](0) << ", " << nrml[dl](1) << ", " << nrml[dl](2) 
			  << ">," << std::endl;
		      out << "\t<" 
			  << ver[dl+n_subdivisions+2](0) << "," 
			  << patch->data(0,dl+n_subdivisions+2)  << ","
			  << ver[dl+n_subdivisions+2](1) << ">, <"
			  << nrml[dl+n_subdivisions+2](0) << ", "
			  << nrml[dl+n_subdivisions+2](1) << ", "
			  << nrml[dl+n_subdivisions+2](2)
			  << ">," << std::endl;
		      out << "\t<" 
			  << ver[dl+1](0) << "," 
			  << patch->data(0,dl+1)  << ","
			  << ver[dl+1](1) << ">, <"
			  << nrml[dl+1](0) << ", " << nrml[dl+1](1) << ", " << nrml[dl+1](2)
			  << ">}" << std::endl;
		    }
		  else
		    {		
						       // writing standard triangles
						       // down/right triangle
		      out << "triangle {" << std::endl << "\t<" 
			  << ver[dl](0) << "," 
			  << patch->data(0,dl) << ","
			  << ver[dl](1) << ">," << std::endl;
		      out << "\t<" 
			  << ver[dl+n_subdivisions+1](0) << "," 
			  << patch->data(0,dl+n_subdivisions+1)  << ","
			  << ver[dl+n_subdivisions+1](1) << ">," << std::endl;
		      out << "\t<" 
			  << ver[dl+n_subdivisions+2](0) << "," 
			  << patch->data(0,dl+n_subdivisions+2)  << ","
			  << ver[dl+n_subdivisions+2](1) << ">}" << std::endl; 
			    
						       // upper/left triangle
		      out << "triangle {" << std::endl << "\t<" 
			  << ver[dl](0) << "," 
			  << patch->data(0,dl) << ","
			  << ver[dl](1) << ">," << std::endl;
		      out << "\t<"
			  << ver[dl+n_subdivisions+2](0) << "," 
			  << patch->data(0,dl+n_subdivisions+2)  << ","
			  << ver[dl+n_subdivisions+2](1) << ">," << std::endl;
		      out << "\t<" 
			  << ver[dl+1](0) << ","
			  << patch->data(0,dl+1)  << ","
			   << ver[dl+1](1) << ">}" << std::endl;
		        }
		}
	    }
	}
      else
	{                                    // writing bicubic_patch
	  Assert (n_subdivisions==3, ExcDimensionMismatch(n_subdivisions,3));
	  out << std::endl
	      << "bicubic_patch {" << std::endl
	      << "  type 0" << std::endl
	      << "  flatness 0" << std::endl
	      << "  u_steps 0" << std::endl
	      << "  v_steps 0" << std::endl;
	  for (int i=0;i<16;++i)
	    {
	      out << "\t<" << ver[i](0) << "," << patch->data(0,i) << "," << ver[i](1) << ">";
	      if (i!=15) out << ",";
	      out << std::endl;
	    }
	  out << "  texture {Tex}" <<  std::endl
	      << "}" << std::endl;
	}
    }
  
  if (!flags.bicubic_patch) 
    {                                   // the end of the mesh
      out << "  texture {Tex}" << std::endl
	  << "}" << std::endl
	  << std::endl;
    }
  
  AssertThrow (out, ExcIO());
}



template <int dim, int spacedim>
void DataOutBase::write_eps (const std::vector<Patch<dim,spacedim> > &patches,
			     const std::vector<std::string>          &/*data_names*/,
			     const EpsFlags                          &flags,
			     std::ostream                            &out) 
{
  Assert (out, ExcIO());
  
  Assert (patches.size() > 0, ExcNoPatches());

				   // Do not allow volume rendering
  AssertThrow (dim<=2, ExcNotImplemented());
  
  switch (dim) 
    {
      case 2:
      {
					 // set up an array of cells to be
					 // written later. this array holds the
					 // cells of all the patches as
					 // projected to the plane perpendicular
					 // to the line of sight.
					 //
					 // note that they are kept sorted by
					 // the set, where we chose the value
					 // of the center point of the cell
					 // along the line of sight as value
					 // for sorting
	std::multiset<EpsCell2d> cells;

					 // two variables in which we
					 // will store the minimum and
					 // maximum values of the field
					 // to be used for colorization
					 //
					 // preset them by 0 to calm down the
					 // compiler; they are initialized later
	double min_color_value=0, max_color_value=0;
	
					 // Array for z-coordinates of points.
					 // The elevation determined by a function if spacedim=2
					 // or the z-cooridate of the grid point if spacedim=3
	double heights[4];

					 // compute the cells for output and
					 // enter them into the set above
					 // note that since dim==2, we
					 // have exactly four vertices per
					 // patch and per cell
	for (typename std::vector<Patch<dim,spacedim> >::const_iterator patch=patches.begin();
	     patch!=patches.end(); ++patch)
	  {
	    const unsigned int n_subdivisions = patch->n_subdivisions;
	    for (unsigned int i=0; i<n_subdivisions; ++i)
	      for (unsigned int j=0; j<n_subdivisions; ++j)
		{
		  const double x_frac = i * 1./n_subdivisions,
			       y_frac = j * 1./n_subdivisions,
					
			      x_frac1 = (i+1) * 1./n_subdivisions,
			      y_frac1 = (j+1) * 1./n_subdivisions;
		  
		  const Point<spacedim> points[4]
		    = { (((patch->vertices[1] * x_frac) +
			  (patch->vertices[0] * (1-x_frac))) * (1-y_frac) +
			 ((patch->vertices[2] * x_frac) +
			  (patch->vertices[3] * (1-x_frac))) * y_frac),

			(((patch->vertices[1] * x_frac1) +
			  (patch->vertices[0] * (1-x_frac1))) * (1-y_frac) +
			 ((patch->vertices[2] * x_frac1) +
			  (patch->vertices[3] * (1-x_frac1))) * y_frac),
			
			(((patch->vertices[1] * x_frac1) +
			  (patch->vertices[0] * (1-x_frac1))) * (1-y_frac1) +
			 ((patch->vertices[2] * x_frac1) +
			  (patch->vertices[3] * (1-x_frac1))) * y_frac1),
			
			(((patch->vertices[1] * x_frac) +
			  (patch->vertices[0] * (1-x_frac))) * (1-y_frac1) +
			 ((patch->vertices[2] * x_frac) +
			  (patch->vertices[3] * (1-x_frac))) * y_frac1) 
		    };
		  
		  switch (spacedim)
		    {
		    case 2:
		      Assert ((flags.height_vector < patch->data.n_rows()) ||
			      patch->data.n_rows() == 0,
			      ExcIndexRange (flags.height_vector, 0,
					     patch->data.n_rows()));
		      heights[0] = patch->data.n_rows() != 0 ?
			patch->data(flags.height_vector,i*(n_subdivisions+1) + j) * flags.z_scaling
			: 0;
			
		      heights[1] = patch->data.n_rows() != 0 ?
			patch->data(flags.height_vector,(i+1)*(n_subdivisions+1) + j) * flags.z_scaling
			: 0;
			
		      heights[2] = patch->data.n_rows() != 0 ?
			patch->data(flags.height_vector,(i+1)*(n_subdivisions+1) + j+1) * flags.z_scaling
			: 0;
			
		      heights[3] = patch->data.n_rows() != 0 ?
			patch->data(flags.height_vector,i*(n_subdivisions+1) + j+1) * flags.z_scaling
			: 0;
		      break;
		    case 3:
						       // Copy z-coordinates into the height vector
		      for (unsigned int i=0;i<4;++i)
			heights[i] = points[i](2);
		      break;
		    default:
		      Assert(false, ExcNotImplemented());
		    }
		  

						   // now compute the projection of
						   // the bilinear cell given by the
						   // four vertices and their heights
						   // and write them to a proper
						   // cell object. note that we only
						   // need the first two components
						   // of the projected position for
						   // output, but we need the value
						   // along the line of sight for
						   // sorting the cells for back-to-
						   // front-output
						   //
						   // this computation was first written
						   // by Stefan Nauber. please no-one
						   // ask me why it works that way (or
						   // may be not), especially not about
						   // the angles and the sign of
						   // the height field, I don't know
						   // it.
		  EpsCell2d eps_cell;
		  const double pi = deal_II_numbers::PI;
		  const double cx = -std::cos(pi-flags.azimut_angle * 2*pi / 360.),
			       cz = -std::cos(flags.turn_angle * 2*pi / 360.),
			       sx = std::sin(pi-flags.azimut_angle * 2*pi / 360.),
			       sz = std::sin(flags.turn_angle * 2*pi / 360.);
		  for (unsigned int vertex=0; vertex<4; ++vertex)
		    {
		      const double x = points[vertex](0),
			y = points[vertex](1),
			z = -heights[vertex];
		      
		      eps_cell.vertices[vertex](0) = -   cz*x+   sz*y;
		      eps_cell.vertices[vertex](1) = -cx*sz*x-cx*cz*y-sx*z;

						       //      ( 1 0    0 )
						       // Dx = ( 0 cx -sx ) 
						       //      ( 0 sx  cx )

						       //      ( cy 0 sy )
						       // Dy = (  0 1  0 )
						       //      (-sy 0 cy )

						       //      ( cz -sz 0 )
						       // Dz = ( sz  cz 0 )
						       //      (  0   0 1 )

//       ( cz -sz 0 )( 1 0    0 )(x)   ( cz*x-sz*(cx*y-sx*z)+0*(sx*y+cx*z) )
// Dxz = ( sz  cz 0 )( 0 cx -sx )(y) = ( sz*x+cz*(cx*y-sx*z)+0*(sx*y+cx*z) )
// 	 (  0   0 1 )( 0 sx  cx )(z)   (  0*x+	*(cx*y-sx*z)+1*(sx*y+cx*z) )
		    }

						   // compute coordinates of
						   // center of cell
		  const Point<spacedim> center_point
		    = (points[0] + points[1] + points[2] + points[3]) / 4;
		  const double center_height
		    = -(heights[0] + heights[1] + heights[2] + heights[3]) / 4;

						   // compute the depth into
						   // the picture
		  eps_cell.depth = -sx*sz*center_point(0)
				   -sx*cz*center_point(1)
				   +cx*center_height;

		  if (flags.draw_cells && flags.shade_cells)
		    {
		      Assert ((flags.color_vector < patch->data.n_rows()) ||
			      patch->data.n_rows() == 0,
			      ExcIndexRange (flags.color_vector, 0,
					     patch->data.n_rows()));
		      const double color_values[4]
			= { patch->data.n_rows() != 0 ?
			    patch->data(flags.color_vector,i*(n_subdivisions+1) + j)       : 1,
			
			    patch->data.n_rows() != 0 ?
			    patch->data(flags.color_vector,(i+1)*(n_subdivisions+1) + j)   : 1,
			    
			    patch->data.n_rows() != 0 ?
			    patch->data(flags.color_vector,(i+1)*(n_subdivisions+1) + j+1) : 1,
			    
			    patch->data.n_rows() != 0 ?
			    patch->data(flags.color_vector,i*(n_subdivisions+1) + j+1)     : 1};

						       // set color value to average of the value
						       // at the vertices
		      eps_cell.color_value = (color_values[0] +
					      color_values[1] +
					      color_values[2] +
					      color_values[3]) / 4;

						       // update bounds of color
						       // field
		      if (patch == patches.begin())
			min_color_value = max_color_value = eps_cell.color_value;
		      else
			{
			  min_color_value = (min_color_value < eps_cell.color_value ?
					     min_color_value : eps_cell.color_value);
			  max_color_value = (max_color_value > eps_cell.color_value ?
					     max_color_value : eps_cell.color_value);
			}
		    }
		  
						   // finally add this cell
		  cells.insert (eps_cell);
		}
	  }

					 // find out minimum and maximum x and
					 // y coordinates to compute offsets
					 // and scaling factors
	double x_min = cells.begin()->vertices[0](0);
	double x_max = x_min;
	double y_min = cells.begin()->vertices[0](1);
	double y_max = y_min;
	
	for (typename std::multiset<EpsCell2d>::const_iterator
	       cell=cells.begin();
	     cell!=cells.end(); ++cell)
	  for (unsigned int vertex=0; vertex<4; ++vertex)
	    {
	      x_min = std::min (x_min, cell->vertices[vertex](0));
	      x_max = std::max (x_max, cell->vertices[vertex](0));
	      y_min = std::min (y_min, cell->vertices[vertex](1));
	      y_max = std::max (y_max, cell->vertices[vertex](1));
	    }
	
					 // scale in x-direction such that
					 // in the output 0 <= x <= 300.
					 // don't scale in y-direction to
					 // preserve the shape of the
					 // triangulation
	const double scale = (flags.size /
			      (flags.size_type==EpsFlags::width ?
			       x_max - x_min :
			       y_min - y_max));
	
	const Point<2> offset(x_min, y_min);


					 // now write preamble
	if (true) 
	  {
					     // block this to have local
					     // variables destroyed after
					     // use
	    std::time_t  time1= std::time (0);
	    std::tm     *time = std::localtime(&time1); 
	    out << "%!PS-Adobe-2.0 EPSF-1.2" << std::endl
		<< "%%Title: deal.II Output" << std::endl
		<< "%%Creator: the deal.II library" << std::endl
		<< "%%Creation Date: " 
		<< time->tm_year+1900 << "/"
		<< time->tm_mon+1 << "/"
		<< time->tm_mday << " - "
		<< time->tm_hour << ":"
		<< std::setw(2) << time->tm_min << ":"
		<< std::setw(2) << time->tm_sec << std::endl
		<< "%%BoundingBox: "
					       // lower left corner
		<< "0 0 "
					       // upper right corner
		<< static_cast<unsigned int>( rint((x_max-x_min) * scale ))
		<< ' '
		<< static_cast<unsigned int>( rint((y_max-y_min) * scale ))
		<< std::endl;
	    
					     // define some abbreviations to keep
					     // the output small:
					     // m=move turtle to
					     // l=define a line
					     // s=set rgb color
					     // sg=set gray value
					     // lx=close the line and plot the line
					     // lf=close the line and fill the interior
	    out << "/m {moveto} bind def"      << std::endl
		<< "/l {lineto} bind def"      << std::endl
		<< "/s {setrgbcolor} bind def" << std::endl
		<< "/sg {setgray} bind def"    << std::endl
		<< "/lx {lineto closepath stroke} bind def" << std::endl
		<< "/lf {lineto closepath fill} bind def"   << std::endl;
	    
	    out << "%%EndProlog" << std::endl
		<< std::endl;
					     // set fine lines
	    out << flags.line_width << " setlinewidth" << std::endl;
					     // allow only five digits
					     // for output (instead of the
					     // default six); this should suffice
					     // even for fine grids, but reduces
					     // the file size significantly
	    out << std::setprecision (5);
	  }

					 // check if min and max
					 // values for the color are
					 // actually different. If
					 // that is not the case (such
					 // things happen, for
					 // example, in the very first
					 // time step of a time
					 // dependent problem, if the
					 // initial values are zero),
					 // all values are equal, and
					 // then we can draw
					 // everything in an arbitrary
					 // color. Thus, change one of
					 // the two values arbitrarily
	if (max_color_value == min_color_value)
	  max_color_value = min_color_value+1;

					 // now we've got all the information
					 // we need. write the cells.
					 // note: due to the ordering, we
					 // traverse the list of cells
					 // back-to-front
	for (typename std::multiset<EpsCell2d>::const_iterator
	       cell=cells.begin();
	     cell!=cells.end(); ++cell)
	  {
	    if (flags.draw_cells) 
	      {
		if (flags.shade_cells)
		  {
		    const EpsFlags::RgbValues rgb_values
		      = (*flags.color_function) (cell->color_value,
						 min_color_value,
						 max_color_value);

						     // write out color
		    if (rgb_values.is_grey())
		      out << rgb_values.red << " sg ";
		    else
		      out << rgb_values.red   << ' '
			  << rgb_values.green << ' '
			  << rgb_values.blue  << " s ";
		  }
		else
		  out << "1 sg ";

		out << (cell->vertices[0]-offset) * scale << " m "
		    << (cell->vertices[1]-offset) * scale << " l "
		    << (cell->vertices[2]-offset) * scale << " l "
		    << (cell->vertices[3]-offset) * scale << " lf"
		    << std::endl;
	      }
	    
	    if (flags.draw_mesh)
	      out << "0 sg "      // draw lines in black
		  << (cell->vertices[0]-offset) * scale << " m "
		  << (cell->vertices[1]-offset) * scale << " l "
		  << (cell->vertices[2]-offset) * scale << " l "
		  << (cell->vertices[3]-offset) * scale << " lx"
		  << std::endl;
	  }
	out << "showpage" << std::endl;
	
	break;
      }
       
      default:
	    Assert (false, ExcNotImplemented());
    }
}



template <int dim, int spacedim>
void DataOutBase::write_gmv (const std::vector<Patch<dim,spacedim> > &patches,
			     const std::vector<std::string>          &data_names,
			     const GmvFlags                          &/*flags*/,
			     std::ostream                            &out) 
{
  AssertThrow (out, ExcIO());

  Assert (patches.size() > 0, ExcNoPatches());
 
  const unsigned int n_data_sets = data_names.size();
				   // check against # of data sets in
				   // first patch. checks against all
				   // other patches are made in
				   // write_gmv_reorder_data_vectors
  Assert (n_data_sets == patches[0].data.n_rows(),
	  ExcDimensionMismatch (patches[0].data.n_rows(), n_data_sets));
  
  
				   ///////////////////////
				   // preamble
  out << "gmvinput ascii"
      << std::endl
      << std::endl;

				   // first count the number of cells
				   // and cells for later use
  unsigned int n_cells = 0,
	       n_nodes = 0;
  for (typename std::vector<Patch<dim,spacedim> >::const_iterator patch=patches.begin();
       patch!=patches.end(); ++patch)
    switch (dim)
      {
	case 1:
	      n_cells += patch->n_subdivisions;
	      n_nodes += patch->n_subdivisions+1;
	      break;
	case 2:
	      n_cells += patch->n_subdivisions *
			 patch->n_subdivisions;
	      n_nodes += (patch->n_subdivisions+1) *
			 (patch->n_subdivisions+1);
	      break;
	case 3:
	      n_cells += patch->n_subdivisions *
			 patch->n_subdivisions *
			 patch->n_subdivisions;
	      n_nodes += (patch->n_subdivisions+1) *
			 (patch->n_subdivisions+1) *
			 (patch->n_subdivisions+1);
	      break;
	default:
	      Assert (false, ExcNotImplemented());
      }


				   // in gmv format the vertex
				   // coordinates and the data have an
				   // order that is a bit unpleasant
				   // (first all x coordinates, then
				   // all y coordinate, ...; first all
				   // data of variable 1, then
				   // variable 2, etc), so we have to
				   // copy the data vectors a bit around
				   //
				   // note that we copy vectors when
				   // looping over the patches since we
				   // have to write them one variable
				   // at a time and don't want to use
				   // more than one loop
				   //
				   // this copying of data vectors can
				   // be done while we already output
				   // the vertices, so do this on a
				   // separate thread and when wanting
				   // to write out the data, we wait
				   // for that thread to finish
  Table<2,double> data_vectors (n_data_sets, n_nodes);
  void (*fun_ptr) (const std::vector<Patch<dim,spacedim> > &,
		   Table<2,double> &)
    = &DataOutBase::template write_gmv_reorder_data_vectors<dim,spacedim>;
  Threads::Thread<> reorder_thread = Threads::spawn (fun_ptr)(patches, data_vectors);

				   ///////////////////////////////
				   // first make up a list of used
				   // vertices along with their
				   // coordinates
				   //
				   // note that we have to print
				   // d=1..3 dimensions
  out << "nodes " << n_nodes << std::endl;
  for (unsigned int d=1; d<=3; ++d)
    {
      for (typename std::vector<Patch<dim,spacedim> >::const_iterator patch=patches.begin();
	   patch!=patches.end(); ++patch)
	{
	  const unsigned int n_subdivisions = patch->n_subdivisions;
	  
					   // if we have nonzero values for
					   // this coordinate
	  if (d<=spacedim)
	    {
	      switch (dim)
		{
		  case 1:
		  {
		    for (unsigned int i=0; i<n_subdivisions+1; ++i)
		      out << ((patch->vertices[1](0) * i / n_subdivisions) +
			      (patch->vertices[0](0) * (n_subdivisions-i) / n_subdivisions))
			  << ' ';
		    break;
		  }
		   
		  case 2:
		  {
		    for (unsigned int i=0; i<n_subdivisions+1; ++i)
		      for (unsigned int j=0; j<n_subdivisions+1; ++j)
			{
			  const double x_frac = i * 1./n_subdivisions,
				       y_frac = j * 1./n_subdivisions;
			  
							   // compute coordinates for
							   // this patch point
			  out << (((patch->vertices[1](d-1) * x_frac) +
				   (patch->vertices[0](d-1) * (1-x_frac))) * (1-y_frac) +
				  ((patch->vertices[2](d-1) * x_frac) +
				   (patch->vertices[3](d-1) * (1-x_frac))) * y_frac)
			      << ' ';
			}
		    break;
		  }
		   
		  case 3:
		  {
		    for (unsigned int i=0; i<n_subdivisions+1; ++i)
		      for (unsigned int j=0; j<n_subdivisions+1; ++j)
			for (unsigned int k=0; k<n_subdivisions+1; ++k)
			  {
							     // note the broken
							     // design of hexahedra
							     // in deal.II, where
							     // first the z-component
							     // is counted up, before
							     // increasing the y-
							     // coordinate
			    const double x_frac = i * 1./n_subdivisions,
					 y_frac = k * 1./n_subdivisions,
					 z_frac = j * 1./n_subdivisions;
			    
							     // compute coordinates for
							     // this patch point
			    out << ((((patch->vertices[1](d-1) * x_frac) +
				      (patch->vertices[0](d-1) * (1-x_frac))) * (1-y_frac) +
				     ((patch->vertices[2](d-1) * x_frac) +
				      (patch->vertices[3](d-1) * (1-x_frac))) * y_frac)   * (1-z_frac) +
				    (((patch->vertices[5](d-1) * x_frac) +
				      (patch->vertices[4](d-1) * (1-x_frac))) * (1-y_frac) +
				     ((patch->vertices[6](d-1) * x_frac) +
				      (patch->vertices[7](d-1) * (1-x_frac))) * y_frac)   * z_frac)
				<< ' ';
			  }
	      
		    break;
		  }
		   
		  default:
			Assert (false, ExcNotImplemented());
		}
	    }
	  else
					     // d>spacedim. write zeros instead
	    {
	      const unsigned int n_points
		= static_cast<unsigned int>(std::pow (static_cast<double>(n_subdivisions+1), dim));
	      for (unsigned int i=0; i<n_points; ++i)
		out << "0 ";
	    }
	}
      out << std::endl;
    }

  out << std::endl;

				   /////////////////////////////////
				   // now for the cells. note that
				   // vertices are counted from 1 onwards
  if (true)
    {
      out << "cells " << n_cells << std::endl;


      unsigned int first_vertex_of_patch = 0;
      
      for (typename std::vector<Patch<dim,spacedim> >::const_iterator patch=patches.begin();
	   patch!=patches.end(); ++patch)
	{
	  const unsigned int n_subdivisions = patch->n_subdivisions;

					   // write out the cells making
					   // up this patch
	  switch (dim)
	    {
	      case 1:
	      {
		for (unsigned int i=0; i<n_subdivisions; ++i)
		  out << "line 2\n  "
		      << first_vertex_of_patch+i+1 << ' '
		      << first_vertex_of_patch+i+1+1 << std::endl;
		break;
	      }
	       
	      case 2:
	      {
		for (unsigned int i=0; i<n_subdivisions; ++i)
		  for (unsigned int j=0; j<n_subdivisions; ++j)
		    out << "quad 4\n  "
			<< first_vertex_of_patch+i*(n_subdivisions+1)+j+1 << ' '
			<< first_vertex_of_patch+(i+1)*(n_subdivisions+1)+j+1 << ' '
			<< first_vertex_of_patch+(i+1)*(n_subdivisions+1)+j+1+1 << ' '
			<< first_vertex_of_patch+i*(n_subdivisions+1)+j+1+1
			<< std::endl;
		break;
	      }
	       
	      case 3:
	      {
		for (unsigned int i=0; i<n_subdivisions; ++i)
		  for (unsigned int j=0; j<n_subdivisions; ++j)
		    for (unsigned int k=0; k<n_subdivisions; ++k)
		      {
			out << "hex 8\n   "
							   // note: vertex indices start with 1!
			    << first_vertex_of_patch+(i*(n_subdivisions+1)+j      )*(n_subdivisions+1)+k  +1 << ' '
			    << first_vertex_of_patch+((i+1)*(n_subdivisions+1)+j  )*(n_subdivisions+1)+k  +1 << ' '
			    << first_vertex_of_patch+((i+1)*(n_subdivisions+1)+j+1)*(n_subdivisions+1)+k  +1 << ' '
			    << first_vertex_of_patch+(i*(n_subdivisions+1)+j+1    )*(n_subdivisions+1)+k  +1 << ' '
			    << first_vertex_of_patch+(i*(n_subdivisions+1)+j      )*(n_subdivisions+1)+k+1+1 << ' '
			    << first_vertex_of_patch+((i+1)*(n_subdivisions+1)+j  )*(n_subdivisions+1)+k+1+1 << ' '
			    << first_vertex_of_patch+((i+1)*(n_subdivisions+1)+j+1)*(n_subdivisions+1)+k+1+1 << ' '
			    << first_vertex_of_patch+(i*(n_subdivisions+1)+j+1    )*(n_subdivisions+1)+k+1+1 << ' '
			    << std::endl;
		      }
		break;
	      }

	      default:
		    Assert (false, ExcNotImplemented());
	    }


					   // finally update the number
					   // of the first vertex of this patch
	  switch (dim)
	    {
	      case 1:
		    first_vertex_of_patch += n_subdivisions+1;
		    break;
	      case 2:
		    first_vertex_of_patch += (n_subdivisions+1) *
					     (n_subdivisions+1);
		    break;
	      case 3:
		    first_vertex_of_patch += (n_subdivisions+1) *
					     (n_subdivisions+1) *
					     (n_subdivisions+1);
		    break;
	      default:
		    Assert (false, ExcNotImplemented());
	    }
	}
      out << std::endl;
    }

				   ///////////////////////////////////////
				   // data output.
  out << "variable" << std::endl;

				   // now write the data vectors to
				   // @p{out} first make sure that all
				   // data is in place
  reorder_thread.join ();

				   // then write data.
				   // the '1' means: node data (as opposed
				   // to cell data, which we do not
				   // support explicitly here)
  for (unsigned int data_set=0; data_set<n_data_sets; ++data_set)
    {
      out << data_names[data_set] << " 1" << std::endl;
      std::copy (data_vectors[data_set].begin(),
		 data_vectors[data_set].end(),
		 std::ostream_iterator<double>(out, " "));
      out << std::endl
	  << std::endl;
    }


  
				   // end of variable section
  out << "endvars" << std::endl;
  
				   // end of output
  out << "endgmv"
      << std::endl;
  
				   // assert the stream is still ok
  AssertThrow (out, ExcIO());
}



template <int dim, int spacedim>
void DataOutBase::write_tecplot (const std::vector<Patch<dim,spacedim> > &patches,
				 const std::vector<std::string>          &data_names,
				 const TecplotFlags                      &/*flags*/,
				 std::ostream                            &out)
{
  AssertThrow (out, ExcIO());

  Assert (patches.size() > 0, ExcNoPatches());
 
  const unsigned int n_data_sets = data_names.size();
				   // check against # of data sets in
				   // first patch. checks against all
				   // other patches are made in
				   // write_gmv_reorder_data_vectors
  Assert (n_data_sets == patches[0].data.n_rows(),
	  ExcDimensionMismatch (patches[0].data.n_rows(), n_data_sets));
  

  
				   // first count the number of cells
				   // and cells for later use
  unsigned int n_cells = 0,
               n_nodes = 0;
  for (typename std::vector<Patch<dim,spacedim> >::const_iterator patch=patches.begin();
       patch!=patches.end(); ++patch)
    switch (dim)
      {
	case 1:
	      n_cells += patch->n_subdivisions;
	      n_nodes += patch->n_subdivisions+1;
	      break;
	case 2:
	      n_cells += patch->n_subdivisions *
			 patch->n_subdivisions;
	      n_nodes += (patch->n_subdivisions+1) *
			 (patch->n_subdivisions+1);
	      break;
	case 3:
	      n_cells += patch->n_subdivisions *
			 patch->n_subdivisions *
			 patch->n_subdivisions;
	      n_nodes += (patch->n_subdivisions+1) *
			 (patch->n_subdivisions+1) *
			 (patch->n_subdivisions+1);
	      break;
	default:
	      Assert (false, ExcNotImplemented());
      }

  


				   ///////////
				   // preamble
  {
    std::time_t  time1= std::time (0);
    std::tm     *time = std::localtime(&time1); 
    out << "# This file was generated by the deal.II library." << std::endl
	<< "# Date =  "
	<< time->tm_year+1900 << "/"
	<< time->tm_mon+1 << "/"
	<< time->tm_mday << std::endl
	<< "# Time =  "
	<< time->tm_hour << ":"
	<< std::setw(2) << time->tm_min << ":"
	<< std::setw(2) << time->tm_sec << std::endl
	<< "#" << std::endl
	<< "# For a description of the Tecplot format see the Tecplot documentation."
	<< std::endl
	<< "#" << std::endl;
    

    out << "Variables=";

    switch (spacedim)
      {
        case 1:
	      out << "\"x\"";
	      break;
        case 2:
	      out << "\"x\", \"y\"";
	      break;
        case 3:
	      out << "\"x\", \"y\", \"z\"";
	      break;
	default:
	      Assert (false, ExcNotImplemented());
      }
    
    for (unsigned int data_set=0; data_set<n_data_sets; ++data_set)
      out << ", \"" << data_names[data_set] << "\"";
    
    out << std::endl;
    
    if (dim > 1)
      {
	out << "zone f=feblock, n=" << n_nodes << ", e=" << n_cells << ", et=";
	
	switch (dim)
	  {
	    case 2:
		  out << "quadrilateral" << std::endl;
		  break;
	    case 3:
		  out << "brick" << std::endl;
		  break;
	    default:
		  Assert (false, ExcNotImplemented());
	  }
      }
    else
      out << "zone f=block, n=" << n_nodes << std::endl;
  }

  
                                   // in Tecplot FEBLOCK format the vertex
                                   // coordinates and the data have an
                                   // order that is a bit unpleasant
                                   // (first all x coordinates, then
                                   // all y coordinate, ...; first all
                                   // data of variable 1, then
                                   // variable 2, etc), so we have to
                                   // copy the data vectors a bit around
                                   //
                                   // note that we copy vectors when
                                   // looping over the patches since we
                                   // have to write them one variable
                                   // at a time and don't want to use
                                   // more than one loop
                                   //
                                   // this copying of data vectors can
                                   // be done while we already output
                                   // the vertices, so do this on a
                                   // separate thread and when wanting
                                   // to write out the data, we wait
                                   // for that thread to finish
  
  Table<2,double> data_vectors (n_data_sets, n_nodes);

  void (*fun_ptr) (const std::vector<Patch<dim,spacedim> > &,
                   Table<2,double> &)
    = &DataOutBase::template write_gmv_reorder_data_vectors<dim,spacedim>;
  Threads::Thread<> reorder_thread = Threads::spawn (fun_ptr)(patches, data_vectors);

                                   ///////////////////////////////
                                   // first make up a list of used
                                   // vertices along with their
                                   // coordinates

  
  for (unsigned int d=1; d<=spacedim; ++d)
    {       
          
      for (typename std::vector<Patch<dim,spacedim> >::const_iterator patch=patches.begin();
           patch!=patches.end(); ++patch)
        {
          const unsigned int n_subdivisions = patch->n_subdivisions;
	  
          switch (dim)
            {
              case 1:
              {
                for (unsigned int i=0; i<n_subdivisions+1; ++i)
                  out << ((patch->vertices[1](0) * i / n_subdivisions) +
                          (patch->vertices[0](0) * (n_subdivisions-i) / n_subdivisions))
                      << '\n';
                break;
              }
	      
              case 2:
              {
                for (unsigned int i=0; i<n_subdivisions+1; ++i)
                  for (unsigned int j=0; j<n_subdivisions+1; ++j)
                    {
                      const double x_frac = i * 1./n_subdivisions,
                                   y_frac = j * 1./n_subdivisions;
		      
                                                       // compute coordinates for
                                                       // this patch point

                      out << (((patch->vertices[1](d-1) * x_frac) +
                               (patch->vertices[0](d-1) * (1-x_frac))) * (1-y_frac) +
                              ((patch->vertices[2](d-1) * x_frac) +
                               (patch->vertices[3](d-1) * (1-x_frac))) * y_frac)
                          << '\n';
                    }
                break;
              }
	      
              case 3:
              {
                for (unsigned int i=0; i<n_subdivisions+1; ++i)
                  for (unsigned int j=0; j<n_subdivisions+1; ++j)
                    for (unsigned int k=0; k<n_subdivisions+1; ++k)
                      {
                                                         // note the broken
                                                         // design of hexahedra
                                                         // in deal.II, where
                                                         // first the z-component
                                                         // is counted up, before
                                                         // increasing the y-
                                                         // coordinate
                        const double x_frac = i * 1./n_subdivisions,
                                     y_frac = k * 1./n_subdivisions,
                                     z_frac = j * 1./n_subdivisions;
			
                                                         // compute coordinates for
                                                         // this patch point
			 
                        out << ((((patch->vertices[1](d-1) * x_frac) +
                                  (patch->vertices[0](d-1) * (1-x_frac))) * (1-y_frac) +
                                 ((patch->vertices[2](d-1) * x_frac) +
                                  (patch->vertices[3](d-1) * (1-x_frac))) * y_frac)   * (1-z_frac) +
                                (((patch->vertices[5](d-1) * x_frac) +
                                  (patch->vertices[4](d-1) * (1-x_frac))) * (1-y_frac) +
                                 ((patch->vertices[6](d-1) * x_frac) +
                                  (patch->vertices[7](d-1) * (1-x_frac))) * y_frac)   * z_frac)
                            << '\n';
                      }
                break;
              }
	      
              default:
                    Assert (false, ExcNotImplemented());
            }
        }
      out << std::endl;
    }


                                   ///////////////////////////////////////
                                   // data output.
                                   //
                                   // now write the data vectors to
                                   // @p{out} first make sure that all
                                   // data is in place
  reorder_thread.join ();

                                   // then write data.
  for (unsigned int data_set=0; data_set<n_data_sets; ++data_set)
    {
      std::copy (data_vectors[data_set].begin(),
		 data_vectors[data_set].end(),
		 std::ostream_iterator<double>(out, "\n"));
      out << std::endl;
    }

  
                                   /////////////////////////////////
                                   // now for the cells. note that
                                   // vertices are counted from 1 onwards

  unsigned int first_vertex_of_patch = 0;
      
  for (typename std::vector<Patch<dim,spacedim> >::const_iterator patch=patches.begin();
       patch!=patches.end(); ++patch)
    {
      const unsigned int n_subdivisions = patch->n_subdivisions;
      
                                       // write out the cells making
                                       // up this patch
      switch (dim)
        {
          case 1:
          {
            break;
          }
          case 2:
          {
            for (unsigned int i=0; i<n_subdivisions; ++i)
              for (unsigned int j=0; j<n_subdivisions; ++j)
                {

                  out << first_vertex_of_patch+i*(n_subdivisions+1)+j+1 << ' '
                      << first_vertex_of_patch+(i+1)*(n_subdivisions+1)+j+1 << ' '
                      << first_vertex_of_patch+(i+1)*(n_subdivisions+1)+j+1+1 << ' '
                      << first_vertex_of_patch+i*(n_subdivisions+1)+j+1+1
                      << std::endl;
                }
            break;
          }
	      
          case 3:
          {
            for (unsigned int i=0; i<n_subdivisions; ++i)
              for (unsigned int j=0; j<n_subdivisions; ++j)
                for (unsigned int k=0; k<n_subdivisions; ++k)
                  {
                                                     // note: vertex indices start with 1!
		     
                    out << first_vertex_of_patch+(i*(n_subdivisions+1)+j      )*(n_subdivisions+1)+k  +1 << ' '
                        << first_vertex_of_patch+((i+1)*(n_subdivisions+1)+j  )*(n_subdivisions+1)+k  +1 << ' '
                        << first_vertex_of_patch+((i+1)*(n_subdivisions+1)+j+1)*(n_subdivisions+1)+k  +1 << ' '
                        << first_vertex_of_patch+(i*(n_subdivisions+1)+j+1    )*(n_subdivisions+1)+k  +1 << ' '
                        << first_vertex_of_patch+(i*(n_subdivisions+1)+j      )*(n_subdivisions+1)+k+1+1 << ' '
                        << first_vertex_of_patch+((i+1)*(n_subdivisions+1)+j  )*(n_subdivisions+1)+k+1+1 << ' '
                        << first_vertex_of_patch+((i+1)*(n_subdivisions+1)+j+1)*(n_subdivisions+1)+k+1+1 << ' '
                        << first_vertex_of_patch+(i*(n_subdivisions+1)+j+1    )*(n_subdivisions+1)+k+1+1 << ' '
                        << std::endl;
                  }
            break;
          }

          default:
                Assert (false, ExcNotImplemented());
        }


                                       // finally update the number
                                       // of the first vertex of this patch
      switch (dim)
        {
          case 1:
                first_vertex_of_patch += n_subdivisions+1;
                break;
          case 2:
                first_vertex_of_patch += (n_subdivisions+1) *
                                         (n_subdivisions+1);
                break;
          case 3:
                first_vertex_of_patch += (n_subdivisions+1) *
                                         (n_subdivisions+1) *
                                         (n_subdivisions+1);
                break;
          default:
                Assert (false, ExcNotImplemented());
        }
       
    }

  
                                   // assert the stream is still ok
  AssertThrow (out, ExcIO());
}



//---------------------------------------------------------------------------
// Macros for handling Tecplot API data

#ifdef DEAL_II_HAVE_TECPLOT

#ifdef DEAL_II_ANON_NAMESPACE_BOGUS_WARNING
  namespace TEC
#else
  namespace
#endif
{
  class TecplotMacros
  {
    public:
      TecplotMacros(const unsigned int n_nodes = 0,
                    const unsigned int n_vars = 0,
                    const unsigned int n_cells = 0,
                    const unsigned int n_vert = 0);
      ~TecplotMacros();
      float & nd(const unsigned int i, const unsigned int j);
      int   & cd(const unsigned int i, const unsigned int j);
      std::vector<float> nodalData;
      std::vector<int>   connData;
    private:
      unsigned int n_nodes;
      unsigned int n_vars;
      unsigned int n_cells;
      unsigned int n_vert;
  };


  inline
  TecplotMacros::TecplotMacros(const unsigned int n_nodes,
			       const unsigned int n_vars,
			       const unsigned int n_cells,
			       const unsigned int n_vert)
                  :
		  n_nodes(n_nodes),
		  n_vars(n_vars),
		  n_cells(n_cells),
		  n_vert(n_vert)
  {
    nodalData.resize(n_nodes*n_vars);
    connData.resize(n_cells*n_vert);
  }



  inline
  TecplotMacros::~TecplotMacros()
  {}



  inline
  float & TecplotMacros::nd (const unsigned int i,
                             const unsigned int j)
  {
    return nodalData[i*n_nodes+j]; 
  }



  inline
  int & TecplotMacros::cd (const unsigned int i,
                           const unsigned int j)
  {
    return connData[i+j*n_vert]; 
  }
 
}


#endif
//---------------------------------------------------------------------------



template <int dim, int spacedim>
void DataOutBase::write_tecplot_binary (const std::vector<Patch<dim,spacedim> > &patches,
					const std::vector<std::string>          &data_names,
					const TecplotFlags                      &flags,
					std::ostream                            &out)
{
  
#ifndef DEAL_II_HAVE_TECPLOT
  
                                   // simply call the ASCII output
                                   // function if the Tecplot API
                                   // isn't present
  write_tecplot (patches, data_names, flags, out);
  return;
  
#else
  
                                   // Tecplot binary output only good
                                   // for 2D & 3D
  if (dim == 1)
    {
      write_tecplot (patches, data_names, flags, out);
      return;
    }

                                   // if the user hasn't specified a
                                   // file name we should call the
                                   // ASCII function and use the
                                   // ostream @p{out} instead of doing
                                   // something silly later
  char* file_name = (char*) flags.tecplot_binary_file_name;

  if (file_name == NULL)
    {
				       // At least in debug mode we
				       // should tell users why they
				       // don't get tecplot binary
				       // output
      Assert(false, ExcMessage("Specify the name of the tecplot_binary"
			       " file through the TecplotFlags interface."));
      write_tecplot (patches, data_names, flags, out);
      return;      
    }
  
  
  AssertThrow (out, ExcIO());

  Assert (patches.size() > 0, ExcNoPatches());
 
  const unsigned int n_data_sets = data_names.size();
				   // check against # of data sets in
				   // first patch. checks against all
				   // other patches are made in
				   // write_gmv_reorder_data_vectors
  Assert (n_data_sets == patches[0].data.n_rows(),
	  ExcDimensionMismatch (patches[0].data.n_rows(), n_data_sets));
  

  
				   // first count the number of cells
				   // and cells for later use
  unsigned int n_cells = 0,
               n_nodes = 0;
  for (typename std::vector<Patch<dim,spacedim> >::const_iterator
         patch=patches.begin();
       patch!=patches.end(); ++patch)
    switch (dim)
      {
	case 2:
	      n_cells += patch->n_subdivisions *
			 patch->n_subdivisions;
	      n_nodes += (patch->n_subdivisions+1) *
			 (patch->n_subdivisions+1);
	      break;
	case 3:
	      n_cells += patch->n_subdivisions *
			 patch->n_subdivisions *
			 patch->n_subdivisions;
	      n_nodes += (patch->n_subdivisions+1) *
			 (patch->n_subdivisions+1) *
			 (patch->n_subdivisions+1);
	      break;
	default:
	      Assert (false, ExcNotImplemented());
      }

  

  
  
                                    // local variables only needed to write Tecplot
                                    // binary output files  
  const unsigned int vars_per_node  = (spacedim+n_data_sets),  
                     nodes_per_cell = GeometryInfo<dim>::vertices_per_cell;
  
#ifdef DEAL_II_ANON_NAMESPACE_BOGUS_WARNING
  TEC::TecplotMacros tm(n_nodes, vars_per_node, n_cells, nodes_per_cell);
#else
  TecplotMacros tm(n_nodes, vars_per_node, n_cells, nodes_per_cell);
#endif
  
  int is_double = 0,
      tec_debug = 0,
      cell_type;
  
  switch (dim)
    {
      case 2:
	    cell_type = 1;
	    break;
      case 3:
	    cell_type = 3;
	    break;
      default:
            Assert(false, ExcNotImplemented());	    
    }
  
  std::string tec_var_names;  
  switch (spacedim)
    {
      case 2:
	    tec_var_names  = "x y";
	    break;
      case 3:
	    tec_var_names  = "x y z";
	    break;
      default:
            Assert(false, ExcNotImplemented());
    }
  
  for (unsigned int data_set=0; data_set<n_data_sets; ++data_set)
    {
      tec_var_names += " ";
      tec_var_names += data_names[data_set];
    }
				    // in Tecplot FEBLOCK format the vertex
				    // coordinates and the data have an
				    // order that is a bit unpleasant
				    // (first all x coordinates, then
				    // all y coordinate, ...; first all
				    // data of variable 1, then
				    // variable 2, etc), so we have to
				    // copy the data vectors a bit around
				    //
				    // note that we copy vectors when
				    // looping over the patches since we
				    // have to write them one variable
				    // at a time and don't want to use
				    // more than one loop
				    //
				    // this copying of data vectors can
				    // be done while we already output
				    // the vertices, so do this on a
				    // separate thread and when wanting
				    // to write out the data, we wait
				    // for that thread to finish  
  Table<2,double> data_vectors (n_data_sets, n_nodes);

  void (*fun_ptr) (const std::vector<Patch<dim,spacedim> > &,
                   Table<2,double> &)
    = &DataOutBase::template write_gmv_reorder_data_vectors<dim,spacedim>;
  Threads::Thread<> reorder_thread = Threads::spawn (fun_ptr)(patches, data_vectors);

				    ///////////////////////////////
				    // first make up a list of used
				    // vertices along with their
				    // coordinates  
  for (unsigned int d=1; d<=spacedim; ++d)
     {       
       unsigned int entry=0;
       
       for (typename std::vector<Patch<dim,spacedim> >::const_iterator patch=patches.begin();
	    patch!=patches.end(); ++patch)
	 {
	   const unsigned int n_subdivisions = patch->n_subdivisions;
	  
	   switch (dim)
	     {
	       case 2:
	       {
		 for (unsigned int i=0; i<n_subdivisions+1; ++i)
		   for (unsigned int j=0; j<n_subdivisions+1; ++j)
		     {
		       const double x_frac = i * 1./n_subdivisions,
				    y_frac = j * 1./n_subdivisions;
		      
							// compute coordinates for
							// this patch point
		       tm.nd((d-1),entry) = static_cast<float>(
			 (((patch->vertices[1](d-1) * x_frac) +
			   (patch->vertices[0](d-1) * (1-x_frac))) * (1-y_frac) +
			  ((patch->vertices[2](d-1) * x_frac) +
			   (patch->vertices[3](d-1) * (1-x_frac))) * y_frac)
		       );
		       entry++;
		     }
		 break;
	       }
	      
	       case 3:
	       {
		 for (unsigned int i=0; i<n_subdivisions+1; ++i)
		   for (unsigned int j=0; j<n_subdivisions+1; ++j)
		     for (unsigned int k=0; k<n_subdivisions+1; ++k)
		       {
							  // note the broken
							  // design of hexahedra
							  // in deal.II, where
							  // first the z-component
							  // is counted up, before
							  // increasing the y-
							  // coordinate
			 const double x_frac = i * 1./n_subdivisions,
				      y_frac = k * 1./n_subdivisions,
				      z_frac = j * 1./n_subdivisions;
			
							  // compute coordinates for
							  // this patch point
			 tm.nd((d-1),entry) = static_cast<float>(
			   ((((patch->vertices[1](d-1) * x_frac) +
			      (patch->vertices[0](d-1) * (1-x_frac))) * (1-y_frac) +
			     ((patch->vertices[2](d-1) * x_frac) +
			      (patch->vertices[3](d-1) * (1-x_frac))) * y_frac)   * (1-z_frac) +
			    (((patch->vertices[5](d-1) * x_frac) +
			      (patch->vertices[4](d-1) * (1-x_frac))) * (1-y_frac) +
			     ((patch->vertices[6](d-1) * x_frac) +
			      (patch->vertices[7](d-1) * (1-x_frac))) * y_frac)   * z_frac)
			 );
			 entry++;
		       }
		 break;
	       }
	      
	       default:
		     Assert (false, ExcNotImplemented());
	     }
	 }
     }


				    ///////////////////////////////////////
				    // data output.
				    //
   reorder_thread.join ();

				    // then write data.
   for (unsigned int data_set=0; data_set<n_data_sets; ++data_set)
     for (unsigned int entry=0; entry<data_vectors[data_set].size(); entry++)
       tm.nd((spacedim+data_set),entry) = static_cast<float>(data_vectors[data_set][entry]);



  
				    /////////////////////////////////
				    // now for the cells. note that
				    // vertices are counted from 1 onwards
   unsigned int first_vertex_of_patch = 0;
   unsigned int elem=0;
      
   for (typename std::vector<Patch<dim,spacedim> >::const_iterator patch=patches.begin();
	patch!=patches.end(); ++patch)
     {
       const unsigned int n_subdivisions = patch->n_subdivisions;
      
					// write out the cells making
					// up this patch
       switch (dim)
	 {
	   case 2:
	   {
	     for (unsigned int i=0; i<n_subdivisions; ++i)
	       for (unsigned int j=0; j<n_subdivisions; ++j)
		 {

		   tm.cd(0,elem) = first_vertex_of_patch+i*(n_subdivisions+1)+j+1;
		   tm.cd(1,elem) = first_vertex_of_patch+(i+1)*(n_subdivisions+1)+j+1;
		   tm.cd(2,elem) = first_vertex_of_patch+(i+1)*(n_subdivisions+1)+j+1+1;
		   tm.cd(3,elem) = first_vertex_of_patch+i*(n_subdivisions+1)+j+1+1;
		   
		   elem++;
		 }
	     break;
	   }
	      
	   case 3:
	   {
	     for (unsigned int i=0; i<n_subdivisions; ++i)
	       for (unsigned int j=0; j<n_subdivisions; ++j)
		 for (unsigned int k=0; k<n_subdivisions; ++k)
		   {
						      // note: vertex indices start with 1!

		     
		     tm.cd(0,elem) = first_vertex_of_patch+(i*(n_subdivisions+1)+j      )*(n_subdivisions+1)+k  +1;
		     tm.cd(1,elem) = first_vertex_of_patch+((i+1)*(n_subdivisions+1)+j  )*(n_subdivisions+1)+k  +1;
		     tm.cd(2,elem) = first_vertex_of_patch+((i+1)*(n_subdivisions+1)+j+1)*(n_subdivisions+1)+k  +1;
		     tm.cd(3,elem) = first_vertex_of_patch+(i*(n_subdivisions+1)+j+1    )*(n_subdivisions+1)+k  +1;
		     tm.cd(4,elem) = first_vertex_of_patch+(i*(n_subdivisions+1)+j      )*(n_subdivisions+1)+k+1+1;
		     tm.cd(5,elem) = first_vertex_of_patch+((i+1)*(n_subdivisions+1)+j  )*(n_subdivisions+1)+k+1+1;
		     tm.cd(6,elem) = first_vertex_of_patch+((i+1)*(n_subdivisions+1)+j+1)*(n_subdivisions+1)+k+1+1;
		     tm.cd(7,elem) = first_vertex_of_patch+(i*(n_subdivisions+1)+j+1    )*(n_subdivisions+1)+k+1+1;
		     
		     elem++;
		   }
	     break;
	   }

	   default:
		 Assert (false, ExcNotImplemented());
	 }


					// finally update the number
					// of the first vertex of this patch
       switch (dim)
	 {
	   case 2:
		 first_vertex_of_patch += (n_subdivisions+1) *
					  (n_subdivisions+1);
		 break;
	   case 3:
		 first_vertex_of_patch += (n_subdivisions+1) *
					  (n_subdivisions+1) *
					  (n_subdivisions+1);
		 break;
	   default:
		 Assert (false, ExcNotImplemented());
	 }      
     }


   {     
     int ierr      = 0,
         num_nodes = static_cast<int>(n_nodes),
         num_cells = static_cast<int>(n_cells);

     char dot[2] = {'.', 0};
				      // Unfortunately, TECINI takes a
				      // char *, but c_str() gives a
				      // const char *.  As we don't do
				      // anything else with
				      // tec_var_names following
				      // const_cast is ok
     char *var_names=const_cast<char *> (tec_var_names.c_str());
     ierr = TECINI (NULL,
		    var_names,
		    file_name,
		    dot,
		    &tec_debug,
		    &is_double);
     
     Assert (ierr == 0, ExcErrorOpeningTecplotFile(file_name));

     char FEBLOCK[] = {'F','E','B','L','O','C','K',0};
     ierr = TECZNE (NULL,
		    &num_nodes,
		    &num_cells,
		    &cell_type,
		    FEBLOCK,
		    NULL);
     
     Assert (ierr == 0, ExcTecplotAPIError());
     
     int total = (vars_per_node*num_nodes);

     ierr = TECDAT (&total,
		    &tm.nodalData[0],
		    &is_double);
     
     Assert (ierr == 0, ExcTecplotAPIError());
     
     ierr = TECNOD (&tm.connData[0]);
     
     Assert (ierr == 0, ExcTecplotAPIError());
     
     ierr = TECEND ();
     
     Assert (ierr == 0, ExcTecplotAPIError());     
   }
#endif
}



template <int dim, int spacedim>
void DataOutBase::write_vtk (const std::vector<Patch<dim,spacedim> > &patches,
			     const std::vector<std::string>          &data_names,
			     const VtkFlags                          &/*flags*/,
			     std::ostream                            &out) 
{
  AssertThrow (out, ExcIO());

  Assert (patches.size() > 0, ExcNoPatches());
 
  const unsigned int n_data_sets = data_names.size();
				   // check against # of data sets in
				   // first patch. checks against all
				   // other patches are made in
				   // write_gmv_reorder_data_vectors
  Assert (n_data_sets == patches[0].data.n_rows(),
	  ExcDimensionMismatch (patches[0].data.n_rows(), n_data_sets));
  
  
				   ///////////////////////
				   // preamble
  if (true)
    {
      std::time_t  time1= std::time (0);
      std::tm     *time = std::localtime(&time1);
      out << "# vtk DataFile Version 3.0"
	  << std::endl
	  << "This file was generated by the deal.II library on "
	  << time->tm_year+1900 << "/"
	  << time->tm_mon+1 << "/"
	  << time->tm_mday << " at "
	  << time->tm_hour << ":"
	  << std::setw(2) << time->tm_min << ":"
	  << std::setw(2) << time->tm_sec
	  << std::endl
	  << "ASCII"
	  << std::endl
	  << "DATASET UNSTRUCTURED_GRID"
	  << std::endl;
    }
  

				   // first count the number of cells
				   // and cells for later use
  unsigned int n_cells = 0,
	       n_nodes = 0;
  for (typename std::vector<Patch<dim,spacedim> >::const_iterator patch=patches.begin();
       patch!=patches.end(); ++patch)
    switch (dim)
      {
	case 1:
	      n_cells += patch->n_subdivisions;
	      n_nodes += patch->n_subdivisions+1;
	      break;
	case 2:
	      n_cells += patch->n_subdivisions *
			 patch->n_subdivisions;
	      n_nodes += (patch->n_subdivisions+1) *
			 (patch->n_subdivisions+1);
	      break;
	case 3:
	      n_cells += patch->n_subdivisions *
			 patch->n_subdivisions *
			 patch->n_subdivisions;
	      n_nodes += (patch->n_subdivisions+1) *
			 (patch->n_subdivisions+1) *
			 (patch->n_subdivisions+1);
	      break;
	default:
	      Assert (false, ExcNotImplemented());
      }


				   // in gmv format the vertex
				   // coordinates and the data have an
				   // order that is a bit unpleasant
				   // (first all x coordinates, then
				   // all y coordinate, ...; first all
				   // data of variable 1, then
				   // variable 2, etc), so we have to
				   // copy the data vectors a bit around
				   //
				   // note that we copy vectors when
				   // looping over the patches since we
				   // have to write them one variable
				   // at a time and don't want to use
				   // more than one loop
				   //
				   // this copying of data vectors can
				   // be done while we already output
				   // the vertices, so do this on a
				   // separate thread and when wanting
				   // to write out the data, we wait
				   // for that thread to finish
  Table<2,double> data_vectors (n_data_sets, n_nodes);

  void (*fun_ptr) (const std::vector<Patch<dim,spacedim> > &,
		   Table<2,double> &)
    = &DataOutBase::template write_gmv_reorder_data_vectors<dim,spacedim>;
  Threads::Thread<> reorder_thread = Threads::spawn (fun_ptr)(patches, data_vectors);

				   ///////////////////////////////
				   // first make up a list of used
				   // vertices along with their
				   // coordinates
				   //
				   // note that we have to print
				   // d=1..3 dimensions
  out << "POINTS " << n_nodes << " double" << std::endl;
  for (typename std::vector<Patch<dim,spacedim> >::const_iterator patch=patches.begin();
       patch!=patches.end(); ++patch)
    {
      const unsigned int n_subdivisions = patch->n_subdivisions;
      
      switch (dim)
	{
	  case 1:
	  {
	    for (unsigned int i=0; i<n_subdivisions+1; ++i)
	      out << ((patch->vertices[1](0) * i / n_subdivisions) +
		      (patch->vertices[0](0) * (n_subdivisions-i) / n_subdivisions))
		  << " 0 0\n";
	    break;
	  }
		   
	  case 2:
	  {
	    for (unsigned int i=0; i<n_subdivisions+1; ++i)
	      for (unsigned int j=0; j<n_subdivisions+1; ++j)
		{
		  const double x_frac = i * 1./n_subdivisions,
			       y_frac = j * 1./n_subdivisions;
		      
		  out << (((patch->vertices[1] * x_frac) +
			   (patch->vertices[0] * (1-x_frac))) * (1-y_frac) +
			  ((patch->vertices[2] * x_frac) +
			   (patch->vertices[3] * (1-x_frac))) * y_frac)
		      << " 0\n";
		}
	    break;
	  }
	       
	  case 3:
	  {
	    for (unsigned int i=0; i<n_subdivisions+1; ++i)
	      for (unsigned int j=0; j<n_subdivisions+1; ++j)
		for (unsigned int k=0; k<n_subdivisions+1; ++k)
		  {
						     // note the broken
						     // design of hexahedra
						     // in deal.II, where
						     // first the z-component
						     // is counted up, before
						     // increasing the y-
						     // coordinate
		    const double x_frac = i * 1./n_subdivisions,
				 y_frac = k * 1./n_subdivisions,
				 z_frac = j * 1./n_subdivisions;
			
						     // compute coordinates for
						     // this patch point
		    out << ((((patch->vertices[1] * x_frac) +
			      (patch->vertices[0] * (1-x_frac))) * (1-y_frac) +
			     ((patch->vertices[2] * x_frac) +
			      (patch->vertices[3] * (1-x_frac))) * y_frac)   * (1-z_frac) +
			    (((patch->vertices[5] * x_frac) +
			      (patch->vertices[4] * (1-x_frac))) * (1-y_frac) +
			     ((patch->vertices[6] * x_frac) +
			      (patch->vertices[7] * (1-x_frac))) * y_frac)   * z_frac)
			<< std::endl;
		  }
		
	    break;
	  }
	       
	  default:
		Assert (false, ExcNotImplemented());
	}
    }

				   /////////////////////////////////
				   // now for the cells
  if (true)
    {
      out << "CELLS " << n_cells << ' '
	  << n_cells*(GeometryInfo<dim>::vertices_per_cell+1)
	  << std::endl;


      unsigned int first_vertex_of_patch = 0;
      
      for (typename std::vector<Patch<dim,spacedim> >::const_iterator patch=patches.begin();
	   patch!=patches.end(); ++patch)
	{
	  const unsigned int n_subdivisions = patch->n_subdivisions;

					   // write out the cells making
					   // up this patch
	  switch (dim)
	    {
	      case 1:
	      {
		for (unsigned int i=0; i<n_subdivisions; ++i)
		  out << "2 "
		      << first_vertex_of_patch+i << ' '
		      << first_vertex_of_patch+i+1 << std::endl;
		break;
	      }
	       
	      case 2:
	      {
		for (unsigned int i=0; i<n_subdivisions; ++i)
		  for (unsigned int j=0; j<n_subdivisions; ++j)
		    out << "4 "
			<< first_vertex_of_patch+i*(n_subdivisions+1)+j << ' '
			<< first_vertex_of_patch+(i+1)*(n_subdivisions+1)+j << ' '
			<< first_vertex_of_patch+(i+1)*(n_subdivisions+1)+j+1 << ' '
			<< first_vertex_of_patch+i*(n_subdivisions+1)+j+1
			<< std::endl;
		break;
	      }
	       
	      case 3:
	      {
		for (unsigned int i=0; i<n_subdivisions; ++i)
		  for (unsigned int j=0; j<n_subdivisions; ++j)
		    for (unsigned int k=0; k<n_subdivisions; ++k)
		      {
			out << "8 "
							   // note: vertex indices start with 1!
			    << first_vertex_of_patch+(i*(n_subdivisions+1)+j      )*(n_subdivisions+1)+k   << ' '
			    << first_vertex_of_patch+((i+1)*(n_subdivisions+1)+j  )*(n_subdivisions+1)+k   << ' '
			    << first_vertex_of_patch+((i+1)*(n_subdivisions+1)+j+1)*(n_subdivisions+1)+k   << ' '
			    << first_vertex_of_patch+(i*(n_subdivisions+1)+j+1    )*(n_subdivisions+1)+k   << ' '
			    << first_vertex_of_patch+(i*(n_subdivisions+1)+j      )*(n_subdivisions+1)+k+1 << ' '
			    << first_vertex_of_patch+((i+1)*(n_subdivisions+1)+j  )*(n_subdivisions+1)+k+1 << ' '
			    << first_vertex_of_patch+((i+1)*(n_subdivisions+1)+j+1)*(n_subdivisions+1)+k+1 << ' '
			    << first_vertex_of_patch+(i*(n_subdivisions+1)+j+1    )*(n_subdivisions+1)+k+1 << ' '
			    << std::endl;
		      }
		break;
	      }

	      default:
		    Assert (false, ExcNotImplemented());
	    }


					   // finally update the number
					   // of the first vertex of this patch
	  switch (dim)
	    {
	      case 1:
		    first_vertex_of_patch += n_subdivisions+1;
		    break;
	      case 2:
		    first_vertex_of_patch += (n_subdivisions+1) *
					     (n_subdivisions+1);
		    break;
	      case 3:
		    first_vertex_of_patch += (n_subdivisions+1) *
					     (n_subdivisions+1) *
					     (n_subdivisions+1);
		    break;
	      default:
		    Assert (false, ExcNotImplemented());
	    }
	}

				       // next output the types of the
				       // cells. since all cells are
				       // the same, this is simple
      out << "CELL_TYPES " << n_cells << std::endl;
      for (unsigned int i=0; i<n_cells; ++i)
	switch (dim)
	  {
	    case 1:
		  out << "3\n";    // represents VTK_LINE
		  break;
	    case 2:
		  out << "9\n";    // represents VTK_QUAD
		  break;
	    case 3:
		  out << "12\n";    // represents VTK_HEXAHEDRON
		  break;
	    default:
		  Assert (false, ExcNotImplemented());
	  }
    }

				   ///////////////////////////////////////
				   // data output.

				   // now write the data vectors to
				   // @p{out} first make sure that all
				   // data is in place
  reorder_thread.join ();

				   // then write data.  the
				   // 'POINTD_DATA' means: node data
				   // (as opposed to cell data, which
				   // we do not support explicitly
				   // here). all following data sets
				   // are point data
  out << "POINT_DATA " << n_nodes
      << std::endl;
  for (unsigned int data_set=0; data_set<n_data_sets; ++data_set)
    {
      out << "SCALARS "
	  << data_names[data_set]
	  << " double 1"
	  << std::endl
	  << "LOOKUP_TABLE default"
	  << std::endl;
      std::copy (data_vectors[data_set].begin(),
		 data_vectors[data_set].end(),
		 std::ostream_iterator<double>(out, " "));
      out << std::endl;
    }
  
				   // assert the stream is still ok
  AssertThrow (out, ExcIO());
}



template <int dim, int spacedim>
void
DataOutBase::
write_deal_II_intermediate (const std::vector<Patch<dim,spacedim> > &patches,
			    const std::vector<std::string>          &data_names,
			    const Deal_II_IntermediateFlags         &/*flags*/,
			    std::ostream                            &out) 
{
                                   // first write tokens indicating the
                                   // template parameters. we need this in
                                   // here because we may want to read in data
                                   // again even if we don't know in advance
                                   // the template parameters, see step-19
  out << dim << ' ' << spacedim << std::endl;

                                   // then write a header
  out << "[deal.II intermediate format graphics data]" << std::endl
      << "[written by " << PACKAGE_STRING << "]" << std::endl;

  out << data_names.size() << std::endl;
  for (unsigned int i=0; i<data_names.size(); ++i)
    out << data_names[i] << std::endl;
  
  out << patches.size() << std::endl;
  for (unsigned int i=0; i<patches.size(); ++i)
    out << patches[i] << std::endl;

  out << std::endl;
} 



std::pair<unsigned int, unsigned int>
DataOutBase::
determine_intermediate_format_dimensions (std::istream &input)
{
  Assert (input, ExcIO());
          
  int dim, spacedim;
  input >> dim >> spacedim;

  return std::make_pair (dim, spacedim);
}



template <int dim, int spacedim>
void
DataOutBase::write_gmv_reorder_data_vectors (const std::vector<Patch<dim,spacedim> > &patches,
					     Table<2,double>                         &data_vectors)
{
				   // unlike in the main function, we
				   // don't have here the data_names
				   // field, so we initialize it with
				   // the number of data sets in the
				   // first patch. the equivalence of
				   // these two definitions is checked
				   // in the main function.
  const unsigned int n_data_sets = patches[0].data.n_rows();

  Assert (data_vectors.size()[0] == n_data_sets,
	  ExcInternalError());
  
				   // loop over all patches
  unsigned int next_value = 0;
  for (typename std::vector<Patch<dim,spacedim> >::const_iterator patch=patches.begin();
       patch != patches.end(); ++patch)
    {
      const unsigned int n_subdivisions = patch->n_subdivisions;
	  
      Assert (patch->data.n_rows() == n_data_sets,
	      ExcDimensionMismatch (patch->data.n_rows(), n_data_sets));
      Assert (patch->data.n_cols() == (dim==1 ?
				  n_subdivisions+1 :
				  (dim==2 ?
				   (n_subdivisions+1)*(n_subdivisions+1) :
				   (dim==3 ?
				    (n_subdivisions+1)*(n_subdivisions+1)*(n_subdivisions+1) :
				    0))),
	      ExcInvalidDatasetSize (patch->data.n_cols(), n_subdivisions+1));
	  
      switch (dim)
	{
	  case 1:
	  {      
	    for (unsigned int i=0; i<n_subdivisions+1; ++i, ++next_value) 
	      for (unsigned int data_set=0; data_set<n_data_sets; ++data_set)
		data_vectors[data_set][next_value] = patch->data(data_set,i);
		
	    break;
	  }
		     
	  case 2:
	  {
	    for (unsigned int i=0; i<n_subdivisions+1; ++i)
	      for (unsigned int j=0; j<n_subdivisions+1; ++j)
		{
		  for (unsigned int data_set=0; data_set<n_data_sets; ++data_set)
		    data_vectors[data_set][next_value]
		      = patch->data(data_set,i*(n_subdivisions+1) + j);
		  ++next_value;
		}
		
	    break;
	  }
	       
	  case 3:
	  {
	    for (unsigned int i=0; i<n_subdivisions+1; ++i)
	      for (unsigned int j=0; j<n_subdivisions+1; ++j)
		for (unsigned int k=0; k<n_subdivisions+1; ++k)
		  {
		    for (unsigned int data_set=0; data_set<n_data_sets; ++data_set)
		      data_vectors[data_set][next_value]
			= patch->data(data_set,
				      (i*(n_subdivisions+1)+j)*(n_subdivisions+1)+k);
		    ++next_value;
		  }

	    break;
	  }
	       
	  default:
		Assert (false, ExcNotImplemented());
	}
    }

  for (unsigned int data_set=0; data_set<n_data_sets; ++data_set)
    Assert (data_vectors[data_set].size() == next_value,
	    ExcInternalError());
}



/* --------------------------- class DataOutInterface ---------------------- */


template <int dim, int spacedim>
DataOutInterface<dim,spacedim>::~DataOutInterface ()
{}




template <int dim, int spacedim>
void DataOutInterface<dim,spacedim>::write_dx (std::ostream &out) const 
{
  DataOutBase::write_dx (get_patches(), get_dataset_names(),
			 dx_flags, out);
}



template <int dim, int spacedim>
void DataOutInterface<dim,spacedim>::write_ucd (std::ostream &out) const 
{
  DataOutBase::write_ucd (get_patches(), get_dataset_names(),
			  ucd_flags, out);
}



template <int dim, int spacedim>
void DataOutInterface<dim,spacedim>::write_gnuplot (std::ostream &out) const 
{
  DataOutBase::write_gnuplot (get_patches(), get_dataset_names(),
			      gnuplot_flags, out);
}



template <int dim, int spacedim>
void DataOutInterface<dim,spacedim>::write_povray (std::ostream &out) const 
{
  DataOutBase::write_povray (get_patches(), get_dataset_names(),
			     povray_flags, out);
}



template <int dim, int spacedim>
void DataOutInterface<dim,spacedim>::write_eps (std::ostream &out) const 
{
  DataOutBase::write_eps (get_patches(), get_dataset_names(),
			  eps_flags, out);
}



template <int dim, int spacedim>
void DataOutInterface<dim,spacedim>::write_gmv (std::ostream &out) const 
{
  DataOutBase::write_gmv (get_patches(), get_dataset_names(),
			  gmv_flags, out);
}



template <int dim, int spacedim>
void DataOutInterface<dim,spacedim>::write_tecplot (std::ostream &out) const 
{
  DataOutBase::write_tecplot (get_patches(), get_dataset_names(),
			      tecplot_flags, out);
}



template <int dim, int spacedim>
void DataOutInterface<dim,spacedim>::write_tecplot_binary (std::ostream &out) const 
{
  DataOutBase::write_tecplot_binary (get_patches(), get_dataset_names(),
				     tecplot_flags, out);
}



template <int dim, int spacedim>
void DataOutInterface<dim,spacedim>::write_vtk (std::ostream &out) const 
{
  DataOutBase::write_vtk (get_patches(), get_dataset_names(),
			  vtk_flags, out);
}



template <int dim, int spacedim>
void DataOutInterface<dim,spacedim>::
write_deal_II_intermediate (std::ostream &out) const 
{
  DataOutBase::write_deal_II_intermediate (get_patches(), get_dataset_names(),
					   deal_II_intermediate_flags, out);
}



template <int dim, int spacedim>
void
DataOutInterface<dim,spacedim>::write (std::ostream &out,
				       const OutputFormat output_format_) const
{
  OutputFormat output_format = output_format_;
  if (output_format == default_format)
    output_format = default_fmt;
  
  switch (output_format) 
    {
      case dx:
	    write_dx (out);
	    break;
	    
      case ucd:
	    write_ucd (out);
	    break;
	    
      case gnuplot:
	    write_gnuplot (out);
	    break;
	    
      case povray:
	    write_povray (out);
	    break;
	    
      case eps:
	    write_eps(out);
	    break;
	    
      case gmv:
	    write_gmv (out);
	    break;
	    
      case tecplot:
	    write_tecplot (out);
	    break;
	    
      case tecplot_binary:
	    write_tecplot_binary (out);
	    break;
	    
      case vtk:
	    write_vtk (out);
	    break;

      case deal_II_intermediate:
	    write_deal_II_intermediate (out);
	    break;
	    
      default:
	    Assert (false, ExcNotImplemented());
    }
}



template <int dim, int spacedim>
void
DataOutInterface<dim,spacedim>::set_default_format(const OutputFormat fmt)
{
  Assert (fmt != default_format, ExcNotImplemented());
  default_fmt = fmt;
}



template <int dim, int spacedim>
void
DataOutInterface<dim,spacedim>::set_flags (const DXFlags &flags) 
{
  dx_flags = flags;
}



template <int dim, int spacedim>
void
DataOutInterface<dim,spacedim>::set_flags (const UcdFlags &flags) 
{
  ucd_flags = flags;
}



template <int dim, int spacedim>
void
DataOutInterface<dim,spacedim>::set_flags (const GnuplotFlags &flags) 
{
  gnuplot_flags = flags;
}



template <int dim, int spacedim>
void
DataOutInterface<dim,spacedim>::set_flags (const PovrayFlags &flags) 
{
  povray_flags = flags;
}



template <int dim, int spacedim>
void
DataOutInterface<dim,spacedim>::set_flags (const EpsFlags &flags) 
{
  eps_flags = flags;
}



template <int dim, int spacedim>
void
DataOutInterface<dim,spacedim>::set_flags (const GmvFlags &flags) 
{
  gmv_flags = flags;
}



template <int dim, int spacedim>
void
DataOutInterface<dim,spacedim>::set_flags (const TecplotFlags &flags) 
{
  tecplot_flags = flags;
}



template <int dim, int spacedim>
void
DataOutInterface<dim,spacedim>::set_flags (const VtkFlags &flags) 
{
  vtk_flags = flags;
}



template <int dim, int spacedim>
void
DataOutInterface<dim,spacedim>::set_flags (const Deal_II_IntermediateFlags &flags) 
{
  deal_II_intermediate_flags = flags;
}



template <int dim, int spacedim>
std::string
DataOutInterface<dim,spacedim>::
default_suffix (const OutputFormat output_format) const
{
  if (output_format == default_format)
    return DataOutBase::default_suffix (default_fmt);
  else
    return DataOutBase::default_suffix (output_format);
}



template <int dim, int spacedim>
void
DataOutInterface<dim,spacedim>::declare_parameters (ParameterHandler &prm) 
{
  prm.declare_entry ("Output format", "gnuplot",
		     Patterns::Selection (get_output_format_names ()),
                     "A name for the output format to be used");

  prm.enter_subsection ("DX output parameters");
  DXFlags::declare_parameters (prm);
  prm.leave_subsection ();
  
  prm.enter_subsection ("UCD output parameters");
  UcdFlags::declare_parameters (prm);
  prm.leave_subsection ();
  
  prm.enter_subsection ("Gnuplot output parameters");
  GnuplotFlags::declare_parameters (prm);
  prm.leave_subsection ();

  prm.enter_subsection ("Povray output parameters");
  PovrayFlags::declare_parameters (prm);
  prm.leave_subsection ();

  prm.enter_subsection ("Eps output parameters");
  EpsFlags::declare_parameters (prm);
  prm.leave_subsection ();

  prm.enter_subsection ("Gmv output parameters");
  GmvFlags::declare_parameters (prm);
  prm.leave_subsection ();

  prm.enter_subsection ("Tecplot output parameters");
  TecplotFlags::declare_parameters (prm);
  prm.leave_subsection ();

  prm.enter_subsection ("Vtk output parameters");
  VtkFlags::declare_parameters (prm);
  prm.leave_subsection ();


  prm.enter_subsection ("deal.II intermediate output parameters");
  Deal_II_IntermediateFlags::declare_parameters (prm);
  prm.leave_subsection ();
}



template <int dim, int spacedim>
void
DataOutInterface<dim,spacedim>::parse_parameters (ParameterHandler &prm) 
{
  const std::string& output_name = prm.get ("Output format");
  default_fmt = parse_output_format (output_name);

  prm.enter_subsection ("DX output parameters");
  dx_flags.parse_parameters (prm);
  prm.leave_subsection ();
  
  prm.enter_subsection ("UCD output parameters");
  ucd_flags.parse_parameters (prm);
  prm.leave_subsection ();
  
  prm.enter_subsection ("Gnuplot output parameters");
  gnuplot_flags.parse_parameters (prm);
  prm.leave_subsection ();

  prm.enter_subsection ("Povray output parameters");
  povray_flags.parse_parameters (prm);
  prm.leave_subsection ();

  prm.enter_subsection ("Eps output parameters");
  eps_flags.parse_parameters (prm);
  prm.leave_subsection ();

  prm.enter_subsection ("Gmv output parameters");
  gmv_flags.parse_parameters (prm);
  prm.leave_subsection ();

  prm.enter_subsection ("Tecplot output parameters");
  tecplot_flags.parse_parameters (prm);
  prm.leave_subsection ();

  prm.enter_subsection ("Vtk output parameters");
  vtk_flags.parse_parameters (prm);
  prm.leave_subsection ();

  prm.enter_subsection ("deal.II intermediate output parameters");
  deal_II_intermediate_flags.parse_parameters (prm);
  prm.leave_subsection ();
}



template <int dim, int spacedim>
unsigned int 
DataOutInterface<dim,spacedim>::memory_consumption () const
{
  return (sizeof (default_fmt) +
	  MemoryConsumption::memory_consumption (dx_flags) +
	  MemoryConsumption::memory_consumption (ucd_flags) +
	  MemoryConsumption::memory_consumption (gnuplot_flags) +
	  MemoryConsumption::memory_consumption (povray_flags) +
	  MemoryConsumption::memory_consumption (eps_flags) +
	  MemoryConsumption::memory_consumption (gmv_flags) +
	  MemoryConsumption::memory_consumption (tecplot_flags) +
	  MemoryConsumption::memory_consumption (vtk_flags) +
	  MemoryConsumption::memory_consumption (deal_II_intermediate_flags));
}




template <int dim, int spacedim>
void
DataOutReader<dim,spacedim>::read (std::istream &in) 
{
  Assert (in, ExcIO());

				   // first empty previous content
  {
    std::vector<typename ::DataOutBase::Patch<dim,spacedim> >
      tmp;
    tmp.swap (patches);
  }
  {
    std::vector<std::string> tmp;
    tmp.swap (dataset_names);
  }

				   // then check that we have the correct
				   // header of this file. both the first and
				   // second real lines have to match, as well
				   // as the dimension information written
				   // before that
  {
    std::pair<unsigned int, unsigned int>
      dimension_info
      = ::DataOutBase::determine_intermediate_format_dimensions (in);
    AssertThrow ((dimension_info.first  == dim) &&
                 (dimension_info.second == spacedim),
                 ExcIncompatibleDimensions (dimension_info.first, dim,
                                            dimension_info.second, spacedim));

                                     // read to the end of the line
    std::string tmp;
    getline (in, tmp);
  }
  
  {
    std::string header;
    getline (in, header);

#ifdef HAVE_STD_STRINGSTREAM
    std::ostringstream s;
#else
    std::ostrstream s;
#endif

    s << "[deal.II intermediate format graphics data]";

#ifndef HAVE_STD_STRINGSTREAM
    s << std::ends;
#endif
    
    Assert (header == s.str(), ExcUnexpectedInput(s.str(),header));
  }
  {
    std::string header;
    getline (in, header);

#ifdef HAVE_STD_STRINGSTREAM
    std::ostringstream s;
#else
    std::ostrstream s;
#endif

    s << "[written by " << PACKAGE_STRING << "]";

#ifndef HAVE_STD_STRINGSTREAM
    s << std::ends;
#endif
    
    Assert (header == s.str(), ExcUnexpectedInput(s.str(),header));
  }  
  
				   // then read the rest of the data
  unsigned int n_datasets;
  in >> n_datasets;
  dataset_names.resize (n_datasets);
  for (unsigned int i=0; i<n_datasets; ++i)
    in >> dataset_names[i];

  unsigned int n_patches;
  in >> n_patches;
  patches.resize (n_patches);
  for (unsigned int i=0; i<n_patches; ++i)
    in >> patches[i];
  
  Assert (in, ExcIO());  
}



template <int dim, int spacedim>
void
DataOutReader<dim,spacedim>::
merge (const DataOutReader<dim,spacedim> &source) 
{
  typedef typename ::DataOutBase::Patch<dim,spacedim> Patch;
  
  const std::vector<Patch> source_patches = source.get_patches ();
  Assert (patches.size () != 0,        ExcNoPatches ());
  Assert (source_patches.size () != 0, ExcNoPatches ());
                                   // check equality of component
                                   // names
  Assert (get_dataset_names() == source.get_dataset_names(),
          ExcIncompatibleDatasetNames());
                                   // make sure patches are compatible
  Assert (patches[0].n_subdivisions == source_patches[0].n_subdivisions,
          ExcIncompatiblePatchLists());
  Assert (patches[0].data.n_rows() == source_patches[0].data.n_rows(),
          ExcIncompatiblePatchLists());
  Assert (patches[0].data.n_cols() == source_patches[0].data.n_cols(),
          ExcIncompatiblePatchLists());

                                   // merge patches. store old number
                                   // of elements, since we need to
                                   // adjust patch numbers, etc
                                   // afterwards
  const unsigned int old_n_patches = patches.size();
  patches.insert (patches.end(),
                  source_patches.begin(),
                  source_patches.end());

                                   // adjust patch numbers
  for (unsigned int i=old_n_patches; i<patches.size(); ++i)
    patches[i].patch_index += old_n_patches;
  
                                   // adjust patch neighbors
  for (unsigned int i=old_n_patches; i<patches.size(); ++i)
    for (unsigned int n=0; n<GeometryInfo<dim>::faces_per_cell; ++n)
      if (patches[i].neighbors[n] != Patch::no_neighbor)
        patches[i].neighbors[n] += old_n_patches;
}



template <int dim, int spacedim>
const std::vector<typename ::DataOutBase::Patch<dim,spacedim> > &
DataOutReader<dim,spacedim>::get_patches () const
{
  return patches;
}



template <int dim, int spacedim>
std::vector<std::string>
DataOutReader<dim,spacedim>::get_dataset_names () const
{
  return dataset_names;
}




template <int dim, int spacedim>
std::ostream &
operator << (std::ostream                           &out,
	     const DataOutBase::Patch<dim,spacedim> &patch)
{
				   // write a header line
  out << "[deal.II intermediate Patch<" << dim << ',' << spacedim << ">]"
      << std::endl;

				   // then write all the data that is
				   // in this patch
  for (unsigned int i=0; i<GeometryInfo<dim>::vertices_per_cell; ++i)
    out << patch.vertices[i] << " ";
  out << std::endl;

  for (unsigned int i=0; i<GeometryInfo<dim>::faces_per_cell; ++i)
    out << patch.neighbors[i] << " ";
  out << std::endl;

  out << patch.patch_index << ' ' << patch.n_subdivisions
      << std::endl;

  out << patch.data.n_rows() << ' ' << patch.data.n_cols() << std::endl;
  for (unsigned int i=0; i<patch.data.n_rows(); ++i)
    for (unsigned int j=0; j<patch.data.n_cols(); ++j)
      out << patch.data[i][j] << ' ';
  out << std::endl;
  out << std::endl;

  return out;
}



template <int dim, int spacedim>
std::istream &
operator >> (std::istream                     &in,
	     DataOutBase::Patch<dim,spacedim> &patch)
{
  Assert (in, ExcIO());

				   // read a header line and compare
				   // it to what we usually
				   // write. skip all lines that
				   // contain only blanks at the start
  {
    std::string header;
    do
      {
	getline (in, header);
	while ((header.size() != 0) &&
	       (header[header.size()-1] == ' '))
	  header.erase(header.size()-1);
      }
    while ((header == "") && in);

#ifdef HAVE_STD_STRINGSTREAM
    std::ostringstream s;
#else
    std::ostrstream s;
#endif

    s << "[deal.II intermediate Patch<" << dim << ',' << spacedim << ">]";

#ifndef HAVE_STD_STRINGSTREAM
    s << std::ends;
#endif
    
    Assert (header == s.str(), ExcUnexpectedInput(s.str(),header));
  }
  

				   // then read all the data that is
				   // in this patch
  for (unsigned int i=0; i<GeometryInfo<dim>::vertices_per_cell; ++i)
    in >> patch.vertices[i];

  for (unsigned int i=0; i<GeometryInfo<dim>::faces_per_cell; ++i)
    in >> patch.neighbors[i];

  in >> patch.patch_index >> patch.n_subdivisions;

  unsigned int n_rows, n_cols;
  in >> n_rows >> n_cols;
  patch.data.reinit (n_rows, n_cols);
  for (unsigned int i=0; i<patch.data.n_rows(); ++i)
    for (unsigned int j=0; j<patch.data.n_cols(); ++j)
      in >> patch.data[i][j];

  Assert (in, ExcIO());
  
  return in;
}




// explicit instantiations
#define INSTANTIATE(dim,spacedim) \
  template class DataOutInterface<dim,spacedim>;                \
  template class DataOutReader<dim,spacedim>;                   \
  template class DataOutBase::Patch<dim,spacedim>;              \
  template                                                      \
  std::ostream &                                                \
  operator << (std::ostream                           &out,     \
	       const DataOutBase::Patch<dim,spacedim> &patch);  \
  template                                                      \
  std::istream &                                                \
  operator >> (std::istream                     &in,            \
	       DataOutBase::Patch<dim,spacedim> &patch)

INSTANTIATE(1,1);
INSTANTIATE(2,2);
INSTANTIATE(3,3);
INSTANTIATE(4,4);
INSTANTIATE(1,2);
INSTANTIATE(2,3);
INSTANTIATE(3,4);


