//---------------------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 2001, 2002, 2003, 2004, 2005 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------



#include <grid/grid_tools.h>
#include <grid/tria.h>
#include <grid/tria_accessor.h>
#include <grid/tria_iterator.h>
#include <lac/sparsity_pattern.h>
#include <lac/compressed_sparsity_pattern.h>
#include <dofs/dof_handler.h>
#include <dofs/dof_accessor.h>
#include <dofs/dof_tools.h>
#include <fe/fe_dgq.h>
#include <multigrid/mg_dof_handler.h>
#include <multigrid/mg_dof_accessor.h>

#include <cmath>

#if deal_II_dimension != 1

template <int dim>
double
GridTools::diameter (const Triangulation<dim> &tria)
{
				   // the algorithm used simply
				   // traverses all cells and picks
				   // out the boundary vertices. it
				   // may or may not be faster to
				   // simply get all vectors, don't
				   // mark boundary vertices, and
				   // compute the distances thereof,
				   // but at least as the mesh is
				   // refined, it seems better to
				   // first mark boundary nodes, as
				   // marking is O(N) in the number of
				   // cells/vertices, while computing
				   // the maximal distance is O(N*N)
  const std::vector<Point<dim> > &vertices = tria.get_vertices ();
  std::vector<bool> boundary_vertices (vertices.size(), false);

  typename Triangulation<dim>::active_cell_iterator
    cell = tria.begin_active();
  const typename Triangulation<dim>::active_cell_iterator
    endc = tria.end();
  for (; cell!=endc; ++cell)
    for (unsigned int face=0; face<GeometryInfo<dim>::faces_per_cell; ++face)
      if (cell->face(face)->at_boundary ())
	for (unsigned int i=0; i<GeometryInfo<dim>::vertices_per_face; ++i)
	  boundary_vertices[cell->face(face)->vertex_index(i)] = true;

				   // now traverse the list of
				   // boundary vertices and check
				   // distances. since distances are
				   // symmetric, we only have to check
				   // one half
  double max_distance_sqr = 0;
  std::vector<bool>::const_iterator pi = boundary_vertices.begin();
  const unsigned int N = boundary_vertices.size();
  for (unsigned int i=0; i<N; ++i, ++pi)
    {
      std::vector<bool>::const_iterator pj = pi+1;
      for (unsigned int j=i+1; j<N; ++j, ++pj)
	if ((*pi==true) && (*pj==true) &&
	    ((vertices[i]-vertices[j]).square() > max_distance_sqr))
	  max_distance_sqr = (vertices[i]-vertices[j]).square();
    };

  return std::sqrt(max_distance_sqr);
}


#else

double
GridTools::diameter (const Triangulation<1> &tria)
{
				   // for 1d, simply check the
				   // vertices of the left- and
				   // rightmost coarse grid cell
  Triangulation<1>::cell_iterator leftmost  = tria.begin(0);
  Triangulation<1>::cell_iterator rightmost = tria.begin(0);

  while (!leftmost->at_boundary(0))  leftmost  = leftmost->neighbor(0);
  while (!rightmost->at_boundary(1)) rightmost = rightmost->neighbor(1);

  return std::sqrt((leftmost->vertex(0) - rightmost->vertex(1)).square());
}

#endif



#if deal_II_dimension == 3

