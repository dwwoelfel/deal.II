//----------------------------  grid_out.cc  ---------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 1998, 1999, 2000, 2001, 2002 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  grid_out.cc  ---------------------------


#include <base/point.h>
#include <base/quadrature.h>
#include <grid/grid_out.h>
#include <grid/tria.h>
#include <grid/tria_accessor.h>
#include <grid/tria_iterator.h>
#include <fe/mapping.h>

#include <iomanip>
#include <algorithm>
#include <list>
#include <ctime>
#include <cmath>


#if deal_II_dimension == 1


template <int dim>
void GridOut::write_dx (const Triangulation<dim> &,
			std::ostream             &)
{
  Assert (false, ExcNotImplemented());
}

#else


template <int dim>
void GridOut::write_dx (const Triangulation<dim> &tria,
			std::ostream             &out) 
{
//TODO:[GK] allow for boundary faces only  
  Assert(dx_flags.write_all_faces, ExcNotImplemented());
  AssertThrow (out, ExcIO());
				   // Copied and adapted from write_ucd
  const std::vector<Point<dim> > &vertices    = tria.get_vertices();
  const std::vector<bool>        &vertex_used = tria.get_used_vertices();
  
  const unsigned int n_vertices = tria.n_used_vertices();

  typename Triangulation<dim>::active_cell_iterator       cell;
  const typename Triangulation<dim>::active_cell_iterator endc=tria.end();

  
				   // write the vertices
  out << "object \"vertices\" class array type float rank 1 shape " << dim
      << " items " << n_vertices << " data follows"
      << std::endl;
  
  for (unsigned int i=0; i<vertices.size(); ++i)
    if (vertex_used[i])
      {
	out << '\t' << vertices[i] << std::endl;
      };
  
				   // write cells or faces
  const bool write_cells = dx_flags.write_cells;
  const bool write_faces = (dim>1) ? dx_flags.write_faces : false;
  
  const unsigned int n_cells = tria.n_active_cells();
  const unsigned int n_faces = tria.n_active_cells()
			       * GeometryInfo<dim>::faces_per_cell;

  const unsigned int n_vertices_per_cell = GeometryInfo<dim>::vertices_per_cell;
  const unsigned int n_vertices_per_face = GeometryInfo<dim>::vertices_per_face;
  
  if (write_cells)
    {
      out << "object \"cells\" class array type int rank 1 shape "
	  << n_vertices_per_cell
	  << " items " << n_cells << " data follows" << std::endl;
      
      for (cell = tria.begin_active(); cell != endc; ++cell)
	{
	  for (unsigned int v=0; v<GeometryInfo<dim>::vertices_per_cell; ++v)
	    out << '\t' << cell->vertex_index(GeometryInfo<dim>::dx_to_deal[v]);
	  out << std::endl;
	}
      out << "attribute \"element type\" string \"";
      if (dim==1) out << "lines";
      if (dim==2) out << "quads";
      if (dim==3) out << "cubes";
      out << "\"" << std::endl
	  << "attribute \"ref\" string \"positions\"" << std::endl << std::endl;

				       // Additional cell information
      
      out << "object \"material\" class array type int rank 0 items "
	  << n_cells << " data follows" << std::endl;
      for (cell = tria.begin_active(); cell != endc; ++cell)
	out << ' ' << (unsigned int)cell->material_id();
      out  << std::endl
	   << "attribute \"dep\" string \"connections\"" << std::endl << std::endl;
      
      out << "object \"level\" class array type int rank 0 items "
	  << n_cells << " data follows" << std::endl;
      for (cell = tria.begin_active(); cell != endc; ++cell)
	out << ' ' << cell->level();
      out  << std::endl
	   << "attribute \"dep\" string \"connections\"" << std::endl << std::endl;
      
      if (dx_flags.write_measure)
	{
	  out << "object \"measure\" class array type float rank 0 items "
	      << n_cells << " data follows" << std::endl;
	  for (cell = tria.begin_active(); cell != endc; ++cell)
	    out << '\t' << cell->measure();
	  out  << std::endl
	       << "attribute \"dep\" string \"connections\"" << std::endl << std::endl;
	}
      
      if (dx_flags.write_diameter)
	{
	  out << "object \"diameter\" class array type float rank 0 items "
	      << n_cells << " data follows" << std::endl;
	  for (cell = tria.begin_active(); cell != endc; ++cell)
	    out << '\t' << cell->diameter();
	  out  << std::endl
	       << "attribute \"dep\" string \"connections\"" << std::endl << std::endl;
	}
    }
  
  if (write_faces)
    {
      out << "object \"faces\" class array type int rank 1 shape "
	  << n_vertices_per_face
	  << " items " << n_faces << " data follows"
	  << std::endl;

      for (cell = tria.begin_active(); cell != endc; ++cell)
	{
	  for (unsigned int f=0;f<GeometryInfo<dim>::faces_per_cell;++f)
	    {
	      typename Triangulation<dim>::face_iterator face = cell->face(f);
	      
	      for (unsigned int v=0; v<GeometryInfo<dim>::vertices_per_face; ++v)
		out << '\t' << face->vertex_index(GeometryInfo<dim-1>::dx_to_deal[v]);
	      out << std::endl;
	    }
	}
      out << "attribute \"element type\" string \"";
      if (dim==2) out << "lines";
      if (dim==3) out << "quads";
      out << "\"" << std::endl
	  << "attribute \"ref\" string \"positions\"" << std::endl << std::endl;
      

				       // Additional face information
      
      out << "object \"boundary\" class array type int rank 0 items "
	  << n_faces << " data follows" << std::endl;
      for (cell = tria.begin_active(); cell != endc; ++cell)
	{
					   // Little trick to get -1
					   // for the interior
	  for (unsigned int f=0;f<GeometryInfo<dim>::faces_per_cell;++f)
	    out << ' ' << (int)(signed char)cell->face(f)->boundary_indicator();
	  out << std::endl;
	}
      out << "attribute \"dep\" string \"connections\"" << std::endl << std::endl;
      
      if (dx_flags.write_measure)
	{
	  out << "object \"face measure\" class array type float rank 0 items "
	      << n_faces << " data follows" << std::endl;
	  for (cell = tria.begin_active(); cell != endc; ++cell)
	    {
	      for (unsigned int f=0;f<GeometryInfo<dim>::faces_per_cell;++f)
		out << ' ' << cell->face(f)->measure();
	      out << std::endl;
	    }
	  out << "attribute \"dep\" string \"connections\"" << std::endl << std::endl;
	}

      if (dx_flags.write_diameter)
	{
	  out << "object \"face diameter\" class array type float rank 0 items "
	      << n_faces << " data follows" << std::endl;
	  for (cell = tria.begin_active(); cell != endc; ++cell)
	    {
	      for (unsigned int f=0;f<GeometryInfo<dim>::faces_per_cell;++f)
		out << ' ' << cell->face(f)->diameter();
	      out << std::endl;
	    }
	  out << "attribute \"dep\" string \"connections\"" << std::endl << std::endl;
	}

    }
  

				   // Write additional face information
  
  if (write_faces)
    {
      
    }
  else
    {
     }

				   // The wrapper
  out << "object \"deal data\" class field" << std::endl
      << "component \"positions\" value \"vertices\"" << std::endl
      << "component \"connections\" value \"cells\"" << std::endl;

  if (write_cells)
    {
      out << "object \"cell data\" class field" << std::endl
	  << "component \"positions\" value \"vertices\"" << std::endl
	  << "component \"connections\" value \"cells\"" << std::endl;
      out << "component \"material\" value \"material\"" << std::endl;
      out << "component \"level\" value \"level\"" << std::endl;
      if (dx_flags.write_measure)
	out << "component \"measure\" value \"measure\"" << std::endl;
      if (dx_flags.write_diameter)
	out << "component \"diameter\" value \"diameter\"" << std::endl;
    }

  if (write_faces)
    {
      out << "object \"face data\" class field" << std::endl
	  << "component \"positions\" value \"vertices\"" << std::endl
	  << "component \"connections\" value \"faces\"" << std::endl;
      out << "component \"boundary\" value \"boundary\"" << std::endl;
      if (dx_flags.write_measure)
	out << "component \"measure\" value \"face measure\"" << std::endl;
      if (dx_flags.write_diameter)
	out << "component \"diameter\" value \"face diameter\"" << std::endl;
    }
  
  out << std::endl
      << "object \"grid data\" class group" << std::endl;
    if (write_cells)
      out << "member \"cells\" value \"cell data\"" << std::endl;
    if (write_faces)
      out << "member \"faces\" value \"face data\"" << std::endl;
  out << "end" << std::endl;  
}


