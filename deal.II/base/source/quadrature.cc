//---------------------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2005, 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------


#include <base/geometry_info.h>
#include <base/quadrature.h>
#include <base/qprojector.h>
#include <base/memory_consumption.h>

#include <cmath>
#include <iterator>


namespace
{
/**
 * Integer to the power of dim
 */
  template <int dim>
  inline unsigned int dimpow (unsigned int n)
  {
    unsigned int result = n;
    for (unsigned int i=1;i<dim;++i)
      result *= n;
    return result;
  }
}



template <>
Quadrature<0>::Quadrature (const unsigned int)
              : n_quadrature_points(0)
{}



template <>
Quadrature<0>::~Quadrature ()
{}



template <int dim>
Quadrature<dim>::Quadrature (const unsigned int n_q) :
		n_quadrature_points(n_q),
		quadrature_points (n_q, Point<dim>()),
		weights (n_q, 0)
{}



template <int dim>
Quadrature<dim>::Quadrature (const std::vector<Point<dim> > &points,
			     const std::vector<double>      &weights)
		:
		n_quadrature_points(points.size()),
		quadrature_points(points),
		weights(weights)
{
  Assert (weights.size() == points.size(),
          ExcDimensionMismatch(weights.size(), points.size()));
}



template <int dim>
Quadrature<dim>::Quadrature (const std::vector<Point<dim> > &points)
		:
		n_quadrature_points(points.size()),
		quadrature_points(points),
		weights(points.size(), std::atof("Inf"))
{
  Assert(weights.size() == points.size(),
	 ExcDimensionMismatch(weights.size(), points.size()));
}



template <int dim>
Quadrature<dim>::Quadrature (const Point<dim> &point)
                :
		n_quadrature_points(1),
		quadrature_points(std::vector<Point<dim> > (1, point)),
		weights(std::vector<double> (1, 1.))
{}


template <>
Quadrature<0>::Quadrature (const Quadrature<-1> &,
			   const Quadrature<1> &)
		:
                n_quadrature_points (0)
{
  Assert (false, ExcInternalError());
}



template <>
Quadrature<1>::Quadrature (const Quadrature<0> &,
			   const Quadrature<1> &)
                :
                n_quadrature_points (0)
{
  Assert (false, ExcInternalError());
}



template <int dim>
Quadrature<dim>::Quadrature (const SubQuadrature &q1,
			     const Quadrature<1> &q2)
		:
		n_quadrature_points (q1.n_quadrature_points *
				     q2.n_quadrature_points),
		quadrature_points (n_quadrature_points),
		weights (n_quadrature_points, 0)
{
  unsigned int present_index = 0;
  for (unsigned int i2=0; i2<q2.n_quadrature_points; ++i2)
    for (unsigned int i1=0; i1<q1.n_quadrature_points; ++i1)
      {
					 // compose coordinates of
					 // new quadrature point by tensor
					 // product in the last component
	for (unsigned int d=0; d<dim-1; ++d)
	  quadrature_points[present_index](d)
	    = q1.point(i1)(d);
	quadrature_points[present_index](dim-1)
	  = q2.point(i2)(0);
					       
	weights[present_index] = q1.weight(i1) * q2.weight(i2);

	++present_index;
      };

#ifdef DEBUG
  if (n_quadrature_points > 0)
    {
      double sum = 0;
      for (unsigned int i=0; i<n_quadrature_points; ++i)
	sum += weights[i];
				       // we cant guarantee the sum of weights
				       // to be exactly one, but it should be
				       // near that. 
      Assert ((sum>0.999999) && (sum<1.000001), ExcInternalError());
    }
#endif
}



template <>
Quadrature<1>::Quadrature (const Quadrature<0> &)
		:
		Subscriptor(),
		n_quadrature_points (deal_II_numbers::invalid_unsigned_int),
		quadrature_points (),
		weights ()
{
                                   // this function should never be
                                   // called -- this should be the
                                   // copy constructor in 1d...
  Assert (false, ExcInternalError());
}