template <>
double
GridTools::cell_measure(const std::vector<Point<3> > &all_vertices,
			const int vertex_indices[GeometryInfo<3>::vertices_per_cell])
{
				   // note that this is the
				   // cell_measure based on the new
				   // deal.II numbering. When called
				   // from inside GridReordering make
				   // sure that you reorder the
				   // vertex_indices before
  const double x[8] = { all_vertices[vertex_indices[0]](0),
			all_vertices[vertex_indices[1]](0),
			all_vertices[vertex_indices[2]](0),
			all_vertices[vertex_indices[3]](0),
			all_vertices[vertex_indices[4]](0),
			all_vertices[vertex_indices[5]](0),
			all_vertices[vertex_indices[6]](0),
			all_vertices[vertex_indices[7]](0)   };
  const double y[8] = { all_vertices[vertex_indices[0]](1),
			all_vertices[vertex_indices[1]](1),
			all_vertices[vertex_indices[2]](1),
			all_vertices[vertex_indices[3]](1),
			all_vertices[vertex_indices[4]](1),
			all_vertices[vertex_indices[5]](1),
			all_vertices[vertex_indices[6]](1),
			all_vertices[vertex_indices[7]](1)  };
  const double z[8] = { all_vertices[vertex_indices[0]](2),
			all_vertices[vertex_indices[1]](2),
			all_vertices[vertex_indices[2]](2),
			all_vertices[vertex_indices[3]](2),
			all_vertices[vertex_indices[4]](2),
			all_vertices[vertex_indices[5]](2),
			all_vertices[vertex_indices[6]](2),
			all_vertices[vertex_indices[7]](2)  };

/*
  This is the same Maple script as in the barycenter method above
  except of that here the shape functions tphi[0]-tphi[7] are ordered
  according to the lexicographic numbering.

  x := array(0..7):
  y := array(0..7):
  z := array(0..7):
  tphi[0] := (1-xi)*(1-eta)*(1-zeta):
  tphi[1] :=     xi*(1-eta)*(1-zeta):
  tphi[2] := (1-xi)*    eta*(1-zeta):
  tphi[3] :=     xi*    eta*(1-zeta):
  tphi[4] := (1-xi)*(1-eta)*zeta:
  tphi[5] :=     xi*(1-eta)*zeta:
  tphi[6] := (1-xi)*    eta*zeta:
  tphi[7] :=     xi*    eta*zeta:
  x_real := sum(x[s]*tphi[s], s=0..7):
  y_real := sum(y[s]*tphi[s], s=0..7):
  z_real := sum(z[s]*tphi[s], s=0..7):
  with (linalg):
  J := matrix(3,3, [[diff(x_real, xi), diff(x_real, eta), diff(x_real, zeta)],
  [diff(y_real, xi), diff(y_real, eta), diff(y_real, zeta)],
  [diff(z_real, xi), diff(z_real, eta), diff(z_real, zeta)]]): 
  detJ := det (J):

  measure := simplify ( int ( int ( int (detJ, xi=0..1), eta=0..1), zeta=0..1)):

  readlib(C):

  C(measure, optimized);

  The C code produced by this maple script is further optimized by
  hand. In particular, division by 12 is performed only once, not
  hundred of times.
*/

  const double t3 = y[3]*x[2];
  const double t5 = z[1]*x[5];
  const double t9 = z[3]*x[2];
  const double t11 = x[1]*y[0];
  const double t14 = x[4]*y[0];
  const double t18 = x[5]*y[7];
  const double t20 = y[1]*x[3];
  const double t22 = y[5]*x[4];
  const double t26 = z[7]*x[6];
  const double t28 = x[0]*y[4];
  const double t34 = z[3]*x[1]*y[2]+t3*z[1]-t5*y[7]+y[7]*x[4]*z[6]+t9*y[6]-t11*z[4]-t5*y[3]-t14*z[2]+z[1]*x[4]*y[0]-t18*z[3]+t20*z[0]-t22*z[0]-y[0]*x[5]*z[4]-t26*y[3]+t28*z[2]-t9*y[1]-y[1]*x[4]*z[0]-t11*z[5];
  const double t37 = y[1]*x[0];
  const double t44 = x[1]*y[5];
  const double t46 = z[1]*x[0];
  const double t49 = x[0]*y[2];
  const double t52 = y[5]*x[7];
  const double t54 = x[3]*y[7];
  const double t56 = x[2]*z[0];
  const double t58 = x[3]*y[2];
  const double t64 = -x[6]*y[4]*z[2]-t37*z[2]+t18*z[6]-x[3]*y[6]*z[2]+t11*z[2]+t5*y[0]+t44*z[4]-t46*y[4]-t20*z[7]-t49*z[6]-t22*z[1]+t52*z[3]-t54*z[2]-t56*y[4]-t58*z[0]+y[1]*x[2]*z[0]+t9*y[7]+t37*z[4];
  const double t66 = x[1]*y[7];
  const double t68 = y[0]*x[6];
  const double t70 = x[7]*y[6];
  const double t73 = z[5]*x[4];
  const double t76 = x[6]*y[7];
  const double t90 = x[4]*z[0];
  const double t92 = x[1]*y[3];
  const double t95 = -t66*z[3]-t68*z[2]-t70*z[2]+t26*y[5]-t73*y[6]-t14*z[6]+t76*z[2]-t3*z[6]+x[6]*y[2]*z[4]-z[3]*x[6]*y[2]+t26*y[4]-t44*z[3]-x[1]*y[2]*z[0]+x[5]*y[6]*z[4]+t54*z[5]+t90*y[2]-t92*z[2]+t46*y[2];
  const double t102 = x[2]*y[0];
  const double t107 = y[3]*x[7];
  const double t114 = x[0]*y[6];
  const double t125 = y[0]*x[3]*z[2]-z[7]*x[5]*y[6]-x[2]*y[6]*z[4]+t102*z[6]-t52*z[6]+x[2]*y[4]*z[6]-t107*z[5]-t54*z[6]+t58*z[6]-x[7]*y[4]*z[6]+t37*z[5]-t114*z[4]+t102*z[4]-z[1]*x[2]*y[0]+t28*z[6]-y[5]*x[6]*z[4]-z[5]*x[1]*y[4]-t73*y[7];
  const double t129 = z[0]*x[6];
  const double t133 = y[1]*x[7];
  const double t145 = y[1]*x[5];
  const double t156 = t90*y[6]-t129*y[4]+z[7]*x[2]*y[6]-t133*z[5]+x[5]*y[3]*z[7]-t26*y[2]-t70*z[3]+t46*y[3]+z[5]*x[7]*y[4]+z[7]*x[3]*y[6]-t49*z[4]+t145*z[7]-x[2]*y[7]*z[6]+t70*z[5]+t66*z[5]-z[7]*x[4]*y[6]+t18*z[4]+x[1]*y[4]*z[0];
  const double t160 = x[5]*y[4];
  const double t165 = z[1]*x[7];
  const double t178 = z[1]*x[3];
  const double t181 = t107*z[6]+t22*z[7]+t76*z[3]+t160*z[1]-x[4]*y[2]*z[6]+t70*z[4]+t165*y[5]+x[7]*y[2]*z[6]-t76*z[5]-t76*z[4]+t133*z[3]-t58*z[1]+y[5]*x[0]*z[4]+t114*z[2]-t3*z[7]+t20*z[2]+t178*y[7]+t129*y[2];
  const double t207 = t92*z[7]+t22*z[6]+z[3]*x[0]*y[2]-x[0]*y[3]*z[2]-z[3]*x[7]*y[2]-t165*y[3]-t9*y[0]+t58*z[7]+y[3]*x[6]*z[2]+t107*z[2]+t73*y[0]-x[3]*y[5]*z[7]+t3*z[0]-t56*y[6]-z[5]*x[0]*y[4]+t73*y[1]-t160*z[6]+t160*z[0];
  const double t228 = -t44*z[7]+z[5]*x[6]*y[4]-t52*z[4]-t145*z[4]+t68*z[4]+t92*z[5]-t92*z[0]+t11*z[3]+t44*z[0]+t178*y[5]-t46*y[5]-t178*y[0]-t145*z[0]-t20*z[5]-t37*z[3]-t160*z[7]+t145*z[3]+x[4]*y[6]*z[2];

  return (t34+t64+t95+t125+t156+t181+t207+t228)/12.;
}

