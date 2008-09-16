//---------------------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------


#include <base/memory_consumption.h>
#include <base/quadrature.h>
#include <lac/vector.h>
#include <lac/block_vector.h>
#include <lac/petsc_vector.h>
#include <lac/petsc_block_vector.h>
#include <lac/trilinos_vector.h>
#include <grid/tria_iterator.h>
#include <grid/tria_accessor.h>
#include <grid/tria_boundary.h>
#include <dofs/dof_accessor.h>
#include <fe/mapping_q1.h>
#include <fe/fe_values.h>
#include <fe/fe.h>

#include <iomanip>

DEAL_II_NAMESPACE_OPEN

/* ---------------- FEValuesBase<dim>::CellIteratorBase --------- */

template <int dim>
FEValuesBase<dim>::CellIteratorBase::~CellIteratorBase ()
{}



/* ---------------- FEValuesBase<dim>::CellIterator<CI> --------- */


template <int dim>
template <typename CI>
FEValuesBase<dim>::CellIterator<CI>::CellIterator (const CI &cell)
                :
                cell(cell)
{}



template <int dim>
template <typename CI>
FEValuesBase<dim>::CellIterator<CI>::
operator const typename Triangulation<dim>::cell_iterator () const
{
  return cell;
}



template <int dim>
template <typename CI>
unsigned int
FEValuesBase<dim>::CellIterator<CI>::n_dofs_for_dof_handler () const
{
  return cell->get_dof_handler().n_dofs();
}



template <int dim>
template <typename CI>
void
FEValuesBase<dim>::CellIterator<CI>::
get_interpolated_dof_values (const Vector<double> &in,
                             Vector<double>       &out) const
{
  cell->get_interpolated_dof_values (in, out);
}


template <int dim>
template <typename CI>
void
FEValuesBase<dim>::CellIterator<CI>::
get_interpolated_dof_values (const Vector<float> &in,
                             Vector<float>       &out) const
{
  cell->get_interpolated_dof_values (in, out);
}


template <int dim>
template <typename CI>
void
FEValuesBase<dim>::CellIterator<CI>::
get_interpolated_dof_values (const Vector<long double> &in,
                             Vector<long double>       &out) const
{
  cell->get_interpolated_dof_values (in, out);
}


template <int dim>
template <typename CI>
void
FEValuesBase<dim>::CellIterator<CI>::
get_interpolated_dof_values (const BlockVector<double> &in,
                             Vector<double>            &out) const
{
  cell->get_interpolated_dof_values (in, out);
}


template <int dim>
template <typename CI>
void
FEValuesBase<dim>::CellIterator<CI>::
get_interpolated_dof_values (const BlockVector<float> &in,
                             Vector<float>            &out) const
{
  cell->get_interpolated_dof_values (in, out);
}


template <int dim>
template <typename CI>
void
FEValuesBase<dim>::CellIterator<CI>::
get_interpolated_dof_values (const BlockVector<long double> &in,
                             Vector<long double>            &out) const
{
  cell->get_interpolated_dof_values (in, out);
}



#ifdef DEAL_II_USE_PETSC

template <int dim>
template <typename CI>
void
FEValuesBase<dim>::CellIterator<CI>::
get_interpolated_dof_values (const PETScWrappers::Vector &in,
                             Vector<PetscScalar>         &out) const
{
  cell->get_interpolated_dof_values (in, out);
}



template <int dim>
template <typename CI>
void
FEValuesBase<dim>::CellIterator<CI>::
get_interpolated_dof_values (const PETScWrappers::BlockVector &in,
                             Vector<PetscScalar>              &out) const
{
  cell->get_interpolated_dof_values (in, out);
}

#endif

#ifdef DEAL_II_USE_TRILINOS

template <int dim>
template <typename CI>
void
FEValuesBase<dim>::CellIterator<CI>::
get_interpolated_dof_values (const TrilinosWrappers::Vector &in,
                             Vector<TrilinosScalar>         &out) const
{
  cell->get_interpolated_dof_values (in, out);
}



template <int dim>
template <typename CI>
void
FEValuesBase<dim>::CellIterator<CI>::
get_interpolated_dof_values (const TrilinosWrappers::BlockVector &in,
                             Vector<TrilinosScalar>              &out) const
{
  cell->get_interpolated_dof_values (in, out);
}



template <int dim>
template <typename CI>
void
FEValuesBase<dim>::CellIterator<CI>::
get_interpolated_dof_values (const TrilinosWrappers::MPI::Vector &in,
                             Vector<TrilinosScalar>              &out) const
{
  cell->get_interpolated_dof_values (in, out);
}



template <int dim>
template <typename CI>
void
FEValuesBase<dim>::CellIterator<CI>::
get_interpolated_dof_values (const TrilinosWrappers::MPI::BlockVector &in,
                             Vector<TrilinosScalar>                   &out) const
{
  cell->get_interpolated_dof_values (in, out);
}

#endif


/* ---------------- FEValuesBase<dim>::TriaCellIterator --------- */

template <int dim>
const char * const
FEValuesBase<dim>::TriaCellIterator::message_string
= ("You have previously called the FEValues::reinit function with a\n"
   "cell iterator of type Triangulation<dim>::cell_iterator. However,\n"
   "when you do this, you cannot call some functions in the FEValues\n"
   "class, such as the get_function_values/gradients/hessians\n"
   "functions. If you need these functions, then you need to call\n"
   "FEValues::reinit with an iterator type that allows to extract\n"
   "degrees of freedom, such as DoFHandler<dim>::cell_iterator.");


template <int dim>
FEValuesBase<dim>::TriaCellIterator::
TriaCellIterator (const typename Triangulation<dim>::cell_iterator &cell)
                :
                cell(cell)
{}



template <int dim>
FEValuesBase<dim>::TriaCellIterator::
operator const typename Triangulation<dim>::cell_iterator () const
{
  return cell;
}



template <int dim>
unsigned int
FEValuesBase<dim>::TriaCellIterator::n_dofs_for_dof_handler () const
{
  Assert (false, ExcMessage (message_string));
  return 0;
}


template <int dim>
void
FEValuesBase<dim>::TriaCellIterator::
get_interpolated_dof_values (const Vector<double> &,
                             Vector<double>       &) const
{
  Assert (false, ExcMessage (message_string));
}


template <int dim>
void
FEValuesBase<dim>::TriaCellIterator::
get_interpolated_dof_values (const Vector<float> &,
                             Vector<float>       &) const
{
  Assert (false, ExcMessage (message_string));
}


template <int dim>
void
FEValuesBase<dim>::TriaCellIterator::
get_interpolated_dof_values (const Vector<long double> &,
                             Vector<long double>       &) const
{
  Assert (false, ExcMessage (message_string));
}


template <int dim>
void
FEValuesBase<dim>::TriaCellIterator::
get_interpolated_dof_values (const BlockVector<double> &,
                             Vector<double>            &) const
{
  Assert (false, ExcMessage (message_string));
}



template <int dim>
void
FEValuesBase<dim>::TriaCellIterator::
get_interpolated_dof_values (const BlockVector<float> &,
                             Vector<float>            &) const
{
  Assert (false, ExcMessage (message_string));
}


template <int dim>
void
FEValuesBase<dim>::TriaCellIterator::
get_interpolated_dof_values (const BlockVector<long double> &,
                             Vector<long double>            &) const
{
  Assert (false, ExcMessage (message_string));
}



#ifdef DEAL_II_USE_PETSC

template <int dim>
void
FEValuesBase<dim>::TriaCellIterator::
get_interpolated_dof_values (const PETScWrappers::Vector &,
                             Vector<PetscScalar>         &) const
{
  Assert (false, ExcMessage (message_string));
}



template <int dim>
void
FEValuesBase<dim>::TriaCellIterator::
get_interpolated_dof_values (const PETScWrappers::BlockVector &,
                             Vector<PetscScalar>              &) const
{
  Assert (false, ExcMessage (message_string));
}

#endif

#ifdef DEAL_II_USE_TRILINOS

template <int dim>
void
FEValuesBase<dim>::TriaCellIterator::
get_interpolated_dof_values (const TrilinosWrappers::Vector &,
                             Vector<TrilinosScalar>         &) const
{
  Assert (false, ExcMessage (message_string));
}



template <int dim>
void
FEValuesBase<dim>::TriaCellIterator::
get_interpolated_dof_values (const TrilinosWrappers::BlockVector &,
                             Vector<TrilinosScalar>              &) const
{
  Assert (false, ExcMessage (message_string));
}



template <int dim>
void
FEValuesBase<dim>::TriaCellIterator::
get_interpolated_dof_values (const TrilinosWrappers::MPI::Vector &,
                             Vector<TrilinosScalar>              &) const
{
  Assert (false, ExcMessage (message_string));
}



