//----------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 1998, 1999, 2000, 2001, 2002 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------------------------------------------


// only compile in 2d
#if deal_II_dimension == 2

#include <fe/fe_dgp.h>

// Transfer matrices for finite elements

#define SQRT3 1.732050807569

namespace FE_DGP_2d
{
  static const double dgp0_into_dgp0_refined_0[] =
  {
	1.,
  };

  static const double dgp0_into_dgp0_refined_1[] =
  {
	1.,
  };

  static const double dgp0_into_dgp0_refined_2[] =
  {
	1.,
  };

  static const double dgp0_into_dgp0_refined_3[] =
  {
	1.,
  };

  static const double dgp1_into_dgp1_refined_0[] =
  {
	1., -SQRT3/2., -SQRT3/2.,
	0, .5, 0,
	0, 0, .5,
  };

  static const double dgp1_into_dgp1_refined_1[] =
  {
	1., SQRT3/2., -SQRT3/2.,
	0, .5, 0,
	0, 0, .5,
  };

  static const double dgp1_into_dgp1_refined_2[] =
  {
	1., SQRT3/2., SQRT3/2.,
	0, .5, 0,
	0, 0, .5,
  };

  static const double dgp1_into_dgp1_refined_3[] =
  {
	1., -SQRT3/2., SQRT3/2.,
	0, .5, 0,
	0, 0, .5,
  };

  static const double dgp2_into_dgp2_refined_0[] =
  {
	1., -SQRT3/2., 0, -SQRT3/2., .75, 0,
	0, .5, -26.14263759/27., 0, -SQRT3/4., 0,
	0, 0, .25, 0, 0, 0,
	0, 0, 0, .5, -SQRT3/4., -26.14263759/27.,
	0, 0, 0, 0, .25, 0,
	0, 0, 0, 0, 0, .25,
  };

  static const double dgp2_into_dgp2_refined_1[] =
  {
	1., SQRT3/2., 0, -SQRT3/2., -.75, 0,
	0, .5, 26.14263759/27., 0, -SQRT3/4., 0,
	0, 0, .25, 0, 0, 0,
	0, 0, 0, .5, SQRT3/4., -26.14263759/27.,
	0, 0, 0, 0, .25, 0,
	0, 0, 0, 0, 0, .25,
  };

  static const double dgp2_into_dgp2_refined_2[] =
  {
	1., SQRT3/2., 0, SQRT3/2., .75, 0,
	0, .5, 26.14263759/27., 0, SQRT3/4., 0,
	0, 0, .25, 0, 0, 0,
	0, 0, 0, .5, SQRT3/4., 26.14263759/27.,
	0, 0, 0, 0, .25, 0,
	0, 0, 0, 0, 0, .25,
  };

  static const double dgp2_into_dgp2_refined_3[] =
  {
	1., -SQRT3/2., 0, SQRT3/2., -.75, 0,
	0, .5, -26.14263759/27., 0, SQRT3/4., 0,
	0, 0, .25, 0, 0, 0,
	0, 0, 0, .5, -SQRT3/4., 26.14263759/27.,
	0, 0, 0, 0, .25, 0,
	0, 0, 0, 0, 0, .25,
  };

  static const double dgp3_into_dgp3_refined_0[] =
  {
	1., -SQRT3/2., 0, 8.929410675/27., -SQRT3/2., .75, 0, 0, 0, 8.929410675/27.,
	0, .5, -26.14263759/27., 15.46619297/27., 0, -SQRT3/4., 22.64018827/27., 0, 0, 0,
	0, 0, .25, -19.96676927/27., 0, 0, -SQRT3/8., 0, 0, 0,
	0, 0, 0, .125, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, .5, -SQRT3/4., 0, -26.14263759/27., 22.64018827/27., 15.46619297/27.,
	0, 0, 0, 0, 0, .25, -13.07131879/27., 0, -13.07131879/27., 0,
	0, 0, 0, 0, 0, 0, .125, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, .25, -SQRT3/8., -19.96676927/27.,
	0, 0, 0, 0, 0, 0, 0, 0, .125, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, .125,
  };

  static const double dgp3_into_dgp3_refined_1[] =
  {
	1., SQRT3/2., 0, -8.929410675/27., -SQRT3/2., -.75, 0, 0, 0, 8.929410675/27.,
	0, .5, 26.14263759/27., 15.46619297/27., 0, -SQRT3/4., -22.64018827/27., 0, 0, 0,
	0, 0, .25, 19.96676927/27., 0, 0, -SQRT3/8., 0, 0, 0,
	0, 0, 0, .125, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, .5, SQRT3/4., 0, -26.14263759/27., -22.64018827/27., 15.46619297/27.,
	0, 0, 0, 0, 0, .25, 13.07131879/27., 0, -13.07131879/27., 0,
	0, 0, 0, 0, 0, 0, .125, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, .25, SQRT3/8., -19.96676927/27.,
	0, 0, 0, 0, 0, 0, 0, 0, .125, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, .125,
  };

  static const double dgp3_into_dgp3_refined_2[] =
  {
	1., SQRT3/2., 0, -8.929410675/27., SQRT3/2., .75, 0, 0, 0, -8.929410675/27.,
	0, .5, 26.14263759/27., 15.46619297/27., 0, SQRT3/4., 22.64018827/27., 0, 0, 0,
	0, 0, .25, 19.96676927/27., 0, 0, SQRT3/8., 0, 0, 0,
	0, 0, 0, .125, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, .5, SQRT3/4., 0, 26.14263759/27., 22.64018827/27., 15.46619297/27.,
	0, 0, 0, 0, 0, .25, 13.07131879/27., 0, 13.07131879/27., 0,
	0, 0, 0, 0, 0, 0, .125, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, .25, SQRT3/8., 19.96676927/27.,
	0, 0, 0, 0, 0, 0, 0, 0, .125, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, .125,
  };

