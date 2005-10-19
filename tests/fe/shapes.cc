// shapes.cc,v 1.18 2003/04/09 15:49:55 wolf Exp
// (c) Guido Kanschat
//
// Show the shape functions implemented.

#include "../tests.h"
#include <base/quadrature_lib.h>
#include <base/logstream.h>
#include <lac/vector.h>
#include <grid/tria.h>
#include <grid/tria_iterator.h>
#include <dofs/dof_accessor.h>
#include <grid/grid_generator.h>
#include <fe/fe_q.h>
#include <fe/fe_q_hierarchical.h>
#include <fe/fe_dgp.h>
#include <fe/fe_dgp_monomial.h>
#include <fe/fe_dgp_nonparametric.h>
#include <fe/fe_dgq.h>
#include <fe/fe_nedelec.h>
#include <fe/fe_system.h>
#include <fe/mapping_q1.h>
#include <fe/fe_values.h>
#include <vector>
#include <fstream>
#include <string>

#define PRECISION 2

char fname[50];

////////////////////////////////////////////////////////////////////////////
// Plot shape function values at quadrature points inside the cell [0,1]^d
//
// Output values in each line are
//
// x (y) (z) value[0]+1 value[1]+1 ...
////////////////////////////////////////////////////////////////////////////
template<int dim>
inline void
plot_shape_functions(Mapping<dim>& mapping,
		     FiniteElement<dim>& finel,
		     const char* name)
{
  Triangulation<dim> tr;
  DoFHandler<dim> dof(tr);
  GridGenerator::hyper_cube(tr, 0., 1.);
  dof.distribute_dofs(finel);
  typename DoFHandler<dim>::cell_iterator c = dof.begin();

  const unsigned int div = 11;

  QTrapez<1> q_trapez;
  QIterated<dim> q(q_trapez, div);
  FEValues<dim> fe(mapping, finel, q, UpdateFlags(update_values
						  | update_gradients
						  | update_second_derivatives));

  sprintf(fname, "Cell%dd-%s", dim, name);
//  cerr << "\n" << fname << "\n";
  deallog.push(fname);

  fe.reinit(c);
  
  unsigned int k=0;
  for (unsigned int mz=0;mz<=((dim>2) ? div : 0) ;++mz)
    {
      for (unsigned int my=0;my<=((dim>1) ? div : 0) ;++my)
	{
	  for (unsigned int mx=0;mx<=div;++mx)
	    {
	      deallog << q.point(k);
	  
	      for (unsigned int i=0;i<finel.dofs_per_cell;++i)
		{
		  deallog << " " << fe.shape_value(i,k) + 1.;

						   // some additional
						   // checks
		  for (unsigned int c=0; c<fe.get_fe().n_components(); ++c)
		    {
		      if (fe.get_fe().system_to_component_index(i).first == c)
			Assert ((fe.shape_value(i,k) ==
				 fe.shape_value_component(i,k,c))
				&&
				(fe.shape_grad(i,k) ==
				 fe.shape_grad_component(i,k,c))
				&&
				(fe.shape_2nd_derivative(i,k) ==
				 fe.shape_2nd_derivative_component(i,k,c)),
				ExcInternalError())
		      else
			Assert ((fe.shape_value_component(i,k,c) == 0) &&
				(fe.shape_grad_component(i,k,c) == Tensor<1,dim>()) &&
				(fe.shape_2nd_derivative_component(i,k,c) == Tensor<2,dim>()),
				ExcInternalError());
		    };
		}
	      deallog << std::endl;
	      k++;
	    }
	  deallog << std::endl;
	}
      deallog << std::endl;
    }
  deallog.pop();
}



