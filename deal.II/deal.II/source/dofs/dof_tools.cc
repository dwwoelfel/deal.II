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


#include <base/multithread_info.h>
#include <base/thread_management.h>
#include <base/quadrature_lib.h>
#include <base/table.h>
#include <grid/tria.h>
#include <grid/tria_iterator.h>
#include <grid/intergrid_map.h>
#include <dofs/dof_handler.h>
#include <dofs/dof_accessor.h>
#include <dofs/dof_constraints.h>
#include <fe/fe.h>
#include <fe/fe_values.h>
#include <dofs/dof_tools.h>
#include <lac/sparsity_pattern.h>
#include <lac/compressed_sparsity_pattern.h>
#include <lac/block_sparsity_pattern.h>
#include <lac/vector.h>

#include <multigrid/mg_dof_handler.h>
#include <multigrid/mg_dof_accessor.h>

#include <algorithm>
#include <numeric>


namespace 
{
  // Functions for the DoFHandler
  template <int dim>
  unsigned int
  max_dofs_per_cell (const DoFHandler<dim> &dh)
  {
    return dh.get_fe().dofs_per_cell;
  }



  template <int dim>
  unsigned int
  max_dofs_per_face (const DoFHandler<dim> &dh) 
  {
    return dh.get_fe().dofs_per_face;
  }



  template <int dim>
  unsigned int
  max_dofs_per_vertex (const DoFHandler<dim> &dh) 
  {
    return dh.get_fe().dofs_per_vertex;
  }



  template <int dim>
  unsigned int
  n_components (const DoFHandler<dim> &dh) 
  {
    return dh.get_fe().n_components();
  }



  template <int dim>
  unsigned int
  fe_is_primitive (const DoFHandler<dim> &dh) 
  {
    return dh.get_fe().is_primitive();
  }

  // Functions for the hp::DoFHandler
  template <int dim>
  unsigned int
  max_dofs_per_cell (const hp::DoFHandler<dim> &dh) 
  {
    return dh.get_fe().max_dofs_per_cell ();
  }



  template <int dim>
  unsigned int
  max_dofs_per_face (const hp::DoFHandler<dim> &dh) 
  {
    return dh.get_fe().max_dofs_per_face ();
  }



  template <int dim>
  unsigned int
  max_dofs_per_vertex (const hp::DoFHandler<dim> &dh) 
  {
    return dh.get_fe().max_dofs_per_vertex ();
  }



  template <int dim>
  unsigned int
  n_components (const hp::DoFHandler<dim> &dh) 
  {
//TODO:[?] Verify that this is really correct
    return dh.get_fe()[0].n_components();
  }



  template <int dim>
  unsigned int
  fe_is_primitive (const hp::DoFHandler<dim> &dh) 
  {
//TODO:[?] Verify that this is really correct
    return dh.get_fe()[0].is_primitive();
  }
}



#if  deal_II_dimension == 1

// Specialization for 1D
template <class DH>
void
DoFTools::
compute_row_length_vector(
  const DH&                  dofs,
  std::vector<unsigned int>& row_lengths,
  const Coupling             flux_coupling)
{
  Assert (row_lengths.size() == dofs.n_dofs(),
	  ExcDimensionMismatch(row_lengths.size(), dofs.n_dofs()));

				   // Function starts here by
				   // resetting the counters.
  std::fill(row_lengths.begin(), row_lengths.end(), 0);
  
  const typename DH::cell_iterator end = dofs.end();
  typename DH::active_cell_iterator cell;
  std::vector<unsigned int> cell_indices;
  
  for (cell = dofs.begin_active(); cell != end; ++cell)
    {
      const FiniteElement<DH::dimension>& fe = cell->get_fe();
      cell_indices.resize(fe.dofs_per_cell);
      cell->get_dof_indices(cell_indices);

                                       // each dof can couple with each other
                                       // dof on this cell
      for (unsigned int i=0; i<fe.dofs_per_cell; ++i)
	row_lengths[cell_indices[i]] += fe.dofs_per_cell;
      
				       // If fluxes couple, add
				       // coupling to neighbor cells
      if (flux_coupling != none)
	for (unsigned int face=0;face<GeometryInfo<DH::dimension>::faces_per_cell;++face)
	  {
	    if (cell->at_boundary(face))
              continue;
            const FiniteElement<DH::dimension>& nfe = cell->get_fe();
	    for (unsigned int i=0; i<fe.dofs_per_cell; ++i)
	      row_lengths[cell_indices[i]] += nfe.dofs_per_cell;
	}
    }
}


// Specialization for 1D
template <class DH>
void
DoFTools::compute_row_length_vector(
  const DH& dofs,
  std::vector<unsigned int>& row_lengths,
  const Table<2,Coupling>& couplings,
  const Table<2,Coupling>& flux_couplings)
{
  Assert (row_lengths.size() == dofs.n_dofs(),
	  ExcDimensionMismatch(row_lengths.size(), dofs.n_dofs()));
  
  				   // Function starts here by
				   // resetting the counters.
  std::fill(row_lengths.begin(), row_lengths.end(), 0);  
  
  const typename DH::cell_iterator end = dofs.end();
  typename DH::active_cell_iterator cell;
  std::vector<unsigned int> cell_indices;
  
  for (cell = dofs.begin_active(); cell != end; ++cell)
    {
      const FiniteElement<DH::dimension>& fe = cell->get_fe();
      Assert (fe.is_primitive(),
	      typename FiniteElement<DH::dimension>::ExcFENotPrimitive());
      Assert (couplings.n_rows()==fe.n_components(),
	      ExcDimensionMismatch(couplings.n_rows(), fe.n_components()));
      Assert (couplings.n_cols()==fe.n_components(),
	      ExcDimensionMismatch(couplings.n_cols(), fe.n_components()));
      Assert (flux_couplings.n_rows()==fe.n_components(),
	      ExcDimensionMismatch(flux_couplings.n_rows(), fe.n_components()));
      Assert (flux_couplings.n_cols()==fe.n_components(),
	      ExcDimensionMismatch(flux_couplings.n_cols(), fe.n_components()));

      cell_indices.resize(fe.dofs_per_cell);
      cell->get_dof_indices(cell_indices);
      
                                       // each dof can couple with each other
                                       // dof on this cell
      for (unsigned int i=0; i<fe.dofs_per_cell; ++i)
	for (unsigned int comp=0;comp<fe.n_components();++comp)
	  if (couplings(fe.system_to_component_index(i).first,comp) != none)
	    row_lengths[cell_indices[i]]
	      += fe.base_element(fe.component_to_base_index(comp).first).dofs_per_cell;
      
				       // If fluxes couple, add
				       // coupling to neighbor cells
      for (unsigned int face=0;face<GeometryInfo<DH::dimension>::faces_per_cell;++face)
	{
	  if (cell->at_boundary(face)) continue;
	  const FiniteElement<DH::dimension>& nfe = cell->neighbor(face)->get_fe();
	  for (unsigned int i=0; i<fe.dofs_per_cell; ++i)
	    for (unsigned int comp=0;comp<fe.n_components();++comp)
	      if (flux_couplings(fe.system_to_component_index(i).first,comp) != none)
		row_lengths[cell_indices[i]]
		  += nfe.base_element(fe.component_to_base_index(comp).first).dofs_per_cell;
	}
    }
}

template <class DH>
void
DoFTools::compute_row_length_vector(
  const DH& dofs,
  std::vector<std::vector<unsigned int> >& row_lengths,
  const Table<2,Coupling>&,
  const Table<2,Coupling>&)
{
  Assert (false, ExcNotImplemented());
  Assert (row_lengths.size() == dofs.n_dofs(),
	  ExcDimensionMismatch(row_lengths.size(), dofs.n_dofs()));
}

#else

template <class DH, class CellIterator>
static
void
compute_cell_row_length_matrix(
  Table<2, unsigned int>& matrix,
  const CellIterator& cell,
  const FiniteElement<DH::dimension>& fe,
  const Table<2,DoFTools::Coupling>& couple_cell,
  const Table<2,DoFTools::Coupling>& couple_face)
{
				   // First, dofs on
				   // vertices. We assume that
				   // each vertex dof couples
				   // with all dofs on
				   // adjacent grid cells.

				   // Adding all dofs of the cells
				   // will add dofs of the faces
				   // of the cell adjacent to the
				   // vertex twice. Therefore, we
				   // subtract these here and add
				   // them in a loop over the
				   // faces below.

				   // in 1d, faces and vertices
				   // are identical. Nevertheless,
				   // this will only work if
				   // dofs_per_face is zero and
				   // dofs_per_vertex is
				   // arbitrary, not the other way
				   // round.
  unsigned int increment;
  unsigned int i=0;
  while (i < fe.first_line_index)
    {
      const unsigned int iblock = fe.system_to_block_index(i).first;
      for (unsigned int base=0;base<fe.n_base_elements();++base)
	{
	  increment = fe.base_element(base).dofs_per_cell
		      - DH::dimension * fe.base_element(base).dofs_per_face;
	      
	  for (unsigned int mult=0;mult<fe.element_multiplicity(base);++mult)
	    {
	      const unsigned int jblock = fe.first_block_of_base(base) + mult;
	      if (couple_cell(iblock, jblock) != DoFTools::none)
		{
		  matrix(i, jblock) += increment;
		}
	    }
	}
      ++i;
    }
				   // From now on, if an object is
				   // a cell, its dofs only couple
				   // inside the cell. Since the
				   // faces are handled below, we
				   // have to subtract ALL faces
				   // in this case.
      
				   // In all other cases we
				   // subtract adjacent faces to be
				   // added in the loop below.
  while (i < fe.first_quad_index)
    {
      const unsigned int iblock = fe.system_to_block_index(i).first;
      for (unsigned int base=0;base<fe.n_base_elements();++base)
	{
	  increment = fe.base_element(base).dofs_per_cell
		      - ((DH::dimension>1)
			 ? (DH::dimension-1)
			 : GeometryInfo<DH::dimension>::faces_per_cell)
		      * fe.base_element(base).dofs_per_face;
	      
	  for (unsigned int mult=0;mult<fe.element_multiplicity(base);++mult)
	    {
	      const unsigned int jblock = fe.first_block_of_base(base) + mult;
		  
	      if (couple_cell(iblock, jblock) != DoFTools::none)
		{
		  matrix(i, jblock) += increment;
		}
	    }
	}
      ++i;
    }
      
				   // Now quads in 2D and 3D
  while (i < fe.first_hex_index)
    {
      const unsigned int iblock = fe.system_to_block_index(i).first;
      for (unsigned int base=0;base<fe.n_base_elements();++base)
	{
	  increment = fe.base_element(base).dofs_per_cell
		      - ((DH::dimension>2)
			 ? (DH::dimension-2)
			 : GeometryInfo<DH::dimension>::faces_per_cell)
		      * fe.base_element(base).dofs_per_face;
	      
	  for (unsigned int mult=0;mult<fe.element_multiplicity(base);++mult)
	    {
	      const unsigned int jblock = fe.first_block_of_base(base) + mult;
	      if (couple_cell(iblock, jblock) != DoFTools::none)
		{
		  matrix(i, jblock) += increment;
		}
	    }
	}
      ++i;
    }
      
				   // Finally, cells in 3D
  while (i < fe.dofs_per_cell)
    {
      const unsigned int iblock = fe.system_to_block_index(i).first;
      for (unsigned int base=0;base<fe.n_base_elements();++base)
	{
	  increment = fe.base_element(base).dofs_per_cell
		      - GeometryInfo<DH::dimension>::faces_per_cell
		      * fe.base_element(base).dofs_per_face;
	      
	  for (unsigned int mult=0;mult<fe.element_multiplicity(base);++mult)
	    {
	      const unsigned int jblock = fe.first_block_of_base(base) + mult;
	      if (couple_cell(iblock, jblock) != DoFTools::none)
		{
		  matrix(i, jblock) += increment;
		}
	    }
	}
      ++i;
    }
      
				   // At this point, we have
				   // counted all dofs
				   // contributiong from cells
				   // coupled topologically to the
				   // adjacent cells, but we
				   // subtracted some faces.
  
				   // Now, let's go by the faces
				   // and add the missing
				   // contribution as well as the
				   // flux contributions.
  for (unsigned int iface=0;iface<GeometryInfo<DH::dimension>::faces_per_cell;++iface)
    {
      if (cell->at_boundary(iface))
	{
	  for (unsigned int i=0;i<fe.dofs_per_cell;++i)
	    {
	      const unsigned int iblock = fe.system_to_block_index(i).first;
	      for (unsigned int base=0;base<fe.n_base_elements();++base)
		{
		  increment = fe.base_element(base).dofs_per_face;
		      
		  for (unsigned int mult=0;mult<fe.element_multiplicity(base);++mult)
		    {
		      const unsigned int jblock = fe.first_block_of_base(base) + mult;
		      if (couple_cell(iblock, jblock) != DoFTools::none ||
			  (couple_face.size(0) != 0 &&
			   couple_face(iblock, jblock) != DoFTools::none))
			{
			  matrix(i, jblock) += increment;
			}
		    }
		}
	    }
	}
    }
}

// This will not work if the block structures of fe and nfe
// differ. Then, the coupling tables will have to be doubled.
template <class DH>
static
void
compute_face_row_length_matrix(
  Table<2, unsigned int>& matrix,
  Table<2, unsigned int>& nmatrix,
  const FiniteElement<DH::dimension>& fe,
  const FiniteElement<DH::dimension>& nfe,
  const Table<2,DoFTools::Coupling>& couple_cell,
  const Table<2,DoFTools::Coupling>& couple_face)
{
				   // This function will be called
				   // once per face, at the refinement
				   // edge from a refined cell.

				   // Compute contributions due to
				   // numerical fluxes.
  if (couple_face.size(0) != 0)
    for (unsigned int base=0;base<nfe.n_base_elements();++base)
      {
	const unsigned int increment = nfe.base_element(base).dofs_per_cell
				       - nfe.base_element(base).dofs_per_face;
	
	for (unsigned int mult=0;mult<nfe.element_multiplicity(base);++mult)
	  {
	    const unsigned int jblock = nfe.first_block_of_base(base) + mult;
	    for (unsigned int i=0;i<fe.dofs_per_cell;++i)
	      if (couple_face(fe.system_to_block_index(i).first, jblock)
		  != DoFTools::none)
		{
		  matrix(i,jblock) += increment;
		}
	  }
      }
  
				   // Compute the contributions due to
				   // numerical fluxes on the neighbor
				   // cell
  if (couple_face.size(0) != 0)
    for (unsigned int base=0;base<fe.n_base_elements();++base)
      {
	const unsigned int increment = fe.base_element(base).dofs_per_cell
				       - fe.base_element(base).dofs_per_face;
	for (unsigned int mult=0;mult<fe.element_multiplicity(base);++mult)
	  {
	    const unsigned int jblock = fe.first_block_of_base(base) + mult;
	    for (unsigned int i=0;i<nfe.dofs_per_cell;++i)
	      if (couple_face(nfe.system_to_block_index(i).first, jblock)
		  != DoFTools::none)
		{
		  nmatrix(i,jblock) += increment;
		}
	  }
      }
				   // At this point, we assume
				   // that each cell added its
				   // dofs minus the face to
				   // the couplings of the
				   // face dofs. Since we
				   // subtracted two faces, we
				   // have to re-add one.
  
				   // If one side of the face
				   // is refined, all the fine
				   // face dofs couple with
				   // the coarse one.
  
				   // Wolfgang, do they couple
				   // with each other by
				   // constraints?
  
				   // This will not work with
				   // different couplings on
				   // different cells.
  for (unsigned int base=0;base<nfe.n_base_elements();++base)
    for (unsigned int mult=0;mult<nfe.element_multiplicity(base);++mult)
      {
	const unsigned int jblock = nfe.first_block_of_base(base) + mult;
	for (unsigned int i=0;i<fe.dofs_per_cell;++i)
	  if (couple_cell(fe.system_to_block_index(i).first,
			  jblock) != DoFTools::none)
	    matrix(i, jblock)
	      += nfe.base_element(base).dofs_per_face;
      }
  
  for (unsigned int base=0;base<fe.n_base_elements();++base)
    for (unsigned int mult=0;mult<fe.element_multiplicity(base);++mult)
      {
	const unsigned int jblock = fe.first_block_of_base(base) + mult;
	for (unsigned int i=0;i<nfe.dofs_per_cell;++i)
	  if (couple_cell(nfe.system_to_block_index(i).first,
			  jblock) != DoFTools::none)
	    nmatrix(i, jblock)
	      += fe.base_element(base).dofs_per_face;
      }
}


// Template for 2D and 3D. For 1D see specialization above
template <class DH>
void
DoFTools::compute_row_length_vector(
  const DH&                  dofs,
  std::vector<unsigned int>& row_lengths,
  const Coupling             flux_coupling)
{
  Assert (row_lengths.size() == dofs.n_dofs(),
	  ExcDimensionMismatch(row_lengths.size(), dofs.n_dofs()));

				   // Function starts here by
				   // resetting the counters.
  std::fill(row_lengths.begin(), row_lengths.end(), 0);
				   // We need the user flags, so we
				   // save them for later restoration
  std::vector<bool> old_flags;
				   // We need a non-constant
				   // triangulation for the user
				   // flags. Since we restore them in
				   // the end, this cast is safe.
  Triangulation<DH::dimension>& user_flags_triangulation =
    const_cast<Triangulation<DH::dimension>&> (dofs.get_tria());
  user_flags_triangulation.save_user_flags(old_flags);
  user_flags_triangulation.clear_user_flags();
  
  const typename DH::cell_iterator end = dofs.end();
  typename DH::active_cell_iterator cell;
  std::vector<unsigned int> cell_indices;
  std::vector<unsigned int> neighbor_indices;

				   // We loop over cells and go from
				   // cells to lower dimensional
				   // objects. This is the only way to
				   // cope with the fact, that an
				   // unknown number of cells may
				   // share an object of dimension
				   // smaller than dim-1.
  for (cell = dofs.begin_active(); cell != end; ++cell)
    {
      const FiniteElement<DH::dimension>& fe = cell->get_fe();
      cell_indices.resize(fe.dofs_per_cell);
      cell->get_dof_indices(cell_indices);
      unsigned int i = 0;
				       // First, dofs on
				       // vertices. We assume that
				       // each vertex dof couples
				       // with all dofs on
				       // adjacent grid cells.

				       // Adding all dofs of the cells
				       // will add dofs of the faces
				       // of the cell adjacent to the
				       // vertex twice. Therefore, we
				       // subtract these here and add
				       // them in a loop over the
				       // faces below.

				       // in 1d, faces and vertices
				       // are identical. Nevertheless,
				       // this will only work if
				       // dofs_per_face is zero and
				       // dofs_per_vertex is
				       // arbitrary, not the other way
				       // round.
//TODO: This assumes that even in hp context, the dofs per face coincide!      
      unsigned int increment = fe.dofs_per_cell - DH::dimension * fe.dofs_per_face;
      while (i < fe.first_line_index)
	row_lengths[cell_indices[i++]] += increment;
				       // From now on, if an object is
				       // a cell, its dofs only couple
				       // inside the cell. Since the
				       // faces are handled below, we
				       // have to subtract ALL faces
				       // in this case.
      
				       // In all other cases we
				       // subtract adjacent faces to be
				       // added in the loop below.
      increment = (DH::dimension>1)
		  ? fe.dofs_per_cell - (DH::dimension-1) * fe.dofs_per_face
		  : fe.dofs_per_cell - GeometryInfo<DH::dimension>::faces_per_cell * fe.dofs_per_face;
      while (i < fe.first_quad_index)
	row_lengths[cell_indices[i++]] += increment;
      
				       // Now quads in 2D and 3D
      increment = (DH::dimension>2)
		  ? fe.dofs_per_cell - (DH::dimension-2) * fe.dofs_per_face
		  : fe.dofs_per_cell - GeometryInfo<DH::dimension>::faces_per_cell * fe.dofs_per_face;
      while (i < fe.first_hex_index)
	row_lengths[cell_indices[i++]] += increment;
				       // Finally, cells in 3D
      increment = fe.dofs_per_cell - GeometryInfo<DH::dimension>::faces_per_cell * fe.dofs_per_face;
      while (i < fe.dofs_per_cell)
	row_lengths[cell_indices[i++]] += increment;

				   // At this point, we have
				   // counted all dofs
				   // contributiong from cells
				   // coupled topologically to the
				   // adjacent cells, but we
				   // subtracted some faces.
  
				   // Now, let's go by the faces
				   // and add the missing
				   // contribution as well as the
				   // flux contributions.
      for (unsigned int iface=0;iface<GeometryInfo<DH::dimension>::faces_per_cell;++iface)
	{
	  if (cell->at_boundary(iface))
	    {
	      for (unsigned int i=0;i<fe.dofs_per_cell;++i)
		row_lengths[cell_indices[i]] += fe.dofs_per_face;
	      continue;
	    }
	  
	  const typename DH::cell_iterator neighbor = cell->neighbor(iface);
	  const FiniteElement<DH::dimension>& nfe = neighbor->get_fe();
	  typename DH::face_iterator face = cell->face(iface);
	  
					   // Flux couplings are
					   // computed from both sides
					   // for simplicity.
	  
					   // The dofs on the common face
					   // will be handled below,
					   // therefore, we subtract them
					   // here.
	  if (flux_coupling != none)
	    {
	      unsigned int increment = nfe.dofs_per_cell - nfe.dofs_per_face;
	      for (unsigned int i=0;i<fe.dofs_per_cell;++i)
		row_lengths[cell_indices[i]] += increment;
	    }
	  
					   // Do this only once per
					   // face and not on the
					   // hanging faces.
	  if (face->user_flag_set() || neighbor->has_children())
	    continue;
	  face->set_user_flag();
					   // At this point, we assume
					   // that each cell added its
					   // dofs minus the face to
					   // the couplings of the
					   // face dofs. Since we
					   // subtracted two faces, we
					   // have to re-add one.
	  
					   // If one side of the face
					   // is refined, all the fine
					   // face dofs couple with
					   // the coarse one.
	  neighbor_indices.resize(nfe.dofs_per_cell);
	  neighbor->get_dof_indices(neighbor_indices);
	  for (unsigned int i=0;i<fe.dofs_per_cell;++i)
	    row_lengths[cell_indices[i]] += nfe.dofs_per_face;
	  for (unsigned int i=0;i<nfe.dofs_per_cell;++i)
	    row_lengths[neighbor_indices[i]] += fe.dofs_per_face;
	}
    }
  user_flags_triangulation.load_user_flags(old_flags);
}