  static const double dgp3_into_dgp3_refined_3[] =
  {
	1., -SQRT3/2., 0, 8.929410675/27., SQRT3/2., -.75, 0, 0, 0, -8.929410675/27.,
	0, .5, -26.14263759/27., 15.46619297/27., 0, SQRT3/4., -22.64018827/27., 0, 0, 0,
	0, 0, .25, -19.96676927/27., 0, 0, SQRT3/8., 0, 0, 0,
	0, 0, 0, .125, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, .5, -SQRT3/4., 0, 26.14263759/27., -22.64018827/27., 15.46619297/27.,
	0, 0, 0, 0, 0, .25, -13.07131879/27., 0, 13.07131879/27., 0,
	0, 0, 0, 0, 0, 0, .125, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, .25, -SQRT3/8., 19.96676927/27.,
	0, 0, 0, 0, 0, 0, 0, 0, .125, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, .125,
  };

  static const double dgp4_into_dgp4_refined_0[] =
  {
	1., -SQRT3/2., 0, 8.929410675/27., 0, -SQRT3/2., .75, 0, -7.733096485/27., 0, 0, 0, 8.929410675/27., -7.733096485/27., 0,
	0, .5, -26.14263759/27., 15.46619297/27., SQRT3/8., 0, -SQRT3/4., 22.64018827/27., -13.39411601/27., 0, 0, 0, 0, 4.464705337/27., 0,
	0, 0, .25, -19.96676927/27., 22.64018827/27., 0, 0, -SQRT3/8., 17.29172942/27., 0, 0, 0, 0, 0, 0,
	0, 0, 0, .125, -13.39411601/27., 0, 0, 0, -SQRT3/16., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1./16., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, .5, -SQRT3/4., 0, 4.464705337/27., -26.14263759/27., 22.64018827/27., 0, 15.46619297/27., -13.39411601/27., SQRT3/8.,
	0, 0, 0, 0, 0, 0, .25, -13.07131879/27., 7.733096485/27., 0, -13.07131879/27., 15./16., 0, 7.733096485/27., 0,
	0, 0, 0, 0, 0, 0, 0, .125, -9.983384634/27., 0, 0, -6.535659397/27., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1./16., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, .25, -SQRT3/8., 0, -19.96676927/27., 17.29172942/27., 22.64018827/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .125, -6.535659397/27., 0, -9.983384634/27., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .125, -SQRT3/16., -13.39411601/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16.,
  };

  static const double dgp4_into_dgp4_refined_1[] =
  {
	1., SQRT3/2., 0, -8.929410675/27., 0., -SQRT3/2., -.75, 0, 7.733096485/27., 0, 0, 0, 8.929410675/27., 7.733096485/27., 0,
	0, .5, 26.14263759/27., 15.46619297/27., -SQRT3/8., 0, -SQRT3/4., -22.64018827/27., -13.39411601/27., 0, 0, 0, 0, 4.464705337/27., 0,
	0, 0, .25, 19.96676927/27., 22.64018827/27., 0, 0, -SQRT3/8., -17.29172942/27., 0, 0, 0, 0, 0, 0,
	0, 0, 0, .125, 13.39411601/27., 0, 0, 0, -SQRT3/16., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1./16., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, .5, SQRT3/4., 0, -4.464705337/27., -26.14263759/27., -22.64018827/27., 0, 15.46619297/27., 13.39411601/27., SQRT3/8.,
	0, 0, 0, 0, 0, 0, .25, 13.07131879/27., 7.733096485/27., 0, -13.07131879/27., -15./16., 0, 7.733096485/27., 0,
	0, 0, 0, 0, 0, 0, 0, .125, 9.983384634/27., 0, 0, -6.535659397/27., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1./16., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, .25, SQRT3/8., 0, -19.96676927/27., -17.29172942/27., 22.64018827/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .125, 6.535659397/27., 0, -9.983384634/27., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .125, SQRT3/16., -13.39411601/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16.,
  };

  static const double dgp4_into_dgp4_refined_2[] =
  {
	1., SQRT3/2., 0, -8.929410675/27., 0., SQRT3/2., .75, 0, -7.733096485/27., 0, 0, 0, -8.929410675/27., -7.733096485/27., 0.,
	0, .5, 26.14263759/27., 15.46619297/27., -SQRT3/8., 0, SQRT3/4., 22.64018827/27., 13.39411601/27., 0, 0, 0, 0, -4.464705337/27., 0,
	0, 0, .25, 19.96676927/27., 22.64018827/27., 0, 0, SQRT3/8., 17.29172942/27., 0, 0, 0, 0, 0, 0,
	0, 0, 0, .125, 13.39411601/27., 0, 0, 0, SQRT3/16., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1./16., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, .5, SQRT3/4., 0, -4.464705337/27., 26.14263759/27., 22.64018827/27., 0, 15.46619297/27., 13.39411601/27., -SQRT3/8.,
	0, 0, 0, 0, 0, 0, .25, 13.07131879/27., 7.733096485/27., 0, 13.07131879/27., 15./16., 0, 7.733096485/27., 0,
	0, 0, 0, 0, 0, 0, 0, .125, 9.983384634/27., 0, 0, 6.535659397/27., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1./16., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, .25, SQRT3/8., 0, 19.96676927/27., 17.29172942/27., 22.64018827/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .125, 6.535659397/27., 0, 9.983384634/27., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .125, SQRT3/16., 13.39411601/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16.,
  };