template <int dim>
void
FEValuesBase<dim>::TriaCellIterator::
get_interpolated_dof_values (const TrilinosWrappers::MPI::BlockVector &,
                             Vector<TrilinosScalar>                   &) const
{
  Assert (false, ExcMessage (message_string));
}

#endif




/* --------------------- FEValuesData ----------------- */


template <int dim>
void
FEValuesData<dim>::initialize (const unsigned int        n_quadrature_points,
			       const FiniteElement<dim> &fe,
			       const UpdateFlags         flags)
{
  this->update_flags = flags;

				   // initialize the table mapping
				   // from shape function number to
				   // the rows in the tables denoting
				   // its first non-zero
				   // component. with this also count
				   // the total number of non-zero
				   // components accumulated over all
				   // shape functions
  this->shape_function_to_row_table.resize (fe.dofs_per_cell);
  unsigned int row = 0;
  for (unsigned int i=0; i<fe.dofs_per_cell; ++i)
    {
      this->shape_function_to_row_table[i] = row;
      row += fe.n_nonzero_components (i);
    };
	 
  const unsigned int n_nonzero_shape_components = row;
  Assert (n_nonzero_shape_components >= fe.dofs_per_cell,
	  ExcInternalError());

				   // with the number of rows now
				   // known, initialize those fields
				   // that we will need to their
				   // correct size
  if (flags & update_values)
    this->shape_values.reinit(n_nonzero_shape_components,
			      n_quadrature_points);

  if (flags & update_gradients)
    this->shape_gradients.resize (n_nonzero_shape_components,
                                  std::vector<Tensor<1,dim> > (n_quadrature_points));

  if (flags & update_hessians)
    this->shape_hessians.resize (n_nonzero_shape_components,
                                        std::vector<Tensor<2,dim> > (n_quadrature_points));
  
  if (flags & update_quadrature_points)
    this->quadrature_points.resize(n_quadrature_points);

  if (flags & update_JxW_values)
    this->JxW_values.resize(n_quadrature_points);

  if (flags & update_jacobians)
    this->jacobians.resize(n_quadrature_points);

  if (flags & update_jacobian_grads)
    this->jacobian_grads.resize(n_quadrature_points);

  if (flags & update_inverse_jacobians)
    this->inverse_jacobians.resize(n_quadrature_points);

  if (flags & update_boundary_forms)
    this->boundary_forms.resize(n_quadrature_points);

  if (flags & update_normal_vectors)
    this->normal_vectors.resize(n_quadrature_points);

  if (flags & update_cell_JxW_values)
    this->cell_JxW_values.resize(n_quadrature_points);
}


/*------------------------------- FEValuesBase ---------------------------*/


template <int dim>
FEValuesBase<dim>::FEValuesBase (const unsigned int n_q_points,
				 const unsigned int dofs_per_cell,
				 const UpdateFlags flags,
				 const Mapping<dim>       &mapping,
				 const FiniteElement<dim> &fe)
		:
		n_quadrature_points (n_q_points),
		dofs_per_cell (dofs_per_cell),
		mapping(&mapping),
		fe(&fe),
		mapping_data(0),
		fe_data(0)
{
  this->update_flags = flags;
}



template <int dim>
FEValuesBase<dim>::~FEValuesBase ()
{
				   // delete those fields that were
				   // created by the mapping and
				   // finite element objects,
				   // respectively, but of which we
				   // have assumed ownership
  if (fe_data != 0)
    {
      typename Mapping<dim>::InternalDataBase *tmp1=fe_data;
      fe_data=0;
      delete tmp1;
    }

  if (mapping_data != 0)
    {
      typename Mapping<dim>::InternalDataBase *tmp1=mapping_data;
      mapping_data=0;
      delete tmp1;
    }
}



template <int dim>
template <class InputVector, typename number>
void FEValuesBase<dim>::get_function_values (
  const InputVector            &fe_function,
  std::vector<number> &values) const
{
  Assert (this->update_flags & update_values, ExcAccessToUninitializedField());
  Assert (fe->n_components() == 1,
	  ExcDimensionMismatch(fe->n_components(), 1));
  Assert (values.size() == n_quadrature_points,
	  ExcDimensionMismatch(values.size(), n_quadrature_points));
  Assert (present_cell.get() != 0,
	  ExcMessage ("FEValues object is not reinit'ed to any cell"));
  Assert (fe_function.size() == present_cell->n_dofs_for_dof_handler(),
	  ExcDimensionMismatch(fe_function.size(),
			       present_cell->n_dofs_for_dof_handler()));

				   // get function values of dofs
				   // on this cell
  Vector<typename InputVector::value_type> dof_values (dofs_per_cell);
  present_cell->get_interpolated_dof_values(fe_function, dof_values);

				   // initialize with zero
  std::fill_n (values.begin(), n_quadrature_points, 0);

				   // add up contributions of trial
				   // functions. note that here we
				   // deal with scalar finite
				   // elements, so no need to check
				   // for non-primitivity of shape
				   // functions
  for (unsigned int point=0; point<n_quadrature_points; ++point)
    for (unsigned int shape_func=0; shape_func<dofs_per_cell; ++shape_func)
      values[point] += (dof_values(shape_func) *
			this->shape_value(shape_func, point));
}



template <int dim>
template <class InputVector, typename number>
void FEValuesBase<dim>::get_function_values (
  const InputVector& fe_function,
  const VectorSlice<const std::vector<unsigned int> >& indices,
  std::vector<number> &values) const
{
  Assert (this->update_flags & update_values, ExcAccessToUninitializedField());
				   // This function fills a single
				   // component only
  Assert (fe->n_components() == 1,
	  ExcDimensionMismatch(fe->n_components(), 1));
				   // One index for each dof
  Assert (indices.size() == dofs_per_cell,
	  ExcDimensionMismatch(indices.size(), dofs_per_cell));
				   // This vector has one entry for
				   // each quadrature point
  Assert (values.size() == n_quadrature_points,
	  ExcDimensionMismatch(values.size(), n_quadrature_points));
  
				   // initialize with zero
  std::fill_n (values.begin(), n_quadrature_points, 0);
  
				   // add up contributions of trial
				   // functions. note that here we
				   // deal with scalar finite
				   // elements, so no need to check
				   // for non-primitivity of shape
				   // functions
  for (unsigned int point=0; point<n_quadrature_points; ++point)
    for (unsigned int shape_func=0; shape_func<dofs_per_cell; ++shape_func)
      values[point] += (fe_function(indices[shape_func]) *
			this->shape_value(shape_func, point));
}



template <int dim>
template <class InputVector, typename number>
void FEValuesBase<dim>::get_function_values (
  const InputVector&            fe_function,
  std::vector<Vector<number> >& values) const
{
//TODO: Find out how to do this assertion.  
				   // This vector must correspond to a
				   // complete discretization
//  Assert (fe_function.size() == present_cell->get_dof_handler().n_dofs(),
//	  ExcDimensionMismatch(fe_function.size(),
//			     present_cell->get_dof_handler().n_dofs()));
				   // One entry per quadrature point
  Assert (present_cell.get() != 0,
	  ExcMessage ("FEValues object is not reinit'ed to any cell"));
  Assert (values.size() == n_quadrature_points,
	  ExcDimensionMismatch(values.size(), n_quadrature_points));

  const unsigned int n_components = fe->n_components();
				   // Assert that we can write all
				   // components into the result
				   // vectors
  for (unsigned i=0;i<values.size();++i)
    Assert (values[i].size() == n_components,
	    ExcDimensionMismatch(values[i].size(), n_components));
  
  Assert (this->update_flags & update_values, ExcAccessToUninitializedField());
  Assert (fe_function.size() == present_cell->n_dofs_for_dof_handler(),
	  ExcDimensionMismatch(fe_function.size(), present_cell->n_dofs_for_dof_handler()));
    
				   // get function values of dofs
				   // on this cell
  Vector<typename InputVector::value_type> dof_values (dofs_per_cell);
  present_cell->get_interpolated_dof_values(fe_function, dof_values);
  
				   // initialize with zero
  for (unsigned i=0;i<values.size();++i)
    std::fill_n (values[i].begin(), values[i].size(), 0);

				   // add up contributions of trial
				   // functions. now check whether the
				   // shape function is primitive or
				   // not. if it is, then set its only
				   // non-zero component, otherwise
				   // loop over components
  for (unsigned int point=0; point<n_quadrature_points; ++point)
    for (unsigned int shape_func=0; shape_func<dofs_per_cell; ++shape_func)
      if (fe->is_primitive(shape_func))
	values[point](fe->system_to_component_index(shape_func).first)
	  += (dof_values(shape_func) * shape_value(shape_func, point));
      else
	for (unsigned int c=0; c<n_components; ++c)
	  values[point](c) += (dof_values(shape_func) *
			       shape_value_component(shape_func, point, c));
}



