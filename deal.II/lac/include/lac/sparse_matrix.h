//----------------------------  sparse_matrix.h  ---------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  sparse_matrix.h  ---------------------------
#ifndef __deal2__sparse_matrix_h
#define __deal2__sparse_matrix_h


#include <base/config.h>
#include <base/exceptions.h>
#include <base/subscriptor.h>
#include <base/smartpointer.h>
#include <lac/sparsity_pattern.h>

template<typename number> class Vector;
template<typename number> class FullMatrix;

/*! @addtogroup Matrix1
 *@{
 */

/**
 * Sparse matrix.
 *
 *
 * @sect2{On template instantiations}
 *
 * Member functions of this class are either implemented in this file
 * or in a file of the same name with suffix ``.templates.h''. For the
 * most common combinations of the template parameters, instantiations
 * of this class are provided in a file with suffix ``.cc'' in the
 * ``source'' directory. If you need an instantiation that is not
 * listed there, you have to include this file along with the
 * corresponding ``.templates.h'' file and instantiate the respective
 * class yourself.
 *
 * @author several, 1994-2003
 */
template <typename number>
class SparseMatrix : public virtual Subscriptor
{
  public:
				     /**
				      * STL conforming iterator.
				      */
    class const_iterator
    {
      private:
                                         /**
                                          * Accessor class for iterators
                                          */
        class Accessor
        {
          public:
                                             /**
                                              * Constructor. Since we use
                                              * accessors only for read
                                              * access, a const matrix
                                              * pointer is sufficient.
                                              */
            Accessor (const SparseMatrix<number> *matrix,
                      const unsigned int          row,
                      const unsigned short        index);

                                             /**
                                              * Row number of the element
                                              * represented by this
                                              * object.
                                              */
            unsigned int row() const;

                                             /**
                                              * Index in row of the element
                                              * represented by this
                                              * object.
                                              */
            unsigned short index() const;

                                             /**
                                              * Column number of the
                                              * element represented by
                                              * this object.
                                              */
            unsigned int column() const;

                                             /**
                                              * Value of this matrix entry.
                                              */
            number value() const;
	
          protected:
                                             /**
                                              * The matrix accessed.
                                              */
            const SparseMatrix<number>* matrix;

                                             /**
                                              * Current row number.
                                              */
            unsigned int a_row;

                                             /**
                                              * Current index in row.
                                              */
            unsigned short a_index;

                                             /**
                                              * Make enclosing class a
                                              * friend.
                                              */
            friend class const_iterator;
        };
        
      public:
                                         /**
                                          * Constructor.
                                          */ 
	const_iterator(const SparseMatrix<number> *matrix,
		       const unsigned int          row,
		       const unsigned short        index);
	  
                                         /**
                                          * Prefix increment.
                                          */
	const_iterator& operator++ ();

                                         /**
                                          * Postfix increment.
                                          */
	const_iterator& operator++ (int);

                                         /**
                                          * Dereferencing operator.
                                          */
	const Accessor& operator* () const;

                                         /**
                                          * Dereferencing operator.
                                          */
	const Accessor* operator-> () const;

                                         /**
                                          * Comparison. True, if
                                          * both iterators point to
                                          * the same matrix
                                          * position.
                                          */
	bool operator == (const const_iterator&) const;
                                         /**
                                          * Inverse of @p{==}.
                                          */
	bool operator != (const const_iterator&) const;

                                         /**
                                          * Comparison
                                          * operator. Result is true
                                          * if either the first row
                                          * number is smaller or if
                                          * the row numbers are
                                          * equal and the first
                                          * index is smaller.
                                          */
	bool operator < (const const_iterator&) const;

      private:
                                         /**
                                          * Store an object of the
                                          * accessor class.
                                          */
        Accessor accessor;
    };
    
				     /**
				      * Type of matrix entries. In analogy to
				      * the STL container classes.
				      */
    typedef number value_type;
    
				     /**
				      * Constructor; initializes the matrix to
				      * be empty, without any structure, i.e.
				      * the matrix is not usable at all. This
				      * constructor is therefore only useful
				      * for matrices which are members of a
				      * class. All other matrices should be
				      * created at a point in the data flow
				      * where all necessary information is
				      * available.
				      *
				      * You have to initialize
				      * the matrix before usage with
				      * @p{reinit(SparsityPattern)}.
				      */
    SparseMatrix ();

				     /**
				      * Copy constructor. This constructor is
				      * only allowed to be called if the matrix
				      * to be copied is empty. This is for the
				      * same reason as for the
				      * @p{SparsityPattern}, see there for the
				      * details.
				      *
				      * If you really want to copy a whole
				      * matrix, you can do so by using the
				      * @p{copy_from} function.
				      */
    SparseMatrix (const SparseMatrix &);

				     /**
				      * Constructor. Takes the given
				      * matrix sparsity structure to
				      * represent the sparsity pattern
				      * of this matrix. You can change
				      * the sparsity pattern later on
				      * by calling the @p{reinit}
				      * function.
				      *
				      * You have to make sure that the
				      * lifetime of the sparsity
				      * structure is at least as long
				      * as that of this matrix or as
				      * long as @p{reinit} is not
				      * called with a new sparsity
				      * structure.
				      *
				      * The constructor is marked
				      * explicit so as to disallow
				      * that someone passes a sparsity
				      * pattern in place of a sparse
				      * matrix to some function, where
				      * an empty matrix would be
				      * generated then.
				      */
    explicit SparseMatrix (const SparsityPattern &sparsity);
    