  static const double dgp4_into_dgp4_refined_3[] =
  {
	1., -SQRT3/2., 0, 8.929410675/27., 0, SQRT3/2., -.75, 0, 7.733096485/27., 0, 0, 0, -8.929410675/27., 7.733096485/27., 0.,
	0, .5, -26.14263759/27., 15.46619297/27., SQRT3/8., 0, SQRT3/4., -22.64018827/27., 13.39411601/27., 0, 0, 0, 0, -4.464705337/27., 0,
	0, 0, .25, -19.96676927/27., 22.64018827/27., 0, 0, SQRT3/8., -17.29172942/27., 0, 0, 0, 0, 0, 0,
	0, 0, 0, .125, -13.39411601/27., 0, 0, 0, SQRT3/16., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1./16., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, .5, -SQRT3/4., 0, 4.464705337/27., 26.14263759/27., -22.64018827/27., 0, 15.46619297/27., -13.39411601/27., -SQRT3/8.,
	0, 0, 0, 0, 0, 0, .25, -13.07131879/27., 7.733096485/27., 0, 13.07131879/27., -15./16., 0, 7.733096485/27., 0,
	0, 0, 0, 0, 0, 0, 0, .125, -9.983384634/27., 0, 0, 6.535659397/27., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1./16., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, .25, -SQRT3/8., 0, 19.96676927/27., -17.29172942/27., 22.64018827/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .125, -6.535659397/27., 0, 9.983384634/27., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .125, -SQRT3/16., 13.39411601/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16.,
  };

  static const double dgp5_into_dgp5_refined_0[] =
  {
	1., -SQRT3/2., 0, 8.929410675/27., 0, -5.596804334/27., -SQRT3/2., .75, 0, -7.733096485/27., 0, 0, 0, 0, 0, 8.929410675/27., -7.733096485/27., 0, 0, 0, -5.596804334/27.,
	0, .5, -26.14263759/27., 15.46619297/27., SQRT3/8., -9.693949466/27., 0, -SQRT3/4., 22.64018827/27., -13.39411601/27., -3./16., 0, 0, 0, 0, 0, 4.464705337/27., -8.645864709/27., 0, 0, 0,
	0, 0, .25, -19.96676927/27., 22.64018827/27., -6.257417473/27., 0, 0, -SQRT3/8., 17.29172942/27., -19.60697819/27., 0, 0, 0, 0, 0, 0, 2.232352669/27., 0, 0, 0,
	0, 0, 0, .125, -13.39411601/27., 22.21162861/27., 0, 0, 0, -SQRT3/16., 11.59964473/27., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1./16., -8.395206501/27., 0, 0, 0, 0, -SQRT3/32., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1./32., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, .5, -SQRT3/4., 0, 4.464705337/27., 0, -26.14263759/27., 22.64018827/27., 0, -8.645864709/27., 15.46619297/27., -13.39411601/27., 0, SQRT3/8., -3./16., -9.693949466/27.,
	0, 0, 0, 0, 0, 0, 0, .25, -13.07131879/27., 7.733096485/27., SQRT3/16., 0, -13.07131879/27., 15./16., -14.97507695/27., 0, 7.733096485/27., -14.97507695/27., 0, SQRT3/16., 0,
	0, 0, 0, 0, 0, 0, 0, 0, .125, -9.983384634/27., 11.32009414/27., 0, 0, -6.535659397/27., 19.33274121/27., 0, 0, 3.866548243/27., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., -6.697058006/27., 0, 0, 0, -3.267829698/27., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .25, -SQRT3/8., 0, 2.232352669/27., -19.96676927/27., 17.29172942/27., 0, 22.64018827/27., -19.60697819/27., -6.257417473/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .125, -6.535659397/27., 3.866548243/27., 0, -9.983384634/27., 19.33274121/27., 0, 11.32009414/27., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., -4.991692317/27., 0, 0, -4.991692317/27., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .125, -SQRT3/16., 0, -13.39411601/27., 11.59964473/27., 22.21162861/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., -3.267829698/27., 0, -6.697058006/27., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., -SQRT3/32., -8.395206501/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32.,
  };