template <int dim>
template <class InputVector, typename number>
void FEValuesBase<dim>::get_function_values (
  const InputVector& fe_function,
  const VectorSlice<const std::vector<unsigned int> >& indices,
  std::vector<Vector<number> >& values) const
{
				   // One value per quadrature point
  Assert (n_quadrature_points == values.size(),
	  ExcDimensionMismatch(values.size(), n_quadrature_points));
  
  const unsigned int n_components = fe->n_components();
  
				   // Size of indices must be a
				   // multiple of dofs_per_cell such
				   // that an integer number of
				   // function values is generated in
				   // each point.
  Assert (indices.size() % dofs_per_cell == 0,
	  ExcNotMultiple(indices.size(), dofs_per_cell));

				   // The number of components of the
				   // result may be a multiple of the
				   // number of components of the
				   // finite element
  const unsigned int result_components = indices.size() * n_components / dofs_per_cell;
  
  for (unsigned i=0;i<values.size();++i)
    Assert (values[i].size() == result_components,
	    ExcDimensionMismatch(values[i].size(), result_components));

				   // If the result has more
				   // components than the finite
				   // element, we need this number for
				   // loops filling all components
  const unsigned int component_multiple = result_components / n_components;
  
  Assert (this->update_flags & update_values, ExcAccessToUninitializedField());
    
				   // initialize with zero
  for (unsigned i=0;i<values.size();++i)
    std::fill_n (values[i].begin(), values[i].size(), 0);

				   // add up contributions of trial
				   // functions. now check whether the
				   // shape function is primitive or
				   // not. if it is, then set its only
				   // non-zero component, otherwise
				   // loop over components
  for (unsigned int mc = 0; mc < component_multiple; ++mc)
    for (unsigned int point=0; point<n_quadrature_points; ++point)
      for (unsigned int shape_func=0; shape_func<dofs_per_cell; ++shape_func)
	if (fe->is_primitive(shape_func))
	  values[point](fe->system_to_component_index(shape_func).first
			+mc * n_components)
	    += (fe_function(indices[shape_func+mc*dofs_per_cell])
		* shape_value(shape_func, point));
	else
	  for (unsigned int c=0; c<n_components; ++c)
	    values[point](c+mc*n_components)
	      += (fe_function(indices[shape_func])
		  * shape_value_component(shape_func, point, c));
}



template <int dim>
template <class InputVector, typename number>
void FEValuesBase<dim>::get_function_values (
  const InputVector& fe_function,
  const VectorSlice<const std::vector<unsigned int> >& indices,
  std::vector<std::vector<number> >& values,
  bool quadrature_points_fastest) const
{
  const unsigned int n_components = fe->n_components();
  
				   // Size of indices must be a
				   // multiple of dofs_per_cell such
				   // that an integer number of
				   // function values is generated in
				   // each point.
  Assert (indices.size() % dofs_per_cell == 0,
	  ExcNotMultiple(indices.size(), dofs_per_cell));

				   // The number of components of the
				   // result may be a multiple of the
				   // number of components of the
				   // finite element
  const unsigned int result_components = indices.size() * n_components / dofs_per_cell;

				   // Check if the value argument is
				   // initialized to the correct sizes
  if (quadrature_points_fastest)
    {
      Assert (values.size() == result_components,
	      ExcDimensionMismatch(values.size(), result_components));
      for (unsigned i=0;i<values.size();++i)
	Assert (values[i].size() == n_quadrature_points,
		ExcDimensionMismatch(values[i].size(), n_quadrature_points));
    }
  else
    {
      Assert(values.size() == n_quadrature_points,
	     ExcDimensionMismatch(values.size(), n_quadrature_points));
      for (unsigned i=0;i<values.size();++i)
	Assert (values[i].size() == result_components,
		ExcDimensionMismatch(values[i].size(), result_components));
    }
  
				   // If the result has more
				   // components than the finite
				   // element, we need this number for
				   // loops filling all components
  const unsigned int component_multiple = result_components / n_components;
  
  Assert (this->update_flags & update_values, ExcAccessToUninitializedField());
    
				   // initialize with zero
  for (unsigned i=0;i<values.size();++i)
    std::fill_n (values[i].begin(), values[i].size(), 0);

				   // add up contributions of trial
				   // functions. now check whether the
				   // shape function is primitive or
				   // not. if it is, then set its only
				   // non-zero component, otherwise
				   // loop over components
  if (quadrature_points_fastest)
    for (unsigned int mc = 0; mc < component_multiple; ++mc)
      for (unsigned int point=0; point<n_quadrature_points; ++point)
	for (unsigned int shape_func=0; shape_func<dofs_per_cell; ++shape_func)
	  if (fe->is_primitive(shape_func))
	    values[fe->system_to_component_index(shape_func).first
		   +mc * n_components][point]
	      += (fe_function(indices[shape_func+mc*dofs_per_cell])
		  * shape_value(shape_func, point));
	  else
	    for (unsigned int c=0; c<n_components; ++c)
	      values[c+mc*n_components][point]
		+= (fe_function(indices[shape_func])
		    * shape_value_component(shape_func, point, c));
  else
    for (unsigned int mc = 0; mc < component_multiple; ++mc)
      for (unsigned int point=0; point<n_quadrature_points; ++point)
	for (unsigned int shape_func=0; shape_func<dofs_per_cell; ++shape_func)
	  if (fe->is_primitive(shape_func))
	    values[point][fe->system_to_component_index(shape_func).first
			  +mc * n_components]
	      += (fe_function(indices[shape_func+mc*dofs_per_cell])
		  * shape_value(shape_func, point));
	  else
	    for (unsigned int c=0; c<n_components; ++c)
	      values[point][c+mc*n_components]
		+= (fe_function(indices[shape_func])
		    * shape_value_component(shape_func, point, c));
}



template <int dim>
template <class InputVector>
void
FEValuesBase<dim>::get_function_gradients (
  const InputVector           &fe_function,
  std::vector<Tensor<1,dim> > &gradients) const
{
  Assert (this->update_flags & update_gradients, ExcAccessToUninitializedField());

  Assert (fe->n_components() == 1,
	  ExcDimensionMismatch(fe->n_components(), 1));
  Assert (gradients.size() == n_quadrature_points,
	  ExcDimensionMismatch(gradients.size(), n_quadrature_points));
  Assert (present_cell.get() != 0,
	  ExcMessage ("FEValues object is not reinit'ed to any cell"));
  Assert (fe_function.size() == present_cell->n_dofs_for_dof_handler(),
	  ExcDimensionMismatch(fe_function.size(),
			       present_cell->n_dofs_for_dof_handler()));

				   // get function values of dofs
				   // on this cell
  Vector<typename InputVector::value_type> dof_values (dofs_per_cell);
  present_cell->get_interpolated_dof_values(fe_function, dof_values);

				   // initialize with zero
  std::fill_n (gradients.begin(), n_quadrature_points, Tensor<1,dim>());

				   // add up contributions of trial
				   // functions. note that here we
				   // deal with scalar finite
				   // elements, so no need to check
				   // for non-primitivity of shape
				   // functions
  for (unsigned int point=0; point<n_quadrature_points; ++point)
    for (unsigned int shape_func=0; shape_func<dofs_per_cell; ++shape_func)
      {
	Tensor<1,dim> tmp = this->shape_grad(shape_func,point);
	tmp *= dof_values(shape_func);
	gradients[point] += tmp;
      };
}



template <int dim>
template <class InputVector>
void FEValuesBase<dim>::get_function_gradients (
  const InputVector& fe_function,
  const VectorSlice<const std::vector<unsigned int> >& indices,
  std::vector<Tensor<1,dim> > &values) const
{
  Assert (this->update_flags & update_gradients, ExcAccessToUninitializedField());
				   // This function fills a single
				   // component only
  Assert (fe->n_components() == 1,
	  ExcDimensionMismatch(fe->n_components(), 1));
				   // One index for each dof
  Assert (indices.size() == dofs_per_cell,
	  ExcDimensionMismatch(indices.size(), dofs_per_cell));
				   // This vector has one entry for
				   // each quadrature point
  Assert (values.size() == n_quadrature_points,
	  ExcDimensionMismatch(values.size(), n_quadrature_points));
  
				   // initialize with zero
  std::fill_n (values.begin(), n_quadrature_points, Tensor<1,dim>());
  
				   // add up contributions of trial
				   // functions. note that here we
				   // deal with scalar finite
				   // elements, so no need to check
				   // for non-primitivity of shape
				   // functions
  for (unsigned int point=0; point<n_quadrature_points; ++point)
    for (unsigned int shape_func=0; shape_func<dofs_per_cell; ++shape_func)
      values[point] += (fe_function(indices[shape_func]) *
			this->shape_grad(shape_func, point));
}




