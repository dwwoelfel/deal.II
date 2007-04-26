//---------------------------------------------------------------------------
//    mg_transfer.templates.h,v 1.22 2006/01/29 15:03:55 guido Exp
//    Version: 
//
//    Copyright (C) 2003, 2004, 2005, 2006, 2007 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------

#ifndef __deal2__mg_transfer_templates_h
#define __deal2__mg_transfer_templates_h

#include <lac/sparse_matrix.h>
#include <grid/tria_iterator.h>
#include <fe/fe.h>
#include <dofs/dof_constraints.h>
#include <multigrid/mg_base.h>
#include <multigrid/mg_dof_accessor.h>
#include <multigrid/mg_tools.h>
#include <multigrid/mg_transfer.h>

#include <algorithm>

DEAL_II_NAMESPACE_OPEN

/* --------------------- MGTransferPrebuilt -------------- */



template <class VECTOR>
template <int dim, class InVector>
void
MGTransferPrebuilt<VECTOR>::copy_to_mg (
  const MGDoFHandler<dim>        &mg_dof_handler,
  MGLevelObject<VECTOR> &dst,
  const InVector                 &src) const
{
  const FiniteElement<dim>& fe = mg_dof_handler.get_fe();
  
  const unsigned int dofs_per_cell = fe.dofs_per_cell;
  
				   // set the elements of the vectors
				   // on all levels to zero
  unsigned int minlevel = dst.get_minlevel();
  unsigned int maxlevel = dst.get_maxlevel();

  MGTools::reinit_vector(mg_dof_handler, dst);

  Assert(sizes.size()==mg_dof_handler.get_tria().n_levels(),
	 ExcMatricesNotBuilt());
  
  std::vector<unsigned int> global_dof_indices (dofs_per_cell);
  std::vector<unsigned int> level_dof_indices  (dofs_per_cell);
  
				   // traverse the grid top-down
				   // (i.e. starting with the most
				   // refined grid). this way, we can
				   // always get that part of one
				   // level of the output vector which
				   // corresponds to a region which is
				   // more refined, by restriction of
				   // the respective vector on the
				   // next finer level, which we then
				   // already have built.
  for (int level=maxlevel; level>=static_cast<signed int>(minlevel); --level)
    {
      typename MGDoFHandler<dim>::active_cell_iterator
	level_cell = mg_dof_handler.begin_active(level);
      const typename MGDoFHandler<dim>::active_cell_iterator
	level_end  = mg_dof_handler.end_active(level);

				       // Compute coarse level right hand side
				       // by restricting from fine level.
      for (; level_cell!=level_end; ++level_cell)
	{
					   // get the dof numbers of
					   // this cell for the global
					   // and the level-wise
					   // numbering
	  level_cell->get_dof_indices(global_dof_indices);
	  level_cell->get_mg_dof_indices (level_dof_indices);

					   // transfer the global
					   // defect in the vector
					   // into the level-wise one
	  for (unsigned int i=0; i<dofs_per_cell; ++i)
	    dst[level](level_dof_indices[i]) = src(global_dof_indices[i]);
	}
				       // for that part of the level
				       // which is further refined:
				       // get the defect by
				       // restriction of the defect on
				       // one level higher
      if (static_cast<unsigned int>(level) < maxlevel)
	{
	  restrict_and_add (level+1, dst[level], dst[level+1]);
	}
    }
}



template <class VECTOR>
template <int dim, class OutVector>
void
MGTransferPrebuilt<VECTOR>::copy_from_mg(
  const MGDoFHandler<dim>&       mg_dof_handler,
  OutVector&                     dst,
  const MGLevelObject<VECTOR>& src) const
{
  const FiniteElement<dim>& fe = mg_dof_handler.get_fe();
  
  const unsigned int dofs_per_cell = fe.dofs_per_cell;
  std::vector<unsigned int> global_dof_indices (dofs_per_cell);
  std::vector<unsigned int> level_dof_indices (dofs_per_cell);

  typename MGDoFHandler<dim>::active_cell_iterator
    level_cell = mg_dof_handler.begin_active();
  const typename MGDoFHandler<dim>::active_cell_iterator
    endc = mg_dof_handler.end();

				   // traverse all cells and copy the
				   // data appropriately to the output
				   // vector

				   // Note that the level is
				   // monotonuosly increasing
  for (; level_cell != endc; ++level_cell)
    {
      const unsigned int level = level_cell->level();
      
				       // get the dof numbers of
				       // this cell for the global
				       // and the level-wise
				       // numbering
      level_cell->get_dof_indices (global_dof_indices);
      level_cell->get_mg_dof_indices(level_dof_indices);

				       // copy level-wise data to
				       // global vector
      for (unsigned int i=0; i<dofs_per_cell; ++i)
	dst(global_dof_indices[i]) = src[level](level_dof_indices[i]);
    }
}



template <class VECTOR>
template <int dim, class OutVector>
void
MGTransferPrebuilt<VECTOR>::copy_from_mg_add (
  const MGDoFHandler<dim>              &mg_dof_handler,
  OutVector                            &dst,
  const MGLevelObject<VECTOR> &src) const
{
  const FiniteElement<dim>& fe = mg_dof_handler.get_fe();
  
  const unsigned int dofs_per_cell = fe.dofs_per_cell;
  std::vector<unsigned int> global_dof_indices (dofs_per_cell);
  std::vector<unsigned int> level_dof_indices (dofs_per_cell);

  typename MGDoFHandler<dim>::active_cell_iterator
    level_cell = mg_dof_handler.begin_active();
  const typename MGDoFHandler<dim>::active_cell_iterator
    endc = mg_dof_handler.end();

				   // traverse all cells and copy the
				   // data appropriately to the output
				   // vector

				   // Note that the level is
				   // monotonuosly increasing
  for (; level_cell != endc; ++level_cell)
    {
      const unsigned int level = level_cell->level();
      
				       // get the dof numbers of
				       // this cell for the global
				       // and the level-wise
				       // numbering
      level_cell->get_dof_indices (global_dof_indices);
      level_cell->get_mg_dof_indices(level_dof_indices);
				       // copy level-wise data to
				       // global vector
      for (unsigned int i=0; i<dofs_per_cell; ++i)
	dst(global_dof_indices[i]) += src[level](level_dof_indices[i]);
    }
}


template <class VECTOR>
unsigned int
MGTransferPrebuilt<VECTOR>::memory_consumption () const
{
  unsigned int result = sizeof(*this);
  result += sizeof(unsigned int) * sizes.size();
#ifdef DEAL_PREFER_MATRIX_EZ
  std::vector<boost::shared_ptr<SparseMatrixEZ<double> > >::const_iterator m;
  const std::vector<boost::shared_ptr<SparseMatrixEZ<double> > >::const_iterator end = prolongation_matrices.end();
  for (m = prolongation_matrices.begin(); m != end ; ++m)
    result += *m->memory_consumption();
#else
  for (unsigned int i=0;i<prolongation_matrices.size();++i)
    result += prolongation_matrices[i]->memory_consumption()
	      + prolongation_sparsities[i]->memory_consumption();
#endif
  return result;
}


DEAL_II_NAMESPACE_CLOSE

#endif