  static const double dgp5_into_dgp5_refined_1[] =
  {
	1., SQRT3/2., 0, -8.929410675/27., 0, 5.596804334/27., -SQRT3/2., -.75, 0, 7.733096485/27., 0, 0, 0, 0, 0, 8.929410675/27., 7.733096485/27., 0, 0, 0, -5.596804334/27.,
	0, .5, 26.14263759/27., 15.46619297/27., -SQRT3/8., -9.693949466/27., 0, -SQRT3/4., -22.64018827/27., -13.39411601/27., 3./16., 0, 0, 0, 0, 0, 4.464705337/27., 8.645864709/27., 0, 0, 0,
	0, 0, .25, 19.96676927/27., 22.64018827/27., 6.257417473/27., 0, 0, -SQRT3/8., -17.29172942/27., -19.60697819/27., 0, 0, 0, 0, 0, 0, 2.232352669/27., 0, 0, 0,
	0, 0, 0, .125, 13.39411601/27., 22.21162861/27., 0, 0, 0, -SQRT3/16., -11.59964473/27., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1./16., 8.395206501/27., 0, 0, 0, 0, -SQRT3/32., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1./32., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, .5, SQRT3/4., 0, -4.464705337/27., 0, -26.14263759/27., -22.64018827/27., 0, 8.645864709/27., 15.46619297/27., 13.39411601/27., 0, SQRT3/8., 3./16., -9.693949466/27.,
	0, 0, 0, 0, 0, 0, 0, .25, 13.07131879/27., 7.733096485/27., -SQRT3/16., 0, -13.07131879/27., -15./16., -14.97507695/27., 0, 7.733096485/27., 14.97507695/27., 0, SQRT3/16., 0,
	0, 0, 0, 0, 0, 0, 0, 0, .125, 9.983384634/27., 11.32009414/27., 0, 0, -6.535659397/27., -19.33274121/27., 0, 0, 3.866548243/27., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., 6.697058006/27., 0, 0, 0, -3.267829698/27., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .25, SQRT3/8., 0, -2.232352669/27., -19.96676927/27., -17.29172942/27., 0, 22.64018827/27., 19.60697819/27., -6.257417473/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .125, 6.535659397/27., 3.866548243/27., 0, -9.983384634/27., -19.33274121/27., 0, 11.32009414/27., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., 4.991692317/27., 0, 0, -4.991692317/27., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .125, SQRT3/16., 0, -13.39411601/27., -11.59964473/27., 22.21162861/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., 3.267829698/27., 0, -6.697058006/27., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., SQRT3/32., -8.395206501/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32.,
  };

  static const double dgp5_into_dgp5_refined_2[] =
  {
	1., SQRT3/2., 0, -8.929410675/27., 0, 5.596804334/27., SQRT3/2., .75, 0, -7.733096485/27., 0, 0, 0, 0, 0, -8.929410675/27., -7.733096485/27., 0, 0, 0, 5.596804334/27.,
	0, .5, 26.14263759/27., 15.46619297/27., -SQRT3/8., -9.693949466/27., 0, SQRT3/4., 22.64018827/27., 13.39411601/27., -3./16., 0, 0, 0, 0, 0, -4.464705337/27., -8.645864709/27., 0, 0, 0,
	0, 0, .25, 19.96676927/27., 22.64018827/27., 6.257417473/27., 0, 0, SQRT3/8., 17.29172942/27., 19.60697819/27., 0, 0, 0, 0, 0, 0, -2.232352669/27., 0, 0, 0,
	0, 0, 0, .125, 13.39411601/27., 22.21162861/27., 0, 0, 0, SQRT3/16., 11.59964473/27., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1./16., 8.395206501/27., 0, 0, 0, 0, SQRT3/32., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1./32., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, .5, SQRT3/4., 0, -4.464705337/27., 0, 26.14263759/27., 22.64018827/27., 0, -8.645864709/27., 15.46619297/27., 13.39411601/27., 0, -SQRT3/8., -3./16., -9.693949466/27.,
	0, 0, 0, 0, 0, 0, 0, .25, 13.07131879/27., 7.733096485/27., -SQRT3/16., 0, 13.07131879/27., 15./16., 14.97507695/27., 0, 7.733096485/27., 14.97507695/27., 0, -SQRT3/16., 0,
	0, 0, 0, 0, 0, 0, 0, 0, .125, 9.983384634/27., 11.32009414/27., 0, 0, 6.535659397/27., 19.33274121/27., 0, 0, 3.866548243/27., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., 6.697058006/27., 0, 0, 0, 3.267829698/27., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .25, SQRT3/8., 0, -2.232352669/27., 19.96676927/27., 17.29172942/27., 0, 22.64018827/27., 19.60697819/27., 6.257417473/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .125, 6.535659397/27., 3.866548243/27., 0, 9.983384634/27., 19.33274121/27., 0, 11.32009414/27., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., 4.991692317/27., 0, 0, 4.991692317/27., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .125, SQRT3/16., 0, 13.39411601/27., 11.59964473/27., 22.21162861/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., 3.267829698/27., 0, 6.697058006/27., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., SQRT3/32., 8.395206501/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32.,
  };

  static const double dgp5_into_dgp5_refined_3[] =
  {
	1., -SQRT3/2., 0, 8.929410675/27., 0, -5.596804334/27., SQRT3/2., -.75, 0, 7.733096485/27., 0, 0, 0, 0, 0, -8.929410675/27., 7.733096485/27., 0, 0, 0, 5.596804334/27.,
	0, .5, -26.14263759/27., 15.46619297/27., SQRT3/8., -9.693949466/27., 0, SQRT3/4., -22.64018827/27., 13.39411601/27., 3./16., 0, 0, 0, 0, 0, -4.464705337/27., 8.645864709/27., 0, 0, 0,
	0, 0, .25, -19.96676927/27., 22.64018827/27., -6.257417473/27., 0, 0, SQRT3/8., -17.29172942/27., 19.60697819/27., 0, 0, 0, 0, 0, 0, -2.232352669/27., 0, 0, 0,
	0, 0, 0, .125, -13.39411601/27., 22.21162861/27., 0, 0, 0, SQRT3/16., -11.59964473/27., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1./16., -8.395206501/27., 0, 0, 0, 0, SQRT3/32., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1./32., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, .5, -SQRT3/4., 0, 4.464705337/27., 0, 26.14263759/27., -22.64018827/27., 0, 8.645864709/27., 15.46619297/27., -13.39411601/27., 0, -SQRT3/8., 3./16., -9.693949466/27.,
	0, 0, 0, 0, 0, 0, 0, .25, -13.07131879/27., 7.733096485/27., SQRT3/16., 0, 13.07131879/27., -15./16., 14.97507695/27., 0, 7.733096485/27., -14.97507695/27., 0, -SQRT3/16., 0,
	0, 0, 0, 0, 0, 0, 0, 0, .125, -9.983384634/27., 11.32009414/27., 0, 0, 6.535659397/27., -19.33274121/27., 0, 0, 3.866548243/27., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., -6.697058006/27., 0, 0, 0, 3.267829698/27., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .25, -SQRT3/8., 0, 2.232352669/27., 19.96676927/27., -17.29172942/27., 0, 22.64018827/27., -19.60697819/27., 6.257417473/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .125, -6.535659397/27., 3.866548243/27., 0, 9.983384634/27., -19.33274121/27., 0, 11.32009414/27., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., -4.991692317/27., 0, 0, 4.991692317/27., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .125, -SQRT3/16., 0, 13.39411601/27., -11.59964473/27., 22.21162861/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., -3.267829698/27., 0, 6.697058006/27., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., -SQRT3/32., 8.395206501/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32.,
  };

