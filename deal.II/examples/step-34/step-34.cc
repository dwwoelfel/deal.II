//----------------------------  step-34.cc  ---------------------------
//    $Id: testsuite.html 13373 2006-07-13 13:12:08Z manigrasso $
//    Version: $Name$ 
//
//    Copyright (C) 2009 by the deal.II authors 
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//    Authors: Luca Heltai, Cataldo Manigrasso
//
//----------------------------  step-34.cc  ---------------------------

#include <base/smartpointer.h>
#include <base/convergence_table.h>
#include <base/quadrature_lib.h>
#include <base/quadrature_selector.h>
#include <base/table.h>
#include <base/parsed_function.h>
#include <base/utilities.h>

#include <lac/full_matrix.h>
#include <lac/precondition.h>
#include <lac/solver_cg.h>
#include <lac/vector.h>
#include <lac/sparse_direct.h>
#include <lac/lapack_full_matrix.h>
#include <lac/matrix_lib.h>

#include <grid/tria.h>
#include <grid/tria_iterator.h>
#include <grid/tria_accessor.h>
#include <grid/grid_generator.h>
#include <grid/grid_in.h>
#include <grid/grid_out.h>
#include <grid/tria_boundary_lib.h>

#include <dofs/dof_accessor.h>
#include <dofs/dof_tools.h>

#include <fe/fe_dgp.h>
#include <fe/fe_system.h>
#include <fe/fe_tools.h>
#include <fe/fe_values.h>
#include <fe/mapping_q1.h>

#include <numerics/data_out.h>
#include <numerics/vectors.h>

#include <cmath>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string>


using namespace dealii;


    // The following two functions are the actual calculations of the
    // single and double layer potential kernels, that is G and Grad
    // G. They are well defined only if the vector $R = y-x$ is
    // different from zero.
    // 
    // Whenever the integration is performed with the singularity
    // inside the given cell, then a special quadrature formula is
    // used that allows one to integrate arbitrary functions against a
    // singular weight on the reference cell.
    //
    // There are two options when the integral is singular. One could
    // take into account the singularity inside the quadrature formula
    // as a weigthing function, or one could use a quadrature formula
    // that is taylored to integrate singular objects, but where the
    // actual weighting function is one. The use of the first method
    // requires the user to provide a "desingularized" single and
    // double layer potentials which can then be integrated on the
    // given cell. When the @p factor_out_singularity parameter is set
    // to true, then the computed kernels do not conatain the singular
    // factor, which is included in the quadrature formulas as a
    // weighting function. This works best in two dimension, where the
    // singular integrals are integrals along a segment of a
    // logarithmic singularity.
    //
    // These integrals are somewhat delicate, because inserting a
    // factor Jx in the variable of integration does not result only
    // in a factor J appearing as a constant factor on the entire
    // integral, but also on an additional integral to be added, that
    // contains the logarithm of J. For this reason in two dimensions
    // we opt for the desingularized kernel, and use the QGaussLogR
    // quadrature formula, that takes care of integrating the correct
    // weight for us. 
    //
    // In the three dimensional case the singular integral is taken
    // care of using the QGaussOneOverR quadrature formula. We could
    // use the desingularized kernel here as well, but this would
    // require us to be careful about the different scaling of r in
    // the reference cell and in real space. The quadrature formula
    // uses as weight 1/r in local coordinates, while we need to
    // integrate 1/R in real coordinates. A factor of r/R has to be
    // introduced in the quadrature formula. This can be done
    // manually, or we simply calculate the standard kernels and then
    // use a desingularized quadrature formula, i.e., one which is
    // taylored for singular integrals, but whose weight is 1 instead
    // of the singularity.
    //
    // Notice that the QGaussLog quadrature formula is made to
    // integrate f(x)ln|x-x0|, but the kernel for two dimensional
    // problems has the opposite sign. This is taken care of by
    // switching the sign of the two dimensional desingularized
    // kernel.
    //
    // The last argument to both functions is simply ignored in three
    // dimensions.
