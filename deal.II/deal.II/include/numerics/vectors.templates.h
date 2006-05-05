//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------

#ifndef _deal2__vectors_templates_h
#define _deal2__vectors_templates_h

#include <base/function.h>
#include <base/quadrature.h>
#include <lac/vector.h>
#include <lac/block_vector.h>
#include <lac/sparse_matrix.h>
#include <lac/precondition.h>
#include <lac/solver_cg.h>
#include <lac/vector_memory.h>
#include <grid/tria_iterator.h>
#include <grid/grid_tools.h>
#include <dofs/dof_handler.h>
#include <dofs/dof_accessor.h>
#include <dofs/dof_constraints.h>
#include <dofs/dof_tools.h>
#include <fe/fe.h>
#include <fe/hp_fe_values.h>
#include <fe/mapping_q1.h>
#include <fe/mapping_collection.h>
#include <fe/q_collection.h>
#include <numerics/vectors.h>
#include <numerics/matrices.h>

#include <numeric>
#include <algorithm>
#include <vector>
#include <cmath>

template <int dim>
inline double sqr_point (const Tensor<1,dim> &p)
{
  return p * p;
}


template <int dim, class VECTOR>
void VectorTools::interpolate (const Mapping<dim>    &mapping,
			       const DoFHandler<dim> &dof,
			       const Function<dim>   &function,
			       VECTOR                &vec)
{
  Assert (dof.get_fe().n_components() == function.n_components,
	  ExcComponentMismatch());
  
  const FiniteElement<dim> &fe           = dof.get_fe();
  const unsigned int        n_components = fe.n_components();
  const bool                fe_is_system = (n_components != 1);
  
  typename DoFHandler<dim>::active_cell_iterator cell = dof.begin_active(),
						 endc = dof.end();

				   // For FESystems many of the
				   // unit_support_points will
				   // appear multiply, as a point
				   // may be unit_support_point
				   // for several of the components
				   // of the system.
				   // The following is rather
				   // complicated as it is
				   // avoided to evaluate
				   // the vectorfunction multiply at
				   // the same point on a cell.
  const std::vector<Point<dim> > &
    unit_support_points = fe.get_unit_support_points();
  Assert (unit_support_points.size() != 0,
	  ExcNonInterpolatingFE());

				   // Find the support points 
				   // on a cell that
				   // are multiply mentioned in 
				   // @p{unit_support_points}.
				   // Mark the first representative
				   // of each multiply mentioned
				   // support point by appending its
				   // dof index to @p{dofs_of_rep_points}.
				   // Each multiple point gets to know
				   // the dof index of its representative
				   // point by the @p{dof_to_rep_dof_table}.

				   // the following vector collects all dofs i,
				   // 0<=i<fe.dofs_per_cell, for that
				   // unit_support_points[i] 
				   // is a representative one. i.e.
				   // the following vector collects all rep dofs.
				   // the position of a rep dof within this vector
				   // is called rep index.
  std::vector<unsigned int> dofs_of_rep_points;
				   // the following table converts a dof i
				   // to the rep index.
  std::vector<unsigned int> dof_to_rep_index_table;
  unsigned int n_rep_points=0;
  for (unsigned int i=0; i<fe.dofs_per_cell; ++i)
    {
      bool representative=true;
				       // the following loop is looped
				       // the other way round to get
				       // the minimal effort of
				       // O(fe.dofs_per_cell) for multiple
				       // support points that are placed
				       // one after the other.
      for (unsigned int j=dofs_of_rep_points.size(); j>0; --j)
	if (unit_support_points[i] 
	    == unit_support_points[dofs_of_rep_points[j-1]])
	  {
	    dof_to_rep_index_table.push_back(j-1);
	    representative=false;
	    break;
	  }
      
      if (representative)
	{
					   // rep_index=dofs_of_rep_points.size()
	  dof_to_rep_index_table.push_back(dofs_of_rep_points.size());
					   // dofs_of_rep_points[rep_index]=i
	  dofs_of_rep_points.push_back(i);
	  ++n_rep_points;
	}
    }
  Assert(dofs_of_rep_points.size()==n_rep_points, ExcInternalError());
  Assert(dof_to_rep_index_table.size()==fe.dofs_per_cell, ExcInternalError());

  std::vector<unsigned int> dofs_on_cell (fe.dofs_per_cell);
  std::vector<Point<dim> >  rep_points (n_rep_points);

				   // get space for the values of the
				   // function at the rep support points.
				   //
				   // have two versions, one for system fe
				   // and one for scalar ones, to take the
				   // more efficient one respectively
  std::vector<double>          function_values_scalar (n_rep_points);
  std::vector<Vector<double> > function_values_system (n_rep_points,
						  Vector<double>(fe.n_components()));

				   // Make a quadrature rule from support points
				   // to feed it into FEValues
  Quadrature<dim> support_quadrature(unit_support_points);

				   // Transformed support points are computed by
				   // FEValues
  FEValues<dim> fe_values (mapping, fe, support_quadrature, update_q_points);
  
  for (; cell!=endc; ++cell)
    {
				       // for each cell:
				       // get location of finite element
				       // support_points
      fe_values.reinit(cell);
      const std::vector<Point<dim> >& support_points =
	fe_values.get_quadrature_points();
      
				       // pick out the representative
				       // support points
      for (unsigned int j=0; j<dofs_of_rep_points.size(); ++j)
	rep_points[j]=support_points[dofs_of_rep_points[j]];

				       // get indices of the dofs on this cell
      cell->get_dof_indices (dofs_on_cell);


      if (fe_is_system)
	{
					   // get function values at
					   // these points. Here: get
					   // all components
	  function.vector_value_list (rep_points, function_values_system);
					   // distribute the function
					   // values to the global
					   // vector
	  for (unsigned int i=0; i<fe.dofs_per_cell; ++i)
	    {
	      const unsigned int component
		= fe.system_to_component_index(i).first;
	      const unsigned int rep_dof=dof_to_rep_index_table[i];
	      vec(dofs_on_cell[i])
		= function_values_system[rep_dof](component);
	    };
	}
      
      else
	{
					   // get first component only,
					   // which is the only component
					   // in the function anyway
	  function.value_list (rep_points, function_values_scalar, 0);
					   // distribute the function
					   // values to the global
					   // vector
	  for (unsigned int i=0; i<fe.dofs_per_cell; ++i)
	    vec(dofs_on_cell[i]) 
	      = function_values_scalar[dof_to_rep_index_table[i]];
	};
    }
}


template <int dim, class VECTOR>
void VectorTools::interpolate (const DoFHandler<dim> &dof,
			       const Function<dim>   &function,
			       VECTOR                &vec)
{
  Assert (DEAL_II_COMPAT_MAPPING, ExcCompatibility("mapping"));
  static const MappingQ1<dim> mapping;
  interpolate(mapping, dof, function, vec);
}




template <int dim, class InVector, class OutVector>
void
VectorTools::interpolate (const DoFHandler<dim>           &dof_1,
			  const DoFHandler<dim>           &dof_2,
			  const FullMatrix<double>        &transfer,
			  const InVector                  &data_1,
			  OutVector                       &data_2)
{
  Vector<double> cell_data_1(dof_1.get_fe().dofs_per_cell);
  Vector<double> cell_data_2(dof_2.get_fe().dofs_per_cell);

  std::vector<short unsigned int> touch_count (dof_2.n_dofs(), 0);
  std::vector<unsigned int>       local_dof_indices (dof_2.get_fe().dofs_per_cell);
  
  typename DoFHandler<dim>::active_cell_iterator h = dof_1.begin_active();
  typename DoFHandler<dim>::active_cell_iterator l = dof_2.begin_active();
  const typename DoFHandler<dim>::cell_iterator endh = dof_1.end();
  
  for(; h != endh; ++h, ++l)
  {
    h->get_dof_values(data_1, cell_data_1);
    transfer.vmult(cell_data_2, cell_data_1);

    l->get_dof_indices (local_dof_indices);
  
				   // distribute cell vector
    for (unsigned int j=0; j<dof_2.get_fe().dofs_per_cell; ++j) 
      {
	data_2(local_dof_indices[j]) += cell_data_2(j);

					 // count, how often we have
					 // added to this dof
	Assert (touch_count[local_dof_indices[j]] < 255,
		ExcInternalError());	
	++touch_count[local_dof_indices[j]];
      };
  };

				   // compute the mean value of the
				   // sum which we have placed in each
				   // entry of the output vector
  for (unsigned int i=0; i<dof_2.n_dofs(); ++i)
    {
      Assert (touch_count[i] != 0,
	      ExcInternalError());
      
      data_2(i) /= touch_count[i];
    };
}