// This is the template for 2D and 3D. See version for 1D above
template <class DH>
void
DoFTools::compute_row_length_vector(
  const DH& dofs,
  std::vector<unsigned int>& row_lengths,
  const Table<2,Coupling>& couplings,
  const Table<2,Coupling>& flux_couplings)
{
  Assert (row_lengths.size() == dofs.n_dofs(),
	  ExcDimensionMismatch(row_lengths.size(), dofs.n_dofs()));
  
				   // Function starts here by
				   // resetting the counters.
  std::fill(row_lengths.begin(), row_lengths.end(), 0);
				   // We need the user flags, so we
				   // save them for later restoration
  std::vector<bool> old_flags;
				   // We need a non-constant
				   // triangulation for the user
				   // flags. Since we restore them in
				   // the end, this cast is safe.
  Triangulation<DH::dimension>& user_flags_triangulation =
    const_cast<Triangulation<DH::dimension>&> (dofs.get_tria());
  user_flags_triangulation.save_user_flags(old_flags);
  user_flags_triangulation.clear_user_flags();
  
  const typename DH::cell_iterator end = dofs.end();
  typename DH::active_cell_iterator cell;
  std::vector<unsigned int> cell_indices;
  std::vector<unsigned int> neighbor_indices;
  
				   // We have to translate the
				   // couplings from components to
				   // blocks, so this works for
				   // nonprimitive elements as well.
  std::vector<Table<2, Coupling> > couple_cell(1);
  std::vector<Table<2, Coupling> > couple_face(1);
  convert_couplings_to_blocks(dofs, couplings, couple_cell);
  if (flux_couplings.size(0) != 0)
    convert_couplings_to_blocks(dofs, flux_couplings, couple_face);

  Table<2, unsigned int> cell_couplings;
  Table<2, unsigned int> neighbor_couplings;
  
				   // We loop over cells and go from
				   // cells to lower dimensional
				   // objects. This is the only way to
				   // cope withthe fact, that an
				   // unknown number of cells may
				   // share an object of dimension
				   // smaller than dim-1.
  for (cell = dofs.begin_active(); cell != end; ++cell)
    {
      const FiniteElement<DH::dimension>& fe = cell->get_fe();
      const unsigned int fe_index = cell->active_fe_index();
      
      Assert (couplings.n_rows()==fe.n_components(),
	      ExcDimensionMismatch(couplings.n_rows(), fe.n_components()));
      Assert (couplings.n_cols()==fe.n_components(),
	      ExcDimensionMismatch(couplings.n_cols(), fe.n_components()));
      Assert (flux_couplings.n_rows()==fe.n_components(),
	      ExcDimensionMismatch(flux_couplings.n_rows(), fe.n_components()));
      Assert (flux_couplings.n_cols()==fe.n_components(),
	      ExcDimensionMismatch(flux_couplings.n_cols(), fe.n_components()));
      
      cell_couplings.reinit(fe.dofs_per_cell, fe.n_blocks());
      cell_indices.resize(fe.dofs_per_cell);
      cell->get_dof_indices(cell_indices);

      compute_cell_row_length_matrix<DH, typename DH::active_cell_iterator>(
	cell_couplings, cell, fe, couple_cell[fe_index], couple_face[fe_index]);
      
				   // At this point, we have
				   // counted all dofs
				   // contributiong from cells
				   // coupled topologically to the
				   // adjacent cells, but we
				   // subtracted some faces.
  
				   // Now, let's go by the faces
				   // and add the missing
				   // contribution as well as the
				   // flux contributions.
      for (unsigned int iface=0;iface<GeometryInfo<DH::dimension>::faces_per_cell;++iface)
	{
	  const typename DH::cell_iterator neighbor = cell->neighbor(iface);
	  const FiniteElement<DH::dimension>& nfe = neighbor->get_fe();
	  typename DH::face_iterator face = cell->face(iface);
					   // Do this only once per
					   // face and not on the
					   // hanging faces.
	  if (face->user_flag_set() || neighbor->has_children())
	    continue;
	  face->set_user_flag();	  

	  neighbor_couplings.reinit(nfe.dofs_per_cell, nfe.n_blocks());
	  neighbor_indices.resize(nfe.dofs_per_cell);
	  neighbor->get_dof_indices(neighbor_indices);

	  compute_face_row_length_matrix<DH>(
	    cell_couplings, neighbor_couplings,// face, cell, neighbor,
	    fe, nfe, couple_cell[fe_index], couple_face[fe_index]);
	  
	  for (unsigned int i=0;i<neighbor_indices.size();++i)
	    for (unsigned int j=0;j<neighbor_couplings.size(1);++j)
	      row_lengths[neighbor_indices[i]] += neighbor_couplings(i,j);

	}
      for (unsigned int i=0;i<cell_indices.size();++i)
	for (unsigned int j=0;j<cell_couplings.size(1);++j)
	  row_lengths[cell_indices[i]] += cell_couplings(i,j);
    }
  user_flags_triangulation.load_user_flags(old_flags);
}

// This is the template for 2D and 3D. See version for 1D above
template <class DH>
void
DoFTools::compute_row_length_vector(
  const DH& dofs,
  std::vector<std::vector<unsigned int> >& row_lengths,
  const Table<2,Coupling>& couplings,
  const Table<2,Coupling>& flux_couplings)
{
  Assert (row_lengths.size() == dofs.get_fe().n_blocks(),
	  ExcDimensionMismatch(row_lengths.size(), dofs.get_fe().n_blocks()));
  
				   // Function starts here by
				   // resetting the counters.
  for (unsigned int i=0;i<row_lengths.size();++i)
    std::fill(row_lengths[i].begin(), row_lengths[i].end(), 0);
				   // We need the user flags, so we
				   // save them for later restoration
  std::vector<bool> old_flags;
				   // We need a non-constant
				   // triangulation for the user
				   // flags. Since we restore them in
				   // the end, this cast is safe.
  Triangulation<DH::dimension>& user_flags_triangulation =
    const_cast<Triangulation<DH::dimension>&> (dofs.get_tria());
  user_flags_triangulation.save_user_flags(old_flags);
  user_flags_triangulation.clear_user_flags();
  
  const typename DH::cell_iterator end = dofs.end();
  typename DH::active_cell_iterator cell;
  std::vector<unsigned int> cell_indices;
  std::vector<unsigned int> neighbor_indices;
  
				   // We have to translate the
				   // couplings from components to
				   // blocks, so this works for
				   // nonprimitive elements as well.
  std::vector<Table<2, Coupling> > couple_cell(1);
  std::vector<Table<2, Coupling> > couple_face(1);
  convert_couplings_to_blocks(dofs, couplings, couple_cell);
  if (flux_couplings.size(0) != 0)
    convert_couplings_to_blocks(dofs, flux_couplings, couple_face);

  Table<2, unsigned int> cell_couplings;
  Table<2, unsigned int> neighbor_couplings;
  
				   // We loop over cells and go from
				   // cells to lower dimensional
				   // objects. This is the only way to
				   // cope withthe fact, that an
				   // unknown number of cells may
				   // share an object of dimension
				   // smaller than dim-1.
  for (cell = dofs.begin_active(); cell != end; ++cell)
    {
      const FiniteElement<DH::dimension>& fe = cell->get_fe();
      const unsigned int fe_index = cell->active_fe_index();
      cell_couplings.reinit(fe.dofs_per_cell, fe.n_blocks());
      cell_indices.resize(fe.dofs_per_cell);
      cell->get_dof_indices(cell_indices);

      compute_cell_row_length_matrix<DH, typename DH::active_cell_iterator>(
	cell_couplings, cell, fe, couple_cell[fe_index], couple_face[fe_index]);
      
				   // At this point, we have
				   // counted all dofs
				   // contributiong from cells
				   // coupled topologically to the
				   // adjacent cells, but we
				   // subtracted some faces.
  
				   // Now, let's go by the faces
				   // and add the missing
				   // contribution as well as the
				   // flux contributions.
      for (unsigned int iface=0;iface<GeometryInfo<DH::dimension>::faces_per_cell;++iface)
	{
	  if (cell->at_boundary(iface))
	    continue;
	  const typename DH::cell_iterator neighbor = cell->neighbor(iface);
	  const FiniteElement<DH::dimension>& nfe = neighbor->get_fe();
	  typename DH::face_iterator face = cell->face(iface);
					   // Do this only once per
					   // face and not on the
					   // hanging faces.
	  if (face->user_flag_set() || neighbor->has_children())
	    continue;
	  face->set_user_flag();	  

	  neighbor_couplings.reinit(nfe.dofs_per_cell, nfe.n_blocks());
	  neighbor_indices.resize(nfe.dofs_per_cell);
	  neighbor->get_dof_indices(neighbor_indices);

	  compute_face_row_length_matrix<DH>(
	    cell_couplings, neighbor_couplings,// face, cell, neighbor,
	    fe, nfe, couple_cell[fe_index], couple_face[fe_index]);
	  
	  for (unsigned int i=0;i<neighbor_indices.size();++i)
	    for (unsigned int j=0;j<neighbor_couplings.size(1);++j)
	      row_lengths[j][neighbor_indices[i]] += neighbor_couplings(i,j);

	}
      for (unsigned int i=0;i<cell_indices.size();++i)
	for (unsigned int j=0;j<cell_couplings.size(1);++j)
	  row_lengths[j][cell_indices[i]] += cell_couplings(i,j);
    }
  user_flags_triangulation.load_user_flags(old_flags);
}

#endif



template <class DH, class SparsityPattern>
void
DoFTools::make_sparsity_pattern (
  const DH        &dof,
  SparsityPattern &sparsity)
{
  const unsigned int n_dofs = dof.n_dofs();

  Assert (sparsity.n_rows() == n_dofs,
	  ExcDimensionMismatch (sparsity.n_rows(), n_dofs));
  Assert (sparsity.n_cols() == n_dofs,
	  ExcDimensionMismatch (sparsity.n_cols(), n_dofs));

  std::vector<unsigned int> dofs_on_this_cell;
  dofs_on_this_cell.reserve (max_dofs_per_cell(dof));
  typename DH::active_cell_iterator cell = dof.begin_active(),
				    endc = dof.end();
  for (; cell!=endc; ++cell) 
    {
      const unsigned int dofs_per_cell = cell->get_fe().dofs_per_cell;
      dofs_on_this_cell.resize (dofs_per_cell);
      cell->get_dof_indices (dofs_on_this_cell);
				       // make sparsity pattern for this cell
      for (unsigned int i=0; i<dofs_per_cell; ++i)
	for (unsigned int j=0; j<dofs_per_cell; ++j)
	  sparsity.add (dofs_on_this_cell[i],
			dofs_on_this_cell[j]);
    }
}



template <class DH, class SparsityPattern>
void
DoFTools::make_sparsity_pattern (
  const DH                &dof,
  const Table<2,Coupling> &couplings,
  SparsityPattern&         sparsity)
{
  const unsigned int n_dofs = dof.n_dofs();
  const unsigned int dofs_per_cell = dof.get_fe().dofs_per_cell;

  Assert (sparsity.n_rows() == n_dofs,
	  ExcDimensionMismatch (sparsity.n_rows(), n_dofs));
  Assert (sparsity.n_cols() == n_dofs,
	  ExcDimensionMismatch (sparsity.n_cols(), n_dofs));
  Assert (couplings.n_rows() == dof.get_fe().n_components(),
	  ExcDimensionMismatch(couplings.n_rows(), dof.get_fe().n_components()));
  Assert (couplings.n_cols() == dof.get_fe().n_components(),
	  ExcDimensionMismatch(couplings.n_cols(), dof.get_fe().n_components()));

				   // first build a mask for each dof,
				   // not like the one given which
				   // represents components. make sure
				   // we do the right thing also with
				   // respect to non-primitive shape
				   // functions, which takes some
				   // additional thought
  std::vector<std::vector<bool> > dof_mask(dofs_per_cell,
					   std::vector<bool>(dofs_per_cell, false));
  for (unsigned int i=0; i<dofs_per_cell; ++i)
    for (unsigned int j=0; j<dofs_per_cell; ++j)
      if (dof.get_fe().is_primitive(i) &&
          dof.get_fe().is_primitive(j))
        dof_mask[i][j] = (couplings(dof.get_fe().system_to_component_index(i).first,
				    dof.get_fe().system_to_component_index(j).first) != none);
      else
        {
          const unsigned int first_nonzero_comp_i
            = (std::find (dof.get_fe().get_nonzero_components(i).begin(),
                          dof.get_fe().get_nonzero_components(i).end(),
                          true)
               -
               dof.get_fe().get_nonzero_components(i).begin());
          const unsigned int first_nonzero_comp_j
            = (std::find (dof.get_fe().get_nonzero_components(j).begin(),
                          dof.get_fe().get_nonzero_components(j).end(),
                          true)
               -
               dof.get_fe().get_nonzero_components(j).begin());
          Assert (first_nonzero_comp_i < dof.get_fe().n_components(),
                  ExcInternalError());
          Assert (first_nonzero_comp_j < dof.get_fe().n_components(),
                  ExcInternalError());          
          
          dof_mask[i][j] = (couplings(first_nonzero_comp_i,first_nonzero_comp_j) != none);
        }
  


  std::vector<unsigned int> dofs_on_this_cell(dofs_per_cell);
  typename DH::active_cell_iterator cell = dof.begin_active(),
				    endc = dof.end();
  for (; cell!=endc; ++cell) 
    {
      cell->get_dof_indices (dofs_on_this_cell);
				       // make sparsity pattern for this cell
      for (unsigned int i=0; i<dofs_per_cell; ++i)
	for (unsigned int j=0; j<dofs_per_cell; ++j)
	  if (dof_mask[i][j] == true)
	    sparsity.add (dofs_on_this_cell[i],
			  dofs_on_this_cell[j]);
    }
}



#if deal_II_dimension == 1

template <class DH, class SparsityPattern>
void
DoFTools::make_boundary_sparsity_pattern (
  const DH                               &dof_handler,
  const typename FunctionMap<DH::dimension>::type &function_map,
  const std::vector<unsigned int>        &dof_to_boundary_mapping,
  SparsityPattern                        &sparsity)
{
				   // first check left, the right
				   // boundary point
  for (unsigned int direction=0; direction<2; ++direction)
    {
				       // if this boundary is not
				       // requested, then go on with next one
      if (function_map.find(direction) ==
	  function_map.end())
	continue;

				       // find active cell at that
				       // boundary: first go to
				       // left/right, then to children
      typename DH::cell_iterator cell = dof_handler.begin(0);
      while (!cell->at_boundary(direction))
	cell = cell->neighbor(direction);
      while (!cell->active())
	cell = cell->child(direction);

      const unsigned int dofs_per_vertex = cell->get_fe().dofs_per_vertex;
      std::vector<unsigned int> boundary_dof_boundary_indices (dofs_per_vertex);
  
				       // next get boundary mapped dof
				       // indices of boundary dofs
      for (unsigned int i=0; i<dofs_per_vertex; ++i)
	boundary_dof_boundary_indices[i]
	  = dof_to_boundary_mapping[cell->vertex_dof_index(direction,i)];

      for (unsigned int i=0; i<dofs_per_vertex; ++i)
	for (unsigned int j=0; j<dofs_per_vertex; ++j)
	  sparsity.add (boundary_dof_boundary_indices[i],
			boundary_dof_boundary_indices[j]);
    };
}



template <class DH, class SparsityPattern>
void DoFTools::make_boundary_sparsity_pattern (
  const DH                        &dof_handler,
  const std::vector<unsigned int> &dof_to_boundary_mapping,
  SparsityPattern                 &sparsity)
{
				   // there are only 2 boundary
				   // indicators in 1d, so it is no
				   // performance problem to call the
				   // other function
  typename DH::FunctionMap boundary_indicators;
  boundary_indicators[0] = 0;
  boundary_indicators[1] = 0;
  make_boundary_sparsity_pattern<DH, SparsityPattern> (dof_handler,
						       boundary_indicators,
						       dof_to_boundary_mapping,
						       sparsity);
}


#else