template <int dim>
Quadrature<dim>::Quadrature (const Quadrature<dim != 1 ? 1 : 0> &q)
		:
		Subscriptor(),
		n_quadrature_points (dimpow<dim>(q.n_quadrature_points)),
		quadrature_points (n_quadrature_points),
		weights (n_quadrature_points, 0.)
{
  Assert (dim <= 3, ExcNotImplemented());
  
  const unsigned int n0 = q.n_quadrature_points;
  const unsigned int n1 = (dim>1) ? n0 : 1;
  const unsigned int n2 = (dim>2) ? n0 : 1;

  unsigned int k=0;
  for (unsigned int i2=0;i2<n2;++i2)
    for (unsigned int i1=0;i1<n1;++i1)
      for (unsigned int i0=0;i0<n0;++i0)
	{
	  quadrature_points[k](0) = q.point(i0)(0);
	  if (dim>1)
	    quadrature_points[k](1) = q.point(i1)(0);
	  if (dim>2)
	    quadrature_points[k](2) = q.point(i2)(0);
	  weights[k] = q.weight(i0);
	  if (dim>1)
	    weights[k] *= q.weight(i1);
	  if (dim>2)
	    weights[k] *= q.weight(i2);
	  ++k;
	}
}



template <int dim>
Quadrature<dim>::Quadrature (const Quadrature<dim> &q)
		:
		Subscriptor(),
		n_quadrature_points (q.n_quadrature_points),
		quadrature_points (q.quadrature_points),
		weights (q.weights)
{}



template <int dim>
Quadrature<dim>::~Quadrature ()
{}



template <>
const Point<0> &
Quadrature<0>::point (const unsigned int) const
{
  Assert (false, ExcInternalError());
  static const Point<0> dummy;
  return dummy;
}



template <int dim>
const Point<dim> &
Quadrature<dim>::point (const unsigned int i) const
{
  Assert (i<n_quadrature_points, ExcIndexRange(i, 0, n_quadrature_points));
  return quadrature_points[i];
}



template <>
const std::vector<Point<0> > &
Quadrature<0>::get_points () const
{
  Assert (false, ExcInternalError());
  return quadrature_points;
}



template <int dim>
const std::vector<Point<dim> > &
Quadrature<dim>::get_points () const
{
  return quadrature_points;
}



template <>
double
Quadrature<0>::weight (const unsigned int) const
{
  Assert (false, ExcInternalError());
  return 0;
}



template <int dim>
double
Quadrature<dim>::weight (const unsigned int i) const
{
  Assert (i<n_quadrature_points, ExcIndexRange(i, 0, n_quadrature_points));
  return weights[i];
}



template <int dim>
const std::vector<double> &
Quadrature<dim>::get_weights () const
{
  return weights;
}



template <>
const std::vector<double> &
Quadrature<0>::get_weights () const
{
  Assert (false, ExcInternalError());
  return weights;
}



template <int dim>
unsigned int
Quadrature<dim>::memory_consumption () const
{
  return (MemoryConsumption::memory_consumption (quadrature_points) +
	  MemoryConsumption::memory_consumption (weights));
}


//---------------------------------------------------------------------------
template<int dim>
QAnisotropic<dim>::QAnisotropic(const Quadrature<1>& qx)
		: Quadrature<dim>(qx.n_quadrature_points)
{
  Assert (dim==1, ExcImpossibleInDim(dim));
  unsigned int k=0;
  for (unsigned int k1=0;k1<qx.n_quadrature_points;++k1)
    {
      this->quadrature_points[k](0) = qx.point(k1)(0);
      this->weights[k++] = qx.weight(k1);
    }
  Assert (k==this->n_quadrature_points, ExcInternalError());
}



template<int dim>
QAnisotropic<dim>::QAnisotropic(const Quadrature<1>& qx,
				const Quadrature<1>& qy)
		: Quadrature<dim>(qx.n_quadrature_points
				  *qy.n_quadrature_points)
{
  Assert (dim==2, ExcImpossibleInDim(dim));
  unsigned int k=0;
  for (unsigned int k2=0;k2<qy.n_quadrature_points;++k2)
    for (unsigned int k1=0;k1<qx.n_quadrature_points;++k1)
    {
      this->quadrature_points[k](0) = qx.point(k1)(0);
      this->quadrature_points[k](1) = qy.point(k2)(0);
      this->weights[k++] = qx.weight(k1) * qy.weight(k2);
    }
  Assert (k==this->n_quadrature_points, ExcInternalError());
}



template<int dim>
QAnisotropic<dim>::QAnisotropic(const Quadrature<1>& qx,
				const Quadrature<1>& qy,
				const Quadrature<1>& qz)
		: Quadrature<dim>(qx.n_quadrature_points
				  *qy.n_quadrature_points
				  *qz.n_quadrature_points)
{
  Assert (dim==3, ExcImpossibleInDim(dim));
  unsigned int k=0;
  for (unsigned int k3=0;k3<qz.n_quadrature_points;++k3)
    for (unsigned int k2=0;k2<qy.n_quadrature_points;++k2)
      for (unsigned int k1=0;k1<qx.n_quadrature_points;++k1)
	{
	  this->quadrature_points[k](0) = qx.point(k1)(0);
	  this->quadrature_points[k](1) = qy.point(k2)(0);
	  this->quadrature_points[k](2) = qz.point(k3)(0);
	  this->weights[k++] = qx.weight(k1) * qy.weight(k2) * qz.weight(k3);
	}
  Assert (k==this->n_quadrature_points, ExcInternalError());
}



