/* $Id$ */
/* Author: David Neckels, Boulder, Colorado, 2007, 2008 */

/*    $Id$       */
/*    Version: $Name$                                          */
/*                                                                */
/*    Copyright (C) 2007, 2008 by the deal.II authors and David Neckels */
/*                                                                */
/*    This file is subject to QPL and may not be  distributed     */
/*    without copyright and license information. Please refer     */
/*    to the file deal.II/doc/license.html for the  text  and     */
/*    further information on this license.                        */

				 // @sect3{Include files}

                                 // First a standard set of deal.II
                                 // includes. Nothing special to comment on
                                 // here:
#include <base/quadrature_lib.h>
#include <base/function.h>
#include <base/parameter_handler.h>
#include <base/function_parser.h>
#include <base/utilities.h>

#include <lac/vector.h>
#include <lac/compressed_sparsity_pattern.h>

#include <grid/tria.h>
#include <grid/grid_generator.h>
#include <grid/grid_out.h>
#include <grid/grid_refinement.h>
#include <grid/tria_accessor.h>
#include <grid/tria_iterator.h>
#include <grid/grid_in.h>

#include <dofs/dof_handler.h>
#include <dofs/dof_accessor.h>
#include <dofs/dof_tools.h>

#include <fe/fe_values.h>
#include <fe/fe_system.h>
#include <fe/mapping_q1.h>
#include <fe/fe_q.h>

#include <numerics/data_out.h>
#include <numerics/vectors.h>
#include <numerics/solution_transfer.h>

                                 // Then, as mentioned in the introduction, we
                                 // use various Trilinos packages as linear
                                 // solvers as well as for automatic
                                 // differentiation. These are in the
                                 // following include files.
                                 //
                                 // In particular, Epetra is the basic
                                 // trilinos vector/matrix library and comes
                                 // with several header files pertaining to
                                 // individual aspects of it that will become
                                 // clear later on:
#include <Epetra_SerialComm.h>
#include <Epetra_Map.h>
#include <Epetra_CrsGraph.h>
#include <Epetra_CrsMatrix.h>
#include <Epetra_Vector.h>
                                 // Next, Teuchos is a Trilinos utility
                                 // library that is used to set parameters
                                 // within the Aztec solver library:
#include <Teuchos_ParameterList.hpp>

                                 // Aztec itself is the iterative solver
                                 // library:
#include <AztecOO.h>
#include <AztecOO_Operator.h>

                                 // Amesos is a direct solver package within
                                 // Trilinos:
#include <Amesos.h>

                                 // Finally, Sacado is the automatic
                                 // differentiation package, which is used to
                                 // find the Jacobian for a fully implicit
                                 // Newton iteration:
#include <Sacado.hpp>


				 // And this again is C++:
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>

				 // To end this section, introduce everythin
				 // in the dealii library into the current
				 // namespace:
using namespace dealii;


				 // @sect3{Euler equation specifics}

				 // Here we define the flux function for this
				 // particular system of conservation laws,
				 // the Euler equations for gas dynamics. We
				 // group all this into a structure that
				 // defines everything that has to do with the
				 // flux. All members of this structures are
				 // static, i.e. the structure has no actual
				 // state specified by instance member
				 // variables. The better way to do this,
				 // rather than a structure with all static
				 // members would be to use a namespace -- but
				 // namespaces can't be templatized and we
				 // want some of the member variables of the
				 // structure to depend on the space
				 // dimension, which we in our usual way
				 // introduce using a template parameter:
template <int dim>
struct EulerEquations
{
				     // First a few variables that
				     // describe the various components of our
				     // solution vector in a generic way. This
				     // includes the number of components in the
				     // system (Euler's equations have one entry
				     // for momenta in each spatial direction,
				     // plus the energy and density components,
				     // for a total of <code>dim+2</code>
				     // components), as well as functions that
				     // describe the index within the solution
				     // vector of the first momentum component,
				     // the density component, and the energy
				     // density component. Note that all these
				     // %numbers depend on the space dimension;
				     // defining them in a generic way (rather
				     // than by implicit convention) makes our
				     // code more flexible and makes it easier
				     // to later extend it, for example by
				     // adding more components to the equations.
    static const unsigned int n_components             = dim + 2;
    static const unsigned int first_momentum_component = 0;
    static const unsigned int density_component        = dim;
    static const unsigned int energy_component         = dim+1;

				     // When generating graphical
				     // output way down in this
				     // program, we need to specify
				     // the names of the solution
				     // variables as well as how the
				     // various components group into
				     // vector and scalar fields. We
				     // could describe this there, but
				     // in order to keep things that
				     // have to do with the Euler
				     // equation localized here and
				     // the rest of the program as
				     // generic as possible, we
				     // provide this sort of
				     // information in the following
				     // two functions:
    static
    std::vector<std::string>
    component_names ()
      {
	std::vector<std::string> names (dim, "momentum");
	names.push_back ("density");
	names.push_back ("energy_density");

	return names;
      }


    static
    std::vector<DataComponentInterpretation::DataComponentInterpretation>
    component_interpretation ()
      {
	std::vector<DataComponentInterpretation::DataComponentInterpretation>
	  data_component_interpretation
	  (dim, DataComponentInterpretation::component_is_part_of_vector);
	data_component_interpretation
	  .push_back (DataComponentInterpretation::component_is_scalar);
	data_component_interpretation
	  .push_back (DataComponentInterpretation::component_is_scalar);
	
	return data_component_interpretation;
      }
    
    
				     // Next, we define the gas
				     // constant. We will set it to 1.4
				     // in its definition immediately
				     // following the declaration of
				     // this class (unlike integer
				     // variables, like the ones above,
				     // static const floating point
				     // member variables cannot be
				     // initialized within the class
				     // declaration in C++). This value
				     // of 1.4 is representative of a
				     // gas that consists of molecules
				     // composed of two atoms, such as
				     // air which consists up to small
				     // traces almost entirely of $N_2$
				     // and $O_2$.
    static const double gas_gamma;

    
				     // In the following, we will need to
				     // compute the kinetic energy and the
				     // pressure from a vector of conserved
				     // variables. This we can do based on the
				     // energy density and the kinetic energy
				     // $\frac 12 \rho |\mathbf v|^2 =
				     // \frac{|\rho \mathbf v|^2}{2\rho}$
				     // (note that the independent variables
				     // contain the momentum components $\rho
				     // v_i$, not the velocities $v_i$).
				     //
				     // There is one slight problem: We will
				     // need to call the following functions
				     // with input arguments of type
				     // <code>std::vector@<number@></code> and
				     // <code>Vector@<number@></code>. The
				     // problem is that the former has an
				     // access operator
				     // <code>operator[]</code> whereas the
				     // latter, for historical reasons, has
				     // <code>operator()</code>. We wouldn't
				     // be able to write the function in a
				     // generic way if we were to use one or
				     // the other of these. Fortunately, we
				     // can use the following trick: instead
				     // of writing <code>v[i]</code> or
				     // <code>v(i)</code>, we can use
				     // <code>*(v.begin() + i)</code>, i.e. we
				     // generate an iterator that points to
				     // the <code>i</code>th element, and then
				     // dereference it. This works for both
				     // kinds of vectors -- not the prettiest
				     // solution, but one that works.
    template <typename number, typename InputVector>
    static
    number
    compute_kinetic_energy (const InputVector &W)
      {
	number kinetic_energy = 0;
	for (unsigned int d=0; d<dim; ++d)
	  kinetic_energy += *(W.begin()+first_momentum_component+d) *
			    *(W.begin()+first_momentum_component+d);
	kinetic_energy *= 1./(2 * *(W.begin() + density_component));

	return kinetic_energy;
      }


    template <typename number, typename InputVector>
    static
    number
    compute_pressure (const InputVector &W)
      {
	return ((gas_gamma-1.0) *
		(*(W.begin() + energy_component) -
		 compute_kinetic_energy<number>(W)));
      }	


				     // We define the flux function
				     // $F(W)$ as one large matrix.
				     // Each row of this matrix
				     // represents a scalar
				     // conservation law for the
				     // component in that row.  The
				     // exact form of this matrix is
				     // given in the
				     // introduction. Note that we
				     // know the size of the matrix:
				     // it has as many rows as the
				     // system has components, and
				     // <code>dim</code> columns;
				     // rather than using a FullMatrix
				     // object for such a matrix
				     // (which has a variable number
				     // of rows and columns and must
				     // therefore allocate memory on
				     // the heap each time such a
				     // matrix is created), we use a
				     // rectangular array of numbers
				     // right away.
				     //
				     // We templatize the numerical
				     // type of the flux function so
				     // that we may use the automatic
				     // differentiation type here.
				     // The flux functions are defined
				     // in terms of the conserved
				     // variables $\rho w_0, \dots,
				     // \rho w_{d-1}, \rho, E$, so
				     // they do not look exactly like
				     // the Euler equations one is
				     // used to seeing.
    template <typename number>
    static
    void flux_matrix (const std::vector<number> &W,
		      number (&flux)[n_components][dim])
      {
					 // First compute the pressure that
					 // appears in the flux matrix, and
					 // then compute the first
					 // <code>dim</code> columns of the
					 // matrix that correspond to the
					 // momentum terms:
	const number pressure = compute_pressure<number> (W);
	
	for (unsigned int d=0; d<dim; ++d)
	  {
	    for (unsigned int e=0; e<dim; ++e)
	      flux[first_momentum_component+d][e]
		= W[first_momentum_component+d] *
		W[first_momentum_component+e] /
		W[density_component];
	  
	    flux[first_momentum_component+d][d] += pressure;
	  }
	
					 // Then the terms for the
					 // density (i.e. mass
					 // conservation), and,
					 // lastly, conservation of
					 // energy:
	for (unsigned int d=0; d<dim; ++d)
	  flux[density_component][d] = W[first_momentum_component+d]; 

	for (unsigned int d=0; d<dim; ++d)
	  flux[energy_component][d] = W[first_momentum_component+d] /
				      W[density_component] *
				      (W[energy_component] + pressure);
      }


