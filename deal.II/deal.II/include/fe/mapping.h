//---------------------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------
#ifndef __deal2__mapping_h
#define __deal2__mapping_h


#include <base/config.h>
#include <base/tensor.h>
#include <base/vector_slice.h>
#include <grid/tria.h>
#include <dofs/dof_handler.h>
#include <fe/fe_update_flags.h>

#include <cmath>

DEAL_II_NAMESPACE_OPEN

template <int dim> class Quadrature;
template <int dim, int spacedim> class FEValuesData;
template <int dim, int spacedim> class FEValuesBase;
template <int dim, int spacedim> class FEValues;
template <int dim, int spacedim> class FEFaceValues;
template <int dim, int spacedim> class FESubfaceValues;

//TODO: Offset in transform functions should be replaced by initializing VectorSlice correctly


/**
 * Abstract base class for mapping classes.
 *
 * The interface for filling the tables of FEValues is provided.
 * Everything else has to happen in derived classes.
 *
 * The following paragraph applies to the implementation of
 * FEValues. Usage of the class is as follows: first, call the
 * functions @p update_once and @p update_each with the update
 * flags you need. This includes the flags needed by the
 * FiniteElement. Then call <tt>get_*_data</tt> and with the or'd
 * results.  This will initialize and return some internal data
 * structures.  On the first cell, call <tt>fill_fe_*_values</tt> with the
 * result of @p update_once. Finally, on each cell, use
 * <tt>fill_fe_*_values</tt> with the result of @p update_each to compute
 * values for a special cell.
 *
 * A hint to implementators: no function except the two functions
 * @p update_once and @p update_each may add any flags.
 * 
 * For more information about the <tt>spacedim</tt> template parameter
 * check the documentation of FiniteElement or the one of
 * Triangulation.
 *
 * @ingroup mapping
 * @author Guido Kanschat, Ralf Hartmann 2000, 2001
 */
template <int dim, int spacedim=dim>
class Mapping : public Subscriptor
{
  public:
    
				     /**
				      * Virtual destructor.
				      */
    virtual ~Mapping ();
    
				     /**
				      * Transforms the point @p p on
				      * the unit cell to the point
				      * @p p_real on the real cell
				      * @p cell and returns @p p_real.
				      */
    virtual Point<spacedim>
    transform_unit_to_real_cell (
      const typename Triangulation<dim,spacedim>::cell_iterator &cell,
      const Point<dim>                                 &p) const = 0;
    
				     /**
				      * Transforms the point @p p on
				      * the real cell to the point
				      * @p p_unit on the unit cell
				      * @p cell and returns @p p_unit.
				      */
    virtual Point<dim>
    transform_real_to_unit_cell (
      const typename Triangulation<dim,spacedim>::cell_iterator &cell,
      const Point<spacedim>                            &p) const = 0;
    
				     /**
				      * Base class for internal data
				      * of finite element and mapping
				      * objects. The internal
				      * mechanism is that upon
				      * construction of a @p FEValues
				      * objects, it asks the mapping
				      * and finite element classes
				      * that are to be used to
				      * allocate memory for their own
				      * purpose in which they may
				      * store data that only needs to
				      * be computed once. For example,
				      * most finite elements will
				      * store the values of the shape
				      * functions at the quadrature
				      * points in this object, since
				      * they do not change from cell
				      * to cell and only need to be
				      * computed once. Since different
				      * @p FEValues objects using
				      * different quadrature rules
				      * might access the same finite
				      * element object at the same
				      * time, it is necessary to
				      * create one such object per
				      * @p FEValues object. Ownership
				      * of this object is then
				      * transferred to the
				      * @p FEValues object, but a
				      * pointer to this object is
				      * passed to the finite element
				      * object every time it shall
				      * compute some data so that it
				      * has access to the precomputed
				      * values stored there.
				      */
    class InternalDataBase: public Subscriptor
    {
      private:
					 /**
					  * Copy constructor forbidden.
					  */
        InternalDataBase (const InternalDataBase&);

      public:
					 /**
					  * Constructor. Sets
					  * @p UpdateFlags to
					  * @p update_default and
					  * @p first_cell to @p true.
					  */
        InternalDataBase ();
	
					 /**
					  * Virtual destructor for
					  * derived classes
					  */
	virtual ~InternalDataBase ();

	                                 /**
					  * Values updated by the constructor or
					  * by reinit.
					  */
	UpdateFlags          update_flags;
	
					 /**
					  * Values computed by
					  * constructor.
					  */
	UpdateFlags          update_once;

					 /**
					  * Values updated on each
					  * cell by reinit.
					  */
	UpdateFlags          update_each;