template <int dim>
template <class InputVector>
void
FEValuesBase<dim>::get_function_gradients (
  const InputVector                         &fe_function,
  std::vector<std::vector<Tensor<1,dim> > > &gradients) const
{
  Assert (gradients.size() == n_quadrature_points,
	  ExcDimensionMismatch(gradients.size(), n_quadrature_points));

  const unsigned int n_components = fe->n_components();
  for (unsigned i=0; i<gradients.size(); ++i)
    Assert (gradients[i].size() == n_components,
	    ExcDimensionMismatch(gradients[i].size(), n_components));

  Assert (this->update_flags & update_gradients, ExcAccessToUninitializedField());
  Assert (present_cell.get() != 0,
	  ExcMessage ("FEValues object is not reinit'ed to any cell"));
  Assert (fe_function.size() == present_cell->n_dofs_for_dof_handler(),
	  ExcDimensionMismatch(fe_function.size(),
			       present_cell->n_dofs_for_dof_handler()));

				   // get function values of dofs
				   // on this cell
  Vector<typename InputVector::value_type> dof_values (dofs_per_cell);
  present_cell->get_interpolated_dof_values(fe_function, dof_values);

				   // initialize with zero
  for (unsigned i=0;i<gradients.size();++i)
    std::fill_n (gradients[i].begin(), gradients[i].size(), Tensor<1,dim>());

				   // add up contributions of trial
				   // functions. now check whether the
				   // shape function is primitive or
				   // not. if it is, then set its only
				   // non-zero component, otherwise
				   // loop over components
  for (unsigned int point=0; point<n_quadrature_points; ++point)
    for (unsigned int shape_func=0; shape_func<dofs_per_cell; ++shape_func)
      if (fe->is_primitive (shape_func))
	{
	  Tensor<1,dim> tmp = this->shape_grad(shape_func,point);
	  tmp *= dof_values(shape_func);
	  gradients[point][fe->system_to_component_index(shape_func).first]
	    += tmp;
	}
      else
	for (unsigned int c=0; c<n_components; ++c)
	  {
	    Tensor<1,dim> tmp = this->shape_grad_component(shape_func,point,c);
	    tmp *= dof_values(shape_func);
	    gradients[point][c] += tmp;
	  }
}



template <int dim>
template <class InputVector>
void FEValuesBase<dim>::get_function_gradients (
  const InputVector& fe_function,
  const VectorSlice<const std::vector<unsigned int> >& indices,
  std::vector<std::vector<Tensor<1,dim> > >& values,
  bool quadrature_points_fastest) const
{
  const unsigned int n_components = fe->n_components();
  
				   // Size of indices must be a
				   // multiple of dofs_per_cell such
				   // that an integer number of
				   // function values is generated in
				   // each point.
  Assert (indices.size() % dofs_per_cell == 0,
	  ExcNotMultiple(indices.size(), dofs_per_cell));

				   // The number of components of the
				   // result may be a multiple of the
				   // number of components of the
				   // finite element
  const unsigned int result_components = indices.size() * n_components / dofs_per_cell;
  
				   // Check if the value argument is
				   // initialized to the correct sizes
  if (quadrature_points_fastest)
    {
      Assert (values.size() == result_components,
	      ExcDimensionMismatch(values.size(), result_components));
      for (unsigned i=0;i<values.size();++i)
	Assert (values[i].size() == n_quadrature_points,
		ExcDimensionMismatch(values[i].size(), n_quadrature_points));
    }
  else
    {
      Assert(values.size() == n_quadrature_points,
	     ExcDimensionMismatch(values.size(), n_quadrature_points));
      for (unsigned i=0;i<values.size();++i)
	Assert (values[i].size() == result_components,
		ExcDimensionMismatch(values[i].size(), result_components));
    }

				   // If the result has more
				   // components than the finite
				   // element, we need this number for
				   // loops filling all components
  const unsigned int component_multiple = result_components / n_components;
  
  Assert (this->update_flags & update_values, ExcAccessToUninitializedField());
    
				   // initialize with zero
  for (unsigned i=0;i<values.size();++i)
    std::fill_n (values[i].begin(), values[i].size(), Tensor<1,dim>());

				   // add up contributions of trial
				   // functions. now check whether the
				   // shape function is primitive or
				   // not. if it is, then set its only
				   // non-zero component, otherwise
				   // loop over components
  if (quadrature_points_fastest)
    for (unsigned int mc = 0; mc < component_multiple; ++mc)
      for (unsigned int point=0; point<n_quadrature_points; ++point)
	for (unsigned int shape_func=0; shape_func<dofs_per_cell; ++shape_func)
	  if (fe->is_primitive(shape_func))
	    values[fe->system_to_component_index(shape_func).first
		   +mc * n_components][point]
	      += fe_function(indices[shape_func+mc*dofs_per_cell])
	      * shape_grad(shape_func, point);
	  else
	    for (unsigned int c=0; c<n_components; ++c)
	      values[c][point] += (fe_function(indices[shape_func]) *
				   shape_grad_component(shape_func, point, c));
  else
    for (unsigned int mc = 0; mc < component_multiple; ++mc)
      for (unsigned int point=0; point<n_quadrature_points; ++point)
	for (unsigned int shape_func=0; shape_func<dofs_per_cell; ++shape_func)
	  if (fe->is_primitive(shape_func))
	    values[point][fe->system_to_component_index(shape_func).first
			  +mc * n_components]
	      += fe_function(indices[shape_func+mc*dofs_per_cell])
	      * shape_grad(shape_func, point);
	  else
	    for (unsigned int c=0; c<n_components; ++c)
	      values[point][c] += (fe_function(indices[shape_func]) *
				   shape_grad_component(shape_func, point, c));
}



template <int dim>
template <class InputVector>
void
FEValuesBase<dim>::
get_function_hessians (const InputVector           &fe_function,
			      std::vector<Tensor<2,dim> > &hessians) const
{
  Assert (fe->n_components() == 1,
	  ExcDimensionMismatch(fe->n_components(), 1));
  Assert (hessians.size() == n_quadrature_points,
	  ExcDimensionMismatch(hessians.size(), n_quadrature_points));
  Assert (this->update_flags & update_hessians, ExcAccessToUninitializedField());
  Assert (present_cell.get() != 0,
	  ExcMessage ("FEValues object is not reinit'ed to any cell"));
  Assert (fe_function.size() == present_cell->n_dofs_for_dof_handler(),
	  ExcDimensionMismatch(fe_function.size(),
			       present_cell->n_dofs_for_dof_handler()));

				   // get function values of dofs
				   // on this cell
  Vector<typename InputVector::value_type> dof_values (dofs_per_cell);
  present_cell->get_interpolated_dof_values(fe_function, dof_values);

				   // initialize with zero
  std::fill_n (hessians.begin(), n_quadrature_points, Tensor<2,dim>());

				   // add up contributions of trial
				   // functions. note that here we
				   // deal with scalar finite
				   // elements, so no need to check
				   // for non-primitivity of shape
				   // functions
  for (unsigned int point=0; point<n_quadrature_points; ++point)
    for (unsigned int shape_func=0; shape_func<dofs_per_cell; ++shape_func)
      {
	Tensor<2,dim> tmp = this->shape_hessian(shape_func,point);
	tmp *= dof_values(shape_func);
	hessians[point] += tmp;
      };
}



template <int dim>
template <class InputVector>
void FEValuesBase<dim>::get_function_hessians (
  const InputVector& fe_function,
  const VectorSlice<const std::vector<unsigned int> >& indices,
  std::vector<Tensor<2,dim> > &values) const
{
  Assert (this->update_flags & update_second_derivatives, ExcAccessToUninitializedField());
				   // This function fills a single
				   // component only
  Assert (fe->n_components() == 1,
	  ExcDimensionMismatch(fe->n_components(), 1));
				   // One index for each dof
  Assert (indices.size() == dofs_per_cell,
	  ExcDimensionMismatch(indices.size(), dofs_per_cell));
				   // This vector has one entry for
				   // each quadrature point
  Assert (values.size() == n_quadrature_points,
	  ExcDimensionMismatch(values.size(), n_quadrature_points));
  
				   // initialize with zero
  std::fill_n (values.begin(), n_quadrature_points, Tensor<2,dim>());
  
				   // add up contributions of trial
				   // functions. note that here we
				   // deal with scalar finite
				   // elements, so no need to check
				   // for non-primitivity of shape
				   // functions
  for (unsigned int point=0; point<n_quadrature_points; ++point)
    for (unsigned int shape_func=0; shape_func<dofs_per_cell; ++shape_func)
      values[point] += (fe_function(indices[shape_func]) *
			this->shape_hessian(shape_func, point));
}