#if deal_II_dimension == 1

template <class VECTOR>
void VectorTools::project (const Mapping<1>       &,
			   const DoFHandler<1>    &,
			   const ConstraintMatrix &,
			   const Quadrature<1>    &,
			   const Function<1>      &,
			   VECTOR                 &,
			   const bool              ,
			   const Quadrature<0>    &,
			   const bool              )
{
				   // this function should easily be
				   // implemented using the template
				   // below. However some changes have
				   // to be made since faces don't
				   // exist in 1D. Maybe integrate the
				   // creation of zero boundary values
				   // into the project_boundary_values
				   // function?
  Assert (false, ExcNotImplemented());
}


#endif


template <int dim, class VECTOR>
void VectorTools::project (const Mapping<dim>       &mapping,
			   const DoFHandler<dim>    &dof,
			   const ConstraintMatrix   &constraints,
			   const Quadrature<dim>    &quadrature,
			   const Function<dim>      &function,
			   VECTOR                   &vec_result,
			   const bool                enforce_zero_boundary,
			   const Quadrature<dim-1>  &q_boundary,
			   const bool                project_to_boundary_first)
{
  Assert (dof.get_fe().n_components() == function.n_components,
	  ExcInvalidFE());

  Assert (vec_result.size() == dof.n_dofs(),
          ExcDimensionMismatch (vec_result.size(), dof.n_dofs()));
  
  const FiniteElement<dim> &fe = dof.get_fe();

				   // make up boundary values
  std::map<unsigned int,double> boundary_values;

  if (enforce_zero_boundary == true) 
				     // no need to project boundary
				     // values, but enforce
				     // homogeneous boundary values
				     // anyway
    {
				       // loop over all boundary faces
				       // to get all dof indices of
				       // dofs on the boundary. note
				       // that in 3d there are cases
				       // where a face is not at the
				       // boundary, yet one of its
				       // lines is, and we should
				       // consider the degrees of
				       // freedom on it as boundary
				       // nodes. likewise, in 2d and
				       // 3d there are cases where a
				       // cell is only at the boundary
				       // by one vertex. nevertheless,
				       // since we do not support
				       // boundaries with dimension
				       // less or equal to dim-2, each
				       // such boundary dof is also
				       // found from some other face
				       // that is actually wholly on
				       // the boundary, not only by
				       // one line or one vertex
      typename DoFHandler<dim>::active_face_iterator face = dof.begin_active_face(),
						     endf = dof.end_face();
      std::vector<unsigned int> face_dof_indices (fe.dofs_per_face);
      for (; face!=endf; ++face)
	if (face->at_boundary())
	  {
	    face->get_dof_indices (face_dof_indices);
	    for (unsigned int i=0; i<fe.dofs_per_face; ++i)
					       // enter zero boundary values
					       // for all boundary nodes
					       //
					       // we need not care about
					       // vector valued elements here,
					       // since we set all components
	      boundary_values[face_dof_indices[i]] = 0.;
	  };
    }
  else
				     // no homogeneous boundary values
    if (project_to_boundary_first == true)
				       // boundary projection required
      {
					 // set up a list of boundary functions for
					 // the different boundary parts. We want the
					 // @p{function} to hold on all parts of the
					 // boundary
	typename FunctionMap<dim>::type boundary_functions;
	for (unsigned char c=0; c<255; ++c)
	  boundary_functions[c] = &function;
	project_boundary_values (dof, boundary_functions, q_boundary,
				 boundary_values);
      };


				   // set up mass matrix and right hand side
  Vector<double> vec (dof.n_dofs());
  SparsityPattern sparsity(dof.n_dofs(),
			   dof.n_dofs(),
			   dof.max_couplings_between_dofs());
  DoFTools::make_sparsity_pattern (dof, sparsity);
  constraints.condense (sparsity);
  
  SparseMatrix<double> mass_matrix (sparsity);
  Vector<double> tmp (mass_matrix.n());

  MatrixCreator::create_mass_matrix (mapping, dof, quadrature, mass_matrix);
  
  VectorTools::create_right_hand_side (mapping, dof, quadrature, function, tmp);

  constraints.condense (mass_matrix);
  constraints.condense (tmp);
  if (boundary_values.size() != 0)
    MatrixTools::apply_boundary_values (boundary_values,
					mass_matrix, vec, tmp,
					true);

  SolverControl           control(1000,1e-16);
  PrimitiveVectorMemory<> memory;
  SolverCG<>              cg(control,memory);

  PreconditionSSOR<> prec;
  prec.initialize(mass_matrix, 1.2);
				   // solve
  cg.solve (mass_matrix, vec, tmp, prec);
  
				   // distribute solution
  constraints.distribute (vec);

                                   // copy vec into vec_result. we
                                   // can't use ve_result itself
                                   // above, since it may be of
                                   // another type than Vector<double>
                                   // and that wouldn't necessarily go
                                   // together with the matrix and
                                   // other functions
  std::copy (vec.begin(), vec.end(), vec_result.begin());
}


template <int dim, class VECTOR>
void VectorTools::project (const DoFHandler<dim>    &dof,
			   const ConstraintMatrix   &constraints,
			   const Quadrature<dim>    &quadrature,
			   const Function<dim>      &function,
			   VECTOR                   &vec,
			   const bool                enforce_zero_boundary,
			   const Quadrature<dim-1>  &q_boundary,
			   const bool                project_to_boundary_first)
{
  Assert (DEAL_II_COMPAT_MAPPING, ExcCompatibility("mapping"));
  static const MappingQ1<dim> mapping;
  project(mapping, dof, constraints, quadrature, function, vec,
	  enforce_zero_boundary, q_boundary, project_to_boundary_first);
}




