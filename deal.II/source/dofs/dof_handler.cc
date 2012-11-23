//---------------------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------


#include <deal.II/base/memory_consumption.h>
#include <deal.II/dofs/dof_handler.h>
#include <deal.II/dofs/dof_handler_policy.h>
#include <deal.II/dofs/dof_levels.h>
#include <deal.II/dofs/dof_faces.h>
#include <deal.II/dofs/dof_accessor.h>
#include <deal.II/grid/tria_accessor.h>
#include <deal.II/grid/tria_iterator.h>
#include <deal.II/grid/tria_levels.h>
#include <deal.II/grid/tria.h>
#include <deal.II/base/geometry_info.h>
#include <deal.II/fe/fe.h>

#include <set>
#include <algorithm>

DEAL_II_NAMESPACE_OPEN


//TODO[WB]: do not use a plain pointer for DoFHandler::faces, but rather an
//auto_ptr or some such thing. alternatively, why not use the DoFFaces object
//right away?

template<int dim, int spacedim>
const unsigned int DoFHandler<dim,spacedim>::dimension;

template<int dim, int spacedim>
const unsigned int DoFHandler<dim,spacedim>::space_dimension;

template <int dim, int spacedim>
const unsigned int DoFHandler<dim,spacedim>::invalid_dof_index;

template <int dim, int spacedim>
const unsigned int DoFHandler<dim,spacedim>::default_fe_index;


// reference the invalid_dof_index variable explicitely to work around
// a bug in the icc8 compiler
namespace internal
{
  template <int dim, int spacedim>
  const unsigned int *dummy ()
  {
    return &dealii::DoFHandler<dim,spacedim>::invalid_dof_index;
  }
}



namespace internal
{
  namespace DoFHandler
  {
    // access class
    // dealii::DoFHandler instead of
    // namespace internal::DoFHandler
    using dealii::DoFHandler;


    /**
     * A class with the same purpose as the similarly named class of the
     * Triangulation class. See there for more information.
     */
    struct Implementation
    {
      /**
       * Implement the function of same name in
       * the mother class.
       */
      template <int spacedim>
      static
      unsigned int
      max_couplings_between_dofs (const DoFHandler<1,spacedim> &dof_handler)
      {
        return std::min(3*dof_handler.selected_fe->dofs_per_vertex +
                        2*dof_handler.selected_fe->dofs_per_line,
                        dof_handler.n_dofs());
      }