				     /**
				      * Destructor. Free all memory, but do not
				      * release the memory of the sparsity
				      * structure.
				      */
    virtual ~SparseMatrix ();

				     /** 
				      * Pseudo operator only copying
				      * empty objects.
				      */
    SparseMatrix<number>& operator = (const SparseMatrix<number> &);

				     /**
				      * Reinitialize the object but
				      * keep to the sparsity pattern
				      * previously used.  This may be
				      * necessary if you @p{reinit}'d
				      * the sparsity structure and
				      * want to update the size of the
				      * matrix.
				      *
				      * Note that memory is only
				      * reallocated if the new size
				      * exceeds the old size. If that
				      * is not the case, the allocated
				      * memory is not reduced. However,
				      * if the sparsity structure is
				      * empty (i.e. the dimensions are
				      * zero), then all memory is
				      * freed.
				      *
				      * If the sparsity pattern has
				      * not changed, then the effect
				      * of this function is simply to
				      * reset all matrix entries to
				      * zero.
				      */
    virtual void reinit ();

				     /**
				      * Reinitialize the sparse matrix
				      * with the given sparsity
				      * pattern. The latter tells the
				      * matrix how many nonzero
				      * elements there need to be
				      * reserved.
				      *
				      * Regarding memory allocation,
				      * the same applies as said
				      * above.
				      *
				      * You have to make sure that the
				      * lifetime of the sparsity
				      * structure is at least as long
				      * as that of this matrix or as
				      * long as @p{reinit} is not called
				      * with a new sparsity structure.
				      *
				      * The elements of the matrix are
				      * set to zero by this function.
				      */
    virtual void reinit (const SparsityPattern &sparsity);

				     /**
				      * Release all memory and return
				      * to a state just like after
				      * having called the default
				      * constructor. It also forgets
				      * the sparsity pattern it was
				      * previously tied to.
				      */
    virtual void clear ();
    
				     /**
				      * Return whether the object is
				      * empty. It is empty if either
				      * both dimensions are zero or no
				      * @p{SparsityPattern} is
				      * associated.
				      */
    bool empty () const;

				     /**
				      * Return the dimension of the
				      * image space.  To remember: the
				      * matrix is of dimension
				      * $m \times n$.
				      */
    unsigned int m () const;
    
				     /**
				      * Return the dimension of the
				      * range space.  To remember: the
				      * matrix is of dimension
				      * $m \times n$.
				      */
    unsigned int n () const;

				     /**
				      * Return the number of nonzero
				      * elements of this
				      * matrix. Actually, it returns
				      * the number of entries in the
				      * sparsity pattern; if any of
				      * the entries should happen to
				      * be zero, it is counted anyway.
				      */
    unsigned int n_nonzero_elements () const;

				     /**
				      * Return the number of actually
				      * nonzero elements of this
				      * matrix.
				      *
				      * Note, that this function does
				      * (in contrary to the
				      * @p{n_nonzero_elements}) NOT
				      * count all entries of the
				      * sparsity pattern but only the
				      * ones that are nonzero.
				      */
    unsigned int n_actually_nonzero_elements () const;
    
				     /**
				      * Set the element @p{(i,j)} to @p{value}.
				      * Throws an error if the entry does
				      * not exist. Still, it is allowed to store
				      * zero values in non-existent fields.
				      */
    void set (const unsigned int i, const unsigned int j,
	      const number value);
    
				     /**
				      * Multiply the entire matrix by a
				      * fixed factor.
				      */
    SparseMatrix & operator *= (const number factor);
    
				     /**
				      * Divide the entire matrix by a
				      * fixed factor.
				      */
    SparseMatrix & operator /= (const number factor);
    
				     /**
				      * Add @p{value} to the element
				      * @p{(i,j)}.  Throws an error if
				      * the entry does not
				      * exist. Still, it is allowed to
				      * store zero values in
				      * non-existent fields.
				      */
    void add (const unsigned int i, const unsigned int j,
	      const number value);

				     /**
				      * Symmetrize the matrix by
				      * forming the mean value between
				      * the existing matrix and its
				      * transpose, $A = \frac 12(A+A^T)$.
				      *
				      * This operation assumes that
				      * the underlying sparsity
				      * pattern represents a symmetric
				      * object. If this is not the
				      * case, then the result of this
				      * operation will not be a
				      * symmetric matrix, since it
				      * only explicitly symmetrizes
				      * by looping over the lower left
				      * triangular part for efficiency
				      * reasons; if there are entries
				      * in the upper right triangle,
				      * then these elements are missed
				      * in the
				      * symmetrization. Symmetrization
				      * of the sparsity pattern can be
				      * obtain by the
				      * @ref{SparsityPattern}@p{::symmetrize}
				      * function.
				      */
    void symmetrize ();
    
