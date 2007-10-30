//-------------------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 2006, 2007 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//-------------------------------------------------------------------------

/**
 * @mainpage
 *
 * @image html logo200.png
 *
 * This is the main page for class and function documentation in
 * deal.II. Documentation on other aspects of deal.II can be found
 * elsewhere. In particular, there are
 * <a href="../tutorial/index.html">tutorial programs on the use of
 * this library</a>.
 *
 * Many of the classes in the deal.II library can be grouped into
 * modules (see the <a href="modules.html">Modules page</a> or the
 * corresponding entry in the menu at the top of this page). These
 * modules follow roughly the following collaboration diagram that
 * finite element programs follow:
 *
 * @image html collaboration.png "Collaboration diagram for the most important groups of classes in deal.II"
 * @image latex collaboration.eps "Collaboration diagram for the most important groups of classes in deal.II" width=.9\textwidth
 *
 * Here is a guide to this classification of groups, as well as links
 * to the documentation pertaining to each of them:
 *
 * <ol>
 * 
 *   <li> <b>Unit cell</b>: deal.II supports only hypercubes as unit
 *   cells, i.e. the unit cell $[0,1]$ in 1d, the unit square $[0,1]^2$ in
 *   2d, and the unit cube $[0,1]^3$ in 3d. We do not support triangles,
 *   tetrahedra, pyramids, or prisms.
 *
 *   Inside the library, most properties of unit cells such as the
 *   number of vertices per cell, ordering of faces, or direction of
 *   edges, are explicitly described in the GeometryInfo class. This
 *   avoids the use of implicit assumptions in many places of the
 *   code.
 *
 *
 *   <li> <b>%Triangulation</b>: Triangulations are collections of
 *   cells that have the general shape derived from the unit cell. The
 *   only thing a triangulation stores are the geometric and topologic
 *   properties of a mesh: where are vertices located, and how are
 *   these vertices connected to cells. A triangulation doesn't know
 *   anything about the finite elements that you may want to used on
 *   this mesh, and a triangulation does not even know anything about
 *   the shape of its cells: in 2d it only knows that a cell has four
 *   vertices (and in 3d that it has 8 vertices), but there are no
 *   provisions that the mapping from the unit cell to a particular
 *   cell in the triangulation is well-behaved in any way: it really
 *   only knows about the geometric location of vertices and their
 *   topological connection.
 *
 *   The properties and data of triangulations are almost always
 *   queried through loops over all cells, or all faces of cells. Most
 *   of the knowledge about a mesh is therefore hidden behind
 *   @em iterators, i.e. pointer-like structures that one can
 *   iterate from one cell to the next, and that one can ask for
 *   information about the cell it presently points to.
 *
 *   The classes that describe triangulations and cells are located
 *   and documented in the @ref grid module. Iterators are described
 *   in the @ref Iterators module.
 *
 *
 *   <li> <b>Finite Element</b>: Finite element classes describe the
 *   properties of a finite element space as defined on the unit
 *   cell. This includes, for example, how many degrees of freedom are
 *   located at vertices, on lines, or in the interior of cells. In
 *   addition to this, finite element classes of course have to
 *   provide values and gradients of individual shape functions at
 *   points on the unit cell.
 *
 *   The finite element classes are described in the @ref feall module.
 *
 *
 *   <li> <b>%Quadrature</b>: As with finite elements, quadrature
 *   objects are defined on the unit cell. They only describe the
 *   location of quadrature points on the unit cell, and the weights
 *   of quadrature points thereon.
 *
 *   The documentation of the classes describing particular quadrature
 *   formulas is found in the @ref Quadrature module.
 *
 *
 *   <li> <b>%DoFHandler</b>: %DoFHandler objects are the confluence
 *   of triangulations and finite elements: the finite element class
 *   describes how many degrees of freedom it needs per vertex, line,
 *   or cell, and the DoFHandler class allocates this space so that
 *   each vertex, line, or cell of the triangulation has the correct
 *   number of them. It also gives them a global numbering.
 *
 *   Just as with triangulation objects, most operations on
 *   DoFHandlers is done by looping over all cells and doing something
 *   on each or a subset of them. The interfaces of the two classes are
 *   therefore rather similar: they allow to get iterators to the
 *   first and last cell (or face, or line, etc) and offer information
 *   through these iterators. The information that can be gotten from
 *   these iterators is the geometric and topological information that
 *   can already be gotten from the triangulation iterators (they are
 *   in fact derived classes) as well as things like the global
 *   numbers of the degrees of freedom on the present cell. On can
 *   also ask an iterator to extract the values corresponding to the
 *   degrees of freedom on the present cell from a data vector that
 *   stores values for all degrees of freedom associated with a
 *   triangulation.
 *
 *   It is worth noting that, just as triangulations, DoFHandler
 *   classes do not know anything about the mapping from the unit cell
 *   to its individual cells. It is also ignorant of the shape
 *   functions that correspond to the degrees of freedom it manages:
 *   all it knows is that there are, for example, 2 degrees of freedom
 *   for each vertex and 4 per cell interior. Nothing about their
 *   specifics is relevant to the DoFHandler class.
 *
 *   The DoFHandler class and its associates are described in the @ref
 *   dofs module. In addition, there are specialized versions that can
 *   handle multilevel and hp discretizations. These are described in
 *   the @ref mg and @ref hp modules.
 *
 *
 *   <li> <b>%Mapping</b>: The next step in a finite element program
 *   is that one would want to compute matrix and right hand side
 *   entries or other quantities on each cell of a triangulation,
 *   using the shape functions of a finite element and quadrature
 *   points defined by a quadrature rule. To this end, it is necessary
 *   to map the shape functions, quadrature points, and quadrature
 *   weights from the unit cell to each cell of a triangulation. This
 *   is not directly done by, but facilitated by the Mapping and
 *   derived classes: they describe how to map points from unit to
 *   real space and back, as well as provide gradients of this
 *   derivative and Jacobian determinants.
 *
 *   These classes are all described in the @ref mapping module.
 *
 *
 *   <li> <b>%FEValues</b>: The next step is to actually take a finite
 *   element and evaluate its shape functions and their gradients at
 *   the points defined by a quadrature formula when mapped to the
 *   real cell. This is the realm of the FEValues class and siblings:
 *   in a sense, they offer a point-wise view of the finite element
 *   function space.
 *
 *   This seems restrictive: in mathematical analysis, we always write
 *   our formulas in terms of integrals over cells, or faces of cells,
 *   involving the finite element shape functions. One would therefore
 *   think that it is necessary to describe finite element spaces as
 *   continuous spaces. However, in practice, this is not necessary:
 *   all integrals are in actual computations replaced by
 *   approximations using quadrature formula, and what is therefore
 *   really only necessary is the ability to evaluate shape functions
 *   at a finite number of given locations inside a domain. The
 *   FEValues classes offer exactly this information: Given finite
 *   element, quadrature, and mapping objects, they compute the
 *   restriction of a continuous function space (as opposed to
 *   discrete, not as opposed to discontinuous) to a discrete number
 *   of points.
 *
 *   There are a number of objects that can do this: FEValues for
 *   evaluation on cells, FEFaceValues for evaluation on faces of
 *   cells, and FESubfaceValues for evaluation on parts of faces of
 *   cells. All these classes are described in the @ref feaccess
 *   module.
 *
 *
 *   <li> <b>Linear System</b>: If one knows how to evaluate the
 *   values and gradients of shape functions on individual cells using
 *   FEValues and friends, and knows how to get the global numbers of
 *   the degrees of freedom on a cell using the DoFHandler iterators,
 *   then the next step is to use the bilinear form of the problem to
 *   assemble the system matrix (and right hand side) of the linear
 *   system. We will then determine the solution of our problem from
 *   this linear system.
 *
 *   To do this, we need to have classes that store and manage the
 *   entries of matrices and vectors. deal.II comes with a whole set
 *   of classes for this purpose, as well as with interfaces to other
 *   software packages that offer similar functionality. Documentation
 *   to this end can be found in the @ref LAC module.
 *
 *
 *   <li> <b>Linear %Solver</b>: In order to determine the solution of
 *   a finite-dimensional, linear system of equations, one needs
 *   linear solvers. In finite element applications, they are
 *   frequently iterative, but sometimes one may also want to use
 *   direct or sparse direct solvers. deal.II has quite a number of
 *   these. They are documented in the @ref Solvers module.
 *
 *
 *   <li> <b>Output</b>: Finally, once one has obtained a solution of
 *   a finite element problem on a given triangulation, one will often
 *   want to postprocess it using a visualization program. This
 *   library doesn't do that by itself, but rather generates output
 *   files in a variety of graphics formats understood by widely
 *   available visualization tools.
 *
 *   A description of the classes that do so is given in the @ref
 *   output module.
 *
 * </ol>
 *
 * In addition, deal.II has a number of groups of classes that go
 * beyond the ones listed here. They pertain to more refined concepts
 * of the hierarchy presented above, or to tangential aspects like
 * handling of input and output that are not necessarily specific to
 * finite element programs, but appear there as well. These classes
 * are all listed in the Classes and Namespaces views reachable from
 * the menu bar at the top of this page, and are also grouped into
 * modules of their own (see the Modules view also in the menu bar).
 */
