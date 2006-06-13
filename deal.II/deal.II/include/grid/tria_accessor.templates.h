//---------------------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------
#ifndef __deal2__tria_accessor_templates_h
#define __deal2__tria_accessor_templates_h


#include <base/config.h>
#include <grid/tria.h>
#include <grid/tria_levels.h>
#include <grid/tria_faces.h>
#include <grid/tria_iterator.h>
#include <grid/tria_accessor.h>
#include <grid/tria_iterator.templates.h>
#include <base/geometry_info.h>

#include <cmath>


/*------------------------ Functions: TriaAccessor ---------------------------*/

template <int dim>
inline
TriaAccessor<dim>::TriaAccessor (const Triangulation<dim> *parent,
                                 const int                 level,
                                 const int                 index,
                                 const AccessorData       *)
                :
                present_level (level),
                present_index (index),
                tria (parent)
{}



template <int dim>
inline
void
TriaAccessor<dim>::copy_from (const TriaAccessor<dim> &a)
{
  present_level = a.present_level;
  present_index = a.present_index;
  tria = a.tria;
}



template <int dim>
inline
bool
TriaAccessor<dim>::operator == (const TriaAccessor<dim> &a) const
{
  Assert (tria == a.tria, ExcCantCompareIterators());
  return ((present_index == a.present_index) &&
	  (present_level == a.present_level));
}



template <int dim>
inline
bool
TriaAccessor<dim>::operator != (const TriaAccessor<dim> &a) const
{
  Assert (tria == a.tria, ExcCantCompareIterators());
  return ((present_index != a.present_index) ||
	  (present_level != a.present_level));
}



template <int dim>
inline
int
TriaAccessor<dim>::level () const
{
  return present_level;
}



template <int dim>
inline
int
TriaAccessor<dim>::index () const
{
  return present_index;
}



template <int dim>
inline
IteratorState::IteratorStates
TriaAccessor<dim>::state () const
{
  if ((present_level>=0) && (present_index>=0))
    return IteratorState::valid;
  else
    if ((present_index==-1) && (present_index==-1))
      return IteratorState::past_the_end;
    else
      return IteratorState::invalid;
}



template <int dim>
inline
const Triangulation<dim> &
TriaAccessor<dim>::get_triangulation () const
{
  return *tria;
}


/*------------------------ Functions: LineAccessor ---------------------------*/


template <int dim>
inline
TriaObjectAccessor<1,dim>::
TriaObjectAccessor (const Triangulation<dim> *parent,
                    const int                 level,
                    const int                 index,
                    const AccessorData       *local_data)
                :
                TriaAccessor<dim> (parent, level, index, local_data)
{}



template <int dim>
inline
bool
TriaObjectAccessor<1,dim>::used () const
{
  Assert (this->state() == IteratorState::valid,
	  typename TriaAccessor<dim>::ExcDereferenceInvalidObject());
  return this->tria->faces->lines.used[this->present_index];
}



template <>
inline
bool
TriaObjectAccessor<1,1>::used () const
{
  const int dim=1;
  Assert (this->state() == IteratorState::valid,
	  TriaAccessor<dim>::ExcDereferenceInvalidObject());
  return this->tria->levels[this->present_level]->lines.used[this->present_index];
}



template <int dim>
inline
bool
TriaObjectAccessor<1,dim>::user_flag_set () const
{
  Assert (this->used(), typename TriaAccessor<dim>::ExcCellNotUsed());
  return this->tria->faces->lines.user_flags[this->present_index];
}



template <>
inline
bool
TriaObjectAccessor<1,1>::user_flag_set () const
{
  const int dim=1;
  Assert (this->used(), TriaAccessor<dim>::ExcCellNotUsed());
  return this->tria->levels[this->present_level]->lines.user_flags[this->present_index];
}



template <int dim>
inline
void
TriaObjectAccessor<1,dim>::set_user_flag () const 
{
  Assert (this->used(), typename TriaAccessor<dim>::ExcCellNotUsed());
  this->tria->faces->lines.user_flags[this->present_index] = true;
}



template <>
inline
void
TriaObjectAccessor<1,1>::set_user_flag () const 
{
  const int dim=1;
  Assert (this->used(), TriaAccessor<dim>::ExcCellNotUsed());
  this->tria->levels[this->present_level]->lines.user_flags[this->present_index] = true;
}



