//----------------------------  quadrature.h  ---------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 by the deal authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  quadrature.h  ---------------------------
#ifndef __deal2__quadrature_h
#define __deal2__quadrature_h


#include <base/config.h>
#include <base/point.h>
#include <base/subscriptor.h>
#include <vector>


/**
 * Base class for quadrature formulae in arbitrary dimensions. This class
 * stores quadrature points and weights on the unit line [0,1], unit
 * square [0,1]x[0,1], etc.
 *
 * There are a number of derived classes, denoting concrete
 * integration formulae. Their names names prefixed by @p{Q}. By now,
 * there are several Newton-Cotes formulae, @ref{QMidpoint},
 * @ref{QTrapez} and @ref{QSimpson}, as well as N-point Gauss formulae
 * @p{QGaussN}. The names refer to the one-dimensional formulae. The
 * schemes for higher dimensions are tensor products of
 * these. Therefore, a three-dimensional @ref{QGauss5} formula has 125
 * quadrature points.
 *
 * @sect2{Mathematical background}
 * For each quadrature formula we denote by @p{m}, the maximal degree of
 * polynomials integrated exactly. This number is given in the
 * documentation of each formula. The order of the integration error
 * is @p{m+1}, that is, the error is the size of the cell two the @p{m+1}
 * by the Bramble-Hilbert Lemma. The number @p{m} is to be found in the
 * documentation of each concrete formula. For the optimal formulae
 * @p{QGaussN} we have $m = 2N-1$. The tensor product formulae are
 * exact on tensor product polynomials of degree @p{m} in each space
 * direction, but they are still only of @p{m+1}st order.
 *
 * @sect2{Implementation details}
 * Most integration formulae in more than one space dimension are
 * tensor products of quadrature formulae in one space dimension, or
 * more generally the tensor product of a formula in @p{(dim-1)}
 * dimensions and one in one dimension. There is a special constructor
 * to generate a quadrature formula from two others.  For example, the
 * @p{QGauss2<dim>} formulae includes $2^dim$ quadrature points in @p{dim}
 * dimensions but is still exact for polynomials of degree 3 and its
 * order of integration is 4.
 *
 * For some programs it is necessary to have a quadrature object for
 * faces.  These programs fail to link if compiled for only one space
 * dimension, since there quadrature rules for faces just don't make
 * no sense. In order to allow these programs to be linked anyway, for
 * class @p{Quadrature<0>} all functions are provided in the
 * @p{quadrature.cc} file, but they will throw exceptions if actually
 * called. The only function which is allowed to be called is the
 * constructor taking one integer, which in this case ignores its
 * parameter, and of course the destructor. Besides this, it is
 * necessary to provide a class @p{Point<0>} to make the compiler
 * happy. This class also does nothing.
 *
 * @author Wolfgang Bangerth, 1998, 1999, 2000
 */
template <int dim>
class Quadrature : public Subscriptor
{
  public:
				     /**
				      * Define a typedef for a
				      * quadrature that acts on an
				      * object of one dimension
				      * less. For cells, this would
				      * then be a face quadrature.
				      */
    typedef Quadrature<dim-1> SubQuadrature;
    
				     /**
				      * Number of quadrature points.
				      */
    const unsigned int n_quadrature_points;

				     /**
				      * Constructor.
				      */
    Quadrature (const unsigned int n_quadrature_points);

				     /**
				      * Build this quadrature formula
				      * as the tensor product of a
				      * formula in a dimension one
				      * less than the present and a
				      * formula in one dimension.
				      *
				      * @p{SubQuadrature<dim>::type}
				      * expands to
				      * @p{Quadrature<dim-1>}.
				      */
    Quadrature (const SubQuadrature &,
		const Quadrature<1> &);
    
				     /**
				      * Construct a quadrature formula
				      * from given vectors of
				      * quadrature points (which
				      * should really be in the unit
				      * cell) and the corresponding
				      * weights. You will want to have
				      * the weights sum up to one, but
				      * this is not checked.
				      */
    Quadrature (const std::vector<Point<dim> > &points,
		const std::vector<double>      &weights);

				     /**
				      * Construct a dummy quadrature
				      * formula from a list of points,
				      * with weights set to
				      * infinity. The resulting object
				      * is therefore not meant to
				      * actually perform integrations,
				      * but rather to be used with
				      * @ref{FEValues} objects in
				      * order to find the position of
				      * some points (the quadrature
				      * points in this object) on the
				      * transformed cell in real
				      * space.
				      */
    Quadrature (const std::vector<Point<dim> > &points);

				     /**
				      * Constructor for a one-point
				      * quadrature. Sets the weight of
				      * this point to one.
				      */
    Quadrature (const Point<dim> &point);
    
				     /**
				      * Virtual destructor.
				      */
    virtual ~Quadrature ();
    
				     /**
				      * Return the @p{i}th quadrature
				      * point.
				      */
    const Point<dim> & point (const unsigned int i) const;