					 /**
					  * If <tt>first_cell==true</tt>
					  * this function returns
					  * @p update_flags,
					  * i.e. <tt>update_once|update_each</tt>.
					  * If <tt>first_cell==false</tt>
					  * it returns
					  * @p update_each.
					  */
	UpdateFlags  current_update_flags() const;

                                         /**
                                          * Return whether we are
                                          * presently initializing
                                          * data for the first
                                          * cell. The value of the
                                          * field this function is
                                          * returning is set to
                                          * @p true in the
                                          * constructor, and cleared
                                          * by the @p FEValues class
                                          * after the first cell has
                                          * been initialized.
                                          *
                                          * This function is used to
                                          * determine whether we need
                                          * to use the @p update_once
                                          * flags for computing data,
                                          * or whether we can use the
                                          * @p update_each flags.
                                          */
        bool is_first_cell () const;

                                         /**
                                          * Set the @p first_cell
                                          * flag to @p false. Used by
                                          * the @p FEValues class to
                                          * indicate that we have
                                          * already done the work on
                                          * the first cell.
                                          */
        virtual void clear_first_cell ();
        
					 /**
					  * Return an estimate (in
					  * bytes) or the memory
					  * consumption of this
					  * object.
					  */
	virtual unsigned int memory_consumption () const;
	
					 /**
					  * The positions of the
					  * mapped (generalized)
					  * support points.
					  */
	std::vector<Point<spacedim> > support_point_values;
	
					 /*
					  * The Jacobian of the
					  * transformation in the
					  * (generalized) support
					  * points.
					  */
	std::vector<Tensor<2,spacedim> > support_point_gradients;
	
					 /*
					  * The inverse of the
					  * Jacobian of the
					  * transformation in the
					  * (generalized) support
					  * points.
					  */
	std::vector<Tensor<2,spacedim> > support_point_inverse_gradients;
	
	
      private:
                                         /**
                                          * The value returned by
                                          * @p is_first_cell.
                                          */
        bool first_cell;
    };
    
				     /**
				      * Transform a field of covariant
				      * vectors. The covariant
				      * transformation multiplies a
				      * vector from the right with the
				      * inverse of the Jacobian matrix
				      * of the transformation from
				      * unit to real space
				      * cell. Alternatively, this is
				      * equivalent to a multiplication
				      * from the left with the
				      * transpose of the inverse
				      * matrix.
				      *
				      * The list of arguments is as follows:
				      * we transform as many elements in the
				      * @p input field, starting from @p offset,
				      * as there are elements in @p
				      * output. The @p input array may hold
				      * more elements than are needed (some
				      * finite element classes use this for a
				      * denser storage of data), but it needs
				      * to have at least
				      * <tt>output.size()</tt> elements
				      * starting with element @p offset.
				      */
    virtual
    void
    transform_covariant (const VectorSlice<const std::vector<Tensor<1,dim> > > input,
                         const unsigned int                                    offset,
                         VectorSlice<std::vector<Tensor<1,spacedim> > >        output,
			 const InternalDataBase &internal) const = 0;

                                     /**
                                      * Transform a set of matrices
                                      * covariantly, i.e. multiply
                                      * each function in the input
                                      * range by the Jacobian matrices
                                      * at the different quadrature
                                      * points from the left.
                                      *
                                      * The same applies as to the
                                      * function above regarding input
                                      * and output arguments.
                                      */
    virtual
    void
    transform_covariant (const VectorSlice<const std::vector<Tensor<2,dim> > > input,
                         const unsigned int                 offset,
			 VectorSlice<std::vector<Tensor<2,spacedim> > >      output,
			 const InternalDataBase &internal) const = 0;
    
				     /**
				      * Transform a field of
				      * contravariant vectors. The
				      * contravariant transformation
				      * multiplies a vector from the
				      * left with the Jacobian matrix
				      * of the transformation from
				      * unit to real space cell.
				      * 
				      * The list of arguments is as follows:
				      * we transform as many elements in the
				      * @p input field, starting from @p offset,
				      * as there are elements in @p
				      * output. The @p input array may hold
				      * more elements than are needed (some
				      * finite element classes use this for a
				      * denser storage of data), but it needs
				      * to have at least
				      * <tt>output.size()</tt> elements
				      * starting with element @p offset.
				      */
    virtual
    void
    transform_contravariant (const VectorSlice<const std::vector<Tensor<1,spacedim> > > input,
                             const unsigned int                 offset,
			     VectorSlice<std::vector<Tensor<1,spacedim> > >      output,
			     const typename Mapping<dim,spacedim>::InternalDataBase &internal) const = 0;

