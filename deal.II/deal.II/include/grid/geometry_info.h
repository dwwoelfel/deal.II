//----------------------------  geometry_info.h  ---------------------------
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
//----------------------------  geometry_info.h  ---------------------------
#ifndef __deal2__geometry_info_h
#define __deal2__geometry_info_h


#include <base/config.h>
#include <base/exceptions.h>
#include <base/point.h>



/**
 * Topological description of cells. This general template is not
 * used, rather explicit specializations are provided for all
 * dimensions for which this class might be of use. The fields
 * provided by each of these specializations are identical, so that
 * access to them in a dimension independent way is possible.
 *
 * @author Wolfgang Bangerth, 1998
 */
template <int dim>
struct GeometryInfo
{};



/**
 * Topological description of zero dimensional cells,
 * i.e. points. This class might not look too useful but often is if
 * in a certain dimension we would like to enquire information about
 * objects with dimension one lower than the present, e.g. about
 * faces.
 *
 * This class contains as static members information on vertices and
 * faces of a @p{dim}-dimensional grid cell. The interface is the same
 * for all dimensions. If a value is of no use in a low dimensional
 * cell, it is (correctly) set to zero, e.g. @p{subfaces_per_cell} in
 * 1d.
 *
 * This information should always replace hard-coded numbers of
 * vertices, neighbors and so on, since it can be used dimension
 * independently.
 *
 * @author Wolfgang Bangerth, 1998
 */
template <>
struct GeometryInfo<0> 
{
				     /**
				      * Present dimension. Does not
				      * look useful, but might be.
				      */
    static const unsigned int dim               = 0;

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
				      * @p{for (i=0; i<vertices_per_face; ++i)},
				      * at least if @p{i} is an @p{unsigned int}.
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
 * Topological description of one dimensional cells.
 *
 * This class contains as static members information on vertices and
 * faces of a @p{dim}-dimensional grid cell. The interface is the same
 * for all dimensions. If a value is of no use in a low dimensional
 * cell, it is (correctly) set to zero, e.g. @p{subfaces_per_cell} in
 * 1d.
 *
 * This information should always replace hard-coded numbers of
 * vertices, neighbors and so on, since it can be used dimension
 * independently.
 *
 * @author Wolfgang Bangerth, 1998
 */
template <>
struct GeometryInfo<1> 
{
				     /**
				      * Present dimension. Does not
				      * look useful, but might be.
				      */
    static const unsigned int dim               = 1;

				     /**
				      * Number of children a cell has.
				      */
    static const unsigned int children_per_cell = 2;

				     /**
				      * Number of faces a cell has.
				      */
    static const unsigned int faces_per_cell    = 2;

				     /**
				      * Number of children each face has
				      * when the adjacent cell is refined.
				      */
    static const unsigned int subfaces_per_face = 0;

				     /**
				      * Number of vertices a cell has.
				      */
    static const unsigned int vertices_per_cell = 2;

				     /**
				      * Number of vertices each face has.
				      * Since this is not useful in one
				      * dimension, we provide a useless
				      * number (in the hope that a compiler
				      * may warn when it sees constructs like
				      * @p{for (i=0; i<vertices_per_face; ++i)},
				      * at least if @p{i} is an @p{unsigned int}.
				      */
    static const unsigned int vertices_per_face = 1;

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
    static const unsigned int lines_per_cell    = 1;

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

				     /**
				      * List of numbers which
				      * denotes which face is opposite
				      * to a given face. In 1d, this
				      * list is @p{{1,0}}, in 2d @p{{2, 3, 0, 1}},
				      * in 3d @p{{1, 0, 4, 5, 2, 3}}.
				      */
    static const unsigned int opposite_face[faces_per_cell];
    