#else

template <int dim>
double
GridTools::cell_measure(const std::vector<Point<dim> > &all_vertices,
			const int [GeometryInfo<dim>::vertices_per_cell])
{
  Assert(false, ExcNotImplemented());
  return 0.;
}

#endif


template <int dim>
void
GridTools::delete_unused_vertices (std::vector<Point<dim> >    &vertices,
				   std::vector<CellData<dim> > &cells,
				   SubCellData                 &subcelldata)
{
				   // first check which vertices are
				   // actually used
  std::vector<bool> vertex_used (vertices.size(), false);
  for (unsigned int c=0; c<cells.size(); ++c)
    for (unsigned int v=0; v<GeometryInfo<dim>::vertices_per_cell; ++v)
      vertex_used[cells[c].vertices[v]] = true;

				   // then renumber the vertices that
				   // are actually used in the same
				   // order as they were beforehand
  const unsigned int invalid_vertex = deal_II_numbers::invalid_unsigned_int;
  std::vector<unsigned int> new_vertex_numbers (vertices.size(), invalid_vertex);
  unsigned int next_free_number = 0;
  for (unsigned int i=0; i<vertices.size(); ++i)
    if (vertex_used[i] == true)
      {
	new_vertex_numbers[i] = next_free_number;
	++next_free_number;
      };

				   // next replace old vertex numbers
				   // by the new ones
  for (unsigned int c=0; c<cells.size(); ++c)
    for (unsigned int v=0; v<GeometryInfo<dim>::vertices_per_cell; ++v)
      cells[c].vertices[v] = new_vertex_numbers[cells[c].vertices[v]];

				   // same for boundary data
  for (unsigned int c=0; c<subcelldata.boundary_lines.size(); ++c)
    for (unsigned int v=0; v<GeometryInfo<1>::vertices_per_cell; ++v)
      subcelldata.boundary_lines[c].vertices[v]
	= new_vertex_numbers[subcelldata.boundary_lines[c].vertices[v]];
  for (unsigned int c=0; c<subcelldata.boundary_quads.size(); ++c)
    for (unsigned int v=0; v<GeometryInfo<2>::vertices_per_cell; ++v)
      subcelldata.boundary_quads[c].vertices[v]
	= new_vertex_numbers[subcelldata.boundary_quads[c].vertices[v]];

				   // finally copy over the vertices
				   // which we really need to a new
				   // array and replace the old one by
				   // the new one
  std::vector<Point<dim> > tmp;
  tmp.reserve (std::count(vertex_used.begin(), vertex_used.end(), true));
  for (unsigned int v=0; v<vertices.size(); ++v)
    if (vertex_used[v] == true)
      tmp.push_back (vertices[v]);
  swap (vertices, tmp);
}