				     /**
				      * Return a reference to the
				      * whole array of quadrature
				      * points.
				      */
    const std::vector<Point<dim> > & get_points () const;
    
				     /**
				      * Return the weight of the @p{i}th
				      * quadrature point.
				      */
    double weight (const unsigned int i) const;

				     /**
				      * Return a reference to the whole array
				      * of weights.
				      */
    const std::vector<double> & get_weights () const;

    				     /**
				      * Determine an estimate for
				      * the memory consumption (in
				      * bytes) of this
				      * object.
				      */
    unsigned int memory_consumption () const;

  protected:
				     /**
				      * List of quadrature points. To be filled
				      * by the constructors of derived classes.
				      */
    std::vector<Point<dim> > quadrature_points;

				     /**
				      * List of weights of the quadrature points.
				      * To be filled by the constructors of
				      * derived classes.
				      */
    std::vector<double>      weights;
};



/**
 * Quadrature formula constructed by iteration of another quadrature formula in
 * each direction. In more than one space dimension, the resulting quadrature
 * formula is constructed in the usual way by building the tensor product of
 * the respective iterated quadrature formula in one space dimension.
 *
 * In one space dimension, the given base formula is copied and scaled onto
 * a given number of subintervals of length @p{1/n_copies}. If the quadrature
 * formula uses both end points of the unit interval, then in the interior
 * of the iterated quadrature formula there would be quadrature points which
 * are used twice; we merge them into one with a weight which is the sum
 * of the weights of the left- and the rightmost quadrature point.
 *
 * Since all dimensions higher than one are built up by tensor products of
 * one dimensional and @p{dim-1} dimensional quadrature formulae, the
 * argument given to the constructor needs to be a quadrature formula in
 * one space dimension, rather than in @p{dim} dimensions.
 *
 * The aim of this class is to provide a
 * low order formula, where the error constant can be tuned by
 * increasing the number of quadrature points. This is useful in
 * integrating non-differentiable functions on cells.
 *
 * @author Wolfgang Bangerth 1999
 */
template <int dim>
class QIterated : public Quadrature<dim>
{
  public:
				     /**
				      * Constructor. Iterate the given
				      * quadrature formula @p{n_copies} times in
				      * each direction.
				      */
    QIterated (const Quadrature<1> &base_quadrature,
	       const unsigned int   n_copies);

				     /**
				      * Exception
				      */
    DeclException0 (ExcSumOfWeightsNotOne);
				     /**
				      * Exception
				      */
    DeclException0 (ExcInvalidQuadratureFormula);
				     /**
				      * Exception
				      */
    DeclException1 (ExcInvalidNumberOfCopies,
		    int,
		    << "The numbers of copies (" << arg1
		    << ") of the quadrature formula is not valid.");
    
  private:
				     /**
				      * Check whether the given
				      * quadrature formula has quadrature
				      * points at the left and right end points
				      * of the interval.
				      */
    static bool
    uses_both_endpoints (const Quadrature<1> &base_quadrature);
};



/**
 *  This class is a helper class to facilitate the usage of quadrature formulae
 *  on faces or subfaces of cells. It computes the locations of quadrature
 *  points on the unit cell from a quadrature object for a mannifold of
 *  one dimension less than that of the cell and the number of the face.
 *  For example, giving the Simpson rule in one dimension and using the
 *  @p{project_to_face} function with face number 1, the returned points will
 *  be $(1,0)$, $(1,0.5)$ and $(1,1)$. Note that faces have an orientation,
 *  so when projecting to face 3, you will get $(0,0)$, $(0,0.5)$ and $(0,1)$,
 *  which is in clockwise sense, while for face 1 the points were in
 *  counterclockwise sense.
 *
 *  For the projection to subfaces (i.e. to the children of a face of the
 *  unit cell), the same applies as above. Note the order in which the
 *  children of a face are numbered, which in two dimensions coincides
 *  with the orientation of the face.
 *  
 *  The different functions are grouped into a common class to avoid putting
 *  them into global namespace (and to make documentation easier, since
 *  presently the documentation tool can only handle classes, not global
 *  functions). However, since they have no local data, all functions are
 *  declared @p{static} and can be called without creating an object of this
 *  class.
 *
 *  For the 3d case, you should note that the orientation of faces is even
 *  more intricate than for two dimensions. Quadrature formulae are projected
 *  upon the faces in their standard orientation, not to the inside or outside
 *  of the hexahedron! Refer to the documentation of the @p{Triangulation} class
 *  for a description of the orientation of the different faces.
 */
template <int dim>
class QProjector
{
  public:
				     /**
				      * Define a typedef for a
				      * quadrature that acts on an
				      * object of one dimension
				      * less. For cells, this would
				      * then be a face quadrature.
				      */
    typedef Quadrature<dim-1> SubQuadrature;
    