      template <int spacedim>
      static
      unsigned int
      max_couplings_between_dofs (const DoFHandler<2,spacedim> &dof_handler)
      {

        // get these numbers by drawing pictures
        // and counting...
        // example:
        //   |     |     |
        // --x-----x--x--X--
        //   |     |  |  |
        //   |     x--x--x
        //   |     |  |  |
        // --x--x--*--x--x--
        //   |  |  |     |
        //   x--x--x     |
        //   |  |  |     |
        // --X--x--x-----x--
        //   |     |     |
        // x = vertices connected with center vertex *;
        //   = total of 19
        // (the X vertices are connected with * if
        // the vertices adjacent to X are hanging
        // nodes)
        // count lines -> 28 (don't forget to count
        // mother and children separately!)
        unsigned int max_couplings;
        switch (dof_handler.tria->max_adjacent_cells())
          {
          case 4:
            max_couplings=19*dof_handler.selected_fe->dofs_per_vertex +
                          28*dof_handler.selected_fe->dofs_per_line +
                          8*dof_handler.selected_fe->dofs_per_quad;
            break;
          case 5:
            max_couplings=21*dof_handler.selected_fe->dofs_per_vertex +
                          31*dof_handler.selected_fe->dofs_per_line +
                          9*dof_handler.selected_fe->dofs_per_quad;
            break;
          case 6:
            max_couplings=28*dof_handler.selected_fe->dofs_per_vertex +
                          42*dof_handler.selected_fe->dofs_per_line +
                          12*dof_handler.selected_fe->dofs_per_quad;
            break;
          case 7:
            max_couplings=30*dof_handler.selected_fe->dofs_per_vertex +
                          45*dof_handler.selected_fe->dofs_per_line +
                          13*dof_handler.selected_fe->dofs_per_quad;
            break;
          case 8:
            max_couplings=37*dof_handler.selected_fe->dofs_per_vertex +
                          56*dof_handler.selected_fe->dofs_per_line +
                          16*dof_handler.selected_fe->dofs_per_quad;
            break;

            // the following numbers are not based on actual counting but by
            // extrapolating the number sequences from the previous ones (for
            // example, for dofs_per_vertex, the sequence above is 19, 21, 28,
            // 30, 37, and is continued as follows):
          case 9:
            max_couplings=39*dof_handler.selected_fe->dofs_per_vertex +
                          59*dof_handler.selected_fe->dofs_per_line +
                          17*dof_handler.selected_fe->dofs_per_quad;
            break;
          case 10:
            max_couplings=46*dof_handler.selected_fe->dofs_per_vertex +
                          70*dof_handler.selected_fe->dofs_per_line +
                          20*dof_handler.selected_fe->dofs_per_quad;
            break;
          case 11:
            max_couplings=48*dof_handler.selected_fe->dofs_per_vertex +
                          73*dof_handler.selected_fe->dofs_per_line +
                          21*dof_handler.selected_fe->dofs_per_quad;
            break;
          case 12:
            max_couplings=55*dof_handler.selected_fe->dofs_per_vertex +
                          84*dof_handler.selected_fe->dofs_per_line +
                          24*dof_handler.selected_fe->dofs_per_quad;
            break;
          case 13:
            max_couplings=57*dof_handler.selected_fe->dofs_per_vertex +
                          87*dof_handler.selected_fe->dofs_per_line +
                          25*dof_handler.selected_fe->dofs_per_quad;
            break;
          case 14:
            max_couplings=63*dof_handler.selected_fe->dofs_per_vertex +
                          98*dof_handler.selected_fe->dofs_per_line +
                          28*dof_handler.selected_fe->dofs_per_quad;
            break;
          case 15:
            max_couplings=65*dof_handler.selected_fe->dofs_per_vertex +
                          103*dof_handler.selected_fe->dofs_per_line +
                          29*dof_handler.selected_fe->dofs_per_quad;
            break;
          case 16:
            max_couplings=72*dof_handler.selected_fe->dofs_per_vertex +
                          114*dof_handler.selected_fe->dofs_per_line +
                          32*dof_handler.selected_fe->dofs_per_quad;
            break;

          default:
            Assert (false, ExcNotImplemented());
            max_couplings=0;
          }
        return std::min(max_couplings,dof_handler.n_dofs());
      }


      template <int spacedim>
      static
      unsigned int
      max_couplings_between_dofs (const DoFHandler<3,spacedim> &dof_handler)
      {
//TODO:[?] Invent significantly better estimates than the ones in this function

        // doing the same thing here is a
        // rather complicated thing, compared
        // to the 2d case, since it is hard
        // to draw pictures with several
        // refined hexahedra :-) so I
        // presently only give a coarse
        // estimate for the case that at most
        // 8 hexes meet at each vertex
        //
        // can anyone give better estimate
        // here?
        const unsigned int max_adjacent_cells
          = dof_handler.tria->max_adjacent_cells();

        unsigned int max_couplings;
        if (max_adjacent_cells <= 8)
          max_couplings=7*7*7*dof_handler.selected_fe->dofs_per_vertex +
                        7*6*7*3*dof_handler.selected_fe->dofs_per_line +
                        9*4*7*3*dof_handler.selected_fe->dofs_per_quad +
                        27*dof_handler.selected_fe->dofs_per_hex;
        else
          {
            Assert (false, ExcNotImplemented());
            max_couplings=0;
          }

        return std::min(max_couplings,dof_handler.n_dofs());
      }


