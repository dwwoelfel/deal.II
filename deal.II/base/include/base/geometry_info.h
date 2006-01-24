//---------------------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------
#ifndef __deal2__geometry_info_h
#define __deal2__geometry_info_h


#include <base/config.h>
#include <base/exceptions.h>
#include <base/point.h>


template <int dim> class GeometryInfo;



/**
 * Dimension independent base class for the <tt>GeometryInfo<dim></tt>
 * classes with <tt>dim=1,2,3,4</tt>. Includes all data and methods
 * which can be defined in an dimension indendent way.
 *
 * @ingroup grid geomprimitives
 * @author Ralf Hartmann, 2005
 */
class GeometryInfoBase
{
  private:
				     /**
				      * The maximal dimension for
				      * which data in this class is
				      * implemented.
				      */
    static const unsigned int max_dim = 4;

				     /**
				      * Number of faces of a cell for
				      * <tt>dim=max_dim</tt>.
				      */
    static const unsigned int faces_per_cell_max_dim = 2*max_dim;

  public:
        
				     /**
				      * For each face of the reference
				      * cell, this field stores the
				      * coordinate direction in which
				      * its normal vector points. In
				      * <tt>dim</tt> dimension these
				      * are the <tt>2*dim</tt> first
				      * entries of
				      * <tt>{0,0,1,1,2,2,3,3}</tt>.
				      *
				      * Remark that this is only the
				      * coordinate number. The actual
				      * direction of the normal vector
				      * is obtained by multiplying the
				      * unit vector in this direction
				      * with #unit_normal_orientation.
				      */
    static const unsigned int unit_normal_direction[faces_per_cell_max_dim];

				     /**
				      * Orientation of the unit normal
				      * vector of a face of the
				      * reference cell. In
				      * <tt>dim</tt> dimension these
				      * are the <tt>2*dim</tt> first
				      * entries of
				      * <tt>{-1,1,-1,1,-1,1,-1,1}</tt>.
				      *
				      * Each value is either
				      * <tt>1</tt> or <tt>-1</tt>,
				      * corresponding to a normal
				      * vector pointing in the
				      * positive or negative
				      * coordinate direction,
				      * respectively.
				      *
				      * Note that this is only the
				      * <em>standard orientation</em>
				      * of faces. At least in 3d,
				      * actual faces of cells in a
				      * triangulation can also have
				      * the opposite orientation,
				      * depending on a flag that one
				      * can query from the cell it
				      * belongs to. For more
				      * information, see the
				      * @ref GlossFaceOrientation "glossary"
				      * entry on
				      * face orientation.
				      */
    static const int unit_normal_orientation[faces_per_cell_max_dim];

				     /**
				      * List of numbers which denotes
				      * which face is opposite to a
				      * given face. Its entries are
				      * <tt>{ 1, 0, 3, 2, 5, 4, 7, 6}</tt>.
				      */
    static const unsigned int opposite_face[faces_per_cell_max_dim];
};




/**
 * Topological description of zero dimensional cells,
 * i.e. points. This class might not look too useful but often is if
 * in a certain dimension we would like to enquire information about
 * objects with dimension one lower than the present, e.g. about
 * faces.
 *
 * This class contains as static members information on vertices and
 * faces of a @p dim-dimensional grid cell. The interface is the same
 * for all dimensions. If a value is of no use in a low dimensional
 * cell, it is (correctly) set to zero, e.g. #subfaces_per_face in
 * 1d.
 *
 * This information should always replace hard-coded numbers of
 * vertices, neighbors and so on, since it can be used dimension
 * independently.
 *
 * @ingroup grid geomprimitives
 * @author Wolfgang Bangerth, 1998
 */
template <>
struct GeometryInfo<0> 
{

				     /**
				      * Number of children a cell has.
				      */
    static const unsigned int children_per_cell = 1;

				     /**
				      * Number of faces a cell has.
				      */
    static const unsigned int faces_per_cell    = 0;

				     /**
				      * Number of children each face has
				      * when the adjacent cell is refined.
				      */
    static const unsigned int subfaces_per_face = 0;