template <class DH, class SparsityPattern>
void
DoFTools::make_boundary_sparsity_pattern (
  const DH                        &dof,
  const std::vector<unsigned int> &dof_to_boundary_mapping,
  SparsityPattern                 &sparsity)
{
  const unsigned int n_dofs = dof.n_dofs();

  Assert (dof_to_boundary_mapping.size() == n_dofs, ExcInternalError());
  Assert (sparsity.n_rows() == dof.n_boundary_dofs(),
	  ExcDimensionMismatch (sparsity.n_rows(), dof.n_boundary_dofs()));
  Assert (sparsity.n_cols() == dof.n_boundary_dofs(),
	  ExcDimensionMismatch (sparsity.n_cols(), dof.n_boundary_dofs()));
#ifdef DEBUG
  if (true)
    {
      unsigned int max_element = 0;
      for (std::vector<unsigned int>::const_iterator i=dof_to_boundary_mapping.begin();
	   i!=dof_to_boundary_mapping.end(); ++i)
	if ((*i != DH::invalid_dof_index) &&
	    (*i > max_element))
	  max_element = *i;
      Assert (max_element  == sparsity.n_rows()-1,
	      ExcInternalError());
    };
#endif

  std::vector<unsigned int> dofs_on_this_face;
  dofs_on_this_face.reserve (max_dofs_per_face(dof));

				   // loop over all faces to check
				   // whether they are at a
				   // boundary. note that we need not
				   // take special care of single
				   // lines (using
				   // @p{cell->has_boundary_lines}),
				   // since we do not support
				   // boundaries of dimension dim-2,
				   // and so every boundary line is
				   // also part of a boundary face.
  typename DH::active_face_iterator face = dof.begin_active_face(),
				    endf = dof.end_face();
  for (; face!=endf; ++face)
    if (face->at_boundary())
      {
        const unsigned int dofs_per_face = face->get_fe().dofs_per_face;
        dofs_on_this_face.resize (dofs_per_face);
	face->get_dof_indices (dofs_on_this_face);

					 // make sparsity pattern for this cell
	for (unsigned int i=0; i<dofs_per_face; ++i)
	  for (unsigned int j=0; j<dofs_per_face; ++j) 
	    sparsity.add (dof_to_boundary_mapping[dofs_on_this_face[i]],
			  dof_to_boundary_mapping[dofs_on_this_face[j]]);
      };
}



template <class DH, class SparsityPattern>
void DoFTools::make_boundary_sparsity_pattern (
  const DH                                        &dof,
  const typename FunctionMap<DH::dimension>::type &boundary_indicators,
  const std::vector<unsigned int>                 &dof_to_boundary_mapping,
  SparsityPattern                                 &sparsity)
{
  const unsigned int n_dofs = dof.n_dofs();

  Assert (dof_to_boundary_mapping.size() == n_dofs, ExcInternalError());
  Assert (boundary_indicators.find(255) == boundary_indicators.end(),
	  typename DH::ExcInvalidBoundaryIndicator());
  Assert (sparsity.n_rows() == dof.n_boundary_dofs (boundary_indicators),
	  ExcDimensionMismatch (sparsity.n_rows(), dof.n_boundary_dofs (boundary_indicators)));
  Assert (sparsity.n_cols() == dof.n_boundary_dofs (boundary_indicators),
	  ExcDimensionMismatch (sparsity.n_cols(), dof.n_boundary_dofs (boundary_indicators)));
#ifdef DEBUG
  if (true)
    {
      unsigned int max_element = 0;
      for (std::vector<unsigned int>::const_iterator i=dof_to_boundary_mapping.begin();
	   i!=dof_to_boundary_mapping.end(); ++i)
	if ((*i != DH::invalid_dof_index) &&
	    (*i > max_element))
	  max_element = *i;
      Assert (max_element  == sparsity.n_rows()-1,
	      ExcInternalError());
    };
#endif

  std::vector<unsigned int> dofs_on_this_face;
  dofs_on_this_face.reserve (max_dofs_per_face(dof));
  typename DH::active_face_iterator face = dof.begin_active_face(),
				    endf = dof.end_face();
  for (; face!=endf; ++face)
    if (boundary_indicators.find(face->boundary_indicator()) !=
	boundary_indicators.end())
      {
        const unsigned int dofs_per_face = face->get_fe().dofs_per_face;
        dofs_on_this_face.resize (dofs_per_face);
	face->get_dof_indices (dofs_on_this_face);

					 // make sparsity pattern for this cell
	for (unsigned int i=0; i<dofs_per_face; ++i)
	  for (unsigned int j=0; j<dofs_per_face; ++j)
	    sparsity.add (dof_to_boundary_mapping[dofs_on_this_face[i]],
			  dof_to_boundary_mapping[dofs_on_this_face[j]]);
      };
}

#endif


#if deal_II_dimension != 1

template <class DH, class SparsityPattern>
void
DoFTools::make_flux_sparsity_pattern (
  const DH        &dof,
  SparsityPattern &sparsity)
{
  const unsigned int n_dofs = dof.n_dofs();
  
  Assert (sparsity.n_rows() == n_dofs,
	  ExcDimensionMismatch (sparsity.n_rows(), n_dofs));
  Assert (sparsity.n_cols() == n_dofs,
	  ExcDimensionMismatch (sparsity.n_cols(), n_dofs));

  std::vector<unsigned int> dofs_on_this_cell;
  std::vector<unsigned int> dofs_on_other_cell;
  dofs_on_this_cell.reserve (max_dofs_per_cell(dof));
  dofs_on_other_cell.reserve (max_dofs_per_cell(dof));
  typename DH::active_cell_iterator cell = dof.begin_active(),
				    endc = dof.end();

				   // Clear user flags because we will
				   // need them. But first we save
				   // them and make sure that we
				   // restore them later such that at
				   // the end of this function the
				   // Triangulation will be in the
				   // same state as it was at the
				   // beginning of this function.
  std::vector<bool> user_flags;
  dof.get_tria().save_user_flags(user_flags);
  const_cast<Triangulation<DH::dimension> &>(dof.get_tria()).clear_user_flags ();
  
  for (; cell!=endc; ++cell)
    {
      const unsigned int n_dofs_on_this_cell = cell->get_fe().dofs_per_cell;
      dofs_on_this_cell.resize (n_dofs_on_this_cell);
      cell->get_dof_indices (dofs_on_this_cell);
				       // make sparsity pattern for this cell
      for (unsigned int i=0; i<n_dofs_on_this_cell; ++i)
	for (unsigned int j=0; j<n_dofs_on_this_cell; ++j)
	  sparsity.add (dofs_on_this_cell[i],
			dofs_on_this_cell[j]);

				       // Loop over all interior neighbors
      for (unsigned int face = 0;
	   face < GeometryInfo<DH::dimension>::faces_per_cell;
	   ++face)
	{
	  typename DH::face_iterator cell_face = cell->face(face);
	  if (cell_face->user_flag_set ())
	    continue;

	  if (! cell_face->at_boundary() )
	    {
	      typename DH::cell_iterator neighbor = cell->neighbor(face);
					       // Refinement edges are
					       // taken care of by
					       // coarser cells
	      if (neighbor->level() < cell->level())
		continue;

	      const unsigned int neighbor_face
                = cell->neighbor_of_neighbor(face);

	      if (cell_face->has_children())
		{
		  for (unsigned int sub_nr = 0;
		       sub_nr != cell_face->n_children();
		       ++sub_nr)
		    {
		      const typename DH::cell_iterator
                        sub_neighbor
			= cell->neighbor_child_on_subface (face, sub_nr);

                      const unsigned int n_dofs_on_neighbor
                        = sub_neighbor->get_fe().dofs_per_cell;
                      dofs_on_other_cell.resize (n_dofs_on_neighbor);
		      sub_neighbor->get_dof_indices (dofs_on_other_cell);

                      for (unsigned int i=0; i<n_dofs_on_this_cell; ++i)
                        for (unsigned int j=0; j<n_dofs_on_neighbor; ++j)
                          {
                            sparsity.add (dofs_on_this_cell[i],
                                          dofs_on_other_cell[j]);
                            sparsity.add (dofs_on_other_cell[j],
                                          dofs_on_this_cell[i]);
                          }
		      sub_neighbor->face(neighbor_face)->set_user_flag ();
		    }
		}
              else
                {
                  const unsigned int n_dofs_on_neighbor
                    = neighbor->get_fe().dofs_per_cell;
                  dofs_on_other_cell.resize (n_dofs_on_neighbor);

                  neighbor->get_dof_indices (dofs_on_other_cell);
		  for (unsigned int i=0; i<n_dofs_on_this_cell; ++i)
                    for (unsigned int j=0; j<n_dofs_on_neighbor; ++j)
                      {
                        sparsity.add (dofs_on_this_cell[i],
                                      dofs_on_other_cell[j]);
                        sparsity.add (dofs_on_other_cell[j],
                                      dofs_on_this_cell[i]);
                      }
		  neighbor->face(neighbor_face)->set_user_flag (); 
		}
	    } 
	}
    }

				   // finally restore the user flags
  const_cast<Triangulation<DH::dimension> &>(dof.get_tria()).load_user_flags(user_flags);
}

#else // deal_II_dimension == 1


template <class DH, class SparsityPattern>
void
DoFTools::make_flux_sparsity_pattern (
  const DH        &dof,
  SparsityPattern &sparsity)
{
  typedef typename DH::cell_iterator        cell_iterator;
  typedef typename DH::active_cell_iterator active_cell_iterator;

  std::vector<unsigned int> local_dof_indices;
  std::vector<unsigned int> neighbor_dof_indices;
  local_dof_indices.reserve (max_dofs_per_cell(dof));
  neighbor_dof_indices.reserve (max_dofs_per_cell(dof));
  
  active_cell_iterator cell = dof.begin_active(),
		       endc = dof.end();
  for (; cell!=endc; ++cell)
    {
				       // first do couplings of dofs
				       // locally on this cell
      const unsigned int n_dofs_on_this_cell = cell->get_fe().dofs_per_cell;
      local_dof_indices.resize (n_dofs_on_this_cell);
      cell->get_dof_indices (local_dof_indices);
      for (unsigned int i=0; i<n_dofs_on_this_cell; ++i)
	for (unsigned int j=0; j<n_dofs_on_this_cell; ++j)
	  sparsity.add (local_dof_indices[i], local_dof_indices[j]);

				       // then do the same for the up
				       // to 2 neighbors
      for (unsigned int nb=0; nb<2; ++nb)
	if (! cell->at_boundary(nb))
	  {
					     // find active neighbor
	    cell_iterator neighbor = cell->neighbor(nb);
	    while (neighbor->has_children())
	      neighbor = neighbor->child(nb==0 ? 1 : 0);

					     // get dofs on it
            const unsigned int n_dofs_on_neighbor
              = neighbor->get_fe().dofs_per_cell;
            neighbor_dof_indices.resize (n_dofs_on_neighbor);
	    neighbor->get_dof_indices (neighbor_dof_indices);

					     // compute couplings
	    for (unsigned int i=0; i<n_dofs_on_this_cell; ++i)
	      for (unsigned int j=0; j<n_dofs_on_neighbor; ++j)
		sparsity.add (local_dof_indices[i], neighbor_dof_indices[j]);
	  };
    };
}

#endif


template <class DH, class SparsityPattern>
void
DoFTools::make_flux_sparsity_pattern (
  const DH                &dof,
  SparsityPattern         &sparsity,
  const Table<2,Coupling> &int_mask,
  const Table<2,Coupling> &flux_mask)
{
  const unsigned int n_dofs = dof.n_dofs();
  const FiniteElement<DH::dimension> &fe = dof.get_fe();
  const unsigned int n_comp = fe.n_components();
  
  Assert (sparsity.n_rows() == n_dofs,
	  ExcDimensionMismatch (sparsity.n_rows(), n_dofs));
  Assert (sparsity.n_cols() == n_dofs,
	  ExcDimensionMismatch (sparsity.n_cols(), n_dofs));
  Assert (int_mask.n_rows() == n_comp,
	  ExcDimensionMismatch (int_mask.n_rows(), n_comp));
  Assert (int_mask.n_cols() == n_comp,
	  ExcDimensionMismatch (int_mask.n_cols(), n_comp));
  Assert (flux_mask.n_rows() == n_comp,
	  ExcDimensionMismatch (flux_mask.n_rows(), n_comp));
  Assert (flux_mask.n_cols() == n_comp,
	  ExcDimensionMismatch (flux_mask.n_cols(), n_comp));
  
  const unsigned int total_dofs = fe.dofs_per_cell;
  std::vector<unsigned int> dofs_on_this_cell(total_dofs);
  std::vector<unsigned int> dofs_on_other_cell(total_dofs);
  Table<2,bool> support_on_face(
    total_dofs, GeometryInfo<DH::dimension>::faces_per_cell);
  
  typename DH::active_cell_iterator cell = dof.begin_active(),
				    endc = dof.end();
  
  Table<2,Coupling> int_dof_mask(total_dofs, total_dofs);
  Table<2,Coupling> flux_dof_mask(total_dofs, total_dofs);
  
  compute_dof_couplings(int_dof_mask, int_mask, fe);
  compute_dof_couplings(flux_dof_mask, flux_mask, fe);
  
  for (unsigned int i=0; i<total_dofs; ++i)
    for (unsigned int f=0; f<GeometryInfo<DH::dimension>::faces_per_cell;++f)
      support_on_face(i,f) = fe.has_support_on_face(i,f);
  
				   // Clear user flags because we will
				   // need them. But first we save
				   // them and make sure that we
				   // restore them later such that at
				   // the end of this function the
				   // Triangulation will be in the
				   // same state as it was at the
				   // beginning of this function.
  std::vector<bool> user_flags;
  dof.get_tria().save_user_flags(user_flags);
  const_cast<Triangulation<DH::dimension> &>(dof.get_tria()).clear_user_flags ();
  
  for (; cell!=endc; ++cell)
    {
      cell->get_dof_indices (dofs_on_this_cell);
				       // make sparsity pattern for this cell
      for (unsigned int i=0; i<total_dofs; ++i)
	for (unsigned int j=0; j<total_dofs; ++j)
	  if (int_dof_mask(i,j) != none)
	    sparsity.add (dofs_on_this_cell[i],
			  dofs_on_this_cell[j]);

				       // Loop over all interior neighbors
      for (unsigned int face = 0;
	   face < GeometryInfo<DH::dimension>::faces_per_cell;
	   ++face)
	{
	  const typename DH::face_iterator
            cell_face = cell->face(face);
	  if (cell_face->user_flag_set ())
	    continue;

	  if (cell->at_boundary (face) )
	    {
	      for (unsigned int i=0; i<total_dofs; ++i)
		{
		  const bool i_non_zero_i = support_on_face (i, face);
		  for (unsigned int j=0; j<total_dofs; ++j)
		    {
		      const bool j_non_zero_i = support_on_face (j, face);
		      
		      if (flux_dof_mask(i,j) == always)
                        sparsity.add (dofs_on_this_cell[i],
                                      dofs_on_this_cell[j]);
		      if (flux_dof_mask(i,j) == nonzero
			  && i_non_zero_i && j_non_zero_i)
			sparsity.add (dofs_on_this_cell[i],
				      dofs_on_this_cell[j]);
		    }
		}
	    }
	  else
	    {
	      typename DH::cell_iterator
		neighbor = cell->neighbor(face);
					       // Refinement edges are taken care of
					       // by coarser cells
	      if (neighbor->level() < cell->level())
		continue;
	      
	      typename DH::face_iterator cell_face = cell->face(face);
	      const unsigned int
                neighbor_face = cell->neighbor_of_neighbor(face);
	      
	      if (cell_face->has_children())
		{
		  for (unsigned int sub_nr = 0;
		       sub_nr != cell_face->n_children();
		       ++sub_nr)
		    {
		      const typename DH::cell_iterator
                        sub_neighbor
			= cell->neighbor_child_on_subface (face, sub_nr);

		      sub_neighbor->get_dof_indices (dofs_on_other_cell);
		      for (unsigned int i=0; i<total_dofs; ++i)
			{
			  const bool i_non_zero_i = support_on_face (i, face);
			  const bool i_non_zero_e = support_on_face (i, neighbor_face);
			  for (unsigned int j=0; j<total_dofs; ++j)
			    {
			      const bool j_non_zero_i = support_on_face (j, face);
			      const bool j_non_zero_e  =support_on_face (j, neighbor_face);
			      if (flux_dof_mask(i,j) == always)
				{
				  sparsity.add (dofs_on_this_cell[i],
						dofs_on_other_cell[j]);
				  sparsity.add (dofs_on_other_cell[i],
						dofs_on_this_cell[j]);
				  sparsity.add (dofs_on_this_cell[i],
						dofs_on_this_cell[j]);
				  sparsity.add (dofs_on_other_cell[i],
						dofs_on_other_cell[j]);
				}
			      if (flux_dof_mask(i,j) == nonzero)
				{
				  if (i_non_zero_i && j_non_zero_e)
				    sparsity.add (dofs_on_this_cell[i],
						  dofs_on_other_cell[j]);
				  if (i_non_zero_e && j_non_zero_i)
				    sparsity.add (dofs_on_other_cell[i],
						  dofs_on_this_cell[j]);
				  if (i_non_zero_i && j_non_zero_i)
				    sparsity.add (dofs_on_this_cell[i],
						  dofs_on_this_cell[j]);
				  if (i_non_zero_e && j_non_zero_e)
				    sparsity.add (dofs_on_other_cell[i],
						  dofs_on_other_cell[j]);
				}
			      
			      if (flux_dof_mask(j,i) == always)
				{
				  sparsity.add (dofs_on_this_cell[j],
						dofs_on_other_cell[i]);
				  sparsity.add (dofs_on_other_cell[j],
						dofs_on_this_cell[i]);
				  sparsity.add (dofs_on_this_cell[j],
						dofs_on_this_cell[i]);
				  sparsity.add (dofs_on_other_cell[j],
						dofs_on_other_cell[i]);
				}
			      if (flux_dof_mask(j,i) == nonzero)
				{
				  if (j_non_zero_i && i_non_zero_e)
				    sparsity.add (dofs_on_this_cell[j],
						  dofs_on_other_cell[i]);
				  if (j_non_zero_e && i_non_zero_i)
				    sparsity.add (dofs_on_other_cell[j],
						  dofs_on_this_cell[i]);
				  if (j_non_zero_i && i_non_zero_i)
				    sparsity.add (dofs_on_this_cell[j],
						  dofs_on_this_cell[i]);
				  if (j_non_zero_e && i_non_zero_e)
				    sparsity.add (dofs_on_other_cell[j],
						  dofs_on_other_cell[i]); 
				}
			    }
			}
		      sub_neighbor->face(neighbor_face)->set_user_flag ();
		    }
		}
              else
                {
		  neighbor->get_dof_indices (dofs_on_other_cell);
		  for (unsigned int i=0; i<total_dofs; ++i)
		    {
		      const bool i_non_zero_i = support_on_face (i, face);
		      const bool i_non_zero_e = support_on_face (i, neighbor_face);
		      for (unsigned int j=0; j<total_dofs; ++j)
			{
			  const bool j_non_zero_i = support_on_face (j, face);
			  const bool j_non_zero_e = support_on_face (j, neighbor_face);
			  if (flux_dof_mask(i,j) == always)
			    {
			      sparsity.add (dofs_on_this_cell[i],
					    dofs_on_other_cell[j]);
			      sparsity.add (dofs_on_other_cell[i],
					    dofs_on_this_cell[j]);
			      sparsity.add (dofs_on_this_cell[i],
					    dofs_on_this_cell[j]);
			      sparsity.add (dofs_on_other_cell[i],
					    dofs_on_other_cell[j]);
			    }
			  if (flux_dof_mask(i,j) == nonzero)
			    {
			      if (i_non_zero_i && j_non_zero_e)
				sparsity.add (dofs_on_this_cell[i],
					      dofs_on_other_cell[j]);
			      if (i_non_zero_e && j_non_zero_i)
				sparsity.add (dofs_on_other_cell[i],
					      dofs_on_this_cell[j]);
			      if (i_non_zero_i && j_non_zero_i)
				sparsity.add (dofs_on_this_cell[i],
					      dofs_on_this_cell[j]);
			      if (i_non_zero_e && j_non_zero_e)
				sparsity.add (dofs_on_other_cell[i],
					      dofs_on_other_cell[j]); 
			    }

			  if (flux_dof_mask(j,i) == always)
			    {
			      sparsity.add (dofs_on_this_cell[j],
					    dofs_on_other_cell[i]);
			      sparsity.add (dofs_on_other_cell[j],
					    dofs_on_this_cell[i]);
			      sparsity.add (dofs_on_this_cell[j],
					    dofs_on_this_cell[i]);
			      sparsity.add (dofs_on_other_cell[j],
					    dofs_on_other_cell[i]);
			    }
			  if (flux_dof_mask(j,i) == nonzero)
			    {
			      if (j_non_zero_i && i_non_zero_e)
				sparsity.add (dofs_on_this_cell[j],
					      dofs_on_other_cell[i]);
			      if (j_non_zero_e && i_non_zero_i)
				sparsity.add (dofs_on_other_cell[j],
					      dofs_on_this_cell[i]);
			      if (j_non_zero_i && i_non_zero_i)
				sparsity.add (dofs_on_this_cell[j],
					      dofs_on_this_cell[i]);
			      if (j_non_zero_e && i_non_zero_e)
				sparsity.add (dofs_on_other_cell[j],
					      dofs_on_other_cell[i]); 
			    }
			}
		    }
		  neighbor->face(neighbor_face)->set_user_flag (); 
		}
	    } 
	}
    }

  				   // finally restore the user flags
  const_cast<Triangulation<DH::dimension> &>(dof.get_tria()).load_user_flags(user_flags);
}