//---------------------------------------------------------------------------



template <int dim>
Quadrature<2>
QProjector<dim>::reflect (const Quadrature<2> &q) 
{
  std::vector<Point<2> > q_points (q.n_quadrature_points);
  std::vector<double>    weights (q.n_quadrature_points);
  for (unsigned int i=0; i<q.n_quadrature_points; ++i)
    {
      q_points[i][0] = q.point(i)[1];
      q_points[i][1] = q.point(i)[0];

      weights[i] = q.weight(i);
    }

  return Quadrature<2> (q_points, weights);
}


template <>
void
QProjector<1>::project_to_face (const Quadrature<0> &,
                                const unsigned int,
                                std::vector<Point<1> > &)
{
  Assert (false, ExcNotImplemented());
}



template <>
void
QProjector<2>::project_to_face (const Quadrature<1>      &quadrature,
                                const unsigned int        face_no,
                                std::vector<Point<2> >   &q_points)
{
  const unsigned int dim=2;
  Assert (face_no<2*dim, ExcIndexRange (face_no, 0, 2*dim));
  Assert (q_points.size() == quadrature.n_quadrature_points,
	  ExcDimensionMismatch (q_points.size(), quadrature.n_quadrature_points));
  
  for (unsigned int p=0; p<quadrature.n_quadrature_points; ++p)
    switch (face_no)
      {
	case 0:
	      q_points[p] = Point<dim>(0,quadrature.point(p)(0));
	      break;	   
	case 1:
	      q_points[p] = Point<dim>(1,quadrature.point(p)(0));
	      break;	   
	case 2:
	      q_points[p] = Point<dim>(quadrature.point(p)(0),0);
	      break;	   
	case 3:
	      q_points[p] = Point<dim>(quadrature.point(p)(0),1);
	      break;
	default:
	      Assert (false, ExcInternalError());
      };
}



template <>
void
QProjector<3>::project_to_face (const Quadrature<2>    &quadrature,
                                const unsigned int      face_no,
                                std::vector<Point<3> > &q_points)
{
  const unsigned int dim=3;
  Assert (face_no<2*dim, ExcIndexRange (face_no, 0, 2*dim));
  Assert (q_points.size() == quadrature.n_quadrature_points,
	  ExcDimensionMismatch (q_points.size(), quadrature.n_quadrature_points));
  
  for (unsigned int p=0; p<quadrature.n_quadrature_points; ++p)
    switch (face_no)
      {
	case 0:
	      q_points[p] = Point<dim>(0,
				       quadrature.point(p)(0),
				       quadrature.point(p)(1));
	      break;	   
	case 1:
	      q_points[p] = Point<dim>(1,
				       quadrature.point(p)(0),
				       quadrature.point(p)(1));
	      break;	   
	case 2:
	      q_points[p] = Point<dim>(quadrature.point(p)(1),
				       0,
				       quadrature.point(p)(0));
	      break;
	case 3:
	      q_points[p] = Point<dim>(quadrature.point(p)(1),
				       1,
				       quadrature.point(p)(0));
	      break;
	case 4:
	      q_points[p] = Point<dim>(quadrature.point(p)(0),
				       quadrature.point(p)(1),
				       0);
	      break;
	case 5:
	      q_points[p] = Point<dim>(quadrature.point(p)(0),
				       quadrature.point(p)(1),
				       1);
	      break;
	      
	default:
	      Assert (false, ExcInternalError());
      };
}



