// ---------------------------------------------------------------------
// $Id$
//
// Copyright (C) 2011 - 2014 by the deal.II authors
//
// This file is part of the deal.II library.
//
// The deal.II library is free software; you can use it, redistribute
// it, and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full text of the license can be found in the file LICENSE at
// the top level of the deal.II distribution.
//
// ---------------------------------------------------------------------


#ifndef __deal2__matrix_free_evaluated_geometry_h
#define __deal2__matrix_free_evaluated_geometry_h


#include <deal.II/base/config.h>
#include <deal.II/base/exceptions.h>
#include <deal.II/base/subscriptor.h>
#include <deal.II/base/vectorization.h>
#include <deal.II/base/aligned_vector.h>
#include <deal.II/matrix_free/shape_info.h>
#include <deal.II/matrix_free/mapping_info.h>
#include <deal.II/fe/fe_values.h>


DEAL_II_NAMESPACE_OPEN


/**
 * The class makes FEEvaluation with the mapping information generated by
 * FEValues.
 */
template <int dim, typename Number=double>
class EvaluatedGeometry : public Subscriptor
{
public:
  /**
   * Constructor, similar to FEValues.
   */
  EvaluatedGeometry (const Mapping<dim> &mapping,
                     const FiniteElement<dim> &fe,
                     const Quadrature<1> &quadrature,
                     const UpdateFlags update_flags);

  /**
   * Constructor. Instead of providing a mapping, use MappingQ1.
   */
  EvaluatedGeometry (const FiniteElement<dim> &fe,
                     const Quadrature<1> &quadrature,
                     const UpdateFlags update_flags);

  /**
   * Initialize with the given cell iterator.
   */
  template <typename ITERATOR>
  void reinit(ITERATOR &cell);

  /**
   * Return a vector of inverse transpose Jacobians. For compatibility with
   * FEEvaluation, it returns tensors of vectorized arrays, even though all
   * components are equal.
   */
  const AlignedVector<Tensor<2,dim,VectorizedArray<Number> > >&
  get_inverse_jacobians() const;

  /**
   * Return a vector of quadrature weights times the Jacobian determinant
   * (JxW). For compatibility with FEEvaluation, it returns tensors of
   * vectorized arrays, even though all components are equal.
   */
  const AlignedVector<VectorizedArray<Number> >&
  get_JxW_values() const;

  /**
   * Return a vector of quadrature points in real space on the given
   * cell. For compatibility with FEEvaluation, it returns tensors of
   * vectorized arrays, even though all components are equal.
   */
  const AlignedVector<Point<dim,VectorizedArray<Number> > >&
  get_quadrature_points() const;

  /**
   * Return a vector of quadrature points in real space on the given
   * cell. For compatibility with FEEvaluation, it returns tensors of
   * vectorized arrays, even though all components are equal.
   */
  const AlignedVector<Tensor<1,dim,VectorizedArray<Number> > >&
  get_normal_vectors() const;

  /**
   * Return a reference to 1D quadrature underlying this object.
   */
  const Quadrature<1>&
  get_quadrature () const;

private:
  /**
   * An underlying FEValues object that performs the (scalar) evaluation.
   */
  FEValues<dim> fe_values;

  /**
   * Get 1D quadrature formula to be used for reinitializing shape info.
   */
  const Quadrature<1> quadrature_1d;

  /**
   * Inverse Jacobians, stored in vectorized array form.
   */
  AlignedVector<Tensor<2,dim,VectorizedArray<Number> > > inverse_jacobians;

  /**
   * Stored Jacobian determinants and quadrature weights
   */
  AlignedVector<VectorizedArray<Number> > jxw_values;

  /**
   * Stored quadrature points
   */
  AlignedVector<Point<dim,VectorizedArray<Number> > > quadrature_points;

  /**
   * Stored normal vectors (for face integration)
   */
  AlignedVector<Tensor<1,dim,VectorizedArray<Number> > > normal_vectors;
};


/*----------------------- Inline functions ----------------------------------*/

