//----------------------------  grid_reordering.cc  ---------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 2000 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  grid_reordering.cc  ---------------------------


#include <algorithm>
#include <grid/grid_reordering.h>




template <int dim>
GridReordering<dim>::Cell::Cell () :
		cell_no (invalid_neighbor)
{
  for (unsigned int i=0; i<GeometryInfo<dim>::faces_per_cell; ++i)
    neighbors[i] = invalid_neighbor;
};



template <int dim>
GridReordering<dim>::Cell::Cell (const CellData<dim> &cd,
				 const unsigned int   cell_no) :
		CellData<dim> (cd), cell_no(cell_no)
{
  for (unsigned int i=0; i<GeometryInfo<dim>::faces_per_cell; ++i)
    neighbors[i] = invalid_neighbor;
};



template <int dim>
inline
unsigned int GridReordering<dim>::Cell::count_neighbors () const
{
  unsigned int n = 0;
  for (unsigned int i=0; i<GeometryInfo<dim>::faces_per_cell; ++i)
    if (neighbors[i] != invalid_neighbor)
      ++n;
  return n;
};



template <int dim>
void
GridReordering<dim>::Cell::insert_faces (map<Face,FaceData> &/*global_faces*/)
{
  Assert (false, ExcNotImplemented());
};


#if deal_II_dimension == 2

template <>
void
GridReordering<2>::Cell::insert_faces (map<Face,FaceData> &global_faces)
{
  const unsigned int dim = 2;

				   // first compute index numbers for
				   // the faces in usual order as
				   // defined by the order of vertices
				   // in the cell object
  Face new_faces[GeometryInfo<dim>::faces_per_cell]
    = { { {vertices[0], vertices[1]} },
        { {vertices[1], vertices[2]} },
	{ {vertices[3], vertices[2]} },
	{ {vertices[0], vertices[3]} } };

				   // then insert them into the global
				   // list and store iterators to
				   // them. note that if the face
				   // already exists, then the stored
				   // data is not touched.
  for (unsigned int face=0; face<GeometryInfo<dim>::faces_per_cell; ++face)
    faces[0][face] = global_faces.insert (make_pair(new_faces[face],
						    FaceData())).first;


				   // then for each of the faces also
				   // insert the reverse form and
				   // store pointers to them. note
				   // that the rotational state in
				   // which all faces are reverted is
				   // `2'
  for (unsigned int face=0; face<GeometryInfo<dim>::faces_per_cell; ++face)
    {
      swap (new_faces[face].vertices[0],
	    new_faces[face].vertices[1]);
      faces[2][face] = global_faces.insert (make_pair(new_faces[face],
						      FaceData())).first;
    };

				   // then finally fill in rotational
				   // states 1 and 3 of the cell. the
				   // faces of these states can be
				   // obtained from states 0 and 2
  faces[1][0] = faces[2][0];
  faces[1][1] = faces[0][1];
  faces[1][2] = faces[2][2];
  faces[1][3] = faces[0][3];
  
  faces[3][0] = faces[0][0];
  faces[3][1] = faces[2][1];
  faces[3][2] = faces[0][2];
  faces[3][3] = faces[2][3];
  

				   // finally fill the crosslink and
				   // other fields of the new
				   // entries. note that since
				   // rotational states 0 and 2 of the
				   // cell are exactly reverted, we
				   // only have to operate on the face
				   // pointers of these two states to
				   // reach all possible faces and
				   // permutations thereof
  for (unsigned int face=0; face<GeometryInfo<dim>::faces_per_cell; ++face)
    {
      if (faces[0][face]->second.adjacent_cells[0] ==
	  FaceData::invalid_adjacent_cell)
	{
					   // face had not been
					   // inserted by previous
					   // cells, since first
					   // adjacent cell is still
					   // untouched. provide
					   // xlinks to rotated faces
	  faces[0][face]->second.reverse_faces[0] = faces[2][face];
	  faces[2][face]->second.reverse_faces[0] = faces[0][face];

					   // and insert this cell as
					   // adjacent_cell of the faces
	  faces[0][face]->second.adjacent_cells[0] = cell_no;
	  faces[2][face]->second.adjacent_cells[0] = cell_no;
	}
      else
	{
					   // face had already been
					   // inserted. make sure that
					   // it was in the same way:
	  Assert (faces[0][face]->second.reverse_faces[0] == faces[2][face],
		  ExcInternalError());	  
	  Assert (faces[2][face]->second.reverse_faces[0] == faces[0][face],
		  ExcInternalError());

					   // now insert ourselves as
					   // second
					   // adjacent_cell. the
					   // respective slots must
					   // necessarily be empty
					   // still
	  Assert (faces[0][face]->second.adjacent_cells[1] ==
		  FaceData::invalid_adjacent_cell,
		  ExcInternalError());
	  Assert (faces[2][face]->second.adjacent_cells[1] ==
		  FaceData::invalid_adjacent_cell,
		  ExcInternalError());
	  faces[0][face]->second.adjacent_cells[1] = cell_no;
	  faces[2][face]->second.adjacent_cells[1] = cell_no;
	};
    };
};