template <>
void
QProjector<1>::project_to_subface (const Quadrature<0> &,
                                   const unsigned int,
                                   const unsigned int,
                                   std::vector<Point<1> > &)
{
  Assert(false, ExcNotImplemented());
}


  
template <>
void
QProjector<2>::project_to_subface (const Quadrature<1>    &quadrature,
                                   const unsigned int      face_no,
                                   const unsigned int      subface_no,
                                   std::vector<Point<2> > &q_points)
{
  const unsigned int dim=2;
  Assert (face_no<2*dim, ExcIndexRange (face_no, 0, 2*dim));
  Assert (subface_no<(1<<(dim-1)), ExcIndexRange (face_no, 0, 1<<(dim-1)));
  Assert (q_points.size() == quadrature.n_quadrature_points,
	  ExcDimensionMismatch (q_points.size(), quadrature.n_quadrature_points));
  
  for (unsigned int p=0; p<quadrature.n_quadrature_points; ++p)
    switch (face_no)
      {
	case 0:
	      switch (subface_no) 
		{
		  case 0:
			q_points[p] = Point<dim>(0,quadrature.point(p)(0)/2);
			break;
		  case 1:
			q_points[p] = Point<dim>(0,quadrature.point(p)(0)/2+0.5);
			break;
		  default:
			Assert (false, ExcInternalError());
		};
	      break;	   
	case 1:
	      switch (subface_no) 
		{
		  case 0:
			q_points[p] = Point<dim>(1,quadrature.point(p)(0)/2);
			break;
		  case 1:
			q_points[p] = Point<dim>(1,quadrature.point(p)(0)/2+0.5);
			break;
		  default:
			Assert (false, ExcInternalError());
		};
	      break;	   
	case 2:
	      switch (subface_no) 
		{
		  case 0:
			q_points[p]
			  = Point<dim>(quadrature.point(p)(0)/2,0);
			break;
		  case 1:
			q_points[p]
			  = Point<dim>(quadrature.point(p)(0)/2+0.5,0);
			break;
		  default:
			Assert (false, ExcInternalError());
		};
	      break;	   
	case 3:
	      switch (subface_no) 
		{
		  case 0:
			q_points[p] = Point<dim>(quadrature.point(p)(0)/2,1);
			break;
		  case 1:
			q_points[p] = Point<dim>(quadrature.point(p)(0)/2+0.5,1);
			break;
		  default:
			Assert (false, ExcInternalError());
		};
	      break;

	default:
	      Assert (false, ExcInternalError());
      };
}



template <>
void
QProjector<3>::project_to_subface (const Quadrature<2>    &quadrature,
                                   const unsigned int      face_no,
                                   const unsigned int      subface_no,
                                   std::vector<Point<3> > &q_points)
{
  const unsigned int dim=3;
  Assert (face_no<2*dim, ExcIndexRange (face_no, 0, 2*dim));
  Assert (subface_no<(1<<(dim-1)), ExcIndexRange (face_no, 0, 1<<(dim-1)));
  Assert (q_points.size() == quadrature.n_quadrature_points,
	  ExcDimensionMismatch (q_points.size(), quadrature.n_quadrature_points));


				   // for all faces and subfaces:
				   // first project onto the first
				   // subface of each face, then move
				   // it to the right place
  for (unsigned int p=0; p<quadrature.n_quadrature_points; ++p)
    switch (face_no)
      {
	case 0:
	      q_points[p] = Point<dim>(0,
				       quadrature.point(p)(0)/2,
				       quadrature.point(p)(1)/2);
	      switch (subface_no) 
		{
		  case 0:
			break;
		  case 1:
			q_points[p][1] += 1./2.;
			break;
		  case 2:
			q_points[p][2] += 1./2.;
			break;
		  case 3:
			q_points[p][1] += 1./2.;
			q_points[p][2] += 1./2.;
			break;
		  default:
			Assert (false, ExcInternalError());
		};
	      break;	   
	case 1:
	      q_points[p] = Point<dim>(1,
				       quadrature.point(p)(0)/2,
				       quadrature.point(p)(1)/2);
	      switch (subface_no) 
		{
		  case 0:
			break;
		  case 1:
			q_points[p][1] += 1./2.;
			break;
		  case 2:
			q_points[p][2] += 1./2.;
			break;
		  case 3:
			q_points[p][1] += 1./2.;
			q_points[p][2] += 1./2.;
			break;
		  default:
			Assert (false, ExcInternalError());
		};
	      break;	   
	case 2:
	      q_points[p] = Point<dim>(quadrature.point(p)(1)/2,
				       0,
				       quadrature.point(p)(0)/2);
	      switch (subface_no) 
		{
		  case 0:
			break;
		  case 1:
			q_points[p][2] += 1./2.;
			break;
		  case 2:
			q_points[p][0] += 1./2.;
			break;
		  case 3:
			q_points[p][0] += 1./2.;
			q_points[p][2] += 1./2.;
			break;
		  default:
			Assert (false, ExcInternalError());
		};
	      break;
	case 3:
	      q_points[p] = Point<dim>(quadrature.point(p)(1)/2,
				       1,
				       quadrature.point(p)(0)/2);
	      switch (subface_no) 
		{
		  case 0:
			break;
		  case 1:
			q_points[p][2] += 1./2.;
			break;
		  case 2:
			q_points[p][0] += 1./2.;
			break;
		  case 3:
			q_points[p][0] += 1./2.;
			q_points[p][2] += 1./2.;
			break;
		  default:
			Assert (false, ExcInternalError());
		};
	      break;
	case 4:
	      q_points[p] = Point<dim>(quadrature.point(p)(0)/2,
				       quadrature.point(p)(1)/2,
				       0);
	      switch (subface_no) 
		{
		  case 0:
			break;
		  case 1:
			q_points[p][0] += 1./2.;
			break;
		  case 2:
			q_points[p][1] += 1./2.;
			break;
		  case 3:
			q_points[p][0] += 1./2.;
			q_points[p][1] += 1./2.;
			break;
		  default:
			Assert (false, ExcInternalError());
		};
	      break;
	case 5:
	      q_points[p] = Point<dim>(quadrature.point(p)(0)/2,
				       quadrature.point(p)(1)/2,
				       1);
	      switch (subface_no) 
		{
		  case 0:
			break;
		  case 1:
			q_points[p][0] += 1./2.;
			break;
		  case 2:
			q_points[p][1] += 1./2.;
			break;
		  case 3:
			q_points[p][0] += 1./2.;
			q_points[p][1] += 1./2.;
			break;
		  default:
			Assert (false, ExcInternalError());
		};
	      break;
	default:
	      Assert (false, ExcInternalError());
      };
}