				     /**
				      * Compute the quadrature points
				      * on the cell if the given
				      * quadrature formula is used on
				      * face @p{face_no}. For further
				      * details, see the general doc
				      * for this class.
				      */
    static void project_to_face (const SubQuadrature &quadrature,
				 const unsigned int      face_no,
				 std::vector<Point<dim> > &q_points);

    				     /**
				      * Compute the quadrature points
				      * on the cell if the given
				      * quadrature formula is used on
				      * face @p{face_no}, subface
				      * number @p{subface_no}. For
				      * further details, see the
				      * general doc for this class.
				      */
    static void project_to_subface (const SubQuadrature &quadrature,
				    const unsigned int   face_no,
				    const unsigned int   subface_no,
				    std::vector<Point<dim> > &q_points);

				     /**
				      * Take a face quadrature formula
				      * and generate a cell quadrature
				      * formula from it where the
				      * quadrature points of the given
				      * argument are projected on all
				      * faces.
				      *
				      * The weights of the new rule
				      * are replications of the
				      * original weights. This is not
				      * a proper handling, in that the
				      * sum of weights does not equal
				      * one, but it is consistent with
				      * the use of this function,
				      * namely to generate sets of
				      * face quadrature points on a
				      * cell, one set of which will
				      * then be selected at each
				      * time. This is used in the
				      * @ref{FEFaceValues} class,
				      * where we initialize the
				      * values, derivatives, etc on
				      * all faces at once, while
				      * selecting the data of one
				      * particular face only happens
				      * later.
				      */
    static Quadrature<dim>
    project_to_all_faces (const SubQuadrature &quadrature);

				     /**
				      * This function is alike the
				      * previous one, but projects the
				      * given face quadrature formula
				      * to the subfaces of a cell,
				      * i.e. to the children of the
				      * faces of the unit cell.
				      */
    static Quadrature<dim>
    project_to_all_subfaces (const SubQuadrature &quadrature);

				     /**
				      * Project a give quadrature
				      * formula to a child of a
				      * cell. You may want to use this
				      * function in case you want to
				      * extend an integral only over
				      * the area which a potential
				      * child would occupy. The child
				      * numbering is the same as the
				      * children would be numbered
				      * upon refinement of the cell.
				      *
				      * As integration using this
				      * quadrature formula now only
				      * extends over a fraction of the
				      * cell, the weights of the
				      * resulting object are scaled by
				      * @p{1./GeometryInfo<dim>::children_per_cell}.
				      */
    static
    Quadrature<dim>
    project_to_child (const Quadrature<dim>  &quadrature,
		      const unsigned int      child_no);
};



/* -------------- declaration of explicit specializations ------------- */

template <>
Quadrature<0>::Quadrature (const unsigned int);
template <>
Quadrature<0>::Quadrature (const Quadrature<-1> &,
			   const Quadrature<1> &);
template <>
Quadrature<0>::~Quadrature ();
template <>
Quadrature<1>::Quadrature (const Quadrature<0> &,
			   const Quadrature<1> &);
template <>
const Point<0> & Quadrature<0>::point (const unsigned int) const;
template <>
const std::vector<Point<0> > & Quadrature<0>::get_points () const;
template <>
double Quadrature<0>::weight (const unsigned int) const;
template <>
const std::vector<double> & Quadrature<0>::get_weights () const;

template <>
void
QProjector<1>::project_to_face (const Quadrature<0> &,
                                const unsigned int,
                                std::vector<Point<1> > &);
template <>
void
QProjector<2>::project_to_face (const Quadrature<1>      &quadrature,
                                const unsigned int        face_no,
                                std::vector<Point<2> >   &q_points);
template <>
void
QProjector<3>::project_to_face (const Quadrature<2>    &quadrature,
                                const unsigned int      face_no,
                                std::vector<Point<3> > &q_points);

template <>
Quadrature<1>
QProjector<1>::project_to_all_faces (const Quadrature<0> &quadrature);


template <>
void
QProjector<1>::project_to_subface (const Quadrature<0> &,
                                   const unsigned int,
                                   const unsigned int,
                                   std::vector<Point<1> > &);
template <>
void
QProjector<2>::project_to_subface (const Quadrature<1>    &quadrature,
                                   const unsigned int      face_no,
                                   const unsigned int      subface_no,
                                   std::vector<Point<2> > &q_points);
template <>
void
QProjector<3>::project_to_subface (const Quadrature<2>    &quadrature,
                                   const unsigned int      face_no,
                                   const unsigned int      subface_no,
                                   std::vector<Point<3> > &q_points);

template <>
Quadrature<1>
QProjector<1>::project_to_all_subfaces (const Quadrature<0> &quadrature);


template <>
bool
QIterated<1>::uses_both_endpoints (const Quadrature<1> &base_quadrature);

template <>
QIterated<1>::QIterated (const Quadrature<1> &base_quadrature,
			 const unsigned int   n_copies);



#endif
