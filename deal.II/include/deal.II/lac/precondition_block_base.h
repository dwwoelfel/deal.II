//---------------------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------
#ifndef __deal2__precondition_block_base_h
#define __deal2__precondition_block_base_h


#include <base/config.h>
#include <base/exceptions.h>
#include <base/subscriptor.h>
#include <base/smartpointer.h>
#include <base/memory_consumption.h>
#include <lac/householder.h>
#include <lac/lapack_full_matrix.h>

#include <vector>

DEAL_II_NAMESPACE_OPEN

template <typename number> class FullMatrix;
template <typename number> class Vector;

/**
 * A class storing the inverse diagonal blocks for block
 * preconditioners and block relaxation methods.
 *
 * This class does the book keeping for preconditioners and relaxation
 * methods based on inverting blocks on the diagonal of a matrix.
 * It allows us to either store all diagonal blocks and their
 * inverses or the same block for each entry, and it keeps track of
 * the choice. Thus, after initializing it and filling the inverse
 * diagonal blocks correctly, a derived class can use inverse() with
 * an integer argument referring to the block number.
 *
 * Additionally, it allows the storage of the original diagonal
 * blocks, not only the inverses. These are for instance used in the
 * intermediate step of the SSOR preconditioner.
 *
 * @author Guido Kanschat
 * @date 2010
 */
template <typename number>
class PreconditionBlockBase
{
  public:
				     /**
				      * Choose a method for inverting
				      * the blocks, and thus the data
				      * type for the inverses.
				      */
    enum Inversion
    {
					   /**
					    * Use the standard
					    * Gauss-Jacobi method
					    * implemented in FullMatrix::inverse().
					    */
	  gauss_jordan,
					   /**
					    * Use QR decomposition of
					    * the Householder class.
					    */
	  householder,
					   /**
					    * Use the singular value
					    * decomposition of LAPACKFullMatrix.
					    */
	  svd
    };
    
				     /**
				      * Constructor initializing
				      * default values.
				      */
    PreconditionBlockBase(bool store_diagonals = false,
			  Inversion method = gauss_jordan);

				     /**
				      * The virtual destructor
				      */
    ~PreconditionBlockBase();
    
    				     /**
				      * Deletes the inverse diagonal
				      * block matrices if existent hence
				      * leaves the class in the state
				      * that it had directly after
				      * calling the constructor.
				      */
    void clear();

				     /**
				      * Resize to this number of
				      * diagonal blocks with the given
				      * block size. If
				      * <tt>compress</tt> is true,
				      * then only one block will be
				      * stored.
				      */
    void reinit(unsigned int nblocks, unsigned int blocksize, bool compress,
		Inversion method = gauss_jordan);
    
				     /**
				      * Tell the class that inverses
				      * are computed.
				      */
    void inverses_computed(bool are_they);
    
				     /**
				      * Use only the inverse of the
				      * first diagonal block to save
				      * memory and computation time.
				      *
				      * Possible applications:
				      * computing on a cartesian grid,
				      * all diagonal blocks are the
				      * same or all diagonal blocks
				      * are at least similar and
				      * inversion of one of them still
				      * yields a preconditioner.
				      */
    void set_same_diagonal ();

				     /**
				      * Does the matrix use only one
				      * diagonal block?
				      */
    bool same_diagonal () const;

				     /**
				      * Check, whether diagonal blocks
				      * (not their inverses)
				      * should be stored.
				      */
    bool store_diagonals() const;

				     /**
				      * Return true, if inverses are
				      * ready for use.
				      */
    bool inverses_ready () const;
    
    				     /**
				      * Checks whether the object is empty.
				      */
    bool empty () const;

				     /**
				      * The number of blocks.
				      */
    unsigned int size() const;
    
				     /**
				      * Read-only access to entries.
				      * This function is only possible
				      * if the inverse diagonal blocks
				      * are stored.
				      */
    number el(unsigned int i, unsigned int j) const;

				     /**
				      * Multiply with the inverse
				      * block at position <tt>i</tt>.
				      */
    template <typename number2>
    void inverse_vmult(unsigned int i, Vector<number2>& dst, const Vector<number2>& src) const;
    
