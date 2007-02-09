//---------------------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 2005, 2006, 2007 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------

#include <base/qprojector.h>
#include <base/polynomials_bdm.h>
#include <base/polynomials_raviart_thomas.h>
#include <base/polynomials_abf.h>
#include <fe/fe_poly_tensor.h>
#include <fe/fe_values.h>
#include <fe/mapping_cartesian.h>

#include <base/logstream.h>

DEAL_II_NAMESPACE_OPEN

namespace
{
//---------------------------------------------------------------------------
// Utility method, which is used to determine the change of sign for
// the DoFs on the faces of the given cell.
//---------------------------------------------------------------------------

/**
 * On noncartesian grids, the sign of the DoFs associated with the faces of
 * the elements has to be changed in some cases.  This procedure implements an
 * algorithm, which determines the DoFs, which need this sign change for a
 * given cell.
 */
#if deal_II_dimension == 1
  void
  get_face_sign_change (const Triangulation<1>::cell_iterator &,
			const unsigned int                     ,
			std::vector<double>                   &face_sign)
  {
				     // nothing to do in 1d
    std::fill (face_sign.begin (), face_sign.end (), 1.0);
  }
#endif  

#if deal_II_dimension == 2
  void
  get_face_sign_change (const Triangulation<2>::cell_iterator &cell,
			const unsigned int                     dofs_per_face,
			std::vector<double>                   &face_sign)
  {
    const unsigned int dim = 2;
    
				     // Default is no sign
				     // change. I.e. multiply by one.
    std::fill (face_sign.begin (), face_sign.end (), 1.0);
    
    for (unsigned int f = GeometryInfo<dim>::faces_per_cell / 2; 
	 f < GeometryInfo<dim>::faces_per_cell; ++f)
      {
	Triangulation<dim>::face_iterator face = cell->face (f);
	if (!face->at_boundary ())
	  {
	    const unsigned int neighbor_level = cell->neighbor (f)->level ();
	    const unsigned int cell_level = cell->level ();

					     // check of neighbor is more
					     // refined (this case should
					     // never be encountered)
	    Assert (neighbor_level <= cell_level,
		    ExcMessage ("neighbor_level is larger than cell level!"));

	    const unsigned int nn = (neighbor_level == cell_level
				     ?
				     cell->neighbor_of_neighbor (f)
				     :
				     cell->neighbor_of_coarser_neighbor (f).first);
	      
	    if (nn < GeometryInfo<dim>::faces_per_cell / 2)
	      for (unsigned int j = 0; j < dofs_per_face; ++j)
		{
		  Assert (f * dofs_per_face + j < face_sign.size(),
			  ExcInternalError());

//TODO: This is probably only going to work for those elements for which all dofs are face dofs
		  face_sign[f * dofs_per_face + j] = -1.0;
		}
	  }
      }
  }
#endif


#if deal_II_dimension == 3
  void
  get_face_sign_change (const Triangulation<3>::cell_iterator &/*cell*/,
			const unsigned int                     /*dofs_per_face*/,
			std::vector<double>                   &face_sign)
  {
    std::fill (face_sign.begin (), face_sign.end (), 1.0);
//TODO: think about what it would take here    
  }
#endif
}



template <class POLY, int dim>
FE_PolyTensor<POLY,dim>::FE_PolyTensor (const unsigned int degree,
					const FiniteElementData<dim> &fe_data,
					const std::vector<bool> &restriction_is_additive_flags,
					const std::vector<std::vector<bool> > &nonzero_components)
		:
		FiniteElement<dim> (fe_data,
				    restriction_is_additive_flags,
				    nonzero_components),
                poly_space(POLY(degree))
{
  cached_point(0) = -1;
}



template <class POLY, int dim>
double
FE_PolyTensor<POLY,dim>::shape_value (const unsigned int, const Point<dim> &) const
{
  Assert(false, typename FiniteElement<dim>::ExcFENotPrimitive());
  return 0.;
}