template <int dim>
inline
void
TriaObjectAccessor<1,dim>::clear_user_flag () const
{
  Assert (this->used(), typename TriaAccessor<dim>::ExcCellNotUsed());
  this->tria->faces->lines.user_flags[this->present_index] = false;
}



template <>
inline
void
TriaObjectAccessor<1,1>::clear_user_flag () const
{
  const int dim=1;
  Assert (this->used(), TriaAccessor<dim>::ExcCellNotUsed());
  this->tria->levels[this->present_level]->lines.user_flags[this->present_index] = false;
}



template <int dim>
inline
bool
TriaObjectAccessor<1,dim>::has_children () const
{
  Assert (this->state() == IteratorState::valid,
	  typename TriaAccessor<dim>::ExcDereferenceInvalidObject());
  return (this->tria->faces->lines.children[this->present_index] != -1);
}



template <>
inline
bool
TriaObjectAccessor<1,1>::has_children () const
{
  const int dim=1;
  Assert (this->state() == IteratorState::valid,
	  TriaAccessor<dim>::ExcDereferenceInvalidObject());
  return (this->tria->levels[this->present_level]->lines.children[this->present_index] != -1);
}



template <int dim>
inline
int
TriaObjectAccessor<1,dim>::child_index (unsigned const int i) const
{
  Assert (i<2, ExcIndexRange(i,0,2));
  Assert (has_children(), typename TriaAccessor<dim>::ExcCellHasNoChildren());
  return this->tria->faces->lines.children[this->present_index]+i;
}



template <>
inline
int
TriaObjectAccessor<1,1>::child_index (unsigned const int i) const
{
  const int dim=1;
  Assert (i<2, ExcIndexRange(i,0,2));
  Assert (has_children(), TriaAccessor<dim>::ExcCellHasNoChildren());
  return this->tria->levels[this->present_level]->lines.children[this->present_index]+i;
}



template <int dim>
inline
TriaIterator<dim,TriaObjectAccessor<1,dim> >
TriaObjectAccessor<1,dim>::child (const unsigned int i) const
{
  Assert (i<2, ExcIndexRange(i,0,2));
  int next_level;
  if (dim==1)
    next_level = this->present_level+1;
  else
    next_level = 0;
  
  TriaIterator<dim,TriaObjectAccessor<1,dim> >
    q (this->tria, next_level, child_index (i));

  Assert ((q.state() == IteratorState::past_the_end) || q->used(),
	  typename TriaAccessor<dim>::ExcUnusedCellAsChild());

  return q;
}



template <int dim>
inline
unsigned int
TriaObjectAccessor<1,dim>::n_children () const
{
  Assert (has_children()==true, TriaAccessor<1>::ExcCellHasNoChildren());
  return GeometryInfo<1>::children_per_cell;
}



template <int dim>
inline
unsigned int
TriaObjectAccessor<1,dim>::max_refinement_depth () const
{
  if (!has_children())
    return 0;

  const unsigned int depths[2] = { child(0)->max_refinement_depth() + 1,
				   child(1)->max_refinement_depth() + 1  };
  return std::max (depths[0], depths[1]);
}



template <int dim>
inline
bool
TriaObjectAccessor<1,dim>::face_orientation (const unsigned int) const
{
  return true;
}



template <int dim>
inline
void
TriaObjectAccessor<1,dim>::operator ++ ()
{
  ++this->present_index;
				   // is index still in the range of
				   // the vector?
  if (this->present_index
      >=
      static_cast<int>(this->tria->faces->lines.cells.size()))
    this->present_level = this->present_index = -1;
}



template <>
inline
void
TriaObjectAccessor<1,1>::operator ++ ()
{
  ++this->present_index;
				   // is index still in the range of
				   // the vector?
  while (this->present_index
	 >=
	 static_cast<int>(this->tria->levels[this->present_level]->lines.cells.size()))
    {
				       // no -> go one level up until we find
				       // one with more than zero cells
      ++this->present_level;
      this->present_index = 0;
				       // highest level reached?
      if (this->present_level >= static_cast<int>(this->tria->levels.size()))
	{
					   // return with past the end pointer
	  this->present_level = this->present_index = -1;
	  return;
	}
    }
}



template <int dim>
inline
void
TriaObjectAccessor<1,dim>::operator -- ()
{
  --this->present_index;
				   // is index still in the range of
				   // the vector?
  if (this->present_index <0 ) 
    this->present_level = this->present_index = -1;
  return;
}