#endif

#if deal_II_dimension == 3

template <>
void
GridReordering<3>::Cell::insert_faces (map<Face,FaceData> &global_faces)
{
  const unsigned int dim = 3;

				   // first generate for each of the 6
				   // faces of a cell in 3d the four
				   // possible orientations and
				   // cross-link them among each other
				   //
				   // do this generation step by first
				   // only inserting each face in
				   // standard orientation and then
				   // fill in the other ones by
				   // rotation of these faces
				   //
				   // note that we have the indices
				   // reversed here compared to the
				   // Cell class, for simplicity
  const Face new_faces_tmp[GeometryInfo<dim>::faces_per_cell]
    = { { { vertices[0], vertices[1], vertices[2], vertices[3] } },
	{ { vertices[4], vertices[5], vertices[6], vertices[7] } },
	{ { vertices[0], vertices[1], vertices[5], vertices[4] } },
	{ { vertices[1], vertices[5], vertices[6], vertices[2] } },
	{ { vertices[3], vertices[2], vertices[6], vertices[7] } },
	{ { vertices[0], vertices[4], vertices[7], vertices[3] } } };
  Face new_faces[GeometryInfo<dim>::faces_per_cell][rotational_states_of_faces]
    = { { new_faces_tmp[0], new_faces_tmp[0], new_faces_tmp[0], new_faces_tmp[0] },
	{ new_faces_tmp[1], new_faces_tmp[0], new_faces_tmp[0], new_faces_tmp[0] },
	{ new_faces_tmp[2], new_faces_tmp[0], new_faces_tmp[0], new_faces_tmp[0] },
	{ new_faces_tmp[3], new_faces_tmp[0], new_faces_tmp[0], new_faces_tmp[0] },
	{ new_faces_tmp[4], new_faces_tmp[0], new_faces_tmp[0], new_faces_tmp[0] },
	{ new_faces_tmp[5], new_faces_tmp[0], new_faces_tmp[0], new_faces_tmp[0] } };
    
  for (unsigned int face=0; face<GeometryInfo<dim>::faces_per_cell; ++face)
    for (unsigned int rot=1; rot<rotational_states_of_faces; ++rot)
      for (unsigned int v=0; v<GeometryInfo<dim>::vertices_per_face; ++v)
	new_faces[face][rot].vertices[v]
	  = new_faces[face][0].vertices[(v+rot) % GeometryInfo<dim>::vertices_per_face];

				   // now insert all the faces, by now
				   // without specific order with
				   // respect to the orientational
				   // states of the cell. note that we
				   // get the indices correct
				   // here. also remark that the face
				   // might already have been in the
				   // map, depending on whether a
				   // newighbor has already inserted
				   // it or not. we don't care about
				   // that here, though
  map<Face,FaceData>::iterator
    new_faces_ptr[rotational_states_of_faces][GeometryInfo<dim>::faces_per_cell];
  for (unsigned int face=0; face<GeometryInfo<dim>::faces_per_cell; ++face)
    for (unsigned int rot=0; rot<rotational_states_of_faces; ++rot)
      new_faces_ptr[rot][face]
	= global_faces.insert (make_pair(new_faces[face][rot], FaceData())).first;
  
				   // and crosslink them to each other
  for (unsigned int face=0; face<GeometryInfo<dim>::faces_per_cell; ++face)
    for (unsigned int rot=0; rot<rotational_states_of_faces; ++rot)
      for (unsigned int other_rot=0; other_rot<rotational_states_of_faces; ++other_rot)
	{
	  if (other_rot < rot)
	    new_faces_ptr[rot][face]->second.reverse_faces[other_rot]
	      = new_faces_ptr[other_rot][face];
	  else
	    if (other_rot > rot)
	      new_faces_ptr[rot][face]->second.reverse_faces[other_rot-1]
		= new_faces_ptr[other_rot][face];
					   // if rot==other_rot, then
					   // we need not link this
					   // cell to itself
	};
  

				   // for each of the faces (whether
				   // already inserted or not) note
				   // that the present cell is one of
				   // the neighbors
  for (unsigned int face=0; face<GeometryInfo<dim>::faces_per_cell; ++face)
    for (unsigned int rot=0; rot<rotational_states_of_faces; ++rot)
      if (new_faces_ptr[rot][face]->second.adjacent_cells[0] ==
	  FaceData::invalid_adjacent_cell)
					 // no, faces had not been
					 // used before, so we are the
					 // first adjacent cell
	new_faces_ptr[rot][face]->second.adjacent_cells[0] = cell_no;
      else
					 // otherwise: cell had been
					 // entered before, so we are
					 // the second neighbor
	{
	  Assert (new_faces_ptr[rot][face]->second.adjacent_cells[1] ==
		  FaceData::invalid_adjacent_cell,
		  ExcInternalError());
	  new_faces_ptr[rot][face]->second.adjacent_cells[1] = cell_no;
	};

				   // still have to link cell in
				   // different orientations to the
				   // faces
  
  Assert (false, ExcNotImplemented());
};