template <int dim>
void VectorTools::create_right_hand_side (const Mapping<dim>    &mapping,
					  const DoFHandler<dim> &dof_handler,
					  const Quadrature<dim> &quadrature,
					  const Function<dim>   &rhs_function,
					  Vector<double>        &rhs_vector)
{
  const FiniteElement<dim> &fe  = dof_handler.get_fe();
  Assert (fe.n_components() == rhs_function.n_components,
	  ExcComponentMismatch());
  Assert (rhs_vector.size() == dof_handler.n_dofs(),
	  ExcDimensionMismatch(rhs_vector.size(), dof_handler.n_dofs()));
  rhs_vector = 0;
  
  UpdateFlags update_flags = UpdateFlags(update_values   |
					 update_q_points |
					 update_JxW_values);
  FEValues<dim> fe_values (mapping, fe, quadrature, update_flags);

  const unsigned int dofs_per_cell = fe_values.dofs_per_cell,
		     n_q_points    = fe_values.n_quadrature_points,
		     n_components  = fe.n_components();
  
  std::vector<unsigned int> dofs (dofs_per_cell);
  Vector<double> cell_vector (dofs_per_cell);

  typename DoFHandler<dim>::active_cell_iterator cell = dof_handler.begin_active(),
						 endc = dof_handler.end();

  if (n_components==1)
    {
      std::vector<double> rhs_values(n_q_points);
      
      for (; cell!=endc; ++cell) 
	{
	  fe_values.reinit(cell);
	  
	  const std::vector<double> &weights   = fe_values.get_JxW_values ();
	  rhs_function.value_list (fe_values.get_quadrature_points(), rhs_values);
	  
	  cell_vector = 0;
	  for (unsigned int point=0; point<n_q_points; ++point)
	    for (unsigned int i=0; i<dofs_per_cell; ++i) 
	      cell_vector(i) += rhs_values[point] *
				fe_values.shape_value(i,point) *
				weights[point];
	
	  cell->get_dof_indices (dofs);
	  
	  for (unsigned int i=0; i<dofs_per_cell; ++i)
	    rhs_vector(dofs[i]) += cell_vector(i);
	}
      
    }
  else
    {
      std::vector<Vector<double> > rhs_values(n_q_points, Vector<double>(n_components));
      
      // Use the faster code if the FiniteElement is primitive
      if (fe.is_primitive ())
	{
	  for (; cell!=endc; ++cell) 
	    {
	      fe_values.reinit(cell);
	      
	      const std::vector<double> &weights   = fe_values.get_JxW_values ();
	      rhs_function.vector_value_list (fe_values.get_quadrature_points(), rhs_values);
	      
	      cell_vector = 0;
	      for (unsigned int point=0; point<n_q_points; ++point)
		for (unsigned int i=0; i<dofs_per_cell; ++i)
		  {
		    const unsigned int component
		      = fe.system_to_component_index(i).first;
		    
		    cell_vector(i) += rhs_values[point](component) *
		                      fe_values.shape_value(i,point) *
		                      weights[point];
		  }
	      
	      cell->get_dof_indices (dofs);
	      
	      for (unsigned int i=0; i<dofs_per_cell; ++i)
		rhs_vector(dofs[i]) += cell_vector(i);
	    }
	}
      else
	// Otherwise do it the way proposed for vector valued elements
	{
	  for (; cell!=endc; ++cell) 
	    {
	      fe_values.reinit(cell);
	      
	      const std::vector<double> &weights   = fe_values.get_JxW_values ();
	      rhs_function.vector_value_list (fe_values.get_quadrature_points(), rhs_values);
	      
	      cell_vector = 0;
	      for (unsigned int point=0; point<n_q_points; ++point)
		for (unsigned int i=0; i<dofs_per_cell; ++i)
		  for (unsigned int comp_i = 0; comp_i < n_components; ++comp_i)
		    if (fe.get_nonzero_components(i)[comp_i])
		      {
			cell_vector(i) += rhs_values[point](comp_i) *
			                  fe_values.shape_value_component(i,point,comp_i) *
			                  weights[point];
		      }
	      
	      cell->get_dof_indices (dofs);
	      
	      for (unsigned int i=0; i<dofs_per_cell; ++i)
		rhs_vector(dofs[i]) += cell_vector(i);
	    }
	}
    }
}


template <int dim>
void VectorTools::create_right_hand_side (const DoFHandler<dim>    &dof_handler,
					  const Quadrature<dim>    &quadrature,
					  const Function<dim>      &rhs_function,
					  Vector<double>           &rhs_vector)
{
  Assert (DEAL_II_COMPAT_MAPPING, ExcCompatibility("mapping"));
  static const MappingQ1<dim> mapping;
  create_right_hand_side(mapping, dof_handler, quadrature,
			 rhs_function, rhs_vector);
}



#if deal_II_dimension != 1

template <int dim>
void
VectorTools::create_boundary_right_hand_side (const Mapping<dim>      &mapping,
					      const DoFHandler<dim>   &dof_handler,
					      const Quadrature<dim-1> &quadrature,
					      const Function<dim>     &rhs_function,
					      Vector<double>          &rhs_vector,
					      const std::set<unsigned char> &boundary_indicators)
{
  const FiniteElement<dim> &fe  = dof_handler.get_fe();
  Assert (fe.n_components() == rhs_function.n_components,
	  ExcComponentMismatch());
  Assert (rhs_vector.size() == dof_handler.n_dofs(),
	  ExcDimensionMismatch(rhs_vector.size(), dof_handler.n_dofs()));
  
  rhs_vector = 0;
  
  UpdateFlags update_flags = UpdateFlags(update_values   |
					 update_q_points |
					 update_JxW_values);
  FEFaceValues<dim> fe_values (mapping, fe, quadrature, update_flags);

  const unsigned int dofs_per_cell = fe_values.dofs_per_cell,
		     n_q_points    = fe_values.n_quadrature_points,
		     n_components  = fe.n_components();
  
  std::vector<unsigned int> dofs (dofs_per_cell);
  Vector<double> cell_vector (dofs_per_cell);

  typename DoFHandler<dim>::active_cell_iterator cell = dof_handler.begin_active(),
						 endc = dof_handler.end();

  if (n_components==1)
    {
      std::vector<double> rhs_values(n_q_points);
      
      for (; cell!=endc; ++cell)
	for (unsigned int face=0; face<GeometryInfo<dim>::faces_per_cell; ++face)
	  if (cell->face(face)->at_boundary () &&
	      (boundary_indicators.find (cell->face(face)->boundary_indicator())
	       !=
	       boundary_indicators.end()))
	    {
	      fe_values.reinit(cell, face);
	  
	      const std::vector<double> &weights   = fe_values.get_JxW_values ();
	      rhs_function.value_list (fe_values.get_quadrature_points(), rhs_values);
	      
	      cell_vector = 0;
	      for (unsigned int point=0; point<n_q_points; ++point)
		for (unsigned int i=0; i<dofs_per_cell; ++i) 
		  cell_vector(i) += rhs_values[point] *
				    fe_values.shape_value(i,point) *
				    weights[point];
	
	      cell->get_dof_indices (dofs);
	  
	      for (unsigned int i=0; i<dofs_per_cell; ++i)
		rhs_vector(dofs[i]) += cell_vector(i);
	    }
    }
  else
    {
      std::vector<Vector<double> > rhs_values(n_q_points, Vector<double>(n_components));
      
      for (; cell!=endc; ++cell) 
	for (unsigned int face=0; face<GeometryInfo<dim>::faces_per_cell; ++face)
	  if (cell->face(face)->at_boundary () &&
	      (boundary_indicators.find (cell->face(face)->boundary_indicator())
	       !=
	       boundary_indicators.end()))
	    {
	      fe_values.reinit(cell, face);
	      
	      const std::vector<double> &weights   = fe_values.get_JxW_values ();
	      rhs_function.vector_value_list (fe_values.get_quadrature_points(), rhs_values);
	      
	      cell_vector = 0;
	      
	      // Use the faster code if the FiniteElement is primitive
	      if (fe.is_primitive ())
		{		  
		  for (unsigned int point=0; point<n_q_points; ++point)
		    for (unsigned int i=0; i<dofs_per_cell; ++i)
		      {
			const unsigned int component
			  = fe.system_to_component_index(i).first;
			
			cell_vector(i) += rhs_values[point](component) *
				          fe_values.shape_value(i,point) *
				          weights[point];
		      }
		}
	      else
		{
		  // And the full featured code, if vector valued FEs are used
		  for (unsigned int point=0; point<n_q_points; ++point)
		    for (unsigned int i=0; i<dofs_per_cell; ++i)
		      for (unsigned int comp_i = 0; comp_i < n_components; ++comp_i)
			if (fe.get_nonzero_components(i)[comp_i])
			  {
			    cell_vector(i) += rhs_values[point](comp_i) *
			                      fe_values.shape_value_component(i,point,comp_i) *
			                      weights[point];
			  }
		}
		  
	      cell->get_dof_indices (dofs);
	      
	      for (unsigned int i=0; i<dofs_per_cell; ++i)
		rhs_vector(dofs[i]) += cell_vector(i);
	    }
    }
}

#else

void
VectorTools::create_boundary_right_hand_side (const Mapping<1>    &,
					      const DoFHandler<1> &,
					      const Quadrature<0> &,
					      const Function<1>   &,
					      Vector<double>      &,
					      const std::set<unsigned char> &)
{
  Assert (false, ExcImpossibleInDim(1));
}

#endif

template <int dim>
void
VectorTools::create_boundary_right_hand_side (const DoFHandler<dim>   &dof_handler,
					      const Quadrature<dim-1> &quadrature,
					      const Function<dim>     &rhs_function,
					      Vector<double>          &rhs_vector,
					      const std::set<unsigned char> &boundary_indicators)
{
  Assert (DEAL_II_COMPAT_MAPPING, ExcCompatibility("mapping"));
  static const MappingQ1<dim> mapping;
  create_boundary_right_hand_side(mapping, dof_handler, quadrature,
				  rhs_function, rhs_vector,
				  boundary_indicators);
}



#if deal_II_dimension == 1