				     /**
				      * Rearrange verices for OpenDX
				      * output.  For a cell being
				      * written in OpenDX format, each
				      * entry in this field contains
				      * the number of a vertex in
				      * @p{deal.II} that corresponds
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
				      * This field store which child cells
				      * are adjacent to a certain face of
				      * the mother cell.
				      *
				      * For example, in 2D the layout of
				      * a cell is as follows:
				      * @begin{verbatim}
				      * .      2
				      * .   3-->--2
				      * .   |     |
				      * . 3 ^     ^ 1
				      * .   |     |
				      * .   0-->--1
				      * .      0
				      * @end{verbatim}
				      * Vertices and faces are indicated
				      * with their numbers, faces also with
				      * their directions.
				      *
				      * Now, when refined, the layout is
				      * like this:
				      * @begin{verbatim}
				      * *--*--*
				      * | 3|2 |
				      * *--*--*
				      * | 0|1 |
				      * *--*--*
				      * @end{verbatim}
				      *
				      * Thus, the child cells on face zero
				      * are (ordered in the direction of the
				      * face) 0 and 1, on face 2 they are
				      * 3 and 2, etc.
				      *
				      * For three spatial dimensions,
				      * the exact order of the children is
				      * laid down in the documentation of
				      * the @ref{Triangulation} class.
				      */
    static unsigned int child_cell_on_face (const unsigned int face,
					    const unsigned int subface);

				     /**
				      * Return the position of the
				      * @p{i}th vertex on the unit
				      * cell. The order of vertices is
				      * the canonical one in deal.II,
				      * as described in the
				      * documentation of the
				      * @ref{Triangulation} class.
				      */
    static Point<1> unit_cell_vertex (const unsigned int vertex);

				     /**
				      * Report, for @p{vertex=0,1} the
				      * indices of the two vertices
				      * adjacent to the line with
				      * index @p{line} among the lines
				      * forming this cell. In 1d, the
				      * only line is the cell itself,
				      * while in 2d and 3d there are 4
				      * and 12 lines, respectively.
				      *
				      * The positions of these
				      * vertices in the unit cell can
				      * be obtained using the
				      * @p{unit_cell_vertex} function.
				      *
				      * The order of the lines, as
				      * well as their direction (which
				      * in turn determines which is
				      * the first and which the second
				      * vertex on a line) is the
				      * canonical one in deal.II, as
				      * described in the documentation
				      * of the @ref{Triangulation}
				      * class.
				      */
    static unsigned int vertices_adjacent_to_line (const unsigned int line,
						   const unsigned int vertex);
};



/**
 * Topological description of two dimensional cells.
 *
 * This class contains as static members information on vertices and
 * faces of a @p{dim}-dimensional grid cell. The interface is the same
 * for all dimensions. If a value is of no use in a low dimensional
 * cell, it is (correctly) set to zero, e.g. @p{subfaces_per_cell} in
 * 1d.
 *
 * This information should always replace hard-coded numbers of
 * vertices, neighbors and so on, since it can be used dimension
 * independently.
 *
 * @author Wolfgang Bangerth, 1998
 */
template <>
struct GeometryInfo<2>
{
				     /**
				      * Present dimension. Does not
				      * look useful, but might be.
				      */
    static const unsigned int dim               = 2;

				     /**
				      * Number of children a cell has.
				      */
    static const unsigned int children_per_cell = 4;

				     /**
				      * Number of faces a cell has.
				      */
    static const unsigned int faces_per_cell    = 4;

				     /**
				      * Number of children each face has
				      * when the adjacent cell is refined.
				      */
    static const unsigned int subfaces_per_face = 2;

				     /**
				      * Number of vertices a cell has.
				      */
    static const unsigned int vertices_per_cell = 4;

				     /**
				      * Number of vertices each face has.
				      * Since this is not useful in one
				      * dimension, we provide a useless
				      * number (in the hope that a compiler
				      * may warn when it sees constructs like
				      * @p{for (i=0; i<vertices_per_face; ++i)},
				      * at least if @p{i} is an @p{unsigned int}.
				      */
    static const unsigned int vertices_per_face = 2;

				     /**
				      * Number of lines each face has.
				      */
    static const unsigned int lines_per_face    = 1;
    
				     /**
				      * Number of quads on each face.
				      */
    static const unsigned int quads_per_face    = 0;

				     /**
				      * Number of lines of a cell.
				      */
    static const unsigned int lines_per_cell    = 4;

				     /**
				      * Number of quadrilaterals of a
				      * cell.
				      */
    static const unsigned int quads_per_cell    = 1;

				     /**
				      * Number of hexahedra of a
				      * cell.
				      */
    static const unsigned int hexes_per_cell    = 0;

				     /**
				      * List of numbers which
				      * denotes which face is opposite
				      * to a given face. In 1d, this
				      * list is @p{{1,0}}, in 2d @p{{2, 3, 0, 1}},
				      * in 3d @p{{1, 0, 4, 5, 2, 3}}.
				      */
    static const unsigned int opposite_face[faces_per_cell];
    