namespace LaplaceKernel
{
template <int dim>
double single_layer(const Point<dim> &R, 
		    const bool factor_out_2d_singularity = false) {
    switch(dim) {
    case 2:
        if(factor_out_2d_singularity == true) 
            return -1./(2*numbers::PI);
        else
            return (-std::log(R.norm()) / (2*numbers::PI) );
        break;
    case 3:
        return (1./( R.norm()*4*numbers::PI ) );
        break;
    default:
        Assert(false, ExcInternalError());
        return 0.;
        break;
    }
    return 0.;
}
        


template <int dim>
Point<dim> double_layer(const Point<dim> &R,
			const bool factor_out_2d_singularity = false) {
  switch(dim) {
    case 2:
	  if (factor_out_2d_singularity)
	    return Point<dim>();
	  else
	    return R / (-2*numbers::PI * R.square());
    case 3:
	  return R / ( -4*numbers::PI * R.square()*R.norm() );

      default:
        Assert(false, ExcInternalError());
        break;
    }
  return Point<dim>();
}
}



template <int dim> 
class BEMProblem 
{
public:
    BEMProblem();
    
    // The structure of a boundary element method code is very similar
    // to the structure of a finite element code. By now you should be
    // familiar with reading paramaters from an external file, and
    // with the splitting of the different tasks into different
    // modules. The same applyes to boundary element methods, and we
    // won't comment too much on them, except on the differences.

    void read_parameters (const std::string filename);
    
    void run();
    
    void read_domain();

    void refine_and_resize();
    
    // The only really different function that we find here is the
    // assembly routine. We wrote this function in the most possible
    // general way, in order to allow for easy generalization to
    // higher order methods and to different fundamental solutions
    // (e.g., Stokes or Maxwell).
    //
    // The most noticeable difference is the fact that the final
    // matrix is full, and that we have a nested loop inside the usual
    // loop on cells that visits all support points of the degrees of
    // freedom.  Moreover, when the support point lyes inside the cell
    // which we are visiting, then the integral we perform becomes
    // singular.
    //
    // The practical consequence is that we have two sets of
    // quadrature formulas, finite element values and temporary
    // elements, one for standard integration and one for the singular
    // integration, which are used where necessary.
    void assemble_system();

    // Notwithstanding the fact that the matrix is full, we use a
    // SparseMatrix object and the SparseDirectUMFPACK solver, since
    // in our experience it works better then using, for example, the
    // LapackFullMatrix class. An alternative approach would be the
    // use of GMRES method, however the construction of an efficient
    // preconditioner for boundary element methods is not a trivial
    // issue, and we won't treat this problem here.
    //
    // We should notice moreover that the solution we will obtain will
    // only be unique up to an additive constant. This is taken care
    // of in the solve_system method, which filters out the mean value
    // of the solution at the end of the computation.
    void solve_system();

    // Once we obtained the solution, we compute the $L^2$ error of
    // the computed potential as well as the $L^\infty$ error of the
    // approximation of the solid angle. The mesh we are using is an
    // approximation of a smooth curve, therefore the computed
    // diagonal matrix of fraction of angles or solid angles
    // $\alpha(\mathbf{x})$ should be constantly equal to $\frac
    // 12$. In this routine we output the error on the potential and
    // the error in the approximation of the computed angle. Notice
    // that the latter error is actually not the error in the
    // computation of the angle, but a measure of how well we are
    // approximating the sphere and the circle.
    //
    // Experimenting a little with the computation of the angles gives
    // very accurate results for simpler geometries. To verify this
    // you can comment out, in the read_domain() method, the
    // tria.set_boundary(1, boundary) line, and check the alpha that
    // is generated by the program. In the three dimensional case, the
    // coarse grid of the sphere is obtained starting from a cube, and
    // the obtained values of alphas are exactly $\frac 12$ on the
    // nodes of the faces, $\frac 14$ on the nodes of the edges and
    // $\frac 18$ on the 8 nodes of the vertices. 
    void compute_errors(const unsigned int cycle);
    
    // Once we obtained a solution on the codimension one domain, we
    // want to interpolate it to the rest of the
    // space. This is done by performing again the convolution of the
    // solution with the kernel in the compute_exterior_solution() function.
    //
    // We would like to plot the velocity variable which is the
    // gradient of the potential solution. The potential solution is
    // only known on the boundary, but we use the convolution with the
    // fundamental solution to interpolate it on a standard dim
    // dimensional continuous finite element space. The plot of the
    // gradient of the extrapolated solution will give us the velocity
    // we want.
    void compute_exterior_solution();
    
    void output_results(unsigned int cycle);
    
private:
    // The usual deal.II classes can be used for boundary element
    // methods by specifying the "codimension" of the problem. This is
    // done by setting the optional template arguments to
    // Triangulation, FiniteElement and DoFHandler to the dimension of
    // the embedding space. In our case we generate either 1 or 2
    // dimensional meshes embedded in 2 or 3 dimensional spaces.
    //
    // The optional argument by default is equal to the first
    // argument, and produces the usual finite element classes that we
    // saw in all previous examples.