// define some transformations in an anonymous namespace
#ifdef DEAL_II_ANON_NAMESPACE_BOGUS_WARNING
  namespace TRANS
#else
  namespace
#endif
{
  template <int dim>
  class ShiftPoint
  {
    public:
      ShiftPoint (const Point<dim> &shift)
		      :
		      shift(shift)
	{};
      Point<dim> operator() (const Point<dim> p) const
	{
	  return p+shift;
	};
    private:
      const Point<dim> shift;
  };


                                   // the following class is only
                                   // needed in 2d, so avoid trouble
                                   // with compilers warning otherwise
#if deal_II_dimension == 2
  class Rotate2d
  {
    public:
      Rotate2d (const double angle)
		      :
		      angle(angle)
	{};
      Point<2> operator() (const Point<2> p) const
	{
	  return Point<2> (std::cos(angle)*p(0) - std::sin(angle) * p(1),
			   std::sin(angle)*p(0) + std::cos(angle) * p(1));
	};
    private:
      const double angle;
  };
#endif


  template <int dim>
  class ScalePoint
  {
    public:
      ScalePoint (const double factor)
		      :
		      factor(factor)
	{};
      Point<dim> operator() (const Point<dim> p) const
	{
	  return p*factor;
	};
    private:
      const double factor;
  };
}