				     /**
				      * Copy the given matrix to this
				      * one.  The operation throws an
				      * error if the sparsity patterns
				      * of the two involved matrices
				      * do not point to the same
				      * object, since in this case the
				      * copy operation is
				      * cheaper. Since this operation
				      * is notheless not for free, we
				      * do not make it available
				      * through @p{operator =}, since
				      * this may lead to unwanted
				      * usage, e.g. in copy arguments
				      * to functions, which should
				      * really be arguments by
				      * reference.
				      *
				      * The source matrix may be a matrix
				      * of arbitrary type, as long as its
				      * data type is convertible to the
				      * data type of this matrix.
				      *
				      * The function returns a reference to
				      * @p{this}.
				      */
    template <typename somenumber>
    SparseMatrix<number> &
    copy_from (const SparseMatrix<somenumber> &source);

				     /**
				      * This function is complete
				      * analogous to the
				      * @ref{SparsityPattern}@p{::copy_from}
				      * function in that it allows to
				      * initialize a whole matrix in
				      * one step. See there for more
				      * information on argument types
				      * and their meaning. You can
				      * also find a small example on
				      * how to use this function
				      * there.
				      *
				      * The only difference to the
				      * cited function is that the
				      * objects which the inner
				      * iterator points to need to be
				      * of type @p{std::pair<unsigned int, value},
				      * where @p{value}
				      * needs to be convertible to the
				      * element type of this class, as
				      * specified by the @p{number}
				      * template argument.
				      *
				      * Previous content of the matrix
				      * is overwritten. Note that the
				      * entries specified by the input
				      * parameters need not
				      * necessarily cover all elements
				      * of the matrix. Elements not
				      * covered remain untouched.
				      */
    template <typename ForwardIterator>
    void copy_from (const ForwardIterator begin,
		    const ForwardIterator end);    

				     /**
				      * Copy the nonzero entries of a
				      * full matrix into this
				      * object. Previous content is
				      * deleted. Note that the
				      * underlying sparsity pattern
				      * must be appropriate to hold
				      * the nonzero entries of the
				      * full matrix.
				      */
    template <typename somenumber>
    void copy_from (const FullMatrix<somenumber> &matrix);
    
				     /**
				      * Add @p{matrix} scaled by
				      * @p{factor} to this matrix. The
				      * function throws an error if
				      * the sparsity patterns of the
				      * two involved matrices do not
				      * point to the same object,
				      * since in this case the
				      * operation is cheaper.
				      *
				      * The source matrix may be a matrix
				      * of arbitrary type, as long as its
				      * data type is convertible to the
				      * data type of this matrix.
				      */
    template <typename somenumber>
    void add_scaled (const number factor,
		     const SparseMatrix<somenumber> &matrix);
    
				     /**
				      * Return the value of the entry
				      * (i,j).  This may be an
				      * expensive operation and you
				      * should always take care where
				      * to call this function.  In
				      * order to avoid abuse, this
				      * function throws an exception
				      * if the required element does
				      * not exist in the matrix.
				      *
				      * In case you want a function
				      * that returns zero instead (for
				      * entries that are not in the
				      * sparsity pattern of the
				      * matrix), use the @p{el}
				      * function.
				      */
    number operator () (const unsigned int i,
			const unsigned int j) const;

				     /**
				      * This function is mostly like
				      * @p{operator()} in that it
				      * returns the value of the
				      * matrix entry @p{(i,j)}. The only
				      * difference is that if this
				      * entry does not exist in the
				      * sparsity pattern, then instead
				      * of raising an exception, zero
				      * is returned. While this may be
				      * convenient in some cases, note
				      * that it is simple to write
				      * algorithms that are slow
				      * compared to an optimal
				      * solution, since the sparsity
				      * of the matrix is not used.
				      */
    number el (const unsigned int i,
	       const unsigned int j) const;

				     /**
				      * Return the main diagonal element in
				      * the @p{i}th row. This function throws an
				      * error if the matrix is not square.
				      *
				      * This function is considerably
				      * faster than the @p{operator()},
				      * since for square matrices, the
				      * diagonal entry is always the
				      * first to be stored in each row
				      * and access therefore does not
				      * involve searching for the
				      * right column number.
				      */
    number diag_element (const unsigned int i) const;

				     /**
				      * Same as above, but return a
				      * writeable reference. You're
				      * sure you know what you do?
				      */
    number & diag_element (const unsigned int i);

				     /**
				      * Access to values in internal
				      * mode.  Returns the value of
				      * the @p{index}th entry in
				      * @p{row}. Here, @p{index} refers to
				      * the internal representation of
				      * the matrix, not the column. Be
				      * sure to understand what you are
				      * doing here.
				      */
    number raw_entry (const unsigned int row,
		      const unsigned int index) const;
    
    				     /**
				      * This is for hackers. Get
				      * access to the @p{i}th element of
				      * this matrix. The elements are
				      * stored in a consecutive way,
				      * refer to the @p{SparsityPattern}
				      * class for more details.
				      *
				      * You should use this interface
				      * very carefully and only if you
				      * are absolutely sure to know
				      * what you do. You should also
				      * note that the structure of
				      * these arrays may change over
				      * time.  If you change the
				      * layout yourself, you should
				      * also rename this function to
				      * avoid programs relying on
				      * outdated information!
				      */
    number global_entry (const unsigned int i) const;