template <>
Quadrature<1>
QProjector<1>::project_to_all_faces (const Quadrature<0> &)
{
  Assert (false, ExcImpossibleInDim(1));
  return Quadrature<1>(0);
}



template <>
Quadrature<2>
QProjector<2>::project_to_all_faces (const SubQuadrature &quadrature)
{
  const unsigned int dim = 2;
  
  const unsigned int n_points = quadrature.n_quadrature_points,
		     n_faces  = GeometryInfo<dim>::faces_per_cell;

				   // first fix quadrature points
  std::vector<Point<dim> > q_points;
  q_points.reserve(n_points * n_faces);
  std::vector <Point<dim> > help(n_points);
  
				   // project to each face and append
				   // results
  for (unsigned int face=0; face<n_faces; ++face)
    {
      project_to_face(quadrature, face, help);
      std::copy (help.begin(), help.end(),
                 std::back_inserter (q_points));
    }

				   // next copy over weights
  std::vector<double> weights;
  weights.reserve (n_points * n_faces);
  for (unsigned int face=0; face<n_faces; ++face)
    std::copy (quadrature.get_weights().begin(),
               quadrature.get_weights().end(),
               std::back_inserter (weights));

  Assert (q_points.size() == n_points * n_faces,
          ExcInternalError());
  Assert (weights.size() == n_points * n_faces,
          ExcInternalError());  
  
  return Quadrature<dim>(q_points, weights);
}



template <>
Quadrature<3>
QProjector<3>::project_to_all_faces (const SubQuadrature &quadrature)
{
  const unsigned int dim = 3;
  
  const SubQuadrature q_reflected = reflect (quadrature);
  
  const unsigned int n_points = quadrature.n_quadrature_points,
		     n_faces  = GeometryInfo<dim>::faces_per_cell;

				   // first fix quadrature points
  std::vector<Point<dim> > q_points;
  q_points.reserve(n_points * n_faces * 2);
  std::vector <Point<dim> > help(n_points);
  
				   // project to each face and append
				   // results
  for (unsigned int face=0; face<n_faces; ++face)
    {
      project_to_face(quadrature, face, help);
      std::copy (help.begin(), help.end(),
                 std::back_inserter (q_points));
    }

				   // next copy over weights
  std::vector<double> weights;
  weights.reserve (n_points * n_faces * 2);
  for (unsigned int face=0; face<n_faces; ++face)
    std::copy (quadrature.get_weights().begin(),
               quadrature.get_weights().end(),
               std::back_inserter (weights));

                                   // then do same for other
                                   // orientation of faces
  for (unsigned int face=0; face<n_faces; ++face)
    {
      project_to_face(q_reflected, face, help);
      std::copy (help.begin(), help.end(),
                 std::back_inserter (q_points));
    }
  for (unsigned int face=0; face<n_faces; ++face)
    std::copy (q_reflected.get_weights().begin(),
               q_reflected.get_weights().end(),
               std::back_inserter (weights));

  Assert (q_points.size() == n_points * n_faces * 2,
          ExcInternalError());
  Assert (weights.size() == n_points * n_faces * 2,
          ExcInternalError());  
  
  return Quadrature<dim>(q_points, weights);
}



template <>
Quadrature<1>
QProjector<1>::project_to_all_subfaces (const Quadrature<0> &)
{
  Assert (false, ExcImpossibleInDim(1));
  return Quadrature<1>(0);
}