#endif


template <int dim>
void GridReordering<dim>::Cell::fix_cell_neighbors ()
{
  for (unsigned int face=0; face<GeometryInfo<dim>::faces_per_cell; ++face)
    {
				       // first assert that the
				       // neighborship info of all
				       // versions of the same face is
				       // identical
      for (unsigned int rot=1; rot<rotational_states_of_faces; ++rot)
	for (unsigned int adjacent_cell=0; adjacent_cell<2; ++adjacent_cell)
	  Assert (faces[rot][face]->second.adjacent_cells[adjacent_cell]
		  ==
		  faces[0][face]->second.adjacent_cells[adjacent_cell],
		  ExcInternalError());
      

				       // then insert the neighbor
				       // behind this face as neighbor
				       // of the present cell. note
				       // that it is not relevant to
				       // which permutation of a face
				       // we refer. note that it might
				       // well be that some of the
				       // neighbor indices are
				       // FaceData::invalid_adjacent_cell
      if (faces[0][face]->second.adjacent_cells[0] == cell_no)
	neighbors[face] = faces[0][face]->second.adjacent_cells[1];
      else
	neighbors[face] = faces[0][face]->second.adjacent_cells[0];
    };
};



template <int dim>
void GridReordering<dim>::Cell::find_backtracking_point ()
{
				   // we know what neighbors we have,
				   // we can determine the neighbor
				   // with the maximal cell_no that is
				   // smaller than that of the present
				   // cell. we need this information
				   // in the backtracking process and
				   // don't want to compute it every
				   // time again
  track_back_to_cell = FaceData::invalid_adjacent_cell;
  for (unsigned int face=0; face<GeometryInfo<dim>::faces_per_cell; ++face)
    if ((neighbors[face] != FaceData::invalid_adjacent_cell)
	&&
	(neighbors[face] < cell_no)
	&&
	((neighbors[face] > track_back_to_cell)
	 ||
	 (track_back_to_cell == FaceData::invalid_adjacent_cell)))
      track_back_to_cell = neighbors[face];

				   // if this cell had no neighbors
				   // with lower cell numbers, we
				   // still need to know what cell to
				   // track back to in case some
				   // higher cell than the present one
				   // failed to coexist with the
				   // existing part of the mesh
				   // irrespective of the rotation
				   // state of this present cell. we
				   // then simply track back to the
				   // cell before this one, lacking a
				   // better alternative. this does,
				   // of course, not hold for cell 0,
				   // from which we should never be
				   // forced to track back
  if (cell_no == 0)
    track_back_to_cell = 0;
  else
    if (track_back_to_cell == FaceData::invalid_adjacent_cell)
      track_back_to_cell = cell_no-1;
};