				     /**
				      * Number of vertices a cell has.
				      */
    static const unsigned int vertices_per_cell = 1;

				     /**
				      * Number of vertices each face has.
				      * Since this is not useful in one
				      * dimension, we provide a useless
				      * number (in the hope that a compiler
				      * may warn when it sees constructs like
				      * <tt>for (i=0; i<vertices_per_face; ++i)</tt>,
				      * at least if @p i is an <tt>unsigned int</tt>.
				      */
    static const unsigned int vertices_per_face = 0;

				     /**
				      * Number of lines each face has.
				      */
    static const unsigned int lines_per_face    = 0;
    
				     /**
				      * Number of quads on each face.
				      */
    static const unsigned int quads_per_face    = 0;

				     /**
				      * Number of lines of a cell.
				      */
    static const unsigned int lines_per_cell    = 0;

				     /**
				      * Number of quadrilaterals of a
				      * cell.
				      */
    static const unsigned int quads_per_cell    = 0;

				     /**
				      * Number of hexahedra of a
				      * cell.
				      */
    static const unsigned int hexes_per_cell    = 0;
};





/**
 * This template specifies the interface to all topological structure
 * of the mesh cells.
 *
 * The information of this class is used extensively in the geometric
 * description of Triangulation objects, as well as in various other
 * parts of the code.
 *
 * @note Instantiations for this template are provided for dimensions 1,2,3,4,
 * and there is a specialization for dim=0 (see the section on @ref
 * Instantiations in the manual).
 *
 * @ingroup grid geomprimitives
 * @author Wolfgang Bangerth, 1998, Ralf Hartmann, 2005
 */
template <int dim>
struct GeometryInfo: public GeometryInfoBase
{
    
				     /**
				      * Number of children of a refined cell.
				      */
    static const unsigned int children_per_cell = 1 << dim;

				     /**
				      * Number of faces of a cell.
				      */
    static const unsigned int faces_per_cell = 2 * dim;

				     /**
				      * Number of children each face has
				      * when the adjacent cell is refined.
				      */
    static const unsigned int subfaces_per_face = GeometryInfo<dim-1>::children_per_cell;

				     /**
				      * Number of vertices of a cell.
				      */
    static const unsigned int vertices_per_cell = 1 << dim;

				     /**
				      * Number of vertices on each
				      * face.
				      */
    static const unsigned int vertices_per_face = GeometryInfo<dim-1>::vertices_per_cell;

				     /**
				      * Number of lines on each face.
				      */
    static const unsigned int lines_per_face
    = GeometryInfo<dim-1>::lines_per_cell;
    
				     /**
				      * Number of quads on each face.
				      */
    static const unsigned int quads_per_face
    = GeometryInfo<dim-1>::quads_per_cell;

				     /**
				      * Number of lines of a cell.
				      *
				      * The formula to compute this makes use
				      * of the fact that when going from one
				      * dimension to the next, the object of
				      * the lower dimension is copied once
				      * (thus twice the old number of lines)
				      * and then a new line is inserted
				      * between each vertex of the old object
				      * and the corresponding one in the copy.
				      */
    static const unsigned int lines_per_cell
    = (2*GeometryInfo<dim-1>::lines_per_cell +
       GeometryInfo<dim-1>::vertices_per_cell);

				     /**
				      * Number of quadrilaterals of a
				      * cell.
				      *
				      * This number is computed recursively
				      * just as the previous one, with the
				      * exception that new quads result from
				      * connecting an original line and its
				      * copy.
				      */
    static const unsigned int quads_per_cell
    = (2*GeometryInfo<dim-1>::quads_per_cell +
       GeometryInfo<dim-1>::lines_per_cell);

				     /**
				      * Number of hexahedra of a
				      * cell.
				      */
    static const unsigned int hexes_per_cell
    = (2*GeometryInfo<dim-1>::hexes_per_cell +
       GeometryInfo<dim-1>::quads_per_cell);