#endif


template <int dim>
void GridOut::write_ucd (const Triangulation<dim> &tria,
			 std::ostream             &out) 
{
  AssertThrow (out, ExcIO());

				   // get the positions of the
				   // vertices and whether they are
				   // used.
  const std::vector<Point<dim> > &vertices    = tria.get_vertices();
  const std::vector<bool>        &vertex_used = tria.get_used_vertices();
  
  const unsigned int n_vertices = tria.n_used_vertices();

  typename Triangulation<dim>::active_cell_iterator       cell=tria.begin_active();
  const typename Triangulation<dim>::active_cell_iterator endc=tria.end();

				   // write preamble
  if (ucd_flags.write_preamble)
    {
				       // block this to have local
				       // variables destroyed after
				       // use
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
    };

				   // start with ucd data
  out << n_vertices << ' '
      << tria.n_active_cells() + (ucd_flags.write_faces ?
				  n_boundary_faces(tria) :
				  0)
      << " 0 0 0"                  // no data
      << std::endl;

				   // actually write the vertices.
				   // note that we shall number them
				   // with first index 1 instead of 0
  for (unsigned int i=0; i<vertices.size(); ++i)
    if (vertex_used[i])
      {
	out << i+1                 // vertex index
	    << "  "
	    << vertices[i];
	for (unsigned int d=dim+1; d<=3; ++d)
	  out << " 0";             // fill with zeroes
	out << std::endl;
      };
	
				   // write cells. Enumerate cells
				   // consecutively, starting with 1
  unsigned int cell_index=1;
  for (cell=tria.begin_active();
       cell!=endc; ++cell, ++cell_index)
    {
      out << cell_index << ' '
	  << static_cast<unsigned int>(cell->material_id())
	  << " ";
      switch (dim) 
	{
	  case 1:  out << "line    "; break;
	  case 2:  out << "quad    "; break;
	  case 3:  out << "hex     "; break;
	  default:
		Assert (false, ExcNotImplemented());
	};

				       // it follows a list of the
				       // vertices of each cell. in 1d
				       // this is simply a list of the
				       // two vertices, in 2d its counter
				       // clockwise, as usual in this
				       // library. in 3d, the same applies
				       // (special thanks to AVS for
				       // numbering their vertices in a
				       // way compatible to deal.II!)
				       //
				       // technical reference:
				       // AVS Developer's Guide, Release 4,
				       // May, 1992, p. E6
				       //
				       // note: vertex numbers are 1-base
      for (unsigned int vertex=0; vertex<GeometryInfo<dim>::vertices_per_cell;
	   ++vertex)
	out << cell->vertex_index(vertex)+1 << ' ';
      out << std::endl;
    };

				   // write faces with non-zero boundary
				   // indicator
  if (ucd_flags.write_faces)
    write_ucd_faces (tria, cell_index, out);
    
  AssertThrow (out, ExcIO());
};



