//----------------------------  tria_boundary_lib.h  ---------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 1998, 1999, 2000 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  tria_boundary_lib.h  ---------------------------
#ifndef __deal2__tria_boundary_lib_h
#define __deal2__tria_boundary_lib_h


#include <grid/tria_boundary.h>


/**
 *   Specialisation of @ref{Boundary}<dim>, which places the new point on
 *   the boundary of a ball in arbitrary dimension. It works by projecting
 *   the point in the middle of the old points onto the ball. The middle is
 *   defined as the arithmetic mean of the points. 
 *
 *   The center of the ball and its radius may be given upon construction of
 *   an object of this type. They default to the origin and a radius of 1.0.
 *
 *   This class is derived from @p{StraightBoundary} rather than from
 *   @p{Boundary}, which would seem natural, since this way we can use the
 *   @p{StraightBoundary<dim>::in_between(neighbors)} function.
 *
 * @author Wolfgang Bangerth, 1998
 */
template <int dim>
class HyperBallBoundary : public StraightBoundary<dim> {
  public:
				     /**
				      * Constructor
				      */
    HyperBallBoundary (const Point<dim> p      = Point<dim>(),
		       const double     radius = 1.0);

				     /**
				      * Refer to the general documentation of
				      * this class and the documentation of the
				      * base class.
				      */
    virtual Point<dim>
    get_new_point_on_line (const typename Triangulation<dim>::line_iterator &line) const;

				     /**
				      * Refer to the general documentation of
				      * this class and the documentation of the
				      * base class.
				      */
    virtual Point<dim>
    get_new_point_on_quad (const typename Triangulation<dim>::quad_iterator &quad) const;

				     /**
				      * Return the center of the ball.
				      */
    Point<dim> get_center () const;

				     /**
				      * Return the radius of the ball.
				      */
    double get_radius () const;
    
  protected:
				     /**
				      * Center point of the hyperball.
				      */
    const Point<dim> center;

				     /**
				      * Radius of the hyperball.
				      */
    const double radius;
};


/**
 * Variant of @p{HyperBallBoundary} which denotes a half hyper ball
 * where the first coordinate is restricted to the range $x>=0$ (or
 * $x>=center(0)$). In two dimensions, this equals the right half
 * circle, in three space dimensions it is a half ball.
 *
 * @author Wolfgang Bangerth, 1999
 */
template <int dim>
class HalfHyperBallBoundary : public HyperBallBoundary<dim> {
  public:
				     /**
				      * Constructor
				      */
    HalfHyperBallBoundary (const Point<dim> p      = Point<dim>(),
			   const double     radius = 1.0);

				     /**
				      * Check if on the line @p{x==0},
				      * otherwise pass to the base
				      * class.
				      */
    virtual Point<dim>
    get_new_point_on_line (const typename Triangulation<dim>::line_iterator &line) const;

				     /**
				      * Check if on the line @p{x==0},
				      * otherwise pass to the base
				      * class.
				      */
    virtual Point<dim>
    get_new_point_on_quad (const typename Triangulation<dim>::quad_iterator &quad) const;
};


/**
 * Class describing the boundaries of a hyper shell. Only the center
 * of the two spheres needs to be given, the radii of inner and outer
 * sphere are computed automatically upon calling one of the virtual
 * functions.
 *
 * @author Wolfgang Bangerth, 1999
 */
template <int dim>
class HyperShellBoundary : public StraightBoundary<dim> 
{
  public:
				     /**
				      * Constructor. The center of the
				      * spheres defaults to the
				      * origin.
				      */
    HyperShellBoundary (const Point<dim> &center = Point<dim>());
    
				     /**
				      * Construct a new point on a line.
				      */
    virtual Point<dim>
    get_new_point_on_line (const typename Triangulation<dim>::line_iterator &line) const;  
    
				     /**
				      * Construct a new point on a quad.
				      */
    virtual Point<dim>
    get_new_point_on_quad (const typename Triangulation<dim>::quad_iterator &quad) const;
    
  private:
				     /**
				      * Store the center of the spheres.
				      */
    const Point<dim> center;
};


#endif