				     /**
				      * Rearrange vertices for UCD
				      * output.  For a cell being
				      * written in UCD format, each
				      * entry in this field contains
				      * the number of a vertex in
				      * <tt>deal.II</tt> that corresponds
				      * to the UCD numbering at this
				      * location.
				      *
				      * Typical example: write a cell
				      * and arrange the vertices, such
				      * that UCD understands them.
				      *
				      * \begin{verbatim}
				      * for (i=0; i< n_vertices; ++i)
				      *   out << cell->vertex(ucd_to_deal[i]);
				      * \end{verbatim}
				      *
				      * As the vertex numbering in
				      * deal.II versions <= 5.1
				      * happened to coincide with the
				      * UCD numbering, this field can
				      * also be used like a
				      * old_to_lexicographic mapping.
				      */
    static const unsigned int ucd_to_deal[vertices_per_cell];

				     /**
				      * Rearrange vertices for OpenDX
				      * output.  For a cell being
				      * written in OpenDX format, each
				      * entry in this field contains
				      * the number of a vertex in
				      * <tt>deal.II</tt> that corresponds
				      * to the DX numbering at this
				      * location.
				      *
				      * Typical example: write a cell
				      * and arrange the vertices, such
				      * that OpenDX understands them.
				      *
				      * \begin{verbatim}
				      * for (i=0; i< n_vertices; ++i)
				      *   out << cell->vertex(dx_to_deal[i]);
				      * \end{verbatim}
				      */
    static const unsigned int dx_to_deal[vertices_per_cell];
    
				     /**
				      * This field stores which child
				      * cells are adjacent to a
				      * certain face of the mother
				      * cell.
				      *
				      * For example, in 2D the layout of
				      * a cell is as follows:
				      * @verbatim
				      * .      3
				      * .   2-->--3
				      * .   |     |
				      * . 0 ^     ^ 1
				      * .   |     |
				      * .   0-->--1
				      * .      2
				      * @endverbatim
				      * Vertices and faces are indicated
				      * with their numbers, faces also with
				      * their directions.
				      *
				      * Now, when refined, the layout is
				      * like this:
				      * @verbatim
				      * *--*--*
				      * | 2|3 |
				      * *--*--*
				      * | 0|1 |
				      * *--*--*
				      * @endverbatim
				      *
				      * Thus, the child cells on face
				      * 0 are (ordered in the
				      * direction of the face) 0 and
				      * 2, on face 3 they are 2 and 3,
				      * etc.
				      *
				      * For three spatial dimensions,
				      * the exact order of the
				      * children is laid down in the
				      * documentation of the
				      * Triangulation class. Through
				      * the <tt>face_orientation</tt>
				      * argument this function handles
				      * faces oriented in both, the
				      * standard and non-standard
				      * orientation.
				      * <tt>face_orientation</tt>
				      * defaults to <tt>true</tt>
				      * (standard orientation) and has
				      * no effect in 2d.
				      */
    static unsigned int child_cell_on_face (const unsigned int face,
					    const unsigned int subface,
					    const bool face_orientation = true);
    
				     /**
				      * Map line vertex number to cell
				      * vertex number, i.e. give the
				      * cell vertex number of the
				      * <tt>vertex</tt>th vertex of
				      * line <tt>line</tt>, e.g.
				      * <tt>GeometryInfo<2>::line_to_cell_vertices(3,0)=2</tt>.
				      *
				      * The order of the lines, as
				      * well as their direction (which
				      * in turn determines which is
				      * the first and which the second
				      * vertex on a line) is the
				      * canonical one in deal.II, as
				      * described in the documentation
				      * of the Triangulation
				      * class.
				      *
				      * For <tt>dim=2</tt> this call
				      * is simply passed down to the
				      * face_to_cell_vertices()
				      * function.
				      */
    static unsigned int line_to_cell_vertices (const unsigned int line,
					       const unsigned int vertex);

