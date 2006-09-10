//---------------------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 2002, 2003, 2004, 2005, 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------

#include <base/quadrature.h>
#include <grid/tria.h>
#include <grid/tria_iterator.h>
#include <dofs/dof_accessor.h>
#include <fe/fe.h>
#include <fe/mapping.h>
#include <fe/fe_dgp_nonparametric.h>
#include <fe/fe_values.h>

#include <sstream>


template <int dim>
FE_DGPNonparametric<dim>::FE_DGPNonparametric (const unsigned int degree)
		:
		FiniteElement<dim> (
		  FiniteElementData<dim>(get_dpo_vector(degree), 1, degree,
					 FiniteElementData<dim>::L2),
		  std::vector<bool>(
		    FiniteElementData<dim>(get_dpo_vector(degree), 1, degree).dofs_per_cell,true),
				    std::vector<std::vector<bool> >(
				      FiniteElementData<dim>(get_dpo_vector(degree),1, degree).dofs_per_cell,
				      std::vector<bool>(1,true))),
                degree(degree),
                polynomial_space (Polynomials::Legendre::generate_complete_basis(degree))
{
  const unsigned int n_dofs = this->dofs_per_cell;
  for (unsigned int i=0;i<GeometryInfo<dim>::children_per_cell;++i)
    {
      this->prolongation[i].reinit(n_dofs, n_dofs);
      for (unsigned int j=0;j<n_dofs;++j)
	this->prolongation[i](j,j) = 1.;
    }
                                   // restriction can be defined
                                   // through projection for
                                   // discontinuous elements, but is
                                   // presently not implemented for DGPNonparametric
                                   // elements.
                                   //
                                   // if it were, then the following
                                   // snippet would be the right code
//    if ((degree < Matrices::n_projection_matrices) &&
//        (Matrices::projection_matrices[degree] != 0))
//      {
//        restriction[0].fill (Matrices::projection_matrices[degree]);
//      }
//    else
//  				     // matrix undefined, set size to zero
//      for (unsigned int i=0;i<GeometryInfo<dim>::children_per_cell;++i)
//        restriction[i].reinit(0, 0);
                                   // since not implemented, set to
                                   // "empty"
  for (unsigned int i=0;i<GeometryInfo<dim>::children_per_cell;++i)
    this->restriction[i].reinit(0, 0);

                                   // note further, that these
                                   // elements have neither support
                                   // nor face-support points, so
                                   // leave these fields empty
}



template <int dim>
std::string
FE_DGPNonparametric<dim>::get_name () const
{
				   // note that the
				   // FETools::get_fe_from_name
				   // function depends on the
				   // particular format of the string
				   // this function returns, so they
				   // have to be kept in synch

  std::ostringstream namebuf;
  namebuf << "FE_DGPNonparametric<" << dim << ">(" << degree << ")";

  return namebuf.str();
}



template <int dim>
FiniteElement<dim> *
FE_DGPNonparametric<dim>::clone() const
{
  return new FE_DGPNonparametric<dim>(degree);
}



template <int dim>
double
FE_DGPNonparametric<dim>::shape_value (const unsigned int i,
			  const Point<dim> &p) const
{
  Assert (i<this->dofs_per_cell, ExcIndexRange(i, 0, this->dofs_per_cell));
  return polynomial_space.compute_value(i, p);
}



template <int dim>
double
FE_DGPNonparametric<dim>::shape_value_component (const unsigned int i,
				    const Point<dim> &p,
				    const unsigned int component) const
{
  Assert (i<this->dofs_per_cell, ExcIndexRange(i, 0, this->dofs_per_cell));
  Assert (component == 0, ExcIndexRange (component, 0, 1));
  return polynomial_space.compute_value(i, p);
}



template <int dim>
Tensor<1,dim>
FE_DGPNonparametric<dim>::shape_grad (const unsigned int i,
			 const Point<dim> &p) const
{
  Assert (i<this->dofs_per_cell, ExcIndexRange(i, 0, this->dofs_per_cell));
  return polynomial_space.compute_grad(i, p);
}


template <int dim>
Tensor<1,dim>
FE_DGPNonparametric<dim>::shape_grad_component (const unsigned int i,
				   const Point<dim> &p,
				   const unsigned int component) const
{
  Assert (i<this->dofs_per_cell, ExcIndexRange(i, 0, this->dofs_per_cell));
  Assert (component == 0, ExcIndexRange (component, 0, 1));
  return polynomial_space.compute_grad(i, p);
}