template <int dim>
inline
bool GridReordering<dim>::Cell::check_consistency (const unsigned int rot) const
{
				   // make sure that for each face of
				   // the cell the permuted faces are
				   // not already in use, as that
				   // would make the cell disallowed
  for (unsigned int face_no=0; face_no<GeometryInfo<dim>::faces_per_cell; ++face_no)
    {
      const FaceData &face = faces[rot][face_no]->second;

      for (unsigned int face_rot=0; face_rot<rotational_states_of_faces-1; ++face_rot)
	{
	  const FaceData &reverse_face = face.reverse_faces[face_rot]->second;
	  if (reverse_face.use_count != 0)
	    return false;
	};
    };

				   // no conflicts found
  return true;
};



template <int dim>
inline
void GridReordering<dim>::Cell::mark_faces_used (const unsigned int rot)
{
  for (unsigned int face=0; face<GeometryInfo<dim>::faces_per_cell; ++face)
    {
      Assert (faces[rot][face]->second.use_count < 2,
	      ExcInternalError());
      ++faces[rot][face]->second.use_count;
    };
};



template <int dim>
inline
void GridReordering<dim>::Cell::mark_faces_unused (const unsigned int rot)
{
  for (unsigned int face=0; face<GeometryInfo<dim>::faces_per_cell; ++face)
    {
      Assert (faces[rot][face]->second.use_count > 0,
	      ExcInternalError());
      --faces[rot][face]->second.use_count;
    };
};



template <int dim>
bool GridReordering<dim>::Face::operator < (const Face &face) const
{
  for (unsigned int v=0; v<GeometryInfo<dim>::vertices_per_face; ++v)
    {
				       // if vertex index is smaller,
				       // then comparison is true
      if (vertices[v] < face.vertices[v])
	return true;
      else
					 // if vertex index is greater,
					 // then comparison is false
	if (vertices[v] > face.vertices[v])
	  return false;
				       // if indices are equal, then test
				       // next index
    };

				   // if all indices are equal:
  return false;
};



template <int dim>
GridReordering<dim>::FaceData::FaceData () :
		use_count (0)
{
  adjacent_cells[0] = adjacent_cells[1] = invalid_adjacent_cell;
};






template <int dim>
inline
void GridReordering<dim>::track_back (vector<Cell>  &cells,
				      RotationStack &rotation_states,
				      unsigned int   track_back_to_cell)
{
  top_of_function:
  
  Assert (track_back_to_cell > 0, ExcInternalError());

  unsigned int last_rotation_state = static_cast<unsigned int>(-1);
  for (unsigned int cell_no=rotation_states.size()-1; cell_no>=track_back_to_cell; --cell_no)
    {
				       // store rotation state of
				       // topmost cell, as we will
				       // have to advance that by one
      last_rotation_state = rotation_states.back();
      
				       // first mark faces of that
				       // cell as no more used
      cells[cell_no].mark_faces_unused (last_rotation_state);

				       // then pop state from
				       // stack
      rotation_states.pop_back();
    };
  Assert (last_rotation_state < rotational_states_of_cells, ExcInternalError());
  
				   // now we will have to find out
				   // whether we can try the last cell
				   // we have popped from the stack in
				   // another rotation state, or will
				   // have to backtrack further:
  if (last_rotation_state < rotational_states_of_cells-1)
    {
				       // possible. push that state to
				       // the stack and leave
      rotation_states.push_back (last_rotation_state+1);
      return;
    }
  else
    {
				       // last cell can't be rotated
				       // further. go on with
				       // backtracking
      const typename vector<Cell>::iterator
	try_cell = cells.begin() + rotation_states.size();
      
      track_back_to_cell = try_cell->track_back_to_cell;
      
//        cout << "Further backtracking from " << rotation_states.size()
//  	   << " to " << track_back_to_cell << endl;

      Assert (track_back_to_cell > 0, ExcInternalError());

				       // track further back. this
				       // could be done by recursive
				       // calls of this function,
				       // which in this case would
				       // represent a tail-recursion
				       // as there is nothing more to
				       // be done after calling the
				       // function recursively, but we
				       // prefer to write down the
				       // tail-recursion by hand using
				       // a goto, since the compiler
				       // seems to have problems to
				       // rewrite the tail recursion
				       // as a goto.
      goto top_of_function;
    };
};