      /**
       * Reserve enough space in the
       * <tt>levels[]</tt> objects to store the
       * numbers of the degrees of freedom
       * needed for the given element. The
       * given element is that one which
       * was selected when calling
       * @p distribute_dofs the last time.
       */
      template <int spacedim>
      static
      void reserve_space (DoFHandler<1,spacedim> &dof_handler)
      {
        dof_handler.vertex_dofs
        .resize(dof_handler.tria->n_vertices() *
                dof_handler.selected_fe->dofs_per_vertex,
                DoFHandler<1,spacedim>::invalid_dof_index);

        for (unsigned int i=0; i<dof_handler.tria->n_levels(); ++i)
          {
            dof_handler.levels
            .push_back (new internal::DoFHandler::DoFLevel<1>);

            dof_handler.levels.back()->dof_object.dofs
            .resize (dof_handler.tria->n_raw_cells(i) *
                     dof_handler.selected_fe->dofs_per_line,
                     DoFHandler<1,spacedim>::invalid_dof_index);

            dof_handler.levels.back()->cell_dof_indices_cache
            .resize (dof_handler.tria->n_raw_cells(i) *
                     dof_handler.selected_fe->dofs_per_cell,
                     DoFHandler<1,spacedim>::invalid_dof_index);
          }
      }


      template <int spacedim>
      static
      void reserve_space (DoFHandler<2,spacedim> &dof_handler)
      {
        dof_handler.vertex_dofs
        .resize(dof_handler.tria->n_vertices() *
                dof_handler.selected_fe->dofs_per_vertex,
                DoFHandler<2,spacedim>::invalid_dof_index);

        for (unsigned int i=0; i<dof_handler.tria->n_levels(); ++i)
          {
            dof_handler.levels.push_back (new internal::DoFHandler::DoFLevel<2>);

            dof_handler.levels.back()->dof_object.dofs
            .resize (dof_handler.tria->n_raw_cells(i) *
                     dof_handler.selected_fe->dofs_per_quad,
                     DoFHandler<2,spacedim>::invalid_dof_index);

            dof_handler.levels.back()->cell_dof_indices_cache
            .resize (dof_handler.tria->n_raw_cells(i) *
                     dof_handler.selected_fe->dofs_per_cell,
                     DoFHandler<2,spacedim>::invalid_dof_index);
          }

        dof_handler.faces = new internal::DoFHandler::DoFFaces<2>;
        // avoid access to n_raw_lines when there are no cells
        if (dof_handler.tria->n_cells() > 0)
          {
            dof_handler.faces->lines.dofs
            .resize (dof_handler.tria->n_raw_lines() *
                     dof_handler.selected_fe->dofs_per_line,
                     DoFHandler<2,spacedim>::invalid_dof_index);
          }
      }


      template <int spacedim>
      static
      void reserve_space (DoFHandler<3,spacedim> &dof_handler)
      {
        dof_handler.vertex_dofs
        .resize(dof_handler.tria->n_vertices() *
                dof_handler.selected_fe->dofs_per_vertex,
                DoFHandler<3,spacedim>::invalid_dof_index);

        for (unsigned int i=0; i<dof_handler.tria->n_levels(); ++i)
          {
            dof_handler.levels.push_back (new internal::DoFHandler::DoFLevel<3>);

            dof_handler.levels.back()->dof_object.dofs
            .resize (dof_handler.tria->n_raw_cells(i) *
                     dof_handler.selected_fe->dofs_per_hex,
                     DoFHandler<3,spacedim>::invalid_dof_index);

            dof_handler.levels.back()->cell_dof_indices_cache
            .resize (dof_handler.tria->n_raw_cells(i) *
                     dof_handler.selected_fe->dofs_per_cell,
                     DoFHandler<3,spacedim>::invalid_dof_index);
          }
        dof_handler.faces = new internal::DoFHandler::DoFFaces<3>;

        // avoid access to n_raw_lines when there are no cells
        if (dof_handler.tria->n_cells() > 0)
          {
            dof_handler.faces->lines.dofs
            .resize (dof_handler.tria->n_raw_lines() *
                     dof_handler.selected_fe->dofs_per_line,
                     DoFHandler<3,spacedim>::invalid_dof_index);
            dof_handler.faces->quads.dofs
            .resize (dof_handler.tria->n_raw_quads() *
                     dof_handler.selected_fe->dofs_per_quad,
                     DoFHandler<3,spacedim>::invalid_dof_index);
          }
      }
    };
  }
}



