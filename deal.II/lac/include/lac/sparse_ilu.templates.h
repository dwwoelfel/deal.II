//---------------------------------------------------------------------------
//    Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008 by the deal.II authors
//    by the deal.II authors and Stephen "Cheffo" Kolaroff
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------
#ifndef __deal2__sparse_ilu_templates_h
#define __deal2__sparse_ilu_templates_h



#include <base/config.h>
#include <lac/vector.h>
#include <lac/sparse_ilu.h>

#include <algorithm>
#include <cmath>

DEAL_II_NAMESPACE_OPEN

template <typename number>
SparseILU<number>::SparseILU ()
{}



template <typename number>
SparseILU<number>::SparseILU (const SparsityPattern &sparsity) :
		SparseLUDecomposition<number> (sparsity)
{}




template <typename number>
template <typename somenumber>
void SparseILU<number>::initialize (const SparseMatrix<somenumber> &matrix,
				    const AdditionalData data)
{
  SparseLUDecomposition<number>::initialize(matrix, data);
      
  decompose(matrix, data.strengthen_diagonal);
}



template <typename number>
template <typename somenumber>
void SparseILU<number>::decompose (const SparseMatrix<somenumber> &matrix,
				   const double strengthen_diagonal)
{
  SparseLUDecomposition<number>::decompose (matrix, strengthen_diagonal);
  Assert (matrix.m()==matrix.n(), ExcNotQuadratic ());
  Assert (this->m()==this->n(),   ExcNotQuadratic ());
  Assert (matrix.m()==this->m(),  ExcDimensionMismatch(matrix.m(), this->m()));
  
  Assert (strengthen_diagonal>=0,
	  ExcInvalidStrengthening (strengthen_diagonal));

  this->copy_from (matrix);

  if (strengthen_diagonal>0)
    this->strengthen_diagonal_impl();

  const SparsityPattern             &sparsity = this->get_sparsity_pattern();
  const std::size_t  * const rowstart_indices = sparsity.get_rowstart_indices();
  const unsigned int * const column_numbers   = sparsity.get_column_numbers();
  
/*
  PSEUDO-ALGORITHM
  (indices=0..N-1)
  
  for i=1..N-1
  a[i-1,i-1] = a[i-1,i-1]^{-1}

  for k=0..i-1
  a[i,k] = a[i,k] * a[k,k]

  for j=k+1..N-1
  if (a[i,j] exists & a[k,j] exists)
  a[i,j] -= a[i,k] * a[k,j]
*/


				   // i := row
  for (unsigned int row=1; row<this->m(); ++row)
    {
				       // invert diagonal element of the
				       // previous row. this is a hack,
				       // which is possible since this
				       // element is not needed any more
				       // in the process of decomposition
				       // and since it makes the backward
				       // step when applying the decomposition
				       // significantly faster
      AssertThrow((this->global_entry(rowstart_indices[row-1]) !=0),
		  ExcDivideByZero());
      
      this->global_entry (rowstart_indices[row-1])
	= 1./this->global_entry (rowstart_indices[row-1]);

				       // let k run over all lower-left
				       // elements of row i; skip
				       // diagonal element at start
      const unsigned int * first_of_row
	= &column_numbers[rowstart_indices[row]+1];
      const unsigned int * first_after_diagonal = this->prebuilt_lower_bound[row];

				       // k := *col_ptr
      for (const unsigned int * col_ptr = first_of_row;
           col_ptr!=first_after_diagonal; ++col_ptr)
	{
	  const unsigned int global_index_ik = col_ptr-column_numbers;
	  this->global_entry(global_index_ik) *= this->diag_element(*col_ptr);

					   // now do the inner loop over
					   // j. note that we need to do
					   // it in the right order, i.e.
					   // taking into account that the
					   // columns are sorted within each
					   // row correctly, but excluding
					   // the main diagonal entry
                                           //
                                           // the explicit use of operator()
                                           // works around a bug in some gcc
                                           // versions (see PR 18803)
	  const int global_index_ki = sparsity.operator()(*col_ptr,row);

	  if (global_index_ki != -1)
	    this->diag_element(row) -= this->global_entry(global_index_ik) *
				       this->global_entry(global_index_ki);

	  for (const unsigned int * j = col_ptr+1;
	       j<&column_numbers[rowstart_indices[row+1]];
	       ++j)
	    {
//TODO:[WB] make code faster by using the following comment	      
					       // note: this inner loop could
					       // be made considerably faster
					       // if we consulted the row
					       // with number *col_ptr,
					       // instead of always asking
					       // sparsity(*col_ptr,*j),
					       // since we traverse this
					       // row linearly. I just didn't
					       // have the time to figure out
					       // the details.
                                               //
                                               // the explicit use of
                                               // operator() works around a
                                               // bug in some gcc versions
                                               // (see PR 18803)
       	      const int global_index_ij = j - &column_numbers[0],
			global_index_kj = sparsity.operator()(*col_ptr,*j);
	      if ((global_index_ij != -1) &&
		  (global_index_kj != -1))
		this->global_entry(global_index_ij) -= this->global_entry(global_index_ik) *
						       this->global_entry(global_index_kj);
	    };
	};
    };

				   // Here the very last diagonal
				   // element still has to be inverted
				   // because the for-loop doesn't do
				   // it...
  this->diag_element(this->m()-1) = 1./this->diag_element(this->m()-1);
}




