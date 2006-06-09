//----------------------------------------------------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------------------------------------------------

#include <grid/tria.h>

/**
 * A set of test meshes for the deal.II test suite.
 *
 * These meshes exhibit certain key features for writing tests. If you
 * want to test certain properties of algorithms, the following table
 * might be of help.
 *
 * <table border=1>
 * <tr><th>Mesh</th><th>Feature tested</th></tr>
 * <tr><td>#hypercube(tr)</td><td>works at all on a single
 * cell</td></tr>
 * <tr><td>#hypercube(tr,2)</td><td>works on uniform meshes</td></tr>
 * <tr><td>#hypercube(tr,3,true)</td><td>works with local
 * refinement</td></tr>
 * <tr><td>#star_shaped(tr,1)</td><td>method is robust if more than
 * usual cells meet in one vertex</td></tr>
 * <tr><td>#star_shaped(tr,2,true)</td><td>method is robust if more than
 * usual cells meet in one vertex and local refinement exceeds one
 * level</td></tr>
 * </table>
 *
 * @author Guido Kanschat, 2006
 */
namespace TestGrids
{
				   /**
				    * Generate grids based on
				    * hypercube. These meshes have a
				    * regular geometry and topology.
				    *
				    * @param <tt>refinement</tt>
				    * denotes the number of refinement
				    * steps of the root cell.
				    *
				    * @param if <tt>local</tt> is
				    * <tt>true</tt>, refine only the
				    * cell containing the corner with
				    * only negative coordinates.
				    */
  template <int dim>
  void hypercube(Triangulation<dim>& tr,
		 unsigned int refinement = 0,
		 bool local = false);
				   /**
				    * Create a star-shaped mesh,
				    * having more than the average
				    * <tt>2<sup>dim</sup></tt> cells
				    * in the central vertex.
				    *
				    * @param <tt>refinement</tt>
				    * denotes the number of refinement
				    * steps of the root mesh.
				    *
				    * @param if <tt>local</tt> is
				    * <tt>true</tt>, refine only one
				    * of the coarse cells.
				    */
  template <int dim>
  void star_shaped(Triangulation<dim>& tr,
		   unsigned int refinement = 0,
		   bool local = false);
				   /**
				    * Local refinement of every other
				    * cell in a checkerboard fashion.
				    */
  template <int dim>
  void checkers(Triangulation<dim>& tr);
				   /**
				    * Islands of local refinement
				    */
  template <int dim>
  void islands(Triangulation<dim>& tr);
				   /**
				    * Local refinement with an
				    * unrefined hole.
				    */
  template <int dim>
  void laguna(Triangulation<dim>& tr);


  template <int dim>
  void hypercube(Triangulation<dim>& tr,
		 unsigned int refinement,
		 bool local)
  {
    GridGenerator::hyper_cube(tr);
    if (!local)
      {
	if (refinement > 0)
	  tr.refine_global(refinement);
      }
    else
      {
	for (unsigned int i=0;i<refinement;++i)
	  {
	    tr.begin_active()->set_refine_flag();
	    tr.execute_coarsening_and_refinement();
	  }
      }
  }
}