//TODO[?] Actually the Mapping object should be a MappingCollection object for the
// hp::DoFHandler.
template <template <int> class DH>
void
VectorTools::interpolate_boundary_values (const Mapping<1>         &,
					  const DH<1>              &dof,
					  const unsigned char       boundary_component,
					  const Function<1>        &boundary_function,
					  std::map<unsigned int,double> &boundary_values,
					  const std::vector<bool>       &component_mask_)
{
  Assert (boundary_component != 255,
	  ExcInvalidBoundaryIndicator());

				   // check whether boundary values at
				   // the left or right boundary of
				   // the line are
				   // requested. @p{direction} denotes
				   // the neighboring direction in
				   // which we seek the boundary,
				   // i.e. 0 is left boundary and 1 is
				   // right.
  const unsigned int direction = boundary_component;
  Assert (direction < 2, ExcInvalidBoundaryIndicator());
  
				   // first find the outermost active
				   // cell by first traversing the coarse
				   // grid to its end and then going
				   // to the children
  typename DH<1>::cell_iterator outermost_cell = dof.begin(0);
  while (outermost_cell->neighbor(direction).state() == IteratorState::valid)
    outermost_cell = outermost_cell->neighbor(direction);
  
  while (outermost_cell->has_children())
    outermost_cell = outermost_cell->child(direction);

                                   // get the FE corresponding to this
                                   // cell
  const FiniteElement<1> &fe = outermost_cell->get_fe();
  Assert (fe.n_components() == boundary_function.n_components,
	  ExcComponentMismatch());

				   // set the component mask to either
				   // the original value or a vector
				   // of @p{true}s
  const std::vector<bool> component_mask ((component_mask_.size() == 0) ?
					  std::vector<bool> (fe.n_components(), true) :
					  component_mask_);
  Assert (std::count(component_mask.begin(), component_mask.end(), true) > 0,
	  ExcComponentMismatch());

				   // now set the value of the
				   // outermost degree of
				   // freedom. setting also
				   // creates the entry in the map
				   // if it did not exist
				   // beforehand
				   //
				   // save some time by requesting
				   // values only once for each point,
				   // irrespective of the number of
				   // components of the function
  Vector<double> function_values (fe.n_components());
  if (fe.n_components() == 1)
    function_values(0)
      = boundary_function.value (outermost_cell->vertex(direction));
  else
    boundary_function.vector_value (outermost_cell->vertex(direction),
				    function_values);
  
  for (unsigned int i=0; i<fe.dofs_per_vertex; ++i)
    if (component_mask[fe.face_system_to_component_index(i).first])
      boundary_values[outermost_cell->vertex_dof_index(direction,i)]
	= function_values(fe.face_system_to_component_index(i).first);
}


//TODO[?] Actually the Mapping object should be a MappingCollection object for the
// hp::DoFHandler.
void
VectorTools::interpolate_boundary_values (const Mapping<1>              &mapping,
					  const DoFHandler<1>           &dof,
					  const FunctionMap<1>::type    &function_map,
					  std::map<unsigned int,double> &boundary_values,
					  const std::vector<bool>       &component_mask)
{
  for (FunctionMap<1>::type::const_iterator i=function_map.begin();
       i!=function_map.end(); ++i)
    interpolate_boundary_values (mapping, dof, i->first, *i->second,
				 boundary_values, component_mask);
}


//TODO[?] Actually the Mapping object should be a MappingCollection object for the
// hp::DoFHandler.
void
VectorTools::interpolate_boundary_values (const Mapping<1>              &mapping,
					  const hp::DoFHandler<1>           &dof,
					  const FunctionMap<1>::type    &function_map,
					  std::map<unsigned int,double> &boundary_values,
					  const std::vector<bool>       &component_mask)
{
  for (FunctionMap<1>::type::const_iterator i=function_map.begin();
       i!=function_map.end(); ++i)
    interpolate_boundary_values (mapping, dof, i->first, *i->second,
				 boundary_values, component_mask);
}


#endif