template<int dim>
inline void
plot_face_shape_functions(Mapping<dim>& mapping,
			  FiniteElement<dim>& finel,
			  const char* name)
{
  Triangulation<dim> tr;
  DoFHandler<dim> dof(tr);
  GridGenerator::hyper_cube(tr, 0., 1.);
  tr.refine_global(1);
  typename DoFHandler<dim>::active_cell_iterator c = dof.begin_active();
  ++c;
  c->set_refine_flag();
  tr.execute_coarsening_and_refinement ();
  c = dof.begin_active();

  dof.distribute_dofs(finel);

  const unsigned int div = 4;

  QTrapez<1> q_trapez;
  QIterated<dim-1> q(q_trapez, div);
  FEFaceValues<dim> fe(mapping, finel, q, UpdateFlags(update_values
						      | update_gradients
						      | update_second_derivatives
						      | update_q_points));
  FESubfaceValues<dim> sub(mapping, finel, q, UpdateFlags(update_values
							  | update_gradients
							  | update_second_derivatives
							  | update_q_points));

  sprintf(fname, "Face%dd-%s", dim, name);
  deallog.push(fname);
  
  for (unsigned int f=0; f<GeometryInfo<dim>::faces_per_cell; ++f)
    {
      if (!c->face(f)->has_children())
	{
	  fe.reinit(c, f);
  
	  unsigned int k=0;
	  for (unsigned int my=0;my<=((dim>2) ? div : 0) ;++my)
	    {
	      for (unsigned int mx=0;mx<=div;++mx)
		{
		  deallog << fe.quadrature_point(k);
		  
		  for (unsigned int i=0;i<finel.dofs_per_cell;++i)
		    {
		      deallog << " " << fe.shape_value(i,k) + 1.;

						       // some additional
						       // checks
		      for (unsigned int c=0; c<fe.get_fe().n_components(); ++c)
			{
			  if (fe.get_fe().system_to_component_index(i).first == c)
			    Assert ((fe.shape_value(i,k) ==
				     fe.shape_value_component(i,k,c))
				    &&
				    (fe.shape_grad(i,k) ==
				     fe.shape_grad_component(i,k,c))
				    &&
				    (fe.shape_2nd_derivative(i,k) ==
				     fe.shape_2nd_derivative_component(i,k,c)),
				    ExcInternalError())
			  else
			    Assert ((fe.shape_value_component(i,k,c) == 0) &&
				    (fe.shape_grad_component(i,k,c) == Tensor<1,dim>()) &&
				    (fe.shape_2nd_derivative_component(i,k,c) == Tensor<2,dim>()),
				    ExcInternalError());
			};
		    }
		  deallog << std::endl;
		  k++;
		}
	      deallog << std::endl;
	    }
	  deallog << std::endl;
	} else {
	  for (unsigned int s=0;s<GeometryInfo<dim>::subfaces_per_face; ++s)
	    {
	      sub.reinit(c, f, s);
	      
	      unsigned int k=0;
	      for (unsigned int my=0;my<=((dim>2) ? div : 0) ;++my)
		{
		  for (unsigned int mx=0;mx<=div;++mx)
		    {
		      deallog << sub.quadrature_point(k);
		      
		      for (unsigned int i=0;i<finel.dofs_per_cell;++i)
			{
			  deallog << " " << sub.shape_value(i,k) + 1.;

							   // some additional
							   // checks
			  for (unsigned int c=0; c<fe.get_fe().n_components(); ++c)
			    {
			      if (fe.get_fe().system_to_component_index(i).first == c)
				{
				  const double v1 = sub.shape_value(i,k),
					       v2 = sub.shape_value_component(i,k,c);
				  Assert (v1 == v2, ExcInternalError());

				  const Tensor<1,dim> g1 = sub.shape_grad(i,k),
						      g2 = sub.shape_grad_component(i,k,c);
				  Assert (g1 == g2, ExcInternalError());

				  const Tensor<2,dim> s1 = sub.shape_2nd_derivative(i,k),
						      s2 = sub.shape_2nd_derivative_component(i,k,c);
				  Assert (s1 == s2, ExcInternalError());
				}       
			      else
				Assert ((sub.shape_value_component(i,k,c) == 0) &&
					(sub.shape_grad_component(i,k,c) == Tensor<1,dim>()) &&
					(sub.shape_2nd_derivative_component(i,k,c) == Tensor<2,dim>()),
					ExcInternalError());
			    };
			}
		      deallog << std::endl;
		      k++;
		    }
		  deallog << std::endl;
		}
	      deallog << std::endl;	      
	    }
	}
    }
  deallog.pop();
}


template<>
void plot_face_shape_functions (Mapping<1>&,
				FiniteElement<1>&,
				const char*)
{}