    Triangulation<dim-1, dim>   tria;
    FE_Q<dim-1,dim>             fe;
    DoFHandler<dim-1,dim>       dh;

    // In BEM methods, the matrix that is generated is
    // dense. Depending on the size of the problem, the final system
    // might be solved by direct LU decomposition, or by iterative
    // methods. In this example we use the SparseDirectUMFPACK solver,
    // applied to a "fake" sparse matrix (a sparse matrix will all
    // entries different from zero). We found that this method is
    // faster than using the LapackFullMatrix object.

    SparsityPattern             sparsity;
    SparseMatrix<double>        system_matrix;    
    Vector<double>              system_rhs;
    Vector<double>              phi;
    Vector<double>              alpha;
    
    // The reconstruction of the solution in the entire space is done
    // on a continuous finite element grid of dimension dim. These are
    // the usual ones, and we don't comment any further on them.
    
    Triangulation<dim>  external_tria;
    FE_Q<dim>           external_fe;
    DoFHandler<dim>     external_dh;
    Vector<double>      external_phi;
    
    // The convergence table is used to output errors in the exact
    // solution and in the computed alphas. 
    ConvergenceTable	convergence_table;
    
    // The following variables are the one that we fill through a
    // parameter file.  The new objects that we use in this example
    // are the ParsedFunction object and the QuadratureSelector
    // object.
    //
    // The ParsedFunction class allows us to easily and quickly define
    // new function objects via parameter files, with custom
    // definitions which can be very complex (see the documentation of
    // that class for all the available options).
    //
    // The QuadratureSelector class allows us to generate quadrature
    // formulas based on an identifying string and on the possible
    // degree of the formula itself. We used this to allow custom
    // selection of the quadrature formulas for the standard
    // integration, and to define the order of the singular quadrature
    // rule.
    //
    // Notice that the pointer given below for the quadrature rule is
    // only used for non singular integrals. Whenever the integral is
    // singular, then only the degree of the quadrature pointer is
    // used, and the integration is a special one (see the
    // assemble_matrix below for further details).
    //
    // We also define a couple of parameters which are used in case we
    // wanted to extend the solution to the entire domain. 
    Functions::ParsedFunction<dim> wind;
    Functions::ParsedFunction<dim> exact_solution;

    std_cxx0x::shared_ptr<Quadrature<dim-1> > quadrature;
    unsigned int singular_quadrature_order;

    unsigned int n_cycles;
    unsigned int external_refinement;

    bool run_in_this_dimension;
    bool extend_solution;
};



// The constructor initializes the variuous object in the same way of
// finite element problems. The only new ingredient here is the
// ParsedFunction object, which needs, at construction time, the
// specification of the number of components.
//
// For the exact solution this is one, and no action is required since
// one is the default value for a ParsedFunction object. The wind,
// however, requires dim components to be specified. Notice that when
// declaring entries in a parameter file for the expression of the
// ParsedFunction, we need to specify the number of components
// explicitly, since the function
// ParsedFunction<dim>::declare_parameters is static, and has no
// knowledge of the number of components. 
template <int dim>
BEMProblem<dim>::BEMProblem() :
    fe(1),
    dh(tria),
    external_fe(1),
    external_dh(external_tria),
    wind(dim)
{}