template <int dim>
template <class InputVector>
void
FEValuesBase<dim>::
get_function_hessians (const InputVector                         &fe_function,
			      std::vector<std::vector<Tensor<2,dim> > > &second_derivs,
			      bool quadrature_points_fastest) const
{
  Assert (n_quadrature_points == second_derivs.size(),
	  ExcDimensionMismatch(second_derivs.size(), n_quadrature_points));

  const unsigned int n_components = fe->n_components();
  for (unsigned i=0;i<second_derivs.size();++i)
    Assert (second_derivs[i].size() == n_components,
	    ExcDimensionMismatch(second_derivs[i].size(), n_components));

  Assert (this->update_flags & update_hessians, ExcAccessToUninitializedField());
  Assert (present_cell.get() != 0,
	  ExcMessage ("FEValues object is not reinit'ed to any cell"));
  Assert (fe_function.size() == present_cell->n_dofs_for_dof_handler(),
	  ExcDimensionMismatch(fe_function.size(),
			       present_cell->n_dofs_for_dof_handler()));

				   // get function values of dofs
				   // on this cell
  Vector<typename InputVector::value_type> dof_values (dofs_per_cell);
  present_cell->get_interpolated_dof_values(fe_function, dof_values);

				   // initialize with zero
  for (unsigned i=0;i<second_derivs.size();++i)
    std::fill_n (second_derivs[i].begin(), second_derivs[i].size(), Tensor<2,dim>());

				   // add up contributions of trial
				   // functions
  if (quadrature_points_fastest)
    for (unsigned int point=0; point<n_quadrature_points; ++point)
      for (unsigned int shape_func=0; shape_func<dofs_per_cell; ++shape_func)
	if (fe->is_primitive(shape_func))
	  {
	    Tensor<2,dim> tmp(shape_hessian(shape_func,point));
	    tmp *= dof_values(shape_func);
	    second_derivs[fe->system_to_component_index(shape_func).first][point]
	      += tmp;
	  }
	else
	  for (unsigned int c=0; c<n_components; ++c)
	    {
	      Tensor<2,dim> tmp = this->shape_hessian_component(shape_func,point,c);
	      tmp *= dof_values(shape_func);
	      second_derivs[c][point] += tmp;
	    }
  else
    for (unsigned int point=0; point<n_quadrature_points; ++point)
      for (unsigned int shape_func=0; shape_func<dofs_per_cell; ++shape_func)
	if (fe->is_primitive(shape_func))
	  {
	    Tensor<2,dim> tmp(shape_hessian(shape_func,point));
	    tmp *= dof_values(shape_func);
	    second_derivs[point][fe->system_to_component_index(shape_func).first]
	      += tmp;
	  }
	else
	  for (unsigned int c=0; c<n_components; ++c)
	    {
	      Tensor<2,dim> tmp = this->shape_hessian_component(shape_func,point,c);
	      tmp *= dof_values(shape_func);
	      second_derivs[point][c] += tmp;
	    }
}



template <int dim>
template <class InputVector>
void FEValuesBase<dim>::get_function_hessians (
  const InputVector& fe_function,
  const VectorSlice<const std::vector<unsigned int> >& indices,
  std::vector<std::vector<Tensor<2,dim> > >& values,
  bool quadrature_points_fastest) const
{
  Assert (this->update_flags & update_second_derivatives, ExcAccessToUninitializedField());
    
  const unsigned int n_components = fe->n_components();
  
				   // Size of indices must be a
				   // multiple of dofs_per_cell such
				   // that an integer number of
				   // function values is generated in
				   // each point.
  Assert (indices.size() % dofs_per_cell == 0,
	  ExcNotMultiple(indices.size(), dofs_per_cell));

				   // The number of components of the
				   // result may be a multiple of the
				   // number of components of the
				   // finite element
  const unsigned int result_components = indices.size() * n_components / dofs_per_cell;
  
				   // Check if the value argument is
				   // initialized to the correct sizes
  if (quadrature_points_fastest)
    {
      Assert (values.size() == result_components,
	      ExcDimensionMismatch(values.size(), result_components));
      for (unsigned i=0;i<values.size();++i)
	Assert (values[i].size() == n_quadrature_points,
		ExcDimensionMismatch(values[i].size(), n_quadrature_points));
    }
  else
    {
      Assert(values.size() == n_quadrature_points,
	     ExcDimensionMismatch(values.size(), n_quadrature_points));
      for (unsigned i=0;i<values.size();++i)
	Assert (values[i].size() == result_components,
		ExcDimensionMismatch(values[i].size(), result_components));
    }

				   // If the result has more
				   // components than the finite
				   // element, we need this number for
				   // loops filling all components
  const unsigned int component_multiple = result_components / n_components;
  
				   // initialize with zero
  for (unsigned i=0;i<values.size();++i)
    std::fill_n (values[i].begin(), values[i].size(), Tensor<2,dim>());

				   // add up contributions of trial
				   // functions. now check whether the
				   // shape function is primitive or
				   // not. if it is, then set its only
				   // non-zero component, otherwise
				   // loop over components
  if (quadrature_points_fastest)
    for (unsigned int mc = 0; mc < component_multiple; ++mc)
      for (unsigned int point=0; point<n_quadrature_points; ++point)
	for (unsigned int shape_func=0; shape_func<dofs_per_cell; ++shape_func)
	  if (fe->is_primitive(shape_func))
	    values[fe->system_to_component_index(shape_func).first
		   +mc * n_components][point]
	      += fe_function(indices[shape_func+mc*dofs_per_cell])
	      * shape_hessian(shape_func, point);
	  else
	    for (unsigned int c=0; c<n_components; ++c)
	      values[c][point] += (fe_function(indices[shape_func]) *
				   shape_hessian_component(shape_func, point, c));
  else
    for (unsigned int mc = 0; mc < component_multiple; ++mc)
      for (unsigned int point=0; point<n_quadrature_points; ++point)
	for (unsigned int shape_func=0; shape_func<dofs_per_cell; ++shape_func)
	  if (fe->is_primitive(shape_func))
	    values[point][fe->system_to_component_index(shape_func).first
			  +mc * n_components]
	      += fe_function(indices[shape_func+mc*dofs_per_cell])
	      * shape_hessian(shape_func, point);
	  else
	    for (unsigned int c=0; c<n_components; ++c)
	      values[point][c] += (fe_function(indices[shape_func]) *
				   shape_hessian_component(shape_func, point, c));
}



template <int dim>
unsigned int
FEValuesBase<dim>::memory_consumption () const
{
  return (MemoryConsumption::memory_consumption (this->shape_values) +
	  MemoryConsumption::memory_consumption (this->shape_gradients) +
	  MemoryConsumption::memory_consumption (this->shape_hessians) +
	  MemoryConsumption::memory_consumption (this->JxW_values) +
	  MemoryConsumption::memory_consumption (this->jacobians) +
	  MemoryConsumption::memory_consumption (this->jacobian_grads) +
	  MemoryConsumption::memory_consumption (this->inverse_jacobians) +
	  MemoryConsumption::memory_consumption (this->quadrature_points) +
	  MemoryConsumption::memory_consumption (this->normal_vectors) +
	  MemoryConsumption::memory_consumption (this->boundary_forms) +
	  MemoryConsumption::memory_consumption (this->cell_JxW_values) +
	  sizeof(this->update_flags) +
	  MemoryConsumption::memory_consumption (n_quadrature_points) +
	  MemoryConsumption::memory_consumption (dofs_per_cell) +
	  MemoryConsumption::memory_consumption (mapping) +
	  MemoryConsumption::memory_consumption (fe) +
	  MemoryConsumption::memory_consumption (mapping_data) +
	  MemoryConsumption::memory_consumption (*mapping_data) +
	  MemoryConsumption::memory_consumption (fe_data) +
	  MemoryConsumption::memory_consumption (*fe_data) +
	  MemoryConsumption::memory_consumption (this->shape_function_to_row_table));
}



template <int dim>
UpdateFlags
FEValuesBase<dim>::compute_update_flags (const UpdateFlags update_flags) const
{
  
				   // first find out which objects
				   // need to be recomputed on each
				   // cell we visit. this we have to
				   // ask the finite element and mapping.
				   // elements are first since they
				   // might require update in mapping
  UpdateFlags flags = update_flags
		      | fe->update_once (update_flags)
		      | fe->update_each (update_flags);
  flags |= mapping->update_once (flags)
	   | mapping->update_each (flags);

  return flags;
}



/*------------------------------- FEValues -------------------------------*/


template <int dim>
const unsigned int FEValues<dim>::dimension;

template <int dim>
const unsigned int FEValues<dim>::integral_dimension;



template <int dim>
FEValues<dim>::FEValues (const Mapping<dim>       &mapping,
			 const FiniteElement<dim> &fe,
			 const Quadrature<dim>    &q,
			 const UpdateFlags         update_flags)
		:
		FEValuesBase<dim> (q.size(),
				   fe.dofs_per_cell,
				   update_default,
				   mapping,
				   fe),
  quadrature (q)
{
  initialize (update_flags);
}