template <int dim, template <int> class DH>
void
VectorTools::
interpolate_boundary_values (const Mapping<dim>            &mapping,
                             const DH<dim>                 &dof,
                             const typename FunctionMap<dim>::type &function_map,
                             std::map<unsigned int,double> &boundary_values,
                             const std::vector<bool>       &component_mask_)
{
				   // if for whatever reason we were
				   // passed an empty map, return
				   // immediately
  if (function_map.size() == 0)
    return;
  
  Assert (function_map.find(255) == function_map.end(),
	  ExcInvalidBoundaryIndicator());

  const unsigned int        n_components = get_n_components(dof);
  const bool                fe_is_system = (n_components != 1);

  for (typename FunctionMap<dim>::type::const_iterator i=function_map.begin();
       i!=function_map.end(); ++i)
    Assert (n_components == i->second->n_components,
	    ExcInvalidFE());

				   // set the component mask to either
				   // the original value or a vector
				   // of @p{true}s
  const std::vector<bool> component_mask ((component_mask_.size() == 0) ?
					  std::vector<bool> (n_components, true) :
					  component_mask_);
  Assert (std::count(component_mask.begin(), component_mask.end(), true) > 0,
	  ExcComponentMismatch());

				   // field to store the indices
  std::vector<unsigned int> face_dofs;
  face_dofs.reserve (max_dofs_per_face(dof));
  std::fill (face_dofs.begin (), face_dofs.end (), DoFHandler<dim>::invalid_dof_index);

  std::vector<Point<dim> >  dof_locations;
  dof_locations.reserve (max_dofs_per_face(dof));
  std::fill (dof_locations.begin(), dof_locations.end (), Point<dim>());

				   // array to store the values of
				   // the boundary function at the
				   // boundary points. have to arrays
				   // for scalar and vector functions
				   // to use the more efficient one
				   // respectively
  std::vector<double>          dof_values_scalar;
  std::vector<Vector<double> > dof_values_system;
  dof_values_scalar.reserve (max_dofs_per_face (dof));
  dof_values_system.reserve (max_dofs_per_face (dof));

  typename DH<dim>::active_cell_iterator cell = dof.begin_active(),
					 endc = dof.end();
  for (; cell!=endc; ++cell)
    for (unsigned int face_no = 0; face_no < GeometryInfo<dim>::faces_per_cell;
	 ++face_no)
      {
        const FiniteElement<dim> &fe = cell->get_fe();

					 // we can presently deal only with
					 // primitive elements for boundary
					 // values. this does not preclude
					 // us using non-primitive elements
					 // in components that we aren't
					 // interested in, however. make
					 // sure that all shape functions
					 // that are non-zero for the
					 // components we are interested in,
					 // are in fact primitive
	for (unsigned int i=0; i<cell->get_fe().dofs_per_cell; ++i)
	  {
	    const std::vector<bool> &nonzero_component_array
	      = cell->get_fe().get_nonzero_components (i);
	    for (unsigned int c=0; c<n_components; ++c)
	      if ((nonzero_component_array[c] == true)
		  &&
		  (component_mask[c] == true))
		Assert (cell->get_fe().is_primitive (i),
			ExcMessage ("This function can only deal with requested boundary "
				    "values that correspond to primitive (scalar) base "
				    "elements"));
	  }
	
	typename DH<dim>::face_iterator face = cell->face(face_no);
	const unsigned char boundary_component = face->boundary_indicator();
	if (function_map.find(boundary_component) != function_map.end()) 
					   // face is of the right component
	  {
//TODO[?] Should work for both DoFHandlers. But probably not the most efficient
// implementation.
				             // next generate a quadrature rule
				             // on the face from the unit
				             // support points. this wil be used
				             // to obtain the quadrature points
				             // on the real cell's face
            std::vector<Point<dim-1> > 
	      unit_support_points = fe.get_unit_face_support_points();
  
				             // check whether there are support
				             // points on the face. if not, then
				             // we should try a more clever
				             // way. the idea is that a finite
				             // element may not offer support
				             // points for all its shape
				             // functions, but maybe only
				             // some. if it offers support
				             // points for the components we are
				             // interested in in this function,
				             // then that's fine. if not, the
				             // function we call in the finite
				             // element will raise an
				             // exception. the support points
				             // for the other shape functions
				             // are left uninitialized (well,
				             // initialized by the default
				             // constructor), since we don't
				             // need them anyway.
					     //
					     // As a detour, we must
					     // make sure we only
					     // query
					     // face_system_to_component_index
					     // if the index
					     // corresponds to a
					     // primitive shape
					     // function. since we
					     // know that all the
					     // components we are
					     // interested in are
					     // primitive (by the
					     // above check), we can
					     // safely put such a
					     // check in front
	    if (unit_support_points.size() == 0)
	      {
	        unit_support_points.resize (fe.dofs_per_face);
                for (unsigned int i=0; i<fe.dofs_per_face; ++i)
		  if (fe.is_primitive (fe.face_to_equivalent_cell_index(i)))
		    if (component_mask[fe.face_system_to_component_index(i).first]
			== true)
		      unit_support_points[i] = fe.unit_face_support_point(i);
              }

            Quadrature<dim-1> aux_quad (unit_support_points);
            FEFaceValues<dim> fe_values (mapping, fe, aux_quad, update_q_points);
//TODO[?] End of inefficient code

					     // get indices, physical location and
					     // boundary values of dofs on this
					     // face
            face_dofs.resize (fe.dofs_per_face);
	    face->get_dof_indices (face_dofs, cell->active_fe_index());
	    fe_values.reinit(cell, face_no);
	    const std::vector<Point<dim> > &dof_locations
              = fe_values.get_quadrature_points ();
	    
	    if (fe_is_system)
	      {
                                                 // resize
                                                 // array. avoid
                                                 // construction of a
                                                 // memory allocating
                                                 // temporary if
                                                 // possible
                if (dof_values_system.size() < fe.dofs_per_face)
                  dof_values_system.resize (fe.dofs_per_face,
                                            Vector<double>(fe.n_components()));
                else
                  dof_values_system.resize (fe.dofs_per_face);
                
		function_map.find(boundary_component)->second
                  ->vector_value_list (dof_locations, dof_values_system);
		
						 // enter those dofs
						 // into the list that
						 // match the
						 // component
						 // signature. avoid
						 // the usual
						 // complication that
						 // we can't just use
						 // *_system_to_component_index
						 // for non-primitive
						 // FEs
		for (unsigned int i=0; i<face_dofs.size(); ++i)
                  {
                    unsigned int component;
                    if (fe.is_primitive())
                      component = fe.face_system_to_component_index(i).first;
                    else
                      {
                                                         // non-primitive
                                                         // case. make
                                                         // sure that
                                                         // this
                                                         // particular
                                                         // shape
                                                         // function
                                                         // _is_
                                                         // primitive,
                                                         // and get at
                                                         // it's
                                                         // component. use
                                                         // usual
                                                         // trick to
                                                         // transfer
                                                         // face dof
                                                         // index to
                                                         // cell dof
                                                         // index
                        const unsigned int cell_i
                          = (dim == 1 ?
                             i
                             :
                             (dim == 2 ?
                              (i<2*fe.dofs_per_vertex ? i : i+2*fe.dofs_per_vertex)
                              :
                              (dim == 3 ?
                               (i<4*fe.dofs_per_vertex ?
                                i
                                :
                                (i<4*fe.dofs_per_vertex+4*fe.dofs_per_line ?
                                 i+4*fe.dofs_per_vertex
                                 :
                                 i+4*fe.dofs_per_vertex+8*fe.dofs_per_line))
                               :
                               deal_II_numbers::invalid_unsigned_int)));
                        Assert (cell_i < fe.dofs_per_cell, ExcInternalError());

                                                         // make sure
                                                         // that if
                                                         // this is
                                                         // not a
                                                         // primitive
                                                         // shape function,
                                                         // then all
                                                         // the
                                                         // corresponding
                                                         // components
                                                         // in the
                                                         // mask are
                                                         // not set
                        if (!fe.is_primitive(cell_i))
                          for (unsigned int c=0; c<n_components; ++c)
                            if (fe.get_nonzero_components(cell_i)[c])
                              Assert (component_mask[c] == false,
                                      ExcFENotPrimitive());

                                                         // let's pick
                                                         // the first
                                                         // of
                                                         // possibly
                                                         // more than
                                                         // one
                                                         // non-zero
                                                         // components. if
                                                         // shape
                                                         // function
                                                         // is
                                                         // non-primitive,
                                                         // then we
                                                         // will
                                                         // ignore the
                                                         // result in
                                                         // the
                                                         // following
                                                         // anyway,
                                                         // otherwise
                                                         // there's
                                                         // only one
                                                         // non-zero
                                                         // component
                                                         // which we
                                                         // will use
                        component = (std::find (fe.get_nonzero_components(cell_i).begin(),
                                                fe.get_nonzero_components(cell_i).end(),
                                                true)
                                     -
                                     fe.get_nonzero_components(cell_i).begin());
                      }
                    
                    if (component_mask[component] == true)
                      boundary_values[face_dofs[i]] = dof_values_system[i](component);
                  } 
	      }
	    else
					       // fe has only one component,
					       // so save some computations
	      {
						 // get only the one component that
						 // this function has
                dof_values_scalar.resize (fe.dofs_per_face);
		function_map.find(boundary_component)->second
                  ->value_list (dof_locations, dof_values_scalar, 0);
		
						 // enter into list
		
		for (unsigned int i=0; i<face_dofs.size(); ++i)
		  boundary_values[face_dofs[i]] = dof_values_scalar[i];
	      }
	  }
      }
}



template <int dim, template <int> class DH>
void
VectorTools::interpolate_boundary_values (const Mapping<dim>            &mapping,
					  const DH<dim>                 &dof,
					  const unsigned char            boundary_component,
					  const Function<dim>           &boundary_function,
					  std::map<unsigned int,double> &boundary_values,
					  const std::vector<bool>       &component_mask)
{
  typename FunctionMap<dim>::type function_map;
  function_map[boundary_component] = &boundary_function;
  interpolate_boundary_values (mapping, dof, function_map, boundary_values,
			       component_mask);
}


  
//TODO[?] Change for real hp::DoFHandler
// This function might not work anymore if the real hp::DoFHandler is available.
template <int dim, template <int> class DH>
void
VectorTools::interpolate_boundary_values (const DH<dim>                 &dof,
					  const unsigned char            boundary_component,
					  const Function<dim>           &boundary_function,
					  std::map<unsigned int,double> &boundary_values,
					  const std::vector<bool>       &component_mask)
{
  Assert (DEAL_II_COMPAT_MAPPING, ExcCompatibility("mapping"));
  static const MappingQ1<dim> mapping;
  interpolate_boundary_values(mapping, dof, boundary_component,
			      boundary_function, boundary_values, component_mask);
}



//TODO[?] Change for real hp::DoFHandler
// This function might not work anymore if the real hp::DoFHandler is available.
template <int dim, template <int> class DH>
void
VectorTools::interpolate_boundary_values (const DH<dim>                 &dof,
					  const typename FunctionMap<dim>::type &function_map,
					  std::map<unsigned int,double> &boundary_values,
					  const std::vector<bool>       &component_mask)
{
  Assert (DEAL_II_COMPAT_MAPPING, ExcCompatibility("mapping"));
  static const MappingQ1<dim> mapping;
  interpolate_boundary_values(mapping, dof, function_map,
			      boundary_values, component_mask);
}


#if deal_II_dimension == 1

void
VectorTools::project_boundary_values (const Mapping<1>       &mapping,
				      const DoFHandler<1>    &dof,
				      const FunctionMap<1>::type &boundary_functions,
				      const Quadrature<0>  &,
				      std::map<unsigned int,double> &boundary_values)
{
				   // projection in 1d is equivalent
				   // to interpolation
  interpolate_boundary_values (mapping, dof, boundary_functions,
			       boundary_values, std::vector<bool>());
}

#endif


