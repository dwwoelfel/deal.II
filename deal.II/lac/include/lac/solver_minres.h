//----------------------------  solver_minres.h  ---------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 1998, 1999, 2000 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  solver_minres.h  -----------------------
#ifndef __deal2__solver_minres_h
#define __deal2__solver_minres_h


#include <lac/solver.h>
#include <lac/solver_control.h>
#include <base/logstream.h>
#include <cmath>



/**
 * Preconditioned MinRes method.
 *
 * Like all other solver classes, this class has a local structure called
 * @p{AdditionalData} which is used to pass additional parameters to the
 * solver, like damping parameters or the number of temporary vectors. We
 * use this additional structure instead of passing these values directly
 * to the constructor because this makes the use of the @p{SolverSelector} and
 * other classes much easier and guarantees that these will continue to
 * work even if number or type of the additional parameters for a certain
 * solver changes.
 *
 * However, since the MinRes method does not need additional data, the respective
 * structure is empty and does not offer any functionality. The constructor
 * has a default argument, so you may call it without the additional
 * parameter.
 *
 * The preconditioner has to be positive definite and symmetric
 *
 * The algorithm is taken from the Master thesis of Astrid Batterman
 * with some changes.
 * The full text can be found at
 * @p{http://scholar.lib.vt.edu/theses/public/etd-12164379662151/etd-title.html}
 *
 * @author Thomas Richter, 2000
 */
template <class VECTOR = Vector<double> >
class SolverMinRes : private Solver<VECTOR>
{
  public:
    				     /**
				      * Standardized data struct to
				      * pipe additional data to the
				      * solver. This solver does not
				      * need additional data yet.
				      */
    struct AdditionalData
    {
    };

				     /**
				      * Constructor.
				      */
    SolverMinRes (SolverControl &cn,
		  VectorMemory<VECTOR> &mem,
		  const AdditionalData &data=AdditionalData());

				     /**
				      * Virtual destructor.
				      */
    virtual ~SolverMinRes ();
    
				     /**
				      * Solver method.
				      */
    template<class MATRIX, class PRECONDITIONER>
    typename Solver<VECTOR>::ReturnState
    solve (const MATRIX &A,
	   VECTOR       &x,
	   const VECTOR &b,
	   const PRECONDITIONER& precondition);

				     /**
				      * Exception
				      */
    DeclException0 (ExcPreconditionerNotDefinite);
    

  protected:
				     /**
				      * Implementation of the computation of
				      * the norm of the residual.
				      */
    virtual double criterion();
				     /**
				      * Interface for derived class.
				      * This function gets the current
				      * iteration vector, the residual
				      * and the update vector in each
				      * step. It can be used for a
				      * graphical output of the
				      * convergence history.
				      */
    virtual void print_vectors(const unsigned int step,
			       const VECTOR& x,
			       const VECTOR& r,
			       const VECTOR& d) const;

				     /**
				      * Temporary vectors, allocated through
				      * the @p{VectorMemory} object at the start
				      * of the actual solution process and
				      * deallocated at the end.
				      */
    VECTOR *Vu0, *Vu1, *Vu2;
    VECTOR *Vm0, *Vm1, *Vm2;   
    VECTOR *Vv;
    
				     /**
				      * Within the iteration loop, the
				      * square of the residual vector is
				      * stored in this variable. The
				      * function @p{criterion} uses this
				      * variable to compute the convergence
				      * value, which in this class is the
				      * norm of the residual vector and thus
				      * the square root of the @p{res2} value.
				      */
    double res2;
};


/*------------------------- Implementation ----------------------------*/


template<class VECTOR>
SolverMinRes<VECTOR>::SolverMinRes (SolverControl &cn,
				    VectorMemory<VECTOR> &mem,
				    const AdditionalData &)
		:
		Solver<VECTOR>(cn,mem)
{}


template<class VECTOR>
SolverMinRes<VECTOR>::~SolverMinRes ()
{}



template<class VECTOR>
double
SolverMinRes<VECTOR>::criterion()
{
  return res2;
};


template<class VECTOR>
void
SolverMinRes<VECTOR>::print_vectors(const unsigned int,
				    const VECTOR&,
				    const VECTOR&,
				    const VECTOR&) const
{}