template <int dim>
bool GridReordering<dim>::try_rotate_single_neighbors (vector<Cell>  &cells,
						       RotationStack &rotation_states)
{
				   // the rotation state of the cell
				   // which we try to add by rotating
				   // neighbors has already been
				   // popped from the stack, so we get
				   // its number like this:
  const unsigned int cell_no = rotation_states.size();

//  cout << "Trying to rotate neighbors of cell " << cell_no << ". ";
  
				   // now try each of the neighbors
				   // that have already been added to
				   // the grid. don't try the cell
				   // that we will track back to
				   // anyway if this operation should
				   // fail
  for (unsigned int neighbor=0; neighbor<GeometryInfo<dim>::faces_per_cell; ++neighbor)
    if (cells[cell_no].neighbors[neighbor] < cell_no)
      if (cells[cell_no].neighbors[neighbor] != cells[cell_no].track_back_to_cell)
	{
	  const unsigned int neighbor_no = cells[cell_no].neighbors[neighbor];
	  const unsigned int old_rotation_state = rotation_states[neighbor_no];
	  
					   // unlink faces used by the
					   // present rotation state
	  cells[neighbor_no].mark_faces_unused (old_rotation_state);

					   // then try all rotation
					   // states besides the ones
					   // that have already been
					   // tried:
	  for (unsigned int neighbor_rot=old_rotation_state+1;
	       neighbor_rot<rotational_states_of_cells; ++neighbor_rot)
	    {
					       // first, if the
					       // neighbor itself does
					       // not fit in the grid,
					       // then there is
					       // nothing to do
	      if (! cells[neighbor_no].check_consistency (neighbor_rot))
		continue;

					       // however, if the
					       // neighbor worked,
					       // then mark its faces
					       // as used
					       // preliminarily and
					       // try to fit in the
					       // present cell in some
					       // orientation
	      cells[neighbor_no].mark_faces_used (neighbor_rot);

	      for (unsigned int cell_rot=0; cell_rot<rotational_states_of_cells; ++cell_rot)
		if (cells[cell_no].check_consistency (cell_rot) == true)
		  {
						     // ah, see,
						     // this
						     // combination
						     // of neighbor
						     // rotation and
						     // this cell
						     // works. enter
						     // the
						     // respective
						     // states into
						     // the arrays
						     // and leave
						     // with success
		    rotation_states[neighbor_no] = neighbor_rot;
		    
		    rotation_states.push_back (cell_rot);
		    cells[cell_no].mark_faces_used (cell_rot);

//  		    cout << "Successfully tried neighbor " << neighbor_no
//  			 << " in rot=" << neighbor_rot
//  			 << " with rot=" << cell_rot << endl;

		    return true;
		  };
	      
					       // no, there was no
					       // way to fit the
					       // present cell into
					       // the grid given
					       // this orientation
					       // of the
					       // neighbor. discard
					       // this attempt and
					       // try that neighbors
					       // next rotation
	      cells[neighbor_no].mark_faces_unused (neighbor_rot);
	    };
	  
					   // there was no way to
					   // rotate this neighbor so
					   // that the present cell
					   // fit into the
					   // grid. reinstantiate the
					   // old state and go on to
					   // the next neighbor
	  cells[neighbor_no].mark_faces_used (old_rotation_state);
	};

				   // rotation of neighbors did not
				   // help this cell, there is no
				   // other way than to do a full
				   // backtracking
//  cout << "Unsuccessfully tried to rotate neighbors of cell " << cell_no << endl;
  return false;
};