#if deal_II_dimension == 1

unsigned int GridOut::n_boundary_faces (const Triangulation<1> &) const
{
  return 0;
};

#endif



template <int dim>
unsigned int GridOut::n_boundary_faces (const Triangulation<dim> &tria) const
{
  typename Triangulation<dim>::active_face_iterator face, endf;
  unsigned int n_faces = 0;

  for (face=tria.begin_active_face(), endf=tria.end_face();
       face != endf; ++face)
    if ((face->at_boundary()) &&
	(face->boundary_indicator() != 0))
      n_faces++;

  return n_faces;
};



#if deal_II_dimension == 1

void GridOut::write_ucd_faces (const Triangulation<1> &,
			       const unsigned int,
			       std::ostream &) const
{
  return;
};

#endif



template <int dim>
void GridOut::write_ucd_faces (const Triangulation<dim> &tria,
			       const unsigned int        starting_index,
			       std::ostream             &out) const
{
  typename Triangulation<dim>::active_face_iterator face, endf;
  unsigned int index=starting_index;

  for (face=tria.begin_active_face(), endf=tria.end_face();
       face != endf; ++face)
    if (face->at_boundary() &&
	(face->boundary_indicator() != 0)) 
      {
	out << index << "  "
	    << static_cast<unsigned int>(face->boundary_indicator())
	    << "  ";
	switch (dim) 
	  {
	    case 2: out << "line    ";  break;
	    case 3: out << "quad    ";  break;
	    default:
		  Assert (false, ExcNotImplemented());
	  };
				       // note: vertex numbers are 1-base
	for (unsigned int vertex=0; vertex<GeometryInfo<dim>::vertices_per_face; ++vertex)
	  out << face->vertex_index(vertex)+1 << ' ';
	out << std::endl;

	++index;
      };	  
};