template <class POLY, int dim>
double
FE_PolyTensor<POLY,dim>::shape_value_component (const unsigned int i,
						const Point<dim> &p,
						const unsigned int component) const
{
  Assert (i<this->dofs_per_cell, ExcIndexRange(i,0,this->dofs_per_cell));
  Assert (component < dim, ExcIndexRange (component, 0, dim));
  
  if (cached_point != p || cached_values.size() == 0)
    {
      cached_point = p;
      cached_values.resize(poly_space.n());
      poly_space.compute(p, cached_values, cached_grads, cached_grad_grads);
    }
  
  double s = 0;
  if (inverse_node_matrix.n_cols() == 0)
    return cached_values[i][component];
  else
    for (unsigned int j=0;j<inverse_node_matrix.n_cols();++j)
      s += inverse_node_matrix(j,i) * cached_values[j][component];
  return s;
}



template <class POLY, int dim>
Tensor<1,dim>
FE_PolyTensor<POLY,dim>::shape_grad (const unsigned int,
				     const Point<dim> &) const
{
  Assert(false, typename FiniteElement<dim>::ExcFENotPrimitive());
  return Tensor<1,dim>();
}



template <class POLY, int dim>
Tensor<1,dim>
FE_PolyTensor<POLY,dim>::shape_grad_component (const unsigned int i,
					       const Point<dim> &p,
					       const unsigned int component) const
{
  Assert (i<this->dofs_per_cell, ExcIndexRange(i,0,this->dofs_per_cell));
  Assert (component < dim, ExcIndexRange (component, 0, dim));
  
  if (cached_point != p || cached_grads.size() == 0)
    {
      cached_point = p;
      cached_grads.resize(poly_space.n());
      poly_space.compute(p, cached_values, cached_grads, cached_grad_grads);
    }
  
  Tensor<1,dim> s;
  if (inverse_node_matrix.n_cols() == 0)
    return cached_grads[i][component];
  else
    for (unsigned int j=0;j<inverse_node_matrix.n_cols();++j)
      s += inverse_node_matrix(j,i) * cached_grads[j][component];
  
  return s;
}



template <class POLY, int dim>
Tensor<2,dim>
FE_PolyTensor<POLY,dim>::shape_grad_grad (const unsigned int, const Point<dim> &) const
{
  Assert(false, typename FiniteElement<dim>::ExcFENotPrimitive());
  return Tensor<2,dim>();
}



template <class POLY, int dim>
Tensor<2,dim>
FE_PolyTensor<POLY,dim>::shape_grad_grad_component (const unsigned int i,
						    const Point<dim> &p,
						    const unsigned int component) const
{
  Assert (i<this->dofs_per_cell, ExcIndexRange(i,0,this->dofs_per_cell));
  Assert (component < dim, ExcIndexRange (component, 0, dim));
  
  if (cached_point != p || cached_grad_grads.size() == 0)
    {
      cached_point = p;
      cached_grad_grads.resize(poly_space.n());
      poly_space.compute(p, cached_values, cached_grads, cached_grad_grads);
    }
  
  Tensor<2,dim> s;
  if (inverse_node_matrix.n_cols() == 0)
    return cached_grad_grads[i][component];
  else
    for (unsigned int j=0;j<inverse_node_matrix.n_cols();++j)
      s += inverse_node_matrix(i,j) * cached_grad_grads[j][component];
  
  return s;
}



//---------------------------------------------------------------------------
// Data field initialization
//---------------------------------------------------------------------------