				     /**
				      * Multiply with the transposed inverse
				      * block at position <tt>i</tt>.
				      */
    template <typename number2>
    void inverse_Tvmult(unsigned int i, Vector<number2>& dst, const Vector<number2>& src) const;
    
				     /**
				      * Access to the inverse diagonal
				      * blocks if Inversion is #gauss_jordan.
				      */
    FullMatrix<number>& inverse (unsigned int i);
    
				     /**
				      * Access to the inverse diagonal
				      * blocks if Inversion is #householder.
				      */
    Householder<number>& inverse_householder (unsigned int i);
    
				     /**
				      * Access to the inverse diagonal
				      * blocks if Inversion is #householder.
				      */
    LAPACKFullMatrix<number>& inverse_svd (unsigned int i);
    
				     /**
				      * Access to the inverse diagonal
				      * blocks.
				      */
    const FullMatrix<number>& inverse (unsigned int i) const;
    
				     /**
				      * Access to the inverse diagonal
				      * blocks if Inversion is #householder.
				      */
    const Householder<number>& inverse_householder (unsigned int i) const;
    
				     /**
				      * Access to the inverse diagonal
				      * blocks if Inversion is #householder.
				      */
    const LAPACKFullMatrix<number>& inverse_svd (unsigned int i) const;
    
				     /**
				      * Access to the diagonal
				      * blocks.
				      */
    FullMatrix<number>& diagonal (unsigned int i);
    
				     /**
				      * Access to the diagonal
				      * blocks.
				      */
    const FullMatrix<number>& diagonal (unsigned int i) const;

				     /**
				      * Determine an estimate for the
				      * memory consumption (in bytes)
				      * of this object.
				      */
    unsigned int memory_consumption () const;
    
				     /**
				      * You are trying to access a
				      * diagonal block (not its
				      * inverse), but you decided not
				      * to store the diagonal blocks.
				      */
    DeclException0 (ExcDiagonalsNotStored);

  protected:
				     /**
				      * The method used for inverting blocks.
				      */
    Inversion inversion;
    
  private:
				     /**
				      * The number of (inverse)
				      * diagonal blocks, if only one
				      * is stored.
				      */
    unsigned int n_diagonal_blocks;
    
 				     /**
				      * Storage of the inverse
				      * matrices of the diagonal
				      * blocks matrices as
				      * <tt>FullMatrix<number></tt>
				      * matrices, if Inversion
				      * #gauss_jordan is used. Using
				      * <tt>number=float</tt> saves
				      * memory in comparison with
				      * <tt>number=double</tt>, but
				      * may introduce numerical instability.
				      */
    std::vector<FullMatrix<number> > var_inverse_full;

 				     /**
				      * Storage of the inverse
				      * matrices of the diagonal
				      * blocks matrices as
				      * <tt>Householder</tt>
				      * matrices if Inversion
				      * #householder is used. Using
				      * <tt>number=float</tt> saves
				      * memory in comparison with
				      * <tt>number=double</tt>, but
				      * may introduce numerical instability.
				      */
    std::vector<Householder<number> > var_inverse_householder;

				     /**
				      * Storage of the inverse
				      * matrices of the diagonal
				      * blocks matrices as
				      * <tt>LAPACKFullMatrix</tt>
				      * matrices if Inversion
				      * #svd is used. Using
				      * <tt>number=float</tt> saves
				      * memory in comparison with
				      * <tt>number=double</tt>, but
				      * may introduce numerical instability.
				      */
    std::vector<LAPACKFullMatrix<number> > var_inverse_svd;
    
				     /**
				      * Storage of the original diagonal blocks.
				      *
				      * Used by the blocked SSOR method.
				      */
    std::vector<FullMatrix<number> > var_diagonal;


				     /**
				      * This is true, if the field
				      * #var_diagonal is to be used.
				      */
    bool var_store_diagonals;

				     /**
				      * This is true, if only one
				      * inverse is stored.
				      */
    bool var_same_diagonal;

				     /**
				      * The inverse matrices are
				      * usable. Set by the parent
				      * class via inverses_computed().
				      */
    bool var_inverses_ready;
};

//----------------------------------------------------------------------//