template <int dim>
void
VectorTools::project_boundary_values (const Mapping<dim>       &mapping,
				      const DoFHandler<dim>    &dof,
				      const typename FunctionMap<dim>::type &boundary_functions,
				      const Quadrature<dim-1>  &q,
				      std::map<unsigned int,double> &boundary_values)
{
//TODO:[?] In VectorTools::project_boundary_values, no condensation of sparsity
//    structures, matrices and right hand sides or distribution of
//    solution vectors is performed. This is ok for dim<3 because then
//    there are no constrained nodes on the boundary, but is not
//    acceptable for higher dimensions. Fix this.

  Assert (dof.get_fe().n_components() == boundary_functions.begin()->second->n_components,
	  ExcComponentMismatch());
  
  std::vector<unsigned int> dof_to_boundary_mapping;
  std::set<unsigned char> selected_boundary_components;
  for (typename FunctionMap<dim>::type::const_iterator i=boundary_functions.begin();
       i!=boundary_functions.end(); ++i)
    selected_boundary_components.insert (i->first);
  
  DoFTools::map_dof_to_boundary_indices (dof, selected_boundary_components,
					 dof_to_boundary_mapping);
  
				   // set up sparsity structure
  SparsityPattern sparsity(dof.n_boundary_dofs (boundary_functions),
			   dof.max_couplings_between_boundary_dofs());
  DoFTools::make_boundary_sparsity_pattern (dof,
					    boundary_functions,
					    dof_to_boundary_mapping,
					    sparsity);

				   // note: for three or more dimensions, there
				   // may be constrained nodes on the boundary
				   // in this case the boundary mass matrix has
				   // to be condensed and the solution is to
				   // be distributed afterwards, which is not
				   // yet implemented. The reason for this is
				   // that we cannot simply use the @p{condense}
				   // family of functions, since the matrices
				   // and vectors do not use the global
				   // numbering but rather the boundary
				   // numbering, i.e. the condense function
				   // needs to use another indirection. There
				   // should be not many technical problems,
				   // but it needs to be implemented
  if (dim<3)
    sparsity.compress();
  else
    Assert (false, ExcNotImplemented());


				   // make mass matrix and right hand side
  SparseMatrix<double> mass_matrix(sparsity);
  Vector<double>       rhs(sparsity.n_rows());


  MatrixCreator::create_boundary_mass_matrix (mapping, dof, q, 
					      mass_matrix, boundary_functions,
					      rhs, dof_to_boundary_mapping);

				   // same thing as above: if dim>=3 we need
				   // to consider constraints
  Assert (dim<3, ExcNotImplemented());


  Vector<double> boundary_projection (rhs.size());

  SolverControl           control(1000, 1e-16);
  PrimitiveVectorMemory<> memory;
  SolverCG<>              cg(control,memory);

  PreconditionSSOR<> prec;
  prec.initialize(mass_matrix, 1.2);
				   // solve
  cg.solve (mass_matrix, boundary_projection, rhs, prec);

				   // fill in boundary values
  for (unsigned int i=0; i<dof_to_boundary_mapping.size(); ++i)
    if (dof_to_boundary_mapping[i] != DoFHandler<dim>::invalid_dof_index)
				       // this dof is on one of the
				       // interesting boundary parts
				       //
				       // remember: @p{i} is the global dof
				       // number, @p{dof_to_boundary_mapping[i]}
				       // is the number on the boundary and
				       // thus in the solution vector
      boundary_values[i] = boundary_projection(dof_to_boundary_mapping[i]);
}


template <int dim>
void
VectorTools::project_boundary_values (const DoFHandler<dim>    &dof,
				      const typename FunctionMap<dim>::type &boundary_functions,
				      const Quadrature<dim-1>  &q,
				      std::map<unsigned int,double> &boundary_values)
{
  Assert (DEAL_II_COMPAT_MAPPING, ExcCompatibility("mapping"));
  static const MappingQ1<dim> mapping;
  project_boundary_values(mapping, dof, boundary_functions, q, boundary_values);
}