template <int dim> 
void BEMProblem<dim>::read_parameters (const std::string filename) {
    deallog << std::endl << "Parsing parameter file " << filename << std::endl
            << "for a " << dim << " dimensional simulation. " << std::endl;
    
    ParameterHandler prm;
    
    prm.declare_entry("Number of cycles", "4",
		      Patterns::Integer());
    prm.declare_entry("External refinement", "5",
		      Patterns::Integer());
    prm.declare_entry("Extend solution on the -2,2 box", "true",
		      Patterns::Bool());
    prm.declare_entry("Run 2d simulation", "true",
		      Patterns::Bool());
    prm.declare_entry("Run 3d simulation", "true",
		      Patterns::Bool());
    
    prm.enter_subsection("Quadrature rules");
    {
      prm.declare_entry("Quadrature type", "gauss", 
			Patterns::Selection(QuadratureSelector<(dim-1)>::get_quadrature_names()));
      prm.declare_entry("Quadrature order", "4", Patterns::Integer());
      prm.declare_entry("Singular quadrature order", "5", Patterns::Integer());
    }
    prm.leave_subsection();
    
    // For both two and three dimensions, we set the default input
    // data to be such that the solution is $x+y+c$ or $x+y+z+c$.
    //
    // The use of the ParsedFunction object is pretty straight
    // forward. The declare parameters function takes an additional
    // integer argument that specifies the number of components of the
    // given function. Its default value is one. When the
    // correspending parse_parameters method is called, the calling
    // object has to have the same number of components defined here,
    // otherwise an exception is thrown.
    //
    // When declaring entries, we declare both 2 and three dimensional
    // functions. However only the dim-dimensional one is parsed. This
    // allows us to have only one parameter file for both 2 and 3
    // dimensional problems.
    prm.enter_subsection("Wind function 2d");
    {
      Functions::ParsedFunction<2>::declare_parameters(prm, 2);
      prm.set("Function expression", "1; 1");
    }
    prm.leave_subsection();

    prm.enter_subsection("Wind function 3d");
    {
      Functions::ParsedFunction<3>::declare_parameters(prm, 3);
      prm.set("Function expression", "1; 1; 1");
    }
    prm.leave_subsection();

    prm.enter_subsection("Exact solution 2d");
    {
      Functions::ParsedFunction<2>::declare_parameters(prm);
      prm.set("Function expression", "x+y");
    }
    prm.leave_subsection();

    prm.enter_subsection("Exact solution 3d");
    {
      Functions::ParsedFunction<3>::declare_parameters(prm);
      prm.set("Function expression", "x+y+z");
    }
    prm.leave_subsection();
    
    prm.read_input(filename);

    n_cycles = prm.get_integer("Number of cycles");                   
    external_refinement = prm.get_integer("External refinement");
    extend_solution = prm.get_bool("Extend solution on the -2,2 box");
    
    // If we wanted to switch off one of the two simulations, we could
    // do this by setting the corresponding "Run 2d simulation" or
    // "Run 3d simulation" flag to false.
    //
    // This is another example of how to use parameter files in
    // dimensio independent programming.
    run_in_this_dimension = prm.get_bool("Run " + 
                                         Utilities::int_to_string(dim) +
                                         "d simulation");

    prm.enter_subsection("Quadrature rules");
    {
      quadrature =
	std_cxx0x::shared_ptr<Quadrature<dim-1> >
	(new QuadratureSelector<dim-1> (prm.get("Quadrature type"),
					prm.get_integer("Quadrature order")));
      singular_quadrature_order = prm.get_integer("Singular quadrature order");
    }
    prm.leave_subsection();
    
    prm.enter_subsection(std::string("Wind function ")+
                         Utilities::int_to_string(dim)+std::string("d"));
    {
      wind.parse_parameters(prm);
    }
    prm.leave_subsection();

    prm.enter_subsection(std::string("Exact solution ")+
                         Utilities::int_to_string(dim)+std::string("d"));
    {
      exact_solution.parse_parameters(prm);
    }
    prm.leave_subsection();
}


        
template <int dim>
void BEMProblem<dim>::read_domain() {
    
    // A boundary element method triangulation is basically the same
    // as a (dim-1) triangulation, with the difference that the
    // vertices belong to a (dim) dimensional space.
    //
    // Some of the mesh formats supported in deal.II use by default
    // three dimensional points to describe meshes. These are the
    // formats which are compatible with the boundary element method
    // capabilities of deal.II. In particular we can use either UCD or
    // GMSH formats. In both cases, we have to be particularly careful
    // with the orientation of the mesh, because, unlike in the
    // standard finite element case, no reordering or compatibility
    // check is performed here.
    //
    // All meshes are considered as oriented, because they are
    // embedded in a higher dimensional space. See the documentation
    // of the GridIn and of the Triangulation for further details on
    // the orientation. In our case, the normals to the mesh are
    // external to both the circle and the sphere. 
    //
    // The other detail that is required for appropriate refinement of
    // the boundary element mesh, is an accurate description of the
    // manifold that the mesh is approximating. We already saw this
    // several times for the boundary of standard finite element
    // meshes, and here the principle and usage is the same, except
    // that the Boundary description class takes an additional
    // template parameter that specifies the embedding space
    // dimension. 
    
    static HyperBallBoundary<dim-1, dim> boundary(Point<dim>(),1.);    

    std::ifstream in;
    switch (dim)
      {
	case 2:
	      in.open ("coarse_circle.inp");
	      break;
	      
	case 3:
	      in.open ("coarse_sphere.inp");
	      break;

	default:
	      Assert (false, ExcNotImplemented());
      }

    GridIn<dim-1, dim> gi;
    gi.attach_triangulation (tria);
    gi.read_ucd (in);
    tria.set_boundary(1, boundary);
}