				     // On the boundaries of the
				     // domain and across hanging
				     // nodes we use a numerical flux
				     // function to enforce boundary
				     // conditions.  This routine is
				     // the basic Lax-Friedrich's flux
				     // with a stabilization parameter
				     // $\alpha$. It's form has also
				     // been given already in the
				     // introduction:
    template <typename number>
    static
    void numerical_normal_flux(const Point<dim>          &normal,
			       const std::vector<number> &Wplus,
			       const std::vector<number> &Wminus,
			       const double alpha,
			       Sacado::Fad::DFad<double> (&normal_flux)[n_components])
      {
	Sacado::Fad::DFad<double> iflux[n_components][dim];
	Sacado::Fad::DFad<double> oflux[n_components][dim];
	  
	flux_matrix(Wplus, iflux);
	flux_matrix(Wminus, oflux);
	  
	for (unsigned int di=0; di<n_components; ++di)
	  {
	    normal_flux[di] = 0;
	    for (unsigned int d=0; d<dim; ++d) 
	      normal_flux[di] += 0.5*(iflux[di][d] + oflux[di][d]) * normal(d);
	      
	    normal_flux[di] += 0.5*alpha*(Wplus[di] - Wminus[di]);
	  }
      }

    
				     // Finally, we declare a class that
				     // implements a postprocessing of data
				     // components. The problem this class
				     // solves is that the variables in the
				     // formulation of the Euler equations we
				     // use are in conservative rather than
				     // physical form: they are momentum
				     // densities $\mathbf m=\rho\mathbf v$,
				     // density $\rho$, and energy density
				     // $E$. What we would like to also put
				     // into our output file are velocities
				     // $\mathbf v=\frac{\mathbf m}{\rho}$ and
				     // pressure $p=(\gamma-1)(E-\frac{1}{2}
				     // \rho |\mathbf v|^2)$.
				     //
				     // In addition, we would like to add the
				     // possibility to generate schlieren
				     // plots. Schlieren plots are a way to
				     // visualize shocks and other sharp
				     // interfaces. The word "schlieren" a
				     // German word that may be translated as
				     // "striae" -- it may be simpler to
				     // explain it by an example, however:
				     // schlieren is what you see when you,
				     // for example, pour highly concentrated
				     // alcohol, or a transparent saline
				     // solution into water; the two have the
				     // same color, but they have different
				     // refractive indices and so before they
				     // are fully mixed light goes through the
				     // mixture along bent rays that lead to
				     // brightness variations if you look at
				     // it. That's "schlieren". A similar
				     // effect happens in compressible flow
				     // due because the refractive index
				     // depends on the pressure (and therefore
				     // the density) of the gas.
				     //
				     // The origin of the word refers to
				     // two-dimensional projections of a
				     // three-dimensional volume (we see a 2d
				     // picture of the 3d fluid). In
				     // computational fluid dynamics, we can
				     // get an idea of this effect by
				     // considering what causes it: density
				     // variations. Schlieren plots are
				     // therefore produced by plotting
				     // $s=|\nabla \rho|^2$; obviously, $s$ is
				     // large in shocks and at other highly
				     // dynamic places. If so desired by the
				     // user (by specifying this in the input
				     // file), we would like to generate these
				     // schlieren plots in addition to the
				     // other derived quantities listed above.
				     //
				     // The implementation of the algorithms
				     // to compute derived quantities from the
				     // ones that solve our problem, and to
				     // output them into data file, rests on
				     // the DataPostprocessor class. It has
				     // extensive documentation, and other
				     // uses of the class can also be found in
				     // step-29. We therefore refrain from
				     // extensive comments.
    class Postprocessor : public DataPostprocessor<dim>
    {
      public:
	Postprocessor (const bool do_schlieren_plot);
	
	virtual
	void
	compute_derived_quantities_vector (const std::vector<Vector<double> >              &uh,
					   const std::vector<std::vector<Tensor<1,dim> > > &duh,
					   const std::vector<std::vector<Tensor<2,dim> > > &dduh,
					   const std::vector<Point<dim> >                  &normals,
					   std::vector<Vector<double> >                    &computed_quantities) const;

	virtual std::vector<std::string> get_names () const;
    
	virtual
	std::vector<DataComponentInterpretation::DataComponentInterpretation>
	get_data_component_interpretation () const;
    
	virtual UpdateFlags get_needed_update_flags () const;

	virtual unsigned int n_output_variables() const;

      private:
	const bool do_schlieren_plot;
    };
};
    

template <int dim>
const double EulerEquations<dim>::gas_gamma = 1.4;



template <int dim>
EulerEquations<dim>::Postprocessor::
Postprocessor (const bool do_schlieren_plot)
		:
		do_schlieren_plot (do_schlieren_plot)
{}


				 // This is the only function worth commenting
				 // on. When generating graphical output, the
				 // DataOut and related classes will call this
				 // function on each cell, with values,
				 // gradients, hessians, and normal vectors
				 // (in case we're working on faces) at each
				 // quadrature point. Note that the data at
				 // each quadrature point is itself
				 // vector-valued, namely the conserved
				 // variables. What we're going to do here is
				 // to compute the quantities we're interested
				 // in at each quadrature point. Note that for
				 // this we can ignore the hessians ("dduh")
				 // and normal vectors; to avoid compiler
				 // warnings about unused variables, we
				 // comment out their names.
template <int dim>
void
EulerEquations<dim>::Postprocessor::
compute_derived_quantities_vector (const std::vector<Vector<double> >              &uh,
				   const std::vector<std::vector<Tensor<1,dim> > > &duh,
				   const std::vector<std::vector<Tensor<2,dim> > > &/*dduh*/,
				   const std::vector<Point<dim> >                  &/*normals*/,
				   std::vector<Vector<double> >                    &computed_quantities) const
{
				   // At the beginning of the function, let us
				   // make sure that all variables have the
				   // correct sizes, so that we can access
				   // individual vector elements without
				   // having to wonder whether we might read
				   // or write invalid elements; we also check
				   // that the <code>duh</code> vector only
				   // contains data if we really need it (the
				   // system knows about this because we say
				   // so in the
				   // <code>get_needed_update_flags()</code>
				   // function below). For the inner vectors,
				   // we check that at least the first element
				   // of the outer vector has the correct
				   // inner size:
  const unsigned int n_quadrature_points = uh.size();

  if (do_schlieren_plot == true)
    Assert (duh.size() == n_quadrature_points,
	    ExcInternalError())
			else
			  Assert (duh.size() == 0,
				  ExcInternalError());

  Assert (computed_quantities.size() == n_quadrature_points,
	  ExcInternalError());

  Assert (uh[0].size() == n_components,
	  ExcInternalError());

  if (do_schlieren_plot == true)
    Assert (computed_quantities[0].size() == dim+2,
	    ExcInternalError())
					   else
					     Assert (computed_quantities[0].size() == dim+1,
						     ExcInternalError());

				   // Then loop over all quadrature points and
				   // do our work there. The code should be
				   // pretty self-explanatory. The order of
				   // output variables is first
				   // <code>dim</code> velocities, then the
				   // pressure, and if so desired the
				   // schlieren plot. Note that we try to be
				   // generic about the order of variables in
				   // the input vector, using the
				   // <code>first_momentum_component</code>
				   // and <code>density_component</code>
				   // information:
  for (unsigned int q=0; q<n_quadrature_points; ++q)
    {
      const double density = uh[q](density_component);
      const double energy  = uh[q](energy_component);

      for (unsigned int d=0; d<dim; ++d)
	computed_quantities[q](d)
	  = uh[q](first_momentum_component+d) / density;

      computed_quantities[q](dim) = compute_pressure<double> (uh[q]);

      if (do_schlieren_plot == true)
	computed_quantities[q](dim+1) = duh[q][density_component] *
					duh[q][density_component];
    }
}


template <int dim>
std::vector<std::string>
EulerEquations<dim>::Postprocessor::
get_names () const
{
  std::vector<std::string> names;
  for (unsigned int d=0; d<dim; ++d)
    names.push_back ("velocity");
  names.push_back ("pressure");

  if (do_schlieren_plot == true)
    names.push_back ("schlieren_plot");

  return names;
}


template <int dim>
std::vector<DataComponentInterpretation::DataComponentInterpretation>
EulerEquations<dim>::Postprocessor::
get_data_component_interpretation () const
{
  std::vector<DataComponentInterpretation::DataComponentInterpretation>
    interpretation (dim,
		    DataComponentInterpretation::component_is_part_of_vector);

  interpretation.push_back (DataComponentInterpretation::
			    component_is_scalar);

  if (do_schlieren_plot == true)
    interpretation.push_back (DataComponentInterpretation::
			      component_is_scalar);

  return interpretation;
}



template <int dim>
UpdateFlags
EulerEquations<dim>::Postprocessor::
get_needed_update_flags () const
{
  if (do_schlieren_plot == true)  
    return update_values | update_gradients;
  else
    return update_values;
}



template <int dim>
unsigned int
EulerEquations<dim>::Postprocessor::
n_output_variables () const
{
  if (do_schlieren_plot == true)
    return dim+2;
  else
    return dim+1;
}


				 // @sect3{Run time parameter handling}

				 // Our next job is to define a few
				 // classes that will contain run-time
				 // parameters (for example solver
				 // tolerances, number of iterations,
				 // stabilization parameter, and the
				 // like). One could do this in the
				 // main class, but we separate it
				 // from that one to make the program
				 // more modular and easier to read:
				 // Everything that has to do with
				 // run-time parameters will be in the
				 // following namespace, whereas the
				 // program logic is in the main
				 // class.
				 //
				 // We will split the run-time
				 // parameters into a few separate
				 // structures, which we will all put
				 // into a namespace
				 // <code>Parameters</code>. Of these
				 // classes, there are a few that
				 // group the parameters for
				 // individual groups, such as for
				 // solvers, mesh refinement, or
				 // output. Each of these classes have
				 // functions
				 // <code>declare_parameters()</code>
				 // and
				 // <code>parse_parameters()</code>
				 // that declare parameter subsections
				 // and entries in a ParameterHandler
				 // object, and retrieve actual
				 // parameter values from such an
				 // object, respectively. These
				 // classes declare all their
				 // parameters in subsections of the
				 // ParameterHandler.
				 //
				 // The final class of the following
				 // namespace combines all the
				 // previous classes by deriving from
				 // them and taking care of a few more
				 // entries at the top level of the
				 // input file, as well as a few odd
				 // other entries in subsections that
				 // are too short to warrent a
				 // structure by themselves.
				 //
				 // It is worth pointing out one thing here:
				 // None of the classes below have a
				 // constructor that would initialize the
				 // various member variables. This isn't a
				 // problem, however, since we will read all
				 // variables declared in these classes from
				 // the input file (or indirectly: a
				 // ParameterHandler object will read it from
				 // there, and we will get the values from
				 // this object), and they will be initialized
				 // this way. In case a certain variable is
				 // not specified at all in the input file,
				 // this isn't a problem either: The
				 // ParameterHandler class will in this case
				 // simply take the default value that was
				 // specified when declaring an entry in the
				 // <code>declare_parameters()</code>
				 // functions of the classes below.
namespace Parameters
{