template <>
Quadrature<2>
QProjector<2>::project_to_all_subfaces (const SubQuadrature &quadrature)
{
  const unsigned int dim = 2;
  
  const unsigned int n_points          = quadrature.n_quadrature_points,
		     n_faces           = GeometryInfo<dim>::faces_per_cell,
		     subfaces_per_face = GeometryInfo<dim>::subfaces_per_face;
  
				   // first fix quadrature points
  std::vector<Point<dim> > q_points;
  q_points.reserve (n_points * n_faces * subfaces_per_face);
  std::vector <Point<dim> > help(n_points);
  
				   // project to each face and copy
				   // results
  for (unsigned int face=0; face<n_faces; ++face)
    for (unsigned int subface=0; subface<subfaces_per_face; ++subface)
      {
	project_to_subface(quadrature, face, subface, help);
	std::copy (help.begin(), help.end(),
                   std::back_inserter (q_points));
      };

				   // next copy over weights
  std::vector<double> weights;
  weights.reserve (n_points * n_faces * subfaces_per_face);
  for (unsigned int face=0; face<n_faces; ++face)
    for (unsigned int subface=0; subface<subfaces_per_face; ++subface)
      std::copy (quadrature.get_weights().begin(),
                 quadrature.get_weights().end(),
                 std::back_inserter (weights));

  Assert (q_points.size() == n_points * n_faces * subfaces_per_face,
          ExcInternalError());
  Assert (weights.size() == n_points * n_faces * subfaces_per_face,
          ExcInternalError());
  
  return Quadrature<dim>(q_points, weights);
}



template <>
Quadrature<3>
QProjector<3>::project_to_all_subfaces (const SubQuadrature &quadrature)
{
  const unsigned int dim = 3;
  
  const SubQuadrature q_reflected = reflect (quadrature);

  const unsigned int n_points          = quadrature.n_quadrature_points,
		     n_faces           = GeometryInfo<dim>::faces_per_cell,
		     subfaces_per_face = GeometryInfo<dim>::subfaces_per_face;
  
				   // first fix quadrature points
  std::vector<Point<dim> > q_points;
  q_points.reserve (n_points * n_faces * subfaces_per_face * 2);
  std::vector <Point<dim> > help(n_points);
  
				   // project to each face and copy
				   // results
  for (unsigned int face=0; face<n_faces; ++face)
    for (unsigned int subface=0; subface<subfaces_per_face; ++subface)
      {
	project_to_subface(quadrature, face, subface, help);
	std::copy (help.begin(), help.end(),
                   std::back_inserter (q_points));
      }

				   // next copy over weights
  std::vector<double> weights;
  weights.reserve (n_points * n_faces * subfaces_per_face * 2);
  for (unsigned int face=0; face<n_faces; ++face)
    for (unsigned int subface=0; subface<subfaces_per_face; ++subface)
      std::copy (quadrature.get_weights().begin(),
                 quadrature.get_weights().end(),
                 std::back_inserter (weights));

                                   // do same with other orientation
                                   // of faces
  for (unsigned int face=0; face<n_faces; ++face)
    for (unsigned int subface=0; subface<subfaces_per_face; ++subface)
      {
	project_to_subface(q_reflected, face, subface, help);
	std::copy (help.begin(), help.end(),
                   std::back_inserter (q_points));
      };
  for (unsigned int face=0; face<n_faces; ++face)
    for (unsigned int subface=0; subface<subfaces_per_face; ++subface)
      std::copy (q_reflected.get_weights().begin(),
                 q_reflected.get_weights().end(),
                 std::back_inserter (weights));

  Assert (q_points.size() == n_points * n_faces * subfaces_per_face * 2,
          ExcInternalError());
  Assert (weights.size() == n_points * n_faces * subfaces_per_face * 2,
          ExcInternalError());  
  
  return Quadrature<dim>(q_points, weights);
}



template <int dim>
Quadrature<dim>
QProjector<dim>::project_to_child (const Quadrature<dim>    &quadrature,
				   const unsigned int        child_no)
{
  Assert (child_no < GeometryInfo<dim>::children_per_cell,
	  ExcIndexRange (child_no, 0, GeometryInfo<dim>::children_per_cell));
  
  const unsigned int n_q_points = quadrature.n_quadrature_points;

  std::vector<Point<dim> > q_points(n_q_points);
  for (unsigned int i=0; i<n_q_points; ++i)
    q_points[i]=GeometryInfo<dim>::child_to_cell_coordinates(
      quadrature.point(i), child_no);

				   // for the weights, things are
				   // equally simple: copy them and
				   // scale them
  std::vector<double> weights = quadrature.get_weights ();
  for (unsigned int i=0; i<n_q_points; ++i)
    weights[i] *= (1./GeometryInfo<dim>::children_per_cell);

  return Quadrature<dim> (q_points, weights);
}