// given an FEValues object for a cell that is equal to the unit cell,
// check that the values and gradients that the FEValues object
// generated are equivalent to the values the finite element returns
// for the unit cell
template <int dim>
void
check_values_and_derivatives (const FiniteElement<dim> &fe,
                              const FEValuesBase<dim>  &fe_values,
                              const Quadrature<dim>    &q)
{
                                   // check values
  for (unsigned int x=0; x<q.n_quadrature_points; ++x)
    for (unsigned int i=0; i<fe.dofs_per_cell; ++i)
      {
        for (unsigned int c=0; c<fe.n_components(); ++c)
          {
            const double val1 = fe_values.shape_value_component(i,x,c),
                         val2 = fe.shape_value_component(i,q.point(x),c);
            Assert (std::fabs(val1-val2) < 1e-13, ExcInternalError());
          };

                                         // test something about the
                                         // correctness of indices
                                         // etc, except for the more
                                         // complicated case of
                                         // non-primitive elements
        if (fe.is_primitive(i))
          for (unsigned int c=0; c<fe.n_components(); ++c)
            Assert (((c == fe.system_to_component_index(i).first) &&
                     (fe_values.shape_value(i,x) == fe_values.shape_value_component(i,x,c)))
                    ||
                    ((c != fe.system_to_component_index(i).first) &&
                     (fe_values.shape_value_component(i,x,c) == 0)),
                    ExcInternalError());
      };
        
                                   // check gradients
  for (unsigned int x=0; x<q.n_quadrature_points; ++x)
    for (unsigned int i=0; i<fe.dofs_per_cell; ++i)
      {
        for (unsigned int c=0; c<fe.n_components(); ++c)
          {
            Tensor<1,dim> tmp=fe_values.shape_grad_component(i,x,c);
            tmp -= fe.shape_grad_component (i,q.point(x), c);
            Assert (std::sqrt(tmp*tmp)<1e-14, ExcInternalError());
          };

        if (fe.is_primitive(i))
          for (unsigned int c=0; c<fe.n_components(); ++c)
            Assert (((c == fe.system_to_component_index(i).first) &&
                     (fe_values.shape_grad(i,x) == fe_values.shape_grad_component(i,x,c)))
                    ||
                    ((c != fe.system_to_component_index(i).first) &&
                     (fe_values.shape_grad_component(i,x,c) == Tensor<1,dim>())),
                    ExcInternalError());
      }

                                   // check second derivatives
  double max_diff=0.;
  for (unsigned int x=0; x<q.n_quadrature_points; ++x)
    for (unsigned int i=0; i<fe.dofs_per_cell; ++i)
      {
        for (unsigned int c=0; c<fe.n_components(); ++c)
          {
            Tensor<2,dim> tmp=fe_values.shape_2nd_derivative_component(i,x,c);
            tmp -= fe.shape_grad_grad_component(i,q.point(x),c);
            for (unsigned int j=0; j<dim; ++j)
              for (unsigned int k=0; k<dim; ++k)
                {
                  const double diff=std::fabs(tmp[j][k]);
                  if (diff>max_diff) max_diff=diff;
                  Assert (std::fabs(tmp[j][k]) < 1e-6, ExcInternalError());
                }
          };

        if (fe.is_primitive(i))
          for (unsigned int c=0; c<fe.n_components(); ++c)
            Assert (((c == fe.system_to_component_index(i).first) &&
                     (fe_values.shape_2nd_derivative(i,x) ==
                      fe_values.shape_2nd_derivative_component(i,x,c)))
                    ||
                    ((c != fe.system_to_component_index(i).first) &&
                     (fe_values.shape_2nd_derivative_component(i,x,c) == Tensor<2,dim>())),
                    ExcInternalError());
      }
}



template<int dim>
void test_compute_functions (const Mapping<dim> &mapping,
			     const FiniteElement<dim> &fe,
			     const char*)
{
                                   // generate a grid with only one
                                   // cell, which furthermore has the
                                   // shape of the unit cell. then the
                                   // values/gradients/... we get from
                                   // the FEValues object on this cell
                                   // should really be equal to what
                                   // we get from the finite element
                                   // itself on the unit cell:
  Triangulation<dim> tr;
  DoFHandler<dim> dof(tr);
  GridGenerator::hyper_cube(tr, 0., 1.);
  dof.distribute_dofs(fe);

  const UpdateFlags update_all = (update_values | update_gradients |
                                  update_second_derivatives);
  
                                   // first check this for FEValues
                                   // objects
  if (true)
    {
      const QGauss6<dim> q;
      FEValues<dim> fe_values(mapping, fe, q, update_all);
      fe_values.reinit(dof.begin_active());
      check_values_and_derivatives (fe, fe_values, q);
    };
}