				   // @sect4{Parameters::Solver}
				   //
				   // The first of these classes deals
				   // with parameters for the linear
				   // inner solver. It offers
				   // parameters that indicate which
				   // solver to use (GMRES as a solver
				   // for general non-symmetric
				   // indefinite systems, or a sparse
				   // direct solver), the amount of
				   // output to be produced, as well
				   // as various parameters that tweak
				   // the thresholded incomplete LU
				   // decomposition (ILUT) that we use
				   // as a preconditioner for GMRES.
				   //
				   // In particular, the ILUT takes
				   // the following parameters:
				   // - ilut_fill: the number of extra
				   //   entries to add when forming the ILU
				   //   decomposition
				   // - ilut_atol, ilut_rtol: When
				   //   forming the preconditioner, for
				   //   certain problems bad conditioning
				   //   (or just bad luck) can cause the
				   //   preconditioner to be very poorly
				   //   conditioned.  Hence it can help to
				   //   add diagonal perturbations to the
				   //   original matrix and form the
				   //   preconditioner for this slightly
				   //   better matrix.  ATOL is an absolute
				   //   perturbation that is added to the
				   //   diagonal before forming the prec,
				   //   and RTOL is a scaling factor $rtol
				   //   >= 1$.
				   // - ilut_drop: The ILUT will
				   //   drop any values that
				   //   have magnitude less than this value.
				   //   This is a way to manage the amount
				   //   of memory used by this
				   //   preconditioner.
				   //
				   // The meaning of each parameter is
				   // also briefly described in the
				   // third argument of the
				   // ParameterHandler::declare_entry
				   // call in
				   // <code>declare_parameters()</code>.
  struct Solver 
  {
      enum SolverType { gmres, direct };
      SolverType solver;
      
      enum  OutputType { quiet, verbose };
      OutputType output;

      double linear_residual;
      int max_iterations;

      double ilut_fill;
      double ilut_atol;
      double ilut_rtol;
      double ilut_drop;

      static void declare_parameters (ParameterHandler &prm);
      void parse_parameters (ParameterHandler &prm);
  };



  void Solver::declare_parameters (ParameterHandler &prm)
  {
    prm.enter_subsection("linear solver");
    {
      prm.declare_entry("output", "quiet",
			Patterns::Selection("quiet|verbose"),
			"State whether output from solver runs should be printed. "
			"Choices are <quiet|verbose>.");
      prm.declare_entry("method", "gmres",
			Patterns::Selection("gmres|direct"),
			"The kind of solver for the linear system. "
			"Choices are <gmres|direct>.");
      prm.declare_entry("residual", "1e-10",
			Patterns::Double(),
			"Linear solver residual");
      prm.declare_entry("max iters", "300",
			Patterns::Integer(),
			"Maximum solver iterations");
      prm.declare_entry("ilut fill", "2",
			Patterns::Double(),
			"Ilut preconditioner fill");
      prm.declare_entry("ilut absolute tolerance", "1e-9",
			Patterns::Double(),
			"Ilut preconditioner tolerance");
      prm.declare_entry("ilut relative tolerance", "1.1",
			Patterns::Double(),
			"Ilut relative tolerance");
      prm.declare_entry("ilut drop tolerance", "1e-10",
			Patterns::Double(),
			"Ilut drop tolerance");
    }
    prm.leave_subsection();
  }
  
    

  
  void Solver::parse_parameters (ParameterHandler &prm)
  {
    prm.enter_subsection("linear solver");
    {
      const std::string op = prm.get("output");
      if (op == "verbose")
	output = verbose;
      if (op == "quiet")
	output = quiet;
    
      const std::string sv = prm.get("method");
      if (sv == "direct") 
	solver = direct;
      else if (sv == "gmres")
	solver = gmres;

      linear_residual = prm.get_double("residual");
      max_iterations  = prm.get_integer("max iters");
      ilut_fill       = prm.get_double("ilut fill");
      ilut_atol       = prm.get_double("ilut absolute tolerance");
      ilut_rtol       = prm.get_double("ilut relative tolerance");
      ilut_drop       = prm.get_double("ilut drop tolerance");
    }
    prm.leave_subsection();  
  }
  

  
				   // @sect4{Parameters::Refinement}
				   //
				   // Similarly, here are a few parameters
				   // that determine how the mesh is to be
				   // refined (and if it is to be refined at
				   // all). For what exactly the shock
				   // parameters do, see the mesh refinement
				   // functions further down.
  struct Refinement
  {
      bool do_refine;
      double shock_val;
      double shock_levels;

      static void declare_parameters (ParameterHandler &prm);
      void parse_parameters (ParameterHandler &prm);
  };



  void Refinement::declare_parameters (ParameterHandler &prm)
  {

    prm.enter_subsection("refinement");
    {
      prm.declare_entry("refinement", "true",
			Patterns::Bool(),
			"Whether to perform mesh refinement or not");
      prm.declare_entry("refinement fraction", "0.1",
			Patterns::Double(),
			"Fraction of high refinement");
      prm.declare_entry("unrefinement fraction", "0.1",
			Patterns::Double(),
			"Fraction of low unrefinement");
      prm.declare_entry("max elements", "1000000",
			Patterns::Double(),
			"maximum number of elements");
      prm.declare_entry("shock value", "4.0",
			Patterns::Double(),
			"value for shock indicator");
      prm.declare_entry("shock levels", "3.0",
			Patterns::Double(),
			"number of shock refinement levels");
    }
    prm.leave_subsection();
  }
  

  void Refinement::parse_parameters (ParameterHandler &prm)
  {
    prm.enter_subsection("refinement");
    {
      do_refine     = prm.get_bool ("refinement");
      shock_val     = prm.get_double("shock value");
      shock_levels  = prm.get_double("shock levels");
    }
    prm.leave_subsection();
  }
  


				   // @sect4{Parameters::Flux}
				   //
				   // Next a section on flux modifications to
				   // make it more stable. In particular, two
				   // options are offered to stabilize the
				   // Lax-Friedrichs flux: either choose
				   // $\mathbf{H}(\mathbf{a},\mathbf{b},\mathbf{n})
				   // =
				   // \frac{1}{2}(\mathbf{F}(\mathbf{a})\cdot
				   // \mathbf{n} + \mathbf{F}(\mathbf{b})\cdot
				   // \mathbf{n} + \alpha (\mathbf{a} -
				   // \mathbf{b}))$ where $\alpha$ is either a
				   // fixed number specified in the input
				   // file, or where $\alpha$ is a mesh
				   // dependent value. In the latter case, it
				   // is chosen as $\frac{h}{2\delta T}$ with
				   // $h$ the diameter of the face to which
				   // the flux is applied, and $\delta T$ 
				   // the current time step.
  struct Flux
  {
      enum StabilizationKind { constant, mesh_dependent };
      StabilizationKind stabilization_kind;
      
      double stabilization_value;

      static void declare_parameters (ParameterHandler &prm);
      void parse_parameters (ParameterHandler &prm);
  };


  void Flux::declare_parameters (ParameterHandler &prm)
  {
    prm.enter_subsection("flux");
    {
      prm.declare_entry("stab", "mesh",
			Patterns::Selection("constant|mesh"),
			"Whether to use a constant stabilization parameter or "
			"a mesh-dependent one");
      prm.declare_entry("stab value", "1",
			Patterns::Double(),
			"alpha stabilization");
    }
    prm.leave_subsection();  
  }
  
  
  void Flux::parse_parameters (ParameterHandler &prm)
  {
    prm.enter_subsection("flux");
    {
      const std::string stab = prm.get("stab");
      if (stab == "constant") 
	stabilization_kind = constant;
      else if (stab == "mesh") 
	stabilization_kind = mesh_dependent;
      else
	AssertThrow (false, ExcNotImplemented());
  
      stabilization_value = prm.get_double("stab value");
    }
    prm.leave_subsection();
  }



				   // @sect4{Parameters::Output}
				   //
				   // Then a section on output parameters. We
				   // offer to produce Schlieren plots (the
				   // squared gradient of the density, a tool
				   // to visualize shock fronts), and a time
				   // interval between graphical output in
				   // case we don't want an output file every
				   // time step.
  struct Output
  {
      bool schlieren_plot;
      double output_step;

      static void declare_parameters (ParameterHandler &prm);
      void parse_parameters (ParameterHandler &prm);
  };



  void Output::declare_parameters (ParameterHandler &prm)
  {
    prm.enter_subsection("output");
    {  
      prm.declare_entry("schlieren plot", "true",
			Patterns::Bool (),
			"Whether or not to produce schlieren plots");
      prm.declare_entry("step", "-1",
			Patterns::Double(),
			"Output once per this period");
    }
    prm.leave_subsection();
  }
  


  void Output::parse_parameters (ParameterHandler &prm)
  {
    prm.enter_subsection("output");
    {
      schlieren_plot = prm.get_bool("schlieren plot");
      output_step = prm.get_double("step");
    }
    prm.leave_subsection();
  }



				   // @sect4{Parameters::AllParameters}
				   //
				   // Finally the class that brings it all
				   // together. It declares a number of
				   // parameters itself, mostly ones at the
				   // top level of the parameter file as well
				   // as several in section too small to
				   // warrant their own classes. It also
				   // contains everything that is actually
				   // space dimension dependent, like initial
				   // or boundary conditions.
				   //
				   // Since this class is derived from all the
				   // ones above, the
				   // <code>declare_parameters()</code> and
				   // <code>parse_parameters()</code>
				   // functions call the respective functions
				   // of the base classes as well.
				   //
				   // Note that this class also handles the
				   // declaration of initial and boundary
				   // conditions specified in the input
				   // file. To this end, in both cases, there
				   // are entries like "w_0 value" which
				   // represent an expression in terms of
				   // $x,y,z$ that describe the initial or
				   // boundary condition as a formula that
				   // will later be parsed by the
				   // FunctionParser class. Similar
				   // expressions exist for "w_1", "w_2", etc,
				   // denoting the <code>dim+2</code>
				   // conserved variables of the Euler
				   // system. Similarly, we allow up to
				   // <code>max_n_boundaries</code> boundary
				   // indicators to be used in the input file,
				   // and each of these boundary indicators
				   // can be associated with an inflow,
				   // outflow, or pressure boundary condition,
				   // with inhomogenous boundary conditions
				   // being specified for each component and
				   // each boundary indicator separately.
				   //
				   // The data structure used to store
				   // the boundary indicators is a bit
				   // complicated. It is an array of
				   // <code>max_n_boundaries</code>
				   // elements indicating the range of
				   // boundary indicators that will be
				   // accepted. For each entry in this
				   // array, we store a pair of data
				   // in the
				   // <code>BoundaryCondition</code>
				   // structure: first, an array of
				   // size <code>n_components</code>
				   // that for each component of the
				   // solution vector indicates
				   // whether it is an inflow,
				   // outflow, or other kind of
				   // boundary, and second a
				   // FunctionParser object that
				   // describes all components of the
				   // solution vector for this
				   // boundary id at once.
				   //
				   // The
				   // <code>BoundaryCondition</code>
				   // structure requires a constructor
				   // since we need to tell the
				   // function parser object at
				   // construction time how many
				   // vector components it is to
				   // describe. This initialization
				   // can therefore not wait till we
				   // actually set the formulas the
				   // FunctionParser object represents
				   // later in
				   // <code>AllParameters::parse_parameters()</code>
				   //
				   // For the same reason of having to
				   // tell Function objects their
				   // vector size at construction
				   // time, we have to have a
				   // constructor of the
				   // <code>AllParameters</code> class
				   // that at least initializes the
				   // other FunctionParser object,
				   // i.e. the one describing initial
				   // conditions.
  template <int dim>
  struct AllParameters : public Solver,
			 public Refinement,
			 public Flux,
			 public Output
  {
      static const unsigned int max_n_boundaries = 10;

      enum BoundaryKind
      {
	    inflow_boundary,
	    outflow_boundary,
	    no_penetration_boundary,
	    pressure_boundary
      };

      struct BoundaryConditions
      {
	  BoundaryKind        kind[EulerEquations<dim>::n_components];
	  FunctionParser<dim> values;

	  BoundaryConditions ();
      };
      
      
      AllParameters ();
      
      double diffusion_power;
      double gravity;

      double time_step, final_time;
      double theta;
      bool is_stationary;

      std::string mesh_filename;

      FunctionParser<dim> initial_conditions;
      BoundaryConditions  boundary_conditions[max_n_boundaries];
      
      static void declare_parameters (ParameterHandler &prm);
      void parse_parameters (ParameterHandler &prm);
  };