				     /**
				      * Same as above, but with write
				      * access.  You certainly know
				      * what you do?
				      */
    number & global_entry (const unsigned int i);

				     /**
				      * Matrix-vector multiplication:
				      * let $dst = M*src$ with $M$
				      * being this matrix.
				      */
    template <typename somenumber>
    void vmult (Vector<somenumber>       &dst,
		const Vector<somenumber> &src) const;
    
				     /**
				      * Matrix-vector multiplication:
				      * let $dst = M^T*src$ with $M$
				      * being this matrix. This
				      * function does the same as
				      * @p{vmult} but takes the
				      * transposed matrix.
				      */
    template <typename somenumber>
    void Tvmult (Vector<somenumber>       &dst,
		 const Vector<somenumber> &src) const;
  
				     /**
				      * Adding Matrix-vector
				      * multiplication. Add $M*src$ on
				      * $dst$ with $M$ being this
				      * matrix.
				      */
    template <typename somenumber>
    void vmult_add (Vector<somenumber>       &dst,
		    const Vector<somenumber> &src) const;
    
				     /**
				      * Adding Matrix-vector
				      * multiplication. Add $M^T*src$
				      * to $dst$ with $M$ being this
				      * matrix. This function does the
				      * same as @p{vmult_add} but takes
				      * the transposed matrix.
				      */
    template <typename somenumber>
    void Tvmult_add (Vector<somenumber>       &dst,
		     const Vector<somenumber> &src) const;
  
				     /**
				      * Return the square of the norm
				      * of the vector $v$ with respect
				      * to the norm induced by this
				      * matrix,
				      * i.e. $\left(v,Mv\right)$. This
				      * is useful, e.g. in the finite
				      * element context, where the
				      * $L_2$ norm of a function
				      * equals the matrix norm with
				      * respect to the mass matrix of
				      * the vector representing the
				      * nodal values of the finite
				      * element function.
				      *
				      * Obviously, the matrix needs to
				      * be square for this operation.
				      */
    template <typename somenumber>
    somenumber matrix_norm_square (const Vector<somenumber> &v) const;

				     /**
				      * Compute the matrix scalar
				      * product $\left(u,Mv\right)$.
				      */
    template <typename somenumber>
    somenumber matrix_scalar_product (const Vector<somenumber> &u,
				      const Vector<somenumber> &v) const;
    
    				     /**
				      * Return the l1-norm of the matrix, that is
				      * $|M|_1=max_{all columns j}\sum_{all 
				      * rows i} |M_ij|$,
				      * (max. sum of columns).
				      * This is the
				      * natural matrix norm that is compatible
				      * to the l1-norm for vectors, i.e.
				      * $|Mv|_1\leq |M|_1 |v|_1$.
				      * (cf. Haemmerlin-Hoffmann : Numerische Mathematik)
				      */
    number l1_norm () const;

    				     /**
				      * Return the linfty-norm of the
				      * matrix, that is
				      * $|M|_infty=max_{all rows i}\sum_{all 
				      * columns j} |M_ij|$,
				      * (max. sum of rows).
				      * This is the
				      * natural matrix norm that is compatible
				      * to the linfty-norm of vectors, i.e.
				      * $|Mv|_infty \leq |M|_infty |v|_infty$.
				      * (cf. Haemmerlin-Hoffmann : Numerische Mathematik)
				      */
    number linfty_norm () const;

				     /**
				      * Compute the residual of an
				      * equation @p{Mx=b}, where the
				      * residual is defined to be
				      * @p{r=b-Mx} with @p{x} typically
				      * being an approximate of the
				      * true solution of the
				      * equation. Write the residual
				      * into @p{dst}. The l2 norm of
				      * the residual vector is
				      * returned.
				      */
    template <typename somenumber>
    somenumber residual (Vector<somenumber>       &dst,
			 const Vector<somenumber> &x,
			 const Vector<somenumber> &b) const;
    
				     /**
				      * Apply the Jacobi
				      * preconditioner, which
				      * multiplies every element of
				      * the @p{src} vector by the
				      * inverse of the respective
				      * diagonal element and
				      * multiplies the result with the
				      * damping factor @p{omega}.
				      */
    template <typename somenumber>
    void precondition_Jacobi (Vector<somenumber>       &dst,
			      const Vector<somenumber> &src,
			      const number              omega = 1.) const;

				     /**
				      * Apply SSOR preconditioning to
				      * @p{src}.
				      */
    template <typename somenumber>
    void precondition_SSOR (Vector<somenumber>       &dst,
			    const Vector<somenumber> &src,
			    const number              om = 1.) const;

				     /**
				      * Apply SOR preconditioning matrix to @p{src}.
				      * The result of this method is
				      * $dst = (om D - L)^{-1} src$.
				      */
    template <typename somenumber>
    void precondition_SOR (Vector<somenumber>       &dst,
			   const Vector<somenumber> &src,
 			   const number              om = 1.) const;
    
				     /**
				      * Apply transpose SOR preconditioning matrix to @p{src}.
				      * The result of this method is
				      * $dst = (om D - U)^{-1} src$.
				      */
    template <typename somenumber>
    void precondition_TSOR (Vector<somenumber>       &dst,
			    const Vector<somenumber> &src,
			    const number              om = 1.) const;
    