                                     /**
                                      * Transform a set of matrices
                                      * contravariantly, i.e. multiply
                                      * each function in the input
                                      * range by the inverse Jacobian
                                      * matrices at the different
                                      * quadrature points from the
                                      * right. Note that here it is no
                                      * more equivalent to
                                      * multiplication with the
                                      * transpose of the inverse
                                      * matrices from the left, unless
                                      * the matrices to be multiplied
                                      * with are symmetric.
                                      *
                                      * The same applies as to the
                                      * function above regarding input
                                      * and output arguments.
                                      */
    
    virtual void
    transform_contravariant (const VectorSlice<const std::vector<Tensor<2,spacedim> > > intput,
                             const unsigned int                 offset,
			     const VectorSlice<std::vector<Tensor<2,spacedim> > > output,
			     const typename Mapping<dim,spacedim>::InternalDataBase &internal) const = 0;

				     /**
				      * The transformed (generalized)
				      * support point.
				      */
    const Point<spacedim>& support_point_value(
      const unsigned int index,
      const typename Mapping<dim,spacedim>::InternalDataBase &internal) const;
    
				     /**
				      * The Jacobian
				      * matrix of the transformation
				      * in the (generalized) support
				      * point.
				      */
    const Tensor<2,spacedim>& support_point_gradient(
      const unsigned int index,
      const typename Mapping<dim,spacedim>::InternalDataBase &internal) const;
    
				     /**
				      * The inverse Jacobian
				      * matrix of the transformation
				      * in the (generalized) support
				      * point.
				      */
    const Tensor<2,spacedim>& support_point_inverse_gradient(
      const unsigned int index,
      const typename Mapping<dim,spacedim>::InternalDataBase &internal) const;
    
                                     /**
                                      * Return a pointer to a copy of the
                                      * present object. The caller of this
                                      * copy then assumes ownership of it.
                                      *
                                      * Since one can't create
                                      * objects of class Mapping, this
                                      * function of course has to be
                                      * implemented by derived classes.
                                      *
                                      * This function is mainly used by the
                                      * hp::MappingCollection class.
                                      */
    virtual
    Mapping<dim,spacedim> * clone () const = 0;
    
				     /**
				      * Exception
				      */
    DeclException0 (ExcInvalidData);

  private:
    
				     /**
				      * Indicate fields to be updated
				      * in the constructor of
				      * FEValues. Especially,
				      * fields not asked for by
				      * FEValues, but computed
				      * for efficiency reasons will be
				      * notified here.
				      *
				      * See @ref UpdateFlagsEssay.
				      */
    virtual UpdateFlags update_once (const UpdateFlags) const = 0;
    
				     /**
				      * The same as update_once(),
				      * but for the flags to be updated for
				      * each grid cell.
				      *
				      * See @ref UpdateFlagsEssay.
				      */
    virtual UpdateFlags update_each (const UpdateFlags) const = 0;

				     /**
				      * Prepare internal data
				      * structures and fill in values
				      * independent of the cell.
				      */
    virtual InternalDataBase*
    get_data (const UpdateFlags,
	      const Quadrature<dim>& quadrature) const = 0;

				     /**
				      * Prepare internal data
				      * structure for transformation
				      * of faces and fill in values
				      * independent of the cell.
				      */
    virtual InternalDataBase*
    get_face_data (const UpdateFlags flags,
		   const Quadrature<dim-1>& quadrature) const = 0;
    
				     /**
				      * Prepare internal data
				      * structure for transformation
				      * of children of faces and fill
				      * in values independent of the
				      * cell.
				      */
    virtual InternalDataBase*
    get_subface_data (const UpdateFlags flags,
		      const Quadrature<dim-1>& quadrature) const = 0;


				     /**
				      * Fill the transformation fields
				      * of @p FEValues.  Given a grid
				      * cell and the quadrature points
				      * on the unit cell, it computes
				      * all values specified by
				      * @p flags. The arrays to be
				      * filled have to have the
				      * correct size.
				      *
				      * Values are split into two
				      * groups: first,
				      * @p quadrature_points and
				      * @p JxW_values are
				      * filled with the quadrature
				      * rule transformed to the
				      * cell in physical space.
				      *
				      * The second group contains the
				      * matrices needed to transform
				      * vector-valued functions,
				      * namely
				      * @p jacobians,
				      * the derivatives
				      * @p jacobian_grads,
				      * and the inverse operations in
				      * @p inverse_jacobians.
				      */
/*     virtual void */
/*     fill_fe_values (const typename Triangulation<dim,spacedim>::cell_iterator &cell, */
/* 		    const Quadrature<dim>                         &quadrature, */
/* 		    InternalDataBase                              &internal, */
/* 		    std::vector<Point<spacedim> >                 &quadrature_points, */
/* 		    std::vector<double>                           &JxW_values) const = 0; */