				     /**
				      * Rearrange verices for OpenDX
				      * output.  For a cell being
				      * written in OpenDX format, each
				      * entry in this field contains
				      * the number of a vertex in
				      * @p{deal.II} that corresponds
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
				      * This field store which child cells
				      * are adjacent to a certain face of
				      * the mother cell.
				      *
				      * For example, in 2D the layout of
				      * a cell is as follows:
				      * @begin{verbatim}
				      * .      2
				      * .   3-->--2
				      * .   |     |
				      * . 3 ^     ^ 1
				      * .   |     |
				      * .   0-->--1
				      * .      0
				      * @end{verbatim}
				      * Vertices and faces are indicated
				      * with their numbers, faces also with
				      * their directions.
				      *
				      * Now, when refined, the layout is
				      * like this:
				      * @begin{verbatim}
				      * *--*--*
				      * | 3|2 |
				      * *--*--*
				      * | 0|1 |
				      * *--*--*
				      * @end{verbatim}
				      *
				      * Thus, the child cells on face zero
				      * are (ordered in the direction of the
				      * face) 0 and 1, on face 2 they are
				      * 3 and 2, etc.
				      *
				      * For three spatial dimensions,
				      * the exact order of the children is
				      * laid down in the documentation of
				      * the @ref{Triangulation} class.
				      */
    static unsigned int child_cell_on_face (const unsigned int face,
					    const unsigned int subface);


				     /**
				      * Return the position of the
				      * @p{i}th vertex on the unit
				      * cell. The order of vertices is
				      * the canonical one in deal.II,
				      * as described in the
				      * documentation of the
				      * @ref{Triangulation} class.
				      */
    static Point<2> unit_cell_vertex (const unsigned int i);

				     /**
				      * Report, for @p{vertex=0,1} the
				      * indices of the two vertices
				      * adjacent to the line with
				      * index @p{line} among the lines
				      * forming this cell. In 1d, the
				      * only line is the cell itself,
				      * while in 2d and 3d there are 4
				      * and 12 lines, respectively.
				      *
				      * The positions of these
				      * vertices in the unit cell can
				      * be obtained using the
				      * @p{unit_cell_vertex} function.
				      *
				      * The order of the lines, as
				      * well as their direction (which
				      * in turn determines which is
				      * the first and which the second
				      * vertex on a line) is the
				      * canonical one in deal.II, as
				      * described in the documentation
				      * of the @ref{Triangulation}
				      * class.
				      */
    static unsigned int vertices_adjacent_to_line (const unsigned int line,
						   const unsigned int vertex);
};



/**
 * Topological description of three dimensional cells.
 *
 * This class contains as static members information on vertices and
 * faces of a @p{dim}-dimensional grid cell. The interface is the same
 * for all dimensions. If a value is of no use in a low dimensional
 * cell, it is (correctly) set to zero, e.g. @p{subfaces_per_cell} in
 * 1d.
 *
 * This information should always replace hard-coded numbers of
 * vertices, neighbors and so on, since it can be used dimension
 * independently.
 *
 * @author Wolfgang Bangerth, 1998
 */
template <>
struct GeometryInfo<3> 
{
				     /**
				      * Present dimension. Does not
				      * look useful, but might be.
				      */
    static const unsigned int dim               = 3;

				     /**
				      * Number of children a cell has.
				      */
    static const unsigned int children_per_cell = 8;

				     /**
				      * Number of faces a cell has.
				      */
    static const unsigned int faces_per_cell    = 6;

				     /**
				      * Number of children each face has
				      * when the adjacent cell is refined.
				      */
    static const unsigned int subfaces_per_face = 4;

				     /**
				      * Number of vertices a cell has.
				      */
    static const unsigned int vertices_per_cell = 8;

				     /**
				      * Number of vertices each face has.
				      * Since this is not useful in one
				      * dimension, we provide a useless
				      * number (in the hope that a compiler
				      * may warn when it sees constructs like
				      * @p{for (i=0; i<vertices_per_face; ++i)},
				      * at least if @p{i} is an @p{unsigned int}.
				      */
    static const unsigned int vertices_per_face = 4;

				     /**
				      * Number of lines each face has.
				      */
    static const unsigned int lines_per_face    = 4;
    
				     /**
				      * Number of quads on each face.
				      */
    static const unsigned int quads_per_face    = 1;