template <int dim>
void BEMProblem<dim>::refine_and_resize() {
    tria.refine_global(1);
    
    dh.distribute_dofs(fe);
    
    const unsigned int n_dofs =  dh.n_dofs();
    
    // The matrix is a full matrix. Notwithstanding this fact, the
    // SparseMatrix class coupled with the SparseDirectUMFPACK solver
    // are still faster than Lapack solvers. The drawback is that we
    // need to assemble a full SparsityPattern.
    system_matrix.clear();
    sparsity.reinit(n_dofs, n_dofs, n_dofs);
    for(unsigned int i=0; i<n_dofs;++i)
        for(unsigned int j=0; j<n_dofs; ++j)
            sparsity.add(i,j);
    sparsity.compress();
    system_matrix.reinit(sparsity);
    
    system_rhs.reinit(n_dofs);
    phi.reinit(n_dofs);
    alpha.reinit(n_dofs);
}    

template <int dim>
void BEMProblem<dim>::assemble_system() {
    
    typename DoFHandler<dim-1,dim>::active_cell_iterator
        cell = dh.begin_active(),
        endc = dh.end();
    
    // We create initially the singular quadratures for the
    // threedimensional problem, since in this case they only
    // dependent on the reference element. This quadrature is a
    // standard Gauss quadrature formula reparametrized in such a way
    // that allows one to integrate singularities of the kind 1/R
    // centered at one of the vertices. Here we define a vector of
    // four such quadratures that will be used later on, only in the
    // three dimensional case.
    std::vector<QGaussOneOverR<2> > sing_quadratures_3d; 
    for(unsigned int i=0; i<4; ++i) {
        sing_quadratures_3d.push_back
            (QGaussOneOverR<2>(singular_quadrature_order, i, true));
    }
    
    // Initialize an FEValues object with the quadrature formula for the
    // integration of the kernel in non singular cells. This quadrature is
    // selected with the parameter file, and should be quite precise, since
    // the functions we are integrating are not polynomial functions.
    FEValues<dim-1,dim> fe_v(fe, *quadrature,
                             update_values |
                             update_cell_normal_vectors |
                             update_quadrature_points |
                             update_JxW_values);
    
    const unsigned int n_q_points = fe_v.n_quadrature_points;
    
    std::vector<unsigned int> dofs(fe.dofs_per_cell);

    std::vector<Vector<double> > cell_wind(n_q_points, Vector<double>(dim) );
    double normal_wind;
    
    // Unlike in finite element methods, if we use a collocation
    // boundary element method, then in each assembly loop we only
    // assemble the informations that refer to the coupling between
    // one degree of freedom (the degree associated with support point
    // i) and the current cell. This is done using a vector of
    // fe.dofs_per_cell elements, which will then be distributed to
    // the matrix in the global row i.
    Vector<double>      local_matrix_row_i(fe.dofs_per_cell);
    
    Point<dim> R;

    // The index i runs on the collocation points, which are the
    // support of the ith basis function, while j runs on inner
    // integration. We perform this check here to ensure that we are
    // not trying to use this code for high order elements. It will
    // only work with Q1 elements, that is, for fe.dofs_per_cell ==
    // GeometryInfo<dim>::vertices_per_cell.
    AssertThrow(fe.dofs_per_cell == GeometryInfo<dim-1>::vertices_per_cell,
                ExcDimensionMismatch(fe.dofs_per_cell, 
                                     GeometryInfo<dim-1>::vertices_per_cell));
    
    // Now that we checked that the number of vertices is equal to the
    // number of degrees of freedom, we construct a vector of support
    // points which will be used in the local integrations.
    std::vector<Point<dim> > support_points(dh.n_dofs());
    DoFTools::map_dofs_to_support_points<dim-1, dim>( StaticMappingQ1<dim-1, dim>::mapping,
                                                      dh, support_points);
    
    for(cell = dh.begin_active(); cell != endc; ++cell) {

        fe_v.reinit(cell);
        cell->get_dof_indices(dofs);
        
        const std::vector<Point<dim> > &q_points = fe_v.get_quadrature_points();
        const std::vector<Point<dim> > &normals = fe_v.get_cell_normal_vectors();
        wind.vector_value_list(q_points, cell_wind);
        
        
        for(unsigned int i=0; i<dh.n_dofs() ; ++i) {
            
            local_matrix_row_i = 0;
            
            // The integral that we need to perform is singular if one
            // of the local degrees of freedom is the same of the
            // support point i. Here we check wether this is the case,
            // and we store which one is the singular index.
            bool is_singular = false; 
            unsigned int singular_index = numbers::invalid_unsigned_int;
            
            for(unsigned int j=0; j<fe.dofs_per_cell; ++j) 
                if(dofs[j] == i) {
                    singular_index = j;
                    is_singular = true;
                }

            if(is_singular == false) {
                for(unsigned int q=0; q<n_q_points; ++q) {
                    normal_wind = 0;
                    for(unsigned int d=0; d<dim; ++d) 
                        normal_wind += normals[q][d]*cell_wind[q](d);
                    
                    // Distance between the external support point
                    // and the quadrature point on the internal
                    // cell.
                    R = q_points[q] - support_points[i];
                        
                    system_rhs(i) += ( LaplaceKernel::single_layer(R)   * 
                                       normal_wind                      *
                                       fe_v.JxW(q) );
                        
                    for(unsigned int j=0; j<fe.dofs_per_cell; ++j) {
                        
		      local_matrix_row_i(j) += ( ( LaplaceKernel::double_layer(R)     * 
                                                        normals[q] )            *
                                                      fe_v.shape_value(j,q)     *
                                                      fe_v.JxW(q)       );
                    }
                }
            } else {
                // Now we treat the more delicate case. If we are
                // here, it means that the cell that runs on the j
                // index contains the support_point[i]. In this case
                // both the single and the double layer potential are
                // singular, and they require a special treatment, as
                // explained in the introduction.
                //
                // In the two dimensional case we perform the integration
                // using a QGaussLogR quadrature formula, which is
                // specifically designed to integrate logarithmic
                // singularities on the unit interval, while in three
                // dimensions we use the QGaussOneOverR, which allows us to
                // integrate 1/R singularities on the vertices of the
                // reference element. Since we don't want to rebuild the two
                // dimensional quadrature formula at each singular
                // integration, we built them outside the loop on the cells,
                // and we only use a pointer to that quadrature here.
                //
                // Notice that in one dimensional integration this is not
                // possible, since we need to know the scaling parameter for
                // the quadrature, which is not known a priori.
                //
                // Dimension independent programming here is a little tricky,
                // but can be achieved via dynamic casting. We check that
                // everything went ok with an assertion at the end of this
                // block. Notice that the dynamic cast will only work when the
                // dimension is the correct one, in which case it is possible
                // to cast a QGaussLogR and QGaussOneOverR to a Quadrature<1>
                // and Quadrature<2> object.
                //
                // In the other cases this won't be called, and even if it
                // was, the dynamic_cast function would just return a null
                // pointer. We check that this is not the case with the Assert
                // at the end.
                //
                // Notice that in two dimensions the singular quadrature rule
                // depends also on the size of the current cell. For this
                // reason, it is necessary to create a new quadrature for each
                // singular integration. Since we create it using the new
                // operator of C++, we also need to destroy it using the dual
                // of new: delete. This is done at the end, and only if dim ==
                // 2.
                Assert(singular_index != numbers::invalid_unsigned_int,
                       ExcInternalError());

                Quadrature<dim-1> * singular_quadrature;
                
                if(dim == 2) {
                    singular_quadrature = dynamic_cast<Quadrature<dim-1> *>(
                        new QGaussLogR<1>(singular_quadrature_order,
                                          Point<1>((double)singular_index),
                                          1./cell->measure()));
                        } else {
                            singular_quadrature = dynamic_cast<Quadrature<dim-1> *>(
                                & sing_quadratures_3d[singular_index]);
                        }

                        Assert(singular_quadrature, ExcInternalError());
                        
                        FEValues<dim-1,dim> fe_v_singular (fe, *singular_quadrature, 
                                                           update_jacobians |
                                                           update_values |
                                                           update_cell_normal_vectors |
                                                           update_quadrature_points );

                        fe_v_singular.reinit(cell);
                    
                        std::vector<Vector<double> > singular_cell_wind( (*singular_quadrature).size(), 
									 Vector<double>(dim) );
        
                        const std::vector<Point<dim> > &singular_normals = fe_v_singular.get_cell_normal_vectors();
                        const std::vector<Point<dim> > &singular_q_points = fe_v_singular.get_quadrature_points();
        
                        wind.vector_value_list(singular_q_points, singular_cell_wind);
                    
                        for(unsigned int q=0; q<singular_quadrature->size(); ++q) {
                            R = singular_q_points[q]- support_points[i];
                            double normal_wind = 0;
                            for(unsigned int d=0; d<dim; ++d)
                                normal_wind += (singular_cell_wind[q](d)*
                                                singular_normals[q][d]);
                        
                            system_rhs(i) += ( LaplaceKernel::single_layer(R, is_singular) *
                                               normal_wind                         *
                                               fe_v_singular.JxW(q) );
                        
                            for(unsigned int j=0; j<fe.dofs_per_cell; ++j) {
                                local_matrix_row_i(j) += (( LaplaceKernel::double_layer(R, is_singular) *
							    singular_normals[q])                *
							  fe_v_singular.shape_value(j,q)        *
							  fe_v_singular.JxW(q)       );
                            }
                        }
                        if(dim==2) {
                            delete singular_quadrature;
                        }
            }
            
            // Move the local matrix row to the global matrix.
            for(unsigned int j=0; j<fe.dofs_per_cell; ++j) 
                system_matrix.add(i,dofs[j], local_matrix_row_i(j));
        }
    }
    // One quick way to compute the diagonal matrix of the solid
    // angles, is to use the Neumann matrix itself. It is enough to
    // multiply the matrix with the vector of ones, to get the
    // diagonal matrix of the alpha solid angles.
    Vector<double> ones(dh.n_dofs());
    for(unsigned int i=0; i<dh.n_dofs(); ++i) 
        ones(i) = -1.;
    system_matrix.vmult(alpha, ones);
    for(unsigned int i = 0; i<dh.n_dofs(); ++i) {
        system_matrix.add(i,i,alpha(i));
    }
}