#if deal_II_dimension==1

void GridOut::write_gnuplot (const Triangulation<1> &tria,
			     std::ostream           &out,
			     const Mapping<1>       *) 
{
  AssertThrow (out, ExcIO());

  Triangulation<1>::active_cell_iterator        cell=tria.begin_active();
  const Triangulation<1>::active_cell_iterator  endc=tria.end();
  for (; cell!=endc; ++cell)
    {
      if (gnuplot_flags.write_cell_numbers)
	out << "# cell " << cell << std::endl;

      out << cell->vertex(0)
	  << ' ' << cell->level()
	  << ' ' << static_cast<unsigned int>(cell->material_id()) << std::endl
	  << cell->vertex(1)
		    << ' ' << cell->level()
	  << ' ' << static_cast<unsigned int>(cell->material_id()) << std::endl
	  << std::endl;
      break;
    }
  AssertThrow (out, ExcIO());
}


#else


template <int dim>
void GridOut::write_gnuplot (const Triangulation<dim> &tria,
			     std::ostream             &out,
			     const Mapping<dim>       *mapping) 
{
  AssertThrow (out, ExcIO());

  const unsigned int n_points=
    gnuplot_flags.n_boundary_face_points;

  typename Triangulation<dim>::active_cell_iterator        cell=tria.begin_active();
  const typename Triangulation<dim>::active_cell_iterator  endc=tria.end();

				   // if we are to treat curved
				   // boundaries, then generate a
				   // quadrature formula which will be
				   // used to probe boundary points at
				   // curved faces
  Quadrature<dim> *q_projector=0;
  if (mapping!=0)
    {
      typename std::vector<Point<dim-1> > boundary_points(n_points);
      for (unsigned int i=0; i<n_points; ++i)
	boundary_points[i](0)= 1.*(i+1)/(n_points+1);

      Quadrature<dim-1> quadrature(boundary_points);
      q_projector = new Quadrature<dim> (QProjector<dim>::project_to_all_faces(quadrature));
    }
  
  for (; cell!=endc; ++cell)
    {
      if (gnuplot_flags.write_cell_numbers)
	out << "# cell " << cell << std::endl;

      switch (dim)
	{
	  case 1:
	  {
	    Assert(false, ExcInternalError());
	    break;
	  };
	   
	  case 2:
	  {
	    if (mapping==0 || !cell->at_boundary())
	      {
						 // write out the four
						 // sides of this cell
						 // by putting the
						 // four points (+ the
						 // initial point
						 // again) in a row
						 // and lifting the
						 // drawing pencil at
						 // the end
		out << cell->vertex(0)
		    << ' ' << cell->level()
		    << ' ' << static_cast<unsigned int>(cell->material_id()) << std::endl
		    << cell->vertex(1)
		    << ' ' << cell->level()
		    << ' ' << static_cast<unsigned int>(cell->material_id()) << std::endl
		    << cell->vertex(2)
		    << ' ' << cell->level()
		    << ' ' << static_cast<unsigned int>(cell->material_id()) << std::endl
		    << cell->vertex(3)
		    << ' ' << cell->level()
		    << ' ' << static_cast<unsigned int>(cell->material_id()) << std::endl
		    << cell->vertex(0)
		    << ' ' << cell->level()
		    << ' ' << static_cast<unsigned int>(cell->material_id()) << std::endl
		    << std::endl  // double new line for gnuplot 3d plots
		    << std::endl;
	      }
	    else
					       // cell is at boundary
					       // and we are to treat
					       // curved
					       // boundaries. so loop
					       // over all faces and
					       // draw them as small
					       // pieces of lines
	      {
		for (unsigned int face_no=0;
		     face_no<GeometryInfo<dim>::faces_per_cell; ++face_no)
		  {
		    const typename Triangulation<dim>::face_iterator
		      face = cell->face(face_no);
		    if (face->at_boundary())
		      {
			out << face->vertex(0)
			    << ' ' << cell->level()
			    << ' ' << static_cast<unsigned int>(cell->material_id())
			    << std::endl;
			
			const unsigned int offset=face_no*n_points;
			for (unsigned int i=0; i<n_points; ++i)
			  out << (mapping->transform_unit_to_real_cell
				  (cell, q_projector->point(offset+i)))
			      << ' ' << cell->level()
			      << ' ' << static_cast<unsigned int>(cell->material_id())
			      << std::endl;
			
			out << face->vertex(1)
			    << ' ' << cell->level()
			    << ' ' << static_cast<unsigned int>(cell->material_id())
			    << std::endl
			    << std::endl
			    << std::endl;
		      }
		    else
		      {
							 // if,
							 // however,
							 // the face
							 // is not at
							 // the
							 // boundary,
							 // then draw
							 // it as
							 // usual
			out << face->vertex(0)
			    << ' ' << cell->level()
			    << ' ' << static_cast<unsigned int>(cell->material_id())
			    << std::endl
			    << face->vertex(1)
			    << ' ' << cell->level()
			    << ' ' << static_cast<unsigned int>(cell->material_id())
			    << std::endl
			    << std::endl
			    << std::endl;
		      };
		  };
	      };
	    
	    break;
	  };
	   
	  case 3:
	  {
//TODO:[RH] curved boundaries in 3d gnuplot not supported	    
	    Assert (mapping == 0, ExcNotImplemented());
	    
					     // front face
	    out << cell->vertex(0)
		<< ' ' << cell->level()
		<< ' ' << static_cast<unsigned int>(cell->material_id()) << std::endl
		<< cell->vertex(1)
		<< ' ' << cell->level()
		<< ' ' << static_cast<unsigned int>(cell->material_id()) << std::endl
		<< cell->vertex(2)
		<< ' ' << cell->level()
		<< ' ' << static_cast<unsigned int>(cell->material_id()) << std::endl
		<< cell->vertex(3)
		<< ' ' << cell->level()
		<< ' ' << static_cast<unsigned int>(cell->material_id()) << std::endl
		<< cell->vertex(0)
		<< ' ' << cell->level()
		<< ' ' << static_cast<unsigned int>(cell->material_id()) << std::endl
		<< std::endl;
					     // back face
	    out << cell->vertex(4)
		<< ' ' << cell->level()
		<< ' ' << static_cast<unsigned int>(cell->material_id()) << std::endl
		<< cell->vertex(5)
		<< ' ' << cell->level()
		<< ' ' << static_cast<unsigned int>(cell->material_id()) << std::endl
		<< cell->vertex(6)
		<< ' ' << cell->level()
		<< ' ' << static_cast<unsigned int>(cell->material_id()) << std::endl
		<< cell->vertex(7)
		<< ' ' << cell->level()
		<< ' ' << static_cast<unsigned int>(cell->material_id()) << std::endl
		<< cell->vertex(4)
		<< ' ' << cell->level()
		<< ' ' << static_cast<unsigned int>(cell->material_id()) << std::endl
		<< std::endl;
	    
					     // now for the four connecting lines
	    out << cell->vertex(0)
		<< ' ' << cell->level()
		<< ' ' << static_cast<unsigned int>(cell->material_id()) << std::endl
		<< cell->vertex(4)
		<< ' ' << cell->level()
		<< ' ' << static_cast<unsigned int>(cell->material_id()) << std::endl
		<< std::endl;
	    out << cell->vertex(1)
		<< ' ' << cell->level()
		<< ' ' << static_cast<unsigned int>(cell->material_id()) << std::endl
		<< cell->vertex(5)
		<< ' ' << cell->level()
		<< ' ' << static_cast<unsigned int>(cell->material_id()) << std::endl
		<< std::endl;
	    out << cell->vertex(2)
		<< ' ' << cell->level()
		<< ' ' << static_cast<unsigned int>(cell->material_id()) << std::endl
		<< cell->vertex(6)
		<< ' ' << cell->level()
		<< ' ' << static_cast<unsigned int>(cell->material_id()) << std::endl
		<< std::endl;
	    out << cell->vertex(3)
		<< ' ' << cell->level()
		<< ' ' << static_cast<unsigned int>(cell->material_id()) << std::endl
		<< cell->vertex(7)
		<< ' ' << cell->level()
		<< ' ' << static_cast<unsigned int>(cell->material_id()) << std::endl
		<< std::endl;
	    break;
	  };
	};
    };

  if (q_projector != 0)
    delete q_projector;
  
  
  AssertThrow (out, ExcIO());
};

