//---------------------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 2002, 2003, 2004, 2005 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------

#include <fe/fe_dgp.h>
#include <fe/fe_tools.h>

#ifdef HAVE_STD_STRINGSTREAM
#  include <sstream>
#else
#  include <strstream>
#endif


template <int dim>
FE_DGP<dim>::FE_DGP (const unsigned int degree)
		:
		FE_Poly<PolynomialSpace<dim>, dim> (
		  PolynomialSpace<dim>(Polynomials::Legendre::generate_complete_basis(degree)),
		  FiniteElementData<dim>(get_dpo_vector(degree), 1, degree, FiniteElementData<dim>::L2),
		  std::vector<bool>(FiniteElementData<dim>(get_dpo_vector(degree), 1, degree).dofs_per_cell,true),
		  std::vector<std::vector<bool> >(FiniteElementData<dim>(
		    get_dpo_vector(degree), 1, degree).dofs_per_cell, std::vector<bool>(1,true)))
{
  for (unsigned int i=0; i<GeometryInfo<dim>::children_per_cell; ++i)
    this->prolongation[i].reinit (this->dofs_per_cell,
				  this->dofs_per_cell);
  FETools::compute_embedding_matrices (*this, &this->prolongation[0]);
				   // Fill restriction matrices with L2-projection
  for (unsigned int i=0; i<GeometryInfo<dim>::children_per_cell; ++i)
    this->restriction[i].reinit (this->dofs_per_cell,
				  this->dofs_per_cell);
  FETools::compute_projection_matrices (*this, &this->restriction[0]);
}



template <int dim>
std::string
FE_DGP<dim>::get_name () const
{
				   // note that the
				   // FETools::get_fe_from_name
				   // function depends on the
				   // particular format of the string
				   // this function returns, so they
				   // have to be kept in synch

#ifdef HAVE_STD_STRINGSTREAM
  std::ostringstream namebuf;
#else
  std::ostrstream namebuf;
#endif
  
  namebuf << "FE_DGP<" << dim << ">(" << this->degree << ")";

#ifndef HAVE_STD_STRINGSTREAM
  namebuf << std::ends;
#endif
  return namebuf.str();
}



template <int dim>
FiniteElement<dim> *
FE_DGP<dim>::clone() const
{
  return new FE_DGP<dim>(this->degree);
}



//---------------------------------------------------------------------------
// Auxiliary functions
//---------------------------------------------------------------------------


template <int dim>
std::vector<unsigned int>
FE_DGP<dim>::get_dpo_vector (const unsigned int deg)
{
  std::vector<unsigned int> dpo(dim+1, 0U);
  dpo[dim] = deg+1;
  for (unsigned int i=1;i<dim;++i)
    {
      dpo[dim] *= deg+1+i;
      dpo[dim] /= i+1;
    }
  return dpo;
}


template <int dim>
bool
FE_DGP<dim>::has_support_on_face (const unsigned int,
				  const unsigned int) const
{
                                   // all shape functions have support on all
                                   // faces
  return true;
}



template <int dim>
unsigned int
FE_DGP<dim>::memory_consumption () const
{
  Assert (false, ExcNotImplemented ());
  return 0;
}



template class FE_DGP<deal_II_dimension>;