template <int dim>
void GridReordering<dim>::find_reordering (vector<Cell<dim> >         &cells,
					   vector<CellData<dim> >     &original_cells,
					   const vector<unsigned int> &new_cell_numbers)
{
  cout << "Starting..." << flush;
  
  const unsigned int n_cells = cells.size();
  
				   // stack of value indicating that
				   // the nth cell needs to be rotated
				   // so-and-so often, where n is the
				   // position on the stack
  RotationStack rotation_states;

				   // for the first cell, the
				   // rotational state can never be
				   // important, since we can rotate
				   // all other cells
				   // accordingly. therefore preset
				   // the rotation state of the first
				   // cell
  rotation_states.push_back (0);
  cells[0].mark_faces_used (rotation_states.back());
  
  while (true)
    {
				       // if all cells have a coherent
				       // orientation, then we can
				       // exit the main loop
      if (rotation_states.size() == n_cells)
	break;
      
				       // try to push back another
				       // cell in orientation zero
      rotation_states.push_back (0);

				       // check whether the present
				       // cell in the present
				       // orientation is valid
      check_topmost_cell:

      static unsigned int max_size = 0;
      if (rotation_states.size() > max_size)
	{
	  max_size = rotation_states.size();
  	  if (max_size % 10 == 0)
  	    cout << "New max size " << rotation_states.size() << endl;
	};
      
      const typename vector<Cell>::iterator
	try_cell = cells.begin() + rotation_states.size()-1;
      if (try_cell->check_consistency (rotation_states.back()))
	{
					   // yes, works, we found a
					   // way of how to add the
					   // present cell to the
					   // existing cells without
					   // violating any ordering
					   // constraints. now mark
					   // the respective faces as
					   // used and go on with the
					   // next cell
	  try_cell->mark_faces_used (rotation_states.back());
	  
//  	  cout << "Added cell " << try_cell->cell_no
//  	       << " in rotation " << rotation_states.back() << endl;
					   // go on with next cell
	  continue;
	}
      else
	{
					   // no, doesn't work. see if
					   // we can rotate the top
					   // cell so that it works
	  if (rotation_states.back()+1 < rotational_states_of_cells)
	    {
					       // yes, can be
					       // done. then do so and
					       // check again
	      ++rotation_states.back();
	      goto check_topmost_cell;
	    }
	  else
	    {
					       // no, no more
					       // orientation of the
					       // top cell possible,
					       // we have to backtrack
					       // some way
//	      cout << "Failure with cell " << rotation_states.size()-1 << endl;

					       // first pop rotational
					       // state of top cell,
					       // since for that no
					       // faces have been
					       // marked as used yet
	      rotation_states.pop_back();

					       // in general, if we
					       // fail to insert the
					       // present cell somehow
					       // into the existing
					       // part of the grid,
					       // then we track back
					       // to the neighbor of
					       // the failed cell with
					       // the highest cell
					       // index below the
					       // index of the present
					       // cell. however,
					       // before we do so, we
					       // try a simple
					       // heuristic: if
					       // rotating single
					       // neighbors a little
					       // helps the process
					       // somewhat:
	      const bool rotation_helps
		= try_rotate_single_neighbors (cells, rotation_states);

					       // if rotation helped,
					       // then go on to the
					       // next cell. the
					       // called function has
					       // already marked the
					       // respective faces as
					       // used and has pushed
					       // the rotation state
					       // of the present cell
					       // to the stack
	      if (rotation_helps == true)
		continue;

//	      cout << "Will track back to cell " << try_cell->track_back_to_cell << endl;

					       // if that failed to
					       // help, then track
					       // back
	      track_back (cells, rotation_states, try_cell->track_back_to_cell);
					       // and go on by
					       // checking the now
					       // topmost cell
	      goto check_topmost_cell;
	    };
	};
    };


//    for (unsigned int i=0; i<cells.size(); ++i)
//      cout << "{" << i << ':' << rotation_states[i] << "}";

				   // rotate the cells according to
				   // the results we have found. since
				   // we operate on a stack, we do the
				   // rotations from the back of the
				   // array to the front
  while (rotation_states.size() != 0)
    {
      const unsigned int
	new_cell_number = rotation_states.size()-1;
      const unsigned int
	old_cell_number = find (new_cell_numbers.begin(),
				new_cell_numbers.end(),
				new_cell_number) - new_cell_numbers.begin();
      Assert (old_cell_number < cells.size(), ExcInternalError());

      original_cells[old_cell_number].rotate (rotation_states.back());

				       // to check the correctness of
				       // the program up to here:
				       // unmark the cells' faces to
				       // check whether they have all
				       // correctly declared they
				       // use. checking this is done
				       // in the calling function, as
				       // only that has direct access
				       // to the map of faces (this
				       // function only accesses it
				       // through pointers stored in
				       // the cells)
      cells[new_cell_number].mark_faces_unused (rotation_states.back());

				       // then delete this rotational
				       // state as we don't need it
				       // any more
      rotation_states.pop_back ();
    };

  cout << "Done!" << endl;
};