template<class VECTOR>
template<class MATRIX, class PRECONDITIONER>
typename Solver<VECTOR>::ReturnState 
SolverMinRes<VECTOR>::solve (const MATRIX &A,
			     VECTOR       &x,
			     const VECTOR &b,
			     const PRECONDITIONER& precondition)
{
  SolverControl::State conv=SolverControl::iterate;

  deallog.push("minres");


  unsigned int VS = b.size();

  
				   // Memory allocation
  Vu0  = memory.alloc();
  Vu1  = memory.alloc();
  Vu2  = memory.alloc();
  Vv   = memory.alloc();
  Vm0  = memory.alloc();
  Vm1  = memory.alloc();
  Vm2  = memory.alloc();
				   // define some aliases for simpler access
  typedef VECTOR vecref;
  vecref u[3] = {*Vu0, *Vu1, *Vu2};
  vecref m[3] = {*Vm0, *Vm1, *Vm2};
  vecref v    = *Vv;
				   // resize the vectors, but do not set
				   // the values since they'd be overwritten
				   // soon anyway.
  u[0].reinit(VS,true);
  u[1].reinit(VS,true);
  u[2].reinit(VS,true);
  m[0].reinit(VS,true);
  m[1].reinit(VS,true);
  m[2].reinit(VS,true);
  v.reinit(VS,true);

				   // some values needed
  double delta[3];
  double f[2];
  double e[2]; 

  double r_l2 = 0;
  double r0   = 0;
  double tau = 0;
  double c    = 0;
  double gamma = 0;
  double s = 0;
  double d_ = 0;
  double d = 0;  

				   // The iteration step.
  unsigned int j = 1;
  

				   // Start of the solution process
  A.vmult(m[0],x);
  u[1] = b;
  u[1] -= m[0];
				   // Precondition is applied.
				   // The preconditioner has to be
				   // positiv definite and symmetric

				   // M v = u[1]
  precondition.vmult (v,u[1]);
  
  delta[1] = v * u[1];
				   // Preconditioner positive
  Assert (delta[1]>=0, ExcPreconditionerNotDefinite());
  
  r0 = sqrt(delta[1]);
  r_l2 = r0;
  
  
  u[0].reinit(VS,0);
  delta[0] = 1.;
  m[0].reinit(VS,0);
  m[1].reinit(VS,0);
  m[2].reinit(VS,0);
				   
  conv = control().check(0,r_l2);
  
  while (conv==SolverControl::iterate)
    {
      
      if (delta[1]!=0)
	v.scale(1./sqrt(delta[1]));
      else
	v.reinit(VS,0);

      A.vmult(u[2],v);
      u[2].add (-sqrt(delta[1]/delta[0]), u[0]);

      gamma = u[2] * v;
      u[2].add (-gamma / sqrt(delta[1]), u[1]);
      m[0] = v;
      
				       // precondition: solve M v = u[2]
				       // Preconditioner has to be positiv
				       // definite and symmetric.
      precondition.vmult(v,u[2]);
 
      delta[2] = v * u[2];

      Assert (delta[2]>=0, ExcPreconditionerNotDefinite());

      if (j==1)
	{
	  d_ = gamma;
	  e[1] = sqrt(delta[2]);
	}
      if (j>1)
	{
	  d_ = s * e[0] - c * gamma;
	  e[0] = c * e[0] + s * gamma;
	  f[1] = s * sqrt(delta[2]);
	  e[1] = -c * sqrt(delta[2]);
	}

      d = sqrt (d_*d_ + delta[2]);
      
      if (j>1) tau *= s / c;
      c = d_ / d;
      tau *= c;
      
      s = sqrt(delta[2]) / d;

      if (j==1)
	tau = r0 * c;

      m[0].add (-e[0], m[1]);
      if (j>1)
	m[0].add (-f[0],m[2]);
      m[0].scale(1./d);
      x.add (tau, m[0]);
      r_l2 *= fabs(s);

      conv = control().check(j,r_l2);
      
				       // next iteration step
      ++j;
				       // All vectors have to be shifted
				       // one iteration step.
				       // This should be changed one time.
				       //
				       // the previous code was like this:
				       //   m[2] = m[1];
				       //   m[1] = m[0];
				       // but it can be made more efficient,
				       // since the value of m[0] is no more
				       // needed in the next iteration
      swap (m[2], m[1]);
      swap (m[1], m[0]);
      
				       // likewise, but reverse direction:
				       //   u[0] = u[1];
				       //   u[1] = u[2];
      swap (u[0], u[1]);
      swap (u[1], u[2]);

				       // these are scalars, so need
				       // to bother
      f[0] = f[1];
      e[0] = e[1];
      delta[0] = delta[1];
      delta[1] = delta[2];
    }

				   // Deallocation of Memory
  memory.free(Vu0);
  memory.free(Vu1);
  memory.free(Vu2);
  memory.free(Vv); 
  memory.free(Vm0);
  memory.free(Vm1);
  memory.free(Vm2);
				   // Output
  deallog.pop ();
  
  if (conv == SolverControl::failure)
    return exceeded;
  
  return success;
};


#endif