template <class POLY, int dim>
typename Mapping<dim>::InternalDataBase *
FE_PolyTensor<POLY,dim>::get_data (const UpdateFlags      update_flags,
			     const Mapping<dim>    &mapping,
			     const Quadrature<dim> &quadrature) const
{
				   // generate a new data object and
				   // initialize some fields
  InternalData* data = new InternalData;

				   // check what needs to be
				   // initialized only once and what
				   // on every cell/face/subface we
				   // visit
  data->update_once = update_once(update_flags);
  data->update_each = update_each(update_flags);
  data->update_flags = data->update_once | data->update_each;

  const UpdateFlags flags(data->update_flags);
  const unsigned int n_q_points = quadrature.n_quadrature_points;
  
				   // some scratch arrays
  std::vector<Tensor<1,dim> > values(0);
  std::vector<Tensor<2,dim> > grads(0);
  std::vector<Tensor<3,dim> > grad_grads(0);

				   // initialize fields only if really
				   // necessary. otherwise, don't
				   // allocate memory
  if (flags & update_values)
    {
      values.resize (this->dofs_per_cell);
      data->shape_values.resize (this->dofs_per_cell);
      for (unsigned int i=0;i<this->dofs_per_cell;++i)
	data->shape_values[i].resize (n_q_points);
    }

  if (flags & update_gradients)
    {
      grads.resize (this->dofs_per_cell);
      data->shape_grads.resize (this->dofs_per_cell);
      for (unsigned int i=0;i<this->dofs_per_cell;++i)
	data->shape_grads[i].resize (n_q_points);
    }

				   // if second derivatives through
				   // finite differencing is required,
				   // then initialize some objects for
				   // that
  if (flags & update_second_derivatives)
    {
//      grad_grads.resize (this->dofs_per_cell);      
      data->initialize_2nd (this, mapping, quadrature);
    }

				   // Compute shape function values
				   // and derivatives on the reference
				   // cell. Make sure, that for the
				   // node values N_i holds
				   // N_i(v_j)=\delta_ij for all basis
				   // functions v_j
  if (flags & (update_values | update_gradients))
    for (unsigned int k=0; k<n_q_points; ++k)
      {
	poly_space.compute(quadrature.point(k),
			   values, grads, grad_grads);
	
	if (flags & update_values)
	  {
	    if (inverse_node_matrix.n_cols() == 0)
	      for (unsigned int i=0; i<this->dofs_per_cell; ++i)
		data->shape_values[i][k] = values[i];
	    else
	      for (unsigned int i=0; i<this->dofs_per_cell; ++i)
		for (unsigned int j=0; j<this->dofs_per_cell; ++j)
		  data->shape_values[i][k] += inverse_node_matrix(j,i) * values[j];
	  }
	
	if (flags & update_gradients)
	  {
	    if (inverse_node_matrix.n_cols() == 0)
	      for (unsigned int i=0; i<this->dofs_per_cell; ++i)
		data->shape_grads[i][k] = grads[i];
	    else
	      for (unsigned int i=0; i<this->dofs_per_cell; ++i)
		for (unsigned int j=0; j<this->dofs_per_cell; ++j)
		  data->shape_grads[i][k] += inverse_node_matrix(j,i) * grads[j];
	  }
	
      }
  return data;
}




//---------------------------------------------------------------------------
// Fill data of FEValues
//---------------------------------------------------------------------------