template <int dim>
Tensor<2,dim>
FE_DGPNonparametric<dim>::shape_grad_grad (const unsigned int i,
			      const Point<dim> &p) const
{
  Assert (i<this->dofs_per_cell, ExcIndexRange(i, 0, this->dofs_per_cell));
  return polynomial_space.compute_grad_grad(i, p);
}



template <int dim>
Tensor<2,dim>
FE_DGPNonparametric<dim>::shape_grad_grad_component (const unsigned int i,
					const Point<dim> &p,
					const unsigned int component) const
{
  Assert (i<this->dofs_per_cell, ExcIndexRange(i, 0, this->dofs_per_cell));
  Assert (component == 0, ExcIndexRange (component, 0, 1));
  return polynomial_space.compute_grad_grad(i, p);
}


//---------------------------------------------------------------------------
// Auxiliary functions
//---------------------------------------------------------------------------


template <int dim>
std::vector<unsigned int>
FE_DGPNonparametric<dim>::get_dpo_vector (const unsigned int deg)
{
  std::vector<unsigned int> dpo(dim+1, static_cast<unsigned int>(0));
  dpo[dim] = deg+1;
  for (unsigned int i=1;i<dim;++i)
    {
      dpo[dim] *= deg+1+i;
      dpo[dim] /= i+1;
    }
  return dpo;
}


template <int dim>
UpdateFlags
FE_DGPNonparametric<dim>::update_once (const UpdateFlags) const
{
				   // for this kind of elements, only
				   // the values can be precomputed
				   // once and for all. set this flag
				   // if the values are requested at
				   // all
  return update_default;
}


template <int dim>
UpdateFlags
FE_DGPNonparametric<dim>::update_each (const UpdateFlags flags) const
{
  UpdateFlags out = flags;

  if (flags & (update_values | update_gradients | update_second_derivatives))
    out |= update_q_points ;

  return out;
}


//---------------------------------------------------------------------------
// Data field initialization
//---------------------------------------------------------------------------

template <int dim>
typename Mapping<dim>::InternalDataBase *
FE_DGPNonparametric<dim>::get_data (
  const UpdateFlags      update_flags,
  const Mapping<dim>&,
  const Quadrature<dim>&) const
{
				   // generate a new data object
  InternalData* data = new InternalData;
				   // check what needs to be
				   // initialized only once and what
				   // on every cell/face/subface we
				   // visit
  data->update_once = update_once(update_flags);
  data->update_each = update_each(update_flags);
  data->update_flags = data->update_once | data->update_each;

  const UpdateFlags flags(data->update_flags);

				   // initialize fields only if really
				   // necessary. otherwise, don't
				   // allocate memory
  if (flags & update_values)
    {
      data->values.resize (this->dofs_per_cell);
    }

  if (flags & update_gradients)
    {
      data->grads.resize (this->dofs_per_cell);
    }

  if (flags & update_second_derivatives)
    {
      data->grad_grads.resize (this->dofs_per_cell);
    }
  return data;
}



//---------------------------------------------------------------------------
// Fill data of FEValues
//---------------------------------------------------------------------------

template <int dim>
void
FE_DGPNonparametric<dim>::fill_fe_values (
  const Mapping<dim>&,
  const typename Triangulation<dim>::cell_iterator&,
  const Quadrature<dim>&,
  typename Mapping<dim>::InternalDataBase&,
  typename Mapping<dim>::InternalDataBase& fedata,
  FEValuesData<dim>& data) const
{
				   // convert data object to internal
				   // data for this class. fails with
				   // an exception if that is not
				   // possible
  InternalData &fe_data = dynamic_cast<InternalData &> (fedata);
  
  const UpdateFlags flags(fe_data.current_update_flags());
  Assert (flags & update_q_points, ExcInternalError());
  
  const unsigned int n_q_points = data.quadrature_points.size();
  
  if (flags & (update_values | update_gradients))
    for (unsigned int i=0; i<n_q_points; ++i)
      {
	polynomial_space.compute(data.quadrature_points[i],
				 fe_data.values, fe_data.grads, fe_data.grad_grads);
	for (unsigned int k=0; k<this->dofs_per_cell; ++k)
	  {
	    if (flags & update_values)
	      data.shape_values[k][i] = fe_data.values[k];
	    if (flags & update_gradients)
	      data.shape_gradients[k][i] = fe_data.grads[k];
	    if (flags & update_second_derivatives)
	      data.shape_2nd_derivatives[k][i] = fe_data.grad_grads[k];
	  }
      }
}



