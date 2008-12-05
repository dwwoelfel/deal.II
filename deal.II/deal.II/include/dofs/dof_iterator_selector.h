//----------------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2006, 2007, 2008 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------------------------------------------------
#ifndef __deal2__dof_iterators_h
#define __deal2__dof_iterators_h

#include <base/config.h>


DEAL_II_NAMESPACE_OPEN

template <int, int, int> class InvalidAccessor;

template <int structdim, class DH> class DoFAccessor;
template <class DH> class DoFCellAccessor;

template <int dim, int spacedim> class FiniteElement;
template <typename Accessor> class TriaRawIterator;
template <typename Accessor> class TriaIterator;
template <typename Accessor> class TriaActiveIterator;
template <int dim, int spacedim> class Triangulation;
template <int dim, int spacedim> class DoFHandler;


namespace internal
{
  namespace DoFHandler
  {
    template <class DH>
    struct Iterators;


/**
 * Define some types for DoF handling in one dimension.
 *
 * The types have the same meaning as those declared in
 * internal::Triangulation::Iterators<1,spacedim>, only the treatment of
 * templates is a little more complicated. See the @ref Iterators module
 * for more information.
 *
 * @author Wolfgang Bangerth, Oliver Kayser-Herold, 1998, 2003, 2008
 */
    template <template <int, int> class DH, int spacedim>
    struct Iterators<DH<1, spacedim> >
    {
        typedef DH<1,spacedim> DoFHandler_type;
    
        typedef TriaRawIterator<DoFCellAccessor<DoFHandler_type> >    raw_line_iterator;
        typedef TriaIterator<DoFCellAccessor<DoFHandler_type> >       line_iterator;
        typedef TriaActiveIterator<DoFCellAccessor<DoFHandler_type> > active_line_iterator;
    
        typedef TriaRawIterator   <InvalidAccessor<2,1,spacedim> > raw_quad_iterator;
        typedef TriaIterator      <InvalidAccessor<2,1,spacedim> > quad_iterator;
        typedef TriaActiveIterator<InvalidAccessor<2,1,spacedim> > active_quad_iterator;

        typedef TriaRawIterator   <InvalidAccessor<3,1,spacedim> > raw_hex_iterator;
        typedef TriaIterator      <InvalidAccessor<3,1,spacedim> > hex_iterator;
        typedef TriaActiveIterator<InvalidAccessor<3,1,spacedim> > active_hex_iterator;
    
        typedef raw_line_iterator    raw_cell_iterator;
        typedef line_iterator        cell_iterator;
        typedef active_line_iterator active_cell_iterator;
    
        typedef TriaRawIterator   <InvalidAccessor<0,1,spacedim> > raw_face_iterator;
        typedef TriaIterator      <InvalidAccessor<0,1,spacedim> > face_iterator;
        typedef TriaActiveIterator<InvalidAccessor<0,1,spacedim> > active_face_iterator;
    };




/**
 * Define some types for DoF handling in two dimensions.
 *
 * The types have the same meaning as those declared in
 * internal::Triangulation::Iterators<2,spacedim>, only the treatment of
 * templates is a little more complicated. See the @ref Iterators module
 * for more information.
 *
 * @author Wolfgang Bangerth, Oliver Kayser-Herold, 1998, 2003, 2008
 */
    template <template <int, int> class DH, int spacedim>
    struct Iterators<DH<2, spacedim> >
    {
        typedef DH<2,spacedim> DoFHandler_type;
    
        typedef TriaRawIterator<DoFAccessor<1, DoFHandler_type> >    raw_line_iterator;
        typedef TriaIterator<DoFAccessor<1, DoFHandler_type> >       line_iterator;
        typedef TriaActiveIterator<DoFAccessor<1, DoFHandler_type> > active_line_iterator;
    
        typedef TriaRawIterator<DoFCellAccessor<DoFHandler_type> >         raw_quad_iterator;
        typedef TriaIterator<DoFCellAccessor<DoFHandler_type> >            quad_iterator;
        typedef TriaActiveIterator<DoFCellAccessor<DoFHandler_type> >      active_quad_iterator;
    
        typedef TriaRawIterator   <InvalidAccessor<3,2,spacedim> > raw_hex_iterator;
        typedef TriaIterator      <InvalidAccessor<3,2,spacedim> > hex_iterator;
        typedef TriaActiveIterator<InvalidAccessor<3,2,spacedim> > active_hex_iterator;
    
        typedef raw_quad_iterator    raw_cell_iterator;
        typedef quad_iterator        cell_iterator;
        typedef active_quad_iterator active_cell_iterator;
    
        typedef raw_line_iterator    raw_face_iterator;
        typedef line_iterator        face_iterator;
        typedef active_line_iterator active_face_iterator;    
    };




/**
 * Define some types for DoF handling in three dimensions.
 *
 * The types have the same meaning as those declared in
 * internal::Triangulation::Iterators<3,spacedim>, only the treatment of
 * templates is a little more complicated. See the @ref Iterators module
 * for more information.
 *
 * @author Wolfgang Bangerth, Oliver Kayser-Herold, 1998, 2003, 2008
 */
    template <template <int, int> class DH, int spacedim>
    struct Iterators<DH<3, spacedim> >
    {
        typedef DH<3, spacedim> DoFHandler_type;
      
        typedef TriaRawIterator<DoFAccessor<1, DoFHandler_type> >    raw_line_iterator;
        typedef TriaIterator<DoFAccessor<1, DoFHandler_type> >       line_iterator;
        typedef TriaActiveIterator<DoFAccessor<1, DoFHandler_type> > active_line_iterator;
      
        typedef TriaRawIterator<DoFAccessor<2, DoFHandler_type> >    raw_quad_iterator;
        typedef TriaIterator<DoFAccessor<2, DoFHandler_type> >       quad_iterator;
        typedef TriaActiveIterator<DoFAccessor<2, DoFHandler_type> > active_quad_iterator;
      
        typedef TriaRawIterator<DoFCellAccessor<DoFHandler_type> >         raw_hex_iterator;
        typedef TriaIterator<DoFCellAccessor<DoFHandler_type> >            hex_iterator;
        typedef TriaActiveIterator<DoFCellAccessor<DoFHandler_type> >      active_hex_iterator;
      
        typedef raw_hex_iterator    raw_cell_iterator;
        typedef hex_iterator        cell_iterator;
        typedef active_hex_iterator active_cell_iterator;
      
        typedef raw_quad_iterator    raw_face_iterator;
        typedef quad_iterator        face_iterator;
        typedef active_quad_iterator active_face_iterator;    
    };
  }
}

DEAL_II_NAMESPACE_CLOSE

#endif // __deal2__dof_iterator_selector_h
