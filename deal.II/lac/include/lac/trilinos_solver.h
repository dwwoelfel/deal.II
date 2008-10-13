//---------------------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 2008 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------
#ifndef __deal2__trilinos_solver_h
#define __deal2__trilinos_solver_h

#include <base/config.h>
#include <lac/exceptions.h>
#include <lac/solver_control.h>
#include <boost/shared_ptr.hpp>


#ifdef DEAL_II_USE_TRILINOS

#include <Epetra_LinearProblem.h>
#include <AztecOO.h>
#include <Epetra_Operator.h>

DEAL_II_NAMESPACE_OPEN


namespace TrilinosWrappers
{
                                   // forward declarations
  class SparseMatrix;
  class VectorBase;
  class PreconditionBase;
  
  
/**
 * Base class for solver classes using the Trilinos solvers. Since
 * solvers in Trilinos are selected based on flags passed to a generic
 * solver object, basically all the actual solver calls happen in this
 * class, and derived classes simply set the right flags to select one
 * solver or another, or to set certain parameters for individual
 * solvers. For a general discussion on the Trilinos solver package
 * AztecOO, we refer to the <a href =
 * "http://trilinos.sandia.gov/packages/aztecoo/AztecOOUserGuide.pdf">AztecOO
 * user guide</a>.
 *
 * This solver class can also be used as a standalone class, where the
 * respective Krylov method is set via the flag
 * <tt>solver_name</tt>. This can be done at runtime (e.g., when
 * parsing the solver from a ParameterList) and is similar to the
 * deal.II class SolverSelector.
 *
 * @ingroup TrilinosWrappers
 * @author Martin Kronbichler, 2008
 */
  class SolverBase
  {
    public:

				       /**
					* Enumeration object that is
					* set in the constructor of
					* the derived classes and
					* tells Trilinos which solver
					* to use. This option can also
					* be set in the user program,
					* so one might use this base
					* class instead of one of the
					* specialized derived classes
					* when the solver should be
					* set at runtime. Currently
					* enabled options are:
					*/
      enum SolverName {cg, cgs, gmres, bicgstab, tfqmr} solver_name;

                                       /**
                                        * Standardized data struct to
                                        * pipe additional data to the
                                        * solver.
                                        */
      struct AdditionalData
      {
				       /**
					* Restart parameter in case
					* the derived class is
					* GMRES.
					* 
					* TODO: Find a better way for
					* doing this.
					*/
	AdditionalData (const unsigned int gmres_restart_parameter = 30);

	const unsigned int gmres_restart_parameter;
      };

                                       /**
                                        * Constructor. Takes the
                                        * solver control object and
                                        * creates the solver.
                                        */
      SolverBase (SolverControl  &cn);

                                       /**
                                        * Second constructor. This
                                        * constructor takes an enum
                                        * object that specifies the
                                        * solver name and sets the
                                        * appropriate Krylov
                                        * method.
                                        */
      SolverBase (const enum SolverName  solver_name,
		  SolverControl         &cn);
      
                                       /**
                                        * Destructor.
                                        */
      virtual ~SolverBase ();

                                       /**
                                        * Solve the linear system
                                        * <tt>Ax=b</tt>. Depending on
                                        * the information provided by
                                        * derived classes and the
                                        * object passed as a
                                        * preconditioner, one of the
                                        * linear solvers and
                                        * preconditioners of Trilinos
                                        * is chosen.
                                        */
      void
      solve (const SparseMatrix     &A,
             VectorBase             &x,
             const VectorBase       &b,
             const PreconditionBase &preconditioner);

                                       /**
                                        * Access to object that controls
                                        * convergence.
                                        */
      SolverControl & control() const;

                                       /**
                                        * Exception
                                        */
      DeclException1 (ExcTrilinosError,
                      int,
                      << "An error with error number " << arg1
                      << " occured while calling a Trilinos function");

    protected:

                                       /**
                                        * Reference to the object that
                                        * controls convergence of the
                                        * iterative solver. In fact,
                                        * for these Trilinos wrappers,
                                        * Trilinos does so itself, but
                                        * we copy the data from this
                                        * object before starting the
                                        * solution process, and copy
                                        * the data back into it
                                        * afterwards.
                                        */
      SolverControl &solver_control;

    private:

                                       /**
					* A structure that collects
					* the Trilinos sparse matrix,
					* the right hand side vector
					* and the solution vector,
					* which is passed down to the
					* Trilinos solver.
					*/
      std::auto_ptr<Epetra_LinearProblem> linear_problem;

                                       /**
                                        * A structure that contains
                                        * the Trilinos solver and
                                        * preconditioner objects.
                                        */
      AztecOO solver;

                                       /**
                                        * Store a copy of the flags for this
                                        * particular solver.
                                        */
      const AdditionalData additional_data;

  };



/**
 * An implementation of the solver interface using the Trilinos CG
 * solver.
 *
 * @ingroup TrilinosWrappers
 * @author Martin Kronbichler, 2008
 */
  class SolverCG : public SolverBase
  {
    public:
                                       /**
                                        * Standardized data struct to
                                        * pipe additional data to the
                                        * solver.
                                        */
      struct AdditionalData
      {};
	
