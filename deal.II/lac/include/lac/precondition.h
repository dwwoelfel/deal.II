/*-------------------- precondition.h --------------------*/
//$Id$
// Guido Kanschat, University of Heidelberg, 1999

#ifndef __lac_precondition_h
#define __lac_precondition_h

/**
 * No preconditioning.
 * This class helps you, if you want to use a linear solver without
 * preconditioning. Since this is a strange idea, the documentation
 * here stays quite short.
 *
 * @author Guido Kanschat, 1999
 */
template<class VECTOR>
class PreconditionIdentity
{
  public:
  				     /**
				      * Execute preconditioning.
				      */
    void operator() (VECTOR&, const VECTOR&) const;
};




/**
 * Preconditioner using a matrix-builtin function.
 * This class forms a preconditioner suitable for the LAC solver
 * classes. Since many preconditioning methods are based on matrix
 * entries, these have to be implemented as member functions of the
 * underlying matrix implementation. This class now is intended to
 * allow easy access to these member functions from LAC solver
 * classes.
 *
 * It seems that all builtin preconditioners have a relaxation
 * parameter, so please use #PreconditionRelaxation# for these.
 *
 * \subsection{Use}
 * You will usually not want to create a named object of this type,
 * although possible. The most common use is like this:
 * \begin{verbatim}
 *    SolverGMRES<SparseMatrix<double>,
 *                Vector<double> >      gmres(control,memory,500);
 *
 *    gmres.solve (matrix, solution, right_hand_side,
 *		   PreconditionUseMatrix<SparseMatrix<double>,Vector<double> >
 *		   (matrix,&SparseMatrix<double>::template precondition_Jacobi));
 * \end{verbatim}
 * This creates an unnamed object to be passed as the fourth parameter to
 * the solver function of the #SolverGMRES# class. It assumes that the
 * #SparseMatrix# class has a function #precondition_Jacobi# taking two
 * vectors (source and destination) as parameters. (Actually, there is no
 * function like that, the existing function takes a third parameter,
 * denoting the relaxation parameter; this example is therefore only meant to
 * illustrate the general idea.)
 *
 * @author Guido Kanschat, Wolfgang Bangerth, 1999
 */
template<class MATRIX, class VECTOR>
class PreconditionUseMatrix
{
  public:
				     /**
				      * Type of the preconditioning
				      * function of the matrix.
				      */
    typedef void ( MATRIX::* function_ptr)(VECTOR&, const VECTOR&) const;
    
				     /**
				      * Constructor.
				      * This constructor stores a
				      * reference to the matrix object
				      * for later use and selects a
				      * preconditioning method, which
				      * must be a member function of
				      * that matrix.
				      */
    PreconditionUseMatrix(const MATRIX      &M,
			  const function_ptr method);
    
				     /**
				      * Execute preconditioning. Calls the
				      * function passed to the constructor
				      * of this object with the two
				      * arguments given here.
				      */
    void operator() (VECTOR       &dst,
		     const VECTOR &src) const;

  private:
				     /**
				      * Pointer to the matrix in use.
				      */
    const MATRIX& matrix;
    
				     /**
				      * Pointer to the preconditioning
				      * function.
				      */
    const function_ptr precondition;
};



/**
 * Preconditioner for builtin relaxation methods.
 * Application of this preconditioner includes
 * use of the #precondition_...# methods of #SparseMatrix#.
 *
 * \subsection{Use}
 * You will usually not want to create a named object of this type,
 * although possible. The most common use is like this:
 * \begin{verbatim}
 *    SolverGMRES<SparseMatrix<double>,
 *                Vector<double> >      gmres(control,memory,500);
 *
 *    gmres.solve (matrix, solution, right_hand_side,
 *		   PreconditionRelaxation<SparseMatrix<double>,Vector<double> >
 *		   (matrix,&SparseMatrix<double>::template precondition_Jacobi,
 *                   0.5));
 * \end{verbatim}
 * This creates an unnamed object to be passed as the fourth parameter to
 * the solver function of the #SolverGMRES# class. It assumes that the
 * #SparseMatrix# class has a function #precondition_Jacobi# taking two
 * vectors (source and destination) and a relaxation value as parameters. (Unlike
 * for the #PreconditionUseMatrix# class, this time it should work, with
 * relaxation parameter $0.5$.)
 *
 * @author Guido Kanschat, Wolfgang Bangerth, 1999
 */
template<class MATRIX, class VECTOR>
class PreconditionRelaxation
{
  public:
				     /**
				      * Type of the preconditioning
				      * function of the matrix.
				      */
    typedef void ( MATRIX::* function_ptr)(VECTOR&, const VECTOR&,
					   typename MATRIX::value_type) const;
    
				     /**
				      * Constructor.
				      * This constructor stores a
				      * reference to the matrix object
				      * for later use and selects a
				      * preconditioning method, which
				      * must be a member function of
				      * that matrix.
				      */
    PreconditionRelaxation(const MATRIX      &M,
			   const function_ptr method,
			   const double       omega = 1.);
    
				     /**
				      * Execute preconditioning. Calls the
				      * function passed to the constructor
				      * of this object with the two
				      * arguments given here, and the
				      * relaxation parameter passed to the
				      * constructor.
				      */
    void operator() (VECTOR&, const VECTOR&) const;

  private:
				     /**
				      * Pointer to the matrix in use.
				      */
    const MATRIX& matrix;
    
				     /**
				      * Pointer to the preconditioning
				      * function.
				      */
    const function_ptr precondition;
    
				     /**
				      * Relaxation parameter.
				      */
    double omega;
};




/* ---------------------------------- Inline functions ------------------- */

template<class VECTOR>
void
PreconditionIdentity<VECTOR>::operator() (VECTOR& dst, const VECTOR& src) const
{
  dst = src;
}

template<class MATRIX, class VECTOR>
PreconditionUseMatrix<MATRIX, VECTOR>::PreconditionUseMatrix(const MATRIX& M,
							     function_ptr method)
		:
		matrix(M), precondition(method)
{}


template<class MATRIX, class VECTOR>
void
PreconditionUseMatrix<MATRIX, VECTOR>::operator() (VECTOR& dst,
						   const VECTOR& src) const
{
  (matrix.*precondition)(dst, src);
}

template<class MATRIX, class VECTOR>
PreconditionRelaxation<MATRIX, VECTOR>::PreconditionRelaxation(const MATRIX& M,
							     function_ptr method,
							       double omega)
		:
		matrix(M), precondition(method), omega(omega)
{}


template<class MATRIX, class VECTOR>
void
PreconditionRelaxation<MATRIX, VECTOR>::operator() (VECTOR& dst,
						   const VECTOR& src) const
{
  (matrix.*precondition)(dst, src, omega);
}

#endif