template <class POLY, int dim>
void
FE_PolyTensor<POLY,dim>::fill_fe_values (
  const Mapping<dim>                   &mapping,
  const typename Triangulation<dim>::cell_iterator &cell,
  const Quadrature<dim>                &quadrature,
  typename Mapping<dim>::InternalDataBase &mapping_data,
  typename Mapping<dim>::InternalDataBase &fedata,
  FEValuesData<dim>                    &data) const
{
				   // convert data object to internal
				   // data for this class. fails with
				   // an exception if that is not
				   // possible
  InternalData &fe_data = dynamic_cast<InternalData &> (fedata);

//   Assert(mapping_type == independent
// 	 || ( mapping_type == independent_on_cartesian
// 	      && dynamic_cast<const MappingCartesian<dim>*>(&mapping) != 0),
// 	 ExcNotImplemented());

  const unsigned int n_q_points = quadrature.n_quadrature_points;
  const UpdateFlags flags(fe_data.current_update_flags());
  
  Assert(!(flags & update_values) || fe_data.shape_values.size() == this->dofs_per_cell,
	 ExcDimensionMismatch(fe_data.shape_values.size(), this->dofs_per_cell));
  Assert(!(flags & update_values) || fe_data.shape_values[0].size() == n_q_points,
	 ExcDimensionMismatch(fe_data.shape_values[0].size(), n_q_points));

  // Create table with sign changes, due to the special structure of the RT elements.
  // TODO: Preliminary hack to demonstrate the overall prinicple!

  // Compute eventual sign changes depending on the neighborhood
  // between two faces.
  std::vector<double> sign_change (this->dofs_per_cell, 1.0);
  get_face_sign_change (cell, this->dofs_per_face, sign_change);
  
  for (unsigned int i=0; i<this->dofs_per_cell; ++i)
    {
      const unsigned int first = data.shape_function_to_row_table[i];
      
      if (flags & update_values)
	{
	  switch (mapping_type)
	    {
	      case independent:
	      case independent_on_cartesian:
	      {
		for (unsigned int k=0; k<n_q_points; ++k)
		  for (unsigned int d=0; d<dim; ++d)
		    data.shape_values(first+d,k) = fe_data.shape_values[i][k][d];
		break;
	      }
	      
	      case covariant:
	      case contravariant:
	      {
						 // Use auxiliary vector for
						 // transformation
		std::vector<Tensor<1,dim> > shape_values (n_q_points);
		if (mapping_type == covariant)
		  mapping.transform_covariant(fe_data.shape_values[i], 0,
					      shape_values, mapping_data);
		else
		  mapping.transform_contravariant(fe_data.shape_values[i], 0,
						  shape_values, mapping_data);
		    
						 // then copy over to target:
		for (unsigned int k=0; k<n_q_points; ++k)
		  {
						     // Recompute determinant
		    const double
		      J = (mapping_type == contravariant ?
			   data.JxW_values[k] / quadrature.weight(k)
			   :
			   1.0);

		    for (unsigned int d=0; d<dim; ++d)
		      data.shape_values(first+d,k)
			= sign_change[i] * (shape_values[k][d] / J);
		  }
		break;
	      }
	      
	      default:
		Assert(false, ExcNotImplemented());
	    }
	}
      
      if (flags & update_gradients)
	{
	  std::vector<Tensor<2,dim> > shape_grads1 (n_q_points);
	  std::vector<Tensor<2,dim> > shape_grads2 (n_q_points);

	  switch (mapping_type)
	    {
	      case independent:
	      case independent_on_cartesian:
	      {
		mapping.transform_covariant(fe_data.shape_grads[i], 0,
					    shape_grads1,
					    mapping_data);
		for (unsigned int k=0; k<n_q_points; ++k)
		  for (unsigned int d=0; d<dim; ++d)
		    data.shape_gradients[first+d][k] = shape_grads1[k][d];
		break;
	      }
	      
	      case covariant:
	      {
		mapping.transform_covariant(fe_data.shape_grads[i], 0,
					    shape_grads1,
					    mapping_data);
		for (unsigned int q=0; q<n_q_points; ++q)
		  shape_grads2[q] = transpose(shape_grads1[q]);
						 // do second transformation
		mapping.transform_covariant(shape_grads2, 0, shape_grads1,
					    mapping_data);
						 // transpose back
		for (unsigned int q=0; q<n_q_points; ++q)
		  shape_grads2[q] = transpose(shape_grads1[q]);
		
		for (unsigned int k=0; k<n_q_points; ++k)
		  for (unsigned int d=0; d<dim; ++d)
		    data.shape_gradients[first+d][k] = shape_grads2[k][d];

		break;
	      }
	      
	      case contravariant:
	      {
		mapping.transform_covariant(fe_data.shape_grads[i], 0,
					    shape_grads1,
					    mapping_data);

		mapping.transform_contravariant(shape_grads1, 0,
						shape_grads2,
						mapping_data);

		for (unsigned int k=0; k<n_q_points; ++k)
		  for (unsigned int d=0; d<dim; ++d)
		    {
						       // Recompute determinant
		      const double
			J = (mapping_type == contravariant ?
			     data.JxW_values[k] / quadrature.weight(k)
			     :
			     1.0);
		      data.shape_gradients[first+d][k] = sign_change[i] * 
			shape_grads2[k][d] / J;
		    }
		
		break;
	      }
	      
	      default:
		Assert(false, ExcNotImplemented());
	    }
	}
    }
  
  if (flags & update_second_derivatives)
    this->compute_2nd (mapping, cell,
		       typename QProjector<dim>::DataSetDescriptor().cell(),
		       mapping_data, fe_data, data);
}