template <int dim>
Quadrature<dim>
QProjector<dim>::project_to_line(
  const Quadrature<1>& quadrature,
  const Point<dim>& p1,
  const Point<dim>& p2)
{
  const unsigned int n = quadrature.n_quadrature_points;
  std::vector<Point<dim> > points(n);
  std::vector<double> weights(n);
  const double length = p1.distance(p2);
  
  for (unsigned int k=0;k<n;++k)
    {
      const double alpha = quadrature.point(k)(0);
      points[k] = alpha * p2;
      points[k] += (1.-alpha) * p1;
      weights[k] = length * quadrature.weight(k);
    }
  return Quadrature<dim> (points, weights);
}


template <int dim>
typename QProjector<dim>::DataSetDescriptor
QProjector<dim>::DataSetDescriptor::cell ()
{
  return 0;
}


template <int dim>
typename QProjector<dim>::DataSetDescriptor
QProjector<dim>::DataSetDescriptor::
face (const unsigned int face_no,
      const bool         face_orientation,
      const unsigned int n_quadrature_points)
{
  Assert (dim != 1, ExcInternalError());
  Assert (face_no < GeometryInfo<dim>::faces_per_cell,
          ExcInternalError());
  
  switch (dim)
    {
      case 1:
      case 2:
            return face_no * n_quadrature_points;

                                             // in 3d, we have to
                                             // account for faces
                                             // that have reverse
                                             // orientation. thus,
                                             // we have to store
                                             // _two_ data sets per
                                             // face or subface
      case 3:
            return ((face_no +
                     (face_orientation == true ?
                      0 : GeometryInfo<dim>::faces_per_cell))
                    * n_quadrature_points);

      default:
            Assert (false, ExcInternalError());
    }
  return deal_II_numbers::invalid_unsigned_int;
}



template <int dim>
typename QProjector<dim>::DataSetDescriptor
QProjector<dim>::DataSetDescriptor::
subface (const unsigned int face_no,
         const unsigned int subface_no,
         const bool         face_orientation,
         const unsigned int n_quadrature_points)
{
  Assert (dim != 1, ExcInternalError());
  Assert (face_no < GeometryInfo<dim>::faces_per_cell,
          ExcInternalError());
                                   // the trick with +1 prevents
                                   // that we get a warning in 1d
  Assert (subface_no+1 < GeometryInfo<dim>::subfaces_per_face+1,
          ExcInternalError());
  
  switch (dim)
    {
      case 1:
      case 2:
            return ((face_no * GeometryInfo<dim>::subfaces_per_face +
                     subface_no)
                    * n_quadrature_points);

                                             // for 3d, same as above:
      case 3:
            return (((face_no * GeometryInfo<dim>::subfaces_per_face +
                      subface_no)
                     + (face_orientation == true ?
                        0 :
                        GeometryInfo<dim>::faces_per_cell *
                        GeometryInfo<dim>::subfaces_per_face)
                     )
                    * n_quadrature_points);
      default:
            Assert (false, ExcInternalError());
    }
  return deal_II_numbers::invalid_unsigned_int;              
}


template <int dim>
QProjector<dim>::DataSetDescriptor::operator unsigned int () const
{
  return dataset_offset;
}



template <int dim>
QProjector<dim>::DataSetDescriptor::
DataSetDescriptor (const unsigned int dataset_offset)
                :
                dataset_offset (dataset_offset)
{}


template <int dim>
QProjector<dim>::DataSetDescriptor::
DataSetDescriptor ()
                :
                dataset_offset (deal_II_numbers::invalid_unsigned_int)
{}



template <int dim>
Quadrature<dim>
QProjector<dim>::project_to_face(const SubQuadrature &quadrature,
				 const unsigned int face_no)
{
  std::vector<Point<dim> > points(quadrature.n_quadrature_points);
  project_to_face(quadrature, face_no, points);
  return Quadrature<dim>(points, quadrature.get_weights());
}


template <int dim>
Quadrature<dim>
QProjector<dim>::project_to_subface(const SubQuadrature &quadrature,
				    const unsigned int face_no,
				    const unsigned int subface_no)
{
  std::vector<Point<dim> > points(quadrature.n_quadrature_points);
  project_to_subface(quadrature, face_no, subface_no, points);
  return Quadrature<dim>(points, quadrature.get_weights());
}