template<int dim, int spacedim>
DoFHandler<dim,spacedim>::DoFHandler (const Triangulation<dim,spacedim> &tria)
  :
  tria(&tria, typeid(*this).name()),
  selected_fe(0, typeid(*this).name()),
  faces(NULL)
{
  // decide whether we need a
  // sequential or a parallel
  // distributed policy
  if (dynamic_cast<const parallel::distributed::Triangulation< dim, spacedim >*>
      (&tria)
      == 0)
    policy.reset (new internal::DoFHandler::Policy::Sequential<dim,spacedim>());
  else
    policy.reset (new internal::DoFHandler::Policy::ParallelDistributed<dim,spacedim>());
}


template<int dim, int spacedim>
DoFHandler<dim,spacedim>::DoFHandler ()
  :
  tria(0, typeid(*this).name()),
  selected_fe(0, typeid(*this).name()),
  faces(NULL)
{}


template <int dim, int spacedim>
DoFHandler<dim,spacedim>::~DoFHandler ()
{
  // release allocated memory
  clear ();
}


template<int dim, int spacedim>
void
DoFHandler<dim,spacedim>::initialize(
  const Triangulation<dim,spacedim> &t,
  const FiniteElement<dim,spacedim> &fe)
{
  tria = &t;
  faces = 0;
  number_cache.n_global_dofs = 0;

  // decide whether we need a
  // sequential or a parallel
  // distributed policy
  if (dynamic_cast<const parallel::distributed::Triangulation< dim, spacedim >*>
      (&t)
      == 0)
    policy.reset (new internal::DoFHandler::Policy::Sequential<dim,spacedim>());
  else
    policy.reset (new internal::DoFHandler::Policy::ParallelDistributed<dim,spacedim>());

  distribute_dofs(fe);
}


/*------------------------ Cell iterator functions ------------------------*/

template <int dim, int spacedim>
typename DoFHandler<dim,spacedim>::cell_iterator
DoFHandler<dim,spacedim>::begin (const unsigned int level) const
{
  return cell_iterator (*this->get_tria().begin(level),
                        this);
}



template <int dim, int spacedim>
typename DoFHandler<dim,spacedim>::active_cell_iterator
DoFHandler<dim,spacedim>::begin_active (const unsigned int level) const
{
  // level is checked in begin
  cell_iterator i = begin (level);
  if (i.state() != IteratorState::valid)
    return i;
  while (i->has_children())
    if ((++i).state() != IteratorState::valid)
      return i;
  return i;
}



template <int dim, int spacedim>
typename DoFHandler<dim,spacedim>::cell_iterator
DoFHandler<dim,spacedim>::end () const
{
  return cell_iterator (&this->get_tria(),
                        -1,
                        -1,
                        this);
}


template <int dim, int spacedim>
typename DoFHandler<dim,spacedim>::cell_iterator
DoFHandler<dim,spacedim>::end (const unsigned int level) const
{
  return (level == this->get_tria().n_levels()-1 ?
          end() :
          begin (level+1));
}