  template <int dim>
  AllParameters<dim>::BoundaryConditions::BoundaryConditions ()
		  :
		  values (EulerEquations<dim>::n_components)
  {}


  template <int dim>
  AllParameters<dim>::AllParameters ()
		  :
		  initial_conditions (EulerEquations<dim>::n_components)
  {}
  

  template <int dim>
  void
  AllParameters<dim>::declare_parameters (ParameterHandler &prm)
  {
    prm.declare_entry("mesh", "grid.inp",
		      Patterns::Anything(),
		      "intput file name");

    prm.declare_entry("diffusion power", "2.0",
		      Patterns::Double(),
		      "power of mesh size for diffusion");

    prm.declare_entry("gravity", "0.0",
		      Patterns::Double(),
		      "gravity forcing");

    prm.enter_subsection("time stepping");
    {
      prm.declare_entry("time step", "0.1",
			Patterns::Double(0),
			"simulation time step");
      prm.declare_entry("final time", "10.0",
			Patterns::Double(0),
			"simulation end time");
      prm.declare_entry("theta scheme value", "0.5",
			Patterns::Double(0,1),
			"value for theta that interpolated between explicit "
			"Euler (theta=0), Crank-Nicolson (theta=0.5), and "
			"implicit Euler (theta=1).");
    }
    prm.leave_subsection();


    for (unsigned int b=0; b<max_n_boundaries; ++b)
      {
	prm.enter_subsection("boundary_" +
			     Utilities::int_to_string(b));
	{
	  prm.declare_entry("no penetration", "false",
			    Patterns::Bool(),
			    "whether the named boundary allows gas to "
			    "penetrate or is a rigid wall");

	  for (unsigned int di=0; di<EulerEquations<dim>::n_components; ++di)
	    {
	      prm.declare_entry("w_" + Utilities::int_to_string(di),
				"outflow",
				Patterns::Selection("inflow|outflow|pressure"),
				"<inflow|outflow|pressure>");
      
	      prm.declare_entry("w_" + Utilities::int_to_string(di) +
				" value", "0.0",
				Patterns::Anything(),
				"expression in x,y,z");
	    }
	}
	prm.leave_subsection();
      }

    prm.enter_subsection("initial condition");
    {
      for (unsigned int di=0; di<EulerEquations<dim>::n_components; ++di)
	prm.declare_entry("w_" + Utilities::int_to_string(di) + " value",
			  "0.0",
			  Patterns::Anything(),
			  "expression in x,y,z");
    }
    prm.leave_subsection();

    Parameters::Solver::declare_parameters (prm);
    Parameters::Refinement::declare_parameters (prm);
    Parameters::Flux::declare_parameters (prm);
    Parameters::Output::declare_parameters (prm);
  }


  template <int dim>
  void
  AllParameters<dim>::parse_parameters (ParameterHandler &prm)
  {
    mesh_filename = prm.get("mesh");
    diffusion_power = prm.get_double("diffusion power");
    gravity = prm.get_double("gravity");

    prm.enter_subsection("time stepping");
    {
      time_step = prm.get_double("time step");
      if (time_step == 0)
	{
	  is_stationary = true;
	  time_step = 1.0;
	  final_time = 1.0;
	}
      else
	is_stationary = false;
      
      final_time = prm.get_double("final time");
      theta = prm.get_double("theta scheme value");
    }
    prm.leave_subsection();

    for (unsigned int boundary_id=0; boundary_id<max_n_boundaries;
	 ++boundary_id)
      {
	prm.enter_subsection("boundary_" +
			     Utilities::int_to_string(boundary_id));
	{
	  std::vector<std::string>
	    expressions(EulerEquations<dim>::n_components, "0.0");
    
	  const bool no_penetration = prm.get_bool("no penetration");

	  for (unsigned int di=0; di<EulerEquations<dim>::n_components; ++di)
	    {
	      const std::string boundary_type
		= prm.get("w_" + Utilities::int_to_string(di));

	      if ((di < dim) && (no_penetration == true))
		boundary_conditions[boundary_id].kind[di] = no_penetration_boundary;
	      else if (boundary_type == "inflow")
		boundary_conditions[boundary_id].kind[di] = inflow_boundary;
	      else if (boundary_type == "pressure")
		boundary_conditions[boundary_id].kind[di] = pressure_boundary;
	      else if (boundary_type == "outflow")
		boundary_conditions[boundary_id].kind[di] = outflow_boundary;
	      else
		AssertThrow (false, ExcNotImplemented());

	      expressions[di] = prm.get("w_" + Utilities::int_to_string(di) +
					" value");	      
	    }

	  boundary_conditions[boundary_id].values
	    .initialize (FunctionParser<dim>::default_variable_names(),
			 expressions,
			 std::map<std::string, double>());
	}
	prm.leave_subsection();
      }

    prm.enter_subsection("initial condition");
    {
      std::vector<std::string> expressions (EulerEquations<dim>::n_components,
					    "0.0");
      for (unsigned int di = 0; di < EulerEquations<dim>::n_components; di++)
	expressions[di] = prm.get("w_" + Utilities::int_to_string(di) +
				  " value");
      initial_conditions.initialize (FunctionParser<dim>::default_variable_names(),
				     expressions,
				     std::map<std::string, double>());
    }
    prm.leave_subsection();

    Parameters::Solver::parse_parameters (prm);
    Parameters::Refinement::parse_parameters (prm);
    Parameters::Flux::parse_parameters (prm);
    Parameters::Output::parse_parameters (prm);
  }  
}

  
      

				 // @sect3{Conservation Law class}

				 // Here finally comes the class that
				 // actually does something with all
				 // the Euler equation and parameter
				 // specifics we've defined above. The
				 // public interface is pretty much
				 // the same as always (the
				 // constructor now takes the name of
				 // a file from which to read
				 // parameters, which is passed on the
				 // command line). The private
				 // function interface is also pretty
				 // similar to the usual arrangement,
				 // with the
				 // <code>assemble_system</code>
				 // function split into three parts:
				 // one that contains the main loop
				 // over all cells and that then calls
				 // the other two for integrals over
				 // cells and faces, respectively.
template <int dim>
class ConservationLaw
{
  public:
    ConservationLaw (const char *input_filename);
    void run ();
    
  private:
    void setup_system ();

    void assemble_system ();
    void assemble_cell_term (const FEValues<dim>             &fe_v,
			     const std::vector<unsigned int> &dofs);
    void assemble_face_term(const unsigned int           face_no,
			    const FEFaceValuesBase<dim> &fe_v,
			    const FEFaceValuesBase<dim> &fe_v_neighbor,
			    const std::vector<unsigned int>   &dofs,
			    const std::vector<unsigned int>   &dofs_neighbor,
			    const bool                   external_face,
			    const unsigned int           boundary_id,
			    const double                 face_diameter);

    std::pair<unsigned int, double> solve (Vector<double> &solution);

    void compute_refinement_indicators (Vector<double> &indicator) const;
    void refine_grid (const Vector<double> &indicator);

    void output_results () const;



				     // The first few member variables
				     // are also rather standard. Note
				     // that we define a mapping
				     // object to be used throughout
				     // the program when assembling
				     // terms (we will hand it to
				     // every FEValues and
				     // FEFaceValues object); the
				     // mapping we use is just the
				     // standard $Q_1$ mapping --
				     // nothing fancy, in other words
				     // -- but declaring one here and
				     // using it throughout the
				     // program will make it simpler
				     // later on to change it if that
				     // should become necessary. This
				     // is, in fact, rather pertinent:
				     // it is known that for
				     // transsonic simulations with
				     // the Euler equations,
				     // computations do not converge
				     // even as $h\rightarrow 0$ if
				     // the boundary approximation is
				     // not of sufficiently high
				     // order.
    Triangulation<dim>   triangulation;
    const MappingQ1<dim> mapping;    
    
    const FESystem<dim>  fe;
    DoFHandler<dim>      dof_handler;

    const QGauss<dim>    quadrature;
    const QGauss<dim-1>  face_quadrature;
    
                                     // Next come a number of data
                                     // vectors that correspond to the
                                     // solution of the previous time
                                     // step
                                     // (<code>old_solution</code>),
                                     // the best guess of the current
                                     // solution
                                     // (<code>current_solution</code>;
                                     // we say <i>guess</i> because
                                     // the Newton iteration to
                                     // compute it may not have
                                     // converged yet, whereas
                                     // <code>old_solution</code>
                                     // refers to the fully converged
                                     // final result of the previous
                                     // time step), and a predictor
                                     // for the solution at the next
                                     // time step, computed by
                                     // extrapolating the current and
                                     // previous solution one time
                                     // step into the future:
    Vector<double>       old_solution;
    Vector<double>       current_solution;
    Vector<double>       predictor;

    Vector<double>       right_hand_side;

				     // This final set of member
				     // variables (except for the
				     // object holding all run-time
				     // parameters at the very bottom)
				     // deals with the interface we
				     // have in this program to the
				     // Trilinos library that provides
				     // us with linear solvers.
				     //
				     // Trilinos is designed to be a
				     // library that also runs in
				     // parallel on distributed memory
				     // systems, so matrices and
				     // vectors need two things: (i) a
				     // communicator object that
				     // facilitates sending messages
				     // to remote machines, and (ii) a
				     // description which elements of
				     // a vector or matrix reside
				     // locally on a machine and which
				     // are stored remotely.
				     //
				     // We do not actually run the
				     // current program in parallel,
				     // and so the objects we use here
				     // are pretty much dummy objects
				     // for this purpose: the
				     // communicator below represents
				     // a system that includes only a
				     // single machine, and the index
				     // map encodes that all elements
				     // are stored
				     // locally. Nevertheless, we need
				     // them.
				     //
				     // Furthermore, we need a matrix
				     // object for the system matrix
				     // to be used in each Newton
				     // step. Note that map and matrix
				     // need to be updated for their
				     // sizes whenever we refine the
				     // mesh. In Trilinos, this is
				     // easiest done by simply
				     // deleting the previous object
				     // and creating a new one. To
				     // minimize hassle and avoid
				     // memory leaks, we use a
				     // <code>std::auto_ptr</code>
				     // instead of a plain pointer for
				     // this.
    Epetra_SerialComm               communicator;
    std::auto_ptr<Epetra_Map>       Map;
    std::auto_ptr<Epetra_CrsMatrix> Matrix;
 
