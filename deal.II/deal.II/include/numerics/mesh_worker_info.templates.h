//---------------------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 2009 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------


#include <numerics/mesh_worker_info.h>

DEAL_II_NAMESPACE_OPEN


namespace MeshWorker
{
  template <int dim, int spacedim>
  DoFInfo<dim,spacedim>::DoFInfo(const BlockInfo& info)
		  : block_info(&info, typeid(*this).name())
  {}


  template <int dim, int spacedim>
  void
  DoFInfo<dim,spacedim>::get_indices(const typename DoFHandler<dim, spacedim>::cell_iterator c)
  {
    indices.resize(c->get_fe().dofs_per_cell);
  
    if (block_info == 0 || block_info->local().size() == 0)
      c->get_dof_indices(indices);
    else
      {
	indices_org.resize(c->get_fe().dofs_per_cell);
	c->get_dof_indices(indices_org);
	for (unsigned int i=0;i<indices.size();++i)
	  indices[this->block_info->renumber(i)] = indices_org[i];
      }
  }


  template <int dim, int spacedim>
  void
  DoFInfo<dim,spacedim>::get_indices(const typename MGDoFHandler<dim, spacedim>::cell_iterator c)
  {
    indices.resize(c->get_fe().dofs_per_cell);
  
    if (block_info == 0 || block_info->local().size() == 0)
      c->get_mg_dof_indices(indices);
    else
      {
	indices_org.resize(c->get_fe().dofs_per_cell);
	c->get_mg_dof_indices(indices_org);
	for (unsigned int i=0;i<indices.size();++i)
	  indices[this->block_info->renumber(i)] = indices_org[i];
      }
  }

//----------------------------------------------------------------------//

  template<int dim, class FVB, int sdim>
  IntegrationInfo<dim,FVB,sdim>::IntegrationInfo(const BlockInfo& block_info)
		  :
		  DoFInfo<dim,sdim>(block_info),
		  fevalv(0),
		  multigrid(false),
		  global_data(boost::shared_ptr<VectorDataBase<dim, sdim> >(new VectorDataBase<dim, sdim>))
  {}


  template<int dim, class FVB, int sdim>
  template <class FEVALUES>
  void
  IntegrationInfo<dim,FVB,sdim>::initialize(
    const FiniteElement<dim,sdim>& el,
    const Mapping<dim,sdim>& mapping,
    const Quadrature<FEVALUES::integral_dimension>& quadrature,
    const UpdateFlags flags)
  {
    if (this->block_info == 0 || this->block_info->local().size() == 0)
      {
	fevalv.resize(1);	      
	fevalv[0] = boost::shared_ptr<FVB> (
	  new FEVALUES (mapping, el, quadrature, flags));
      }
    else
      {
	fevalv.resize(el.n_base_elements());
	for (unsigned int i=0;i<fevalv.size();++i)
	  {
	    fevalv[i] = boost::shared_ptr<FVB> (
	      new FEVALUES (mapping, el.base_element(i), quadrature, flags));
	  }
      }

    values.resize(global_data->n_values());
				     // For all selected finite
				     // element functions
    for (unsigned int i=0;i<values.size();++i)
      {
	values[i].resize(this->local_indices().size());
					 // For all components
	for (unsigned int j=0;j<values[i].size();++j)
	  {
	    values[i][j].resize(quadrature.size());
	  }
      }
    
    gradients.resize(global_data->n_gradients());
				     // For all selected finite
				     // element functions
    for (unsigned int i=0;i<gradients.size();++i)
      {
	gradients[i].resize(this->local_indices().size());
					 // For all components
	for (unsigned int j=0;j<gradients[i].size();++j)
	  {
	    gradients[i][j].resize(quadrature.size());
	  }
      }
    
    hessians.resize(global_data->n_hessians());
				     // For all selected finite
				     // element functions
    for (unsigned int i=0;i<hessians.size();++i)
      {
	hessians[i].resize(this->local_indices().size());
					 // For all components
	for (unsigned int j=0;j<hessians[i].size();++j)
	  {
	    hessians[i][j].resize(quadrature.size());
	  }
      }
  }
  

  template<int dim, class FVB, int sdim>
  void
  IntegrationInfo<dim,FVB,sdim>::initialize_data(
    const boost::shared_ptr<VectorDataBase<dim,sdim> > data)
  {
    global_data = data;
  }


  template<int dim, class FVB, int sdim>
  void
  IntegrationInfo<dim,FVB,sdim>::clear()
  {
    fevalv.resize(0);
  }



  template<int dim, class FVB, int sdim>
  void
  IntegrationInfo<dim,FVB,sdim>::fill_local_data(bool split_fevalues)
  {
     if (split_fevalues)
       {
	unsigned int comp = 0;
					 // Loop over all blocks
	for (unsigned int b=0;b<this->block_info->local().size();++b)
	  {
	    const unsigned int fe_no = this->block_info->base_element(b);
	    const FEValuesBase<dim>& fe = this->fe(fe_no);
	    const unsigned int n_comp = fe.get_fe().n_components();
	    const unsigned int block_start = this->block_info->local().block_start(b);
	    const unsigned int block_size = this->block_info->local().block_size(b);
	    
	    this->global_data->fill(values, gradients, hessians, fe, this->indices,
				    comp, n_comp, block_start, block_size);
 	    comp += n_comp;
	  }
       }
     else
       {
	 const FEValuesBase<dim>& fe = this->fe(0);
	 const unsigned int n_comp = fe.get_fe().n_components();
	 this->global_data->fill(values, gradients, hessians, fe, this->indices,
				 0, n_comp, 0, this->indices.size());
       }
  }
}


DEAL_II_NAMESPACE_CLOSE