template <int dim>
FEValues<dim>::FEValues (const FiniteElement<dim> &fe,
			 const Quadrature<dim>    &q,
			 const UpdateFlags         update_flags)
		:
		FEValuesBase<dim> (q.size(),
				   fe.dofs_per_cell,
				   update_default,
				   StaticMappingQ1<dim>::mapping,
				   fe),
  quadrature (q)
{
  Assert (DEAL_II_COMPAT_MAPPING, ExcCompatibility("mapping"));
  initialize (update_flags);
}



template <int dim>
void
FEValues<dim>::initialize (const UpdateFlags update_flags)
{
				   // you can't compute normal vectors
				   // on cells, only on faces
  Assert ((update_flags & update_normal_vectors) == false,
	  typename FEValuesBase<dim>::ExcInvalidUpdateFlag());

  const UpdateFlags flags = this->compute_update_flags (update_flags);
  
				   // then get objects into which the
				   // FE and the Mapping can store
				   // intermediate data used across
				   // calls to reinit
  this->mapping_data = this->mapping->get_data(flags, quadrature);
  this->fe_data      = this->fe->get_data(flags, *this->mapping, quadrature);

				   // set up objects within this class
  FEValuesData<dim>::initialize (this->n_quadrature_points, *this->fe, flags);
}



template <int dim>
void
FEValues<dim>::reinit (const typename DoFHandler<dim>::cell_iterator &cell)
{
				   // assert that the finite elements
				   // passed to the constructor and
				   // used by the DoFHandler used by
				   // this cell, are the same
  Assert (static_cast<const FiniteElementData<dim>&>(*this->fe) ==
	  static_cast<const FiniteElementData<dim>&>(cell->get_fe()),
	  typename FEValuesBase<dim>::ExcFEDontMatch());

                                   // set new cell. auto_ptr will take
                                   // care that old object gets
                                   // destroyed and also that this
                                   // object gets destroyed in the
                                   // destruction of this class
  this->present_cell.reset 
    (new typename FEValuesBase<dim>::template
     CellIterator<typename DoFHandler<dim>::cell_iterator> (cell));

                                   // this was the part of the work
                                   // that is dependent on the actual
                                   // data type of the iterator. now
                                   // pass on to the function doing
                                   // the real work.
  do_reinit ();
}



template <int dim>
void
FEValues<dim>::reinit (const typename hp::DoFHandler<dim>::cell_iterator &cell)
{
				   // assert that the finite elements
				   // passed to the constructor and
				   // used by the DoFHandler used by
				   // this cell, are the same
  Assert (static_cast<const FiniteElementData<dim>&>(*this->fe) ==
	  static_cast<const FiniteElementData<dim>&>(cell->get_fe()),
	  typename FEValuesBase<dim>::ExcFEDontMatch());

                                   // set new cell. auto_ptr will take
                                   // care that old object gets
                                   // destroyed and also that this
                                   // object gets destroyed in the
                                   // destruction of this class
  this->present_cell.reset 
    (new typename FEValuesBase<dim>::template
     CellIterator<typename hp::DoFHandler<dim>::cell_iterator> (cell));

                                   // this was the part of the work
                                   // that is dependent on the actual
                                   // data type of the iterator. now
                                   // pass on to the function doing
                                   // the real work.
  do_reinit ();
}



template <int dim>
void
FEValues<dim>::reinit (const typename MGDoFHandler<dim>::cell_iterator &cell)
{
				   // assert that the finite elements
				   // passed to the constructor and
				   // used by the DoFHandler used by
				   // this cell, are the same

//TODO: This was documented out ith the repository. Why?
  
//  Assert (static_cast<const FiniteElementData<dim>&>(*this->fe) ==
//	  static_cast<const FiniteElementData<dim>&>(cell->get_fe()),
//	  typename FEValuesBase<dim>::ExcFEDontMatch());

                                   // set new cell. auto_ptr will take
                                   // care that old object gets
                                   // destroyed and also that this
                                   // object gets destroyed in the
                                   // destruction of this class
  this->present_cell.reset 
    (new typename FEValuesBase<dim>::template
     CellIterator<typename MGDoFHandler<dim>::cell_iterator> (cell));

                                   // this was the part of the work
                                   // that is dependent on the actual
                                   // data type of the iterator. now
                                   // pass on to the function doing
                                   // the real work.
  do_reinit ();
}



template <int dim>
void FEValues<dim>::reinit (const typename Triangulation<dim>::cell_iterator &cell)
{
                                   // no FE in this cell, so no check
                                   // necessary here

                                   // set new cell. auto_ptr will take
                                   // care that old object gets
                                   // destroyed and also that this
                                   // object gets destroyed in the
                                   // destruction of this class
  this->present_cell.reset 
    (new typename FEValuesBase<dim>::TriaCellIterator (cell));
                                   // this was the part of the work
                                   // that is dependent on the actual
                                   // data type of the iterator. now
                                   // pass on to the function doing
                                   // the real work.
  do_reinit ();
}



template <int dim>
void FEValues<dim>::do_reinit ()
{
  this->get_mapping().fill_fe_values(*this->present_cell,
				     quadrature,
				     *this->mapping_data,
				     this->quadrature_points,
				     this->JxW_values,
				     this->jacobians,
				     this->jacobian_grads,
				     this->inverse_jacobians);
  
  this->get_fe().fill_fe_values(this->get_mapping(),
				*this->present_cell,
				quadrature,
				*this->mapping_data,
				*this->fe_data,
				*this);

  this->fe_data->clear_first_cell ();
  this->mapping_data->clear_first_cell ();
}



template <int dim>
unsigned int
FEValues<dim>::memory_consumption () const
{
  return (FEValuesBase<dim>::memory_consumption () +
	  MemoryConsumption::memory_consumption (quadrature));
}


/*------------------------------- FEFaceValuesBase --------------------------*/


template <int dim>
FEFaceValuesBase<dim>::FEFaceValuesBase (const unsigned int n_q_points,
					 const unsigned int dofs_per_cell,
					 const UpdateFlags,
					 const Mapping<dim> &mapping,      
					 const FiniteElement<dim> &fe,
					 const Quadrature<dim-1>& quadrature)
		:
		FEValuesBase<dim> (n_q_points,
				   dofs_per_cell,
				   update_default,
				   mapping,
				   fe),
                quadrature(quadrature)
{}



template <int dim>
const std::vector<Point<dim> > &
FEFaceValuesBase<dim>::get_normal_vectors () const
{
  Assert (this->update_flags & update_normal_vectors,
	  typename FEValuesBase<dim>::ExcAccessToUninitializedField());
  return this->normal_vectors;
}



template <int dim>
const std::vector<Tensor<1,dim> > &
FEFaceValuesBase<dim>::get_boundary_forms () const
{
  Assert (this->update_flags & update_boundary_forms,
	  typename FEValuesBase<dim>::ExcAccessToUninitializedField());
  return this->boundary_forms;
}



template <int dim>
unsigned int
FEFaceValuesBase<dim>::memory_consumption () const
{
  return (FEValuesBase<dim>::memory_consumption () +
	  MemoryConsumption::memory_consumption (quadrature));
}


/*------------------------------- FEFaceValues -------------------------------*/

template <int dim>
const unsigned int FEFaceValues<dim>::dimension;

template <int dim>
const unsigned int FEFaceValues<dim>::integral_dimension;


template <int dim>
FEFaceValues<dim>::FEFaceValues (const Mapping<dim>       &mapping,
				 const FiniteElement<dim> &fe,
				 const Quadrature<dim-1>  &quadrature,
				 const UpdateFlags         update_flags)
		:
		FEFaceValuesBase<dim> (quadrature.size(),
				       fe.dofs_per_cell,
				       update_flags,
				       mapping,
				       fe, quadrature)
{
  initialize (update_flags);
}



template <int dim>
FEFaceValues<dim>::FEFaceValues (const FiniteElement<dim> &fe,
				 const Quadrature<dim-1>  &quadrature,
				 const UpdateFlags         update_flags)
		:
		FEFaceValuesBase<dim> (quadrature.size(),
				       fe.dofs_per_cell,
				       update_flags,
				       StaticMappingQ1<dim>::mapping,
				       fe, quadrature)
{
  Assert (DEAL_II_COMPAT_MAPPING, ExcCompatibility("mapping"));
  initialize (update_flags);
}



template <int dim>
void
FEFaceValues<dim>::initialize (const UpdateFlags update_flags)
{
  const UpdateFlags flags = this->compute_update_flags (update_flags);
  
				   // then get objects into which the
				   // FE and the Mapping can store
				   // intermediate data used across
				   // calls to reinit
  this->mapping_data = this->mapping->get_face_data(flags, this->quadrature);
  this->fe_data      = this->fe->get_face_data(flags, *this->mapping, this->quadrature);

				   // set up objects within this class
  FEValuesData<dim>::initialize(this->n_quadrature_points, *this->fe, flags);
}