template <int dim, typename Number>
inline
EvaluatedGeometry<dim,Number>::EvaluatedGeometry (const Mapping<dim> &mapping,
                                                  const FiniteElement<dim> &fe,
                                                  const Quadrature<1> &quadrature,
                                                  const UpdateFlags update_flags)
  :
  fe_values(mapping, fe, Quadrature<dim>(quadrature),
            internal::MatrixFreeFunctions::MappingInfo<dim,Number>::compute_update_flags(update_flags)),
  quadrature_1d(quadrature),
  inverse_jacobians(fe_values.get_quadrature().size()),
  jxw_values(fe_values.get_quadrature().size()),
  quadrature_points(fe_values.get_quadrature().size()),
  normal_vectors(fe_values.get_quadrature().size())
{
  Assert(!(fe_values.get_update_flags() & update_jacobian_grads),
         ExcNotImplemented());
}



template <int dim, typename Number>
inline
EvaluatedGeometry<dim,Number>::EvaluatedGeometry (const FiniteElement<dim> &fe,
                                                  const Quadrature<1> &quadrature,
                                                  const UpdateFlags update_flags)
  :
  fe_values(fe, Quadrature<dim>(quadrature),
            internal::MatrixFreeFunctions::MappingInfo<dim,Number>::compute_update_flags(update_flags)),
  quadrature_1d(quadrature),
  inverse_jacobians(fe_values.get_quadrature().size()),
  jxw_values(fe_values.get_quadrature().size()),
  quadrature_points(fe_values.get_quadrature().size()),
  normal_vectors(fe_values.get_quadrature().size())
{
  Assert(!(fe_values.get_update_flags() & update_jacobian_grads),
         ExcNotImplemented());
}



template <int dim, typename Number>
template <typename ITERATOR>
inline
void
EvaluatedGeometry<dim,Number>::reinit(ITERATOR &cell)
{
  fe_values.reinit(cell);
  for (unsigned int q=0; q<fe_values.get_quadrature().size(); ++q)
    {
      if (fe_values.get_update_flags() & update_inverse_jacobians)
        for (unsigned int d=0; d<dim; ++d)
          for (unsigned int e=0; e<dim; ++e)
            inverse_jacobians[q][d][e] = fe_values.inverse_jacobian(q)[e][d];
      if (fe_values.get_update_flags() & update_quadrature_points)
        for (unsigned int d=0; d<dim; ++d)
          quadrature_points[q][d] = fe_values.quadrature_point(q)[d];
      if (fe_values.get_update_flags() & update_normal_vectors)
        for (unsigned int d=0; d<dim; ++d)
          normal_vectors[q][d] = fe_values.normal_vector(q)[d];
      if (fe_values.get_update_flags() & update_JxW_values)
        jxw_values[q] = fe_values.JxW(q);
    }
}



template <int dim, typename Number>
inline
const AlignedVector<Tensor<2,dim,VectorizedArray<Number> > >&
EvaluatedGeometry<dim,Number>::get_inverse_jacobians() const
{
  return inverse_jacobians;
}



template <int dim, typename Number>
inline
const AlignedVector<Tensor<1,dim,VectorizedArray<Number> > >&
EvaluatedGeometry<dim,Number>::get_normal_vectors() const
{
  return normal_vectors;
}



template <int dim, typename Number>
inline
const AlignedVector<Point<dim,VectorizedArray<Number> > >&
EvaluatedGeometry<dim,Number>::get_quadrature_points() const
{
  return quadrature_points;
}



template <int dim, typename Number>
inline
const AlignedVector<VectorizedArray<Number> >&
EvaluatedGeometry<dim,Number>::get_JxW_values() const
{
  return jxw_values;
}



template <int dim, typename Number>
inline
const Quadrature<1>&
EvaluatedGeometry<dim,Number>::get_quadrature() const
{
  return quadrature_1d;
}


#ifndef DOXYGEN


#endif  // ifndef DOXYGEN


DEAL_II_NAMESPACE_CLOSE

#endif
