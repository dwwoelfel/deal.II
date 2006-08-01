//----------------------------  rt_approximation_01.cc  ---------------------------
//    rt_approximation_01.cc,v 1.3 2003/06/09 16:00:38 wolf Exp
//    Version: 
//
//    Copyright (C) 2003, 2005, 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  rt_approximation_01.cc  ---------------------------

/*
 * Little snippet, which tests, to what degree the RT/ABF elements
 * can approximate a polynomial exactly on deformed geometries.
 */



#include "../tests.h"
#include <base/logstream.h>

#define PRECISION 2

#include <fstream>

std::ofstream logfile ("rt_approximation_01/output");

char buf[1000];

#include <fstream>

#include <grid/grid_generator.h>
#include <grid/grid_out.h>
#include <fe/mapping_q.h>
#include <fe/mapping_q1_eulerian.h>

#include <numerics/vectors.h>
#include <numerics/matrices.h>
#include <numerics/data_out.h>
#include <lac/vector.h>
#include <lac/solver_cg.h>
#include <lac/precondition.h>
#include <lac/vector_memory.h>
#include <lac/sparsity_pattern.h>
#include <lac/sparse_matrix.h>

#include <base/function.h>
#include <base/quadrature_lib.h>
#include <dofs/dof_constraints.h>
#include <dofs/dof_tools.h>

#include <fe/fe_system.h>
#include <fe/fe.h>
#include <fe/fe_system.h>
#include <fe/fe_q.h>
#include <fe/fe_values.h>
#include <fe/fe_raviart_thomas.h>
#include <fe/fe_abf.h>
#include <fe/fe_dgq.h>
#include <fe/mapping_q1_eulerian.h>

#include <fstream>


template <int dim>
class TestMap1 : public Function<dim>
{
  public:
    TestMap1 (const unsigned int n_components) :
		    Function<dim> (n_components)
      {};
  
    virtual ~TestMap1 () {};
  
    virtual double value (const Point<dim>   &p,
			  const unsigned int  component = 0) const;
  
    void vector_value (const Point<dim> &p,
		       Vector<double>   &return_value) const;
};



template <int dim>
double
TestMap1<dim>::value (const Point<dim>   &p,
		      const unsigned int  component) const 
{
				   // u = x^2, v = y^2, dudx = 2 x, dvdy = 2 y, div u = 2x + 2y
				   // I.e. \int div u = 2 (for unit square)

  if (component == 0)
    return (p(0) * p(0));
  if (component == 1)
    return (p(1) * p(1));

  return (0);
}


template <int dim>
void TestMap1<dim>::vector_value (const Point<dim> &p,
				  Vector<double>   &return_value) const
{
  Assert (return_value.size() == this->n_components,
	  ExcDimensionMismatch (return_value.size(), this->n_components));
  
				   // Just fill the vector with the appropriate components
  for (unsigned int iCount = 0; iCount < this->n_components; iCount++)
    return_value (iCount) = value (p, iCount);
}



///-----------------------------------------------------------------------

template <int dim>
class TestDef1 : public Function<dim>
{
  private:
    const double phi;

  public:
    TestDef1 (const unsigned int n_components, const double ph) :
		    Function<dim> (n_components),
		    phi (ph)
      {};
    
    virtual ~TestDef1 () {};
    
    virtual double value (const Point<dim>   &p,
			  const unsigned int  component = 0) const;
    
    void vector_value (const Point<dim> &p,
		       Vector<double>   &return_value) const;
};



template <int dim>
double
TestDef1<dim>::value (const Point<dim>   &p,
		      const unsigned int  component) const 
{
  Point<2> center;
  center(0) = 0.5;
  center(1) = 0.5;
  double rad = p.distance (center),
       phi_p = atan2 (p(0) - center(0), p(1) - center(1));

  if (component == 0)
    return rad * (sin (phi + phi_p) - sin (phi_p));
  else
    return rad * (cos (phi + phi_p) - cos (phi_p));
}


template <int dim>
void TestDef1<dim>::vector_value (const Point<dim> &p,
				  Vector<double>   &return_value) const
{
  Assert (return_value.size() == this->n_components,
	  ExcDimensionMismatch (return_value.size(), this->n_components));
  for (unsigned int iCount = 0; iCount < this->n_components; iCount++)
    return_value (iCount) = value (p, iCount);
}


///-----------------------------------------------------------------------


template <int dim>
class TestDef2 : public Function<dim>
{
  private:
    const double scale;

  public:
    TestDef2 (const unsigned int n_components, const double sc) :
		    Function<dim> (n_components),
		    scale (sc)
      {};
    
    virtual ~TestDef2 () {};
    
    virtual double value (const Point<dim>   &p,
			  const unsigned int  component = 0) const;
    
    void vector_value (const Point<dim> &p,
		       Vector<double>   &return_value) const;
};