template <int dim>
void
GridTools::shift (const Point<dim>   &shift_vector,
		  Triangulation<dim> &triangulation)
{
#ifdef DEAL_II_ANON_NAMESPACE_BOGUS_WARNING
  transform (TRANS::ShiftPoint<dim>(shift_vector), triangulation);
#else
  transform (ShiftPoint<dim>(shift_vector), triangulation);
#endif  
}


#if deal_II_dimension == 2

void
GridTools::rotate (const double      angle,
		   Triangulation<2> &triangulation)
{
#ifdef DEAL_II_ANON_NAMESPACE_BOGUS_WARNING
  transform (TRANS::Rotate2d(angle), triangulation);
#else
  transform (Rotate2d(angle), triangulation);
#endif  
}

#endif


template <int dim>
void
GridTools::scale (const double        scaling_factor,
		  Triangulation<dim> &triangulation)
{
  Assert (scaling_factor>0, ExcScalingFactorNotPositive (scaling_factor));
#ifdef DEAL_II_ANON_NAMESPACE_BOGUS_WARNING
  transform (TRANS::ScalePoint<dim>(scaling_factor), triangulation);
#else
  transform (ScalePoint<dim>(scaling_factor), triangulation);
#endif  
}



template <int dim, typename Container>
typename Container::active_cell_iterator
GridTools::find_active_cell_around_point (const Container  &container,
                                          const Point<dim> &p)
{
                                   // first find the coarse grid cell
                                   // that contains the point. we can
                                   // only do this by a linear search
  typename Container::cell_iterator cell = container.begin(0);
  for (; cell!=container.end(0); ++cell)
    if (cell->point_inside (p))
      break;

                                   // make sure that we found a cell
                                   // in the coarse grid that contains
                                   // this point. for cases where this
                                   // might happen unexpectedly, see
                                   // the documentation of this
                                   // function
  AssertThrow (cell != container.end(0),
               ExcPointNotFoundInCoarseGrid<dim> (p));

                                   // now do the logarithmic part of
                                   // the algorithm: go from child to
                                   // grandchild
  while (cell->has_children())
    {
      const unsigned int n_children=cell->n_children();
      unsigned int c=0;
      for (; c<n_children; ++c)
        if (cell->child(c)->point_inside (p))
          break;

                                       // make sure we found a child
                                       // cell
      AssertThrow (c != n_children, ExcPointNotFound<dim> (p));

                                       // then reset cell to the child
      cell = cell->child(c);
    }

                                   // now that we have a terminal
                                   // cell, return it
  return cell;
}



template <int dim>
void
GridTools::
partition_triangulation (const unsigned int  n_partitions,
                         Triangulation<dim> &triangulation)
{
  Assert (n_partitions > 0, ExcInvalidNumberOfPartitions(n_partitions));

                                   // check for an easy return
  if (n_partitions == 1)
    {
      for (typename Triangulation<dim>::active_cell_iterator
             cell = triangulation.begin_active();
           cell != triangulation.end(); ++cell)
        cell->set_subdomain_id (0);
      return;
    }

                                   // we decompose the domain by first
                                   // generating the connection graph of all
                                   // cells with their neighbors, and then
                                   // passing this graph off to METIS. To make
                                   // things a little simpler and more
                                   // general, we let the function
                                   // DoFTools:make_flux_sparsity_pattern
                                   // function generate the connection graph
                                   // for us and reuse the SparsityPattern
                                   // data structure for the connection
                                   // graph. The connection structure of the
                                   // mesh is obtained by using a fake
                                   // piecewise constant finite element
                                   //
                                   // Since in 3d the generation of a
                                   // sparsity pattern can be expensive, we
                                   // take the detour of the compressed
                                   // sparsity pattern, which is a little
                                   // slower but more efficient in terms of
                                   // memory
  FE_DGQ<dim> fake_q0(0);
  DoFHandler<dim> dof_handler (triangulation);
  dof_handler.distribute_dofs (fake_q0);
  Assert (dof_handler.n_dofs() == triangulation.n_active_cells(),
          ExcInternalError());
  
  CompressedSparsityPattern csp (dof_handler.n_dofs(),
                                 dof_handler.n_dofs());
  DoFTools::make_flux_sparsity_pattern (dof_handler, csp);
  
  SparsityPattern sparsity_pattern;
  sparsity_pattern.copy_from (csp);

                                   // partition this connection graph and get
                                   // back a vector of indices, one per degree
                                   // of freedom (which is associated with a
                                   // cell)
  std::vector<unsigned int> partition_indices (triangulation.n_active_cells());
  sparsity_pattern.partition (n_partitions,  partition_indices);

                                   // finally loop over all cells and set the
                                   // subdomain ids. for this, get the DoF
                                   // index of each cell and extract the
                                   // subdomain id from the vector obtained
                                   // above
  std::vector<unsigned int> dof_indices(1);
  for (typename DoFHandler<dim>::active_cell_iterator
         cell = dof_handler.begin_active();
       cell != dof_handler.end(); ++cell)
    {
      cell->get_dof_indices(dof_indices);
      Assert (dof_indices[0] < triangulation.n_active_cells(),
              ExcInternalError());
      Assert (partition_indices[dof_indices[0]] < n_partitions,
              ExcInternalError());
      
      cell->set_subdomain_id (partition_indices[dof_indices[0]]);
    }
}



