//--------------------------------------------------------------------
//      $Id$   
//    Version: $Name$
//
//    Copyright (C) 2000, 2001, 2002 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//--------------------------------------------------------------------


#include <base/polynomial.h>
#include <base/thread_management.h>

//TODO:[?] This class leaks memory, but only at the very end of a program.
// Since it expands the Legendre<number>::coefficients array, the elements
// of this static variable are not destroyed at the end of the program
// run. While this is not a problem (since the returned memory could
// not be used anyway then), it is a little confusing when looking at
// a memory checked such as "purify". Maybe, this should be handled somehow
// to avoid this confusion in future.

//TODO:[GK] These polynomials are orthogonal on [-1,1], but the
//integral over p^2 is not 1. Find out if this is ok.

// Reserve space for polynomials up to degree 19. Should be sufficient
// for the start.
template <typename number>
typename std::vector<const typename std::vector<number> *>
Legendre<number>::recursive_coefficients(
  20, static_cast<const typename std::vector<number>*>(0));
template <typename number>
typename std::vector<const typename std::vector<number> *>
Legendre<number>::shifted_coefficients(
  20, static_cast<const typename std::vector<number>*>(0));


// have a lock that guarantees that at most one thread is changing and
// accessing the @p{coefficients} array. make this lock local to this
// file
namespace 
{
  Threads::ThreadMutex coefficients_lock;
};



template <typename number>
void
Legendre<number>::compute_coefficients (const unsigned int k_)
{
  unsigned int k = k_;

				   // first make sure that no other
				   // thread intercepts the operation
				   // of this function
  coefficients_lock.acquire ();

				   // The first 2 coefficients are hard-coded
  if (k==0)
    k=1;
				   // check: does the information
				   // already exist?
  if ((recursive_coefficients.size() < k+1) ||
      ((recursive_coefficients.size() >= k+1) &&
       (recursive_coefficients[k] == 0)))
				     // no, then generate the
				     // respective coefficients
    {
      recursive_coefficients.resize (k+1, 0);
      
      if (k<=1)
	{
					   // create coefficients
					   // vectors for k=0 and k=1
					   //
					   // allocate the respective
					   // amount of memory and
					   // later assign it to the
					   // coefficients array to
					   // make it const
	  std::vector<number> *c0 = new std::vector<number>(1);
	  (*c0)[0] = 1.;

	  std::vector<number> *c1 = new std::vector<number>(2);
	  (*c1)[0] = 0.;
	  (*c1)[1] = 1.;

					   // now make these arrays
					   // const
	  recursive_coefficients[0] = c0;
	  recursive_coefficients[1] = c1;
					   // Compute polynomials
					   // orthogonal on [0,1]
  	  c0 = new std::vector<number>(*c0);
  	  c1 = new std::vector<number>(*c1);
	  
    	  Polynomial<number>::shift(*c0, (long double) -1.);
    	  Polynomial<number>::scale(*c0, 2.);
    	  Polynomial<number>::shift(*c1, (long double) -1.);
    	  Polynomial<number>::scale(*c1, 2.);
    	  Polynomial<number>::multiply(*c1, sqrt(3.));
  	  shifted_coefficients[0]=c0;
  	  shifted_coefficients[1]=c1;
	}
      else
	{
					   // for larger numbers,
					   // compute the coefficients
					   // recursively. to do so,
					   // we have to release the
					   // lock temporarily to
					   // allow the called
					   // function to acquire it
					   // itself
	  coefficients_lock.release ();
	  compute_coefficients(k-1);
	  coefficients_lock.acquire ();

	  std::vector<number> *ck = new std::vector<number>(k+1);
	  
	  const number a = 1./(k);
	  const number b = a*(2*k-1);
	  const number c = a*(k-1);
	  
	  (*ck)[k]   = b*(*recursive_coefficients[k-1])[k-1];
	  (*ck)[k-1] = b*(*recursive_coefficients[k-1])[k-2];
	  for (unsigned int i=1 ; i<= k-2 ; ++i)
	    (*ck)[i] = b*(*recursive_coefficients[k-1])[i-1]
		       -c*(*recursive_coefficients[k-2])[i];

	  (*ck)[0]   = -c*(*recursive_coefficients[k-2])[0];

					   // finally assign the newly
					   // created vector to the
					   // const pointer in the
					   // coefficients array
	  recursive_coefficients[k] = ck;
					   // and compute the
					   // coefficients for [0,1]
  	  ck = new std::vector<number>(*ck);
    	  shift(*ck,(long double) -1.);
    	  Polynomial<number>::scale(*ck, 2.);
    	  Polynomial<number>::multiply(*ck, sqrt(2.*k+1.));
  	  shifted_coefficients[k] = ck;
	};
    };

				   // now, everything is done, so
				   // release the lock again
  coefficients_lock.release ();
}



template <typename number>
const typename std::vector<number> &
Legendre<number>::get_coefficients (const unsigned int k)
{
				   // first make sure the coefficients
				   // get computed if so necessary
  compute_coefficients (k);

				   // then get a pointer to the array
				   // of coefficients. do that in a MT
				   // safe way
  coefficients_lock.acquire ();
  const std::vector<number> *p = shifted_coefficients[k];
  coefficients_lock.release ();

				   // return the object pointed
				   // to. since this object does not
				   // change any more once computed,
				   // this is MT safe
  return *p;
}



template <typename number>
Legendre<number>::Legendre (const unsigned int k)
		:
		Polynomial<number> (get_coefficients(k))
{}



template <typename number>
std::vector<Polynomial<number> >
Legendre<number>::generate_complete_basis (const unsigned int degree)
{
  std::vector<Polynomial<double> > v;
  v.reserve(degree+1);
  for (unsigned int i=0; i<=degree; ++i)
    v.push_back (Legendre<double>(i));
  return v;
};



// explicit instantiations
template class Legendre<double>;