template <int dim, int spacedim>
typename DoFHandler<dim, spacedim>::active_cell_iterator
DoFHandler<dim, spacedim>::end_active (const unsigned int level) const
{
  return (level == this->get_tria().n_levels()-1 ?
          active_cell_iterator(end()) :
          begin_active (level+1));
}



//---------------------------------------------------------------------------



template <>
unsigned int DoFHandler<1>::n_boundary_dofs () const
{
  return 2*get_fe().dofs_per_vertex;
}



template <>
unsigned int DoFHandler<1>::n_boundary_dofs (const FunctionMap &boundary_indicators) const
{
  // check that only boundary
  // indicators 0 and 1 are allowed
  // in 1d
  for (FunctionMap::const_iterator i=boundary_indicators.begin();
       i!=boundary_indicators.end(); ++i)
    Assert ((i->first == 0) || (i->first == 1),
            ExcInvalidBoundaryIndicator());

  return boundary_indicators.size()*get_fe().dofs_per_vertex;
}



template <>
unsigned int DoFHandler<1>::n_boundary_dofs (const std::set<types::boundary_id> &boundary_indicators) const
{
  // check that only boundary
  // indicators 0 and 1 are allowed
  // in 1d
  for (std::set<types::boundary_id>::const_iterator i=boundary_indicators.begin();
       i!=boundary_indicators.end(); ++i)
    Assert ((*i == 0) || (*i == 1),
            ExcInvalidBoundaryIndicator());

  return boundary_indicators.size()*get_fe().dofs_per_vertex;
}


template <>
unsigned int DoFHandler<1,2>::n_boundary_dofs () const
{
  return 2*get_fe().dofs_per_vertex;
}



template <>
unsigned int DoFHandler<1,2>::n_boundary_dofs (const FunctionMap &boundary_indicators) const
{
  // check that only boundary
  // indicators 0 and 1 are allowed
  // in 1d
  for (FunctionMap::const_iterator i=boundary_indicators.begin();
       i!=boundary_indicators.end(); ++i)
    Assert ((i->first == 0) || (i->first == 1),
            ExcInvalidBoundaryIndicator());

  return boundary_indicators.size()*get_fe().dofs_per_vertex;
}



template <>
unsigned int DoFHandler<1,2>::n_boundary_dofs (const std::set<types::boundary_id> &boundary_indicators) const
{
  // check that only boundary
  // indicators 0 and 1 are allowed
  // in 1d
  for (std::set<types::boundary_id>::const_iterator i=boundary_indicators.begin();
       i!=boundary_indicators.end(); ++i)
    Assert ((*i == 0) || (*i == 1),
            ExcInvalidBoundaryIndicator());

  return boundary_indicators.size()*get_fe().dofs_per_vertex;
}



template<int dim, int spacedim>
unsigned int DoFHandler<dim,spacedim>::n_boundary_dofs () const
{
  std::set<int> boundary_dofs;

  const unsigned int dofs_per_face = get_fe().dofs_per_face;
  std::vector<unsigned int> dofs_on_face(dofs_per_face);

  // loop over all faces of all cells
  // and see whether they are at a
  // boundary. note (i) that we visit
  // interior faces twice (which we
  // don't care about) but exterior
  // faces only once as is
  // appropriate, and (ii) that we
  // need not take special care of
  // single lines (using
  // @p{cell->has_boundary_lines}),
  // since we do not support
  // boundaries of dimension dim-2,
  // and so every boundary line is
  // also part of a boundary face.
  active_cell_iterator cell = begin_active (),
                       endc = end();
  for (; cell!=endc; ++cell)
    for (unsigned int f=0; f<GeometryInfo<dim>::faces_per_cell; ++f)
      if (cell->at_boundary(f))
        {
          cell->face(f)->get_dof_indices (dofs_on_face);
          for (unsigned int i=0; i<dofs_per_face; ++i)
            boundary_dofs.insert(dofs_on_face[i]);
        }
  return boundary_dofs.size();
}