    Parameters::AllParameters<dim> parameters;
};


				 // @sect4{ConservationLaw::ConservationLaw}
				 //
				 // There is nothing much to say about
				 // the constructor. Essentially, it
				 // reads the input file and fills the
				 // parameter object with the parsed
				 // values:
template <int dim>
ConservationLaw<dim>::ConservationLaw (const char *input_filename)
		:
		mapping (),
                fe (FE_Q<dim>(1), EulerEquations<dim>::n_components),
		dof_handler (triangulation),
		quadrature (2),
		face_quadrature (2)
{
  ParameterHandler prm;
  Parameters::AllParameters<dim>::declare_parameters (prm);

  prm.read_input (input_filename);
  parameters.parse_parameters (prm);
}



				 // @sect4{ConservationLaw::setup_system}
				 //
				 // The following function is called
				 // each time the mesh is
				 // changed. Essentially what it does
				 // is to resize the Trilinos
				 // matrix. In addition to just
				 // resizing it, it also builds a
				 // sparsity pattern, initializes the
				 // row lengths of the matrix with the
				 // ones from this sparsity pattern,
				 // and finally puts zero entries into
				 // the places where nonzero entries
				 // will later be found. This will
				 // make subsequent operations on the
				 // matrix faster, because no new
				 // memory will need to be allocated:
template <int dim>
void ConservationLaw<dim>::setup_system ()
{
  Map.reset (new Epetra_Map(dof_handler.n_dofs(), 0, communicator));


				   // Now create a sparsity pattern,
				   // condense it, and count the
				   // number of nonzero entries per
				   // row:
  CompressedSparsityPattern sparsity_pattern (dof_handler.n_dofs(),
					      dof_handler.n_dofs());
  DoFTools::make_sparsity_pattern (dof_handler, sparsity_pattern);

  ConstraintMatrix hanging_node_constraints;
  DoFTools::make_hanging_node_constraints (dof_handler,
					   hanging_node_constraints);
  hanging_node_constraints.close ();
  
  hanging_node_constraints.condense (sparsity_pattern);
  sparsity_pattern.compress();
  
  std::vector<int> row_lengths (dof_handler.n_dofs());
  for (unsigned int i=0; i<dof_handler.n_dofs(); ++i)
    row_lengths[i] = sparsity_pattern.row_length (i);

				   // Next we build the matrix, using
				   // the constructor that optimizes
				   // with the existing lengths per
				   // row variable. After this, loop
				   // over the individual rows of the
				   // deal.II sparsity pattern and
				   // create entries in the Trilinos
				   // matrix in the corresponding
				   // places. At the end, call the
				   // <code>FillComplete()</code>
				   // function that indicates that no
				   // other matrix entries will be
				   // needed:
  Matrix.reset (new Epetra_CrsMatrix(Copy, *Map, &row_lengths[0], true));

  const unsigned int max_nonzero_entries
    = *std::max_element (row_lengths.begin(), row_lengths.end());
  
  std::vector<double> values(max_nonzero_entries, 0);
  std::vector<int> row_indices(max_nonzero_entries);
  
  for (unsigned int row=0; row<dof_handler.n_dofs(); ++row)
    {
      row_indices.resize (row_lengths[row], 0);
      values.resize (row_lengths[row], 0.);
      
      for (int i=0; i<row_lengths[row]; ++i)
	row_indices[i] = sparsity_pattern.column_number (row, i);
      
      Matrix->InsertGlobalValues(row, row_lengths[row],
				 &values[0], &row_indices[0]);
    }

  Matrix->FillComplete();
}


				 // @sect4{ConservationLaw::assemble_system}
				 //
                                 // This and the following two
                                 // functions are the meat of this
                                 // program: They assemble the linear
                                 // system that results from applying
                                 // Newton's method to the nonlinear
                                 // system of conservation
                                 // equations.
				 //
				 // This first function puts all of
                                 // the assembly pieces together in a
                                 // routine that dispatches the
                                 // correct piece for each cell/face.
                                 // The actual implementation of the
                                 // assembly on these objects is done
                                 // in the following functions.
template <int dim>
void ConservationLaw<dim>::assemble_system ()
{
  const unsigned int dofs_per_cell = dof_handler.get_fe().dofs_per_cell;

				   // We track the dofs on this cell and (if necessary)
				   // the adjacent cell.
  std::vector<unsigned int> dofs (dofs_per_cell);
  std::vector<unsigned int> dofs_neighbor (dofs_per_cell);

				   // First we create the
				   // ``UpdateFlags'' for the
				   // ``FEValues'' and the
				   // ``FEFaceValues'' objects.
  const UpdateFlags update_flags = update_values
				   | update_gradients
				   | update_q_points
				   | update_JxW_values,

				   // Note, that on faces we do not
				   // need gradients but we need
				   // normal vectors.
	       face_update_flags = update_values
				   | update_q_points
				   | update_JxW_values
				   | update_normal_vectors,
  
				   // On the neighboring cell we only
				   // need the shape values. Given a
				   // specific face, the quadrature
				   // points and `JxW values' are the
				   // same as for the current cells,
				   // the normal vectors are known to
				   // be the negative of the normal
				   // vectors of the current cell.
      neighbor_face_update_flags = update_values;
   
				   // Then we create the ``FEValues''
				   // object. Note, that since version
				   // 3.2.0 of deal.II the constructor
				   // of this class takes a
				   // ``Mapping'' object as first
				   // argument. Although the
				   // constructor without ``Mapping''
				   // argument is still supported it
				   // is recommended to use the new
				   // constructor. This reduces the
				   // effect of `hidden magic' (the
				   // old constructor implicitely
				   // assumes a ``MappingQ1'' mapping)
				   // and makes it easier to change
				   // the mapping object later.
  FEValues<dim> fe_v (mapping, fe, quadrature, update_flags);
  
				   // Similarly we create the
				   // ``FEFaceValues'' and
				   // ``FESubfaceValues'' objects for
				   // both, the current and the
				   // neighboring cell. Within the
				   // following nested loop over all
				   // cells and all faces of the cell
				   // they will be reinited to the
				   // current cell and the face (and
				   // subface) number.
  FEFaceValues<dim>    fe_v_face             (mapping, fe, face_quadrature,
					      face_update_flags);
  FESubfaceValues<dim> fe_v_subface          (mapping, fe, face_quadrature,
					      face_update_flags);
  FEFaceValues<dim>    fe_v_face_neighbor    (mapping, fe, face_quadrature,
					      neighbor_face_update_flags);
  FESubfaceValues<dim> fe_v_subface_neighbor (mapping, fe, face_quadrature,
					      neighbor_face_update_flags);

				   // Furthermore we need some cell
				   // iterators.
  typename DoFHandler<dim>::active_cell_iterator
    cell = dof_handler.begin_active(),
    endc = dof_handler.end();

				   // Now we start the loop over all
				   // active cells.
  for (; cell!=endc; ++cell) 
    {
      
				       // Now we reinit the ``FEValues''
				       // object for the current cell
      fe_v.reinit (cell);

                                       // Collect the local dofs and
                                       // asssemble the cell term.
      cell->get_dof_indices (dofs);

      assemble_cell_term(fe_v, dofs);

                                       // We use the DG style loop through faces
                                       // to determine if we need to apply a
                                       // 'hanging node' flux calculation or a boundary
                                       // computation.
      for (unsigned int face_no=0; face_no<GeometryInfo<dim>::faces_per_cell; ++face_no)
	{
	  if (cell->at_boundary(face_no))
	    {
					       // We reinit the
					       // ``FEFaceValues''
					       // object to the
					       // current face
	      fe_v_face.reinit (cell, face_no);

					       // and assemble the
					       // corresponding face
					       // terms.  We send the same
                                               // fe_v and dofs as described
                                               // in the assembly routine.
	      assemble_face_term(face_no, fe_v_face,
				 fe_v_face,
				 dofs,
				 dofs,
				 true,
				 cell->face(face_no)->boundary_indicator(),
				 cell->face(face_no)->diameter());
	    }
	  else
	    {
					       // Now we are not on
					       // the boundary of the
					       // domain, therefore
					       // there must exist a
					       // neighboring cell.
	      typename DoFHandler<dim>::cell_iterator neighbor=
		cell->neighbor(face_no);;

	      if (cell->face(face_no)->has_children())
		{
						   // case I: This cell refined compared to neighbor

		  const unsigned int neighbor2=
		    cell->neighbor_of_neighbor(face_no);
		  
		  
						   // We loop over
						   // subfaces
		  for (unsigned int subface_no=0;
		       subface_no<GeometryInfo<dim>::subfaces_per_face;
		       ++subface_no)
		    {
		      typename DoFHandler<dim>::active_cell_iterator
                        neighbor_child
                        = cell->neighbor_child_on_subface (face_no, subface_no);

		      Assert (neighbor_child->face(neighbor2) == cell->face(face_no)->child(subface_no),
			      ExcInternalError());
		      Assert (!neighbor_child->has_children(), ExcInternalError());

		      fe_v_subface.reinit (cell, face_no, subface_no);
		      fe_v_face_neighbor.reinit (neighbor_child, neighbor2);
		      neighbor_child->get_dof_indices (dofs_neighbor);

						       // Assemble as if we are working with
						       // a DG element.
		      assemble_face_term(face_no, fe_v_subface,
					 fe_v_face_neighbor,
					 dofs,
					 dofs_neighbor,
					 false,
					 numbers::invalid_unsigned_int,
					 neighbor_child->diameter());		      
		    }
						   // End of ``if
						   // (face->has_children())''
		}
	      else
		{
						   // We have no children, but 
						   // the neighbor cell may be refine
						   // compared to use
		  neighbor->get_dof_indices (dofs_neighbor);
		  if (neighbor->level() != cell->level()) 
		    {
						       // case II: This is refined compared to neighbor
		      Assert(neighbor->level() < cell->level(), ExcInternalError());
		      const std::pair<unsigned int, unsigned int> faceno_subfaceno=
			cell->neighbor_of_coarser_neighbor(face_no);
		      const unsigned int neighbor_face_no=faceno_subfaceno.first,
				      neighbor_subface_no=faceno_subfaceno.second;

		      Assert (neighbor->neighbor_child_on_subface (neighbor_face_no,
                                                                   neighbor_subface_no)
                              == cell,
                              ExcInternalError());

						       // Reinit the
						       // appropriate
						       // ``FEFaceValues''
						       // and assemble
						       // the face
						       // terms.
		      fe_v_face.reinit (cell, face_no);
		      fe_v_subface_neighbor.reinit (neighbor, neighbor_face_no,
						    neighbor_subface_no);
		      
		      assemble_face_term(face_no, fe_v_face,
					 fe_v_subface_neighbor,
					 dofs,
					 dofs_neighbor,
					 false,
					 numbers::invalid_unsigned_int,
					 cell->face(face_no)->diameter());
		    }

		} 
					       // End of ``face not at boundary'':
	    }
					   // End of loop over all faces:
	} 
      
				       // End iteration through cells.
    } 

				   // Notify Epetra that the matrix is done.
  Matrix->FillComplete();
}


				 // @sect4{ConservationLaw::assemble_cell_term}
				 //
                                 // Assembles the cell term, adding minus the residual
                                 // to the right hand side, and adding in the Jacobian
                                 // contributions.