template <class POLY, int dim>
void
FE_PolyTensor<POLY,dim>::fill_fe_face_values (
  const Mapping<dim>                   &mapping,
  const typename Triangulation<dim>::cell_iterator &cell,
  const unsigned int                    face,
  const Quadrature<dim-1>              &quadrature,
  typename Mapping<dim>::InternalDataBase       &mapping_data,
  typename Mapping<dim>::InternalDataBase       &fedata,
  FEValuesData<dim>                    &data) const
{
				   // convert data object to internal
				   // data for this class. fails with
				   // an exception if that is not
				   // possible
  InternalData &fe_data = dynamic_cast<InternalData &> (fedata);

  const unsigned int n_q_points = quadrature.n_quadrature_points;
				   // offset determines which data set
				   // to take (all data sets for all
				   // faces are stored contiguously)
  
  const typename QProjector<dim>::DataSetDescriptor dsd;
  const typename QProjector<dim>::DataSetDescriptor offset
    = dsd.face (face,
		cell->face_orientation(face),
		cell->face_flip(face),
		cell->face_rotation(face),
		n_q_points);
  
  const UpdateFlags flags(fe_data.update_once | fe_data.update_each);

//   Assert(mapping_type == independent
// 	 || ( mapping_type == independent_on_cartesian
// 	      && dynamic_cast<const MappingCartesian<dim>*>(&mapping) != 0),
// 	 ExcNotImplemented());
//TODO: Size assertions

// Create table with sign changes, due to the special structure of the RT elements.
// TODO: Preliminary hack to demonstrate the overall prinicple!

				   // Compute eventual sign changes depending
				   // on the neighborhood between two faces.
  std::vector<double> sign_change (this->dofs_per_cell, 1.0);
  get_face_sign_change (cell, this->dofs_per_face, sign_change);
  
  for (unsigned int i=0; i<this->dofs_per_cell; ++i)
    {
      const unsigned int first = data.shape_function_to_row_table[i];
      
      if (flags & update_values)
	{
	  switch (mapping_type)
	    {
	      case independent:
	      case independent_on_cartesian:
	      {
		for (unsigned int k=0; k<n_q_points; ++k)
		  for (unsigned int d=0; d<dim; ++d)
		    data.shape_values(first+d,k) = fe_data.shape_values[i][k+offset][d];
		break;
	      }
	      
	      case covariant:
	      case contravariant:
	      {
						 // Use auxiliary vector
						 // for transformation
		std::vector<Tensor<1,dim> > shape_values (n_q_points);
		if (mapping_type == covariant)
		  mapping.transform_covariant(fe_data.shape_values[i], offset,
					      shape_values, mapping_data);
		else
		  mapping.transform_contravariant(fe_data.shape_values[i], offset,
						  shape_values, mapping_data);
		
						 // then copy over to target:
		for (unsigned int k=0; k<n_q_points; ++k)
		  {
						     // recompute
						     // determinant. note that
						     // we have to take the
						     // *cell* transformation
						     // (cell_JxW_values)
						     // because the usual
						     // JxW_values field
						     // contains the
						     // transformation of the
						     // face in
						     // fill_fe_face_values
		    const double
		      J = (mapping_type == contravariant ?
			   data.cell_JxW_values[k] / quadrature.weight(k)
			   :
			   1.0);
		    
		    for (unsigned int d=0; d<dim; ++d)
		      data.shape_values(first+d,k)
			= sign_change[i] * (shape_values[k][d] / J);
		  }
		
		break;
	      }
	      
	      default:
		Assert(false, ExcNotImplemented());
	    }
	}
      
      if (flags & update_gradients)
	{
	  std::vector<Tensor<2,dim> > shape_grads1 (n_q_points);
	  std::vector<Tensor<2,dim> > shape_grads2 (n_q_points);

	  switch (mapping_type)
	    {
	      case independent:
	      case independent_on_cartesian:
	      {
		mapping.transform_covariant(fe_data.shape_grads[i], offset,
					    shape_grads1, mapping_data);
		for (unsigned int k=0; k<n_q_points; ++k)
		  for (unsigned int d=0; d<dim; ++d)
		    data.shape_gradients[first+d][k] = shape_grads1[k][d];
		break;
	      }
	      
	      case covariant:
	      {
		mapping.transform_covariant(fe_data.shape_grads[i], offset,
					    shape_grads1,
					    mapping_data);
		for (unsigned int q=0; q<n_q_points; ++q)
		  shape_grads2[q] = transpose(shape_grads1[q]);
						 // do second transformation
		mapping.transform_covariant(shape_grads2, 0, shape_grads1,
					    mapping_data);
						 // transpose back
		for (unsigned int q=0; q<n_q_points; ++q)
		  shape_grads2[q] = transpose(shape_grads1[q]);
		
		for (unsigned int k=0; k<n_q_points; ++k)
		  for (unsigned int d=0; d<dim; ++d)
		    data.shape_gradients[first+d][k] = shape_grads1[k][d];
		break;
	      }
	      
	      case contravariant:
	      {
		mapping.transform_covariant(fe_data.shape_grads[i], offset,
					    shape_grads1,
					    mapping_data);

		mapping.transform_contravariant(shape_grads1, 0,
						shape_grads2,
						mapping_data);

		for (unsigned int k=0; k<n_q_points; ++k)
		  for (unsigned int d=0; d<dim; ++d)
		    {
						       // recompute
						       // determinant in the
						       // same way as above
		      const double
			J = data.cell_JxW_values[k] / quadrature.weight(k);
		      data.shape_gradients[first+d][k]
			= sign_change[i] * shape_grads2[k][d] / J;
		    }
		break;
	      }
	      
	      default:
		Assert(false, ExcNotImplemented());
	    }
	}
    }
  
  if (flags & update_second_derivatives)
    this->compute_2nd (mapping, cell, offset, mapping_data, fe_data, data);
}