template <>
inline
void
TriaObjectAccessor<1,1>::operator -- ()
{
  --this->present_index;
				   // is index still in the range of
				   // the vector?
  while (this->present_index < 0) 
    {
				       // no -> go one level down
      --this->present_level;
				       // lowest level reached?
      if (this->present_level == -1) 
	{
					   // return with past the end pointer
	  this->present_level = this->present_index = -1;
	  return;
	}
					 // else
      this->present_index = this->tria->levels[this->present_level]->lines.cells.size()-1;
    }
}


/*------------------------ Functions: QuadAccessor ---------------------------*/


template <int dim>
inline
TriaObjectAccessor<2,dim>::
TriaObjectAccessor (const Triangulation<dim> *parent,
                    const int                 level,
                    const int                 index,
                    const AccessorData       *local_data)
                :
                TriaAccessor<dim> (parent, level, index, local_data)
{}



template <int dim>
inline
bool
TriaObjectAccessor<2,dim>::used () const
{
  Assert (this->state() == IteratorState::valid,
	  typename TriaAccessor<dim>::ExcDereferenceInvalidObject());
    return this->tria->faces->quads.used[this->present_index];
}



template <>
inline
bool
TriaObjectAccessor<2,2>::used () const
{
  const int dim=2;
  Assert (this->state() == IteratorState::valid,
	  TriaAccessor<dim>::ExcDereferenceInvalidObject());
  return this->tria->levels[this->present_level]->quads.used[this->present_index];
}



template <int dim>
inline
bool
TriaObjectAccessor<2,dim>::user_flag_set () const
{
  Assert (this->used(), typename TriaAccessor<dim>::ExcCellNotUsed());
  return this->tria->faces->quads.user_flags[this->present_index];
}



template <>
inline
bool
TriaObjectAccessor<2,2>::user_flag_set () const
{
  const int dim=2;
  Assert (this->used(), TriaAccessor<dim>::ExcCellNotUsed());
  return this->tria->levels[this->present_level]->quads.user_flags[this->present_index];
}



template <int dim>
inline
void
TriaObjectAccessor<2,dim>::set_user_flag () const
{
  Assert (this->used(), typename TriaAccessor<dim>::ExcCellNotUsed());
  this->tria->faces->quads.user_flags[this->present_index] = true;
}



template <>
inline
void
TriaObjectAccessor<2,2>::set_user_flag () const
{
  const int dim=2;
  Assert (this->used(), TriaAccessor<dim>::ExcCellNotUsed());
  this->tria->levels[this->present_level]->quads.user_flags[this->present_index] = true;
}



template <int dim>
inline
void
TriaObjectAccessor<2,dim>::clear_user_flag () const
{
  Assert (this->used(), typename TriaAccessor<dim>::ExcCellNotUsed());
  this->tria->faces->quads.user_flags[this->present_index] = false;
}



template <>
inline
void
TriaObjectAccessor<2,2>::clear_user_flag () const
{
  const int dim=2;
  Assert (this->used(), TriaAccessor<dim>::ExcCellNotUsed());
  this->tria->levels[this->present_level]->quads.user_flags[this->present_index] = false;
}



template <int dim>
inline
TriaIterator<dim,TriaObjectAccessor<1,dim> >
TriaObjectAccessor<2,dim>::line (const unsigned int i) const
{
  return
    TriaIterator<dim,TriaObjectAccessor<1,dim> >
    (
      this->tria,
      0,
      line_index (i)
    );
}



template <int dim>
inline
unsigned int
TriaObjectAccessor<2,dim>::line_index (const unsigned int i) const
{
  Assert (i<4, ExcIndexRange(i,0,4));
  return this->tria->faces->quads.cells[this->present_index].line(i);
}

template <>
inline
unsigned int
TriaObjectAccessor<2,2>::line_index (const unsigned int i) const
{
Assert (i<4, ExcIndexRange(i,0,4));
  return this->tria->levels[this->present_level]->quads.cells[this->present_index].line(i);
}


template <int dim>
inline
bool
TriaObjectAccessor<2,dim>::has_children () const
{
  Assert (this->state() == IteratorState::valid,
	  typename TriaAccessor<dim>::ExcDereferenceInvalidObject());
  return (this->tria->faces->quads.children[this->present_index] != -1);
}