template <int dim>
void FEFaceValues<dim>::reinit (const typename DoFHandler<dim>::cell_iterator &cell,
				const unsigned int              face_no)
{
				   // assert that the finite elements
				   // passed to the constructor and
				   // used by the DoFHandler used by
				   // this cell, are the same
//   Assert (static_cast<const FiniteElementData<dim>&>(*this->fe) ==
// 	  static_cast<const FiniteElementData<dim>&>(cell->get_dof_handler().get_fe()),
// 	  typename FEValuesBase<dim>::ExcFEDontMatch());

  Assert (face_no < GeometryInfo<dim>::faces_per_cell,
	  ExcIndexRange (face_no, 0, GeometryInfo<dim>::faces_per_cell));
  
                                   // set new cell. auto_ptr will take
                                   // care that old object gets
                                   // destroyed and also that this
                                   // object gets destroyed in the
                                   // destruction of this class
  this->present_cell.reset 
    (new typename FEValuesBase<dim>::template
     CellIterator<typename DoFHandler<dim>::cell_iterator> (cell));
  
                                   // this was the part of the work
                                   // that is dependent on the actual
                                   // data type of the iterator. now
                                   // pass on to the function doing
                                   // the real work.
  do_reinit (face_no);
}



template <int dim>
void FEFaceValues<dim>::reinit (const typename hp::DoFHandler<dim>::cell_iterator &cell,
				const unsigned int              face_no)
{
				   // assert that the finite elements
				   // passed to the constructor and
				   // used by the DoFHandler used by
				   // this cell, are the same
  Assert (static_cast<const FiniteElementData<dim>&>(*this->fe) ==
	  static_cast<const FiniteElementData<dim>&>(
	    cell->get_dof_handler().get_fe()[cell->active_fe_index ()]),
	  typename FEValuesBase<dim>::ExcFEDontMatch());

  Assert (face_no < GeometryInfo<dim>::faces_per_cell,
	  ExcIndexRange (face_no, 0, GeometryInfo<dim>::faces_per_cell));
  
                                   // set new cell. auto_ptr will take
                                   // care that old object gets
                                   // destroyed and also that this
                                   // object gets destroyed in the
                                   // destruction of this class
  this->present_cell.reset 
    (new typename FEValuesBase<dim>::template
     CellIterator<typename hp::DoFHandler<dim>::cell_iterator> (cell));

                                   // this was the part of the work
                                   // that is dependent on the actual
                                   // data type of the iterator. now
                                   // pass on to the function doing
                                   // the real work.
  do_reinit (face_no);
}


template <int dim>
void FEFaceValues<dim>::reinit (const typename MGDoFHandler<dim>::cell_iterator &cell,
				const unsigned int              face_no)
{
				   // assert that the finite elements
				   // passed to the constructor and
				   // used by the DoFHandler used by
				   // this cell, are the same
  Assert (static_cast<const FiniteElementData<dim>&>(*this->fe) ==
	  static_cast<const FiniteElementData<dim>&>(cell->get_dof_handler().get_fe()),
	  typename FEValuesBase<dim>::ExcFEDontMatch());

  Assert (face_no < GeometryInfo<dim>::faces_per_cell,
	  ExcIndexRange (face_no, 0, GeometryInfo<dim>::faces_per_cell));
  
                                   // set new cell. auto_ptr will take
                                   // care that old object gets
                                   // destroyed and also that this
                                   // object gets destroyed in the
                                   // destruction of this class
  this->present_cell.reset 
    (new typename FEValuesBase<dim>::template
     CellIterator<typename MGDoFHandler<dim>::cell_iterator> (cell));

                                   // this was the part of the work
                                   // that is dependent on the actual
                                   // data type of the iterator. now
                                   // pass on to the function doing
                                   // the real work.
  do_reinit (face_no);
}


template <int dim>
void FEFaceValues<dim>::reinit (const typename Triangulation<dim>::cell_iterator &cell,
				const unsigned int              face_no)
{
  Assert (face_no < GeometryInfo<dim>::faces_per_cell,
	  ExcIndexRange (face_no, 0, GeometryInfo<dim>::faces_per_cell));

                                   // set new cell. auto_ptr will take
                                   // care that old object gets
                                   // destroyed and also that this
                                   // object gets destroyed in the
                                   // destruction of this class
  this->present_cell.reset 
    (new typename FEValuesBase<dim>::TriaCellIterator (cell));

                                   // this was the part of the work
                                   // that is dependent on the actual
                                   // data type of the iterator. now
                                   // pass on to the function doing
                                   // the real work.
  do_reinit (face_no);
}



template <int dim>
void FEFaceValues<dim>::do_reinit (const unsigned int face_no)
{
				   // first of all, set the
				   // present_face_index (if
				   // available)
  const typename Triangulation<dim>::cell_iterator cell=*this->present_cell;
  this->present_face_index=cell->face_index(face_no);
  
  this->get_mapping().fill_fe_face_values(*this->present_cell, face_no,
					  this->quadrature,
					  *this->mapping_data,
					  this->quadrature_points,
					  this->JxW_values,
					  this->boundary_forms,
					  this->normal_vectors,
					  this->cell_JxW_values);
  
  this->get_fe().fill_fe_face_values(this->get_mapping(),
				     *this->present_cell, face_no,
				     this->quadrature,
				     *this->mapping_data,
				     *this->fe_data,
				     *this);

  this->fe_data->clear_first_cell ();
  this->mapping_data->clear_first_cell ();
}


/*------------------------------- FESubFaceValues -------------------------------*/


template <int dim>
const unsigned int FESubfaceValues<dim>::dimension;

template <int dim>
const unsigned int FESubfaceValues<dim>::integral_dimension;



template <int dim>
FESubfaceValues<dim>::FESubfaceValues (const Mapping<dim>       &mapping,
				       const FiniteElement<dim> &fe,
				       const Quadrature<dim-1>  &quadrature,
				       const UpdateFlags         update_flags)
		:
		FEFaceValuesBase<dim> (quadrature.size(),
				       fe.dofs_per_cell,
				       update_flags,
				       mapping,
				       fe, quadrature)
{
  initialize (update_flags);
}



template <int dim>
FESubfaceValues<dim>::FESubfaceValues (const FiniteElement<dim> &fe,
				       const Quadrature<dim-1>  &quadrature,
				       const UpdateFlags         update_flags)
		:
		FEFaceValuesBase<dim> (quadrature.size(),
				       fe.dofs_per_cell,
				       update_flags,
				       StaticMappingQ1<dim>::mapping,
				       fe, quadrature)
{
  Assert (DEAL_II_COMPAT_MAPPING, ExcCompatibility("mapping"));
  initialize (update_flags);
}



template <int dim>
void
FESubfaceValues<dim>::initialize (const UpdateFlags update_flags)
{
  const UpdateFlags flags = this->compute_update_flags (update_flags);
  
				   // then get objects into which the
				   // FE and the Mapping can store
				   // intermediate data used across
				   // calls to reinit
  this->mapping_data = this->mapping->get_subface_data(flags, this->quadrature);
  this->fe_data      = this->fe->get_subface_data(flags,
						  *this->mapping,
						  this->quadrature);

				   // set up objects within this class
  FEValuesData<dim>::initialize(this->n_quadrature_points, *this->fe, flags);
}



template <int dim>
void FESubfaceValues<dim>::reinit (const typename DoFHandler<dim>::cell_iterator &cell,
				   const unsigned int         face_no,
				   const unsigned int         subface_no)
{
				   // assert that the finite elements
				   // passed to the constructor and
				   // used by the DoFHandler used by
				   // this cell, are the same
//   Assert (static_cast<const FiniteElementData<dim>&>(*this->fe) ==
// 	  static_cast<const FiniteElementData<dim>&>(cell->get_dof_handler().get_fe()),
// 	  typename FEValuesBase<dim>::ExcFEDontMatch());
  Assert (face_no < GeometryInfo<dim>::faces_per_cell,
	  ExcIndexRange (face_no, 0, GeometryInfo<dim>::faces_per_cell));
				   // We would like to check for
				   // subface_no < cell->face(face_no)->n_children(),
				   // but unfortunately the current
				   // function is also called for
				   // faces without children (see
				   // tests/fe/mapping.cc). Therefore,
				   // we must use following workaround
				   // of two separate assertions
  Assert (cell->face(face_no)->has_children() ||
	  subface_no < GeometryInfo<dim>::max_children_per_face,
	  ExcIndexRange (subface_no, 0, GeometryInfo<dim>::max_children_per_face));
  Assert (!cell->face(face_no)->has_children() ||
	  subface_no < cell->face(face_no)->number_of_children(),
	  ExcIndexRange (subface_no, 0, cell->face(face_no)->number_of_children()));

  Assert (cell->has_children() == false,
          ExcMessage ("You can't use subface data for cells that are "
                      "already refined. Iterate over their children "
                      "instead in these cases."));

                                   // set new cell. auto_ptr will take
                                   // care that old object gets
                                   // destroyed and also that this
                                   // object gets destroyed in the
                                   // destruction of this class
  this->present_cell.reset 
    (new typename FEValuesBase<dim>::template
     CellIterator<typename DoFHandler<dim>::cell_iterator> (cell));

                                   // this was the part of the work
                                   // that is dependent on the actual
                                   // data type of the iterator. now
                                   // pass on to the function doing
                                   // the real work.
  do_reinit (face_no, subface_no);
}