#endif

struct LineEntry
{
    Point<2> first;
    Point<2> second;
    bool colorize;
    LineEntry(const Point<2>& f, const Point<2>& s, const bool c)
		    :
		    first(f), second(s), colorize(c)
      {}
};


#if deal_II_dimension==1

void GridOut::write_eps (const Triangulation<1> &,
			 std::ostream &,
			 const Mapping<1> *) 
{
  Assert(false, ExcNotImplemented());
}


#else

template <int dim>
void GridOut::write_eps (const Triangulation<dim> &tria,
			 std::ostream             &out,
			 const Mapping<dim>       *mapping) 
{
  
  typedef std::list<LineEntry> LineList;

				   // get a pointer to the flags
				   // common to all dimensions,
				   // in order to avoid the recurring
				   // distinctions between
				   // eps_flags_1, eps_flags_2, ...
  const GridOutFlags::EpsFlagsBase
    &eps_flags_base = (dim==2 ?
		       static_cast<GridOutFlags::EpsFlagsBase&>(eps_flags_2) :
		       (dim==3 ?
			static_cast<GridOutFlags::EpsFlagsBase&>(eps_flags_3) :
			*static_cast<GridOutFlags::EpsFlagsBase*>(0)));
  
  AssertThrow (out, ExcIO());
  const unsigned int n_points = eps_flags_base.n_boundary_face_points;

				   // make up a list of lines by which
				   // we will construct the triangulation
				   //
				   // this part unfortunately is a bit
				   // dimension dependent, so we have to
				   // treat every dimension different.
				   // however, by directly producing
				   // the lines to be printed, i.e. their
				   // 2d images, we can later do the
				   // actual output dimension independent
				   // again
  LineList line_list;

  switch (dim)
    {
      case 1:
      {
	Assert(false, ExcInternalError());
	break;
      };
       
      case 2:
      {
	typename Triangulation<dim>::active_line_iterator
	  line   =tria.begin_active_line (),
	  endline=tria.end_line ();

					 // first treat all interior
					 // lines and make up a list
					 // of them. if curved lines
					 // shall not be supported
					 // (i.e. no mapping is
					 // provided), then also treat
					 // all other lines
	for (; line!=endline; ++line)
	  if (mapping==0 || !line->at_boundary())
					     // one would expect
					     // make_pair(line->vertex(0),
					     //           line->vertex(1))
					     // here, but that is not
					     // dimension independent, since
					     // vertex(i) is Point<dim>,
					     // but we want a Point<2>.
					     // in fact, whenever we're here,
					     // the vertex is a Point<dim>,
					     // but the compiler does not
					     // know this. hopefully, the
					     // compiler will optimize away
					     // this little kludge
	    line_list.push_back (LineEntry(Point<2>(line->vertex(0)(0),
						    line->vertex(0)(1)),
					   Point<2>(line->vertex(1)(0),
						    line->vertex(1)(1)),
					   line->user_flag_set()));
	
					 // next if we are to treat
					 // curved boundaries
					 // specially, then add lines
					 // to the list consisting of
					 // pieces of the boundary
					 // lines
	if (mapping!=0)
	  {
					     // to do so, first
					     // generate a sequence of
					     // points on a face and
					     // project them onto the
					     // faces of a unit cell
	    typename std::vector<Point<dim-1> > boundary_points (n_points);
	    
	    for (unsigned int i=0; i<n_points; ++i)
	      boundary_points[i](0) = 1.*(i+1)/(n_points+1);
	    
	    Quadrature<dim-1> quadrature (boundary_points);
	    Quadrature<dim>   q_projector (QProjector<dim>::project_to_all_faces(quadrature));

					     // next loop over all
					     // boundary faces and
					     // generate the info from
					     // them
	    typename Triangulation<dim>::active_cell_iterator cell=tria.begin_active ();
	    const typename Triangulation<dim>::active_cell_iterator end=tria.end ();
	    for (; cell!=end; ++cell)
	      for (unsigned int face_no=0; face_no<GeometryInfo<dim>::faces_per_cell; ++face_no)
		{
		  const typename Triangulation<dim>::face_iterator
		    face = cell->face(face_no);
		  
		  if (face->at_boundary())
		    {
		      Point<dim> p0_dim(face->vertex(0));
		      Point<2>   p0    (p0_dim(0), p0_dim(1));

						       // loop over
						       // all pieces
						       // of the line
						       // and generate
						       // line-lets
		      const unsigned int offset=face_no*n_points;
		      for (unsigned int i=0; i<n_points; ++i)
			{
			  const Point<dim> p1_dim (mapping->transform_unit_to_real_cell
						   (cell, q_projector.point(offset+i)));
			  const Point<2>   p1     (p1_dim(0), p1_dim(1));
			  
			  line_list.push_back (LineEntry(p0, p1,
							 face->user_flag_set()));
			  p0=p1;
			}

						       // generate last piece
		      const Point<dim> p1_dim (face->vertex(1));
		      const Point<2>   p1     (p1_dim(0), p1_dim(1));
		      line_list.push_back (LineEntry(p0, p1,
						     face->user_flag_set()));
		    };
		};
	  };
	
	break;
      };
       
      case 3:
      {
					 // curved boundary output
					 // presently not supported
//TODO:[RH] curved boundaries in eps for 3d	
	Assert (mapping == 0, ExcNotImplemented());
	
	typename Triangulation<dim>::active_line_iterator
	  line   =tria.begin_active_line (),
	  endline=tria.end_line ();
	
					 // loop over all lines and compute their
					 // projection on the plane perpendicular
					 // to the direction of sight

					 // direction of view equals the unit 
					 // vector of the position of the
					 // spectator to the origin.
					 //
					 // we chose here the viewpoint as in
					 // gnuplot as default.
					 //
//TODO:[WB] Fix a potential problem with viewing angles in 3d Eps GridOut
					 // note: the following might be wrong
					 // if one of the base vectors below
					 // is in direction of the viewer, but
					 // I am too tired at present to fix
					 // this
//TODO:[?] Unify the various places where PI is defined to a central instance  
	const double pi = 3.141592653589793238462;
	const double z_angle    = eps_flags_3.azimut_angle;
	const double turn_angle = eps_flags_3.turn_angle;
	const Point<dim> view_direction(-std::sin(z_angle * 2.*pi / 360.) * std::sin(turn_angle * 2.*pi / 360.),
					+std::sin(z_angle * 2.*pi / 360.) * std::cos(turn_angle * 2.*pi / 360.),
					-std::cos(z_angle * 2.*pi / 360.));
	
					 // decide about the two unit vectors
					 // in this plane. we chose the first one
					 // to be the projection of the z-axis
					 // to this plane
	const Point<dim> vector1
	  = Point<dim>(0,0,1) - ((Point<dim>(0,0,1) * view_direction) * view_direction);
	const Point<dim> unit_vector1 = vector1 / std::sqrt(vector1.square());
	
					 // now the third vector is fixed. we
					 // chose the projection of a more or
					 // less arbitrary vector to the plane
					 // perpendicular to the first one
	const Point<dim> vector2
	  = (Point<dim>(1,0,0)
	     - ((Point<dim>(1,0,0) * view_direction) * view_direction)
	     - ((Point<dim>(1,0,0) * unit_vector1)   * unit_vector1));
	const Point<dim> unit_vector2 = vector2 / std::sqrt(vector2.square());
	
	for (; line!=endline; ++line) 
	  line_list.push_back (LineEntry(Point<2>(line->vertex(0) * unit_vector2,
						  line->vertex(0) * unit_vector1),
					 Point<2>(line->vertex(1) * unit_vector2,
						  line->vertex(1) * unit_vector1),
					 line->user_flag_set()));

	break;
      };

      default:
	    Assert (false, ExcNotImplemented());
    };



				   // find out minimum and maximum x and
				   // y coordinates to compute offsets
				   // and scaling factors
  double x_min = tria.begin_active_line()->vertex(0)(0);
  double x_max = x_min;
  double y_min = tria.begin_active_line()->vertex(0)(1);
  double y_max = y_min;

  for (LineList::const_iterator line=line_list.begin();
       line!=line_list.end(); ++line)
    {
      x_min = std::min (x_min, line->first(0));
      x_min = std::min (x_min, line->second(0));

      x_max = std::max (x_max, line->first(0));
      x_max = std::max (x_max, line->second(0));

      y_min = std::min (y_min, line->first(1));
      y_min = std::min (y_min, line->second(1));

      y_max = std::max (y_max, line->first(1));
      y_max = std::max (y_max, line->second(1));
    };

				   // scale in x-direction such that
				   // in the output 0 <= x <= 300.
				   // don't scale in y-direction to
				   // preserve the shape of the
				   // triangulation
  const double scale = (eps_flags_base.size /
			(eps_flags_base.size_type==GridOutFlags::EpsFlagsBase::width ?
			 x_max - x_min :
			 y_min - y_max));


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
	  << static_cast<unsigned int>( (x_max-x_min) * scale )+1
	  << ' '
	  << static_cast<unsigned int>( (y_max-y_min) * scale )+1
	  << std::endl;

				       // define some abbreviations to keep
				       // the output small:
				       // m=move turtle to
				       // x=execute line stroke
				       // b=black pen
				       // r=red pen
      out << "/m {moveto} bind def" << std::endl
	  << "/x {lineto stroke} bind def" << std::endl
	  << "/b {0 0 0 setrgbcolor} def" << std::endl
	  << "/r {1 0 0 setrgbcolor} def" << std::endl;

				       // in 2d, we can also plot cell
				       // numbers, but this requires a
				       // somewhat more lengthy
				       // preamble. please don't ask
				       // me what most of this means,
				       // it is reverse engineered
				       // from what GNUPLOT uses in
				       // its output
      if ((dim == 2) && (eps_flags_2.write_cell_numbers == true))
	{
	  out << ("/R {rmoveto} bind def\n"
		  "/Symbol-Oblique /Symbol findfont [1 0 .167 1 0 0] makefont\n"
		  "dup length dict begin {1 index /FID eq {pop pop} {def} ifelse} forall\n"
		  "currentdict end definefont\n"
		  "/MFshow {{dup dup 0 get findfont exch 1 get scalefont setfont\n"
		  "[ currentpoint ] exch dup 2 get 0 exch rmoveto dup dup 5 get exch 4 get\n"
		  "{show} {stringwidth pop 0 rmoveto}ifelse dup 3 get\n"
		  "{2 get neg 0 exch rmoveto pop} {pop aload pop moveto}ifelse} forall} bind def\n"
		  "/MFwidth {0 exch {dup 3 get{dup dup 0 get findfont exch 1 get scalefont setfont\n"
		  "5 get stringwidth pop add}\n"
		  "{pop} ifelse} forall} bind def\n"
		  "/MCshow { currentpoint stroke m\n"
		  "exch dup MFwidth -2 div 3 -1 roll R MFshow } def\n")
	      << std::endl;
	};
      
      out << "%%EndProlog" << std::endl
	  << std::endl;

				       // set fine lines
      out << eps_flags_base.line_width << " setlinewidth" << std::endl;
    };

				   // now write the lines
  const Point<2> offset(x_min, y_min);
  
  for (LineList::const_iterator line=line_list.begin();
       line!=line_list.end(); ++line)
    out << ((line->colorize && eps_flags_base.color_lines_on_user_flag) ? "r " : "b ")
	<< (line->first  - offset) * scale << " m "
	<< (line->second - offset) * scale << " x" << std::endl;

				   // finally write the cell numbers
				   // in 2d, if that is desired
  if ((dim == 2) && (eps_flags_2.write_cell_numbers == true))
    {
      out << "(Helvetica) findfont 140 scalefont setfont"
	  << std::endl;
      
      typename Triangulation<dim>::active_cell_iterator
	cell = tria.begin_active (),
	endc = tria.end ();
      for (; cell!=endc; ++cell)
	{
	  out << (cell->center()(0)-offset(0))*scale << ' '
	      << (cell->center()(1)-offset(1))*scale
	      << " m" << std::endl
	      << "[ [(Helvetica) 12.0 0.0 true true (";
	  if (eps_flags_2.write_cell_number_level)
	    out << cell;
	  else
	    out << cell->index();

	  out  << " )] "
	       << "] -6 MCshow"
	       << std::endl;
	};
    };

  out << "showpage" << std::endl;
  
  AssertThrow (out, ExcIO());
};

