// $Id$
// (c) Guido Kanschat
//
// Show the shape functions implemented.

#include <base/quadrature_lib.h>
#include <base/logstream.h>
#include <lac/vector.h>
#include <grid/tria.h>
#include <grid/tria_iterator.h>
#include <dofs/dof_accessor.h>
#include <grid/grid_generator.h>
#include <fe/fe_q.h>
#include <fe/fe_dgp.h>
#include <fe/fe_dgq.h>
#include <fe/fe_system.h>
#include <fe/mapping_q1.h>
#include <fe/fe_values.h>
#include <vector>
#include <fstream>
#include <string>

#define PRECISION 2

char fname[50];

template<int dim>
inline void
plot_shape_functions(Mapping<dim>& mapping,
		     FiniteElement<dim>& finel,
		     const char* name)
{
  Triangulation<dim> tr;
  DoFHandler<dim> dof(tr);
  GridGenerator::hyper_cube(tr, 0., 1.);
  typename DoFHandler<dim>::cell_iterator c = dof.begin();
  dof.distribute_dofs(finel);

  const unsigned int div = 11;

  QTrapez<1> q_trapez;
  QIterated<dim> q(q_trapez, div);
  FEValues<dim> fe(mapping, finel, q, UpdateFlags(update_values |
						  update_gradients |
						  update_second_derivatives));
  fe.reinit(c);
  
  sprintf(fname, "Shapes%dd-%s", dim, name);
  deallog.push(fname);

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

  sprintf(fname, "ShapesFace%dd-%s", dim, name);
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
				Assert ((sub.shape_value(i,k) ==
					 sub.shape_value_component(i,k,c))
					&&
					(sub.shape_grad(i,k) ==
					 sub.shape_grad_component(i,k,c))
					&&
					(sub.shape_2nd_derivative(i,k) ==
					 sub.shape_2nd_derivative_component(i,k,c)),
					ExcInternalError())
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


template<int dim>
void test_compute_functions (const Mapping<dim> &mapping,
			     const FiniteElement<dim> &fe,
			     const char* name)
{
  Triangulation<dim> tr;
  DoFHandler<dim> dof(tr);
  GridGenerator::hyper_cube(tr, 0., 1.);
  dof.distribute_dofs(fe);
  const QGauss6<dim> q;
  FEValues<dim> fe_values(mapping, fe, q, UpdateFlags(update_values|
						      update_gradients|
						      update_second_derivatives));
  typename DoFHandler<dim>::active_cell_iterator cell = dof.begin_active();
  fe_values.reinit(cell);

  bool coincide=true;
  for (unsigned int x=0; x<q.n_quadrature_points; ++x)
    for (unsigned int i=0; i<fe.dofs_per_cell; ++i)
      {
	if (fabs(fe_values.shape_value(i,x)-fe.shape_value(i,q.point(x)))>1e-14)
	  coincide=false;

	for (unsigned int c=0; c<fe.n_components(); ++c)
	  Assert (((c == fe.system_to_component_index(i).first) &&
		   (fe_values.shape_value(i,x) == fe_values.shape_value_component(i,x,c)))
		  ||
		  ((c != fe.system_to_component_index(i).first) &&
		   (fe_values.shape_value_component(i,x,c) == 0)),
		  ExcInternalError());
      };
  
  if (!coincide)
    deallog << "Error in fe.shape_value for " << name << std::endl;

  coincide=true;
  for (unsigned int x=0; x<q.n_quadrature_points; ++x)
    for (unsigned int i=0; i<fe.dofs_per_cell; ++i)
      {
	Tensor<1,dim> tmp=fe_values.shape_grad(i,x);
	tmp-=fe.shape_grad(i,q.point(x));
	if (sqrt(tmp*tmp)>1e-14)
	  coincide=false;

	for (unsigned int c=0; c<fe.n_components(); ++c)
	  Assert (((c == fe.system_to_component_index(i).first) &&
		   (fe_values.shape_grad(i,x) == fe_values.shape_grad_component(i,x,c)))
		  ||
		  ((c != fe.system_to_component_index(i).first) &&
		   (fe_values.shape_grad_component(i,x,c) == Tensor<1,dim>())),
		  ExcInternalError());
      }

  if (!coincide)
    deallog << "Error in fe.shape_grad for " << name << std::endl;
  
  coincide=true;
  double max_diff=0.;
  for (unsigned int x=0; x<q.n_quadrature_points; ++x)
    for (unsigned int i=0; i<fe.dofs_per_cell; ++i)
      {
	Tensor<2,dim> tmp=fe_values.shape_2nd_derivative(i,x);
	tmp-=fe.shape_grad_grad(i,q.point(x));
	for (unsigned int j=0; j<dim; ++j)
	  for (unsigned int k=0; k<dim; ++k)
	    {
	      const double diff=fabs(tmp[j][k]);
	      if (diff>max_diff) max_diff=diff;
	      if (fabs(tmp[j][k])>1e-6)
		coincide=false;
	    }	

	for (unsigned int c=0; c<fe.n_components(); ++c)
	  Assert (((c == fe.system_to_component_index(i).first) &&
		   (fe_values.shape_2nd_derivative(i,x) ==
		    fe_values.shape_2nd_derivative_component(i,x,c)))
		  ||
		  ((c != fe.system_to_component_index(i).first) &&
		   (fe_values.shape_2nd_derivative_component(i,x,c) == Tensor<2,dim>())),
		  ExcInternalError());
      }

  if (!coincide)  
    deallog << "Error in fe.shape_grad_grad for " << name << std::endl
	    << "max_diff=" << max_diff << std::endl;
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
  FE_Q<dim> q3(3);
  plot_shape_functions(m, q3, "Q3");
  plot_face_shape_functions(m, q3, "Q3");
  test_compute_functions(m, q3, "Q3");
  FE_Q<dim> q4(4);
  plot_shape_functions(m, q4, "Q4");
  plot_face_shape_functions(m, q4, "Q4");
  test_compute_functions(m, q4, "Q4");
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
  FE_DGQ<dim> q4(4);
  plot_shape_functions(m, q4, "DGQ4");
  plot_face_shape_functions(m, q4, "DGQ4");
  test_compute_functions(m, q4, "DGQ4");
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
  FE_DGP<dim> p4(4);
  plot_shape_functions(m, p4, "DGP4");
  plot_face_shape_functions(m, p4, "DGP4");
  test_compute_functions(m, p4, "DGP4");
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


int
main()
{
  std::ofstream logfile ("shapes.output");
  logfile.precision (PRECISION);
  logfile.setf(std::ios::fixed);  
  deallog.attach(logfile);
  deallog.depth_console(0);
  
  plot_FE_Q_shape_functions<1>();
  plot_FE_Q_shape_functions<2>();
//  plot_FE_DGP_shape_functions<1>();
  plot_FE_DGP_shape_functions<2>();
//  plot_FE_Q_shape_functions<3>();

				   // FESystem test.
  MappingQ1<2> m;
  FESystem<2> q2_q3(FE_Q<2>(2), 1,
		    FE_Q<2>(3), 1);
  test_compute_functions(m, q2_q3, "Q2_Q3");
//  plot_shape_functions(m, q2_q3, "Q2_Q3");
  
  return 0;
}