#if deal_II_dimension == 1

void DoFTools::make_hanging_node_constraints (
  const DoFHandler<1> &,
  ConstraintMatrix &)
{
				   // nothing to be done here
}


void DoFTools::make_hanging_node_constraints (
  const hp::DoFHandler<1> &,
  ConstraintMatrix &)
{
				   // nothing to be done here
}

#endif



#if deal_II_dimension == 2

void DoFTools::make_hanging_node_constraints (
  const DoFHandler<2> &dof_handler,
  ConstraintMatrix    &constraints)
{
  const unsigned int dim = 2;
  
  const FiniteElement<dim> &fe   = dof_handler.get_fe();
  
				   // have space for the degrees of
				   // freedom on mother and child
				   // lines
  const unsigned int n_dofs_on_mother   = 2*fe.dofs_per_vertex + fe.dofs_per_line,
		     n_dofs_on_children = fe.dofs_per_vertex + 2*fe.dofs_per_line;

  std::vector<unsigned int> dofs_on_mother(n_dofs_on_mother);
  std::vector<unsigned int> dofs_on_children(n_dofs_on_children);

  Assert(n_dofs_on_mother == fe.constraints().n(),
	 ExcDimensionMismatch(n_dofs_on_mother,
			      fe.constraints().n()));
  Assert(n_dofs_on_children == fe.constraints().m(),
	 ExcDimensionMismatch(n_dofs_on_children,
			      fe.constraints().m()));

				   // loop over all lines; only on
				   // lines there can be constraints.
				   // We do so by looping over all
				   // active cells and checking
				   // whether any of the faces are
				   // refined which can only be from
				   // the neighboring cell because
				   // this one is active. In that
				   // case, the face is subject to
				   // constraints
				   //
				   // note that even though we may
				   // visit a face twice if the
				   // neighboring cells are equally
				   // refined, we can only visit each
				   // face with hanging nodes once
  DoFHandler<dim>::active_cell_iterator cell = dof_handler.begin_active(),
					endc = dof_handler.end();
  for (; cell!=endc; ++cell)
    for (unsigned int face=0; face<GeometryInfo<dim>::faces_per_cell; ++face)
      if (cell->face(face)->has_children()) 
	{
	  const DoFHandler<dim>::line_iterator line = cell->face(face);
	  
					   // fill the dofs indices. Use same
					   // enumeration scheme as in
					   // @p{FiniteElement::constraints()}
	  unsigned int next_index = 0;
	  for (unsigned int vertex=0; vertex<2; ++vertex)
	    for (unsigned int dof=0; dof!=fe.dofs_per_vertex; ++dof)
	      dofs_on_mother[next_index++] = line->vertex_dof_index(vertex,dof);
	  for (unsigned int dof=0; dof!=fe.dofs_per_line; ++dof)
	    dofs_on_mother[next_index++] = line->dof_index(dof);
	  Assert (next_index == dofs_on_mother.size(),
		  ExcInternalError());
	  
	  next_index = 0;
	  for (unsigned int dof=0; dof!=fe.dofs_per_vertex; ++dof)
	    dofs_on_children[next_index++] = line->child(0)->vertex_dof_index(1,dof);
	  for (unsigned int child=0; child<2; ++child)
	    for (unsigned int dof=0; dof!=fe.dofs_per_line; ++dof)
	      dofs_on_children[next_index++] = line->child(child)->dof_index(dof);
	  Assert (next_index == dofs_on_children.size(),
		  ExcInternalError());
	  
					   // for each row in the constraint
					   // matrix for this line:
	  for (unsigned int row=0; row!=dofs_on_children.size(); ++row) 
	    {
	      constraints.add_line (dofs_on_children[row]);
	      for (unsigned int i=0; i!=dofs_on_mother.size(); ++i)
		constraints.add_entry (dofs_on_children[row],
				       dofs_on_mother[i],
				       fe.constraints()(row,i));
	    };
	};
}


void DoFTools::make_hanging_node_constraints (
    const hp::DoFHandler<2> &/*dof_handler*/,
    ConstraintMatrix    &/*constraints*/)
{
//TODO[?]: Implement (required for continuous elements)
    Assert (false, ExcNotImplemented());
}

#endif



#if deal_II_dimension == 3

void DoFTools::make_hanging_node_constraints (
  const DoFHandler<3> &dof_handler,
  ConstraintMatrix    &constraints)
{
  const unsigned int dim = 3;
  
  const FiniteElement<dim> &fe   = dof_handler.get_fe();
  
				   // have space for the degrees of
				   // freedom on mother and child
				   // lines
  const unsigned int
    n_dofs_on_mother   = (4*fe.dofs_per_vertex+
			  4*fe.dofs_per_line+
			  fe.dofs_per_quad),
    n_dofs_on_children = (5*fe.dofs_per_vertex+
			  12*fe.dofs_per_line+
			  4*fe.dofs_per_quad);

  std::vector<unsigned int> dofs_on_mother(n_dofs_on_mother);
  std::vector<unsigned int> dofs_on_children(n_dofs_on_children);

  Assert(n_dofs_on_mother == fe.constraints().n(),
	 ExcDimensionMismatch(n_dofs_on_mother,
			      fe.constraints().n()));
  Assert(n_dofs_on_children == fe.constraints().m(),
	 ExcDimensionMismatch(n_dofs_on_children,
			      fe.constraints().m()));

				   // loop over all lines; only on
				   // lines there can be constraints.
				   // We do so by looping over all
				   // active cells and checking
				   // whether any of the faces are
				   // refined which can only be from
				   // the neighboring cell because
				   // this one is active. In that
				   // case, the face is subject to
				   // constraints
				   //
				   // note that even though we may
				   // visit a face twice if the
				   // neighboring cells are equally
				   // refined, we can only visit each
				   // face with hanging nodes once
  DoFHandler<dim>::active_cell_iterator cell = dof_handler.begin_active(),
					endc = dof_handler.end();
  for (; cell!=endc; ++cell)
    for (unsigned int f=0; f<GeometryInfo<dim>::faces_per_cell; ++f)
      if (cell->face(f)->has_children()) 
	{
	  const DoFHandler<dim>::face_iterator face = cell->face(f);
	  
					   // fill the dofs indices. Use same
					   // enumeration scheme as in
					   // @p{FiniteElement::constraints()}
	  unsigned int next_index = 0;
	  for (unsigned int vertex=0; vertex<4; ++vertex)
	    for (unsigned int dof=0; dof!=fe.dofs_per_vertex; ++dof)
	      dofs_on_mother[next_index++] = face->vertex_dof_index(vertex,dof);
	  for (unsigned int line=0; line<4; ++line)
	    for (unsigned int dof=0; dof!=fe.dofs_per_line; ++dof)
	      dofs_on_mother[next_index++] = face->line(line)->dof_index(dof);
	  for (unsigned int dof=0; dof!=fe.dofs_per_quad; ++dof)
	    dofs_on_mother[next_index++] = face->dof_index(dof);
	  Assert (next_index == dofs_on_mother.size(),
		  ExcInternalError());
	  
	  next_index = 0;

					   // assert some consistency
					   // assumptions
	  Assert ((face->child(0)->vertex_index(3) ==
		   face->child(1)->vertex_index(2)) &&
		  (face->child(0)->vertex_index(3) ==
		   face->child(2)->vertex_index(1)) &&
		  (face->child(0)->vertex_index(3) ==
		   face->child(3)->vertex_index(0)),
		  ExcInternalError());
	  for (unsigned int dof=0; dof!=fe.dofs_per_vertex; ++dof)
	    dofs_on_children[next_index++]
	      = face->child(0)->vertex_dof_index(3,dof);
	  
					   // dof numbers on the centers of
					   // the lines bounding this face
	  for (unsigned int line=0; line<4; ++line)
	    for (unsigned int dof=0; dof!=fe.dofs_per_vertex; ++dof)
	      dofs_on_children[next_index++]
		= face->line(line)->child(0)->vertex_dof_index(1,dof);
	  
					   // next the dofs on the lines interior
					   // to the face; the order of these
					   // lines is laid down in the
					   // FiniteElement class documentation
	  for (unsigned int dof=0; dof<fe.dofs_per_line; ++dof)
	    dofs_on_children[next_index++]
	      = face->child(0)->line(1)->dof_index(dof);
	  for (unsigned int dof=0; dof<fe.dofs_per_line; ++dof)
	    dofs_on_children[next_index++]
	      = face->child(2)->line(1)->dof_index(dof);
	  for (unsigned int dof=0; dof<fe.dofs_per_line; ++dof)
	    dofs_on_children[next_index++]
	      = face->child(0)->line(3)->dof_index(dof);
	  for (unsigned int dof=0; dof<fe.dofs_per_line; ++dof)
	    dofs_on_children[next_index++]
	      = face->child(1)->line(3)->dof_index(dof);
	  
					   // dofs on the bordering lines
	  for (unsigned int line=0; line<4; ++line)
	    for (unsigned int child=0; child<2; ++child)
	      for (unsigned int dof=0; dof!=fe.dofs_per_line; ++dof)
		dofs_on_children[next_index++]
		  = face->line(line)->child(child)->dof_index(dof);
	  
					   // finally, for the dofs interior
					   // to the four child faces
	  for (unsigned int child=0; child<4; ++child)
	    for (unsigned int dof=0; dof!=fe.dofs_per_quad; ++dof)
	      dofs_on_children[next_index++]
		= face->child(child)->dof_index(dof);
	  Assert (next_index == dofs_on_children.size(),
		  ExcInternalError());
	  
					   // for each row in the constraint
					   // matrix for this line:
	  for (unsigned int row=0; row!=dofs_on_children.size(); ++row) 
	    {
	      constraints.add_line (dofs_on_children[row]);
	      for (unsigned int i=0; i!=dofs_on_mother.size(); ++i)
		constraints.add_entry (dofs_on_children[row],
				       dofs_on_mother[i],
				       fe.constraints()(row,i));
	    };
	};
}


void DoFTools::make_hanging_node_constraints (
    const hp::DoFHandler<3> &/*dof_handler*/,
    ConstraintMatrix    &/*constraints*/)
{
//TODO:[?] Implement (required for continuous elements)
    Assert (false, ExcNotImplemented());
}

#endif



template <class DH, typename Number>
void DoFTools::distribute_cell_to_dof_vector (
  const DH             &dof_handler,
  const Vector<Number> &cell_data,
  Vector<double>       &dof_data,
  const unsigned int    component)
{
  const Triangulation<DH::dimension> &tria = dof_handler.get_tria();
  
  Assert (cell_data.size()==tria.n_active_cells(),
	  ExcWrongSize (cell_data.size(), tria.n_active_cells()));
  Assert (dof_data.size()==dof_handler.n_dofs(),
	  ExcWrongSize (dof_data.size(), dof_handler.n_dofs()));
  Assert (component < n_components(dof_handler),
	  ExcInvalidComponent(component, n_components(dof_handler)));
  Assert (fe_is_primitive(dof_handler) == true,
          ExcFENotPrimitive());
  
				   // store a flag whether we should care
				   // about different components. this is
				   // just a simplification, we could ask
				   // for this at every single place
				   // equally well
  const bool consider_components = (n_components(dof_handler) != 1);
  
				   // count how often we have added a value
				   // in the sum for each dof
  std::vector<unsigned char> touch_count (dof_handler.n_dofs(), 0);

  typename DH::active_cell_iterator cell = dof_handler.begin_active(),
				    endc = dof_handler.end();
  std::vector<unsigned int> dof_indices;
  dof_indices.reserve (max_dofs_per_cell(dof_handler));

  for (unsigned int present_cell = 0; cell!=endc; ++cell, ++present_cell) 
    {
      const unsigned int dofs_per_cell = cell->get_fe().dofs_per_cell;
      dof_indices.resize (dofs_per_cell);
      cell->get_dof_indices (dof_indices);
      
      for (unsigned int i=0; i<dofs_per_cell; ++i)
					 // consider this dof only if it
					 // is the right component. if there
					 // is only one component, short cut
					 // the test
	if (!consider_components ||
	    (cell->get_fe().system_to_component_index(i).first == component))
	  {
					     // sum up contribution of the
					     // present_cell to this dof
	    dof_data(dof_indices[i]) += cell_data(present_cell);
					     // note that we added another
					     // summand
	    ++touch_count[dof_indices[i]];
	  };
    };
  
				   // compute the mean value on all the
				   // dofs by dividing with the number
				   // of summands.
  for (unsigned int i=0; i<dof_handler.n_dofs(); ++i)
    {
				       // assert that each dof was used
				       // at least once. this needs not be
				       // the case if the vector has more than
				       // one component
      Assert (consider_components || (touch_count[i]!=0),
	      ExcInternalError());
      if (touch_count[i] != 0)
	dof_data(i) /=  touch_count[i];
    };
}



template <int dim>
void
DoFTools::extract_dofs (
  const DoFHandler<dim>   &dof,
  const std::vector<bool> &component_select,
  std::vector<bool>       &selected_dofs,
  const bool               count_by_blocks)
{
  const FiniteElement<dim> &fe = dof.get_fe();
  
  if (count_by_blocks == true)
    {
      Assert(component_select.size() == fe.n_blocks(),
	     ExcDimensionMismatch(component_select.size(), fe.n_blocks()));
    }
  else
    {
      Assert(component_select.size() == n_components(dof),
	     ExcDimensionMismatch(component_select.size(), n_components(dof)));
    }
  
  Assert(selected_dofs.size() == dof.n_dofs(),
	 ExcDimensionMismatch(selected_dofs.size(), dof.n_dofs()));

                                   // two special cases: no component
                                   // is selected, and all components
                                   // are selected; both rather
                                   // stupid, but easy to catch
  if (std::count (component_select.begin(), component_select.end(), true)
      == 0)
    {
      std::fill_n (selected_dofs.begin(), dof.n_dofs(), false);
      return;
    };
  if (std::count (component_select.begin(), component_select.end(), true)
      == static_cast<signed int>(component_select.size()))
    {
      std::fill_n (selected_dofs.begin(), dof.n_dofs(), true);
      return;
    };
  

				   // preset all values by false
  std::fill_n (selected_dofs.begin(), dof.n_dofs(), false);

                                   // next set up a table for the
                                   // degrees of freedom on each of
                                   // the cells whether it is
                                   // something interesting or not
  std::vector<bool> local_selected_dofs (fe.dofs_per_cell, false);
  for (unsigned int i=0; i<fe.dofs_per_cell; ++i)
    if (count_by_blocks == true)
      local_selected_dofs[i]
        = component_select[fe.system_to_block_index(i).first];
    else
      if (fe.is_primitive(i))
	local_selected_dofs[i]
	  = component_select[fe.system_to_component_index(i).first];
      else
					 // if this shape function is
					 // not primitive, then we have
					 // to work harder. we have to
					 // find out whether _any_ of
					 // the vector components of
					 // this element is selected or
					 // not
					 //
					 // to do so, get the first and
					 // last vector components of
					 // the base element to which
					 // the local dof with index i
					 // belongs
	{
	  unsigned int first_comp = 0;
	  const unsigned int this_base = fe.system_to_base_index(i).first.first;
	  const unsigned int this_multiplicity
	    = fe.system_to_base_index(i).first.second;
	  
	  for (unsigned int b=0; b<this_base; ++b)
	    first_comp += fe.base_element(b).n_components() *
			  fe.element_multiplicity(b);
	  for (unsigned int m=0; m<this_multiplicity; ++m)
	    first_comp += fe.base_element(this_base).n_components();
	  const unsigned int end_comp = first_comp +
					fe.base_element(this_base).n_components();
	  
	  Assert (first_comp < fe.n_components(), ExcInternalError());
	  Assert (end_comp <= fe.n_components(),  ExcInternalError());
	  
					   // now check whether any of
					   // the components in between
					   // is set
	  for (unsigned int c=first_comp; c<end_comp; ++c)
	    if (component_select[c] == true)
	      {
		local_selected_dofs[i] = true;
		break;
	      }
	}
  
                                   // then loop over all cells and do
                                   // the work
  std::vector<unsigned int> indices(fe.dofs_per_cell);
  typename DoFHandler<dim>::active_cell_iterator c;
  for (c=dof.begin_active(); c!=dof.end(); ++ c)
    {
      c->get_dof_indices(indices);
      for (unsigned int i=0; i<fe.dofs_per_cell; ++i)
        selected_dofs[indices[i]] = local_selected_dofs[i];
    }
}


template<int dim>
void
DoFTools::extract_level_dofs(
  const unsigned int       level,
  const MGDoFHandler<dim> &dof,
  const std::vector<bool> &component_select,
  std::vector<bool>       &selected_dofs,
  const bool               count_by_blocks)
{
  const FiniteElement<dim>& fe = dof.get_fe();
  
  if (count_by_blocks == true)
    {
      Assert(component_select.size() == fe.n_blocks(),
	     ExcDimensionMismatch(component_select.size(), fe.n_blocks()));
    }
  else
    {
      Assert(component_select.size() == fe.n_components(),
	     ExcDimensionMismatch(component_select.size(), fe.n_components()));
    }
  
  Assert(selected_dofs.size() == dof.n_dofs(level),
	 ExcDimensionMismatch(selected_dofs.size(), dof.n_dofs(level)));

                                   // two special cases: no component
                                   // is selected, and all components
                                   // are selected, both rather
                                   // stupid, but easy to catch
  if (std::count (component_select.begin(), component_select.end(), true)
      == 0)
    {
      std::fill_n (selected_dofs.begin(), dof.n_dofs(level), false);
      return;
    };
  if (std::count (component_select.begin(), component_select.end(), true)
      == static_cast<signed int>(component_select.size()))
    {
      std::fill_n (selected_dofs.begin(), dof.n_dofs(level), true);
      return;
    };

    				   // preset all values by false
  std::fill_n (selected_dofs.begin(), dof.n_dofs(level), false);

                                   // next set up a table for the
                                   // degrees of freedom on each of
                                   // the cells whether it is
                                   // something interesting or not
  std::vector<bool> local_selected_dofs (fe.dofs_per_cell, false);
  for (unsigned int i=0; i<fe.dofs_per_cell; ++i)
    if (count_by_blocks == true)
      local_selected_dofs[i]
        = component_select[fe.system_to_block_index(i).first];
    else
      if (fe.is_primitive(i))
	local_selected_dofs[i]
	  = component_select[fe.system_to_component_index(i).first];
      else
					 // if this shape function is
					 // not primitive, then we have
					 // to work harder. we have to
					 // find out whether _any_ of
					 // the vector components of
					 // this element is selected or
					 // not
					 //
					 // to do so, get the first and
					 // last vector components of
					 // the base element to which
					 // the local dof with index i
					 // belongs
	{
	  unsigned int first_comp = 0;
	  const unsigned int this_base = fe.system_to_base_index(i).first.first;
	  const unsigned int this_multiplicity
	    = fe.system_to_base_index(i).first.second;
	  
	  for (unsigned int b=0; b<this_base; ++b)
	    first_comp += fe.base_element(b).n_components() *
			  fe.element_multiplicity(b);
	  for (unsigned int m=0; m<this_multiplicity; ++m)
	    first_comp += fe.base_element(this_base).n_components();
	  const unsigned int end_comp = first_comp +
					fe.base_element(this_base).n_components();
	  
	  Assert (first_comp < fe.n_components(), ExcInternalError());
	  Assert (end_comp <= fe.n_components(),  ExcInternalError());
	  
					   // now check whether any of
					   // the components in between
					   // is set
	  for (unsigned int c=first_comp; c<end_comp; ++c)
	    if (component_select[c] == true)
	      {
		local_selected_dofs[i] = true;
		break;
	      }
	}
  
                                   // then loop over all cells and do
                                   // work
  std::vector<unsigned int> indices(fe.dofs_per_cell);  
  typename MGDoFHandler<dim>::cell_iterator c;
  for (c = dof.begin(level) ; c != dof.end(level) ; ++ c)
    {
      c->get_mg_dof_indices(indices);
      for (unsigned int i=0; i<fe.dofs_per_cell; ++i)
        selected_dofs[indices[i]] = local_selected_dofs[i];
    }
}