template <int dim>
vector<unsigned int>
GridReordering<dim>::presort_cells (vector<Cell<dim> > &cells,
				    map<Face,FaceData> &faces)
{
				   // first find the cell with the
				   // least neighbors
  unsigned int min_neighbors           = cells[0].count_neighbors();
  unsigned int cell_with_min_neighbors = 0;
  for (unsigned int i=1; i<cells.size(); ++i)
    if (min_neighbors > cells[i].count_neighbors())
      {  
	min_neighbors = cells[i].count_neighbors();
	cell_with_min_neighbors = i;
	if (min_neighbors == 1)
					   // better is not possible
	  break;
      };

				   // have an array into which we
				   // insert the new cells numbers of
				   // each cell
  const unsigned int invalid_cell_number = static_cast<unsigned int>(-1);
  vector<unsigned int> new_cell_numbers (cells.size(), invalid_cell_number);

				   // and have an array of the next
				   // cells to be numbered (old numbers)
  vector<unsigned int> next_round_cells (1, cell_with_min_neighbors);

  unsigned int next_free_new_number = 0;
  
				   // while there are still cells to
				   // be renumbered:
  while (next_round_cells.size() != 0)
    {
      for (unsigned int i=0; i<next_round_cells.size(); ++i)
	{
	  Assert (new_cell_numbers[next_round_cells[i]] == invalid_cell_number,
		  ExcInternalError());
	  
	  new_cell_numbers[next_round_cells[i]] = next_free_new_number;
	  ++next_free_new_number;
	};

				       // for the next round, find all
				       // neighbors of the cells of
				       // this round which have not
				       // yet been renumbered
      vector<unsigned int> new_next_round_cells;
      for (unsigned int i=0; i<next_round_cells.size(); ++i)
	for (unsigned int n=0; n<GeometryInfo<dim>::faces_per_cell; ++n)
	  if (cells[next_round_cells[i]].neighbors[n] != Cell<dim>::invalid_neighbor)
	    if (new_cell_numbers[cells[next_round_cells[i]].neighbors[n]]
		== invalid_cell_number)
	      new_next_round_cells.push_back (cells[next_round_cells[i]].neighbors[n]);

	    
      
				       // if no more cells have been
				       // found, then we must have
				       // renumbered all cells already
      if (new_next_round_cells.size() == 0)
	Assert (next_free_new_number == cells.size(), ExcInternalError());

				       // eliminate duplicates from
				       // the new_next_round_cells
				       // array. note that a cell
				       // which is entered into this
				       // array might have been
				       // entered more than once since
				       // it might be a neighbor of
				       // more than one cell of the
				       // present round
				       //
				       // in order to eliminate
				       // duplicates, we first sort
				       // tha array and then copy over
				       // only unique elements to the
				       // next_round_cells array,
				       // which is needed for the next
				       // loop iteration anyway
      sort (new_next_round_cells.begin(), new_next_round_cells.end());
      next_round_cells.clear ();
      unique_copy (new_next_round_cells.begin(), new_next_round_cells.end(),
		   back_inserter(next_round_cells));
    };
  Assert (find (new_cell_numbers.begin(), new_cell_numbers.end(), invalid_cell_number)
	  ==
	  new_cell_numbers.end(),
	  ExcInternalError());

				   // now that we know in which order
				   // to sort the cells, do so:
  vector<Cell<dim> > new_cells (cells.size());
  for (unsigned int i=0; i<cells.size(); ++i)
    new_cells[new_cell_numbers[i]] = cells[i];
				   // then switch old and new array
  swap (cells, new_cells);
  
				   // now we still have to convert all
				   // old cell numbers to new cells
				   // numbers
  for (unsigned int c=0; c<cells.size(); ++c)
    {
      cells[c].cell_no = new_cell_numbers[cells[c].cell_no];
      Assert (cells[c].cell_no == c, ExcInternalError());

      for (unsigned int n=0; n<GeometryInfo<dim>::faces_per_cell; ++n)
	cells[c].neighbors[n] = new_cell_numbers[cells[c].neighbors[n]];
    };

  for (typename map<Face,FaceData>::iterator i=faces.begin(); i!=faces.end(); ++i)
    for (unsigned int k=0; k<2; ++k)
      if (i->second.adjacent_cells[k] != FaceData::invalid_adjacent_cell)
	i->second.adjacent_cells[k] = new_cell_numbers[i->second.adjacent_cells[k]];

//    for (unsigned int i=0; i<cells.size(); ++i)
//      cout << "[" << i << "->" << new_cell_numbers[i] << "]";
  
  return new_cell_numbers;
};

		      