template <typename number>
inline
PreconditionBlockBase<number>::PreconditionBlockBase(
  bool store, Inversion method)
		:
		inversion(method),
		n_diagonal_blocks(0),
		var_store_diagonals(store),
		var_same_diagonal(false),
		var_inverses_ready(false)
{}


template <typename number>
inline
void
PreconditionBlockBase<number>::clear()
{
  if (var_inverse_full.size()!=0)
    var_inverse_full.erase(var_inverse_full.begin(), var_inverse_full.end());
  if (var_inverse_householder.size()!=0)
    var_inverse_householder.erase(var_inverse_householder.begin(), var_inverse_householder.end());
  if (var_inverse_svd.size()!=0)
    var_inverse_svd.erase(var_inverse_svd.begin(), var_inverse_svd.end());
  if (var_diagonal.size()!=0)
    var_diagonal.erase(var_diagonal.begin(), var_diagonal.end());
  var_same_diagonal = false;
  var_inverses_ready = false;
  n_diagonal_blocks = 0;
}

template <typename number>
inline
void
PreconditionBlockBase<number>::reinit(unsigned int n, unsigned int b, bool compress,
Inversion method)
{
  inversion = method;
  var_same_diagonal = compress;
  var_inverses_ready = false;
  n_diagonal_blocks = n;
  
  if (compress)
    {
      switch(inversion)
	{
	  case gauss_jordan:
		var_inverse_full.resize(1);
		var_inverse_full[0].reinit(b,b);
		break;
	  case householder:
		var_inverse_householder.resize(1);
		break;
	  case svd:
		var_inverse_svd.resize(1);
		var_inverse_svd[0].reinit(b,b);
		break;
	  default:
		Assert(false, ExcNotImplemented());
	}
      
      if (store_diagonals())
	{
	  var_diagonal.resize(1);
	  var_diagonal[0].reinit(b,b);
	}
    }
  else
    {
				       // set the arrays to the right
				       // size. we could do it like this:
				       // var_inverse = vector<>(nblocks,FullMatrix<>())
				       // but this would involve copying many
				       // FullMatrix objects.
				       //
				       // the following is a neat trick which
				       // avoids copying
      if (store_diagonals())
	{
	  std::vector<FullMatrix<number> >
	    tmp(n, FullMatrix<number>(b));
	  var_diagonal.swap (tmp);
	}

      switch(inversion)
	{
	  case gauss_jordan:
	  {
	    std::vector<FullMatrix<number> >
	      tmp(n, FullMatrix<number>(b));
	    var_inverse_full.swap (tmp);
	    break;
	  }
	  case householder:
		var_inverse_householder.resize(n);
		break;
	  case svd:
	  {
	    std::vector<LAPACKFullMatrix<number> >
	      tmp(n, LAPACKFullMatrix<number>(b));
	    var_inverse_svd.swap (tmp);
	    break;
	  }
	  default:	
		Assert(false, ExcNotImplemented());	
	}
    }
}


template <typename number>
inline
unsigned int
PreconditionBlockBase<number>::size() const
{
  return n_diagonal_blocks;
}

template <typename number>
template <typename number2>
inline
void
PreconditionBlockBase<number>::inverse_vmult(
  unsigned int i, Vector<number2>& dst, const Vector<number2>& src) const
{
  const unsigned int ii = same_diagonal() ? 0U : i;
  
  switch (inversion)
    {
      case gauss_jordan:
	    AssertIndexRange (ii, var_inverse_full.size());
	    var_inverse_full[ii].vmult(dst, src);
	    break;
      case householder:
	    AssertIndexRange (ii, var_inverse_householder.size());
	    var_inverse_householder[ii].vmult(dst, src);
	    break;
      case svd:
	    AssertIndexRange (ii, var_inverse_svd.size());
	    var_inverse_svd[ii].vmult(dst, src);
	    break;
      default:
	    Assert(false, ExcNotImplemented());
    }
}