template <>
inline
bool
TriaObjectAccessor<2,2>::has_children () const
{
  const int dim=2;
  Assert (this->state() == IteratorState::valid,
	  TriaAccessor<dim>::ExcDereferenceInvalidObject());
  return (this->tria->levels[this->present_level]->quads.children[this->present_index] != -1);
}



template <int dim>
inline
int TriaObjectAccessor<2,dim>::child_index (const unsigned int i) const
{
  Assert (i<4, ExcIndexRange(i,0,4));
  Assert (has_children(), typename TriaAccessor<dim>::ExcCellHasNoChildren());
  return this->tria->faces->quads.children[this->present_index]+i;
}



template <>
inline
int TriaObjectAccessor<2,2>::child_index (const unsigned int i) const
{
  const int dim=2;
  Assert (i<4, ExcIndexRange(i,0,4));
  Assert (has_children(), TriaAccessor<dim>::ExcCellHasNoChildren());
  return this->tria->levels[this->present_level]->quads.children[this->present_index]+i;
}



template <int dim>
inline
TriaIterator<dim,TriaObjectAccessor<2,dim> >
TriaObjectAccessor<2,dim>::child (const unsigned int i) const
{
  Assert (i<4, ExcIndexRange(i,0,4));
  int next_level;
  if (dim==2)
    next_level = this->present_level+1;
  else
    next_level = 0;
  
  TriaIterator<dim,TriaObjectAccessor<2,dim> >
    q (this->tria, next_level, child_index (i));
  
  Assert ((q.state() == IteratorState::past_the_end) || q->used(),
	  typename TriaAccessor<dim>::ExcUnusedCellAsChild());

  return q;
}



template <int dim>
inline
unsigned int
TriaObjectAccessor<2,dim>::n_children () const
{
  Assert (has_children()==true, TriaAccessor<2>::ExcCellHasNoChildren());
  return GeometryInfo<2>::children_per_cell;
}



template <int dim>
inline
unsigned int
TriaObjectAccessor<2,dim>::max_refinement_depth () const
{
  if (!has_children())
    return 0;

  const unsigned int depths[4] = { child(0)->max_refinement_depth() + 1,
				   child(1)->max_refinement_depth() + 1,
				   child(2)->max_refinement_depth() + 1,
				   child(3)->max_refinement_depth() + 1 };
  return std::max (std::max (depths[0], depths[1]),
		   std::max (depths[2], depths[3]));
}



template <int dim>
inline
bool
TriaObjectAccessor<2,dim>::face_orientation (const unsigned int) const
{
  return true;
}



template <int dim>
inline
void
TriaObjectAccessor<2,dim>::operator ++ ()
{
  ++this->present_index;
				   // is index still in the range of
				   // the vector?
  if (this->present_index
      >=
      static_cast<int>(this->tria->faces->quads.cells.size()))
    this->present_level = this->present_index = -1;
}



template <>
inline
void
TriaObjectAccessor<2,2>::operator ++ ()
{
  ++this->present_index;
				   // is index still in the range of
				   // the vector?
  while (this->present_index
	 >=
	 static_cast<int>(this->tria->levels[this->present_level]->quads.cells.size()))
    {
				       // no -> go one level up
      ++this->present_level;
      this->present_index = 0;
				       // highest level reached?
      if (this->present_level >= static_cast<int>(this->tria->levels.size()))
	{
					   // return with past the end pointer
	  this->present_level = this->present_index = -1;
	  return;
	}
    }
}



template <int dim>
inline
void
TriaObjectAccessor<2,dim>::operator -- ()
{
  --this->present_index;
				   // is index still in the range of
				   // the vector?
  if (this->present_index<0)
    this->present_level = this->present_index = -1;
}


template <>
inline
void
TriaObjectAccessor<2,2>::operator -- ()
{
  --this->present_index;
				   // is index still in the range of
				   // the vector?
  while (this->present_index < 0) 
    {
				       // no -> go one level down
      --this->present_level;
				       // lowest level reached?
      if (this->present_level == -1) 
	{
					   // return with past the end pointer
	  this->present_level = this->present_index = -1;
	  return;
	}
				       // else
      this->present_index = this->tria->levels[this->present_level]->quads.cells.size()-1;
    }
}


/*------------------------ Functions: HexAccessor ---------------------------*/