				     /**
				      * Number of lines of a cell.
				      */
    static const unsigned int lines_per_cell    = 12;

				     /**
				      * Number of quadrilaterals of a
				      * cell.
				      */
    static const unsigned int quads_per_cell    = 6;

				     /**
				      * Number of hexahedra of a
				      * cell.
				      */
    static const unsigned int hexes_per_cell    = 1;

				     /**
				      * List of numbers which
				      * denotes which face is opposite
				      * to a given face. In 1d, this
				      * list is @p{{1,0}}, in 2d @p{{2, 3, 0, 1}},
				      * in 3d @p{{1, 0, 4, 5, 2, 3}}.
				      */
    static const unsigned int opposite_face[faces_per_cell];
    
				     /**
				      * Rearrange verices for OpenDX
				      * output.  For a cell being
				      * written in OpenDX format, each
				      * entry in this field contains
				      * the number of a vertex in
				      * @p{deal.II} that corresponds
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
				      * This field store which child cells
				      * are adjacent to a certain face of
				      * the mother cell.
				      *
				      * For example, in 2D the layout of
				      * a cell is as follows:
				      * @begin{verbatim}
				      * .      2
				      * .   3-->--2
				      * .   |     |
				      * . 3 ^     ^ 1
				      * .   |     |
				      * .   0-->--1
				      * .      0
				      * @end{verbatim}
				      * Vertices and faces are indicated
				      * with their numbers, faces also with
				      * their directions.
				      *
				      * Now, when refined, the layout is
				      * like this:
				      * @begin{verbatim}
				      * *--*--*
				      * | 3|2 |
				      * *--*--*
				      * | 0|1 |
				      * *--*--*
				      * @end{verbatim}
				      *
				      * Thus, the child cells on face zero
				      * are (ordered in the direction of the
				      * face) 0 and 1, on face 2 they are
				      * 3 and 2, etc.
				      *
				      * For three spatial dimensions,
				      * the exact order of the children is
				      * laid down in the documentation of
				      * the @ref{Triangulation} class.
				      */
    static unsigned int child_cell_on_face (const unsigned int face,
					    const unsigned int subface);
				     /**
				      * Return the position of the
				      * @p{i}th vertex on the unit
				      * cell. The order of vertices is
				      * the canonical one in deal.II,
				      * as described in the
				      * documentation of the
				      * @ref{Triangulation} class.
				      */
    static Point<3> unit_cell_vertex (const unsigned int i);

				     /**
				      * Report, for @p{vertex=0,1} the
				      * indices of the two vertices
				      * adjacent to the line with
				      * index @p{line} among the lines
				      * forming this cell. In 1d, the
				      * only line is the cell itself,
				      * while in 2d and 3d there are 4
				      * and 12 lines, respectively.
				      *
				      * The positions of these
				      * vertices in the unit cell can
				      * be obtained using the
				      * @p{unit_cell_vertex} function.
				      *
				      * The order of the lines, as
				      * well as their direction (which
				      * in turn determines which is
				      * the first and which the second
				      * vertex on a line) is the
				      * canonical one in deal.II, as
				      * described in the documentation
				      * of the @ref{Triangulation}
				      * class.
				      */
    static unsigned int vertices_adjacent_to_line (const unsigned int line,
						   const unsigned int vertex);
};



/**
 * Topological description of four dimensional cells. This class is
 * required in some exotic cases where we compute information in a
 * one-larger dimension than the present, and do so also in 3d (for
 * example, stacking the solutions of a d-dimensional time dependent
 * equation timestep after timestep in a (d+1)-dimensional space).
 *
 * This class contains as static members information on vertices and
 * faces of a @p{dim}-dimensional grid cell. The interface is the same
 * for all dimensions. If a value is of no use in a low dimensional
 * cell, it is (correctly) set to zero, e.g. @p{subfaces_per_cell} in
 * 1d.
 *
 * This information should always replace hard-coded numbers of
 * vertices, neighbors and so on, since it can be used dimension
 * independently.
 *
 * @author Wolfgang Bangerth, 1998
 */
template <>
struct GeometryInfo<4>
{
				     /**
				      * Present dimension. Does not
				      * look useful, but might be.
				      */
    static const unsigned int dim               = 4;

				     /**
				      * Number of children a cell has.
				      */
    static const unsigned int children_per_cell = 16;

				     /**
				      * Number of faces a cell has.
				      */
    static const unsigned int faces_per_cell    = 8;