template<int dim>
void plot_FE_Q_shape_functions()
{
  MappingQ1<dim> m;

  FE_Q<dim> q1(1);
  plot_shape_functions(m, q1, "Q1");
  plot_face_shape_functions(m, q1, "Q1");
  test_compute_functions(m, q1, "Q1");

  FE_Q<dim> q2(2);
  plot_shape_functions(m, q2, "Q2");
  plot_face_shape_functions(m, q2, "Q2");
  test_compute_functions(m, q2, "Q2");

				   // skip the following tests to
				   // reduce run-time
  if (dim < 3)
    {
      FE_Q<dim> q3(3);
      plot_shape_functions(m, q3, "Q3");
      plot_face_shape_functions(m, q3, "Q3");
      test_compute_functions(m, q3, "Q3");

      FE_Q<dim> q4(4);
      plot_shape_functions(m, q4, "Q4");
      plot_face_shape_functions(m, q4, "Q4");
      test_compute_functions(m, q4, "Q4");
    };
  
//    FE_Q<dim> q5(5);
//    plot_shape_functions(m, q5, "Q5");
//    FE_Q<dim> q6(6);
//    plot_shape_functions(m, q6, "Q6");
//    FE_Q<dim> q7(7);
//    plot_shape_functions(m, q7, "Q7");
//    FE_Q<dim> q8(8);
//    plot_shape_functions(m, q8, "Q8");
//    FE_Q<dim> q9(9);
//    plot_shape_functions(m, q9, "Q9");
//    FE_Q<dim> q10(10);
//    plot_shape_functions(m, q10, "Q10");
}


template<int dim>
void plot_FE_Q_Hierarchical_shape_functions()
{
  MappingQ1<dim> m;

  FE_Q_Hierarchical<dim> q1(1);
  plot_shape_functions(m, q1, "QHierarchical1");
  plot_face_shape_functions(m, q1, "QHierarchical1");
  test_compute_functions(m, q1, "QHierarchical1");

  FE_Q_Hierarchical<dim> q2(2);
  plot_shape_functions(m, q2, "QHierarchical2");
  plot_face_shape_functions(m, q2, "QHierarchical2");
  test_compute_functions(m, q2, "QHierarchical2");

				   // skip the following tests to
				   // reduce run-time
  if (dim < 3)
    {
      FE_Q_Hierarchical<dim> q3(3);
      plot_shape_functions(m, q3, "QHierarchical3");
      plot_face_shape_functions(m, q3, "QHierarchical3");
      test_compute_functions(m, q3, "QHierarchical3");

      FE_Q_Hierarchical<dim> q4(4);
      plot_shape_functions(m, q4, "QHierarchical4");
      plot_face_shape_functions(m, q4, "QHierarchical4");
      test_compute_functions(m, q4, "QHierarchical4");
    }
}



template<int dim>
void plot_FE_DGQ_shape_functions()
{
  MappingQ1<dim> m;
  
  FE_DGQ<dim> q1(1);
  plot_shape_functions(m, q1, "DGQ1");
  plot_face_shape_functions(m, q1, "DGQ1");
  test_compute_functions(m, q1, "DGQ1");

  FE_DGQ<dim> q2(2);
  plot_shape_functions(m, q2, "DGQ2");
  plot_face_shape_functions(m, q2, "DGQ2");
  test_compute_functions(m, q2, "DGQ2");

  FE_DGQ<dim> q3(3);
  plot_shape_functions(m, q3, "DGQ3");
  plot_face_shape_functions(m, q3, "DGQ3");
  test_compute_functions(m, q3, "DGQ3");
  
  QGaussLobatto<1> quadrature_gl(5);
  FE_DGQ<dim> qgl(quadrature_gl);
  plot_shape_functions(m, qgl, "DGQGL");
  plot_face_shape_functions(m, qgl, "DGQGL");
  test_compute_functions(m, qgl, "DGQGL");
  
  QGauss<1> quadrature_g(5);
  FE_DGQ<dim> qg(quadrature_g);
  plot_shape_functions(m, qg, "DGQG");
  plot_face_shape_functions(m, qg, "DGQG");
  test_compute_functions(m, qg, "DGQG");
  
//    FE_DGQ<dim> q4(4);
//    plot_shape_functions(m, q4, "DGQ4");
//    plot_face_shape_functions(m, q4, "DGQ4");
//    test_compute_functions(m, q4, "DGQ4");

//    FE_DGQ<dim> q5(5);
//    plot_shape_functions(m, q5, "DGQ5");
//    FE_DGQ<dim> q6(6);
//    plot_shape_functions(m, q6, "DGQ6");
//    FE_DGQ<dim> q7(7);
//    plot_shape_functions(m, q7, "DGQ7");
//    FE_DGQ<dim> q8(8);
//    plot_shape_functions(m, q8, "DGQ8");
//    FE_DGQ<dim> q9(9);
//    plot_shape_functions(m, q9, "DGQ9");
//    FE_DGQ<dim> q10(10);
//    plot_shape_functions(m, q10, "DGQ10");
}