template <int dim>
void FESubfaceValues<dim>::reinit (const typename hp::DoFHandler<dim>::cell_iterator &cell,
				   const unsigned int         face_no,
				   const unsigned int         subface_no)
{
				   // assert that the finite elements
				   // passed to the constructor and
				   // used by the hp::DoFHandler used by
				   // this cell, are the same
  Assert (static_cast<const FiniteElementData<dim>&>(*this->fe) ==
	  static_cast<const FiniteElementData<dim>&>(
	    cell->get_dof_handler().get_fe()[cell->active_fe_index ()]),
	  typename FEValuesBase<dim>::ExcFEDontMatch());
  Assert (face_no < GeometryInfo<dim>::faces_per_cell,
	  ExcIndexRange (face_no, 0, GeometryInfo<dim>::faces_per_cell));
  Assert (subface_no < cell->face(face_no)->number_of_children(),
	  ExcIndexRange (subface_no, 0, cell->face(face_no)->number_of_children()));
  Assert (cell->has_children() == false,
          ExcMessage ("You can't use subface data for cells that are "
                      "already refined. Iterate over their children "
                      "instead in these cases."));

                                   // set new cell. auto_ptr will take
                                   // care that old object gets
                                   // destroyed and also that this
                                   // object gets destroyed in the
                                   // destruction of this class
  this->present_cell.reset
    (new typename FEValuesBase<dim>::template
     CellIterator<typename hp::DoFHandler<dim>::cell_iterator> (cell));

                                   // this was the part of the work
                                   // that is dependent on the actual
                                   // data type of the iterator. now
                                   // pass on to the function doing
                                   // the real work.
  do_reinit (face_no, subface_no);
}

  
template <int dim>
void FESubfaceValues<dim>::reinit (const typename MGDoFHandler<dim>::cell_iterator &cell,
				   const unsigned int         face_no,
				   const unsigned int         subface_no)
{
   Assert (static_cast<const FiniteElementData<dim>&>(*this->fe) ==
	   static_cast<const FiniteElementData<dim>&>(cell->get_dof_handler().get_fe()),
	   typename FEValuesBase<dim>::ExcFEDontMatch());
  Assert (face_no < GeometryInfo<dim>::faces_per_cell,
	  ExcIndexRange (face_no, 0, GeometryInfo<dim>::faces_per_cell));
  Assert (subface_no < cell->face(face_no)->number_of_children(),
	  ExcIndexRange (subface_no, 0, cell->face(face_no)->number_of_children()));
  Assert (cell->has_children() == false,
          ExcMessage ("You can't use subface data for cells that are "
                      "already refined. Iterate over their children "
                      "instead in these cases."));

                                   // set new cell. auto_ptr will take
                                   // care that old object gets
                                   // destroyed and also that this
                                   // object gets destroyed in the
                                   // destruction of this class
  this->present_cell.reset 
    (new typename FEValuesBase<dim>::template
     CellIterator<typename MGDoFHandler<dim>::cell_iterator> (cell));

                                   // this was the part of the work
                                   // that is dependent on the actual
                                   // data type of the iterator. now
                                   // pass on to the function doing
                                   // the real work.
  do_reinit (face_no, subface_no);
}



template <int dim>
void FESubfaceValues<dim>::reinit (const typename Triangulation<dim>::cell_iterator &cell,
				   const unsigned int         face_no,
				   const unsigned int         subface_no)
{
  Assert (face_no < GeometryInfo<dim>::faces_per_cell,
	  ExcIndexRange (face_no, 0, GeometryInfo<dim>::faces_per_cell));
  Assert (subface_no < cell->face(face_no)->n_children(),
	  ExcIndexRange (subface_no, 0, cell->face(face_no)->n_children()));
  
                                   // set new cell. auto_ptr will take
                                   // care that old object gets
                                   // destroyed and also that this
                                   // object gets destroyed in the
                                   // destruction of this class
  this->present_cell.reset 
    (new typename FEValuesBase<dim>::TriaCellIterator (cell));

                                   // this was the part of the work
                                   // that is dependent on the actual
                                   // data type of the iterator. now
                                   // pass on to the function doing
                                   // the real work.
  do_reinit (face_no, subface_no);
}



template <int dim>
void FESubfaceValues<dim>::do_reinit (const unsigned int face_no,
                                      const unsigned int subface_no)
{
				   // first of all, set the present_face_index
				   // (if available)
  const typename Triangulation<dim>::cell_iterator cell=*this->present_cell;

  if (!cell->face(face_no)->has_children())
				     // no subfaces at all, so set
				     // present_face_index to this face rather
				     // than any subface
    this->present_face_index=cell->face_index(face_no);
  else
    if (dim!=3)
      this->present_face_index=cell->face(face_no)->child_index(subface_no);
    else
      {
					 // this is the same logic we use in
					 // cell->neighbor_child_on_subface(). See
					 // there for an explanation of the
					 // different cases
	unsigned int subface_index=numbers::invalid_unsigned_int;
	switch (cell->subface_case(face_no))
	  {
	    case internal::SubfaceCase<3>::case_x:
	    case internal::SubfaceCase<3>::case_y:
	    case internal::SubfaceCase<3>::case_xy:
		  subface_index=cell->face(face_no)->child_index(subface_no);
		  break;
	    case internal::SubfaceCase<3>::case_x1y2y:
	    case internal::SubfaceCase<3>::case_y1x2x:
		  subface_index=cell->face(face_no)->child(subface_no/2)->child_index(subface_no%2);
		  break;
	    case internal::SubfaceCase<3>::case_x1y:
	    case internal::SubfaceCase<3>::case_y1x:
		  switch (subface_no)
		    {
		      case 0:
		      case 1:
			    subface_index=cell->face(face_no)->child(0)->child_index(subface_no);
			    break;
		      case 2:
			    subface_index=cell->face(face_no)->child_index(1);
			    break;
		      default:
			    Assert(false, ExcInternalError());
		    }
		  break;
	    case internal::SubfaceCase<3>::case_x2y:
	    case internal::SubfaceCase<3>::case_y2x:
		  switch (subface_no)
		    {
		      case 0:
			    subface_index=cell->face(face_no)->child_index(0);
			    break;
		      case 1:
		      case 2:
			    subface_index=cell->face(face_no)->child(1)->child_index(subface_no-1);
			    break;
		      default:
			    Assert(false, ExcInternalError());
		    }
		  break;
	    default:
		  Assert(false, ExcInternalError());
		  break;
	  }
	Assert(subface_index!=numbers::invalid_unsigned_int,
	       ExcInternalError());
	this->present_face_index=subface_index;
      }
  
				   // now ask the mapping and the finite element
				   // to do the actual work
  this->get_mapping().fill_fe_subface_values(*this->present_cell,
                                             face_no, subface_no,
					     this->quadrature,
					     *this->mapping_data,
					     this->quadrature_points,
					     this->JxW_values,
					     this->boundary_forms,
					     this->normal_vectors,
					     this->cell_JxW_values);
  
  this->get_fe().fill_fe_subface_values(this->get_mapping(),
					*this->present_cell,
                                        face_no, subface_no,
					this->quadrature,
					*this->mapping_data,
					*this->fe_data,
					*this);

  this->fe_data->clear_first_cell ();
  this->mapping_data->clear_first_cell ();
}


/*------------------------------- Explicit Instantiations -------------*/

template class FEValuesData<deal_II_dimension>;
template class FEValuesBase<deal_II_dimension>;
template class FEValues<deal_II_dimension>;
template class FEValuesBase<deal_II_dimension>::
  CellIterator<DoFHandler<deal_II_dimension>::cell_iterator>;
template class FEValuesBase<deal_II_dimension>::
  CellIterator<MGDoFHandler<deal_II_dimension>::cell_iterator>;

#if deal_II_dimension >= 2
template class FEFaceValuesBase<deal_II_dimension>;
template class FEFaceValues<deal_II_dimension>;
template class FESubfaceValues<deal_II_dimension>;
#endif


//---------------------------------------------------------------------------

// Instantiations are in a different file using the macro IN for the vector type.
// This way, we avoid code reduplication

#include "fe_values.inst"

DEAL_II_NAMESPACE_CLOSE