                                       /**
                                        * Constructor. In contrast to
                                        * deal.II's own solvers, there is no
                                        * need to give a vector memory
                                        * object.
					*
                                        * The last argument takes a structure
                                        * with additional, solver dependent
                                        * flags for tuning.
                                        */
      SolverCG (SolverControl        &cn,
                const AdditionalData &data = AdditionalData());

    protected:
                                       /**
                                        * Store a copy of the flags for this
                                        * particular solver.
                                        */
      const AdditionalData additional_data;
  };



/**
 * An implementation of the solver interface using the Trilinos CGS
 * solver.
 *
 * @ingroup TrilinosWrappers
 * @author Martin Kronbichler, 2008
 */
  class SolverCGS : public SolverBase
  {
    public:
                                       /**
                                        * Standardized data struct to
                                        * pipe additional data to the
                                        * solver.
                                        */
      struct AdditionalData
      {};
	
                                       /**
                                        * Constructor. In contrast to
                                        * deal.II's own solvers, there is no
                                        * need to give a vector memory
                                        * object.
					*
                                        * The last argument takes a structure
                                        * with additional, solver dependent
                                        * flags for tuning.
                                        */
      SolverCGS (SolverControl        &cn,
                 const AdditionalData &data = AdditionalData());

    protected:
                                       /**
                                        * Store a copy of the flags for this
                                        * particular solver.
                                        */
      const AdditionalData additional_data;
  };



/**
 * An implementation of the solver interface using the Trilinos GMRES
 * solver.
 *
 * @author Martin Kronbichler, 2008
 */
  class SolverGMRES : public SolverBase
  {
    public:
                                       /**
                                        * Standardized data struct to
                                        * pipe additional data to the
                                        * solver.
                                        */
      struct AdditionalData
      {
                                           /**
                                            * Constructor. By default, set the
                                            * number of temporary vectors to
                                            * 30, i.e. do a restart every 30
                                            * iterations.
                                            */
          AdditionalData (const unsigned int restart_parameter = 30);
	
                                           /**
                                            * Maximum number of
                                            * tmp vectors.
                                            */
          unsigned int restart_parameter;
      };
	
                                       /**
                                        * Constructor. In contrast to
                                        * deal.II's own solvers, there is no
                                        * need to give a vector memory
                                        * object.
                                        *
                                        * The last argument takes a structure
                                        * with additional, solver dependent
                                        * flags for tuning.
                                        */
      SolverGMRES (SolverControl        &cn,
                   const AdditionalData &data = AdditionalData());

    protected:
                                       /**
                                        * Store a copy of the flags for this
                                        * particular solver.
                                        */
      const AdditionalData additional_data;
  };



/**
 * An implementation of the solver interface using the Trilinos BiCGStab
 * solver.
 *
 * @ingroup TrilinosWrappers
 * @author Martin Kronbichler, 2008
 */
  class SolverBicgstab : public SolverBase
  {
    public:
                                       /**
                                        * Standardized data struct to
                                        * pipe additional data to the
                                        * solver.
                                        */
      struct AdditionalData
      {};
	
                                       /**
                                        * Constructor. In contrast to
                                        * deal.II's own solvers, there is no
                                        * need to give a vector memory
                                        * object.
                                        *
                                        * The last argument takes a structure
                                        * with additional, solver dependent
                                        * flags for tuning.
                                        */
      SolverBicgstab (SolverControl        &cn,
                      const AdditionalData &data = AdditionalData());

    protected:
                                       /**
                                        * Store a copy of the flags for this
                                        * particular solver.
                                        */
      const AdditionalData additional_data;
  };



/**
 * An implementation of the solver interface using the Trilinos TFQMR
 * solver.
 *
 * @ingroup TrilinosWrappers
 * @author Martin Kronbichler, 2008
 */
  class SolverTFQMR : public SolverBase
  {
    public:
                                       /**
                                        * Standardized data struct to
                                        * pipe additional data to the
                                        * solver.
                                        */
      struct AdditionalData
      {};
	
                                       /**
                                        * Constructor. In contrast to
                                        * deal.II's own solvers, there is no
                                        * need to give a vector memory
                                        * object.
					*
                                        * The last argument takes a structure
                                        * with additional, solver dependent
                                        * flags for tuning.
                                        */
      SolverTFQMR (SolverControl        &cn,
                   const AdditionalData &data = AdditionalData());

    protected:
                                       /**
                                        * Store a copy of the flags for this
                                        * particular solver.
                                        */
      const AdditionalData additional_data;
  };

}

DEAL_II_NAMESPACE_CLOSE

#endif // DEAL_II_USE_TRILINOS

/*----------------------------   trilinos_solver.h     ---------------------------*/

#endif
/*----------------------------   trilinos_solver.h     ---------------------------*/