				     /**
				      * Number of children each face has
				      * when the adjacent cell is refined.
				      */
    static const unsigned int subfaces_per_face = 8;

				     /**
				      * Number of vertices a cell has.
				      */
    static const unsigned int vertices_per_cell = 16;

				     /**
				      * Number of vertices each face has.
				      * Since this is not useful in one
				      * dimension, we provide a useless
				      * number (in the hope that a compiler
				      * may warn when it sees constructs like
				      * @p{for (i=0; i<vertices_per_face; ++i)},
				      * at least if @p{i} is an @p{unsigned int}.
				      */
    static const unsigned int vertices_per_face = 8;

				     /**
				      * Number of lines each face has.
				      */
    static const unsigned int lines_per_face    = 12;
    
				     /**
				      * Number of quads on each face.
				      */
    static const unsigned int quads_per_face    = 6;

				     /**
				      * Number of lines of a cell.
				      */
    static const unsigned int lines_per_cell    = 32;

				     /**
				      * Number of quadrilaterals of a
				      * cell.
				      */
    static const unsigned int quads_per_cell    = 24;

				     /**
				      * Number of hexahedra of a
				      * cell.
				      */
    static const unsigned int hexes_per_cell    = 8;
};


/* ---------------- inline functions --------------- */

inline
Point<1>
GeometryInfo<1>::unit_cell_vertex (const unsigned int vertex)
{
  Assert (vertex < vertices_per_cell,
	  ExcIndexRange (vertex, 0, vertices_per_cell));

  const Point<dim> vertices[vertices_per_cell] =
    { Point<dim>(0.), Point<dim>(1.) };
  return vertices[vertex];
};



inline
Point<2>
GeometryInfo<2>::unit_cell_vertex (const unsigned int vertex)
{
  Assert (vertex < vertices_per_cell,
	  ExcIndexRange (vertex, 0, vertices_per_cell));

  const Point<dim> vertices[vertices_per_cell] =
    { Point<dim>(0., 0.), Point<dim>(1., 0.),
      Point<dim>(1.,1.), Point<dim>(0.,1.) };
  return vertices[vertex];
};



inline
Point<3>
GeometryInfo<3>::unit_cell_vertex (const unsigned int vertex)
{
  Assert (vertex < vertices_per_cell,
	  ExcIndexRange (vertex, 0, vertices_per_cell));

  const Point<dim> vertices[vertices_per_cell] =
    { Point<dim>(0., 0., 0.), Point<dim>(1., 0., 0.),
      Point<dim>(1., 0., 1.), Point<dim>(0., 0., 1.),
      Point<dim>(0., 1., 0.), Point<dim>(1., 1., 0.),
      Point<dim>(1., 1., 1.), Point<dim>(0., 1., 1.) };
  return vertices[vertex];
};



inline
unsigned int
GeometryInfo<1>::vertices_adjacent_to_line (const unsigned int line,
					    const unsigned int vertex)
{
  Assert (line < lines_per_cell,
	  ExcIndexRange (line, 0, lines_per_cell));
  Assert (vertex < vertices_per_cell,
	  ExcIndexRange (vertex, 0, 2));

  return vertex;
};



inline
unsigned int
GeometryInfo<2>::vertices_adjacent_to_line (const unsigned int line,
					    const unsigned int vertex)
{
  Assert (line < lines_per_cell,
	  ExcIndexRange (line, 0, lines_per_cell));
  Assert (vertex < vertices_per_cell,
	  ExcIndexRange (vertex, 0, 2));

  const unsigned int vertex_indices[lines_per_cell][2] =
    { {0, 1}, {1, 2}, {3, 2}, {0, 3} };

  return vertex_indices[line][vertex];
};



inline
unsigned int
GeometryInfo<3>::vertices_adjacent_to_line (const unsigned int line,
					    const unsigned int vertex)
{
  Assert (line < lines_per_cell,
	  ExcIndexRange (line, 0, lines_per_cell));
  Assert (vertex < vertices_per_cell,
	  ExcIndexRange (vertex, 0, 2));

  const unsigned int vertex_indices[lines_per_cell][2] =
    { {0, 1}, {1, 2}, {3, 2}, {0, 3},
      {4, 5}, {5, 6}, {7, 6}, {4, 7},
      {0, 4}, {1, 5}, {2, 6}, {3, 7}  };

  return vertex_indices[line][vertex];
};




#endif