template <int dim>
void ConservationLaw<dim>::assemble_cell_term (const FEValues<dim>             &fe_v,
					       const std::vector<unsigned int> &dofs) 
{
  unsigned int dofs_per_cell = fe_v.dofs_per_cell;
  unsigned int n_q_points = fe_v.n_quadrature_points;

				   // We will define the dofs on this cell in these fad variables.
  std::vector<Sacado::Fad::DFad<double> > DOF(dofs_per_cell);

				   // Values of the conservative variables at the quadrature points.
  std::vector<std::vector<Sacado::Fad::DFad<double> > > W (n_q_points,
							   std::vector<Sacado::Fad::DFad<double> >(EulerEquations<dim>::n_components));

				   // Values at the last time step of the conservative variables.
				   // Note that these do not use fad variables, since they do
				   // not depend on the 'variables to be sought'=DOFS.
  std::vector<std::vector<double > > Wl (n_q_points,
					 std::vector<double >(EulerEquations<dim>::n_components));

				   // Here we will hold the averaged values of the conservative
				   // variables that we will linearize around (cn=Crank Nicholson).
  std::vector<std::vector<Sacado::Fad::DFad<double> > > Wcn (n_q_points,
							     std::vector<Sacado::Fad::DFad<double> >(EulerEquations<dim>::n_components));

				   // Gradients of the current variables.  It is a
				   // bit of a shame that we have to compute these; we almost don't.
				   // The nice thing about a simple conservation law is that the
				   // the flux doesn't generally involve any gradients.  We do
				   // need these, however, for the diffusion stabilization. 
  std::vector<std::vector<std::vector<Sacado::Fad::DFad<double> > > > Wgrads (n_q_points,
									      std::vector<std::vector<Sacado::Fad::DFad<double> > >(EulerEquations<dim>::n_components,
																    std::vector<Sacado::Fad::DFad<double> >(dim)));


				   // Here is the magical point where we declare a subset
				   // of the fad variables as degrees of freedom.  All 
				   // calculations that reference these variables (either
				   // directly or indirectly) will accumulate sensitivies
				   // with respect to these dofs.
  for (unsigned int in = 0; in < dofs_per_cell; in++) {
    DOF[in] = current_solution(dofs[in]);
    DOF[in].diff(in, dofs_per_cell);
  }

				   // Here we compute the shape function values and gradients
				   // at the quadrature points.  Ideally, we could call into 
				   // something like get_function_values, get_function_grads,
				   // but since we don't want to make the entire old_solution vector
				   // fad types, only the local cell variables, we explicitly
				   // code this loop;
  for (unsigned int q = 0; q < n_q_points; q++) {
    for (unsigned int di = 0; di < EulerEquations<dim>::n_components; di++) {
      W[q][di] = 0;
      Wl[q][di] = 0;
      Wcn[q][di] = 0;
      for (unsigned int d = 0; d < dim; d++) {
        Wgrads[q][di][d] = 0;
      }
    }
    for (unsigned int sf = 0; sf < dofs_per_cell; sf++) {
      int di = fe_v.get_fe().system_to_component_index(sf).first;
      W[q][di] +=
	DOF[sf]*fe_v.shape_value_component(sf, q, di);
      Wl[q][di] +=
	old_solution(dofs[sf])*fe_v.shape_value_component(sf, q, di);
      Wcn[q][di] +=
	(parameters.theta*DOF[sf]+(1-parameters.theta)*old_solution(dofs[sf]))*fe_v.shape_value_component(sf, q, di);

      for (unsigned int d = 0; d < dim; d++) {
	Wgrads[q][di][d] += DOF[sf]*
			    fe_v.shape_grad_component(sf, q, di)[d];
      } // for d

    }

  } // for q

                                   // Gather the flux values for all components at
                                   // all of the quadrature points.  This also
                                   // computes the matrix of sensitivities.  Perhaps
                                   // this could be done in a better way, since this
                                   // could be a rather large object, but for now it 
                                   // seems to work just fine.
  typedef Sacado::Fad::DFad<double> FluxMatrix[EulerEquations<dim>::n_components][dim];
  FluxMatrix *flux = new FluxMatrix[n_q_points];
  
  for (unsigned int q=0; q < n_q_points; ++q)
    EulerEquations<dim>::flux_matrix(Wcn[q], flux[q]);
  

				   // We now have all of the function values/grads/fluxes,
				   // so perform the assembly.  We have an outer loop
				   // through the components of the system, and an
				   // inner loop over the quadrature points, where we
				   // accumulate contributions to the ith residual.
				   //
				   // We initialy sum all contributions of the residual
				   // in the positive sense, so that we don't need to
				   // negative the Jacobian entries.  Then, when we sum
				   // into the <code> right_hand_side </code> vector,
				   // we negate this residual.
  for (unsigned int i=0; i<fe_v.dofs_per_cell; ++i) 
    {
				       // Find which component this dof contributes to.
      const unsigned int
	component_i = fe_v.get_fe().system_to_component_index(i).first;

				       // The residual for each row (i) will be accumulating 
				       // into this fad variable.  At the end of the assembly
				       // for this row, we will query for the sensitivities
				       // to this variable and add them into the Jacobian.
      Sacado::Fad::DFad<double> F_i;

      for (unsigned int point=0; point<fe_v.n_quadrature_points; ++point)
	{
					   // Integrate the flux times gradient of the test function
	  for (unsigned int d=0; d<dim; d++) 
	    F_i -= flux[point][component_i][d] *
		   fe_v.shape_grad_component(i, point, component_i)[d] *
		   fe_v.JxW(point);

					   // The mass term (if the simulation is non-stationary).
	  if (parameters.is_stationary == false)
	    F_i += 1.0 / parameters.time_step *
		   (W[point][component_i] - Wl[point][component_i]) *
		   fe_v.shape_value_component(i, point, component_i) *
		   fe_v.JxW(point);
	  
					   // Stabilization (cell wise diffusion)
	  for (unsigned int d = 0; d < dim; d++)
	    F_i += 1.0*std::pow(fe_v.get_cell()->diameter(), parameters.diffusion_power) *
		   fe_v.shape_grad_component(i, point, component_i)[d] *
		   Wgrads[point][component_i][d] *
		   fe_v.JxW(point);
          
					   // The gravity component only enters into the energy 
					   // equation and into the vertical component of the 
					   // velocity.
	  if (component_i == dim - 1)
	    F_i += parameters.gravity *
		   Wcn[point][EulerEquations<dim>::density_component] *
		   fe_v.shape_value_component(i,point, component_i) *
		   fe_v.JxW(point);
	  else if (component_i == EulerEquations<dim>::energy_component)
	    F_i += parameters.gravity *
		   Wcn[point][EulerEquations<dim>::density_component] *
		   Wcn[point][dim-1] *
		   fe_v.shape_value_component(i,point, component_i) *
		   fe_v.JxW(point);
	}

				       // Here we gain access to the array of sensitivities
				       // of the residual.  We then sum these into the
				       // Epetra matrix.
      double *values = &(F_i.fastAccessDx(0));
      Matrix->SumIntoGlobalValues(dofs[i],
				  dofs_per_cell,
				  values,
				  reinterpret_cast<int*>(const_cast<unsigned int*>(&dofs[0])));
      right_hand_side(dofs[i]) -= F_i.val();
    }

  delete[] flux;
}


				 // @sect4{ConservationLaw::assemble_face_term}
				 //
				 // These are either
				 // boundary terms or terms across differing 
				 // levels of refinement.  In the first case,
				 // fe_v==fe_v_neighbor and dofs==dofs_neighbor.
				 // The int boundary < 0 if not at a boundary,
				 // otherwise it is the boundary indicator.