#if deal_II_dimension != 1

template <class DH>
void
DoFTools::extract_boundary_dofs (const DH                      &dof_handler,
				 const std::vector<bool>       &component_select,
				 std::vector<bool>             &selected_dofs,
				 const std::set<unsigned char> &boundary_indicators)
{
  Assert (component_select.size() == n_components(dof_handler),
	  ExcWrongSize (component_select.size(),
			n_components(dof_handler)));
  Assert (boundary_indicators.find (255) == boundary_indicators.end(),
	  ExcInvalidBoundaryIndicator());
  const unsigned int dim=DH::dimension;

				   // let's see whether we have to
				   // check for certain boundary
				   // indicators or whether we can
				   // accept all
  const bool check_boundary_indicator = (boundary_indicators.size() != 0);

                                   // also see whether we have to
                                   // check whether a certain vector
                                   // component is selected, or all
  const bool check_vector_component
    = (component_select != std::vector<bool>(component_select.size(),
                                             true));
  
				   // clear and reset array by default
				   // values
  selected_dofs.clear ();
  selected_dofs.resize (dof_handler.n_dofs(), false);
  std::vector<unsigned int> face_dof_indices;
  face_dof_indices.reserve (max_dofs_per_face(dof_handler));

				   // now loop over all cells and
				   // check whether their faces are at
				   // the boundary. note that we need
				   // not take special care of single
				   // lines being at the boundary
				   // (using
				   // @p{cell->has_boundary_lines}),
				   // since we do not support
				   // boundaries of dimension dim-2,
				   // and so every isolated boundary
				   // line is also part of a boundary
				   // face which we will be visiting
				   // sooner or later
  for (typename DH::active_cell_iterator cell=dof_handler.begin_active();
       cell!=dof_handler.end(); ++cell)
    for (unsigned int face=0;
	 face<GeometryInfo<DH::dimension>::faces_per_cell; ++face)
      if (cell->at_boundary(face))
	if (! check_boundary_indicator ||
	    (boundary_indicators.find (cell->face(face)->boundary_indicator())
	     != boundary_indicators.end()))
	  {
            const FiniteElement<DH::dimension> &fe = cell->get_fe();
            
            const unsigned int dofs_per_face = fe.dofs_per_face;
            face_dof_indices.resize (dofs_per_face);
	    cell->face(face)->get_dof_indices (face_dof_indices);

 	    for (unsigned int i=0; i<fe.dofs_per_face; ++i)
 	      if (!check_vector_component)
  		selected_dofs[face_dof_indices[i]] = true;
              else
                                                 // check for
                                                 // component is
                                                 // required. somewhat
                                                 // tricky as usual
                                                 // for the case that
                                                 // the shape function
                                                 // is non-primitive,
                                                 // but use usual
                                                 // convention (see
                                                 // docs)
                {
                                                   // first get at the
                                                   // cell-global
                                                   // number of a face
                                                   // dof, to ask the
                                                   // fe certain
                                                   // questions
                  const unsigned int cell_index
                    = (dim == 1 ?
                       i
                       :
                       (dim == 2 ?
                        (i<2*fe.dofs_per_vertex ? i : i+2*fe.dofs_per_vertex)
                        :
                        (dim == 3 ?
                         (i<4*fe.dofs_per_vertex ?
                          i
                          :
                          (i<4*fe.dofs_per_vertex+4*fe.dofs_per_line ?
                           i+4*fe.dofs_per_vertex
                           :
                           i+4*fe.dofs_per_vertex+8*fe.dofs_per_line))
                         :
                         deal_II_numbers::invalid_unsigned_int)));
                  if (fe.is_primitive (cell_index))
                    selected_dofs[face_dof_indices[i]]
                      = (component_select[fe.face_system_to_component_index(i).first]
                         == true);
                  else // not primitive
                    {
                      const unsigned int first_nonzero_comp
                        = (std::find (fe.get_nonzero_components(cell_index).begin(),
                                      fe.get_nonzero_components(cell_index).end(),
                                      true)
                           -
                           fe.get_nonzero_components(cell_index).begin());
                      Assert (first_nonzero_comp < fe.n_components(),
                              ExcInternalError());
                       
                      selected_dofs[face_dof_indices[i]]
                        = (component_select[first_nonzero_comp]
                           == true);
                    }
                }
	  }
}


#else  // 1d


template <class DH>
void
DoFTools::extract_boundary_dofs (const DH                 &dof_handler,
				 const std::vector<bool>  &component_select,
				 std::vector<bool>        &selected_dofs,
				 const std::set<unsigned char> &boundary_indicators)
{
  Assert (component_select.size() == n_components(dof_handler),
	  ExcWrongSize (component_select.size(),
			n_components(dof_handler)));
	  
				   // clear and reset array by default
				   // values
  selected_dofs.clear ();
  selected_dofs.resize (dof_handler.n_dofs(), false);

				   // let's see whether we have to
				   // check for certain boundary
				   // indicators or whether we can
				   // accept all
  const bool check_left_vertex  = ((boundary_indicators.size() == 0) ||
				   (boundary_indicators.find(0) !=
				    boundary_indicators.end()));
  const bool check_right_vertex = ((boundary_indicators.size() == 0) ||
				   (boundary_indicators.find(1) !=
				    boundary_indicators.end()));

                                   // see whether we have to check
                                   // whether a certain vector
                                   // component is selected, or all
  const bool check_vector_component
    = (component_select != std::vector<bool>(component_select.size(),
                                             true));
  
				   // loop over coarse grid cells
  for (typename DH::cell_iterator cell=dof_handler.begin(0);
       cell!=dof_handler.end(0); ++cell)
    {
      const FiniteElement<1> &fe = cell->get_fe();
            
				       // check left-most vertex
      if (check_left_vertex)
	if (cell->neighbor(0) == dof_handler.end())
          {
				                 // In 1D the number of DoFs
	                                         // on the faces should be
                                                 // equal to the number of DoFs
                                                 // on the vertices.
            Assert (fe.dofs_per_face == 
		    fe.dofs_per_vertex,
                    ExcInternalError());
            
            for (unsigned int i=0; i<fe.dofs_per_face; ++i)
              if (!check_vector_component)
                selected_dofs[cell->vertex_dof_index(0,i)] = true;
              else
                                                 // check
                                                 // component. make sure
                                                 // we don't ask the
                                                 // wrong question
                                                 // (leading to an
                                                 // exception) in case
                                                 // the shape function
                                                 // is non-primitive. note
                                                 // that the face dof
                                                 // index i is also the
                                                 // cell dof index of a
                                                 // corresponding dof in 1d
                {
                  const unsigned int component =
                    (fe.is_primitive(i) ?
                     fe.face_system_to_component_index(i).first :
                     (std::find (fe.get_nonzero_components(i).begin(),
                                 fe.get_nonzero_components(i).end(),
                                 true)
                      -
                      fe.get_nonzero_components(i).begin()));
                  Assert (component < fe.n_components(),
                          ExcInternalError());
                 
                  if (component_select[component] == true)
                    selected_dofs[cell->vertex_dof_index(0,i)] = true;
                }
          }
      
                                             // check right-most
                                             // vertex. same procedure here
                                             // as above
      if (check_right_vertex)
	if (cell->neighbor(1) == dof_handler.end())
          {
            Assert (fe.dofs_per_face ==
                    fe.dofs_per_vertex,
                    ExcInternalError());
            
            for (unsigned int i=0; i<fe.dofs_per_face; ++i)
              if (!check_vector_component)
                selected_dofs[cell->vertex_dof_index(1,i)] = true;
              else
                {
                  const unsigned int component =
                    (fe.is_primitive(i) ?
                     fe.face_system_to_component_index(i).first :
                     (std::find (fe.get_nonzero_components(i).begin(),
                                 fe.get_nonzero_components(i).end(),
                                 true)
                      -
                      fe.get_nonzero_components(i).begin()));
                  Assert (component < fe.n_components(),
                          ExcInternalError());
                 
                  if (component_select[component] == true)
                    selected_dofs[cell->vertex_dof_index(1,i)] = true;
                }
          }
    }
}


#endif



#if deal_II_dimension == 1

void
DoFTools::extract_hanging_node_dofs (const DoFHandler<1> &dof_handler,
				     std::vector<bool>   &selected_dofs)
{
  Assert(selected_dofs.size() == dof_handler.n_dofs(),
	 ExcDimensionMismatch(selected_dofs.size(), dof_handler.n_dofs()));
				   // preset all values by false
  std::fill_n (selected_dofs.begin(), dof_handler.n_dofs(), false);

				   // there are no hanging nodes in 1d
}

#endif


#if deal_II_dimension == 2

void
DoFTools::extract_hanging_node_dofs (const DoFHandler<2> &dof_handler,
				     std::vector<bool>   &selected_dofs)
{
  const unsigned int dim = 2;
  
  Assert(selected_dofs.size() == dof_handler.n_dofs(),
	 ExcDimensionMismatch(selected_dofs.size(), dof_handler.n_dofs()));
				   // preset all values by false
  fill_n (selected_dofs.begin(), dof_handler.n_dofs(), false);

  const FiniteElement<dim> &fe   = dof_handler.get_fe();

				   // this function is similar to the
				   // make_sparsity_pattern function,
				   // see there for more information
  DoFHandler<dim>::active_cell_iterator cell = dof_handler.begin_active(),
					endc = dof_handler.end();
  for (; cell!=endc; ++cell)
    for (unsigned int face=0; face<GeometryInfo<dim>::faces_per_cell; ++face)
      if (cell->face(face)->has_children()) 
	{
	  const DoFHandler<dim>::line_iterator line = cell->face(face);

	  for (unsigned int dof=0; dof!=fe.dofs_per_vertex; ++dof)
	    selected_dofs[line->child(0)->vertex_dof_index(1,dof)] = true;
	  
	  for (unsigned int child=0; child<2; ++child)
	    for (unsigned int dof=0; dof!=fe.dofs_per_line; ++dof)
	      selected_dofs[line->child(child)->dof_index(dof)] = true;
	};
}

#endif



#if deal_II_dimension == 3

void
DoFTools::extract_hanging_node_dofs (const DoFHandler<3> &dof_handler,
				     std::vector<bool>   &selected_dofs)
{
  const unsigned int dim = 3;

  Assert(selected_dofs.size() == dof_handler.n_dofs(),
	 ExcDimensionMismatch(selected_dofs.size(), dof_handler.n_dofs()));
				   // preset all values by false
  std::fill_n (selected_dofs.begin(), dof_handler.n_dofs(), false);

  const FiniteElement<dim> &fe   = dof_handler.get_fe();
  
				   // this function is similar to the
				   // make_sparsity_pattern function,
				   // see there for more information

  DoFHandler<dim>::active_cell_iterator cell = dof_handler.begin_active(),
					endc = dof_handler.end();
  for (; cell!=endc; ++cell)
    for (unsigned int f=0; f<GeometryInfo<dim>::faces_per_cell; ++f)
      if (cell->face(f)->has_children()) 
	{
	  const DoFHandler<dim>::face_iterator face = cell->face(f);
	  
	  for (unsigned int dof=0; dof!=fe.dofs_per_vertex; ++dof)
	    selected_dofs[face->child(0)->vertex_dof_index(2,dof)] = true;
	  
					   // dof numbers on the centers of
					   // the lines bounding this face
	  for (unsigned int line=0; line<4; ++line)
	    for (unsigned int dof=0; dof!=fe.dofs_per_vertex; ++dof)
	      selected_dofs[face->line(line)->child(0)->vertex_dof_index(1,dof)] = true;
	  
					   // next the dofs on the lines interior
					   // to the face; the order of these
					   // lines is laid down in the
					   // FiniteElement class documentation
	  for (unsigned int dof=0; dof<fe.dofs_per_line; ++dof)
	    selected_dofs[face->child(0)->line(1)->dof_index(dof)] = true;
	  for (unsigned int dof=0; dof<fe.dofs_per_line; ++dof)
	    selected_dofs[face->child(1)->line(2)->dof_index(dof)] = true;
	  for (unsigned int dof=0; dof<fe.dofs_per_line; ++dof)
	    selected_dofs[face->child(2)->line(3)->dof_index(dof)] = true;
	  for (unsigned int dof=0; dof<fe.dofs_per_line; ++dof)
	    selected_dofs[face->child(3)->line(0)->dof_index(dof)] = true;
	  
					   // dofs on the bordering lines
	  for (unsigned int line=0; line<4; ++line)
	    for (unsigned int child=0; child<2; ++child)
	      for (unsigned int dof=0; dof!=fe.dofs_per_line; ++dof)
		selected_dofs[face->line(line)->child(child)->dof_index(dof)] = true;
	
					   // finally, for the dofs interior
					   // to the four child faces
	  for (unsigned int child=0; child<4; ++child)
	    for (unsigned int dof=0; dof!=fe.dofs_per_quad; ++dof)
	      selected_dofs[face->child(child)->dof_index(dof)] = true;
	};
}

#endif



template <class DH>
void
DoFTools::extract_subdomain_dofs (const DH           &dof_handler,
				  const unsigned int  subdomain_id,
				  std::vector<bool>  &selected_dofs)
{
  Assert(selected_dofs.size() == dof_handler.n_dofs(),
	 ExcDimensionMismatch(selected_dofs.size(), dof_handler.n_dofs()));

                                   // preset all values by false
  std::fill_n (selected_dofs.begin(), dof_handler.n_dofs(), false);

  std::vector<unsigned int> local_dof_indices;
  local_dof_indices.reserve (max_dofs_per_cell(dof_handler));
  
				   // this function is similar to the
				   // make_sparsity_pattern function,
				   // see there for more information
  typename DH::active_cell_iterator 
    cell = dof_handler.begin_active(),
    endc = dof_handler.end();
  for (; cell!=endc; ++cell)
    if (cell->subdomain_id() == subdomain_id)
      {
        const unsigned int dofs_per_cell = cell->get_fe().dofs_per_cell;
        local_dof_indices.resize (dofs_per_cell);
	cell->get_dof_indices (local_dof_indices);
	for (unsigned int i=0; i<dofs_per_cell; ++i)
	  selected_dofs[local_dof_indices[i]] = true;
      };
}



template <class DH>
void
DoFTools::get_subdomain_association (const DH                  &dof_handler,
				     std::vector<unsigned int> &subdomain_association)
{
  Assert(subdomain_association.size() == dof_handler.n_dofs(),
	 ExcDimensionMismatch(subdomain_association.size(),
                              dof_handler.n_dofs()));

                                   // preset all values by an invalid value
  std::fill_n (subdomain_association.begin(), dof_handler.n_dofs(),
               deal_II_numbers::invalid_unsigned_int);

  std::vector<unsigned int> local_dof_indices;
  local_dof_indices.reserve (max_dofs_per_cell(dof_handler));
  
				   // this function is similar to the
				   // make_sparsity_pattern function,
				   // see there for more information

  typename DH::active_cell_iterator
    cell = dof_handler.begin_active(),
    endc = dof_handler.end();
  for (; cell!=endc; ++cell)
    {
      const unsigned int subdomain_id = cell->subdomain_id();
      const unsigned int dofs_per_cell = cell->get_fe().dofs_per_cell;
      local_dof_indices.resize (dofs_per_cell);
      cell->get_dof_indices (local_dof_indices);

                                       // set subdomain ids. if dofs already
                                       // have their values set then they must
                                       // be on partition interfaces. don't
                                       // worry about that, just overwrite it
      for (unsigned int i=0; i<dofs_per_cell; ++i)
        subdomain_association[local_dof_indices[i]] = subdomain_id;
    };

  Assert (std::find (subdomain_association.begin(),
                     subdomain_association.end(),
                     deal_II_numbers::invalid_unsigned_int)
          == subdomain_association.end(),
          ExcInternalError());
}



template <class DH>
unsigned int
DoFTools::count_dofs_with_subdomain_association (
  const DH           &dof_handler,
  const unsigned int  subdomain)
{
                                   // in debug mode, make sure that there are
                                   // some cells at least with this subdomain
                                   // id
#ifdef DEBUG
  {
    bool found = false;
    for (typename Triangulation<DH::dimension>::active_cell_iterator
           cell=dof_handler.get_tria().begin_active();
         cell!=dof_handler.get_tria().end(); ++cell)
      if (cell->subdomain_id() == subdomain)
        {
          found = true;
          break;
        }
    Assert (found == true,
            ExcMessage ("There are no cells for the given subdomain!"));
  } 
#endif

  std::vector<unsigned int> subdomain_association (dof_handler.n_dofs());
  get_subdomain_association (dof_handler, subdomain_association);

  return std::count (subdomain_association.begin(),
                     subdomain_association.end(),
                     subdomain);
}



template <int dim>
void
DoFTools::count_dofs_per_component (
  const DoFHandler<dim>&     dof_handler,
  std::vector<unsigned int>& dofs_per_component,
  bool only_once,
  std::vector<unsigned int>  target_component)
{
  const FiniteElement<dim>& fe = dof_handler.get_fe();
  const unsigned int n_components = fe.n_components();
  dofs_per_component.resize (n_components);
  std::fill (dofs_per_component.begin(), dofs_per_component.end(), 0U);
  
				   // If the empty vector was given as
				   // default argument, set up this
				   // vector as identity.
  if (target_component.size()==0)
    {
      target_component.resize(n_components);
      for (unsigned int i=0;i<n_components;++i)
	target_component[i] = i;
    }
  
  Assert(target_component.size()==n_components,
	 ExcDimensionMismatch(target_component.size(),n_components));

				   // special case for only one
				   // component. treat this first
				   // since it does not require any
				   // computations
  if (n_components == 1)
    {
      dofs_per_component[0] = dof_handler.n_dofs();
      return;
    }

      
				   // otherwise determine the number
				   // of dofs in each component
				   // separately. do so in parallel
  std::vector<std::vector<bool> >
    dofs_in_component (n_components,
                       std::vector<bool>(dof_handler.n_dofs(), false));
  std::vector<std::vector<bool> >
    component_select (n_components,
                      std::vector<bool>(n_components, false));
  Threads::ThreadGroup<> threads;
  for (unsigned int i=0; i<n_components; ++i)
    {
      void (*fun_ptr) (const DoFHandler<dim>   &,
		       const std::vector<bool> &,
		       std::vector<bool>       &,
		       bool)
        = &DoFTools::template extract_dofs<dim>;
      component_select[i][i] = true;
      threads += Threads::spawn (fun_ptr)(dof_handler, component_select[i],
                                          dofs_in_component[i], false);
    };
  threads.join_all ();

				   // next count what we got
  unsigned int component = 0;
  for (unsigned int b=0;b<fe.n_base_elements();++b)
    {
      const FiniteElement<dim>& base = fe.base_element(b);
				       // Dimension of base element
      unsigned int d = base.n_components();
      
      for (unsigned int m=0;m<fe.element_multiplicity(b);++m)
	{
	  for (unsigned int dd=0;dd<d;++dd)
	    {
	      if (base.is_primitive() || (!only_once || dd==0))
	      dofs_per_component[target_component[component]]
		+= std::count(dofs_in_component[component].begin(),
			      dofs_in_component[component].end(),
			      true);
	      ++component;
	    }
	}
    }
  
				   // finally sanity check. this is
				   // only valid if the finite element
				   // is actually primitive, so
				   // exclude other elements from this
  Assert (!dof_handler.get_fe().is_primitive()
          ||
          (std::accumulate (dofs_per_component.begin(),
                            dofs_per_component.end(), 0U)
           == dof_handler.n_dofs()),
	  ExcInternalError());
}