template <int dim>
inline
TriaObjectAccessor<3,dim>::
TriaObjectAccessor (const Triangulation<dim> *parent,
                    const int                 level,
                    const int                 index,
                    const AccessorData       *local_data)
                :
                TriaAccessor<dim> (parent, level, index, local_data)
{}



template <>
inline
bool
TriaObjectAccessor<3,3>::used () const
{
  const int dim=3;
  Assert (this->state() == IteratorState::valid,
	  TriaAccessor<dim>::ExcDereferenceInvalidObject());
   return this->tria->levels[this->present_level]->hexes.used[this->present_index];
}



template <>
inline
bool
TriaObjectAccessor<3,3>::user_flag_set () const
{
  const int dim=3;
  Assert (this->used(), TriaAccessor<dim>::ExcCellNotUsed());
    return this->tria->levels[this->present_level]->hexes.user_flags[this->present_index];
}



template <>
inline
void
TriaObjectAccessor<3,3>::set_user_flag () const
{
  const int dim=3;
  Assert (this->used(), TriaAccessor<dim>::ExcCellNotUsed());
    this->tria->levels[this->present_level]->hexes.user_flags[this->present_index] = true;
}



template <>
inline
void TriaObjectAccessor<3,3>::clear_user_flag () const
{
  const int dim=3;
  Assert (this->used(), TriaAccessor<dim>::ExcCellNotUsed());
    this->tria->levels[this->present_level]->hexes.user_flags[this->present_index] = false;
}



template <int dim>
inline
TriaIterator<dim,TriaObjectAccessor<1,dim> >
TriaObjectAccessor<3,dim>::line (const unsigned int i) const
{
  Assert (this->used(), typename TriaAccessor<dim>::ExcCellNotUsed());
  return
    TriaIterator<dim,TriaObjectAccessor<1,dim> >
    (
      this->tria,
      0,
      line_index (i)
    );
}



template <int dim>
inline
TriaIterator<dim,TriaObjectAccessor<2,dim> >
TriaObjectAccessor<3,dim>::quad (const unsigned int i) const
{
  Assert (this->used(), typename TriaAccessor<dim>::ExcCellNotUsed());
  return
    TriaIterator<dim,TriaObjectAccessor<2,dim> >
    (
      this->tria,
      0,
      quad_index (i)
    );
}



template <int dim>
inline
unsigned int
TriaObjectAccessor<3,dim>::line_index (const unsigned int i) const
{
  Assert (i<12, ExcIndexRange(i,0,12));

                                   // get the line index by asking the
                                   // quads. make sure we handle
                                   // reverted faces correctly
                                   //
                                   // so set up a table that for each
                                   // line describes a) from which
                                   // quad to take it, b) which line
                                   // therein it is if the face is
                                   // oriented correctly, and c) if in
                                   // the opposite direction
  static const unsigned int lookup_table[12][3] =
    { { 4, 0, 2 }, // take first four lines from bottom face
      { 4, 1, 3 },
      { 4, 2, 0 },
      { 4, 3, 1 },

      { 5, 0, 2 }, // second four lines from top face
      { 5, 1, 3 },
      { 5, 2, 0 },
      { 5, 3, 1 },

      { 0, 0, 2 }, // the rest randomly
      { 1, 0, 2 },
      { 0, 1, 3 },
      { 1, 1, 3 }};

  return (this->quad(lookup_table[i][0])
          ->line_index(face_orientation(lookup_table[i][0]) ?
                       lookup_table[i][1] :
                       lookup_table[i][2]));
}



template <>
inline
unsigned int
TriaObjectAccessor<3,3>::quad_index (const unsigned int i) const
{
  Assert (i<6, ExcIndexRange(i,0,6));

  return this->tria->levels[this->present_level]->hexes.cells[this->present_index].quad(i);
}



template <>
inline
bool
TriaObjectAccessor<3,3>::has_children () const
{
  const int dim=3;
  Assert (this->state() == IteratorState::valid,
	  TriaAccessor<dim>::ExcDereferenceInvalidObject());
    return (this->tria->levels[this->present_level]->hexes.children[this->present_index] != -1);
}


template <>
inline
int TriaObjectAccessor<3,3>::child_index (const unsigned int i) const
{
  const int dim=3;
  Assert (i<8, ExcIndexRange(i,0,8));
  Assert (has_children(), TriaAccessor<dim>::ExcCellHasNoChildren());
    return this->tria->levels[this->present_level]->hexes.children[this->present_index]+i;
}