template <int dim>
void GridReordering<dim>::reorder_cells (vector<CellData<dim> > &original_cells)
{
				   // the present function might
				   // actually work in 3d, but the
				   // ones it calls probably not. will
				   // have to think about what needs
				   // to be changed. however, rather
				   // than killing the program when
				   // calling functions that may not
				   // work, kill it here if the
				   // dimension is not appropriate
  Assert (dim==2, ExcNotImplemented());
  
				   // we need more information than
				   // provided by the input parameter,
				   // in particular we need
				   // neighborship relations between
				   // cells. therefore copy over the
				   // old cells to another class that
				   // provides space to these
				   // informations
  vector<Cell<dim> > cells;
  cells.reserve (original_cells.size());
  for (unsigned int i=0; i<original_cells.size(); ++i)
    cells.push_back (Cell<dim>(original_cells[i], i));
  
				   // first generate all the faces
				   // possible, i.e. in each possible
				   // direction and rotational state
  map<Face,FaceData> faces;
  for (unsigned int cell_no=0; cell_no<cells.size(); ++cell_no)
    cells[cell_no].insert_faces (faces);

				   // after all faces have been filled
				   // and the faces have indices of
				   // their neighbors, we may also
				   // insert the neighbor indices into
				   // the cells themselves
  for (unsigned int cell_no=0; cell_no<cells.size(); ++cell_no)
    cells[cell_no].fix_cell_neighbors ();


				   // do a preordering step in order
				   // to make further backtracking
				   // more local
  const vector<unsigned int>
    new_cell_numbers = presort_cells (cells, faces);

				   // finally do some preliminary work
				   // to make backtracking simpler
				   // later
  for (unsigned int cell_no=0; cell_no<cells.size(); ++cell_no)
    cells[cell_no].find_backtracking_point ();
  
				   // now do the main work
  find_reordering (cells, original_cells, new_cell_numbers);


  
				   // finally check the consistency of
				   // the program by ensuring that all
				   // faces have no use-marks any
				   // more. to this end, the
				   // find_reordering function has
				   // cleared all used marks it knows
				   // of
  for (typename map<Face,FaceData>::iterator i=faces.begin(); i!=faces.end(); ++i)
    Assert (i->second.use_count == 0, ExcInternalError());
};



#if deal_II_dimension == 1

template <>
void GridReordering<1>::reorder_cells (vector<CellData<1> > &)
{
				   // there should not be much to do
				   // in 1d...
};

#endif



// explicit instantiations. only require the main function, it should
// then claim whatever templates it needs. note that in 1d, the
// respective function is already specialized
#if deal_II_dimension >= 2
template
void
GridReordering<deal_II_dimension>::
reorder_cells (vector<CellData<deal_II_dimension> > &);
#endif