				     /**
				      * Perform SSOR preconditioning
				      * in-place.  Apply the
				      * preconditioner matrix without
				      * copying to a second vector.
				      * @p{omega} is the relaxation
				      * parameter.
				      */
    template <typename somenumber>
    void SSOR (Vector<somenumber> &v,
	       const number        omega = 1.) const;

				     /**
				      * Perform an SOR preconditioning in-place.
				      * The result is $v = (\omega D - L)^{-1} v$.
				      * @p{omega} is the damping parameter.
				      */
    template <typename somenumber>
    void SOR (Vector<somenumber> &v,
	      const number        om = 1.) const;

				     /**
				      * Perform a transpose SOR preconditioning in-place.
				      * The result is $v = (\omega D - L)^{-1} v$.
				      * @p{omega} is the damping parameter.
				      */
    template <typename somenumber>
    void TSOR (Vector<somenumber> &v,
	      const number        om = 1.) const;

				     /**
				      * Perform a permuted SOR
				      * preconditioning in-place.
				      *
				      * The standard SOR method is
				      * applied in the order
				      * prescribed by @p{permutation},
				      * that is, first the row
				      * @p{permutation[0]}, then
				      * @p{permutation[1]} and so
				      * on. For efficiency reasons,
				      * the permutation as well as its
				      * inverse are required.
				      *
				      * @p{omega} is the relaxation
				      * parameter.
				      */
    template <typename somenumber>
    void PSOR (Vector<somenumber> &v,
	      const std::vector<unsigned int>& permutation,
	      const std::vector<unsigned int>& inverse_permutation,
	      const number        om = 1.) const;

				     /**
				      * Perform a transposed permuted SOR
				      * preconditioning in-place.
				      *
				      * The transposed SOR method is
				      * applied in the order
				      * prescribed by @p{permutation},
				      * that is, first the row
				      * @p{permutation[m()-1]}, then
				      * @p{permutation[m()-2]} and so
				      * on. For efficiency reasons,
				      * the permutation as well as its
				      * inverse are required.
				      *
				      * @p{omega} is the relaxation
				      * parameter.
				      */
    template <typename somenumber>
    void TPSOR (Vector<somenumber> &v,
	      const std::vector<unsigned int>& permutation,
	      const std::vector<unsigned int>& inverse_permutation,
	      const number        om = 1.) const;

				     /**
				      * Do one SOR step on @p{v}.
				      * Performs a direct SOR step
				      * with right hand side @p{b}.
				      */
    template <typename somenumber>
    void SOR_step (Vector<somenumber> &v,
		   const Vector<somenumber> &b,
		   const number        om = 1.) const;

				     /**
				      * Do one adjoint SOR step on
				      * @p{v}.  Performs a direct TSOR
				      * step with right hand side @p{b}.
				      */
    template <typename somenumber>
    void TSOR_step (Vector<somenumber> &v,
		    const Vector<somenumber> &b,
		    const number        om = 1.) const;

				     /**
				      * Do one adjoint SSOR step on
				      * @p{v}.  Performs a direct SSOR
				      * step with right hand side @p{b}
				      * by performing TSOR after SOR.
				      */
    template <typename somenumber>
    void SSOR_step (Vector<somenumber> &v,
		    const Vector<somenumber> &b,
		    const number        om = 1.) const;

				     /**
				      * Return a (constant) reference
				      * to the underlying sparsity
				      * pattern of this matrix.
				      *
				      * Though the return value is
				      * declared @p{const}, you should
				      * be aware that it may change if
				      * you call any nonconstant
				      * function of objects which
				      * operate on it.
				      */
    const SparsityPattern & get_sparsity_pattern () const;

				     /**
				      * STL-like iterator with the
				      * first entry.
				      */
    const_iterator begin () const;

				     /**
				      * Final iterator.
				      */
    const_iterator end () const;
    
				     /**
				      * STL-like iterator with the
				      * first entry of row @p{r}.
				      */
    const_iterator begin (const unsigned int r) const;

				     /**
				      * Final iterator of row @p{r}.
				      */
    const_iterator end (const unsigned int r) const;
    
				     /**
				      * Print the matrix to the given
				      * stream, using the format
				      * @p{(line,col) value}, i.e. one
				      * nonzero entry of the matrix
				      * per line.
				      */
    void print (std::ostream &out) const;

				     /**
				      * Print the matrix in the usual
				      * format, i.e. as a matrix and
				      * not as a list of nonzero
				      * elements. For better
				      * readability, elements not in
				      * the matrix are displayed as
				      * empty space, while matrix
				      * elements which are explicitly
				      * set to zero are displayed as
				      * such.
				      *
				      * The parameters allow for a
				      * flexible setting of the output
				      * format: @p{precision} and
				      * @p{scientific} are used to
				      * determine the number format,
				      * where @p{scientific} = @p{false}
				      * means fixed point notation.  A
				      * zero entry for @p{width} makes
				      * the function compute a width,
				      * but it may be changed to a
				      * positive value, if output is
				      * crude.
				      *
				      * Additionally, a character for
				      * an empty value may be
				      * specified.
				      *
				      * Finally, the whole matrix can
				      * be multiplied with a common
				      * denominator to produce more
				      * readable output, even
				      * integers.
				      *
				      * This function
				      * may produce @em{large} amounts of
				      * output if applied to a large matrix!
				      */
    void print_formatted (std::ostream       &out,
			  const unsigned int  precision   = 3,
			  const bool          scientific  = true,
			  const unsigned int  width       = 0,
			  const char         *zero_string = " ",
			  const double        denominator = 1.) const;