template <>
inline
TriaIterator<3,TriaObjectAccessor<3,3> >
TriaObjectAccessor<3,3>::child (const unsigned int i) const
{
  const int dim=3;
  Assert (i<8, ExcIndexRange(i,0,8));
  
  TriaIterator<dim,TriaObjectAccessor<3,dim> > q (this->tria, this->present_level+1, child_index (i));
  
  Assert ((q.state() == IteratorState::past_the_end) || q->used(),
	  TriaAccessor<dim>::ExcUnusedCellAsChild());

  return q;
}



template <int dim>
inline
unsigned int
TriaObjectAccessor<3,dim>::n_children () const
{
  Assert (has_children()==true, TriaAccessor<3>::ExcCellHasNoChildren());
  return GeometryInfo<3>::children_per_cell;
}


template <int dim>
inline
unsigned int
TriaObjectAccessor<3,dim>::max_refinement_depth () const
{
  if (!has_children())
    return 0;

  const unsigned int depths[8] = { child(0)->max_refinement_depth() + 1,
				   child(1)->max_refinement_depth() + 1,
				   child(2)->max_refinement_depth() + 1,
				   child(3)->max_refinement_depth() + 1,
				   child(4)->max_refinement_depth() + 1,
				   child(5)->max_refinement_depth() + 1,
				   child(6)->max_refinement_depth() + 1,
				   child(7)->max_refinement_depth() + 1  };
  return std::max (std::max (std::max (depths[0], depths[1]),
			     std::max (depths[2], depths[3])),
		   std::max (std::max (depths[4], depths[5]),
			     std::max (depths[6], depths[7])));
}



template <>
inline
bool
TriaObjectAccessor<3, 3>::
face_orientation (const unsigned int face) const
{
  const int dim=3;
  Assert (used(), TriaAccessor<dim>::ExcCellNotUsed());
  Assert (face<GeometryInfo<3>::faces_per_cell,
          ExcIndexRange (face, 0, GeometryInfo<3>::faces_per_cell));
      Assert (this->present_index * GeometryInfo<3>::faces_per_cell + face
	      < this->tria->levels[this->present_level]
	      ->hexes.face_orientations.size(),
	      ExcInternalError());
      
      return (this->tria->levels[this->present_level]
	      ->hexes.face_orientations[this->present_index *
					GeometryInfo<3>::faces_per_cell
					+ face]);
}



template <>
inline
void
TriaObjectAccessor<3,3>::operator ++ ()
{
  ++this->present_index;
				   // is index still in the range of
				   // the vector?
    while (this->present_index
	   >=
	   static_cast<int>(this->tria->levels[this->present_level]->hexes.cells.size()))
      {
					 // no -> go one level up
	++this->present_level;
	this->present_index = 0;
					 // highest level reached?
	if (this->present_level >= static_cast<int>(this->tria->levels.size()))
	  {
					     // return with past the end pointer
	    this->present_level = this->present_index = -1;
	    return;
	  }
      }
}



template <>
inline
void
TriaObjectAccessor<3,3>::operator -- ()
{
  --this->present_index;
				   // is index still in the range of
				   // the vector?
    while (this->present_index < 0) 
      {
					 // no -> go one level down
	--this->present_level;
					 // lowest level reached?
	if (this->present_level == -1) 
	  {
					     // return with past the end pointer
	    this->present_level = this->present_index = -1;
	    return;
	  }
					 // else
	this->present_index = this->tria->levels[this->present_level]->hexes.cells.size()-1;
      }
}


/*------------------------ Functions: CellAccessor<dim> -----------------------*/


template <int dim>
inline
CellAccessor<dim>::
CellAccessor (const Triangulation<dim> *parent,
              const int                 level,
              const int                 index,
              const AccessorData       *local_data)
                :
                TriaObjectAccessor<dim,dim> (parent, level, index, local_data)
{}



template <>
inline
TriaIterator<1,TriaObjectAccessor<0, 1> >
CellAccessor<1>::face (const unsigned int) const 
{
  Assert (false, ExcImpossibleInDim(1));
  return TriaIterator<1,TriaObjectAccessor<0, 1> >();
}



template <>
inline
Triangulation<2>::face_iterator
CellAccessor<2>::face (const unsigned int i) const 
{
  return this->line(i);
}