template <typename number>
template <typename number2>
inline
void
PreconditionBlockBase<number>::inverse_Tvmult(
  unsigned int i, Vector<number2>& dst, const Vector<number2>& src) const
{
  const unsigned int ii = same_diagonal() ? 0U : i;
  
  switch (inversion)
    {
      case gauss_jordan:
	    AssertIndexRange (ii, var_inverse_full.size());
	    var_inverse_full[ii].Tvmult(dst, src);
	    break;
      case householder:
	    AssertIndexRange (ii, var_inverse_householder.size());
	    var_inverse_householder[ii].Tvmult(dst, src);
	    break;
      case svd:
	    AssertIndexRange (ii, var_inverse_svd.size());
	    var_inverse_svd[ii].Tvmult(dst, src);
	    break;
      default:
	    Assert(false, ExcNotImplemented());
    }
}


template <typename number>
inline
const FullMatrix<number>&
PreconditionBlockBase<number>::inverse(unsigned int i) const
{
  if (same_diagonal())
    return var_inverse_full[0];
  
  Assert (i < var_inverse_full.size(), ExcIndexRange(i,0,var_inverse_full.size()));
  return var_inverse_full[i];
}


template <typename number>
inline
const Householder<number>&
PreconditionBlockBase<number>::inverse_householder(unsigned int i) const
{
  if (same_diagonal())
    return var_inverse_householder[0];
  
  AssertIndexRange (i, var_inverse_householder.size());
  return var_inverse_householder[i];
}


template <typename number>
inline
const LAPACKFullMatrix<number>&
PreconditionBlockBase<number>::inverse_svd(unsigned int i) const
{
  if (same_diagonal())
    return var_inverse_svd[0];
  
  AssertIndexRange (i, var_inverse_svd.size());
  return var_inverse_svd[i];
}


template <typename number>
inline
const FullMatrix<number>&
PreconditionBlockBase<number>::diagonal(unsigned int i) const
{
  Assert(store_diagonals(), ExcDiagonalsNotStored());
  
  if (same_diagonal())
    return var_diagonal[0];
  
  Assert (i < var_diagonal.size(), ExcIndexRange(i,0,var_diagonal.size()));
  return var_diagonal[i];
}


template <typename number>
inline
FullMatrix<number>&
PreconditionBlockBase<number>::inverse(unsigned int i)
{
  if (same_diagonal())
    return var_inverse_full[0];
  
  Assert (i < var_inverse_full.size(), ExcIndexRange(i,0,var_inverse_full.size()));
  return var_inverse_full[i];
}


template <typename number>
inline
Householder<number>&
PreconditionBlockBase<number>::inverse_householder(unsigned int i)
{
  if (same_diagonal())
    return var_inverse_householder[0];
  
  AssertIndexRange (i, var_inverse_householder.size());
  return var_inverse_householder[i];
}


template <typename number>
inline
LAPACKFullMatrix<number>&
PreconditionBlockBase<number>::inverse_svd(unsigned int i)
{
  if (same_diagonal())
    return var_inverse_svd[0];
  
  AssertIndexRange (i, var_inverse_svd.size());
  return var_inverse_svd[i];
}


template <typename number>
inline
FullMatrix<number>&
PreconditionBlockBase<number>::diagonal(unsigned int i)
{
  Assert(store_diagonals(), ExcDiagonalsNotStored());
  
  if (same_diagonal())
    return var_diagonal[0];
  
  Assert (i < var_diagonal.size(), ExcIndexRange(i,0,var_diagonal.size()));
  return var_diagonal[i];
}


template <typename number>
inline bool
PreconditionBlockBase<number>::same_diagonal() const
{
  return var_same_diagonal;
}


template <typename number>
inline bool
PreconditionBlockBase<number>::store_diagonals() const
{
  return var_store_diagonals;
}


template <typename number>
inline void
PreconditionBlockBase<number>::inverses_computed(bool x)
{
  var_inverses_ready = x;
}


template <typename number>
inline bool
PreconditionBlockBase<number>::inverses_ready() const
{
  return var_inverses_ready;
}


template <typename number>
inline
unsigned int
PreconditionBlockBase<number>::memory_consumption () const
{
  unsigned int mem = sizeof(*this);
  for (unsigned int i=0; i<var_inverse_full.size(); ++i)
    mem += MemoryConsumption::memory_consumption(var_inverse_full[i]);
  for (unsigned int i=0; i<var_diagonal.size(); ++i)
    mem += MemoryConsumption::memory_consumption(var_diagonal[i]);
  return mem;
}


DEAL_II_NAMESPACE_CLOSE

#endif
