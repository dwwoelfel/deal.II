/*----------------------------   quadrature.h     ---------------------------*/
/*      <Id:>                 */
#ifndef __quadrature_H
#define __quadrature_H
/*----------------------------   quadrature.h     ---------------------------*/


#include <grid/point.h>
#include <vector.h>



/**
  Base class for quadrature formulae in arbitrary dimensions. This class
  stores quadrature points and weights on the unit line [0,1], unit
  square [0,1]x[0,1], etc. This information is used together with
  objects of the \Ref{FiniteElement} class to compute the values stored
  in the \Ref{FEValues} objects.
  */
template <int dim>
class Quadrature {
  public:
				     /**
				      * Number of quadrature points.
				      */
    const unsigned int n_quadrature_points;

				     /**
				      * Constructor.
				      */
    Quadrature (const unsigned int n_quadrature_points);
    
				     /**
				      * Return the #i#th quadrature point.
				      */
    const Point<dim> & quad_point (const unsigned int i) const;

				     /**
				      * Return the weight of the #i#th
				      * quadrature point.
				      */
    double weight (const unsigned int i) const;

  protected:
				     /**
				      * List of quadrature points. To be filled
				      * by the constructors of derived classes.
				      */
    vector<Point<dim> > quadrature_points;

				     /**
				      * List of weights of the quadrature points.
				      * To be filled by the constructors of
				      * derived classes.
				      */
    vector<double>      weights;
};





/*----------------------------   quadrature.h     ---------------------------*/
/* end of #ifndef __quadrature_H */
#endif
/*----------------------------   quadrature.h     ---------------------------*/