template <typename number>
template <typename somenumber>
void SparseILU<number>::vmult (Vector<somenumber>       &dst,
                               const Vector<somenumber> &src) const 
{
  Assert (dst.size() == src.size(), ExcDimensionMismatch(dst.size(), src.size()));
  Assert (dst.size() == this->m(), ExcDimensionMismatch(dst.size(), this->m()));
  
  const unsigned int N=dst.size();
  const std::size_t  * const rowstart_indices
    = this->get_sparsity_pattern().get_rowstart_indices();
  const unsigned int * const column_numbers
    = this->get_sparsity_pattern().get_column_numbers();
				   // solve LUx=b in two steps:
				   // first Ly = b, then
				   //       Ux = y
				   //
				   // first a forward solve. since
				   // the diagonal values of L are
				   // one, there holds
				   // y_i = b_i
				   //       - sum_{j=0}^{i-1} L_{ij}y_j
				   // we split the y_i = b_i off and
				   // perform it at the outset of the
				   // loop
  dst = src;
  for (unsigned int row=0; row<N; ++row)
    {
				       // get start of this row. skip the
				       // diagonal element
      const unsigned int * const rowstart = &column_numbers[rowstart_indices[row]+1];
				       // find the position where the part
				       // right of the diagonal starts
      const unsigned int * const first_after_diagonal = this->prebuilt_lower_bound[row];
      
      for (const unsigned int * col=rowstart; col!=first_after_diagonal; ++col)
	dst(row) -= this->global_entry (col-column_numbers) * dst(*col);
    }

				   // now the backward solve. same
				   // procedure, but we need not set
				   // dst before, since this is already
				   // done.
				   //
				   // note that we need to scale now,
				   // since the diagonal is not equal to
                                   // one now
  for (int row=N-1; row>=0; --row)
    {
				       // get end of this row
      const unsigned int * const rowend = &column_numbers[rowstart_indices[row+1]];
				       // find the position where the part
				       // right of the diagonal starts
      const unsigned int * const first_after_diagonal = this->prebuilt_lower_bound[row];
      
      for (const unsigned int * col=first_after_diagonal; col!=rowend; ++col)
	dst(row) -= this->global_entry (col-column_numbers) * dst(*col);

				       // scale by the diagonal element.
				       // note that the diagonal element
				       // was stored inverted
      dst(row) *= this->diag_element(row);
    }
}


template <typename number>
template <typename somenumber>
void SparseILU<number>::Tvmult (Vector<somenumber>       &dst,
				const Vector<somenumber> &src) const 
{
  Assert (dst.size() == src.size(), ExcDimensionMismatch(dst.size(), src.size()));
  Assert (dst.size() == this->m(), ExcDimensionMismatch(dst.size(), this->m()));
  
  const unsigned int N=dst.size();
  const std::size_t  * const rowstart_indices
    = this->get_sparsity_pattern().get_rowstart_indices();
  const unsigned int * const column_numbers
    = this->get_sparsity_pattern().get_column_numbers();
				   // solve (LU)'x=b in two steps:
				   // first U'y = b, then
				   //       L'x = y
				   //
				   // first a forward solve. Due to the
                                   // fact that the transpose of U'
                                   // is not easily accessible, a
                                   // temporary vector is required.
  Vector<somenumber> tmp (N);

  dst = src;
  for (unsigned int row=0; row<N; ++row)
    {
      dst(row) -= tmp (row);
				       // scale by the diagonal element.
				       // note that the diagonal element
				       // was stored inverted
      dst(row) *= this->diag_element(row);

				       // get end of this row
      const unsigned int * const rowend = &column_numbers[rowstart_indices[row+1]];
				       // find the position where the part
				       // right of the diagonal starts
      const unsigned int * const first_after_diagonal = this->prebuilt_lower_bound[row];
      
      for (const unsigned int * col=first_after_diagonal; col!=rowend; ++col)
	tmp(*col) += this->global_entry (col-column_numbers) * dst(row);
    }

				   // now the backward solve. same
				   // procedure, but we need not set
				   // dst before, since this is already
				   // done.
				   //
				   // note that we no scaling is required
                                   // now, since the diagonal is one
				   // now
  tmp = 0;
  for (int row=N-1; row>=0; --row)
    {
      dst(row) -= tmp (row);

				       // get start of this row. skip the
				       // diagonal element
      const unsigned int * const rowstart = &column_numbers[rowstart_indices[row]+1];
				       // find the position where the part
				       // right of the diagonal starts
      const unsigned int * const first_after_diagonal = this->prebuilt_lower_bound[row];
      
      for (const unsigned int * col=rowstart; col!=first_after_diagonal; ++col)
	tmp(*col) += this->global_entry (col-column_numbers) * dst(row);
    }
}


template <typename number>
unsigned int
SparseILU<number>::memory_consumption () const
{
  return SparseLUDecomposition<number>::memory_consumption ();
}



/*----------------------------   sparse_ilu.templates.h     ---------------------------*/

DEAL_II_NAMESPACE_CLOSE

#endif
/*----------------------------   sparse_ilu.templates.h     ---------------------------*/