template <>
inline
Triangulation<3>::face_iterator
CellAccessor<3>::face (const unsigned int i) const 
{
  return this->quad(i);
}



template <int dim>
inline
int
CellAccessor<dim>::neighbor_index (const unsigned int i) const 
{
  Assert (i<GeometryInfo<dim>::faces_per_cell,
	  typename TriaAccessor<dim>::ExcInvalidNeighbor(i));
  return this->tria->levels[this->present_level]->
    neighbors[this->present_index*GeometryInfo<dim>::faces_per_cell+i].second;
}



template <int dim>
inline
int
CellAccessor<dim>::neighbor_level (const unsigned int i) const
{
  Assert (i<GeometryInfo<dim>::faces_per_cell,
	  typename TriaAccessor<dim>::ExcInvalidNeighbor(i));
  return this->tria->levels[this->present_level]->
    neighbors[this->present_index*GeometryInfo<dim>::faces_per_cell+i].first;
}



template <int dim>
inline
bool
CellAccessor<dim>::refine_flag_set () const
{
  Assert (this->used(), typename TriaAccessor<dim>::ExcCellNotUsed());
				   // cells flagged for refinement must be active
				   // (the @p set_refine_flag function checks this,
				   // but activity may change when refinement is
				   // executed and for some reason the refine
				   // flag is not cleared).
  Assert (this->active() ||  !this->tria->levels[this->present_level]->refine_flags[this->present_index],
	  ExcRefineCellNotActive());
  return this->tria->levels[this->present_level]->refine_flags[this->present_index];
}



template <int dim>
inline
void
CellAccessor<dim>::set_refine_flag () const
{
  Assert (this->used() && this->active(), ExcRefineCellNotActive());
  Assert (!coarsen_flag_set(),
	  ExcCellFlaggedForCoarsening());
  
  this->tria->levels[this->present_level]->refine_flags[this->present_index] = true;
}



template <int dim>
inline
void
CellAccessor<dim>::clear_refine_flag () const
{
  Assert (this->used() && this->active(), ExcRefineCellNotActive());
  this->tria->levels[this->present_level]->refine_flags[this->present_index] = false;
}



template <int dim>
inline
bool
CellAccessor<dim>::coarsen_flag_set () const
{
  Assert (this->used(), typename TriaAccessor<dim>::ExcCellNotUsed());
				   // cells flagged for coarsening must be active
				   // (the @p set_refine_flag function checks this,
				   // but activity may change when refinement is
				   // executed and for some reason the refine
				   // flag is not cleared).
  Assert (this->active() ||  !this->tria->levels[this->present_level]->coarsen_flags[this->present_index],
	  ExcRefineCellNotActive());
  return this->tria->levels[this->present_level]->coarsen_flags[this->present_index];
}



template <int dim>
inline
void
CellAccessor<dim>::set_coarsen_flag () const
{
  Assert (this->used() && this->active(), ExcRefineCellNotActive());
  Assert (!refine_flag_set(), ExcCellFlaggedForRefinement());
  
  this->tria->levels[this->present_level]->coarsen_flags[this->present_index] = true;
}



template <int dim>
inline
void
CellAccessor<dim>::clear_coarsen_flag () const
{
  Assert (this->used() && this->active(), ExcRefineCellNotActive());
  this->tria->levels[this->present_level]->coarsen_flags[this->present_index] = false;
}



template <int dim>
inline
TriaIterator<dim,CellAccessor<dim> >
CellAccessor<dim>::neighbor (const unsigned int i) const
{
  TriaIterator<dim,CellAccessor<dim> >
    q (this->tria, neighbor_level (i), neighbor_index (i));

  Assert ((q.state() == IteratorState::past_the_end) || q->used(),
	  typename TriaAccessor<dim>::ExcUnusedCellAsNeighbor());

  return q;
}



template <int dim>
inline
TriaIterator<dim,CellAccessor<dim> >
CellAccessor<dim>::child (const unsigned int i) const
{
  TriaIterator<dim,CellAccessor<dim> >
    q (this->tria, this->present_level+1, this->child_index (i));

  Assert ((q.state() == IteratorState::past_the_end) || q->used(),
	  typename TriaAccessor<dim>::ExcUnusedCellAsChild());

  return q;
}



template <int dim>
inline
bool
CellAccessor<dim>::active () const
{
  return !this->has_children();
}


#endif
