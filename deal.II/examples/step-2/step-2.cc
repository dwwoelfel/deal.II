/* $Id$ */
/* Author: Wolfgang Bangerth, University of Heidelberg, 1999 */

/*    $Id$       */
/*    Version: $Name$                                          */
/*                                                                */
/*    Copyright (C) 1999, 2000, 2001, 2002, 2003, 2006 by the deal.II authors */
/*                                                                */
/*    This file is subject to QPL and may not be  distributed     */
/*    without copyright and license information. Please refer     */
/*    to the file deal.II/doc/license.html for the  text  and     */
/*    further information on this license.                        */

				 // The first few includes are just
				 // like in the previous program, so
				 // do not require additional comments:
#include <grid/tria.h>
#include <grid/tria_accessor.h>
#include <grid/tria_iterator.h>
#include <grid/grid_generator.h>
#include <grid/tria_boundary_lib.h>

				 // However, the next file is new. We need
				 // this include file for the association of
				 // degrees of freedom ("DoF"s) to vertices,
				 // lines, and cells:
#include <dofs/dof_handler.h>

				 // The following include contains the
				 // description of the bilinear finite
				 // element, including the facts that
				 // it has one degree of freedom on
				 // each vertex of the triangulation,
				 // but none on faces and none in the
				 // interior of the cells.
				 //
				 // (In fact, the file contains the
				 // description of Lagrange elements in
				 // general, i.e. also the quadratic, cubic,
				 // etc versions, and not only for 2d but also
				 // 1d and 3d.)
#include <fe/fe_q.h>
				 // In the following file, several
				 // tools for manipulating degrees of
				 // freedom can be found:
#include <dofs/dof_tools.h>
				 // We will use a sparse matrix to
				 // visualize the pattern of nonzero
				 // entries resulting from the
				 // distribution of degrees of freedom
				 // on the grid. That class can be
				 // found here:
#include <lac/sparse_matrix.h>
				 // We will want to use a special
				 // algorithm to renumber degrees of
				 // freedom. It is declared here:
#include <dofs/dof_renumbering.h>

				 // And this is again needed for C++ output:
#include <fstream>


                                 // @sect3{Mesh generation}

				 // This is the function that produced the
				 // circular grid in the previous step-1
				 // example program. The sole difference is
				 // that it returns the grid it produces via
				 // its argument.
				 //
				 // The details of what the function does are
				 // explained in step-1. The only thing we
				 // would like to comment on is this:
				 // 
                                 // Since we want to export the triangulation
                                 // through this function's parameter, we need
                                 // to make sure that the boundary object
                                 // lives at least as long as the
                                 // triangulation does. However, in step-1,
                                 // the boundary object is a local variable,
                                 // and it would be deleted at the end of the
                                 // function, which is too early. We avoid the
                                 // problem by declaring it 'static' which
                                 // makes sure that the object is initialized
                                 // the first time control flow passes its
                                 // point of declaration, but at the same time
                                 // assures that it lives until the end of the
                                 // program.
void make_grid (Triangulation<2> &triangulation)
{
  const Point<2> center (1,0);
  const double inner_radius = 0.5,
	       outer_radius = 1.0;
  GridGenerator::hyper_shell (triangulation,
			      center, inner_radius, outer_radius,
                              10);

  static const HyperShellBoundary<2> boundary_description(center);
  triangulation.set_boundary (0, boundary_description);
  
  for (unsigned int step=0; step<5; ++step)
    {
      Triangulation<2>::active_cell_iterator
	cell = triangulation.begin_active(),
	endc = triangulation.end();

      for (; cell!=endc; ++cell)
	for (unsigned int vertex=0;
	     vertex < GeometryInfo<2>::vertices_per_cell;
	     ++vertex)
	  {
            const double distance_from_center
              = center.distance (cell->vertex(vertex));
	    
	    if (std::fabs(distance_from_center - inner_radius) < 1e-10)
	      {
		cell->set_refine_flag ();
		break;
	      }
	  }

      triangulation.execute_coarsening_and_refinement ();
    }
}

                                 // @sect3{Creation of a DoFHandler}

				 // Up to now, we only have a grid, i.e. some
				 // geometrical (the position of the vertices)
				 // and some topological information (how
				 // vertices are connected to lines, and lines
				 // to cells, as well as which cells neighbor
				 // which other cells). To use numerical
				 // algorithms, one needs some logic
				 // information in addition to that: we would
				 // like to associate degree of freedom
				 // numbers to each vertex (or line, or cell,
				 // in case we were using higher order
				 // elements) to later generate matrices and
				 // vectors which describe a finite element
				 // field on the triangulation.
                                 //
                                 // This function shows how to do this. The
                                 // object to consider is the ``DoFHandler''
                                 // class template.  Before we do so, however,
                                 // we first need something that describes how
                                 // many degrees of freedom are to be
                                 // associated to each of these objects. Since
                                 // this is one aspect of the definition of a
                                 // finite element space, the finite element
                                 // base class stores this information. In the
                                 // present context, we therefore create an
                                 // object of the derived class ``FE_Q'' that
                                 // describes Lagrange elements. Its
                                 // constructor takes one argument that states
                                 // the polynomial degree of the element,
                                 // which here is one (indicating a bi-linear
                                 // element); this then corresponds to one
                                 // degree of freedom for each vertex, while
                                 // there are none on lines and inside the
                                 // quadrilateral. A value of, say, three
                                 // given to the constructor would instead
                                 // give us a bi-cubic element with one degree
                                 // of freedom per vertex, two per line, and
                                 // four inside the cell. In general, ``FE_Q''
                                 // denotes the family of continuous elements
                                 // with complete polynomials
                                 // (i.e. tensor-product polynomials) up to
                                 // the specified order.
                                 //
                                 // We first need to create an object of this
                                 // class and then pass it on to the
                                 // ``DoFHandler'' object to allocate storage
                                 // for the degrees of freedom (in deal.II
                                 // lingo: we ``distribute degrees of
                                 // freedom''). Note that the DoFHandler
                                 // object will store a reference to this
                                 // finite element object, so we need have to
                                 // make sure its lifetime is at least as long
                                 // as that of the ``DoFHandler''; one way to
                                 // make sure this is so is to make it static
                                 // as well, in order to prevent its
                                 // preemptive destruction. (However, the
                                 // library would warn us if we forgot about
                                 // this and abort the program if that
                                 // occured. You can check this, if you want,
                                 // by removing the 'static' declaration.)