template<int dim, int spacedim>
unsigned int
DoFHandler<dim,spacedim>::n_boundary_dofs (const FunctionMap &boundary_indicators) const
{
  Assert (boundary_indicators.find(numbers::internal_face_boundary_id) == boundary_indicators.end(),
          ExcInvalidBoundaryIndicator());

  std::set<int> boundary_dofs;

  const unsigned int dofs_per_face = get_fe().dofs_per_face;
  std::vector<unsigned int> dofs_on_face(dofs_per_face);

  // same as in the previous
  // function, but with an additional
  // check for the boundary indicator
  active_cell_iterator cell = begin_active (),
                       endc = end();
  for (; cell!=endc; ++cell)
    for (unsigned int f=0; f<GeometryInfo<dim>::faces_per_cell; ++f)
      if (cell->at_boundary(f)
          &&
          (boundary_indicators.find(cell->face(f)->boundary_indicator()) !=
           boundary_indicators.end()))
        {
          cell->face(f)->get_dof_indices (dofs_on_face);
          for (unsigned int i=0; i<dofs_per_face; ++i)
            boundary_dofs.insert(dofs_on_face[i]);
        }

  return boundary_dofs.size();
}



template<int dim, int spacedim>
unsigned int
DoFHandler<dim,spacedim>::n_boundary_dofs (const std::set<types::boundary_id> &boundary_indicators) const
{
  Assert (boundary_indicators.find (numbers::internal_face_boundary_id) == boundary_indicators.end(),
          ExcInvalidBoundaryIndicator());

  std::set<int> boundary_dofs;

  const unsigned int dofs_per_face = get_fe().dofs_per_face;
  std::vector<unsigned int> dofs_on_face(dofs_per_face);

  // same as in the previous
  // function, but with a different
  // check for the boundary indicator
  active_cell_iterator cell = begin_active (),
                       endc = end();
  for (; cell!=endc; ++cell)
    for (unsigned int f=0; f<GeometryInfo<dim>::faces_per_cell; ++f)
      if (cell->at_boundary(f)
          &&
          (std::find (boundary_indicators.begin(),
                      boundary_indicators.end(),
                      cell->face(f)->boundary_indicator()) !=
           boundary_indicators.end()))
        {
          cell->face(f)->get_dof_indices (dofs_on_face);
          for (unsigned int i=0; i<dofs_per_face; ++i)
            boundary_dofs.insert(dofs_on_face[i]);
        }

  return boundary_dofs.size();
}



template<int dim, int spacedim>
std::size_t
DoFHandler<dim,spacedim>::memory_consumption () const
{
  std::size_t mem = (MemoryConsumption::memory_consumption (tria) +
                     MemoryConsumption::memory_consumption (selected_fe) +
                     MemoryConsumption::memory_consumption (block_info_object) +
                     MemoryConsumption::memory_consumption (levels) +
                     MemoryConsumption::memory_consumption (*faces) +
                     MemoryConsumption::memory_consumption (faces) +
                     sizeof (number_cache) +
                     MemoryConsumption::memory_consumption (vertex_dofs));
  for (unsigned int i=0; i<levels.size(); ++i)
    mem += MemoryConsumption::memory_consumption (*levels[i]);

  return mem;
}



template<int dim, int spacedim>
void DoFHandler<dim,spacedim>::
distribute_dofs (const FiniteElement<dim,spacedim> &ff)
{
  selected_fe = &ff;

  // delete all levels and set them
  // up newly. note that we still
  // have to allocate space for all
  // degrees of freedom on this mesh
  // (including ghost and cells that
  // are entirely stored on different
  // processors), though we may not
  // assign numbers to some of them
  // (i.e. they will remain at
  // invalid_dof_index). We need to
  // allocate the space because we
  // will want to be able to query
  // the dof_indices on each cell,
  // and simply be told that we don't
  // know them on some cell (i.e. get
  // back invalid_dof_index)
  clear_space ();
  internal::DoFHandler::Implementation::reserve_space (*this);

  // hand things off to the policy
  number_cache = policy->distribute_dofs (*this);

  // initialize the block info object
  // only if this is a sequential
  // triangulation. it doesn't work
  // correctly yet if it is parallel
  if (dynamic_cast<const parallel::distributed::Triangulation<dim,spacedim>*>(&*tria) == 0)
    block_info_object.initialize(*this);
}