template <int dim>
double
TestDef2<dim>::value (const Point<dim>   &p,
		      const unsigned int  component) const 
{
  double x = p(0),
	 y = p(1);

  if (component == 0)
    return scale * x;
  else
    return scale * y;
}


template <int dim>
void TestDef2<dim>::vector_value (const Point<dim> &p,
				  Vector<double>   &return_value) const
{
  Assert (return_value.size() == this->n_components,
	  ExcDimensionMismatch (return_value.size(), this->n_components));
  for (unsigned int iCount = 0; iCount < this->n_components; iCount++)
    return_value (iCount) = value (p, iCount);
}


///-----------------------------------------------------------------------
// testDef3 implements parallelograms ...


template <int dim>
class TestDef3 : public Function<dim>
{
  private:
    const double scale;

  public:
    TestDef3 (const unsigned int n_components, const double sc) :
		    Function<dim> (n_components),
		    scale (sc)
      {};
    
    virtual ~TestDef3 () {};
    
    virtual double value (const Point<dim>   &p,
			  const unsigned int  component = 0) const;
    
    void vector_value (const Point<dim> &p,
		       Vector<double>   &return_value) const;
};


template <int dim>
double
TestDef3<dim>::value (const Point<dim>   &p,
		      const unsigned int  component) const 
{
  double y = p(1);

  if (component == 0)
    return scale * y;
  else
    return 0;
}


template <int dim>
void TestDef3<dim>::vector_value (const Point<dim> &p,
				  Vector<double>   &return_value) const
{
  Assert (return_value.size() == this->n_components,
	  ExcDimensionMismatch (return_value.size(), this->n_components));
  for (unsigned int iCount = 0; iCount < this->n_components; iCount++)
    return_value (iCount) = value (p, iCount);
}

// Wrapper class for polynomials. 

template <int dim>
class TestPoly : public Function<dim>
{
  private:
    std::vector<Polynomials::Polynomial<double> > polys;

  public:
    TestPoly (unsigned int deg) :
		    Function<dim> (2)
      {
	std::vector<double> coeff (deg, 0.0);
	for (unsigned int p=0; p < 4; ++p)
	  {
	    for (unsigned int i=0; i < deg; ++i)
	      {
		double c = ((double) rand ()) / ((double) RAND_MAX + 1);
		coeff[i] = c;
	      }
	    polys.push_back (Polynomials::Polynomial<double> (coeff));
	  }
      };
    
    virtual ~TestPoly () {};
    
    virtual double value (const Point<dim>   &p,
			  const unsigned int  component = 0) const;
    
    void vector_value (const Point<dim> &p,
		       Vector<double>   &return_value) const;
};


template <int dim>
double
TestPoly<dim>::value (const Point<dim>   &p,
		      const unsigned int  component) const 
{
  double x = p(0),
	 y = p(1);

				   // Ugly hack, but should do the job ...
  if (component == 0)
    return polys[0].value (x) + polys[1].value (y);
  else
    return polys[2].value (x) + polys[3].value (y);
}


template <int dim>
void TestPoly<dim>::vector_value (const Point<dim> &p,
				  Vector<double>   &return_value) const
{
  Assert (return_value.size() == this->n_components,
	  ExcDimensionMismatch (return_value.size(), this->n_components));
  for (unsigned int iCount = 0; iCount < this->n_components; iCount++)
    return_value (iCount) = value (p, iCount);
}



/*
 * Check the value of the derivative field.
 */

double TestProjection (Mapping<2> &mapping,
		       DoFHandler<2> *dof_handler)
{
  Vector<double> solution;
  solution.reinit (dof_handler->n_dofs ());

				   // Create and Project test function to H_div space and evaluate the error
				   // If the error is in the range of machine precision, this polynomial
				   // degree can obviously be represented by projected field.

  for (unsigned int deg = 1; deg < 4; ++deg)
    {
      TestPoly<2> pol (deg);

				       // Project solution onto RT FE field
      ConstraintMatrix     hn_constraints;
      hn_constraints.clear ();
      DoFTools::make_hanging_node_constraints (*dof_handler, 
					       hn_constraints);
      hn_constraints.close ();
      VectorTools::project (mapping, *dof_handler, hn_constraints,
			    QGauss6<2> (), pol,
			    solution);

				       // Now evaluate error ...
				       // Use a high order quadrature. 
      QGauss<2> quad (6);
      FEValues<2> fe_values (mapping, dof_handler->get_fe (), quad, 
			     UpdateFlags(update_values    |
					 update_q_points  |
					 update_gradients |
					 update_JxW_values|
					 update_contravariant_transformation));

      const unsigned int   n_q_points    = quad.n_quadrature_points;
      const unsigned int   n_components   = dof_handler->get_fe().n_components();
      
				       // Cell iterators
      DoFHandler<2>::active_cell_iterator cell = dof_handler->begin_active(),
					  endc = dof_handler->end();

      double err_u = 0,
	     err_v = 0;

      for (; cell!=endc; ++cell)
	{
	  fe_values.reinit (cell);
	  
					   // Get values from solution vector (For Trap.Rule)
	  std::vector<Vector<double> > this_value
	    (n_q_points, Vector<double>(n_components));
	  fe_values.get_function_values (solution, this_value);
	  
	  for (unsigned int q_point=0; q_point<n_q_points; ++q_point)
	    {
	      double u = this_value[q_point](0),
		     v = this_value[q_point](1);
	      Point<2> p = fe_values.quadrature_point (q_point);

	      double u_ex = pol.value (p, 0),
		     v_ex = pol.value (p, 1);

	      double JxW = fe_values.JxW (q_point);

	      err_u += (u - u_ex) * (u - u_ex) * JxW;
	      err_v += (v - v_ex) * (v - v_ex) * JxW;
	    }
	}

      sprintf (buf, "Deg %i  ErrU %e  ErrV %e\n", deg, err_u, err_v);
      deallog << buf;
    }


				   // Test the core functionality
  DataOut<2> *data_out = new DataOut<2>;
  data_out->attach_dof_handler (*dof_handler);
  data_out->add_data_vector (solution, "solution");
  data_out->build_patches (mapping, 4);

  const char *szFName = "solu.gpl";
  data_out->write_gnuplot (deallog.get_file_stream());

  delete data_out;

  return (0.0);
}