				     /**
				      * Write the data of this object
				      * en bloc to a file. This is
				      * done in a binary mode, so the
				      * output is neither readable by
				      * humans nor (probably) by other
				      * computers using a different
				      * operating system of number
				      * format.
				      *
				      * The purpose of this function
				      * is that you can swap out
				      * matrices and sparsity pattern
				      * if you are short of memory,
				      * want to communicate between
				      * different programs, or allow
				      * objects to be persistent
				      * across different runs of the
				      * program.
				      */
    void block_write (std::ostream &out) const;

				     /**
				      * Read data that has previously
				      * been written by
				      * @p{block_write} en block from
				      * a file. This is done using the
				      * inverse operations to the
				      * above function, so it is
				      * reasonably fast because the
				      * bitstream is not interpreted
				      * except for a few numbers up
				      * front.
				      *
				      * The object is resized on this
				      * operation, and all previous
				      * contents are lost. Note,
				      * however, that no checks are
				      * performed whether new data and
				      * the underlying
				      * @ref{SparsityPattern} object
				      * fit together. It is your
				      * responsibility to make sure
				      * that the sparsity pattern and
				      * the data to be read match.
				      *
				      * A primitive form of error
				      * checking is performed which
				      * will recognize the bluntest
				      * attempts to interpret some
				      * data as a vector stored
				      * bitwise to a file, but not
				      * more.
				      */
    void block_read (std::istream &in);

				     /**
				      * Determine an estimate for the
				      * memory consumption (in bytes)
				      * of this object.
				      */
    unsigned int memory_consumption () const;
    
				     /**
				      * Exception
				      */
    DeclException0 (ExcNotCompressed);
				     /**
				      * Exception
				      */
    DeclException0 (ExcMatrixNotInitialized);
				     /**
				      * Exception
				      */
    DeclException2 (ExcInvalidIndex,
		    int, int,
		    << "The entry with index <" << arg1 << ',' << arg2
		    << "> does not exist.");
				     /**
				      * Exception
				      */
    DeclException1 (ExcInvalidIndex1,
		    int,
		    << "The index " << arg1 << " is not in the allowed range.");
				     /**
				      * Exception
				      */
    DeclException0 (ExcMatrixNotSquare);
				     /**
				      * Exception
				      */
    DeclException0 (ExcDifferentSparsityPatterns);
				     /**
				      * Exception
				      */
    DeclException0 (ExcInvalidConstructorCall);
				     /**
				      * Exception
				      */
    DeclException2 (ExcIteratorRange,
		    int, int,
		    << "The iterators denote a range of " << arg1
		    << " elements, but the given number of rows was " << arg2);
    
  private:
				     /**
				      * Pointer to the sparsity
				      * pattern used for this
				      * matrix. In order to guarantee
				      * that it is not deleted while
				      * still in use, we subscribe to
				      * it using the @p{SmartPointer}
				      * class.
				      */
    SmartPointer<const SparsityPattern> cols;

				     /**
				      * Array of values for all the
				      * nonzero entries. The position
				      * within the matrix, i.e.  the
				      * row and column number for a
				      * given entry can only be
				      * deduced using the sparsity
				      * pattern. The same holds for
				      * the more common operation of
				      * finding an entry by its
				      * coordinates.
				      */
    number *val;

				     /**
				      * Allocated size of
				      * @p{val}. This can be larger
				      * than the actually used part if
				      * the size of the matrix was
				      * reduced somewhen in the past
				      * by associating a sparsity
				      * pattern with a smaller size to
				      * this object, using the
				      * @p{reinit} function.
				      */
    unsigned int max_len;

				     /**
				      * Version of @p{vmult} which only
				      * performs its actions on the
				      * region defined by
				      * @p{[begin_row,end_row)}. This
				      * function is called by @p{vmult}
				      * in the case of enabled
				      * multithreading.
				      */
    template <typename somenumber>
    void threaded_vmult (Vector<somenumber>       &dst,
			 const Vector<somenumber> &src,
			 const unsigned int        begin_row,
			 const unsigned int        end_row) const;

				     /**
				      * Version of
				      * @p{matrix_norm_square} which
				      * only performs its actions on
				      * the region defined by
				      * @p{[begin_row,end_row)}. This
				      * function is called by
				      * @p{matrix_norm_square} in the
				      * case of enabled
				      * multithreading.
				      */
    template <typename somenumber>
    void threaded_matrix_norm_square (const Vector<somenumber> &v,
				      const unsigned int        begin_row,
				      const unsigned int        end_row,
				      somenumber               *partial_sum) const;

    				     /**
				      * Version of
				      * @p{matrix_scalar_product} which
				      * only performs its actions on
				      * the region defined by
				      * @p{[begin_row,end_row)}. This
				      * function is called by
				      * @p{matrix_scalar_product} in the
				      * case of enabled
				      * multithreading.
				      */
    template <typename somenumber>
    void threaded_matrix_scalar_product (const Vector<somenumber> &u,
					 const Vector<somenumber> &v,
					 const unsigned int        begin_row,
					 const unsigned int        end_row,
					 somenumber               *partial_sum) const;