template <int dim>
void BEMProblem<dim>::solve_system() {
    SparseDirectUMFPACK LU;
    LU.initialize (system_matrix);
    LU.vmult (phi, system_rhs);

//TODO: is this true? it seems to me that the BIE is definite...    
    // Since we are solving a purely Neumann problem, the solution is
    // only known up to a constant potential. We solve this issue by
    // subtracting the mean value of the vector from each vector
    // entry.
    phi.add(-phi.mean_value());
}



template <int dim>
void BEMProblem<dim>::compute_errors(const unsigned int cycle) {
    // The computation of the errors is exactly the same in all other
    // example programs, and we won't comment too much. Notice how the
    // same methods that are used in the finite element methods can be
    // used here.
    
    Vector<float> difference_per_cell (tria.n_active_cells());
    VectorTools::integrate_difference (dh, phi,
				       exact_solution,
				       difference_per_cell,
				       QGauss<(dim-1)>(3),
				       VectorTools::L2_norm);
    const double L2_error = difference_per_cell.l2_norm();

    
    // The error in the alpha vector can be computed directly using
    // the linfty_norm method of Vector<double>, since on each node,
    // the value should be $\frac 12$.
    Vector<double> difference_per_node(alpha);
    difference_per_node.add(-.5);
    
    const double alpha_error = difference_per_node.linfty_norm();
    const unsigned int n_active_cells=tria.n_active_cells();
    const unsigned int n_dofs=dh.n_dofs();
    
    deallog << "Cycle " << cycle << ':' 
	    << std::endl
	    << "   Number of active cells:       "
	    << n_active_cells
	    << std::endl
	    << "   Number of degrees of freedom: "
	    << n_dofs
	    << std::endl;
    
    convergence_table.add_value("cycle", cycle);
    convergence_table.add_value("cells", n_active_cells);
    convergence_table.add_value("dofs", n_dofs);
    convergence_table.add_value("L2(phi)", L2_error);
    convergence_table.add_value("Linfty(alpha)", alpha_error);
}