void distribute_dofs (DoFHandler<2> &dof_handler) 
{
                                   // As described above, let us first create
                                   // a finite element object, and then use it
                                   // to allocate degrees of freedom on the
                                   // triangulation with which the dof_handler
                                   // object is associated:
  static const FE_Q<2> finite_element(1);
  dof_handler.distribute_dofs (finite_element);

				   // Now that we have associated a degree of
				   // freedom with a global number to each
				   // vertex, we wonder how to visualize this?
				   // Unfortunately, presently no way is
				   // implemented to directly show the DoF
				   // number associated with each
				   // vertex. However, such information would
				   // hardly ever be truly important, since
				   // the numbering itself is more or less
				   // arbitrary. There are more important
				   // factors, of which we will visualize one
				   // in the following.
				   //
				   // Associated with each vertex of the
				   // triangulation is a shape
				   // function. Assume we want to solve
				   // something like Laplace's equation, then
				   // the different matrix entries will be the
				   // integrals over the gradient of each pair
				   // of such shape functions. Obviously,
				   // since the shape functions are nonzero
				   // only on the cells adjacent to the vertex
				   // they are associated to, matrix entries
				   // will be nonzero only if the supports of
				   // the shape functions associated to that
				   // column and row numbers intersect. This
				   // is only the case for adjacent shape
				   // functions, and therefore only for
				   // adjacent vertices. Now, since the
				   // vertices are numbered more or less
				   // randomly by the above function
				   // (DoFHandler::distribute_dofs), the
				   // pattern of nonzero entries in the matrix
				   // will be somewhat ragged, and we will
				   // take a look at it now.
				   //
				   // First we have to create a
				   // structure which we use to store
				   // the places of nonzero
				   // elements. We have to give it the
				   // size of the matrix, which in our
				   // case will be square with as
				   // many rows and columns as there
				   // are degrees of freedom on the
				   // grid:
  SparsityPattern sparsity_pattern (dof_handler.n_dofs(),
				    dof_handler.n_dofs(),
                                    20);
                                   // The last argument to the constructor
                                   // indicates the maximum number of entries
                                   // we expect per row. If this were a
                                   // uniformly refined square, then we know
                                   // that each vertex degree of freedom would
                                   // couple with itself and the eight degrees
                                   // of freedom around it. However, our mesh
                                   // is more complicated, and it may well be
                                   // that more couplings will occur. The
                                   // value 20 we use here is on the safe side
                                   // of that, though it may actually be too
                                   // large. In the step-3 tutorial program,
                                   // we will see a way to compute a
                                   // reasonable upper bound to the number of
                                   // nonzero entries, and later programs will
                                   // show several other methods to compute
                                   // the numbers of zeros per row.

				   // We then fill this object with the
				   // places where nonzero elements will be
				   // located given the present numbering of
				   // degrees of freedom:
  DoFTools::make_sparsity_pattern (dof_handler, sparsity_pattern);
				   // Before further work can be done
				   // on the object, we have to allow
				   // for some internal
				   // reorganization:
  sparsity_pattern.compress ();
                                   // What actually happens in this call is
                                   // the following: upon creation of a
                                   // ``SparsityPattern'' object, memory is
                                   // allocated for a maximum number of
                                   // entries per row (20 in our case). The
                                   // call to
                                   // ``DoFTools::make_sparsity_pattern'' then
                                   // actually allocates entries as necessary,
                                   // but the number of nonzero entries in any
                                   // given row may be less than the 20 we
                                   // have allocated memory for. To save
                                   // memory and to simplify some other
                                   // operations, one then needs to
                                   // ``compress'' the sparsity pattern before
                                   // anything else.

				   // With this, we can now write the results
				   // to a file:
  std::ofstream out ("sparsity_pattern.1");
  sparsity_pattern.print_gnuplot (out);
				   // The result is in GNUPLOT format,
				   // where in each line of the output
				   // file, the coordinates of one
				   // nonzero entry are listed. The
				   // output will be shown below.
				   //
				   // If you look at it, you will note that
				   // the sparsity pattern is symmetric. This
				   // should not come as a surprise, since we
				   // have not given the
				   // ``DoFTools::make_sparsity_pattern'' any
				   // information that would indicate that our
				   // bilinear form may couple shape functions
				   // in a non-symmetric way. You will also
				   // note that it has several distinct
				   // region, which stem from the fact that
				   // the numbering starts from the coarsest
				   // cells and moves on to the finer ones;
				   // since they are all distributed
				   // symmetrically around the origin, this
				   // shows up again in the sparsity pattern.
}


                                 // @sect3{Renumbering of DoFs}

				 // In the sparsity pattern produced
				 // above, the nonzero entries
				 // extended quite far off from the
				 // diagonal. For some algorithms,
				 // this is unfavorable, and we will
				 // show a simple way how to improve
				 // this situation.
				 //
				 // Remember that for an entry (i,j)
				 // in the matrix to be nonzero, the
				 // supports of the shape functions i
				 // and j needed to intersect
				 // (otherwise in the integral, the
				 // integrand would be zero everywhere
				 // since either the one or the other
				 // shape function is zero at some
				 // point). However, the supports of
				 // shape functions intersected only
				 // of they were adjacent to each
				 // other, so in order to have the
				 // nonzero entries clustered around
				 // the diagonal (where i equals j),
				 // we would like to have adjacent
				 // shape functions to be numbered
				 // with indices (DoF numbers) that
				 // differ not too much.
				 //
				 // This can be accomplished by a
				 // simple front marching algorithm,
				 // where one starts at a given vertex
				 // and gives it the index zero. Then,
				 // its neighbors are numbered
				 // successively, making their indices
				 // close to the original one. Then,
				 // their neighbors, if not yet
				 // numbered, are numbered, and so
				 // on.
				 //
				 // One algorithm that adds a little bit of
				 // sophistication along these lines is the
				 // one by Cuthill and McKee. We will use it
				 // in the following function to renumber the
				 // degrees of freedom such that the resulting
				 // sparsity pattern is more localized around
				 // the diagonal. The only interesting part of
				 // the function is the first call to
				 // ``DoFRenumbering::Cuthill_McKee'', the
				 // rest is essentially as before:
void renumber_dofs (DoFHandler<2> &dof_handler) 
{
  DoFRenumbering::Cuthill_McKee (dof_handler);
  SparsityPattern sparsity_pattern (dof_handler.n_dofs(),
				    dof_handler.n_dofs());

  DoFTools::make_sparsity_pattern (dof_handler, sparsity_pattern);
  sparsity_pattern.compress ();

  std::ofstream out ("sparsity_pattern.2");
  sparsity_pattern.print_gnuplot (out);
}

                                 // Again, the output is shown
                                 // below. Note that the nonzero
                                 // entries are clustered far better
                                 // around the diagonal than
                                 // before. This effect is even more
                                 // distinguished for larger
                                 // matrices (the present one has
                                 // 1260 rows and columns, but large
                                 // matrices often have several
                                 // 100,000s).

                                 // It is worth noting that the
                                 // ``DoFRenumbering'' class offers a number
                                 // of other algorithms as well to renumber
                                 // degrees of freedom. For example, it would
                                 // of course be ideal if all couplings were
                                 // in the lower or upper triangular part of a
                                 // matrix, since then solving the linear
                                 // system would amoung to only forward or
                                 // backward substitution. This is of course
                                 // unachievable for symmetric sparsity
                                 // patterns, but in some special situations
                                 // involving transport equations, this is
                                 // possible by enumerating degrees of freedom
                                 // from the inflow boundary along streamlines
                                 // to the outflow boundary. Not surprisingly,
                                 // ``DoFRenumbering'' also has algorithms for
                                 // this.


                                 // @sect3{The main function}

				 // Finally, this is the main program. The
				 // only thing it does is to allocate and
				 // create the triangulation, then create a
				 // ``DoFHandler'' object and associate it to
				 // the triangulation, and finally call above
				 // two functions on it:
int main () 
{
  Triangulation<2> triangulation;
  make_grid (triangulation);

  DoFHandler<2> dof_handler (triangulation);

  distribute_dofs (dof_handler);
  renumber_dofs (dof_handler);
}