int main ()
{
  logfile.precision (PRECISION);
  logfile.setf(std::ios::fixed);  
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  Triangulation<2> tria_test;
  DoFHandler<2> *dof_handler,
    *dof_handler_def;
  Point<2> p1 (0,0),
    p2 (1, 1);

  GridGenerator::hyper_rectangle (tria_test, p1, p2);
				   //  tria_test.refine_global (1);
				   //  tria_test.distort_random (0.4);

				   // Create a DoFHandler for the RT space
  FE_RaviartThomas<2> fe (1);
				   //FE_ABF<2> fe (0);
  dof_handler = new DoFHandler<2> (tria_test);
  dof_handler->distribute_dofs (fe);

  QGauss6<2> quad_temp;
  sprintf (buf, "DoFs per Quad: %i per Line %i per Vert %i\n", fe.dofs_per_quad, fe.dofs_per_line, fe.dofs_per_vertex);
  deallog << buf;
  sprintf (buf, "QPoints %i\n", quad_temp.n_quadrature_points);
  deallog << buf;

				   // Create an deformation object for the Eulerian mapping
  FESystem<2> fe_def (FE_Q<2>(1), 2);
  dof_handler_def = new DoFHandler<2> (tria_test);
  dof_handler_def->distribute_dofs (fe_def);

				   // Alloc some DoFs
  Vector<double> solution,
    solution_q,
    deformation;
  solution.reinit (dof_handler->n_dofs ());
  solution_q.reinit (dof_handler->n_dofs ());
  deformation.reinit (dof_handler_def->n_dofs ());

  ConstraintMatrix     hn_constraints_def;
  hn_constraints_def.clear ();
  DoFTools::make_hanging_node_constraints (*dof_handler_def, 
					   hn_constraints_def);
  hn_constraints_def.close ();
  
  {
    MappingQ1Eulerian<2> mapping_euler (deformation, *dof_handler_def);

				     // Try rotating the elements
    for (double rotat = 0; rotat < 2 * M_PI; rotat += 0.25 * M_PI)
      {
					 // Rotate element
	VectorTools::project (*dof_handler_def, hn_constraints_def,
			      QGauss6<2> (), TestDef1<2>(2, rotat),
			      deformation);
	sprintf (buf, "phi = %e\n", rotat);
	deallog << buf;
	TestProjection (mapping_euler, dof_handler);
      }

				     // Try resizing the elements
    for (double scale = -0.75; scale < 4.0; scale += 0.25)
      {
	VectorTools::project (*dof_handler_def, hn_constraints_def,
			      QGauss6<2> (), TestDef2<2>(2, scale),
			      deformation);
	sprintf (buf, "Scale = %e\n", scale);
	deallog << buf;
	TestProjection (mapping_euler, dof_handler);
      }

				     // Try paralellogramming the elements
    for (double scale = -1.0; scale < 1.0; scale += 0.25)
      {
	VectorTools::project (*dof_handler_def, hn_constraints_def,
			      QGauss6<2> (), TestDef3<2>(2, scale),
			      deformation);
	sprintf (buf, "Scale = %e\n", scale);
	deallog << buf;
	TestProjection (mapping_euler, dof_handler);
      }

				     // Try arbitrary deformation ...
    for (unsigned int i=0; i < deformation.size (); ++i)
      {
	double c = ((double) rand ()) / ((double) RAND_MAX + 1);
	deformation (i) = 0.35 * (c - 0.5);
      }

    sprintf (buf, "Arbitrary\n");
    deallog << buf;
    TestProjection (mapping_euler, dof_handler);
  }
  
  delete (dof_handler);
  delete (dof_handler_def);

  return (0);
}
