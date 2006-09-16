//----------------------------  face_interpolation.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2005, 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  face_interpolation.cc  ---------------------------

#include "../tests.h"
#include "fe_tools_common.cc"

// check
//   FE::face_interpolation


std::string output_file_name = "face_interpolation/output";



template <int dim>
void
check_this (const FiniteElement<dim> &fe1,
            const FiniteElement<dim> &fe2)
{
  if (dim == 1)
    return;

				   // check all combinations of fe1 and fe2
  FullMatrix<double> face_constraints;
  try
    {
      face_constraints.reinit (fe1.dofs_per_face,
			       fe1.dofs_per_face);
      fe1.get_face_interpolation_matrix (fe1, face_constraints);

      deallog << fe1.get_name()
	      << "  vs.  "
	      << fe1.get_name()
	      << std::endl;
      output_matrix (face_constraints);      
    }
  catch (...)
    {
    }

  try
    {
      face_constraints.reinit (fe2.dofs_per_face,
			       fe2.dofs_per_face);
      fe2.get_face_interpolation_matrix (fe2, face_constraints);

      deallog << fe2.get_name()
	      << "  vs.  "
	      << fe2.get_name()
	      << std::endl;
      output_matrix (face_constraints);      
    }
  catch (...)
    {
    }

  if (fe1.dofs_per_face <= fe2.dofs_per_face)
    try
      {
	face_constraints.reinit (fe2.dofs_per_face,
				 fe1.dofs_per_face);
	fe1.get_face_interpolation_matrix (fe2, face_constraints);

	deallog << fe1.get_name()
		<< "  vs.  "
		<< fe2.get_name()
		<< std::endl;
	output_matrix (face_constraints);      
      }
    catch (...)
      {
      }

  if (fe2.dofs_per_face <= fe1.dofs_per_face)
    try
      {
	face_constraints.reinit (fe1.dofs_per_face,
				 fe2.dofs_per_face);
	fe2.get_face_interpolation_matrix (fe1, face_constraints);

	deallog << fe2.get_name()
		<< "  vs.  "
		<< fe1.get_name()
		<< std::endl;
	output_matrix (face_constraints);      
      }
    catch (...)
      {
      }
}