                                      /** The function above adjusted
				       * with the variable
				       * cell_normal_vectors for the
				       * case of codimension 1
				       */
    virtual void
    fill_fe_values (const typename Triangulation<dim,spacedim>::cell_iterator &cell,
		    const Quadrature<dim>                         &quadrature,
		    InternalDataBase                              &internal,
		    std::vector<Point<spacedim> >                 &quadrature_points,
		    std::vector<double>                           &JxW_values,
		    std::vector<Tensor<2,spacedim> >              &jacobians,
		    std::vector<Tensor<3,spacedim> >              &jacobian_grads,
		    std::vector<Tensor<2,spacedim> >              &inverse_jacobians,
		    std::vector<Point<spacedim> >                 &cell_normal_vectors
	           ) const=0;



				     /**
				      * Performs the same as @p fill_fe_values
				      * on a face.
				      * Additionally, @p boundary_form and
				      * @p normal_vectors can be
				      * computed on surfaces. The
				      * boundary form is the vector
				      * product of the image of
				      * coordinate vectors on the
				      * surface of the unit
				      * cell. It is a
				      * vector normal to the surface,
				      * pointing outwards and having
				      * the length of the surface
				      * element.
				      * Therefore, it is more economic
				      * to use the boundary form
				      * instead of the product of the
				      * unit normal and the
				      * transformed quadrature weight.
				      */
    virtual void
    fill_fe_face_values (const typename Triangulation<dim,spacedim>::cell_iterator &cell,
			 const unsigned int                                        face_no,
			 const Quadrature<dim-1>                                   &quadrature,
			 InternalDataBase                                          &internal,
			 std::vector<Point<dim> >                                  &quadrature_points,
			 std::vector<double>                                       &JxW_values,
			 std::vector<Tensor<1,dim> >                               &boundary_form,
			 std::vector<Point<spacedim> >                             &normal_vectors,
			 std::vector<double>                                       &cell_JxW_values) const = 0;

				     /**
				      * See above.
				      */
    virtual void
    fill_fe_subface_values (const typename Triangulation<dim,spacedim>::cell_iterator &cell,
			    const unsigned int                        face_no,
			    const unsigned int                        sub_no,
			    const Quadrature<dim-1>                  &quadrature,
			    InternalDataBase                         &internal,
			    std::vector<Point<dim> >        &quadrature_points,
			    std::vector<double>                      &JxW_values,
			    std::vector<Tensor<1,dim> >     &boundary_form,
			    std::vector<Point<spacedim> >        &normal_vectors,
			    std::vector<double>             &cell_JxW_values) const = 0;

				     /**
				      * Give class @p FEValues access
				      * to the private <tt>get_...data</tt>
				      * and <tt>fill_fe_...values</tt>
				      * functions.
				      */
  friend class FEValuesBase<dim,spacedim>;
  friend class FEValues<dim,spacedim>;
  friend class FEFaceValues<dim,spacedim>;
  friend class FESubfaceValues<dim,spacedim>;
};


/* ------------------------- inline functions ------------------------- */


template <int dim, int spacedim>
inline
UpdateFlags
Mapping<dim,spacedim>::InternalDataBase::current_update_flags () const
{
  if (first_cell)
    {
      Assert (update_flags==(update_once|update_each),
              ExcInternalError());
      return update_flags;
    }
  else
    return update_each;
}



template <int dim, int spacedim>
inline
bool
Mapping<dim,spacedim>::InternalDataBase::is_first_cell () const
{
  return first_cell;
}



template <int dim, int spacedim>
inline
void
Mapping<dim,spacedim>::InternalDataBase::clear_first_cell ()
{
  first_cell = false;
}



template <int dim, int spacedim>
inline
const Point<spacedim>&
Mapping<dim,spacedim>::support_point_value(
  const unsigned int index,
  const typename Mapping<dim,spacedim>::InternalDataBase& internal) const
{
  AssertIndexRange(index, internal.support_point_values.size());
  return internal.support_point_values[index];  
}


template <int dim, int spacedim>
inline
const Tensor<2,spacedim>&
Mapping<dim,spacedim>::support_point_gradient(
  const unsigned int index,
  const typename Mapping<dim,spacedim>::InternalDataBase& internal) const
{
  AssertIndexRange(index, internal.support_point_gradients.size());
  return internal.support_point_gradients[index];
}


template <int dim, int spacedim>
inline
const Tensor<2,spacedim>&
Mapping<dim,spacedim>::support_point_inverse_gradient(
  const unsigned int index,
  const typename Mapping<dim,spacedim>::InternalDataBase& internal) const
{
  AssertIndexRange(index, internal.support_point_inverse_gradients.size());
  return internal.support_point_inverse_gradients[index];
}


DEAL_II_NAMESPACE_CLOSE

#endif