  static const double dgp6_into_dgp6_refined_0[] =
  {
	1., -SQRT3/2., 0, 8.929410675/27., 0, -5.596804334/27., 0., -SQRT3/2., .75, 0, -7.733096485/27., 0, 4.846974733/27., 0, 0, 0, 0, 0, 8.929410675/27., -7.733096485/27., 0, 7./64., 0, 0, 0, -5.596804334/27., 4.846974733/27., 0.,
	0, .5, -26.14263759/27., 15.46619297/27., SQRT3/8., -9.693949466/27., -2.634608531/27., 0, -SQRT3/4., 22.64018827/27., -13.39411601/27., -3./16., 8.395206501/27., 0, 0, 0, 0, 0, 0, 4.464705337/27., -8.645864709/27., 5.114962541/27., 0, 0, 0, 0, -2.798402167/27., 0,
	0, 0, .25, -19.96676927/27., 22.64018827/27., -6.257417473/27., -10.20379496/27., 0, 0, -SQRT3/8., 17.29172942/27., -19.60697819/27., 5.419082494/27., 0, 0, 0, 0, 0, 0, 0, 2.232352669/27., -6.603388246/27., 0, 0, 0, 0, 0, 0,
	0, 0, 0, .125, -13.39411601/27., 22.21162861/27., -16.09772402/27., 0, 0, 0, -SQRT3/16., 11.59964473/27., -19.23583463/27., 0, 0, 0, 0, 0, 0, 0, 0, 1.116176334/27., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1./16., -8.395206501/27., 18.25310333/27., 0, 0, 0, 0, -SQRT3/32., 7.2704621/27., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1./32., -5.044891251/27., 0, 0, 0, 0, 0, -SQRT3/64., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 1./64., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, .5, -SQRT3/4., 0, 4.464705337/27., 0, -2.798402167/27., -26.14263759/27., 22.64018827/27., 0, -8.645864709/27., 0, 15.46619297/27., -13.39411601/27., 0, 5.114962541/27., SQRT3/8., -3./16., 0, -9.693949466/27., 8.395206501/27., -2.634608531/27.,
	0, 0, 0, 0, 0, 0, 0, 0, .25, -13.07131879/27., 7.733096485/27., SQRT3/16., -4.846974733/27., 0, -13.07131879/27., 15./16., -14.97507695/27., -5.660047068/27., 0, 7.733096485/27., -14.97507695/27., 21./64., 0, SQRT3/16., -5.660047068/27., 0, -4.846974733/27., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, .125, -9.983384634/27., 11.32009414/27., -3.128708737/27., 0, 0, -6.535659397/27., 19.33274121/27., -21.92126803/27., 0, 0, 3.866548243/27., -11.43740394/27., 0, 0, SQRT3/32., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., -6.697058006/27., 11.1058143/27., 0, 0, 0, -3.267829698/27., 12.96879706/27., 0, 0, 0, 1.933274121/27., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., -4.19760325/27., 0, 0, 0, 0, -1.633914849/27., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./64., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .25, -SQRT3/8., 0, 2.232352669/27., 0, -19.96676927/27., 17.29172942/27., 0, -6.603388246/27., 22.64018827/27., -19.60697819/27., 0, -6.257417473/27., 5.419082494/27., -10.20379496/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .125, -6.535659397/27., 3.866548243/27., SQRT3/32., 0, -9.983384634/27., 19.33274121/27., -11.43740394/27., 0, 11.32009414/27., -21.92126803/27., 0, -3.128708737/27., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., -4.991692317/27., 5.660047068/27., 0, 0, -4.991692317/27., 35./64., 0, 0, 5.660047068/27., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., -3.348529003/27., 0, 0, 0, -2.495846158/27., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./64., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .125, -SQRT3/16., 0, 1.116176334/27., -13.39411601/27., 11.59964473/27., 0, 22.21162861/27., -19.23583463/27., -16.09772402/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., -3.267829698/27., 1.933274121/27., 0, -6.697058006/27., 12.96879706/27., 0, 11.1058143/27., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., -2.495846158/27., 0, 0, -3.348529003/27., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./64., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., -SQRT3/32., 0, -8.395206501/27., 7.2704621/27., 18.25310333/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., -1.633914849/27., 0, -4.19760325/27., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./64., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., -SQRT3/64., -5.044891251/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./64., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./64.,
  };