template<int dim>
void plot_FE_DGP_shape_functions()
{
  MappingQ1<dim> m;

  FE_DGP<dim> p1(1);
  plot_shape_functions(m, p1, "DGP1");
  plot_face_shape_functions(m, p1, "DGP1");
  test_compute_functions(m, p1, "DGP1");

  FE_DGP<dim> p2(2);
  plot_shape_functions(m, p2, "DGP2");
  plot_face_shape_functions(m, p2, "DGP2");
  test_compute_functions(m, p2, "DGP2");
      
  FE_DGP<dim> p3(3);
  plot_shape_functions(m, p3, "DGP3");
  plot_face_shape_functions(m, p3, "DGP3");
  test_compute_functions(m, p3, "DGP3");
      
//    FE_DGP<dim> p4(4);
//    plot_shape_functions(m, p4, "DGP4");
//    plot_face_shape_functions(m, p4, "DGP4");
//    test_compute_functions(m, p4, "DGP4");

//    FE_DGP<dim> p5(5);
//    plot_shape_functions(m, p5, "DGP5");
//    FE_DGP<dim> p6(6);
//    plot_shape_functions(m, p6, "DGP6");
//    FE_DGP<dim> p7(7);
//    plot_shape_functions(m, p7, "DGP7");
//    FE_DGP<dim> p8(8);
//    plot_shape_functions(m, p8, "DGP8");
//    FE_DGP<dim> p9(9);
//    plot_shape_functions(m, p9, "DGP9");
//    FE_DGP<dim> p10(10);
//    plot_shape_functions(m, p10, "DGP10");
}


template<int dim>
void plot_FE_DGPMonomial_shape_functions()
{
  MappingQ1<dim> m;

  FE_DGPMonomial<dim> p1(1);
  plot_shape_functions(m, p1, "DGPMonomial1");
  plot_face_shape_functions(m, p1, "DGPMonomial1");
  test_compute_functions(m, p1, "DGPMonomial1");

  FE_DGPMonomial<dim> p2(2);
  plot_shape_functions(m, p2, "DGPMonomial2");
  plot_face_shape_functions(m, p2, "DGPMonomial2");
  test_compute_functions(m, p2, "DGPMonomial2");

  if (dim<3)
    {
      FE_DGPMonomial<dim> p3(3);
      plot_shape_functions(m, p3, "DGPMonomial3");
      plot_face_shape_functions(m, p3, "DGPMonomial3");
      test_compute_functions(m, p3, "DGPMonomial3");
    }
}


template<int dim>
void plot_FE_DGPNonparametric_shape_functions()
{
  MappingQ1<dim> m;

  FE_DGPNonparametric<dim> p0(0);
  plot_shape_functions(m, p0, "DGPNonparametric0");
  plot_face_shape_functions(m, p0, "DGPNonparametric0");

  FE_DGPNonparametric<dim> p1(1);
  plot_shape_functions(m, p1, "DGPNonparametric1");
  plot_face_shape_functions(m, p1, "DGPNonparametric1");

  FE_DGPNonparametric<dim> p2(2);
  plot_shape_functions(m, p2, "DGPNonparametric2");
  plot_face_shape_functions(m, p2, "DGPNonparametric2");
      
//    FE_DGPNonparametric<dim> p3(3);
//    plot_shape_functions(m, p3, "DGPNonparametric3");
//    plot_face_shape_functions(m, p3, "DGPNonparametric3");
      
//    FE_DGPNonparametric<dim> p4(4);
//    plot_shape_functions(m, p4, "DGPNonparametric4");
//    plot_face_shape_functions(m, p4, "DGPNonparametric4");
}


