//-------------------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 2003, 2005, 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//-------------------------------------------------------------------------

/**
 * @defgroup feall Finite elements
 *
 * All classes related to shape functions and to access to shape
 * functions.  This concerns the actual values of finite elements. For
 * the numbering of degrees of freedom refer to the module on @ref dofs.
 *
 * The classes and functions of this module fall into several sub-groups that
 * are discussed in their respective sub-modules listed above. In addition,
 * the FETools class provides functions that provide information on finite
 * elements, transformations between elements, etc.
 */


/**
 * @defgroup febase Base classes
 *
 * The members of this sub-module describe the implementational mechanics of
 * finite element classes, without actually implementing a concrete
 * element. For example, the FiniteElement base class declares the virtual
 * functions a derived class has to implement if it wants to describe a finite
 * element space. Likewise, the FiniteElementData holds variables that
 * describe certain values characterizing a finite element, such as the number
 * of degrees of freedom per vertex, line, or face.
 *
 * On the other hand, classes like FE_Poly and FE_PolyTensor are higher
 * abstractions. They describe finite elements that are built atop polynomial
 * descriptions of the shape functions on the unit cell. Classes derived from
 * them then only have to provide a description of the particular polynomial
 * from which a finite element is built. For example, the FE_Q class that
 * implements the usual Lagrange elements uses the FE_Poly base class to
 * generate a finite element by providing it with a set of Lagrange
 * interpolation polynomials corresponding to an equidistant subdivision of
 * interpolation points.
 *
 * Finally, the FESystem class is used for vector-valued problems. There, one
 * may want to couple a number of scalar (or also vector-valued) base elements
 * together to form the joint finite element of a vector-valued operator. As
 * an example, for 3d Navier-Stokes flow, one may want to use three Q1
 * elements for the three components of the velocity, and a piecewise constant
 * Q0 element for the pressure. The FESystem class can be used to couple these
 * four base elements together into a single, vector-valued element with 4
 * vector components. The step-8, step-17, and step-18 tutorial programs give
 * an introduction into the use of this class in the context of the
 * vector-valued elasticity (Lam&eacute;) equations. step-20 discusses a mixed
 * Laplace discretization that also uses vector-valued elements.
 * 
 * @ingroup feall
 */


/**
 * @defgroup feaccess Finite element access classes (FEValues etc.)
 *
 * @ingroup feall
 */


/**
 * @defgroup fe Finite element shape functions
 *
 * @ingroup feall
 */


/**
 * @defgroup mapping Mappings between reference and real cell
 *
 * @ingroup feall
 */