				     /**
				      * Map face vertex number to cell
				      * vertex number, i.e. give the
				      * cell vertex number of the
				      * <tt>vertex</tt>th vertex of
				      * face <tt>face</tt>, e.g.
				      * <tt>GeometryInfo<2>::face_to_cell_vertices(3,0)=2</tt>.
				      *
				      * Through the
				      * <tt>face_orientation</tt>
				      * argument this function handles
				      * faces oriented in both, the
				      * standard and non-standard
				      * orientation.
				      * <tt>face_orientation</tt>
				      * defaults to <tt>true</tt>
				      * (standard orientation) and has
				      * no effect in 2d.
				      *
				      * As the children of a cell are
				      * ordered according to the
				      * vertices of the cell, this
				      * call is passed down to the
				      * child_cell_on_face() function.
				      * Hence this function is simply
				      * a wrapper of
				      * child_cell_on_face() giving it
				      * a suggestive name.
				      */
    static unsigned int face_to_cell_vertices (const unsigned int face,
					       const unsigned int vertex,
					       const bool face_orientation = true);

				     /**
				      * Map face line number to cell
				      * line number, i.e. give the
				      * cell line number of the
				      * <tt>line</tt>th line of face
				      * <tt>face</tt>, e.g.
				      * <tt>GeometryInfo<3>::face_to_cell_lines(5,0)=4</tt>.
				      *
				      * Through the
				      * <tt>face_orientation</tt>
				      * argument this function handles
				      * faces oriented in both, the
				      * standard and non-standard
				      * orientation.
				      * <tt>face_orientation</tt>
				      * defaults to <tt>true</tt>
				      * (standard orientation) and has
				      * no effect in 2d.
				      */
    static unsigned int face_to_cell_lines (const unsigned int face,
					    const unsigned int line,
					    const bool face_orientation = true);
    
				     /**
				      * Return the position of the
				      * @p ith vertex on the unit
				      * cell. The order of vertices is
				      * the canonical one in deal.II,
				      * as described in the
				      * documentation of the
				      * Triangulation class.
				      */
    static Point<dim> unit_cell_vertex (const unsigned int vertex);

				     /**
				      * Given a point @p p in unit
				      * coordinates, return the number
				      * of the child cell in which it
				      * would lie in. If the point
				      * lies on the interface of two
				      * children, return any one of
				      * their indices. The result is
				      * always less than
				      * GeometryInfo<dimension>::children_per_cell.
				      *
				      * The order of child cells is
				      * described the documentation of
				      * the Triangulation class.
				      */
    static unsigned int child_cell_from_point (const Point<dim> &p);

				     /**
				      * Given coordinates @p p on the
				      * unit cell, return the values
				      * of the coordinates of this
				      * point in the coordinate system
				      * of the given child. Neither
				      * original nor returned
				      * coordinates need actually be
				      * inside the cell, we simply
				      * perform a scale-and-shift
				      * operation with a shift that
				      * depends on the number of the
				      * child.
				      */
    static Point<dim> cell_to_child_coordinates (const Point<dim>    &p,
						 const unsigned int child_index);

				     /**
				      * The reverse function to the
				      * one above: take a point in the
				      * coordinate system of the
				      * child, and transform it to the
				      * coordinate system of the
				      * mother cell.
				      */
    static Point<dim> child_to_cell_coordinates (const Point<dim>    &p,
						 const unsigned int child_index);

				     /**
				      * Return true if the given point
				      * is inside the unit cell of the
				      * present space dimension.
				      */
    static bool is_inside_unit_cell (const Point<dim> &p);
    
				     /**
				      * Exception
				      */
    DeclException1 (ExcInvalidCoordinate,
		    double,
		    << "The coordinates must satisfy 0 <= x_i <= 1, "
		    << "but here we have x_i=" << arg1);
};


/* -------------- declaration of explicit specializations ------------- */


#ifndef DOXYGEN

template <>
inline
Point<1>
GeometryInfo<1>::unit_cell_vertex (const unsigned int vertex)
{
  Assert (vertex < vertices_per_cell,
	  ExcIndexRange (vertex, 0, vertices_per_cell));

  return Point<1>(static_cast<double>(vertex));
}



template <>
inline
Point<2>
GeometryInfo<2>::unit_cell_vertex (const unsigned int vertex)
{
  Assert (vertex < vertices_per_cell,
	  ExcIndexRange (vertex, 0, vertices_per_cell));

  return Point<2>(vertex%2, vertex/2);
}