template <int dim>
void
DoFTools::count_dofs_per_block (
  const DoFHandler<dim>&     dof_handler,
  std::vector<unsigned int>& dofs_per_block,
  std::vector<unsigned int>  target_block)
{
  const FiniteElement<dim>& fe = dof_handler.get_fe();
  const unsigned int n_blocks = fe.n_blocks();
  dofs_per_block.resize (n_blocks);
  std::fill (dofs_per_block.begin(), dofs_per_block.end(), 0U);
  
				   // If the empty vector was given as
				   // default argument, set up this
				   // vector as identity.
  if (target_block.size()==0)
    {
      target_block.resize(n_blocks);
      for (unsigned int i=0;i<n_blocks;++i)
	target_block[i] = i;
    }
  
  Assert(target_block.size()==n_blocks,
	 ExcDimensionMismatch(target_block.size(),n_blocks));

				   // special case for only one
				   // block. treat this first
				   // since it does not require any
				   // computations
  if (n_blocks == 1)
    {
      dofs_per_block[0] = dof_handler.n_dofs();
      return;
    } 
				   // otherwise determine the number
				   // of dofs in each block
				   // separately. do so in parallel
  std::vector<std::vector<bool> >
    dofs_in_block (n_blocks, std::vector<bool>(dof_handler.n_dofs(), false));
  std::vector<std::vector<bool> >
    block_select (n_blocks, std::vector<bool>(n_blocks, false));
  Threads::ThreadGroup<> threads;
  for (unsigned int i=0; i<n_blocks; ++i)
    {
      void (*fun_ptr) (const DoFHandler<dim>   &,
		       const std::vector<bool> &,
		       std::vector<bool>       &,
		       bool)
        = &DoFTools::template extract_dofs<dim>;
      block_select[i][i] = true;
      threads += Threads::spawn (fun_ptr)(dof_handler, block_select[i],
                                          dofs_in_block[i], true);
    };
  threads.join_all ();

				   // next count what we got
  for (unsigned int block=0;block<fe.n_blocks();++block)
    dofs_per_block[target_block[block]]
      += std::count(dofs_in_block[block].begin(),
		    dofs_in_block[block].end(),
		    true);
}


template <int dim>
void
DoFTools::count_dofs_per_component (
  const DoFHandler<dim>&     dof_handler,
  std::vector<unsigned int>& dofs_per_component,
  std::vector<unsigned int>  target_component)
{
  count_dofs_per_component (dof_handler, dofs_per_component, false, target_component);
}


template <int dim>
void
DoFTools::compute_intergrid_constraints (
  const DoFHandler<dim>              &coarse_grid,
  const unsigned int                  coarse_component,
  const DoFHandler<dim>              &fine_grid,
  const unsigned int                  fine_component,
  const InterGridMap<DoFHandler<dim> > &coarse_to_fine_grid_map,
  ConstraintMatrix                   &constraints)
{
				   // store the weights with which a dof
				   // on the parameter grid contributes
				   // to a dof on the fine grid. see the
				   // long doc below for more info
				   //
				   // allocate as many rows as there are
				   // parameter dofs on the coarse grid
				   // and as many columns as there are
				   // parameter dofs on the fine grid.
				   //
				   // weight_mapping is used to map the
				   // global (fine grid) parameter dof
				   // indices to the columns
				   //
				   // in the original implementation,
				   // the weights array was actually
				   // of FullMatrix<double> type. this
				   // wasted huge amounts of memory,
				   // but was fast. nonetheless, since
				   // the memory consumption was
				   // quadratic in the number of
				   // degrees of freedom, this was not
				   // very practical, so we now use a
				   // vector of rows of the matrix,
				   // and in each row a vector of
				   // pairs (colnum,value). this seems
				   // like the best tradeoff between
				   // memory and speed, as it is now
				   // linear in memory and still fast
				   // enough.
				   //
				   // to save some memory and since
				   // the weights are usually
				   // (negative) powers of 2, we
				   // choose the value type of the
				   // matrix to be @p{float} rather
				   // than @p{double}.
  std::vector<std::map<unsigned int, float> > weights;

				   // this is this mapping. there is one
				   // entry for each dof on the fine grid;
				   // if it is a parameter dof, then its
				   // value is the column in weights for
				   // that parameter dof, if it is any
				   // other dof, then its value is -1,
				   // indicating an error
  std::vector<int> weight_mapping;

  const unsigned int n_parameters_on_fine_grid
    = compute_intergrid_weights_1 (coarse_grid, coarse_component, fine_grid, fine_component,
				   coarse_to_fine_grid_map, weights, weight_mapping);
  
				   // global numbers of dofs
  const unsigned int n_coarse_dofs = coarse_grid.n_dofs(),
		     n_fine_dofs   = fine_grid.n_dofs();


				   // get an array in which we store
				   // which dof on the coarse grid is
				   // a parameter and which is not
  std::vector<bool> coarse_dof_is_parameter (coarse_grid.n_dofs());
  if (true)
    {
      std::vector<bool> mask (coarse_grid.get_fe().n_components(),
			      false);
      mask[coarse_component] = true;
      extract_dofs (coarse_grid, mask, coarse_dof_is_parameter);
    };
  
				   // now we know that the weights in
				   // each row constitute a
				   // constraint. enter this into the
				   // constraints object
				   //
				   // first task: for each parameter
				   // dof on the parameter grid, find
				   // a representant on the fine,
				   // global grid. this is possible
				   // since we use conforming finite
				   // element. we take this
				   // representant to be the first
				   // element in this row with weight
				   // identical to one. the
				   // representant will become an
				   // unconstrained degree of freedom,
				   // while all others will be
				   // constrained to this dof (and
				   // possibly others)
  std::vector<int> representants(n_coarse_dofs, -1);
  for (unsigned int parameter_dof=0; parameter_dof<n_coarse_dofs;
       ++parameter_dof)
    if (coarse_dof_is_parameter[parameter_dof] == true)
      {
					 // if this is the line of a
					 // parameter dof on the
					 // coarse grid, then it
					 // should have at least one
					 // dependent node on the fine
					 // grid
	Assert (weights[parameter_dof].size() > 0, ExcInternalError());

					 // find the column where the
					 // representant is mentioned
	std::map<unsigned int,float>::const_iterator i = weights[parameter_dof].begin();
	for (; i!=weights[parameter_dof].end(); ++i)
	  if (i->second == 1)
	    break;
	Assert (i!=weights[parameter_dof].end(), ExcInternalError());
	const unsigned int column = i->first;
	
					 // now we know in which column of
					 // weights the representant is, but
					 // we don't know its global index. get
					 // it using the inverse operation of
					 // the weight_mapping
	unsigned int global_dof=0;
	for (; global_dof<weight_mapping.size(); ++global_dof)
	  if (weight_mapping[global_dof] == static_cast<int>(column))
	    break;
	Assert (global_dof < weight_mapping.size(), ExcInternalError());
	
					 // now enter the representants global
					 // index into our list
	representants[parameter_dof] = global_dof;
      }
    else
      {
					 // consistency check: if this
					 // is no parameter dof on the
					 // coarse grid, then the
					 // respective row must be
					 // empty!
	Assert (weights[parameter_dof].size() == 0, ExcInternalError());
      };
  


				   // note for people that want to
				   // optimize this function: the
				   // largest part of the computing
				   // time is spent in the following,
				   // rather innocent block of
				   // code. basically, it must be the
				   // ConstraintMatrix::add_entry call
				   // which takes the bulk of the
				   // time, but it is not known to the
				   // author how to make it faster...
  std::vector<std::pair<unsigned int,double> > constraint_line;
  for (unsigned int global_dof=0; global_dof<n_fine_dofs; ++global_dof)
    if (weight_mapping[global_dof] != -1)
				       // this global dof is a parameter
				       // dof, so it may carry a constraint
				       // note that for each global dof,
				       // the sum of weights shall be one,
				       // so we can find out whether this
				       // dof is constrained in the following
				       // way: if the only weight in this row
				       // is a one, and the representant for
				       // the parameter dof of the line in
				       // which this one is is the present
				       // dof, then we consider this dof
				       // to be unconstrained. otherwise,
				       // all other dofs are constrained
      {
	const unsigned int col = weight_mapping[global_dof];
	Assert (col < n_parameters_on_fine_grid, ExcInternalError());
	
	unsigned int first_used_row=0;

	{
	  Assert (weights.size() > 0, ExcInternalError());
	  std::map<unsigned int,float>::const_iterator
	    col_entry = weights[0].end();
	  for (; first_used_row<n_coarse_dofs; ++first_used_row)
	    {
	      col_entry = weights[first_used_row].find(col);
	      if (col_entry != weights[first_used_row].end())
		break;
	    }

	  Assert (col_entry != weights[first_used_row].end(), ExcInternalError());
	    
	  if ((col_entry->second == 1) &&
	      (representants[first_used_row] == static_cast<int>(global_dof)))
					     // dof unconstrained or
					     // constrained to itself
					     // (in case this cell is
					     // mapped to itself, rather
					     // than to children of
					     // itself)
	    continue;
	}


					 // otherwise enter all constraints
	constraints.add_line (global_dof);

	constraint_line.clear ();
	for (unsigned int row=first_used_row; row<n_coarse_dofs; ++row)
	  {
	    const std::map<unsigned int,float>::const_iterator
	      j = weights[row].find(col);
	    if ((j != weights[row].end()) && (j->second != 0))
	      constraint_line.push_back (std::make_pair(representants[row],
							j->second));
	  };
	
	constraints.add_entries (global_dof, constraint_line);
      };
}



template <int dim>
void
DoFTools::
compute_intergrid_transfer_representation (
  const DoFHandler<dim>              &coarse_grid,
  const unsigned int                  coarse_component,
  const DoFHandler<dim>              &fine_grid,
  const unsigned int                  fine_component,
  const InterGridMap<DoFHandler<dim> > &coarse_to_fine_grid_map,
  std::vector<std::map<unsigned int, float> > &transfer_representation)
{
				   // store the weights with which a dof
				   // on the parameter grid contributes
				   // to a dof on the fine grid. see the
				   // long doc below for more info
				   //
				   // allocate as many rows as there are
				   // parameter dofs on the coarse grid
				   // and as many columns as there are
				   // parameter dofs on the fine grid.
				   //
				   // weight_mapping is used to map the
				   // global (fine grid) parameter dof
				   // indices to the columns
				   //
				   // in the original implementation,
				   // the weights array was actually
				   // of FullMatrix<double> type. this
				   // wasted huge amounts of memory,
				   // but was fast. nonetheless, since
				   // the memory consumption was
				   // quadratic in the number of
				   // degrees of freedom, this was not
				   // very practical, so we now use a
				   // vector of rows of the matrix,
				   // and in each row a vector of
				   // pairs (colnum,value). this seems
				   // like the best tradeoff between
				   // memory and speed, as it is now
				   // linear in memory and still fast
				   // enough.
				   //
				   // to save some memory and since
				   // the weights are usually
				   // (negative) powers of 2, we
				   // choose the value type of the
				   // matrix to be @p{float} rather
				   // than @p{double}.
  std::vector<std::map<unsigned int, float> > weights;

				   // this is this mapping. there is one
				   // entry for each dof on the fine grid;
				   // if it is a parameter dof, then its
				   // value is the column in weights for
				   // that parameter dof, if it is any
				   // other dof, then its value is -1,
				   // indicating an error
  std::vector<int> weight_mapping;

  compute_intergrid_weights_1 (coarse_grid, coarse_component, fine_grid, fine_component,
			       coarse_to_fine_grid_map, weights, weight_mapping);
  
				   // now compute the requested
				   // representation
  const unsigned int n_global_parm_dofs
    = std::count_if (weight_mapping.begin(), weight_mapping.end(),
		     std::bind2nd (std::not_equal_to<int> (), -1));
  
				   // first construct the inverse
				   // mapping of weight_mapping
  std::vector<unsigned int> inverse_weight_mapping (n_global_parm_dofs,
						    DoFHandler<dim>::invalid_dof_index);
  for (unsigned int i=0; i<weight_mapping.size(); ++i)
    {
      const unsigned int parameter_dof = weight_mapping[i];
				       // if this global dof is a
				       // parameter
      if (parameter_dof != deal_II_numbers::invalid_unsigned_int)
	{
	  Assert (parameter_dof < n_global_parm_dofs, ExcInternalError());
	  Assert (inverse_weight_mapping[parameter_dof] == DoFHandler<dim>::invalid_dof_index,
		  ExcInternalError());
	  
	  inverse_weight_mapping[parameter_dof] = i;
	};
    };
  
				   // next copy over weights array
				   // and replace respective
				   // numbers
  const unsigned int n_rows = weight_mapping.size();
  
  transfer_representation.clear ();
  transfer_representation.resize (n_rows);
  
  const unsigned int n_coarse_dofs = coarse_grid.n_dofs();
  for (unsigned int i=0; i<n_coarse_dofs; ++i)
    {      
      std::map<unsigned int, float>::const_iterator j = weights[i].begin();
      for (; j!=weights[i].end(); ++j)
	{
	  const unsigned int p = inverse_weight_mapping[j->first];
	  Assert (p<n_rows, ExcInternalError());
	  
	  transfer_representation[p][i] = j->second;
	};
    };
}



template <int dim>
unsigned int
DoFTools::compute_intergrid_weights_1 (
  const DoFHandler<dim>              &coarse_grid,
  const unsigned int                  coarse_component,
  const DoFHandler<dim>              &fine_grid,
  const unsigned int                  fine_component,
  const InterGridMap<DoFHandler<dim> > &coarse_to_fine_grid_map,
  std::vector<std::map<unsigned int, float> > &weights,
  std::vector<int>                   &weight_mapping)
{
				   // aliases to the finite elements
				   // used by the dof handlers:
  const FiniteElement<dim> &coarse_fe = coarse_grid.get_fe(),
			   &fine_fe   = fine_grid.get_fe();

				   // global numbers of dofs
  const unsigned int n_coarse_dofs = coarse_grid.n_dofs(),
		     n_fine_dofs   = fine_grid.n_dofs();

				   // local numbers of dofs
  const unsigned int fine_dofs_per_cell   = fine_fe.dofs_per_cell;

				   // alias the number of dofs per
				   // cell belonging to the
				   // coarse_component which is to be
				   // the restriction of the fine
				   // grid:
  const unsigned int coarse_dofs_per_cell_component
    = coarse_fe.base_element(coarse_fe.component_to_base_index(coarse_component).first).dofs_per_cell;
  

				   // Try to find out whether the
				   // grids stem from the same coarse
				   // grid. This is a rather crude
				   // test, but better than nothing
  Assert (coarse_grid.get_tria().n_cells(0) == fine_grid.get_tria().n_cells(0),
	  ExcGridsDontMatch());

				   // check whether the map correlates
				   // the right objects
  Assert (&coarse_to_fine_grid_map.get_source_grid() == &coarse_grid,
	  ExcGridsDontMatch ());
  Assert (&coarse_to_fine_grid_map.get_destination_grid() == &fine_grid,
	  ExcGridsDontMatch ());
  
  
				   // check whether component numbers
				   // are valid
  Assert (coarse_component < coarse_fe.n_components(),
	  ExcInvalidComponent (coarse_component, coarse_fe.n_components()));
  Assert (fine_component < fine_fe.n_components(),
	  ExcInvalidComponent (fine_component, fine_fe.n_components()));
				   // check whether respective finite
				   // elements are equal
  Assert (coarse_fe.base_element (coarse_fe.component_to_base_index(coarse_component).first)
	  ==
	  fine_fe.base_element (fine_fe.component_to_base_index(fine_component).first),
	  ExcFiniteElementsDontMatch());

#ifdef DEBUG
				   // if in debug mode, check whether
				   // the coarse grid is indeed
				   // coarser everywhere than the fine
				   // grid
  for (typename DoFHandler<dim>::active_cell_iterator cell=coarse_grid.begin_active();
       cell != coarse_grid.end(); ++cell)
    Assert (cell->level() <= coarse_to_fine_grid_map[cell]->level(),
	    ExcGridNotCoarser());
#endif

  

/*
 * From here on: the term `parameter' refers to the selected component
 * on the coarse grid and its analogon on the fine grid. The naming of
 * variables containing this term is due to the fact that
 * `selected_component' is longer, but also due to the fact that the
 * code of this function was initially written for a program where the
 * component which we wanted to match between grids was actually the
 * `parameter' variable.
 *
 * Likewise, the terms `parameter grid' and `state grid' refer to the
 * coarse and fine grids, respectively.
 *
 * Changing the names of variables would in principle be a good idea,
 * but would not make things simpler and would be another source of
 * errors. If anyone feels like doing so: patches would be welcome!
 */


  
				   // set up vectors of cell-local
				   // data; each vector represents one
				   // degree of freedom of the
				   // coarse-grid variable in the
				   // fine-grid element
  std::vector<Vector<double> > parameter_dofs (coarse_dofs_per_cell_component,
					       Vector<double>(fine_dofs_per_cell));
				   // for each coarse dof: find its
				   // position within the fine element
				   // and set this value to one in the
				   // respective vector (all other values
				   // are zero by construction)
  for (unsigned int local_coarse_dof=0;
       local_coarse_dof<coarse_dofs_per_cell_component;
       ++local_coarse_dof)
    for (unsigned int fine_dof=0; fine_dof<fine_fe.dofs_per_cell; ++fine_dof)
      if (fine_fe.system_to_component_index(fine_dof)
          ==
          std::make_pair (fine_component, local_coarse_dof))
        {
          parameter_dofs[local_coarse_dof](fine_dof) = 1.;
          break;
        };


				   // find out how many DoFs there are
				   // on the grids belonging to the
				   // components we want to match
  unsigned int n_parameters_on_fine_grid=0;
  if (true)
    {
				       // have a flag for each dof on
				       // the fine grid and set it
				       // to true if this is an
				       // interesting dof. finally count
				       // how many true's there
      std::vector<bool> dof_is_interesting (fine_grid.n_dofs(), false);
      std::vector<unsigned int>  local_dof_indices (fine_fe.dofs_per_cell);
      
      for (typename DoFHandler<dim>::active_cell_iterator
	     cell=fine_grid.begin_active();
	   cell!=fine_grid.end(); ++cell)
	{
	  cell->get_dof_indices (local_dof_indices);
	  for (unsigned int i=0; i<fine_fe.dofs_per_cell; ++i)
	    if (fine_fe.system_to_component_index(i).first == fine_component)
	      dof_is_interesting[local_dof_indices[i]] = true;
	};

      n_parameters_on_fine_grid = std::count (dof_is_interesting.begin(),
					      dof_is_interesting.end(),
					      true);
    };  
  

				   // set up the weights mapping
  weights.clear ();
  weights.resize (n_coarse_dofs);

  weight_mapping.clear ();
  weight_mapping.resize (n_fine_dofs, -1);
  
  if (true)
    {
      std::vector<unsigned int> local_dof_indices(fine_fe.dofs_per_cell);
      unsigned int next_free_index=0;
      for (typename DoFHandler<dim>::active_cell_iterator cell=fine_grid.begin_active();
	   cell != fine_grid.end(); ++cell)
	{
	  cell->get_dof_indices (local_dof_indices);
	  for (unsigned int i=0; i<fine_fe.dofs_per_cell; ++i)
					     // if this DoF is a
					     // parameter dof and has
					     // not yet been numbered,
					     // then do so
	    if ((fine_fe.system_to_component_index(i).first == fine_component) &&
		(weight_mapping[local_dof_indices[i]] == -1))
	      {
		weight_mapping[local_dof_indices[i]] = next_free_index;
		++next_free_index;
	      };
	};

      Assert (next_free_index == n_parameters_on_fine_grid,
	      ExcInternalError());
    };

  
				   // for each cell on the parameter grid:
				   // find out which degrees of freedom on the
				   // fine grid correspond in which way to
				   // the degrees of freedom on the parameter
				   // grid
				   //
				   // do this in a separate function
				   // to allow for multithreading
				   // there. see this function also if
				   // you want to read more
				   // information on the algorithm
				   // used.
  compute_intergrid_weights_2 (coarse_grid, coarse_component,
			       coarse_to_fine_grid_map, parameter_dofs,
			       weight_mapping, weights);


				   // ok, now we have all weights for each
				   // dof on the fine grid. if in debug
				   // mode lets see if everything went smooth,
				   // i.e. each dof has sum of weights one
				   //
				   // in other words this means that
				   // if the sum of all shape
				   // functions on the parameter grid
				   // is one (which is always the
				   // case), then the representation
				   // on the state grid should be as
				   // well (division of unity)
				   //
				   // if the parameter grid has more
				   // than one component, then the
				   // respective dofs of the other
				   // components have sum of weights
				   // zero, of course. we do not
				   // explicitly ask which component
				   // a dof belongs to, but this at
				   // least tests some errors
#ifdef DEBUG
  for (unsigned int col=0; col<n_parameters_on_fine_grid; ++col)
    {
      double sum=0;
      for (unsigned int row=0; row<n_coarse_dofs; ++row)
	if (weights[row].find(col) != weights[row].end())
	  sum += weights[row][col];
      Assert ((std::fabs(sum-1) < 1.e-12) ||
	      ((coarse_fe.n_components()>1) && (sum==0)), ExcInternalError());
    };
#endif

  
  return n_parameters_on_fine_grid;
}