#endif


template <int dim>
void GridOut::write (const Triangulation<dim> &tria,
		     std::ostream             &out,
		     const OutputFormat        output_format,
		     const Mapping<dim>       *mapping)
{
  switch (output_format)
    {
      case dx:
	    write_dx (tria, out);
	    return;

      case ucd:
	    write_ucd (tria, out);
	    return;

      case gnuplot:
	    write_gnuplot (tria, out, mapping);
	    return;

      case eps:
	    write_eps (tria, out, mapping);
	    return;
    };

  Assert (false, ExcInternalError());
};



// explicit instantiations
template void GridOut::write_ucd<deal_II_dimension>
(const Triangulation<deal_II_dimension> &,
 std::ostream &);
#if deal_II_dimension != 1
template void GridOut::write_gnuplot<deal_II_dimension>
(const Triangulation<deal_II_dimension> &,
 std::ostream &,
 const Mapping<deal_II_dimension> *);
template void GridOut::write_eps<deal_II_dimension>
(const Triangulation<deal_II_dimension> &,
 std::ostream &,
 const Mapping<deal_II_dimension> *);
#endif
template void GridOut::write<deal_II_dimension>
(const Triangulation<deal_II_dimension> &,
 std::ostream &,
 const OutputFormat,
 const Mapping<deal_II_dimension> *);