template <int dim>
void
GridTools::
get_subdomain_association (const Triangulation<dim>  &triangulation,
                           std::vector<unsigned int> &subdomain)
{
  Assert (subdomain.size() == triangulation.n_active_cells(),
          ExcDimensionMismatch (subdomain.size(),
                                triangulation.n_active_cells()));
  unsigned int index = 0;
  for (typename Triangulation<dim>::active_cell_iterator
         cell = triangulation.begin_active();
       cell!=triangulation.end(); ++cell, ++index)
    subdomain[index] = cell->subdomain_id();

  Assert (index == subdomain.size(), ExcInternalError());
}



template <int dim>
unsigned int
GridTools::
count_cells_with_subdomain_association (const Triangulation<dim> &triangulation,
                                        const unsigned int        subdomain)
{
  unsigned int count = 0;
  for (typename Triangulation<dim>::active_cell_iterator
         cell = triangulation.begin_active();
       cell!=triangulation.end(); ++cell)
    if (cell->subdomain_id() == subdomain)
      ++count;

  Assert (count != 0, ExcNonExistentSubdomain(subdomain));

  return count;
}



#if deal_II_dimension != 1
template
double
GridTools::diameter<deal_II_dimension> (const Triangulation<deal_II_dimension> &);
#endif

template
void GridTools::delete_unused_vertices (std::vector<Point<deal_II_dimension> > &,
					std::vector<CellData<deal_II_dimension> > &,
					SubCellData &);

template
void GridTools::shift<deal_II_dimension> (const Point<deal_II_dimension> &,
					  Triangulation<deal_II_dimension> &);

template
void GridTools::scale<deal_II_dimension> (const double,
					  Triangulation<deal_II_dimension> &);

template
Triangulation<deal_II_dimension>::active_cell_iterator
GridTools::find_active_cell_around_point (const Triangulation<deal_II_dimension> &,
                                          const Point<deal_II_dimension> &p);

template
DoFHandler<deal_II_dimension>::active_cell_iterator
GridTools::find_active_cell_around_point (const DoFHandler<deal_II_dimension> &,
                                          const Point<deal_II_dimension> &p);

template
MGDoFHandler<deal_II_dimension>::active_cell_iterator
GridTools::find_active_cell_around_point (const MGDoFHandler<deal_II_dimension> &,
                                          const Point<deal_II_dimension> &p);

template
void
GridTools::partition_triangulation (const unsigned int,
                                    Triangulation<deal_II_dimension> &);

template
void
GridTools::
get_subdomain_association (const Triangulation<deal_II_dimension>  &,
                           std::vector<unsigned int> &);

template
unsigned int
GridTools::
count_cells_with_subdomain_association (const Triangulation<deal_II_dimension> &,
                                        const unsigned int        );