				     /**
				      * Version of @p{residual} which
				      * only performs its actions on
				      * the region defined by
				      * @p{[begin_row,end_row)} (these
				      * numbers are the components of
				      * @p{interval}). This function is
				      * called by @p{residual} in the
				      * case of enabled
				      * multithreading.
				      */
    template <typename somenumber>
    void threaded_residual (Vector<somenumber>       &dst,
			    const Vector<somenumber> &u,
			    const Vector<somenumber> &b,
			    const std::pair<unsigned int,unsigned int> interval,
			    somenumber               *partial_norm) const;

				     // make all other sparse matrices
				     // friends
    template <typename somenumber> friend class SparseMatrix;
};

/*@}*/
/*---------------------- Inline functions -----------------------------------*/



template <typename number>
inline
unsigned int SparseMatrix<number>::m () const
{
  Assert (cols != 0, ExcMatrixNotInitialized());
  return cols->rows;
}


template <typename number>
inline
unsigned int SparseMatrix<number>::n () const
{
  Assert (cols != 0, ExcMatrixNotInitialized());
  return cols->cols;
}


template <typename number>
inline
void SparseMatrix<number>::set (const unsigned int i,
				const unsigned int j,
				const number value)
{
  Assert (cols != 0, ExcMatrixNotInitialized());
				   // it is allowed to set elements of
				   // the matrix that are not part of
				   // the sparsity pattern, if the
				   // value to which we set it is zero
  const unsigned int index = cols->operator()(i,j);
  Assert ((index != SparsityPattern::invalid_entry) ||
	  (value == 0.),
	  ExcInvalidIndex(i,j));

  if (index != SparsityPattern::invalid_entry)
    val[index] = value;
}



template <typename number>
inline
void SparseMatrix<number>::add (const unsigned int i,
				const unsigned int j,
				const number value)
{
  Assert (cols != 0, ExcMatrixNotInitialized());

  const unsigned int index = cols->operator()(i,j);
  Assert ((index != SparsityPattern::invalid_entry) ||
	  (value == 0.),
	  ExcInvalidIndex(i,j));

  if (value != 0.)
    val[index] += value;
}



template <typename number>
inline
SparseMatrix<number> &
SparseMatrix<number>::operator *= (const number factor)
{
  Assert (cols != 0, ExcMatrixNotInitialized());
  Assert (val != 0, ExcMatrixNotInitialized());

  number             *val_ptr    = &val[0];
  const number *const end_ptr    = &val[cols->n_nonzero_elements()];

  while (val_ptr != end_ptr)
    *val_ptr++ *= factor;

  return *this;
}



template <typename number>
inline
SparseMatrix<number> &
SparseMatrix<number>::operator /= (const number factor)
{
  Assert (cols != 0, ExcMatrixNotInitialized());
  Assert (val != 0, ExcMatrixNotInitialized());
  Assert (factor !=0, ExcDivideByZero());

  const number factor_inv = 1. / factor;

  number             *val_ptr    = &val[0];
  const number *const end_ptr    = &val[cols->n_nonzero_elements()];

  while (val_ptr != end_ptr)
    *val_ptr++ *= factor_inv;

  return *this;
}



template <typename number>
inline
number SparseMatrix<number>::operator () (const unsigned int i,
					  const unsigned int j) const
{
  Assert (cols != 0, ExcMatrixNotInitialized());
  Assert (cols->operator()(i,j) != SparsityPattern::invalid_entry,
	  ExcInvalidIndex(i,j));
  return val[cols->operator()(i,j)];
}



template <typename number>
inline
number SparseMatrix<number>::el (const unsigned int i,
				 const unsigned int j) const
{
  Assert (cols != 0, ExcMatrixNotInitialized());
  const unsigned int index = cols->operator()(i,j);

  if (index != SparsityPattern::invalid_entry)
    return val[index];
  else
    return 0;
}



template <typename number>
inline
number SparseMatrix<number>::diag_element (const unsigned int i) const
{
  Assert (cols != 0, ExcMatrixNotInitialized());
  Assert (cols->optimize_diagonal(),  ExcMatrixNotSquare());
  Assert (i<m(), ExcInvalidIndex1(i));
  
				   // Use that the first element in each
				   // row of a square matrix is the main
				   // diagonal
  return val[cols->rowstart[i]];
}



template <typename number>
inline
number & SparseMatrix<number>::diag_element (const unsigned int i)
{
  Assert (cols != 0, ExcMatrixNotInitialized());
  Assert (cols->optimize_diagonal(),  ExcMatrixNotSquare());
  Assert (i<m(), ExcInvalidIndex1(i));
  
				   // Use that the first element in each
				   // row of a square matrix is the main
				   // diagonal
  return val[cols->rowstart[i]];
}



template <typename number>
inline
number
SparseMatrix<number>::raw_entry (const unsigned int row,
				 const unsigned int index) const
{
  Assert(row<cols->rows, ExcIndexRange(row,0,cols->rows));
  Assert(index<cols->row_length(row),
	 ExcIndexRange(index,0,cols->row_length(row)));

  return val[cols->rowstart[row]+index];
}