template<int dim, int spacedim>
void DoFHandler<dim,spacedim>::initialize_local_block_info ()
{
  block_info_object.initialize_local(*this);
}



template<int dim, int spacedim>
void DoFHandler<dim,spacedim>::clear ()
{
  // release lock to old fe
  selected_fe = 0;

  // release memory
  clear_space ();
}



template <int dim, int spacedim>
void
DoFHandler<dim,spacedim>::
renumber_dofs (const std::vector<unsigned int> &new_numbers)
{
  Assert (new_numbers.size() == n_locally_owned_dofs(),
          ExcRenumberingIncomplete());

#ifdef DEBUG
  // assert that the new indices are
  // consecutively numbered if we are
  // working on a single
  // processor. this doesn't need to
  // hold in the case of a parallel
  // mesh since we map the interval
  // [0...n_dofs()) into itself but
  // only globally, not on each
  // processor
  if (n_locally_owned_dofs() == n_dofs())
    {
      std::vector<unsigned int> tmp(new_numbers);
      std::sort (tmp.begin(), tmp.end());
      std::vector<unsigned int>::const_iterator p = tmp.begin();
      unsigned int                         i = 0;
      for (; p!=tmp.end(); ++p, ++i)
        Assert (*p == i, ExcNewNumbersNotConsecutive(i));
    }
  else
    for (unsigned int i=0; i<new_numbers.size(); ++i)
      Assert (new_numbers[i] < n_dofs(),
              ExcMessage ("New DoF index is not less than the total number of dofs."));
#endif

  number_cache = policy->renumber_dofs (new_numbers, *this);
}



template <int dim, int spacedim>
unsigned int
DoFHandler<dim,spacedim>::max_couplings_between_dofs () const
{
  return internal::DoFHandler::Implementation::max_couplings_between_dofs (*this);
}



template <int dim, int spacedim>
unsigned int
DoFHandler<dim,spacedim>::max_couplings_between_boundary_dofs () const
{
  switch (dim)
    {
    case 1:
      return get_fe().dofs_per_vertex;
    case 2:
      return (3*get_fe().dofs_per_vertex +
              2*get_fe().dofs_per_line);
    case 3:
      // we need to take refinement of
      // one boundary face into
      // consideration here; in fact,
      // this function returns what
      // #max_coupling_between_dofs<2>
      // returns
      //
      // we assume here, that only four
      // faces meet at the boundary;
      // this assumption is not
      // justified and needs to be
      // fixed some time. fortunately,
      // omitting it for now does no
      // harm since the matrix will cry
      // foul if its requirements are
      // not satisfied
      return (19*get_fe().dofs_per_vertex +
              28*get_fe().dofs_per_line +
              8*get_fe().dofs_per_quad);
    default:
      Assert (false, ExcNotImplemented());
      return numbers::invalid_unsigned_int;
    }
}



template<int dim, int spacedim>
void DoFHandler<dim,spacedim>::clear_space ()
{
  for (unsigned int i=0; i<levels.size(); ++i)
    delete levels[i];
  levels.resize (0);

  delete faces;
  faces = 0;

  std::vector<unsigned int> tmp;
  std::swap (vertex_dofs, tmp);

  number_cache.clear ();
}


/*-------------- Explicit Instantiations -------------------------------*/
#include "dof_handler.inst"


DEAL_II_NAMESPACE_CLOSE