template <int dim>
void
ConservationLaw<dim>::assemble_face_term(const unsigned int           face_no,
					 const FEFaceValuesBase<dim> &fe_v,
					 const FEFaceValuesBase<dim> &fe_v_neighbor,
					 const std::vector<unsigned int>   &dofs,
					 const std::vector<unsigned int>   &dofs_neighbor,
					 const bool                   external_face,
					 const unsigned int           boundary_id,
					 const double                 face_diameter) 
{
  Sacado::Fad::DFad<double> F_i;
  const unsigned int n_q_points = fe_v.n_quadrature_points;
  const unsigned int dofs_per_cell = fe_v.get_fe().dofs_per_cell;
  const unsigned int ndofs_per_cell = fe_v_neighbor.get_fe().dofs_per_cell;
  Assert(dofs_per_cell == ndofs_per_cell,
	 ExcDimensionMismatch(dofs_per_cell, ndofs_per_cell));

				   // As above, the fad degrees of freedom
  std::vector<Sacado::Fad::DFad<double> > DOF(dofs_per_cell+ndofs_per_cell);

				   // The conservative variables for this cell,
				   // and for 
  std::vector<std::vector<Sacado::Fad::DFad<double> > > Wplus (n_q_points,
							       std::vector<Sacado::Fad::DFad<double> >(EulerEquations<dim>::n_components));
  std::vector<std::vector<Sacado::Fad::DFad<double> > > Wminus (n_q_points,
								std::vector<Sacado::Fad::DFad<double> >(EulerEquations<dim>::n_components));


  const std::vector<Point<dim> > &normals = fe_v.get_normal_vectors ();


				   // If we are at a boundary, then
				   // dofs_neighbor are the same as dofs, so
				   // we do not want to duplicate them.  If
				   // there is a neighbor cell, then we want
				   // to include them.
  int ndofs = (external_face == false ? dofs_per_cell + ndofs_per_cell : dofs_per_cell);
				   // Set the local DOFS.
  for (unsigned int in = 0; in < dofs_per_cell; in++) {
    DOF[in] = current_solution(dofs[in]);
    DOF[in].diff(in, ndofs);
  }
				   // If present, set the neighbor dofs.
  if (external_face == false)
    for (unsigned int in = 0; in < ndofs_per_cell; in++) {
      DOF[in+dofs_per_cell] = current_solution(dofs_neighbor[in]);
      DOF[in+dofs_per_cell].diff(in+dofs_per_cell, ndofs);
    }

				   // Set the values of the local conservative variables.
				   // Initialize all variables to zero.
  for (unsigned int q = 0; q < n_q_points; q++) {
    for (unsigned int di = 0; di < EulerEquations<dim>::n_components; di++) {
      Wplus[q][di] = 0;
      Wminus[q][di] = 0;
    }
    for (unsigned int sf = 0; sf < dofs_per_cell; sf++) {
      int di = fe_v.get_fe().system_to_component_index(sf).first;
      Wplus[q][di] +=
	(parameters.theta*DOF[sf]+(1.0-parameters.theta)*old_solution(dofs[sf]))*fe_v.shape_value_component(sf, q, di);
    }


				     // If there is a cell across, then initialize
				     // the exterior trace as a function of the other
				     // cell degrees of freedom.
    if (external_face == false) {
      for (unsigned int sf = 0; sf < ndofs_per_cell; sf++) {
	int di = fe_v_neighbor.get_fe().system_to_component_index(sf).first;
	Wminus[q][di] +=
	  (parameters.theta*DOF[sf+dofs_per_cell]+(1.0-parameters.theta)*old_solution(dofs_neighbor[sf]))*
	  fe_v_neighbor.shape_value_component(sf, q, di);
      }
    } 
  } // for q

				   // If this is a boundary, then the values
				   // of $W^-$ will be either functions of
				   // $W^+$, or they will be prescribed.  This
				   // switch sets them appropriately.  Since
				   // we are using fad variables here,
				   // sensitivities will be updated
				   // appropriately.  These sensitivities
				   // would be tremendously difficult to
				   // manage without fad!!!
  if (external_face == true)
    {
      Assert (boundary_id < Parameters::AllParameters<dim>::max_n_boundaries,
	      ExcIndexRange (boundary_id, 0,
			     Parameters::AllParameters<dim>::max_n_boundaries));

				       // Evaluate the function object.  This is
				       // a bit tricky; a given boundary might
				       // have both prescribed and implicit
				       // values.  If a particular component is
				       // not prescribed, the values evaluate to
				       // zero and are ignored, below.
      std::vector<Vector<double> > bvals(n_q_points, Vector<double>(EulerEquations<dim>::n_components));
      parameters.boundary_conditions[boundary_id].values.vector_value_list(fe_v.get_quadrature_points(), bvals);

				       // We loop the quadrature points, and we treat each
				       // component individualy.
      for (unsigned int q = 0; q < n_q_points; q++) {
	for (unsigned int di = 0; di < EulerEquations<dim>::n_components; di++) {

					   // An inflow/dirichlet type of boundary condition
	  if (parameters.boundary_conditions[boundary_id].kind[di] == Parameters::AllParameters<dim>::inflow_boundary) {
	    Wminus[q][di] = bvals[q](di);
	  } else if (parameters.boundary_conditions[boundary_id].kind[di] == Parameters::AllParameters<dim>::pressure_boundary) {
					     // A prescribed pressure boundary
					     // condition.  This boundary
					     // condition is complicated by the
					     // fact that even though the
					     // pressure is prescribed, we
					     // really are setting the energy
					     // index here, which will depend on
					     // velocity and pressure. So even
					     // though this seems like a
					     // dirichlet type boundary
					     // condition, we get sensitivities
					     // of energy to velocity and
					     // density (unless these are also
					     // prescribed.
	    Sacado::Fad::DFad<double> rho_vel_sqr = 0;
	    Sacado::Fad::DFad<double> dens;
          
	    dens = parameters.boundary_conditions[boundary_id].kind[EulerEquations<dim>::density_component] == Parameters::AllParameters<dim>::inflow_boundary ? bvals[q](EulerEquations<dim>::density_component) :
		   Wplus[q][EulerEquations<dim>::density_component];

	    for (unsigned int d=0; d < dim; d++) {
	      if (parameters.boundary_conditions[boundary_id].kind[d] == Parameters::AllParameters<dim>::inflow_boundary)
		rho_vel_sqr += bvals[q](d)*bvals[q](d);
	      else
		rho_vel_sqr += Wplus[q][d]*Wplus[q][d];
	    }
	    rho_vel_sqr /= dens;
					     // Finally set the energy value as determined by the
					     // prescribed pressure and the other variables.
	    Wminus[q][di] = bvals[q](di)/(EulerEquations<dim>::gas_gamma-1.0) +
			    0.5*rho_vel_sqr;

	  } else if (parameters.boundary_conditions[boundary_id].kind[di] == Parameters::AllParameters<dim>::outflow_boundary) {
					     // A free/outflow boundary, very simple.
	    Wminus[q][di] = Wplus[q][di];

	  } else { 
					     // We must be at a no-penetration
					     // boundary.  We prescribe the
					     // velocity (we are dealing with a
					     // particular component here so
					     // that the average of the
					     // velocities is orthogonal to the
					     // surface normal.  This creates
					     // sensitivies of across the
					     // velocity components.
	    Sacado::Fad::DFad<double> vdotn = 0;
	    for (unsigned int d = 0; d < dim; d++) {
	      vdotn += Wplus[q][d]*normals[q](d);
	    }

	    Wminus[q][di] = Wplus[q][di] - 2.0*vdotn*normals[q](di);
	  }
	}
      } // for q
    } // b>= 0
   
				   // Determine the Lax-Friedrich's stability parameter,
				   // and evaluate the numerical flux function at the quadrature points
  typedef Sacado::Fad::DFad<double> NormalFlux[EulerEquations<dim>::n_components];
  NormalFlux *normal_fluxes = new NormalFlux[n_q_points];

  double alpha;

  switch(parameters.stabilization_kind)
    {
      case Parameters::Flux::constant:
	    alpha = parameters.stabilization_value;
	    break;
      case Parameters::Flux::mesh_dependent:
	    alpha = face_diameter/(2.0*parameters.time_step);
	    break;
      default:
	    Assert (false, ExcNotImplemented());
	    alpha = 1;
    }

  for (unsigned int q=0; q<n_q_points; ++q)
    EulerEquations<dim>::numerical_normal_flux(normals[q], Wplus[q], Wminus[q], alpha,
					       normal_fluxes[q]);

				   // Now assemble the face term
  for (unsigned int i=0; i<fe_v.dofs_per_cell; ++i)
    {
      if (!fe_v.get_fe().has_support_on_face(i, face_no))
	continue;
      
      F_i = 0;
      for (unsigned int point=0; point<n_q_points; ++point)
	{
	  const unsigned int
	    component_i = fe_v.get_fe().system_to_component_index(i).first;
	  
	  F_i += normal_fluxes[point][component_i] *
		 fe_v.shape_value_component(i, point, component_i) *
		 fe_v.JxW(point);
	} 

				       // Retrieve a pointer to the jacobian.
      double *values = &(F_i.fastAccessDx(0));
      Assert (values != 0, ExcInternalError());

				       // Update the matrix.  Depending on whether there
				       // is/isn't a neighboring cell, we add more/less
				       // entries.
      Matrix->SumIntoGlobalValues(dofs[i],
				  dofs_per_cell,
				  &values[0],
				  reinterpret_cast<int*>(const_cast<unsigned int*>(&dofs[0])));
      
      if (external_face == false)
	Matrix->SumIntoGlobalValues(dofs[i],
				    dofs_per_cell,
				    &values[dofs_per_cell],
				    reinterpret_cast<int*>(const_cast<unsigned int*>(&dofs_neighbor[0])));
      

				       // And add into the residual
      right_hand_side(dofs[i]) -= F_i.val();
    }

  delete[] normal_fluxes;
}



                                 // @sect3{Solving the linear system}
                                 // Actually solve the linear system, using either
                                 // Aztec or Amesos.
template <int dim>
std::pair<unsigned int, double>
ConservationLaw<dim>::solve (Vector<double> &newton_update) 
{

				   // We must hand the solvers Epetra vectors.
				   // Luckily, they support the concept of a 
				   // 'view', so we just send in a pointer to our
				   // dealii vectors.
  Epetra_Vector x(View, *Map, newton_update.begin());
  Epetra_Vector b(View, *Map, right_hand_side.begin());

				   // The Direct option selects the Amesos solver.
  if (parameters.solver == Parameters::Solver::direct) {
   
				     // Setup for solving with
				     // Amesos. Other solvers are
				     // available and may be selected by
				     // changing th string given to the
				     // <code>Create</code> function.
    Epetra_LinearProblem prob;
    prob.SetOperator(Matrix.get());
    Amesos_BaseSolver *solver = Amesos().Create ("Amesos_Klu", prob);

    Assert (solver != NULL, ExcInternalError());

				     // There are two parts to the direct solve.
				     // As I understand, the symbolic part figures
				     // out the sparsity patterns, and then the
				     // numerical part actually performs Gaussian
				     // elimination or whatever the approach is.
    if (parameters.output == Parameters::Solver::verbose)
      std::cout << "Starting Symbolic fact\n" << std::flush;

    solver->SymbolicFactorization();

    if (parameters.output == Parameters::Solver::verbose)
      std::cout << "Starting Numeric fact\n" << std::flush;

    solver->NumericFactorization();

    
				     // Define the linear problem by setting the
				     // right hand and left hand sides.
    prob.SetRHS(&b);
    prob.SetLHS(&x);
				     // And finally solve the problem.
    if (parameters.output == Parameters::Solver::verbose)
      std::cout << "Starting solve\n" << std::flush;
    solver->Solve();
				     // We must free the solver that was created
				     // for us.
    delete solver;

    return std::make_pair<unsigned int, double> (0, 0);
  }
  else if (parameters.solver == Parameters::Solver::gmres)
    {

				       // For the iterative solvers, we use Aztec.
      AztecOO Solver;

				       // Select the appropriate level of verbosity.
      if (parameters.output == Parameters::Solver::quiet)
	Solver.SetAztecOption(AZ_output, AZ_none);

      if (parameters.output == Parameters::Solver::verbose)
	Solver.SetAztecOption(AZ_output, AZ_all);

				       // Select gmres.  Other solvers are available.
      Solver.SetAztecOption(AZ_solver, AZ_gmres);
      Solver.SetRHS(&b);
      Solver.SetLHS(&x);

				       // Set up the ILUT preconditioner.  I do not know
				       // why, but we must pretend like we are in parallel
				       // using domain decomposition or the preconditioner
				       // refuses to activate.
      Solver.SetAztecOption(AZ_precond,         AZ_dom_decomp);
      Solver.SetAztecOption(AZ_subdomain_solve, AZ_ilut);
      Solver.SetAztecOption(AZ_overlap,         0);
      Solver.SetAztecOption(AZ_reorder,         0);

				       // ILUT parameters as described above.
      Solver.SetAztecParam(AZ_drop,      parameters.ilut_drop);
      Solver.SetAztecParam(AZ_ilut_fill, parameters.ilut_fill);
      Solver.SetAztecParam(AZ_athresh,   parameters.ilut_atol);
      Solver.SetAztecParam(AZ_rthresh,   parameters.ilut_rtol);
      Solver.SetUserMatrix(Matrix.get());

				       // Run the solver iteration.  Collect the number
				       // of iterations and the residual.
      Solver.Iterate(parameters.max_iterations, parameters.linear_residual);

      return std::make_pair<unsigned int, double> (Solver.NumIters(),
						   Solver.TrueResidual());
    }

  Assert (false, ExcNotImplemented());
  return std::make_pair<unsigned int, double> (0,0);
}

				 // Loop and assign a value for refinement.  We
				 // simply use the density squared, which selects
				 // shocks with some success.