template <typename number>
inline
number SparseMatrix<number>::global_entry (const unsigned int j) const
{
  Assert (cols != 0, ExcMatrixNotInitialized());
  Assert (j < cols->n_nonzero_elements(),
	  ExcIndexRange (j, 0, cols->n_nonzero_elements()));
  
  return val[j];
}



template <typename number>
inline
number & SparseMatrix<number>::global_entry (const unsigned int j)
{
  Assert (cols != 0, ExcMatrixNotInitialized());
  Assert (j < cols->n_nonzero_elements(),
	  ExcIndexRange (j, 0, cols->n_nonzero_elements()));

  return val[j];
}



template <typename number>
template <typename ForwardIterator>
void
SparseMatrix<number>::copy_from (const ForwardIterator begin,
				 const ForwardIterator end)
{
  Assert (static_cast<unsigned int>(std::distance (begin, end)) == m(),
	  ExcIteratorRange (std::distance (begin, end), m()));

				   // for use in the inner loop, we
				   // define a typedef to the type of
				   // the inner iterators
  typedef typename std::iterator_traits<ForwardIterator>::value_type::const_iterator inner_iterator;
  unsigned int row=0;
  for (ForwardIterator i=begin; i!=end; ++i, ++row)
    {
      const inner_iterator end_of_row = i->end();
      for (inner_iterator j=i->begin(); j!=end_of_row; ++j)
					 // write entries
	set (row, j->first, j->second);
    };
}


//----------------------------------------------------------------------//

template <typename number>
inline
SparseMatrix<number>::const_iterator::Accessor::
Accessor (const SparseMatrix<number>* matrix,
          const unsigned int          r,
          const unsigned short        i)
		:
		matrix(matrix),
		a_row(r),
		a_index(i)
{}


template <typename number>
inline
unsigned int
SparseMatrix<number>::const_iterator::Accessor::row() const
{
  return a_row;
}


template <typename number>
inline
unsigned int
SparseMatrix<number>::const_iterator::Accessor::column() const
{
  const SparsityPattern& pat = matrix->get_sparsity_pattern();
  return pat.get_column_numbers()[pat.get_rowstart_indices()[a_row]+a_index];
}


template <typename number>
inline
unsigned short
SparseMatrix<number>::const_iterator::Accessor::index() const
{
  return a_index;
}



template <typename number>
inline
number
SparseMatrix<number>::const_iterator::Accessor::value() const
{
  return matrix->raw_entry(a_row, a_index);
}


template <typename number>
inline
SparseMatrix<number>::const_iterator::
const_iterator(const SparseMatrix<number> *matrix,
               const unsigned int          r,
               const unsigned short        i)
		:
		accessor(matrix, r, i)
{}


template <typename number>
inline
typename SparseMatrix<number>::const_iterator &
SparseMatrix<number>::const_iterator::operator++ ()
{
  Assert (accessor.a_row < accessor.matrix->m(), ExcIteratorPastEnd());
  
  ++accessor.a_index;
  if (accessor.a_index >=
      accessor.matrix->get_sparsity_pattern().row_length(accessor.a_row))
    {
      accessor.a_index = 0;
      accessor.a_row++;
    }
  return *this;
}


template <typename number>
inline
const typename SparseMatrix<number>::const_iterator::Accessor &
SparseMatrix<number>::const_iterator::operator* () const
{
  return accessor;
}


template <typename number>
inline
const typename SparseMatrix<number>::const_iterator::Accessor *
SparseMatrix<number>::const_iterator::operator-> () const
{
  return &accessor;
}


template <typename number>
inline
bool
SparseMatrix<number>::const_iterator::
operator == (const const_iterator& other) const
{
  return (accessor.row() == other.accessor.row() &&
          accessor.index() == other.accessor.index());
}


template <typename number>
inline
bool
SparseMatrix<number>::const_iterator::
operator != (const const_iterator& other) const
{
  return ! (*this == other);
}


template <typename number>
inline
bool
SparseMatrix<number>::const_iterator::
operator < (const const_iterator& other) const
{
  return (accessor.row() < other.accessor.row() ||
	  (accessor.row() == other.accessor.row() &&
           accessor.index() < other.accessor.index()));
}


template <typename number>
inline
typename SparseMatrix<number>::const_iterator
SparseMatrix<number>::begin () const
{
  return const_iterator(this, 0, 0);
}


template <typename number>
inline
typename SparseMatrix<number>::const_iterator
SparseMatrix<number>::end () const
{
  return const_iterator(this, m(), 0);
}


template <typename number>
inline
typename SparseMatrix<number>::const_iterator
SparseMatrix<number>::begin (const unsigned int r) const
{
  Assert (r<m(), ExcIndexRange(r,0,m()));
  return const_iterator(this, r, 0);
}



template <typename number>
inline
typename SparseMatrix<number>::const_iterator
SparseMatrix<number>::end (const unsigned int r) const
{
  Assert (r<m(), ExcIndexRange(r,0,m()));
  return const_iterator(this, r+1, 0);
}




/*----------------------------   sparse_matrix.h     ---------------------------*/

#endif
/*----------------------------   sparse_matrix.h     ---------------------------*/