// We assume here that the boundary element domain is contained in the
// box $[-2,2]^{\text{dim}}$, and we extrapolate the actual solution
// inside this box using the convolution with the fundamental solution.
template <int dim>
void BEMProblem<dim>::compute_exterior_solution() {
    // Generate the mesh, refine it and distribute dofs on it.
    GridGenerator::hyper_cube(external_tria, -2, 2);
    external_tria.refine_global(external_refinement);
    external_dh.distribute_dofs(external_fe);
    external_phi.reinit(external_dh.n_dofs());
    
    typename DoFHandler<dim-1,dim>::active_cell_iterator
        cell = dh.begin_active(),
        endc = dh.end();


    FEValues<dim-1,dim> fe_v(fe, *quadrature,
                             update_values |
                             update_cell_normal_vectors |
                             update_quadrature_points |
                             update_JxW_values);
    
    const unsigned int n_q_points = fe_v.n_quadrature_points;
    
    std::vector<unsigned int> dofs(fe.dofs_per_cell);
    
    std::vector<double> local_phi(n_q_points);
    std::vector<double> normal_wind(n_q_points);
    std::vector<Vector<double> > local_wind(n_q_points, Vector<double>(dim) );
    
    Point<dim> R;


    typename DoFHandler<dim>::active_cell_iterator
        external_cell = external_dh.begin_active(),
        external_endc = external_dh.end();

    std::vector<Point<dim> > external_support_points(external_dh.n_dofs());
    DoFTools::map_dofs_to_support_points<dim>( StaticMappingQ1<dim>::mapping,
                                               external_dh, external_support_points);
    
    for(cell = dh.begin_active(); cell != endc; ++cell) {
        fe_v.reinit(cell);
                    
        const std::vector<Point<dim> > &q_points = fe_v.get_quadrature_points();
        const std::vector<Point<dim> > &normals = fe_v.get_cell_normal_vectors();
        
        cell->get_dof_indices(dofs);
        fe_v.get_function_values(phi, local_phi);
        
        wind.vector_value_list(q_points, local_wind);
        
        for(unsigned int q=0; q<n_q_points; ++q){
            normal_wind[q] = 0;
            for(unsigned int d=0; d<dim; ++d) 
                normal_wind[q] += normals[q][d]*local_wind[q](d);
        }
            
        for(unsigned int i=0; i<external_dh.n_dofs(); ++i) {
            
            for(unsigned int q=0; q<n_q_points; ++q) {
                
                R =  q_points[q] - external_support_points[i];
                        
                external_phi(i) += ( ( LaplaceKernel::single_layer(R)   * 
                                       normal_wind[q]   +
                                       //
                                       (LaplaceKernel::double_layer(R)        * 
                                        normals[q] )            *
                                       local_phi[q] )           *
                                     fe_v.JxW(q) );
            }
        }
    }
    DataOut<dim, DoFHandler<dim> > dataout;
    
    dataout.attach_dof_handler(external_dh);
    dataout.add_data_vector(external_phi, "external_phi");
    dataout.build_patches();
    
    std::string filename = Utilities::int_to_string(dim) + "d_external.vtk";
    std::ofstream file(filename.c_str());
    dataout.write_vtk(file);
}