template <int dim>
void
FE_DGPNonparametric<dim>::fill_fe_face_values (
  const Mapping<dim>&,
  const typename Triangulation<dim>::cell_iterator&,
  const unsigned int,
  const Quadrature<dim-1>&,
  typename Mapping<dim>::InternalDataBase&,
  typename Mapping<dim>::InternalDataBase&       fedata,
  FEValuesData<dim>&                             data) const
{
				   // convert data object to internal
				   // data for this class. fails with
				   // an exception if that is not
				   // possible
  InternalData &fe_data = dynamic_cast<InternalData &> (fedata);

  const UpdateFlags flags(fe_data.update_once | fe_data.update_each);
  Assert (flags & update_q_points, ExcInternalError());

  const unsigned int n_q_points = data.quadrature_points.size();
  
  if (flags & (update_values | update_gradients))
    for (unsigned int i=0; i<n_q_points; ++i)
      {
	polynomial_space.compute(data.quadrature_points[i],
				 fe_data.values, fe_data.grads, fe_data.grad_grads);
	for (unsigned int k=0; k<this->dofs_per_cell; ++k)
	  {
	    if (flags & update_values)
	      data.shape_values[k][i] = fe_data.values[k];
	    if (flags & update_gradients)
	      data.shape_gradients[k][i] = fe_data.grads[k];
	    if (flags & update_second_derivatives)
	      data.shape_2nd_derivatives[k][i] = fe_data.grad_grads[k];
	  }
      }
}



template <int dim>
void
FE_DGPNonparametric<dim>::fill_fe_subface_values (
  const Mapping<dim>&,
  const typename Triangulation<dim>::cell_iterator&,
  const unsigned int,
  const unsigned int,
  const Quadrature<dim-1>&,
  typename Mapping<dim>::InternalDataBase&,
  typename Mapping<dim>::InternalDataBase&       fedata,
  FEValuesData<dim>&                             data) const
{
				   // convert data object to internal
				   // data for this class. fails with
				   // an exception if that is not
				   // possible
  InternalData &fe_data = dynamic_cast<InternalData &> (fedata);
  
  const UpdateFlags flags(fe_data.update_once | fe_data.update_each);
  Assert (flags & update_q_points, ExcInternalError());

  const unsigned int n_q_points = data.quadrature_points.size();
  
  if (flags & (update_values | update_gradients))
    for (unsigned int i=0; i<n_q_points; ++i)
      {
	polynomial_space.compute(data.quadrature_points[i],
				 fe_data.values, fe_data.grads, fe_data.grad_grads);
	for (unsigned int k=0; k<this->dofs_per_cell; ++k)
	  {
	    if (flags & update_values)
	      data.shape_values[k][i] = fe_data.values[k];
	    if (flags & update_gradients)
	      data.shape_gradients[k][i] = fe_data.grads[k];
	    if (flags & update_second_derivatives)
	      data.shape_2nd_derivatives[k][i] = fe_data.grad_grads[k];
	  }
      }
}



template <int dim>
unsigned int
FE_DGPNonparametric<dim>::n_base_elements () const
{
  return 1;
}



template <int dim>
const FiniteElement<dim> &
FE_DGPNonparametric<dim>::base_element (const unsigned int index) const
{
  Assert (index==0, ExcIndexRange(index, 0, 1));
  return *this;
}



template <int dim>
unsigned int
FE_DGPNonparametric<dim>::element_multiplicity (const unsigned int index) const
{
  Assert (index==0, ExcIndexRange(index, 0, 1));
  return 1;
}



template <int dim>
void
FE_DGPNonparametric<dim>::
get_face_interpolation_matrix (const FiniteElement<dim> &x_source_fe,
			       FullMatrix<double>       &interpolation_matrix) const
{
				   // this is only implemented, if the source
				   // FE is also a DGPNonparametric element. in that case,
				   // both elements have no dofs on their
				   // faces and the face interpolation matrix
				   // is necessarily empty -- i.e. there isn't
				   // much we need to do here.
  AssertThrow ((x_source_fe.get_name().find ("FE_DGPNonparametric<") == 0)
               ||
               (dynamic_cast<const FE_DGPNonparametric<dim>*>(&x_source_fe) != 0),
               typename FiniteElement<dim>::
               ExcInterpolationNotImplemented());
  
  Assert (interpolation_matrix.m() == 0,
	  ExcDimensionMismatch (interpolation_matrix.m(),
				0));
  Assert (interpolation_matrix.n() == 0,
	  ExcDimensionMismatch (interpolation_matrix.m(),
				0));
}