template <int dim>
void
DoFTools::compute_intergrid_weights_2 (
  const DoFHandler<dim>              &coarse_grid,
  const unsigned int                  coarse_component,
  const InterGridMap<DoFHandler<dim> > &coarse_to_fine_grid_map,
  const std::vector<Vector<double> > &parameter_dofs,
  const std::vector<int>             &weight_mapping,
  std::vector<std::map<unsigned int,float> > &weights)
{
				   // simply distribute the range of
				   // cells to different threads
  typedef typename DoFHandler<dim>::active_cell_iterator active_cell_iterator;
  std::vector<std::pair<active_cell_iterator,active_cell_iterator> >
    cell_intervals = Threads::split_range<active_cell_iterator> (coarse_grid.begin_active(),
								 coarse_grid.end(),
								 multithread_info.n_default_threads);

  Threads::ThreadGroup<> threads;
  void (*fun_ptr) (const DoFHandler<dim>              &,
		   const unsigned int                  ,
		   const InterGridMap<DoFHandler<dim> > &,
		   const std::vector<Vector<double> > &,
		   const std::vector<int>             &,
		   std::vector<std::map<unsigned int, float> > &,
		   const typename DoFHandler<dim>::active_cell_iterator &,
		   const typename DoFHandler<dim>::active_cell_iterator &)
    = &DoFTools::template compute_intergrid_weights_3<dim>;
  for (unsigned int i=0; i<multithread_info.n_default_threads; ++i)
    threads += Threads::spawn (fun_ptr)(coarse_grid, coarse_component,
                                        coarse_to_fine_grid_map, parameter_dofs,
                                        weight_mapping, weights,
                                        cell_intervals[i].first,
                                        cell_intervals[i].second);

				   // wait for the threads to finish
  threads.join_all ();
}



template <int dim>
void
DoFTools::compute_intergrid_weights_3 (
  const DoFHandler<dim>              &coarse_grid,
  const unsigned int                  coarse_component,
  const InterGridMap<DoFHandler<dim> > &coarse_to_fine_grid_map,
  const std::vector<Vector<double> > &parameter_dofs,
  const std::vector<int>             &weight_mapping,
  std::vector<std::map<unsigned int, float> > &weights,
  const typename DoFHandler<dim>::active_cell_iterator &begin,
  const typename DoFHandler<dim>::active_cell_iterator &end)
{
				   // aliases to the finite elements
				   // used by the dof handlers:
  const FiniteElement<dim> &coarse_fe = coarse_grid.get_fe();    

				   // for each cell on the parameter grid:
				   // find out which degrees of freedom on the
				   // fine grid correspond in which way to
				   // the degrees of freedom on the parameter
				   // grid
				   //
				   // since for continuous FEs some
				   // dofs exist on more than one
				   // cell, we have to track which
				   // ones were already visited. the
				   // problem is that if we visit a
				   // dof first on one cell and
				   // compute its weight with respect
				   // to some global dofs to be
				   // non-zero, and later visit the
				   // dof again on another cell and
				   // (since we are on another cell)
				   // recompute the weights with
				   // respect to the same dofs as
				   // above to be zero now, we have to
				   // preserve them. we therefore
				   // overwrite all weights if they
				   // are nonzero and do not enforce
				   // zero weights since that might be
				   // only due to the fact that we are
				   // on another cell.
				   //
				   // example:
				   // coarse grid
				   //  |     |     |
				   //  *-----*-----*
				   //  | cell|cell |
				   //  |  1  |  2  |
				   //  |     |     |
				   //  0-----1-----*
				   //
				   // fine grid
				   //  |  |  |  |  |
				   //  *--*--*--*--*
				   //  |  |  |  |  |
				   //  *--*--*--*--*
				   //  |  |  |  |  |
				   //  *--x--y--*--*
				   //
				   // when on cell 1, we compute the
				   // weights of dof 'x' to be 1/2
				   // from parameter dofs 0 and 1,
				   // respectively. however, when
				   // later we are on cell 2, we again
				   // compute the prolongation of
				   // shape function 1 restricted to
				   // cell 2 to the globla grid and
				   // find that the weight of global
				   // dof 'x' now is zero. however, we
				   // should not overwrite the old
				   // value.
				   //
				   // we therefore always only set
				   // nonzero values. why adding up is
				   // not useful: dof 'y' would get
				   // weight 1 from parameter dof 1 on
				   // both cells 1 and 2, but the
				   // correct weight is nevertheless
				   // only 1.

				   // vector to hold the representation of
				   // a single degree of freedom on the
				   // coarse grid (for the selected fe)
				   // on the fine grid
  const unsigned int n_fine_dofs = weight_mapping.size();
  Vector<double> global_parameter_representation (n_fine_dofs);
  
  typename DoFHandler<dim>::active_cell_iterator cell;
  std::vector<unsigned int> parameter_dof_indices (coarse_fe.dofs_per_cell);
  
  for (cell=begin; cell!=end; ++cell)
    {
				       // get the global indices of the
				       // parameter dofs on this parameter
				       // grid cell
      cell->get_dof_indices (parameter_dof_indices);

				       // loop over all dofs on this
				       // cell and check whether they
				       // are interesting for us
      for (unsigned int local_dof=0;
	   local_dof<coarse_fe.dofs_per_cell;
	   ++local_dof)
	if (coarse_fe.system_to_component_index(local_dof).first
	    ==
	    coarse_component)
	  {
					     // the how-many-th
					     // parameter is this on
					     // this cell?
	    const unsigned int local_parameter_dof
	      = coarse_fe.system_to_component_index(local_dof).second;
	    
	    global_parameter_representation = 0;
	    
					     // distribute the representation of
					     // @p{local_parameter_dof} on the
					     // parameter grid cell @p{cell} to
					     // the global data space
	    coarse_to_fine_grid_map[cell]->
	      set_dof_values_by_interpolation (parameter_dofs[local_parameter_dof],
					       global_parameter_representation);
					     // now that we've got the global
					     // representation of each parameter
					     // dof, we've only got to clobber the
					     // non-zero entries in that vector and
					     // store the result
					     //
					     // what we have learned: if entry @p{i}
					     // of the global vector holds the value
					     // @p{v[i]}, then this is the weight with
					     // which the present dof contributes
					     // to @p{i}. there may be several such
					     // @p{i}s and their weights' sum should
					     // be one. Then, @p{v[i]} should be
					     // equal to @p{\sum_j w_{ij} p[j]} with
					     // @p{p[j]} be the values of the degrees
					     // of freedom on the coarse grid. we
					     // can thus compute constraints which
					     // link the degrees of freedom @p{v[i]}
					     // on the fine grid to those on the
					     // coarse grid, @p{p[j]}. Now to use
					     // these as real constraints, rather
					     // than as additional equations, we
					     // have to identify representants
					     // among the @p{i} for each @p{j}. this will
					     // be done by simply taking the first
					     // @p{i} for which @p{w_{ij}==1}.
					     //
					     // guard modification of
					     // the weights array by a
					     // Mutex. since it should
					     // happen rather rarely
					     // that there are several
					     // threads operating on
					     // different intergrid
					     // weights, have only one
					     // mutex for all of them
	    static Threads::ThreadMutex mutex;
            Threads::ThreadMutex::ScopedLock lock (mutex);
	    for (unsigned int i=0; i<global_parameter_representation.size(); ++i)
					       // set this weight if it belongs
					       // to a parameter dof.
	      if (weight_mapping[i] != -1)
		{
						   // only overwrite old
						   // value if not by
						   // zero
		  if (global_parameter_representation(i) != 0)
		    {
		      const unsigned int wi = parameter_dof_indices[local_dof],
					 wj = weight_mapping[i];
		      weights[wi][wj] = global_parameter_representation(i);
		    };
		}
	      else
		Assert (global_parameter_representation(i) == 0,
			ExcInternalError());
	  };
    };
}




#if deal_II_dimension == 1


template <class DH>
void DoFTools::map_dof_to_boundary_indices (
  const DH                      &dof_handler,
  const std::set<unsigned char> &boundary_indicators,
  std::vector<unsigned int> &mapping)
{
  Assert (&dof_handler.get_fe() != 0, ExcNoFESelected());

  mapping.clear ();
  mapping.insert (mapping.end(), dof_handler.n_dofs(),
		  DH::invalid_dof_index);

  unsigned int next_free_index = 0;
  
				   // first check left, the right
				   // boundary point
  for (unsigned int direction=0; direction<2; ++direction)
    {
				       // if this boundary is not
				       // requested, then go on with next one
      if (boundary_indicators.find(direction) ==
	  boundary_indicators.end())
	continue;

				       // find active cell at that
				       // boundary: first go to
				       // left/right, then to children
      typename DH::cell_iterator cell = dof_handler.begin(0);
      while (!cell->at_boundary(direction))
	cell = cell->neighbor(direction);
      while (!cell->active())
	cell = cell->child(direction);

				       // next enumerate these degrees
				       // of freedom
      for (unsigned int i=0; i<cell->get_fe().dofs_per_vertex; ++i)
	mapping[cell->vertex_dof_index(direction,i)] = next_free_index++;
    };
}



template <>
void
DoFTools::map_dof_to_boundary_indices (const DoFHandler<1>       &dof_handler,
                                       std::vector<unsigned int> &mapping)
{
  Assert (&dof_handler.get_fe() != 0, ExcNoFESelected());

				   // in 1d, there are only 2 boundary
				   // indicators, so enumerate them
				   // and pass on to the other
				   // function
  std::set<unsigned char> boundary_indicators;
  boundary_indicators.insert (0U);
  boundary_indicators.insert (1U);

  map_dof_to_boundary_indices<DoFHandler<1> > (dof_handler, boundary_indicators, mapping);
}

#else


template <class DH>
void
DoFTools::map_dof_to_boundary_indices (const DH                  &dof_handler,
                                       std::vector<unsigned int> &mapping)
{
  Assert (&dof_handler.get_fe() != 0, ExcNoFESelected());

  mapping.clear ();
  mapping.insert (mapping.end(), dof_handler.n_dofs(),
		  DH::invalid_dof_index);
  
  std::vector<unsigned int> dofs_on_face;
  dofs_on_face.reserve (max_dofs_per_face(dof_handler));
  unsigned int next_boundary_index = 0;
  
				   // now loop over all cells and
				   // check whether their faces are at
				   // the boundary. note that we need
				   // not take special care of single
				   // lines being at the boundary
				   // (using
				   // @p{cell->has_boundary_lines}),
				   // since we do not support
				   // boundaries of dimension dim-2,
				   // and so every isolated boundary
				   // line is also part of a boundary
				   // face which we will be visiting
				   // sooner or later
  typename DH::active_face_iterator face = dof_handler.begin_active_face(),
				    endf = dof_handler.end_face();
  for (; face!=endf; ++face)
    if (face->at_boundary()) 
      {
        const unsigned int dofs_per_face = face->get_fe().dofs_per_face;
        dofs_on_face.resize (dofs_per_face);
	face->get_dof_indices (dofs_on_face);
	for (unsigned int i=0; i<dofs_per_face; ++i)
	  if (mapping[dofs_on_face[i]] == DH::invalid_dof_index)
	    mapping[dofs_on_face[i]] = next_boundary_index++;
      };

  Assert (next_boundary_index == dof_handler.n_boundary_dofs(),
	  ExcInternalError());
}



template <class DH>
void DoFTools::map_dof_to_boundary_indices (
  const DH                      &dof_handler,
  const std::set<unsigned char> &boundary_indicators,
  std::vector<unsigned int>     &mapping)
{
  Assert (&dof_handler.get_fe() != 0, ExcNoFESelected());
  Assert (boundary_indicators.find (255) == boundary_indicators.end(),
	  ExcInvalidBoundaryIndicator());

  mapping.clear ();
  mapping.insert (mapping.end(), dof_handler.n_dofs(),
		  DH::invalid_dof_index);

				   // return if there is nothing to do
  if (boundary_indicators.size() == 0)
    return;
  
  std::vector<unsigned int> dofs_on_face;
  dofs_on_face.reserve (max_dofs_per_face(dof_handler));
  unsigned int next_boundary_index = 0;
  
  typename DH::active_face_iterator face = dof_handler.begin_active_face(),
				    endf = dof_handler.end_face();
  for (; face!=endf; ++face)
    if (boundary_indicators.find (face->boundary_indicator()) !=
	boundary_indicators.end())
      {
        const unsigned int dofs_per_face = face->get_fe().dofs_per_face;
        dofs_on_face.resize (dofs_per_face);
	face->get_dof_indices (dofs_on_face);
	for (unsigned int i=0; i<dofs_per_face; ++i)
	  if (mapping[dofs_on_face[i]] == DH::invalid_dof_index)
	    mapping[dofs_on_face[i]] = next_boundary_index++;
      };

  Assert (next_boundary_index == dof_handler.n_boundary_dofs (boundary_indicators),
	  ExcInternalError());
}

#endif



template <int dim>
void
DoFTools::map_dofs_to_support_points (const Mapping<dim>       &mapping,
				      const DoFHandler<dim>    &dof_handler,
				      std::vector<Point<dim> > &support_points)
{
  const unsigned int dofs_per_cell = dof_handler.get_fe().dofs_per_cell;
  
				   // check whether fe has support
				   // points
  Assert (dof_handler.get_fe().has_support_points(),
	  ExcFEHasNoSupportPoints());
  Assert (support_points.size() == dof_handler.n_dofs(),
	  ExcWrongSize (support_points.size(), dof_handler.n_dofs()));

				   // now loop over all cells and
				   // enquire the support points on
				   // each of these. use a dummy
				   // quadrature formula where the
				   // quadrature points are located at
				   // the unit support points to
				   // enquire the location of the
				   // support points in real space
				   //
				   // the weights of the quadrature
				   // rule are set to invalid values
				   // by the used constructor.
  Quadrature<dim> q_dummy(dof_handler.get_fe().get_unit_support_points());
  FEValues<dim> fe_values (mapping, dof_handler.get_fe(),
			   q_dummy, update_q_points);
  typename DoFHandler<dim>::active_cell_iterator
    cell = dof_handler.begin_active(),
    endc = dof_handler.end();

  std::vector<unsigned int> local_dof_indices (dofs_per_cell);
  for (; cell!=endc; ++cell)
    {
      fe_values.reinit (cell);
      cell->get_dof_indices (local_dof_indices);
      const std::vector<Point<dim> > & points
  	= fe_values.get_quadrature_points ();
      for (unsigned int i=0; i<dofs_per_cell; ++i)
	support_points[local_dof_indices[i]] = points[i];
    };
}


template <int dim>
void
DoFTools::compute_dof_couplings(
  Table<2,Coupling>&        dof_couplings,
  const Table<2,Coupling>&  component_couplings,
  const FiniteElement<dim>& fe)
{
  Assert(component_couplings.n_rows() == fe.n_components(),
	 ExcDimensionMismatch(component_couplings.n_rows(),
			      fe.n_components()));
  Assert(component_couplings.n_cols() == fe.n_components(),
	 ExcDimensionMismatch(component_couplings.n_cols(),
			      fe.n_components()));
  
  const unsigned int n_dofs = fe.dofs_per_cell;

  Assert(dof_couplings.n_rows() == n_dofs,
	 ExcDimensionMismatch(dof_couplings.n_rows(), n_dofs));
  Assert(dof_couplings.n_cols() == n_dofs,
	 ExcDimensionMismatch(dof_couplings.n_cols(), n_dofs));

  for (unsigned int i=0; i<n_dofs; ++i)
    {
      const unsigned int ii
        = (fe.is_primitive(i) ?
           fe.system_to_component_index(i).first
           :
           (std::find (fe.get_nonzero_components(i).begin(),
                       fe.get_nonzero_components(i).end(),
                       true)
            -
           fe.get_nonzero_components(i).begin())
           );
      Assert (ii < fe.n_components(),
              ExcInternalError());

      for (unsigned int j=0; j<n_dofs; ++j)
	{
          const unsigned int jj
            = (fe.is_primitive(j) ?
               fe.system_to_component_index(j).first
               :
               (std::find (fe.get_nonzero_components(j).begin(),
                           fe.get_nonzero_components(j).end(),
                           true)
                -
                fe.get_nonzero_components(j).begin())
               );
          Assert (jj < fe.n_components(),
                  ExcInternalError());          

	  dof_couplings(i,j) = component_couplings(ii,jj);
	}
    }
}


template<int dim>
void
DoFTools::convert_couplings_to_blocks (
  const DoFHandler<dim>& dof_handler,
  const Table<2, Coupling>& table,
  std::vector<Table<2,Coupling> >& tables_by_block)
{
  const FiniteElement<dim>& fe = dof_handler.get_fe();
  const unsigned int nb = fe.n_blocks();
  
  tables_by_block.resize(1);
  tables_by_block[0].reinit(nb, nb);
  tables_by_block[0].fill(none);
  
  for (unsigned int i=0;i<fe.n_components();++i)
    {
      const unsigned int ib = fe.component_to_block_index(i);
      for (unsigned int j=0;j<fe.n_components();++j)
	{
	  const unsigned int jb = fe.component_to_block_index(j);
	  tables_by_block[0](ib,jb) |= table(i,j);
	}
    }
}