namespace internal
{
  namespace VectorTools
  {
    template <int dim, class InVector, class OutVector, class DH>
    static
    void
    do_integrate_difference (const ::hp::MappingCollection<dim>    &mapping,
                             const DH              &dof,
                             const InVector        &fe_function,
                             const Function<dim>   &exact_solution,
                             OutVector             &difference,
                             const ::hp::QCollection<dim> &q,
                             const ::VectorTools::NormType &norm,
                             const Function<dim>   *weight,
                             const double           exponent_1)
    {
                                       // we mark the "exponent" parameter
                                       // to this function "const" since
                                       // it is strictly incoming, but we
                                       // need to set it to something
                                       // different later on, if
                                       // necessary, so have a read-write
                                       // version of it:
      double exponent = exponent_1;
  
      const unsigned int        n_components = dof.get_fe().n_components();
      const bool                fe_is_system = (n_components != 1);

      if (weight!=0)
        {
          Assert ((weight->n_components==1) || (weight->n_components==n_components),
                  ExcDimensionMismatch(weight->n_components, n_components));
        }

      difference.reinit (dof.get_tria().n_active_cells());
  
      switch (norm)
        {
          case ::VectorTools::L2_norm:
          case ::VectorTools::H1_seminorm:
          case ::VectorTools::H1_norm:
                exponent = 2.;
                break;
          case ::VectorTools::L1_norm:
                exponent = 1.;
                break;
          default:
                break;
        }
  
      UpdateFlags update_flags = UpdateFlags (update_q_points  |
                                              update_JxW_values);
      switch (norm)
        {
          case ::VectorTools::H1_seminorm:
          case ::VectorTools::W1p_seminorm:
          case ::VectorTools::W1infty_seminorm:
                update_flags |= UpdateFlags (update_gradients);
                break;
          case ::VectorTools::H1_norm:
          case ::VectorTools::W1p_norm:
          case ::VectorTools::W1infty_norm:
                update_flags |= UpdateFlags (update_gradients);
                                                 // no break!
          default:
                update_flags |= UpdateFlags (update_values);
                break;
        }  

      ::hp::FECollection<dim> fe_collection (dof.get_fe());
      ::hp::FEValues<dim> x_fe_values(mapping, fe_collection, q, update_flags);

      const unsigned int max_n_q_points = q.max_n_quadrature_points ();
      
      std::vector< Vector<double> >
        function_values (max_n_q_points, Vector<double>(n_components));
      std::vector<std::vector<Tensor<1,dim> > >
        function_grads (max_n_q_points, std::vector<Tensor<1,dim> >(n_components));

      std::vector<double>
        weight_values (max_n_q_points);
      std::vector<Vector<double> >
        weight_vectors (max_n_q_points, Vector<double>(n_components));

      std::vector<Vector<double> >
        psi_values (max_n_q_points, Vector<double>(n_components));
      std::vector<std::vector<Tensor<1,dim> > >
        psi_grads (max_n_q_points, std::vector<Tensor<1,dim> >(n_components));
      std::vector<double>
        psi_scalar (max_n_q_points);

                                       // tmp vector when we use the
                                       // Function<dim> functions for
                                       // scalar functions
      std::vector<double>         tmp_values (max_n_q_points);
      std::vector<Tensor<1,dim> > tmp_gradients (max_n_q_points);
  
                                       // loop over all cells
      typename DH::active_cell_iterator cell = dof.begin_active(),
                                        endc = dof.end();
      for (unsigned int index=0; cell != endc; ++cell, ++index)
        {
          double diff=0;
                                           // initialize for this cell
          x_fe_values.reinit (cell);

          const FEValues<dim> &fe_values  = x_fe_values.get_present_fe_values ();
          const unsigned int   n_q_points = fe_values.n_quadrature_points;
          
                                           // resize all out scratch
                                           // arrays to the number of
                                           // quadrature points we use
                                           // for the present cell
          function_values.resize (n_q_points,
                                  Vector<double>(n_components));
          function_grads.resize (n_q_points,
                                 std::vector<Tensor<1,dim> >(n_components));

          weight_values.resize (n_q_points);
          weight_vectors.resize (n_q_points,
                                 Vector<double>(n_components));

          psi_values.resize (n_q_points,
                             Vector<double>(n_components));
          psi_grads.resize (n_q_points,
                            std::vector<Tensor<1,dim> >(n_components));
          psi_scalar.resize (n_q_points);

          tmp_values.resize (n_q_points);
          tmp_gradients.resize (n_q_points);

          if (weight!=0)
            {
              if (weight->n_components>1)
                weight->vector_value_list (fe_values.get_quadrature_points(),
                                           weight_vectors);
              else
                {
                  weight->value_list (fe_values.get_quadrature_points(),
                                      weight_values);
                  for (unsigned int k=0;k<n_q_points;++k)
                    weight_vectors[k] = weight_values[k];
                }
            }
          else
            {
              for (unsigned int k=0;k<n_q_points;++k)
                weight_vectors[k] = 1.;
            }
      
      
          if (update_flags & update_values)
            {
                                               // first compute the exact solution
                                               // (vectors) at the quadrature points
                                               // try to do this as efficient as
                                               // possible by avoiding a second
                                               // virtual function call in case
                                               // the function really has only
                                               // one component
              if (fe_is_system)
                exact_solution.vector_value_list (fe_values.get_quadrature_points(),
                                                  psi_values);
              else
                {
                  exact_solution.value_list (fe_values.get_quadrature_points(),
                                             tmp_values);
                  for (unsigned int i=0; i<n_q_points; ++i)
                    psi_values[i](0) = tmp_values[i];
                }
	  
                                               // then subtract finite element
                                               // fe_function
              fe_values.get_function_values (fe_function, function_values);
              for (unsigned int q=0; q<n_q_points; ++q)
                psi_values[q] -= function_values[q];
            }

                                           // Do the same for gradients, if required
          if (update_flags & update_gradients)
            {
                                               // try to be a little clever
                                               // to avoid recursive virtual
                                               // function calls when calling
                                               // @p{gradient_list} for functions
                                               // that are really scalar
                                               // functions
              if (fe_is_system)
                exact_solution.vector_gradient_list (fe_values.get_quadrature_points(),
                                                     psi_grads);
              else
                {
                  exact_solution.gradient_list (fe_values.get_quadrature_points(),
                                                tmp_gradients);
                  for (unsigned int i=0; i<n_q_points; ++i)
                    psi_grads[i][0] = tmp_gradients[i];
                }      
	  
                                               // then subtract finite element
                                               // function_grads
              fe_values.get_function_grads (fe_function, function_grads);
              for (unsigned int k=0; k<n_components; ++k)
                for (unsigned int q=0; q<n_q_points; ++q)
                  psi_grads[q][k] -= function_grads[q][k];
            }
      
          switch (norm)
            {
              case ::VectorTools::mean:
                    std::fill_n (psi_scalar.begin(), n_q_points, 0.0);
                                                     // Compute values in
                                                     // quadrature points
                    for (unsigned int k=0; k<n_components; ++k)
                      for (unsigned int q=0; q<n_q_points; ++q)
                        psi_scalar[q] += psi_values[q](k)
                                         * weight_vectors[q](k);

                                                     // Integrate
                    diff = std::inner_product (psi_scalar.begin(), psi_scalar.end(),
                                               fe_values.get_JxW_values().begin(),
                                               0.0);
                    break;
              case ::VectorTools::Lp_norm:
              case ::VectorTools::L1_norm:
              case ::VectorTools::W1p_norm:
                    std::fill_n (psi_scalar.begin(), n_q_points, 0.0);
                                                     // Compute values in
                                                     // quadrature points
                    for (unsigned int k=0; k<n_components; ++k)
                      for (unsigned int q=0; q<n_q_points; ++q)
                        psi_scalar[q] += std::pow(psi_values[q](k)*psi_values[q](k),
                                                  exponent/2.)
                                         * weight_vectors[q](k);
	    
                                                     // Integrate
                    diff = std::inner_product (psi_scalar.begin(), psi_scalar.end(),
                                               fe_values.get_JxW_values().begin(),
                                               0.0);
                                                     // Compute the root only,
                                                     // if no derivative
                                                     // values are added later
                    if (!(update_flags & update_gradients))
                      diff = std::pow(diff, 1./exponent);
                    break;
              case ::VectorTools::L2_norm:
              case ::VectorTools::H1_norm:
                    std::fill_n (psi_scalar.begin(), n_q_points, 0.0);
                                                     // Compute values in
                                                     // quadrature points
                    for (unsigned int k=0; k<n_components; ++k)
                      for (unsigned int q=0; q<n_q_points; ++q)
                        psi_scalar[q] += psi_values[q](k)*psi_values[q](k)
                                         * weight_vectors[q](k);

                                                     // Integrate
                    diff = std::inner_product (psi_scalar.begin(), psi_scalar.end(),
                                               fe_values.get_JxW_values().begin(),
                                               0.0);
                                                     // Compute the root only,
                                                     // if no derivative
                                                     // values are added later
                    if (norm == ::VectorTools::L2_norm)
                      diff=std::sqrt(diff);
                    break;
              case ::VectorTools::Linfty_norm:
              case ::VectorTools::W1infty_norm:
                    std::fill_n (psi_scalar.begin(), n_q_points, 0.0);
                    for (unsigned int k=0; k<n_components; ++k)
                      for (unsigned int q=0; q<n_q_points; ++q)
                        {
                          double newval = std::fabs(psi_values[q](k))
                                          * weight_vectors[q](k);
                          if (psi_scalar[q]<newval)
                            psi_scalar[q] = newval;
                        }
                                                     // Maximum on one cell
                    diff = *std::max_element (psi_scalar.begin(), psi_scalar.end());
                    break;
              case ::VectorTools::H1_seminorm:
              case ::VectorTools::W1p_seminorm:
              case ::VectorTools::W1infty_seminorm:
                    break;
              default:
                    Assert (false, ExcNotImplemented());
                    break;
            }

          switch (norm)
            {
              case ::VectorTools::W1p_seminorm:
              case ::VectorTools::W1p_norm:
                    std::fill_n (psi_scalar.begin(), n_q_points, 0.0);
                    for (unsigned int k=0; k<n_components; ++k)
                      for (unsigned int q=0; q<n_q_points; ++q)
                        psi_scalar[q] += std::pow(sqr_point(psi_grads[q][k]),
                                                  exponent/2.)
                                         * weight_vectors[q](k);
	    
                    diff += std::inner_product (psi_scalar.begin(), psi_scalar.end(),
                                                fe_values.get_JxW_values().begin(),
                                                0.0);
                    diff = std::pow(diff, 1./exponent);
                    break;
              case ::VectorTools::H1_seminorm:
              case ::VectorTools::H1_norm:
                                                     // take square of integrand
                    std::fill_n (psi_scalar.begin(), n_q_points, 0.0);
                    for (unsigned int k=0; k<n_components; ++k)
                      for (unsigned int q=0; q<n_q_points; ++q)
                        psi_scalar[q] += sqr_point(psi_grads[q][k])
                                         * weight_vectors[q](k);

                                                     // add seminorm to L_2 norm or
                                                     // to zero
                    diff += std::inner_product (psi_scalar.begin(), psi_scalar.end(),
                                                fe_values.get_JxW_values().begin(),
                                                0.0);
                    diff = std::sqrt(diff);
                    break;
              case ::VectorTools::W1infty_seminorm:
              case ::VectorTools::W1infty_norm:
                    Assert(false, ExcNotImplemented());
                    std::fill_n (psi_scalar.begin(), n_q_points, 0.0);
                    for (unsigned int k=0; k<n_components; ++k)
                      for (unsigned int q=0; q<n_q_points; ++q)
                        {
                          double t = 0.;
                          for (unsigned int d=0;d<dim;++d)
                            t = std::max(t,std::fabs(psi_grads[q][k][d])
                                         * weight_vectors[q](k));
		  
                          psi_scalar[q] = std::max(psi_scalar[q],t);
                        }

                    for (unsigned int i=0;i<psi_scalar.size();++i)
                      diff = std::max (diff, psi_scalar[i]);
                    break;
              default:
                    break;
            }
                                           // append result of this cell
                                           // to the end of the vector
          Assert (deal_II_numbers::is_finite(diff), ExcInternalError());
          difference(index) = diff;
        }
    }

  } //namespace VectorTools
} // namespace internal