template <int dim>
void
FE_DGPNonparametric<dim>::
get_subface_interpolation_matrix (const FiniteElement<dim> &x_source_fe,
				  const unsigned int ,
				  FullMatrix<double>           &interpolation_matrix) const
{
				   // this is only implemented, if the source
				   // FE is also a DGPNonparametric element. in that case,
				   // both elements have no dofs on their
				   // faces and the face interpolation matrix
				   // is necessarily empty -- i.e. there isn't
				   // much we need to do here.
  AssertThrow ((x_source_fe.get_name().find ("FE_DGPNonparametric<") == 0)
               ||
               (dynamic_cast<const FE_DGPNonparametric<dim>*>(&x_source_fe) != 0),
               typename FiniteElement<dim>::
               ExcInterpolationNotImplemented());
  
  Assert (interpolation_matrix.m() == 0,
	  ExcDimensionMismatch (interpolation_matrix.m(),
				0));
  Assert (interpolation_matrix.n() == 0,
	  ExcDimensionMismatch (interpolation_matrix.m(),
				0));
}



template <int dim>
bool
FE_DGPNonparametric<dim>::hp_constraints_are_implemented () const
{
  return true;
}



template <int dim>
std::vector<std::pair<unsigned int, unsigned int> >
FE_DGPNonparametric<dim>::
hp_vertex_dof_identities (const FiniteElement<dim> &fe_other) const
{
				   // there are no such constraints for DGPNonparametric
				   // elements at all
  if (dynamic_cast<const FE_DGPNonparametric<dim>*>(&fe_other) != 0)
    return
      std::vector<std::pair<unsigned int, unsigned int> > ();
  else
    {
      Assert (false, ExcNotImplemented());
      return std::vector<std::pair<unsigned int, unsigned int> > ();
    }
}



template <int dim>
std::vector<std::pair<unsigned int, unsigned int> >
FE_DGPNonparametric<dim>::
hp_line_dof_identities (const FiniteElement<dim> &fe_other) const
{
				   // there are no such constraints for DGPNonparametric
				   // elements at all
  if (dynamic_cast<const FE_DGPNonparametric<dim>*>(&fe_other) != 0)
    return
      std::vector<std::pair<unsigned int, unsigned int> > ();
  else
    {
      Assert (false, ExcNotImplemented());
      return std::vector<std::pair<unsigned int, unsigned int> > ();
    }
}



template <int dim>
std::vector<std::pair<unsigned int, unsigned int> >
FE_DGPNonparametric<dim>::
hp_quad_dof_identities (const FiniteElement<dim>        &fe_other) const
{
				   // there are no such constraints for DGPNonparametric
				   // elements at all
  if (dynamic_cast<const FE_DGPNonparametric<dim>*>(&fe_other) != 0)
    return
      std::vector<std::pair<unsigned int, unsigned int> > ();
  else
    {
      Assert (false, ExcNotImplemented());
      return std::vector<std::pair<unsigned int, unsigned int> > ();
    }
}



template <int dim>
FiniteElementDomination::Domination
FE_DGPNonparametric<dim>::
compare_for_face_domination (const FiniteElement<dim> &fe_other) const
{
				   // check whether both are discontinuous
				   // elements and both could dominate, see
				   // the description of
				   // FiniteElementDomination::Domination
  if (dynamic_cast<const FE_DGPNonparametric<dim>*>(&fe_other) != 0)
    return FiniteElementDomination::either_element_can_dominate;

  Assert (false, ExcNotImplemented());
  return FiniteElementDomination::neither_element_dominates;
}



template <int dim>
bool
FE_DGPNonparametric<dim>::has_support_on_face (const unsigned int,
				  const unsigned int) const
{
  return true;
}



template <int dim>
unsigned int
FE_DGPNonparametric<dim>::memory_consumption () const
{
  Assert (false, ExcNotImplemented ());
  return 0;
}



template <int dim>
unsigned int
FE_DGPNonparametric<dim>::get_degree () const
{
  return degree;
}




template class FE_DGPNonparametric<deal_II_dimension>;