template <int dim>
void BEMProblem<dim>::output_results(unsigned int cycle) {
    
    DataOut<dim-1, DoFHandler<dim-1, dim> > dataout;
    
    dataout.attach_dof_handler(dh);
    dataout.add_data_vector(phi, "phi");
    dataout.add_data_vector(alpha, "alpha");
    dataout.build_patches();
    
    std::string filename = ( Utilities::int_to_string(dim) + 
                             "d_boundary_solution_" +
                             Utilities::int_to_string(cycle) +
                             ".vtk" );
    std::ofstream file(filename.c_str());
    
    dataout.write_vtk(file);

    convergence_table.set_precision("L2(phi)", 3);
    convergence_table.set_precision("Linfty(alpha)", 3);
    
    convergence_table.set_scientific("L2(phi)", true);
    convergence_table.set_scientific("Linfty(alpha)", true);
    
    if(cycle == n_cycles-1) {
	convergence_table
	    .evaluate_convergence_rates("L2(phi)", ConvergenceTable::reduction_rate_log2);
	convergence_table
	    .evaluate_convergence_rates("Linfty(alpha)", ConvergenceTable::reduction_rate_log2);
	deallog << std::endl;
	convergence_table.write_text(std::cout);
    }
}

template <int dim>
void BEMProblem<dim>::run() {
    
    read_parameters("parameters.prm");
    if(run_in_this_dimension == true) {
        read_domain();
        
        for(unsigned int cycle=0; cycle<n_cycles; ++cycle) {
            refine_and_resize();
            assemble_system();
            solve_system();
	    compute_errors(cycle);
            output_results(cycle);
        }
        if(extend_solution == true)
            compute_exterior_solution();
    } else {
	deallog << "Run in dimension " << dim 
                << " explicitly disabled in parameter file. " 
                << std::endl;
    }
}


int main () 
{
  try
  {
      deallog.depth_console (3);
      BEMProblem<2> laplace_problem_2d;
      laplace_problem_2d.run();

      BEMProblem<3> laplace_problem_3d;      
      laplace_problem_3d.run();
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
    }

  return 0;
}