template <>
inline
Point<3>
GeometryInfo<3>::unit_cell_vertex (const unsigned int vertex)
{
  Assert (vertex < vertices_per_cell,
	  ExcIndexRange (vertex, 0, vertices_per_cell));

  return Point<3>(vertex%2, vertex/2%2, vertex/4);
}



template <int dim>
inline
Point<dim>
GeometryInfo<dim>::unit_cell_vertex (const unsigned int)
{
  Assert(false, ExcNotImplemented());

  return Point<dim> ();  
}



template <>
inline
unsigned int
GeometryInfo<1>::child_cell_from_point (const Point<1> &p)
{
  Assert ((p[0] >= 0) && (p[0] <= 1), ExcInvalidCoordinate(p[0]));
  
  return (p[0] <= 0.5 ? 0 : 1);
}



template <>
inline
unsigned int
GeometryInfo<2>::child_cell_from_point (const Point<2> &p)
{
  Assert ((p[0] >= 0) && (p[0] <= 1), ExcInvalidCoordinate(p[0]));
  Assert ((p[1] >= 0) && (p[1] <= 1), ExcInvalidCoordinate(p[1]));
  
  return (p[0] <= 0.5 ?
	  (p[1] <= 0.5 ? 0 : 2) :
	  (p[1] <= 0.5 ? 1 : 3));
}



template <>
inline
unsigned int
GeometryInfo<3>::child_cell_from_point (const Point<3> &p)
{
  Assert ((p[0] >= 0) && (p[0] <= 1), ExcInvalidCoordinate(p[0]));
  Assert ((p[1] >= 0) && (p[1] <= 1), ExcInvalidCoordinate(p[1]));
  Assert ((p[2] >= 0) && (p[2] <= 1), ExcInvalidCoordinate(p[2]));
  
  return (p[0] <= 0.5 ?
	  (p[1] <= 0.5 ?
	   (p[2] <= 0.5 ? 0 : 4) :
	   (p[2] <= 0.5 ? 2 : 6)) :
	  (p[1] <= 0.5 ?
	   (p[2] <= 0.5 ? 1 : 5) :
	   (p[2] <= 0.5 ? 3 : 7)));
}


template <int dim>
inline
unsigned int
GeometryInfo<dim>::child_cell_from_point (const Point<dim> &)
{
  Assert(false, ExcNotImplemented());

  return 0;
}



template <int dim>
inline
Point<dim>
GeometryInfo<dim>::cell_to_child_coordinates (const Point<dim>    &p,
					      const unsigned int child_index)
{
  Assert (child_index < GeometryInfo<dim>::children_per_cell,
	  ExcIndexRange (child_index, 0, GeometryInfo<dim>::children_per_cell));

  return 2*p - unit_cell_vertex(child_index);
}



template <int dim>
inline
Point<dim>
GeometryInfo<dim>::child_to_cell_coordinates (const Point<dim>    &p,
					      const unsigned int child_index)
{
  Assert (child_index < GeometryInfo<dim>::children_per_cell,
	  ExcIndexRange (child_index, 0, GeometryInfo<dim>::children_per_cell));

  return (p + unit_cell_vertex(child_index))/2;
}


template <>
inline
bool
GeometryInfo<1>::is_inside_unit_cell (const Point<1> &p)
{
  return (p[0] >= 0.) && (p[0] <= 1.);
}



template <>
inline
bool
GeometryInfo<2>::is_inside_unit_cell (const Point<2> &p)
{
  return (p[0] >= 0.) && (p[0] <= 1.) &&
	 (p[1] >= 0.) && (p[1] <= 1.);
}



template <>
inline
bool
GeometryInfo<3>::is_inside_unit_cell (const Point<3> &p)
{
  return (p[0] >= 0.) && (p[0] <= 1.) &&
	 (p[1] >= 0.) && (p[1] <= 1.) &&
	 (p[2] >= 0.) && (p[2] <= 1.);
}

#endif // DOXYGEN

#endif