// ------------------------------------------------------------ //


template <>
bool
QIterated<1>::uses_both_endpoints (const Quadrature<1> &base_quadrature)
{
  bool at_left = false,
      at_right = false;
  for (unsigned int i=0; i<base_quadrature.n_quadrature_points; ++i)
    {
      if (base_quadrature.point(i) == Point<1>(0.0))
	at_left = true;
      if (base_quadrature.point(i) == Point<1>(1.0))
	at_right = true;
    };

  return (at_left && at_right);
}



template <>
QIterated<1>::QIterated (const Quadrature<1> &base_quadrature,
			 const unsigned int   n_copies)
                :
		Quadrature<1> (uses_both_endpoints(base_quadrature) ?
			       (base_quadrature.n_quadrature_points-1) * n_copies + 1 :
			       base_quadrature.n_quadrature_points * n_copies) 
{
  Assert (n_copies > 0, ExcZero());
  
  if (!uses_both_endpoints(base_quadrature))
				     // we don't have to skip some
				     // points in order to get a
				     // reasonable quadrature formula
    {
      unsigned int next_point = 0;
      for (unsigned int copy=0; copy<n_copies; ++copy)
	for (unsigned int q_point=0; q_point<base_quadrature.n_quadrature_points; ++q_point)
	  {
	    this->quadrature_points[next_point]
	      = Point<1>(base_quadrature.point(q_point)(0) / n_copies
			 +
			 (1.0*copy)/n_copies);
	    this->weights[next_point]
	      = base_quadrature.weight(q_point) / n_copies;

	    ++next_point;
	  };
    }
  else
				     // skip doubly available points
    {
      unsigned int next_point = 0;

				       // first find out the weights of
				       // the left and the right boundary
				       // points. note that these usually
				       // are but need not necessarily be
				       // the same
      double double_point_weight = 0;
      unsigned int n_end_points = 0;
      for (unsigned int i=0; i<base_quadrature.n_quadrature_points; ++i)
					 // add up the weight if this
					 // is an endpoint
	if ((base_quadrature.point(i) == Point<1>(0.0)) ||
	    (base_quadrature.point(i) == Point<1>(1.0)))
	  {
	    double_point_weight += base_quadrature.weight(i);
	    ++n_end_points;
	  };
				       // scale the weight correctly
      double_point_weight /= n_copies;
//TODO:[WB] The following assertion looks strange. InternalError???
				       // make sure the base quadrature formula
				       // has only one quadrature point
				       // per end point
      Assert (n_end_points == 2, ExcInvalidQuadratureFormula());


      for (unsigned int copy=0; copy<n_copies; ++copy)
	for (unsigned int q_point=0; q_point<base_quadrature.n_quadrature_points; ++q_point)
	  {
					     // skip the left point of
					     // this copy since we
					     // have already entered
					     // it the last time
	    if ((copy > 0) &&
		(base_quadrature.point(q_point) == Point<1>(0.0)))
	      continue;
	    
	    this->quadrature_points[next_point]
	      = Point<1>(base_quadrature.point(q_point)(0) / n_copies
			 +
			 (1.0*copy)/n_copies);

					     // if this is the
					     // rightmost point of one
					     // of the non-last
					     // copies: give it the
					     // double weight
	    if ((copy != n_copies-1) &&
		(base_quadrature.point(q_point) == Point<1>(1.0)))
	      this->weights[next_point] = double_point_weight;
	    else
	      this->weights[next_point] = base_quadrature.weight(q_point) /
					  n_copies;
	    
	    ++next_point;
	  };
    };

#if DEBUG
  double sum_of_weights = 0;
  for (unsigned int i=0; i<this->n_quadrature_points; ++i)
    sum_of_weights += this->weight(i);
  Assert (std::fabs(sum_of_weights-1) < 1e-15,
	  ExcInternalError());
#endif
}



// construct higher dimensional quadrature formula by tensor product
// of lower dimensional iterated quadrature formulae
template <int dim>
QIterated<dim>::QIterated (const Quadrature<1> &base_quadrature,
			   const unsigned int   N)
                :
		Quadrature<dim> (QIterated<dim-1>(base_quadrature, N),
				 QIterated<1>(base_quadrature, N))
{}



// explicit instantiations; note: we need them all for all dimensions
template class Quadrature<1>;
template class Quadrature<2>;
template class Quadrature<3>;
template class QAnisotropic<1>;
template class QAnisotropic<2>;
template class QAnisotropic<3>;
template class QIterated<1>;
template class QIterated<2>;
template class QIterated<3>;
template class QProjector<1>;
template class QProjector<2>;
template class QProjector<3>;