template <int dim>
void
ConservationLaw<dim>::
compute_refinement_indicators (Vector<double> &refinement_indicators) const
{  
  const unsigned int dofs_per_cell = dof_handler.get_fe().dofs_per_cell;
  std::vector<unsigned int> dofs (dofs_per_cell);
  UpdateFlags update_flags = update_values
			     | update_gradients
			     | update_q_points
			     | update_JxW_values;

  QGauss<dim>  quadrature_formula(1);
  unsigned int n_q_points = quadrature_formula.n_quadrature_points;


  FEValues<dim> fe_v (
    mapping, fe, quadrature_formula, update_flags);

  std::vector<Vector<double> > U(n_q_points,
                                 Vector<double>(EulerEquations<dim>::n_components));
  std::vector<std::vector<Tensor<1,dim> > > dU(n_q_points,
					       std::vector<Tensor<1,dim> >(EulerEquations<dim>::n_components));
  
  typename DoFHandler<dim>::active_cell_iterator
    cell = dof_handler.begin_active(),
    endc = dof_handler.end();
  for (unsigned int cell_no=0; cell!=endc; ++cell, ++cell_no)
    {
      fe_v.reinit(cell);

      fe_v.get_function_values(predictor, U);
      fe_v.get_function_grads(predictor, dU);

      refinement_indicators(cell_no) = 0;
      for (unsigned int q = 0; q < n_q_points; q++) {
	double ng = 0;
	for (unsigned int d = 0; d < dim; d++)
	  ng += dU[q][EulerEquations<dim>::density_component][d] *
		dU[q][EulerEquations<dim>::density_component][d];

	refinement_indicators(cell_no) += std::log(1+std::sqrt(ng));
      
      } 
      refinement_indicators(cell_no) /= n_q_points;
  } 
}

template <int dim>
void ConservationLaw<dim>::refine_grid (const Vector<double> &refinement_indicators)
{

  SolutionTransfer<dim, double> soltrans(dof_handler);

  typename DoFHandler<dim>::active_cell_iterator
    cell = dof_handler.begin_active(),
    endc = dof_handler.end();

				   // Loop cells.  If the indicator
				   // for the cell matches the refinement criterion,
				   // refine, else unrefine.  The unrefinement has
				   // a slight hysterisis to avoid 'flashing' from refined
				   // to unrefined.
  for (unsigned int cell_no=0; cell!=endc; ++cell, ++cell_no) {
    cell->clear_coarsen_flag();
    cell->clear_refine_flag();
    if (cell->level() < parameters.shock_levels &&
        std::fabs(refinement_indicators(cell_no)) > parameters.shock_val ) {
      cell->set_refine_flag();
    } else {
      if (cell->level() > 0 &&
	  std::fabs(refinement_indicators(cell_no)) < 0.75*parameters.shock_val)
	cell->set_coarsen_flag();
    }
  }

				   // The following code prolongs the old_solution
				   // to the new grid and carries out the refinement.
  std::vector<Vector<double> > interp_in;
  std::vector<Vector<double> > interp_out;

  interp_in.push_back(old_solution);
  interp_in.push_back(predictor);

  triangulation.prepare_coarsening_and_refinement();
  soltrans.prepare_for_coarsening_and_refinement(interp_in);

  triangulation.execute_coarsening_and_refinement ();

  dof_handler.clear();
  dof_handler.distribute_dofs (fe);

  {
    Vector<double> new_old_solution(1);
    Vector<double> new_predictor(1);

    interp_out.push_back(new_old_solution);
    interp_out.push_back(new_predictor);
    interp_out[0].reinit(dof_handler.n_dofs());
    interp_out[1].reinit(dof_handler.n_dofs());
  }

  soltrans.interpolate(interp_in, interp_out);

  old_solution.reinit (interp_out[0].size());
  old_solution = interp_out[0];

  predictor.reinit (interp_out[1].size());
  predictor = interp_out[1];

				   // resize these vectors for the new grid.
  current_solution.reinit(dof_handler.n_dofs());
  current_solution = old_solution;
  right_hand_side.reinit (dof_handler.n_dofs());
}



template <int dim>
void ConservationLaw<dim>::output_results () const
{
  typename EulerEquations<dim>::Postprocessor 
    postprocessor (parameters.schlieren_plot);

  DataOut<dim> data_out;
  data_out.attach_dof_handler (dof_handler);
  
  data_out.add_data_vector (old_solution,
			    EulerEquations<dim>::component_names (),
			    DataOut<dim>::type_dof_data,
			    EulerEquations<dim>::component_interpretation ());

  data_out.add_data_vector (old_solution, postprocessor);

  data_out.build_patches ();

  static unsigned int output_file_number = 0;
  std::string filename = "old_solution-" +
			 Utilities::int_to_string (output_file_number, 3) +
			 ".vtk";
  std::ofstream output (filename.c_str());
  data_out.write_vtk (output);

  ++output_file_number;
}




				 // @sect3{Run the simulation}
				 // Contains the initialization
				 // the time loop, and the inner Newton iteration.
template <int dim>
void ConservationLaw<dim>::run () 
{

				   // Open and load the mesh.
  {
    GridIn<dim> grid_in;
    grid_in.attach_triangulation(triangulation);

    std::ifstream input_file(parameters.mesh_filename.c_str());
    Assert (input_file, ExcFileNotOpen(parameters.mesh_filename.c_str()));

    grid_in.read_ucd(input_file);   
  }
  
				   // Initialize fields and matrices.
				   // First we need to distribute the
				   // DoFs.
  dof_handler.clear();
  dof_handler.distribute_dofs (fe);
  
                                   // Size all of the fields.
  old_solution.reinit (dof_handler.n_dofs());
  current_solution.reinit (dof_handler.n_dofs());
  predictor.reinit (dof_handler.n_dofs());
  right_hand_side.reinit (dof_handler.n_dofs());

  setup_system();

  VectorTools::interpolate(dof_handler,
                           parameters.initial_conditions, old_solution);
  current_solution = old_solution;
  predictor = old_solution;

				   // Initial refinement.  We apply the ic,
				   // estimate, refine, and repeat until
				   // happy.
  if (parameters.do_refine == true)
    for (unsigned int i = 0; i < parameters.shock_levels; i++)
      {
	Vector<double> refinement_indicators (triangulation.n_active_cells());
	compute_refinement_indicators(refinement_indicators);
	refine_grid(refinement_indicators);
	setup_system();

	VectorTools::interpolate(dof_handler,
				 parameters.initial_conditions, old_solution);
	current_solution = old_solution;
	predictor = old_solution;
      }

  output_results ();

				   // Determine when we will output next.
  double time = 0;
  double next_output = time + parameters.output_step;

				   // @sect4{Main time stepping loop}
  predictor = old_solution;
  Vector<double> newton_update (dof_handler.n_dofs());
  while (time < parameters.final_time)
    {
      std::cout << "T=" << time << ", ";


      std::cout << "   Number of active cells:       "
		<< triangulation.n_active_cells()
		<< std::endl;


      std::cout << "   Number of degrees of freedom: "
		<< dof_handler.n_dofs()
		<< std::endl;

      bool nonlin_done = false;

				       // Print some relevant information during the
				       // Newton iteration.
      std::cout << "NonLin Res:       Lin Iter     Lin Res" << std::endl;
      std::cout << "______________________________________" << std::endl;

      const unsigned int max_nonlin = 7;
      unsigned int nonlin_iter = 0;

				       // @sect5{Newton iteration}
      current_solution = predictor;
      while (!nonlin_done) {
	Matrix->PutScalar(0);
	Matrix->FillComplete();
	
        right_hand_side = 0;
        assemble_system ();
	
					 // Flash a star to the screen so one can
					 // know when the assembly has stopped and the linear
					 // old_solution is starting.
        std::cout << "* " << std::flush;

					 // Test against a (hardcoded) nonlinear tolderance.
					 // Do not solve the linear system at the last step 
					 // (since it would be a waste).
                      
	const double res_norm = right_hand_side.l2_norm();
        if (std::fabs(res_norm) < 1e-10)
	  {
	    nonlin_done = true;
	    std::printf("%-16.3e (converged)\n", res_norm);
	  }
	else
	  {
					     // Solve the linear system and update with the
					     // delta.
	    newton_update = 0;

	    std::pair<unsigned int, double> convergence
	      = solve (newton_update);
	    
	    current_solution.add(1.0, newton_update);
	    
	    std::printf("%-16.3e %04d        %-5.2e\n",
			res_norm, convergence.first, convergence.second);
	  }

        ++nonlin_iter;

	AssertThrow (nonlin_iter <= max_nonlin,
		     ExcMessage ("No convergence in nonlinear solver"));
      } 

				       // Various post convergence tasks.

				       // We use a predictor to try and make
				       // adaptivity work better.  The idea is to
				       // try and refine ahead of a front, rather
				       // than stepping into a coarse set of
				       // elements and smearing the old_solution.  This
				       // simple time extrapolator does the job.
      predictor = current_solution;
      predictor.sadd(3/2.0, -1/2.0, old_solution);

      old_solution = current_solution;

      Vector<double> refinement_indicators (triangulation.n_active_cells());
      compute_refinement_indicators(refinement_indicators);

      time += parameters.time_step;

				       // Output if it is time.
      if (parameters.output_step < 0)
	output_results ();
      else if (time >= next_output)
	{
	  output_results ();
	  next_output += parameters.output_step;
	}

				       // Refine, if refinement is selected.
      if (parameters.do_refine == true)
	{
	  refine_grid(refinement_indicators);
	  setup_system();

	  newton_update.reinit (dof_handler.n_dofs());
	}
    }
}

				 // The following ``main'' function is
				 // similar to previous examples and
				 // need not to be commented on. Note
				 // that the program aborts if no
				 // input file name is given on the
				 // command line.
int main (int argc, char *argv[]) 
{
  if (argc != 2)
    {
      std::cout << "Usage:" << argv[0] << " infile" << std::endl;
      std::exit(1);
    }
  
  try
    {
      ConservationLaw<2> cons (argv[1]);
      cons.run ();
    }
  catch (std::exception &exc)
    {
      std::cerr << std::endl << std::endl
		<< "----------------------------------------------------"
		<< std::endl;
      std::cerr << "Exception on processing: " << std::endl
		<< exc.what() << std::endl
		<< "Aborting!" << std::endl
		<< "----------------------------------------------------"
		<< std::endl;
      return 1;
    }
  catch (...) 
    {
      std::cerr << std::endl << std::endl
		<< "----------------------------------------------------"
		<< std::endl;
      std::cerr << "Unknown exception!" << std::endl
		<< "Aborting!" << std::endl
		<< "----------------------------------------------------"
		<< std::endl;
      return 1;
    };
  
  return 0;
}