  static const double dgp6_into_dgp6_refined_1[] =
  {
	1., SQRT3/2., 0, -8.929410675/27., 0, 5.596804334/27., 0., -SQRT3/2., -.75, 0, 7.733096485/27., 0, -4.846974733/27., 0, 0, 0, 0, 0, 8.929410675/27., 7.733096485/27., 0, -7./64., 0, 0, 0, -5.596804334/27., -4.846974733/27., 0.,
	0, .5, 26.14263759/27., 15.46619297/27., -SQRT3/8., -9.693949466/27., 2.634608531/27., 0, -SQRT3/4., -22.64018827/27., -13.39411601/27., 3./16., 8.395206501/27., 0, 0, 0, 0, 0, 0, 4.464705337/27., 8.645864709/27., 5.114962541/27., 0, 0, 0, 0, -2.798402167/27., 0,
	0, 0, .25, 19.96676927/27., 22.64018827/27., 6.257417473/27., -10.20379496/27., 0, 0, -SQRT3/8., -17.29172942/27., -19.60697819/27., -5.419082494/27., 0, 0, 0, 0, 0, 0, 0, 2.232352669/27., 6.603388246/27., 0, 0, 0, 0, 0, 0,
	0, 0, 0, .125, 13.39411601/27., 22.21162861/27., 16.09772402/27., 0, 0, 0, -SQRT3/16., -11.59964473/27., -19.23583463/27., 0, 0, 0, 0, 0, 0, 0, 0, 1.116176334/27., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1./16., 8.395206501/27., 18.25310333/27., 0, 0, 0, 0, -SQRT3/32., -7.2704621/27., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1./32., 5.044891251/27., 0, 0, 0, 0, 0, -SQRT3/64., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0., 0., 1./64., 0, 0, 0, 0, 0., 0., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, .5, SQRT3/4., 0, -4.464705337/27., 0, 2.798402167/27., -26.14263759/27., -22.64018827/27., 0, 8.645864709/27., 0, 15.46619297/27., 13.39411601/27., 0, -5.114962541/27., SQRT3/8., 3./16., 0, -9.693949466/27., -8.395206501/27., -2.634608531/27.,
	0, 0, 0, 0, 0, 0, 0, 0, .25, 13.07131879/27., 7.733096485/27., -SQRT3/16., -4.846974733/27., 0, -13.07131879/27., -15./16., -14.97507695/27., 5.660047068/27., 0, 7.733096485/27., 14.97507695/27., 21./64., 0, SQRT3/16., 5.660047068/27., 0, -4.846974733/27., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, .125, 9.983384634/27., 11.32009414/27., 3.128708737/27., 0, 0, -6.535659397/27., -19.33274121/27., -21.92126803/27., 0, 0, 3.866548243/27., 11.43740394/27., 0, 0, SQRT3/32., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., 6.697058006/27., 11.1058143/27., 0, 0, 0, -3.267829698/27., -12.96879706/27., 0, 0, 0, 1.933274121/27., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., 4.19760325/27., 0, 0, 0, 0, -1.633914849/27., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./64., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .25, SQRT3/8., 0, -2.232352669/27., 0, -19.96676927/27., -17.29172942/27., 0, 6.603388246/27., 22.64018827/27., 19.60697819/27., 0, -6.257417473/27., -5.419082494/27., -10.20379496/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .125, 6.535659397/27., 3.866548243/27., -SQRT3/32., 0, -9.983384634/27., -19.33274121/27., -11.43740394/27., 0, 11.32009414/27., 21.92126803/27., 0, -3.128708737/27., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., 4.991692317/27., 5.660047068/27., 0, 0, -4.991692317/27., -35./64., 0, 0, 5.660047068/27., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., 3.348529003/27., 0, 0, 0, -2.495846158/27., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./64., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .125, SQRT3/16., 0, -1.116176334/27., -13.39411601/27., -11.59964473/27., 0, 22.21162861/27., 19.23583463/27., -16.09772402/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., 3.267829698/27., 1.933274121/27., 0, -6.697058006/27., -12.96879706/27., 0, 11.1058143/27., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., 2.495846158/27., 0, 0, -3.348529003/27., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./64., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., SQRT3/32., 0, -8.395206501/27., -7.2704621/27., 18.25310333/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., 1.633914849/27., 0, -4.19760325/27., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./64., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., SQRT3/64., -5.044891251/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./64., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./64.,
  };