template <class POLY, int dim>
void
FE_PolyTensor<POLY,dim>::fill_fe_subface_values (
  const Mapping<dim>                   &mapping,
  const typename Triangulation<dim>::cell_iterator &cell,
  const unsigned int                    face,
  const unsigned int                    subface,
  const Quadrature<dim-1>              &quadrature,
  typename Mapping<dim>::InternalDataBase       &mapping_data,
  typename Mapping<dim>::InternalDataBase       &fedata,
  FEValuesData<dim>                    &data) const
{
				   // convert data object to internal
				   // data for this class. fails with
				   // an exception if that is not
				   // possible
  InternalData &fe_data = dynamic_cast<InternalData &> (fedata);

  const unsigned int n_q_points = quadrature.n_quadrature_points;
				   // offset determines which data set
				   // to take (all data sets for all
				 // sub-faces are stored contiguously)

  const typename QProjector<dim>::DataSetDescriptor dsd;
  const typename QProjector<dim>::DataSetDescriptor offset
    = dsd.subface (face, subface,
		   cell->face_orientation(face),
		   cell->face_flip(face),
		   cell->face_rotation(face),
		   n_q_points);

  const UpdateFlags flags(fe_data.update_once | fe_data.update_each);

//   Assert(mapping_type == independent
// 	 || ( mapping_type == independent_on_cartesian
// 	      && dynamic_cast<const MappingCartesian<dim>*>(&mapping) != 0),
// 	 ExcNotImplemented());
//TODO: Size assertions

//TODO: Sign change for the face DoFs!

				   // Compute eventual sign changes depending
				   // on the neighborhood between two faces.
  std::vector<double> sign_change (this->dofs_per_cell, 1.0);
  get_face_sign_change (cell, this->dofs_per_face, sign_change);
  
  for (unsigned int i=0; i<this->dofs_per_cell; ++i)
    {
      const unsigned int first = data.shape_function_to_row_table[i];
      
      if (flags & update_values)
	{
	  switch (mapping_type)
	    {
	      case independent:
	      case independent_on_cartesian:
	      {
		for (unsigned int k=0; k<n_q_points; ++k)
		  for (unsigned int d=0; d<dim; ++d)
		    data.shape_values(first+d,k) = fe_data.shape_values[i][k+offset][d];
		break;
	      }
	      
	      case covariant:
	      case contravariant:
	      {
						 // Use auxiliary vector for
						 // transformation
		std::vector<Tensor<1,dim> > shape_values (n_q_points);
		if (mapping_type == covariant)
		  mapping.transform_covariant(fe_data.shape_values[i], offset,
					      shape_values, mapping_data);
		else
		  mapping.transform_contravariant(fe_data.shape_values[i], offset,
						  shape_values, mapping_data);
		    
						 // then copy over to target:
		for (unsigned int k=0; k<n_q_points; ++k)
		  {
						     // recompute
						     // determinant. note that
						     // we have to take the
						     // *cell* transformation
						     // (cell_JxW_values)
						     // because the usual
						     // JxW_values field
						     // contains the
						     // transformation of the
						     // face in
						     // fill_fe_face_values
		    const double
		      J = (mapping_type == contravariant ?
			   data.cell_JxW_values[k] / quadrature.weight(k)
			   :
			   1.0);
		    
		    for (unsigned int d=0; d<dim; ++d)
		      data.shape_values(first+d,k)
			= sign_change[i] * (shape_values[k][d] / J);
		  }

		break;
	      }
	      
	      default:
		Assert(false, ExcNotImplemented());
	    }
	}
      
      if (flags & update_gradients)
	{
	  std::vector<Tensor<2,dim> > shape_grads1 (n_q_points);
	  std::vector<Tensor<2,dim> > shape_grads2 (n_q_points);

	  switch (mapping_type)
	    {
	      case independent:
	      case independent_on_cartesian:
	      {
		mapping.transform_covariant(fe_data.shape_grads[i], offset,
					    shape_grads1, mapping_data);
		for (unsigned int k=0; k<n_q_points; ++k)
		  for (unsigned int d=0; d<dim; ++d)
		    data.shape_gradients[first+d][k] = shape_grads1[k][d];
		break;
	      }
	      
	      case covariant:
	      {
		mapping.transform_covariant(fe_data.shape_grads[i], offset,
					    shape_grads1,
					    mapping_data);
		for (unsigned int q=0; q<n_q_points; ++q)
		  shape_grads2[q] = transpose(shape_grads1[q]);
						 // do second transformation
		mapping.transform_covariant(shape_grads2, 0, shape_grads1,
					    mapping_data);
						 // transpose back
		for (unsigned int q=0; q<n_q_points; ++q)
		  shape_grads2[q] = transpose(shape_grads1[q]);
		
		for (unsigned int k=0; k<n_q_points; ++k)
		  for (unsigned int d=0; d<dim; ++d)
		    data.shape_gradients[first+d][k] = shape_grads1[k][d];
		break;
	      }
	      
	      case contravariant:
	      {
		mapping.transform_covariant(fe_data.shape_grads[i], offset,
					    shape_grads1,
					    mapping_data);

		mapping.transform_contravariant(shape_grads1, 0,
						shape_grads2,
						mapping_data);

		for (unsigned int k=0; k<n_q_points; ++k)
		  for (unsigned int d=0; d<dim; ++d)
		    {
						       // recompute
						       // determinant in the
						       // same way as above
		      const double
			J = data.cell_JxW_values[k] / quadrature.weight(k);
		      data.shape_gradients[first+d][k]
			= sign_change[i] * shape_grads2[k][d] / J;
		    }
		break;
	      }

	      default:
		Assert(false, ExcNotImplemented());
	    }
	}
    }
  
  if (flags & update_second_derivatives)
    this->compute_2nd (mapping, cell, offset, mapping_data, fe_data, data);
}