template<int dim>
void
DoFTools::convert_couplings_to_blocks (
  const hp::DoFHandler<dim>& dof_handler,
  const Table<2, Coupling>& table,
  std::vector<Table<2,Coupling> >& tables_by_block)
{
  const hp::FECollection<dim>& fe_collection = dof_handler.get_fe();
  tables_by_block.resize(fe_collection.size());

  for (unsigned int f=0;f<fe_collection.size();++f)
    {
      const FiniteElement<dim>& fe = fe_collection[f];
      
      const unsigned int nb = fe.n_blocks();
      tables_by_block[f].reinit(nb, nb);
      tables_by_block[f].fill(none);
      for (unsigned int i=0;i<fe.n_components();++i)
	{
	  const unsigned int ib = fe.component_to_block_index(i);
	  for (unsigned int j=0;j<fe.n_components();++j)
	    {
	      const unsigned int jb = fe.component_to_block_index(j);
	      tables_by_block[f](ib,jb) |= table(i,j);
	    }
	}
    }
}


// explicit instantiations
template void
DoFTools::compute_row_length_vector(
  const DoFHandler<deal_II_dimension>& dofs, std::vector<unsigned int>& row_lengths,
  const Coupling flux_coupling);

template void
DoFTools::compute_row_length_vector(
  const hp::DoFHandler<deal_II_dimension>& dofs, std::vector<unsigned int>& row_lengths,
  const Coupling flux_coupling);

template void
DoFTools::compute_row_length_vector(
  const DoFHandler<deal_II_dimension>& dofs, std::vector<unsigned int>& row_lengths,
  const Table<2,Coupling>& couplings, const Table<2,Coupling>& flux_couplings);

template void
DoFTools::compute_row_length_vector(
  const hp::DoFHandler<deal_II_dimension>& dofs, std::vector<unsigned int>& row_lengths,
  const Table<2,Coupling>& couplings, const Table<2,Coupling>& flux_couplings);

template void
DoFTools::compute_row_length_vector(
  const DoFHandler<deal_II_dimension>&,
  std::vector<std::vector<unsigned int> >&,
  const Table<2,Coupling>& couplings, const Table<2,Coupling>& flux_couplings);

// template void
// DoFTools::compute_row_length_vector(
//   const hp::DoFHandler<deal_II_dimension>&,
//   std::vector<std::vector<unsigned int> >&,
//   const Table<2,Coupling>& couplings, const Table<2,Coupling>& flux_couplings);

template void
DoFTools::make_sparsity_pattern<DoFHandler<deal_II_dimension>,SparsityPattern>
(const DoFHandler<deal_II_dimension> &dof,
 SparsityPattern    &sparsity);
template void
DoFTools::make_sparsity_pattern<DoFHandler<deal_II_dimension>,CompressedSparsityPattern>
(const DoFHandler<deal_II_dimension> &dof,
 CompressedSparsityPattern    &sparsity);
template void
DoFTools::make_sparsity_pattern<DoFHandler<deal_II_dimension>,BlockSparsityPattern>
(const DoFHandler<deal_II_dimension> &dof,
 BlockSparsityPattern                &sparsity);
template void
DoFTools::make_sparsity_pattern<DoFHandler<deal_II_dimension>,CompressedBlockSparsityPattern>
(const DoFHandler<deal_II_dimension> &dof,
 CompressedBlockSparsityPattern      &sparsity);

template void
DoFTools::make_sparsity_pattern<hp::DoFHandler<deal_II_dimension>,SparsityPattern>
(const hp::DoFHandler<deal_II_dimension> &dof,
 SparsityPattern    &sparsity);
template void
DoFTools::make_sparsity_pattern<hp::DoFHandler<deal_II_dimension>,CompressedSparsityPattern>
(const hp::DoFHandler<deal_II_dimension> &dof,
 CompressedSparsityPattern    &sparsity);
template void
DoFTools::make_sparsity_pattern<hp::DoFHandler<deal_II_dimension>,BlockSparsityPattern>
(const hp::DoFHandler<deal_II_dimension> &dof,
 BlockSparsityPattern                &sparsity);
template void
DoFTools::make_sparsity_pattern<hp::DoFHandler<deal_II_dimension>,CompressedBlockSparsityPattern>
(const hp::DoFHandler<deal_II_dimension> &dof,
 CompressedBlockSparsityPattern      &sparsity);


template void 
DoFTools::make_sparsity_pattern<DoFHandler<deal_II_dimension>,SparsityPattern>
(const DoFHandler<deal_II_dimension>&, const Table<2,Coupling>&, SparsityPattern&);
template void 
DoFTools::make_sparsity_pattern<DoFHandler<deal_II_dimension>,CompressedSparsityPattern>
(const DoFHandler<deal_II_dimension>&, const Table<2,Coupling>&, CompressedSparsityPattern&);
template void 
DoFTools::make_sparsity_pattern<DoFHandler<deal_II_dimension>,BlockSparsityPattern>
(const DoFHandler<deal_II_dimension>&, const Table<2,Coupling>&, BlockSparsityPattern&);
template void 
DoFTools::make_sparsity_pattern<DoFHandler<deal_II_dimension>,CompressedBlockSparsityPattern>
(const DoFHandler<deal_II_dimension>&, const Table<2,Coupling>&, CompressedBlockSparsityPattern&);

// #if deal_II_dimension > 1
template void
DoFTools::make_boundary_sparsity_pattern<DoFHandler<deal_II_dimension>,SparsityPattern>
(const DoFHandler<deal_II_dimension>& dof,
 const std::vector<unsigned int>  &,
 SparsityPattern    &);
template void
DoFTools::make_boundary_sparsity_pattern<DoFHandler<deal_II_dimension>,CompressedSparsityPattern>
(const DoFHandler<deal_II_dimension>& dof,
 const std::vector<unsigned int>  &,
 CompressedSparsityPattern    &);
template void
DoFTools::make_boundary_sparsity_pattern<DoFHandler<deal_II_dimension>,BlockSparsityPattern>
(const DoFHandler<deal_II_dimension>& dof,
 const std::vector<unsigned int>  &,
 BlockSparsityPattern    &);
template void
DoFTools::make_boundary_sparsity_pattern<DoFHandler<deal_II_dimension>,CompressedBlockSparsityPattern>
(const DoFHandler<deal_II_dimension>& dof,
 const std::vector<unsigned int>  &,
 CompressedBlockSparsityPattern    &);

template void
DoFTools::make_boundary_sparsity_pattern<hp::DoFHandler<deal_II_dimension>,SparsityPattern>
(const hp::DoFHandler<deal_II_dimension>& dof,
 const std::vector<unsigned int>  &,
 SparsityPattern    &);
template void
DoFTools::make_boundary_sparsity_pattern<hp::DoFHandler<deal_II_dimension>,CompressedSparsityPattern>
(const hp::DoFHandler<deal_II_dimension>& dof,
 const std::vector<unsigned int>  &,
 CompressedSparsityPattern    &);
template void
DoFTools::make_boundary_sparsity_pattern<hp::DoFHandler<deal_II_dimension>,BlockSparsityPattern>
(const hp::DoFHandler<deal_II_dimension>& dof,
 const std::vector<unsigned int>  &,
 BlockSparsityPattern    &);
template void
DoFTools::make_boundary_sparsity_pattern<hp::DoFHandler<deal_II_dimension>,CompressedBlockSparsityPattern>
(const hp::DoFHandler<deal_II_dimension>& dof,
 const std::vector<unsigned int>  &,
 CompressedBlockSparsityPattern    &);


template void
DoFTools::make_boundary_sparsity_pattern<DoFHandler<deal_II_dimension>,SparsityPattern>
(const DoFHandler<deal_II_dimension>& dof,
 const FunctionMap<deal_II_dimension>::type  &boundary_indicators,
 const std::vector<unsigned int>  &dof_to_boundary_mapping,
 SparsityPattern    &sparsity);
template void
DoFTools::make_boundary_sparsity_pattern<DoFHandler<deal_II_dimension>,CompressedSparsityPattern>
(const DoFHandler<deal_II_dimension>& dof,
 const FunctionMap<deal_II_dimension>::type  &boundary_indicators,
 const std::vector<unsigned int>  &dof_to_boundary_mapping,
 CompressedSparsityPattern    &sparsity);
template void
DoFTools::make_boundary_sparsity_pattern<DoFHandler<deal_II_dimension>,BlockSparsityPattern>
(const DoFHandler<deal_II_dimension>& dof,
 const FunctionMap<deal_II_dimension>::type  &boundary_indicators,
 const std::vector<unsigned int>  &dof_to_boundary_mapping,
 BlockSparsityPattern    &sparsity);
template void
DoFTools::make_boundary_sparsity_pattern<DoFHandler<deal_II_dimension>,CompressedBlockSparsityPattern>
(const DoFHandler<deal_II_dimension>& dof,
 const FunctionMap<deal_II_dimension>::type  &boundary_indicators,
 const std::vector<unsigned int>  &dof_to_boundary_mapping,
 CompressedBlockSparsityPattern    &sparsity);

template void
DoFTools::make_boundary_sparsity_pattern<hp::DoFHandler<deal_II_dimension>,SparsityPattern>
(const hp::DoFHandler<deal_II_dimension>& dof,
 const FunctionMap<deal_II_dimension>::type  &boundary_indicators,
 const std::vector<unsigned int>  &dof_to_boundary_mapping,
 SparsityPattern    &sparsity);
template void
DoFTools::make_boundary_sparsity_pattern<hp::DoFHandler<deal_II_dimension>,CompressedSparsityPattern>
(const hp::DoFHandler<deal_II_dimension>& dof,
 const FunctionMap<deal_II_dimension>::type  &boundary_indicators,
 const std::vector<unsigned int>  &dof_to_boundary_mapping,
 CompressedSparsityPattern    &sparsity);
template void
DoFTools::make_boundary_sparsity_pattern<hp::DoFHandler<deal_II_dimension>,BlockSparsityPattern>
(const hp::DoFHandler<deal_II_dimension>& dof,
 const FunctionMap<deal_II_dimension>::type  &boundary_indicators,
 const std::vector<unsigned int>  &dof_to_boundary_mapping,
 BlockSparsityPattern    &sparsity);
template void
DoFTools::make_boundary_sparsity_pattern<hp::DoFHandler<deal_II_dimension>,CompressedBlockSparsityPattern>
(const hp::DoFHandler<deal_II_dimension>& dof,
 const FunctionMap<deal_II_dimension>::type  &boundary_indicators,
 const std::vector<unsigned int>  &dof_to_boundary_mapping,
 CompressedBlockSparsityPattern    &sparsity);


template void
DoFTools::make_flux_sparsity_pattern<DoFHandler<deal_II_dimension>,SparsityPattern>
(const DoFHandler<deal_II_dimension> &dof,
 SparsityPattern    &sparsity);
template void
DoFTools::make_flux_sparsity_pattern<DoFHandler<deal_II_dimension>,CompressedSparsityPattern>
(const DoFHandler<deal_II_dimension> &dof,
 CompressedSparsityPattern    &sparsity);
template void
DoFTools::make_flux_sparsity_pattern<DoFHandler<deal_II_dimension>,BlockSparsityPattern>
(const DoFHandler<deal_II_dimension> &dof,
 BlockSparsityPattern    &sparsity);
template void
DoFTools::make_flux_sparsity_pattern<DoFHandler<deal_II_dimension>,CompressedBlockSparsityPattern>
(const DoFHandler<deal_II_dimension> &dof,
 CompressedBlockSparsityPattern    &sparsity);

template void
DoFTools::make_flux_sparsity_pattern<hp::DoFHandler<deal_II_dimension>,SparsityPattern>
(const hp::DoFHandler<deal_II_dimension> &dof,
 SparsityPattern    &sparsity);
template void
DoFTools::make_flux_sparsity_pattern<hp::DoFHandler<deal_II_dimension>,CompressedSparsityPattern>
(const hp::DoFHandler<deal_II_dimension> &dof,
 CompressedSparsityPattern    &sparsity);
template void
DoFTools::make_flux_sparsity_pattern<hp::DoFHandler<deal_II_dimension>,BlockSparsityPattern>
(const hp::DoFHandler<deal_II_dimension> &dof,
 BlockSparsityPattern    &sparsity);
template void
DoFTools::make_flux_sparsity_pattern<hp::DoFHandler<deal_II_dimension>,CompressedBlockSparsityPattern>
(const hp::DoFHandler<deal_II_dimension> &dof,
 CompressedBlockSparsityPattern    &sparsity);


#if deal_II_dimension > 1
template void
DoFTools::make_flux_sparsity_pattern<DoFHandler<deal_II_dimension>,SparsityPattern>
(const DoFHandler<deal_II_dimension> &dof,
 SparsityPattern    &,
 const Table<2,Coupling>&,
 const Table<2,Coupling>&);
template void
DoFTools::make_flux_sparsity_pattern<DoFHandler<deal_II_dimension>,CompressedSparsityPattern>
(const DoFHandler<deal_II_dimension> &dof,
 CompressedSparsityPattern    &,
 const Table<2,Coupling>&,
 const Table<2,Coupling>&);
template void
DoFTools::make_flux_sparsity_pattern<DoFHandler<deal_II_dimension>,BlockSparsityPattern>
(const DoFHandler<deal_II_dimension> &dof,
 BlockSparsityPattern    &,
 const Table<2,Coupling>&,
 const Table<2,Coupling>&);
template void
DoFTools::make_flux_sparsity_pattern<DoFHandler<deal_II_dimension>,CompressedBlockSparsityPattern>
(const DoFHandler<deal_II_dimension> &dof,
 CompressedBlockSparsityPattern    &,
 const Table<2,Coupling>&,
 const Table<2,Coupling>&);
#endif



template
void
DoFTools::distribute_cell_to_dof_vector<DoFHandler<deal_II_dimension> >
(const DoFHandler<deal_II_dimension> &dof_handler,
 const Vector<float> &cell_data,
 Vector<double>      &dof_data,
 const unsigned int   component);
template
void
DoFTools::distribute_cell_to_dof_vector<DoFHandler<deal_II_dimension> >
(const DoFHandler<deal_II_dimension> &dof_handler,
 const Vector<double> &cell_data,
 Vector<double>       &dof_data,
 const unsigned int    component);

template
void
DoFTools::distribute_cell_to_dof_vector<hp::DoFHandler<deal_II_dimension> >
(const hp::DoFHandler<deal_II_dimension> &dof_handler,
 const Vector<float> &cell_data,
 Vector<double>      &dof_data,
 const unsigned int   component);
template
void
DoFTools::distribute_cell_to_dof_vector<hp::DoFHandler<deal_II_dimension> >
(const hp::DoFHandler<deal_II_dimension> &dof_handler,
 const Vector<double> &cell_data,
 Vector<double>       &dof_data,
 const unsigned int    component);


template void DoFTools::extract_dofs<deal_II_dimension>
(const DoFHandler<deal_II_dimension>&,
 const std::vector<bool>&, std::vector<bool>&, bool);

template void DoFTools::extract_level_dofs<deal_II_dimension>
(const unsigned int level, const MGDoFHandler<deal_II_dimension>&,
 const std::vector<bool>&, std::vector<bool>&, bool);

template
void
DoFTools::extract_boundary_dofs<DoFHandler<deal_II_dimension> >
(const DoFHandler<deal_II_dimension> &,
 const std::vector<bool>                  &,
 std::vector<bool>                        &,
 const std::set<unsigned char> &);
template
void
DoFTools::extract_boundary_dofs<hp::DoFHandler<deal_II_dimension> >
(const hp::DoFHandler<deal_II_dimension> &,
 const std::vector<bool>                  &,
 std::vector<bool>                        &,
 const std::set<unsigned char> &);

template
void
DoFTools::extract_subdomain_dofs<DoFHandler<deal_II_dimension> >
(const DoFHandler<deal_II_dimension> &dof_handler,
 const unsigned int     subdomain_id,
 std::vector<bool>     &selected_dofs);
template
void
DoFTools::extract_subdomain_dofs<hp::DoFHandler<deal_II_dimension> >
(const hp::DoFHandler<deal_II_dimension> &dof_handler,
 const unsigned int     subdomain_id,
 std::vector<bool>     &selected_dofs);

template
void
DoFTools::get_subdomain_association<DoFHandler<deal_II_dimension> >
(const DoFHandler<deal_II_dimension> &dof_handler,
 std::vector<unsigned int>           &subdomain_association);
template
void
DoFTools::get_subdomain_association<hp::DoFHandler<deal_II_dimension> >
(const hp::DoFHandler<deal_II_dimension> &dof_handler,
 std::vector<unsigned int>           &subdomain_association);


template
unsigned int
DoFTools::count_dofs_with_subdomain_association<DoFHandler<deal_II_dimension> >
(const DoFHandler<deal_II_dimension> &,
 const unsigned int);
template
unsigned int
DoFTools::count_dofs_with_subdomain_association<hp::DoFHandler<deal_II_dimension> >
(const hp::DoFHandler<deal_II_dimension> &,
 const unsigned int);
template
unsigned int
DoFTools::count_dofs_with_subdomain_association<MGDoFHandler<deal_II_dimension> >
(const MGDoFHandler<deal_II_dimension> &,
 const unsigned int);


template
void
DoFTools::count_dofs_per_component<deal_II_dimension> (
  const DoFHandler<deal_II_dimension>&,
  std::vector<unsigned int>&, bool, std::vector<unsigned int>);

template
void
DoFTools::count_dofs_per_block<deal_II_dimension> (
  const DoFHandler<deal_II_dimension>&,
  std::vector<unsigned int>&, std::vector<unsigned int>);

template
void
DoFTools::count_dofs_per_component<deal_II_dimension> (
  const DoFHandler<deal_II_dimension>&,
  std::vector<unsigned int>&, std::vector<unsigned int>);

template
void
DoFTools::compute_intergrid_constraints<deal_II_dimension> (
  const DoFHandler<deal_II_dimension> &, const unsigned int,
  const DoFHandler<deal_II_dimension> &, const unsigned int,
  const InterGridMap<DoFHandler<deal_II_dimension> > &,
  ConstraintMatrix&);

template
void
DoFTools::compute_intergrid_transfer_representation<deal_II_dimension>
(const DoFHandler<deal_II_dimension> &, const unsigned int,
 const DoFHandler<deal_II_dimension> &, const unsigned int,
 const InterGridMap<DoFHandler<deal_II_dimension> > &,
 std::vector<std::map<unsigned int, float> > &);


template
void
DoFTools::map_dof_to_boundary_indices<DoFHandler<deal_II_dimension> >
(const DoFHandler<deal_II_dimension> &,
 const std::set<unsigned char> &,
 std::vector<unsigned int> &);

#if deal_II_dimension != 1

template
void
DoFTools::map_dof_to_boundary_indices<DoFHandler<deal_II_dimension> >
(const DoFHandler<deal_II_dimension> &,
 std::vector<unsigned int> &);

#endif


template
void
DoFTools::map_dofs_to_support_points<deal_II_dimension>
(const Mapping<deal_II_dimension>&,
 const DoFHandler<deal_II_dimension>&,
 std::vector<Point<deal_II_dimension> >&);

template
void
DoFTools::compute_dof_couplings<deal_II_dimension>(
  Table<2,Coupling>&, const Table<2,Coupling>&,
  const FiniteElement<deal_II_dimension>&);

template
void
DoFTools::convert_couplings_to_blocks (
  const DoFHandler<deal_II_dimension>&, const Table<2, Coupling>&,
  std::vector<Table<2,Coupling> >&);

template
void
DoFTools::convert_couplings_to_blocks (
  const hp::DoFHandler<deal_II_dimension>&, const Table<2, Coupling>&,
  std::vector<Table<2,Coupling> >&);