  static const double dgp6_into_dgp6_refined_2[] =
  {
	1., SQRT3/2., 0, -8.929410675/27., 0, 5.596804334/27., 0., SQRT3/2., .75, 0, -7.733096485/27., 0, 4.846974733/27., 0, 0, 0, 0, 0, -8.929410675/27., -7.733096485/27., 0, 7./64., 0, 0, 0, 5.596804334/27., 4.846974733/27., 0.,
	0, .5, 26.14263759/27., 15.46619297/27., -SQRT3/8., -9.693949466/27., 2.634608531/27., 0, SQRT3/4., 22.64018827/27., 13.39411601/27., -3./16., -8.395206501/27., 0, 0, 0, 0, 0, 0, -4.464705337/27., -8.645864709/27., -5.114962541/27., 0, 0, 0, 0, 2.798402167/27., 0,
	0, 0, .25, 19.96676927/27., 22.64018827/27., 6.257417473/27., -10.20379496/27., 0, 0, SQRT3/8., 17.29172942/27., 19.60697819/27., 5.419082494/27., 0, 0, 0, 0, 0, 0, 0, -2.232352669/27., -6.603388246/27., 0, 0, 0, 0, 0, 0,
	0, 0, 0, .125, 13.39411601/27., 22.21162861/27., 16.09772402/27., 0, 0, 0, SQRT3/16., 11.59964473/27., 19.23583463/27., 0, 0, 0, 0, 0, 0, 0, 0, -1.116176334/27., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1./16., 8.395206501/27., 18.25310333/27., 0, 0, 0, 0, SQRT3/32., 7.2704621/27., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1./32., 5.044891251/27., 0, 0, 0, 0, 0, SQRT3/64., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0., 0., 1./64., 0, 0, 0, 0, 0., 0., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, .5, SQRT3/4., 0, -4.464705337/27., 0, 2.798402167/27., 26.14263759/27., 22.64018827/27., 0, -8.645864709/27., 0, 15.46619297/27., 13.39411601/27., 0, -5.114962541/27., -SQRT3/8., -3./16., 0, -9.693949466/27., -8.395206501/27., 2.634608531/27.,
	0, 0, 0, 0, 0, 0, 0, 0, .25, 13.07131879/27., 7.733096485/27., -SQRT3/16., -4.846974733/27., 0, 13.07131879/27., 15./16., 14.97507695/27., -5.660047068/27., 0, 7.733096485/27., 14.97507695/27., 21./64., 0, -SQRT3/16., -5.660047068/27., 0, -4.846974733/27., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, .125, 9.983384634/27., 11.32009414/27., 3.128708737/27., 0, 0, 6.535659397/27., 19.33274121/27., 21.92126803/27., 0, 0, 3.866548243/27., 11.43740394/27., 0, 0, -SQRT3/32., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., 6.697058006/27., 11.1058143/27., 0, 0, 0, 3.267829698/27., 12.96879706/27., 0, 0, 0, 1.933274121/27., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., 4.19760325/27., 0, 0, 0, 0, 1.633914849/27., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./64., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .25, SQRT3/8., 0, -2.232352669/27., 0, 19.96676927/27., 17.29172942/27., 0, -6.603388246/27., 22.64018827/27., 19.60697819/27., 0, 6.257417473/27., 5.419082494/27., -10.20379496/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .125, 6.535659397/27., 3.866548243/27., -SQRT3/32., 0, 9.983384634/27., 19.33274121/27., 11.43740394/27., 0, 11.32009414/27., 21.92126803/27., 0, 3.128708737/27., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., 4.991692317/27., 5.660047068/27., 0, 0, 4.991692317/27., 35./64., 0, 0, 5.660047068/27., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., 3.348529003/27., 0, 0, 0, 2.495846158/27., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./64., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .125, SQRT3/16., 0, -1.116176334/27., 13.39411601/27., 11.59964473/27., 0, 22.21162861/27., 19.23583463/27., 16.09772402/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., 3.267829698/27., 1.933274121/27., 0, 6.697058006/27., 12.96879706/27., 0, 11.1058143/27., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., 2.495846158/27., 0, 0, 3.348529003/27., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./64., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., SQRT3/32., 0, 8.395206501/27., 7.2704621/27., 18.25310333/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., 1.633914849/27., 0, 4.19760325/27., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./64., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., SQRT3/64., 5.044891251/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./64., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0., 0., 0, 0., 0., 1./64.,
  };