template <class POLY, int dim>
unsigned int
FE_PolyTensor<POLY,dim>::n_base_elements () const
{
  return 1;
}



template <class POLY, int dim>
const FiniteElement<dim> &
FE_PolyTensor<POLY,dim>::base_element (const unsigned int index) const
{
  Assert (index==0, ExcIndexRange(index, 0, 1));
  return *this;
}



template <class POLY, int dim>
unsigned int
FE_PolyTensor<POLY,dim>::element_multiplicity (const unsigned int index) const
{
  Assert (index==0, ExcIndexRange(index, 0, 1));
  return 1;
}


template <class POLY, int dim>
UpdateFlags
FE_PolyTensor<POLY,dim>::update_once (const UpdateFlags flags) const
{
  Assert (mapping_type != no_mapping, ExcNotInitialized());
  const bool values_once = (mapping_type == independent);
  
  UpdateFlags out = update_default;

  if (values_once && (flags & update_values))
    out |= update_values;

  return out;
}


template <class POLY, int dim>
UpdateFlags
FE_PolyTensor<POLY,dim>::update_each (const UpdateFlags flags) const
{
  Assert (mapping_type != no_mapping, ExcNotInitialized());
  const bool values_once = (mapping_type == independent);
  
  UpdateFlags out = update_default;

  if (!values_once && (flags & update_values))
    out |= update_values             | update_covariant_transformation;
  if (flags & update_gradients)
    out |= update_gradients          | update_covariant_transformation;
  if (flags & update_second_derivatives)
    out |= update_second_derivatives | update_covariant_transformation;

  return out;
}


template class FE_PolyTensor<PolynomialsRaviartThomas<deal_II_dimension>,deal_II_dimension>;
template class FE_PolyTensor<PolynomialsABF<deal_II_dimension>,deal_II_dimension>;

DEAL_II_NAMESPACE_CLOSE