template<int dim>
void plot_FE_Nedelec_shape_functions()
{
  MappingQ1<dim> m;
  FE_Nedelec<dim> p1(1);
//   plot_shape_functions(m, p1, "Nedelec1");
//   plot_face_shape_functions(m, p1, "Nedelec1");
  test_compute_functions(m, p1, "Nedelec1");
}



template<int dim>
void plot_FE_System_shape_functions()
{
  MappingQ1<dim> m;

//   FESystem<dim> p1(FE_Q<dim>(2), 1,
//                    FE_Q<dim>(dim<3 ? 3 : 2), 2);
//   plot_shape_functions(m, p1, "System1");
//   plot_face_shape_functions(m, p1, "System1");
//   test_compute_functions(m, p1, "System1");

//   FESystem<dim> p2(FE_Q<dim>(2), 1,
//                    FESystem<dim> (FE_Q<dim>(1),1,
//                                   FE_DGP<dim>(3),3,
//                                   FE_DGQ<dim>(0),2), 2,
//                    FE_DGQ<dim>(0), 2);
//   plot_shape_functions(m, p2, "System2");
//   plot_face_shape_functions(m, p2, "System2");
//   test_compute_functions(m, p2, "System2");

                                   // some tests with the Nedelec
                                   // element. don't try to make sense
                                   // out of the composed elements,
                                   // they are simply constructed as
                                   // complicated as possible to
                                   // trigger as many assertions as
                                   // possible (and they _have_, in
                                   // the past, literally dozens of
                                   // assertions)
  if (dim != 1) 
    {
      FESystem<dim> p3(FE_Nedelec<dim>(1), 1,
                       FESystem<dim> (FE_Q<dim>(1),1,
                                      FE_DGP<dim>(3),3,
                                      FE_Nedelec<dim>(1),2), 2,
                       FE_DGQ<dim>(0), 2);
      test_compute_functions(m, p3, "System_Nedelec_1");

                                       // the following is simply too
                                       // expensive in 3d...
      if (dim != 3)
        {
          FESystem<dim> p4(p3, 1,
                           FESystem<dim> (FE_Q<dim>(1),1,
                                          p3,3,
                                          FE_Nedelec<dim>(1),2), 1,
                           p3, 1);
          test_compute_functions(m, p4, "System_Nedelec_2");
        };
    };
}


int
main()
{
  std::ofstream logfile ("shapes.output");
  logfile.precision (PRECISION);
  logfile.setf(std::ios::fixed);
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);
  
  plot_FE_Q_shape_functions<1>();
  plot_FE_Q_shape_functions<2>();
  plot_FE_Q_shape_functions<3>();

  plot_FE_Q_Hierarchical_shape_functions<1>();
  plot_FE_Q_Hierarchical_shape_functions<2>();
  plot_FE_Q_Hierarchical_shape_functions<3>();

  plot_FE_DGQ_shape_functions<1>();
  plot_FE_DGQ_shape_functions<2>();
  plot_FE_DGQ_shape_functions<3>();
  
  plot_FE_DGP_shape_functions<1>();
  plot_FE_DGP_shape_functions<2>();
  plot_FE_DGP_shape_functions<3>();
  
  plot_FE_DGPMonomial_shape_functions<1>();
  plot_FE_DGPMonomial_shape_functions<2>();
  plot_FE_DGPMonomial_shape_functions<3>();

  plot_FE_DGPNonparametric_shape_functions<1>();
  plot_FE_DGPNonparametric_shape_functions<2>();
  plot_FE_DGPNonparametric_shape_functions<3>();
  
  plot_FE_Nedelec_shape_functions<2>();
  plot_FE_Nedelec_shape_functions<3>();
  
  plot_FE_System_shape_functions<1>();
  plot_FE_System_shape_functions<2>();
  plot_FE_System_shape_functions<3>();
  
  return 0;
}