template <int dim, class InVector, class OutVector>
void
VectorTools::integrate_difference (const Mapping<dim>    &mapping,
				   const DoFHandler<dim> &dof,
				   const InVector        &fe_function,
				   const Function<dim>   &exact_solution,
				   OutVector             &difference,
				   const Quadrature<dim> &q,
				   const NormType        &norm,
				   const Function<dim>   *weight,
				   const double           exponent)
{
  internal::VectorTools
    ::do_integrate_difference (hp::MappingCollection<dim>(mapping),
                               dof, fe_function, exact_solution,
                               difference, hp::QCollection<dim>(q),
                               norm, weight, exponent);
}


template <int dim, class InVector, class OutVector>
void
VectorTools::integrate_difference (const DoFHandler<dim>    &dof,
				   const InVector           &fe_function,
				   const Function<dim>      &exact_solution,
				   OutVector                &difference,
				   const Quadrature<dim>    &q,
				   const NormType           &norm,
				   const Function<dim>      *weight,
				   const double              exponent)
{
  Assert (DEAL_II_COMPAT_MAPPING, ExcCompatibility("mapping"));
  internal::VectorTools
    ::do_integrate_difference(hp::StaticMappingQ1<dim>::mapping_collection,
                              dof, fe_function, exact_solution,
                              difference, hp::QCollection<dim>(q),
                              norm, weight, exponent);
}



template <int dim, class InVector, class OutVector>
void
VectorTools::integrate_difference (const ::hp::MappingCollection<dim>    &mapping,
				   const ::hp::DoFHandler<dim> &dof,
				   const InVector        &fe_function,
				   const Function<dim>   &exact_solution,
				   OutVector             &difference,
				   const ::hp::QCollection<dim> &q,
				   const NormType        &norm,
				   const Function<dim>   *weight,
				   const double           exponent)
{
  internal::VectorTools
    ::do_integrate_difference (hp::MappingCollection<dim>(mapping),
                               dof, fe_function, exact_solution,
                               difference, q,
                               norm, weight, exponent);
}


template <int dim, class InVector, class OutVector>
void
VectorTools::integrate_difference (const ::hp::DoFHandler<dim>    &dof,
				   const InVector           &fe_function,
				   const Function<dim>      &exact_solution,
				   OutVector                &difference,
				   const ::hp::QCollection<dim>    &q,
				   const NormType           &norm,
				   const Function<dim>      *weight,
				   const double              exponent)
{
  Assert (DEAL_II_COMPAT_MAPPING, ExcCompatibility("mapping"));
  internal::VectorTools
    ::do_integrate_difference(hp::StaticMappingQ1<dim>::mapping_collection,
                              dof, fe_function, exact_solution,
                              difference, q,
                              norm, weight, exponent);
}



template <int dim, class InVector>
void
VectorTools::point_difference (const DoFHandler<dim> &dof,
			       const InVector        &fe_function,
			       const Function<dim>   &exact_function,
			       Vector<double>        &difference,
			       const Point<dim>      &point)
{
  static const MappingQ1<dim> mapping;
  const FiniteElement<dim>& fe = dof.get_fe();

  Assert(difference.size() == fe.n_components(),
	 ExcDimensionMismatch(difference.size(), fe.n_components()));

                                   // first find the cell in which this point
                                   // is, initialize a quadrature rule with
                                   // it, and then a FEValues object
  const typename DoFHandler<dim>::active_cell_iterator
    cell = GridTools::find_active_cell_around_point (dof, point);

  const Point<dim> unit_point
    = mapping.transform_real_to_unit_cell(cell, point);
  Assert (GeometryInfo<dim>::is_inside_unit_cell (unit_point),
          ExcInternalError());

  const Quadrature<dim> quadrature (unit_point);
  FEValues<dim> fe_values(mapping, fe, quadrature, update_values);
  fe_values.reinit(cell);

                                   // then use this to get at the values of
                                   // the given fe_function at this point
  std::vector<Vector<double> > u_value(1, Vector<double> (fe.n_components()));
  fe_values.get_function_values(fe_function, u_value);

  if (fe.n_components() == 1)
    difference(0) = exact_function.value(point);
  else
    exact_function.vector_value(point, difference);
    
  for (unsigned int i=0; i<difference.size(); ++i)
    difference(i) -= u_value[0](i);
}


template <int dim, class InVector>
void
VectorTools::point_value (const DoFHandler<dim> &dof,
			  const InVector        &fe_function,
			  const Point<dim>      &point,
			  Vector<double>        &value)
{
  static const MappingQ1<dim> mapping;
  const FiniteElement<dim>& fe = dof.get_fe();

  Assert(value.size() == fe.n_components(),
	 ExcDimensionMismatch(value.size(), fe.n_components()));

                                   // first find the cell in which this point
                                   // is, initialize a quadrature rule with
                                   // it, and then a FEValues object
  const typename DoFHandler<dim>::active_cell_iterator
    cell = GridTools::find_active_cell_around_point (dof, point);

  const Point<dim> unit_point
    = mapping.transform_real_to_unit_cell(cell, point);
  Assert (GeometryInfo<dim>::is_inside_unit_cell (unit_point),
          ExcInternalError());

  const Quadrature<dim> quadrature (unit_point);
  FEValues<dim> fe_values(mapping, fe, quadrature, update_values);
  fe_values.reinit(cell);

                                   // then use this to get at the values of
                                   // the given fe_function at this point
  std::vector<Vector<double> > u_value(1, Vector<double> (fe.n_components()));
  fe_values.get_function_values(fe_function, u_value);

  value = u_value[0];
}



template <int dim, class InVector>
double
VectorTools::point_value (const DoFHandler<dim> &dof,
			  const InVector        &fe_function,
			  const Point<dim>      &point)
{
  static const MappingQ1<dim> mapping;
  const FiniteElement<dim>& fe = dof.get_fe();

  Assert(fe.n_components() == 1,
	 ExcMessage ("Finite element is not scalar as is necessary for this function"));

                                   // first find the cell in which this point
                                   // is, initialize a quadrature rule with
                                   // it, and then a FEValues object
  const typename DoFHandler<dim>::active_cell_iterator
    cell = GridTools::find_active_cell_around_point (dof, point);

  const Point<dim> unit_point
    = mapping.transform_real_to_unit_cell(cell, point);
  Assert (GeometryInfo<dim>::is_inside_unit_cell (unit_point),
          ExcInternalError());

  const Quadrature<dim> quadrature (unit_point);
  FEValues<dim> fe_values(mapping, fe, quadrature, update_values);
  fe_values.reinit(cell);

                                   // then use this to get at the values of
                                   // the given fe_function at this point
  std::vector<double> u_value(1);
  fe_values.get_function_values(fe_function, u_value);

  return u_value[0];
}



template <int dim, class InVector>
double
VectorTools::compute_mean_value (const Mapping<dim>    &mapping,
				 const DoFHandler<dim> &dof,
				 const Quadrature<dim> &quadrature,
				 const InVector        &v,
				 const unsigned int     component)
{
  Assert (component < dof.get_fe().n_components(),
	  ExcIndexRange(component, 0, dof.get_fe().n_components()));
  
  FEValues<dim> fe(mapping, dof.get_fe(), quadrature,
		   UpdateFlags(update_JxW_values
			       | update_values));

  typename DoFHandler<dim>::active_cell_iterator c;
  std::vector<Vector<double> > values(quadrature.n_quadrature_points,
				      Vector<double> (dof.get_fe().n_components()));
  
  double mean = 0.;
  double area = 0.;
				   // Compute mean value
  for (c = dof.begin_active(); c != dof.end(); ++c)
    {
      fe.reinit (c);
      fe.get_function_values(v, values);
      for (unsigned int k=0; k< quadrature.n_quadrature_points; ++k)
	{
	  mean += fe.JxW(k) * values[k](component);
	  area += fe.JxW(k);
	};
    };
  
  return (mean/area);
}


template <int dim, class InVector>
double
VectorTools::compute_mean_value (const DoFHandler<dim> &dof,
				 const Quadrature<dim> &quadrature,
				 const InVector        &v,
				 const unsigned int     component)
{
  Assert (DEAL_II_COMPAT_MAPPING, ExcCompatibility("mapping"));
  static const MappingQ1<dim> mapping;
  return compute_mean_value(mapping, dof, quadrature, v, component);
}

#endif