  static const double dgp6_into_dgp6_refined_3[] =
  {
	1., -SQRT3/2., 0, 8.929410675/27., 0, -5.596804334/27., 0., SQRT3/2., -.75, 0, 7.733096485/27., 0, -4.846974733/27., 0, 0, 0, 0, 0, -8.929410675/27., 7.733096485/27., 0, -7./64., 0, 0, 0, 5.596804334/27., -4.846974733/27., 0.,
	0, .5, -26.14263759/27., 15.46619297/27., SQRT3/8., -9.693949466/27., -2.634608531/27., 0, SQRT3/4., -22.64018827/27., 13.39411601/27., 3./16., -8.395206501/27., 0, 0, 0, 0, 0, 0, -4.464705337/27., 8.645864709/27., -5.114962541/27., 0, 0, 0, 0, 2.798402167/27., 0,
	0, 0, .25, -19.96676927/27., 22.64018827/27., -6.257417473/27., -10.20379496/27., 0, 0, SQRT3/8., -17.29172942/27., 19.60697819/27., -5.419082494/27., 0, 0, 0, 0, 0, 0, 0, -2.232352669/27., 6.603388246/27., 0, 0, 0, 0, 0, 0,
	0, 0, 0, .125, -13.39411601/27., 22.21162861/27., -16.09772402/27., 0, 0, 0, SQRT3/16., -11.59964473/27., 19.23583463/27., 0, 0, 0, 0, 0, 0, 0, 0, -1.116176334/27., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1./16., -8.395206501/27., 18.25310333/27., 0, 0, 0, 0, SQRT3/32., -7.2704621/27., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1./32., -5.044891251/27., 0, 0, 0, 0, 0, SQRT3/64., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 1./64., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, .5, -SQRT3/4., 0, 4.464705337/27., 0, -2.798402167/27., 26.14263759/27., -22.64018827/27., 0, 8.645864709/27., 0, 15.46619297/27., -13.39411601/27., 0, 5.114962541/27., -SQRT3/8., 3./16., 0, -9.693949466/27., 8.395206501/27., 2.634608531/27.,
	0, 0, 0, 0, 0, 0, 0, 0, .25, -13.07131879/27., 7.733096485/27., SQRT3/16., -4.846974733/27., 0, 13.07131879/27., -15./16., 14.97507695/27., 5.660047068/27., 0, 7.733096485/27., -14.97507695/27., 21./64., 0, -SQRT3/16., 5.660047068/27., 0, -4.846974733/27., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, .125, -9.983384634/27., 11.32009414/27., -3.128708737/27., 0, 0, 6.535659397/27., -19.33274121/27., 21.92126803/27., 0, 0, 3.866548243/27., -11.43740394/27., 0, 0, -SQRT3/32., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., -6.697058006/27., 11.1058143/27., 0, 0, 0, 3.267829698/27., -12.96879706/27., 0, 0, 0, 1.933274121/27., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., -4.19760325/27., 0, 0, 0, 0, 1.633914849/27., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./64., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .25, -SQRT3/8., 0, 2.232352669/27., 0, 19.96676927/27., -17.29172942/27., 0, 6.603388246/27., 22.64018827/27., -19.60697819/27., 0, 6.257417473/27., -5.419082494/27., -10.20379496/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .125, -6.535659397/27., 3.866548243/27., SQRT3/32., 0, 9.983384634/27., -19.33274121/27., 11.43740394/27., 0, 11.32009414/27., -21.92126803/27., 0, 3.128708737/27., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., -4.991692317/27., 5.660047068/27., 0, 0, 4.991692317/27., -35./64., 0, 0, 5.660047068/27., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., -3.348529003/27., 0, 0, 0, 2.495846158/27., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./64., 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .125, -SQRT3/16., 0, 1.116176334/27., 13.39411601/27., -11.59964473/27., 0, 22.21162861/27., -19.23583463/27., 16.09772402/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., -3.267829698/27., 1.933274121/27., 0, 6.697058006/27., -12.96879706/27., 0, 11.1058143/27., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., -2.495846158/27., 0, 0, 3.348529003/27., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./64., 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./16., -SQRT3/32., 0, 8.395206501/27., -7.2704621/27., 18.25310333/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., -1.633914849/27., 0, 4.19760325/27., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./64., 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./32., -SQRT3/64., 5.044891251/27.,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1./64., 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0., 0., 0, 0., 0., 1./64.,
  };
};


template <>
const double * const
FE_DGP<2>::Matrices::embedding[][GeometryInfo<2>::children_per_cell] =
{
  { FE_DGP_2d::dgp0_into_dgp0_refined_0,
      FE_DGP_2d::dgp0_into_dgp0_refined_1,
      FE_DGP_2d::dgp0_into_dgp0_refined_2,
      FE_DGP_2d::dgp0_into_dgp0_refined_3 },
    { FE_DGP_2d::dgp1_into_dgp1_refined_0,
	FE_DGP_2d::dgp1_into_dgp1_refined_1,
	FE_DGP_2d::dgp1_into_dgp1_refined_2,
	FE_DGP_2d::dgp1_into_dgp1_refined_3 },
      { FE_DGP_2d::dgp2_into_dgp2_refined_0,
	  FE_DGP_2d::dgp2_into_dgp2_refined_1,
	  FE_DGP_2d::dgp2_into_dgp2_refined_2,
	  FE_DGP_2d::dgp2_into_dgp2_refined_3 },
	{ FE_DGP_2d::dgp3_into_dgp3_refined_0,
	    FE_DGP_2d::dgp3_into_dgp3_refined_1,
	    FE_DGP_2d::dgp3_into_dgp3_refined_2,
	    FE_DGP_2d::dgp3_into_dgp3_refined_3 },
	  { FE_DGP_2d::dgp4_into_dgp4_refined_0,
	      FE_DGP_2d::dgp4_into_dgp4_refined_1,
	      FE_DGP_2d::dgp4_into_dgp4_refined_2,
	      FE_DGP_2d::dgp4_into_dgp4_refined_3 },
	    { FE_DGP_2d::dgp5_into_dgp5_refined_0,
		FE_DGP_2d::dgp5_into_dgp5_refined_1,
		FE_DGP_2d::dgp5_into_dgp5_refined_2,
		FE_DGP_2d::dgp5_into_dgp5_refined_3 },
	      { FE_DGP_2d::dgp6_into_dgp6_refined_0,
		  FE_DGP_2d::dgp6_into_dgp6_refined_1,
		  FE_DGP_2d::dgp6_into_dgp6_refined_2,
		  FE_DGP_2d::dgp6_into_dgp6_refined_3 }
};

template <>
const unsigned int FE_DGP<2>::Matrices::n_embedding_matrices
= sizeof(FE_DGP<2>::Matrices::embedding) /
sizeof(FE_DGP<2>::Matrices::embedding[0]);


/*
 * These elements are not defined by interpolation, therefore there
 * are no interpolation matrices.
 */

template <>
const double * const FE_DGP<2>::Matrices::projection_matrices[][GeometryInfo<2>::children_per_cell];

template <>
const unsigned int FE_DGP<2>::Matrices::n_projection_matrices
= 0;


#else // #if deal_II_dimension
// On gcc2.95 on Alpha OSF1, the native assembler does not like empty
// files, so provide some dummy code
namespace { void dummy () {}; };
#endif // #if deal_II_dimension == 2
