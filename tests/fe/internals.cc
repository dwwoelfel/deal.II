// internals.cc,v 1.12 2003/04/09 15:49:55 wolf Exp
// (c) Guido Kanschat
//
// Compute support points

#include "../tests.h"
#include <deal.II/base/quadrature_lib.h>
#include <deal.II/base/qprojector.h>
#include <deal.II/base/logstream.h>
#include <deal.II/lac/vector.h>
#include <deal.II/grid/tria.h>
#include <deal.II/grid/tria_iterator.h>
#include <deal.II/dofs/dof_accessor.h>
#include <deal.II/grid/grid_generator.h>
#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_q_hierarchical.h>
#include <deal.II/fe/fe_dgq.h>
#include <deal.II/fe/fe_dgp.h>
#include <deal.II/fe/fe_nedelec.h>
#include <deal.II/fe/fe_bdm.h>
#include <deal.II/fe/fe_raviart_thomas.h>
#include <deal.II/fe/fe_system.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>


template <typename number>
void print_formatted (const FullMatrix<number> &A,
		      const unsigned int        precision,
		      const unsigned int        width)
{
  for (unsigned int i=0; i<A.m(); ++i)
    {
      for (unsigned int j=0; j<A.n(); ++j)
	{
	  if (A(i,j) != 0)
	    deallog << std::setw(width) << std::setprecision(precision)
		    << A(i,j);
	  else
	    deallog << std::setw(width) << std::setprecision(precision)
		    << "~";
	  deallog << ' ';
	};
      deallog << std::endl;
    };
}


template <int dim>
inline void
check_support (const FiniteElement<dim>& finel, const char* name)
{
  Triangulation<dim> tr;
  GridGenerator::hyper_cube(tr, 0., 1.);
  DoFHandler<dim> dof (tr);
  dof.distribute_dofs (finel);

  deallog << name << '<' << dim << '>' << " cell support points" << std::endl;
  
  const std::vector<Point<dim> > &cell_points = finel.get_unit_support_points ();

  for (unsigned int k=0;k<cell_points.size();++k)
    deallog << std::setprecision(3) << cell_points[k] << std::endl;
  
  const std::vector<Point<dim-1> > &face_points = finel.get_unit_face_support_points ();
  const std::vector<double> dummy_weights (face_points.size());
  
  Quadrature<dim-1> q(face_points, dummy_weights);
  
  for (unsigned int i=0;i<GeometryInfo<dim>::faces_per_cell;++i)
    {
      std::vector<Point<dim> > q_points (q.get_points().size());
      QProjector<dim>::project_to_face (q, i, q_points);
      Quadrature<dim> qp(q_points);
      deallog << name << '<' << dim << '>' << " face " << i
		<< " support points" << std::endl;
        
      for (unsigned int k=0; k<face_points.size(); ++k)
	deallog << std::setprecision(3) << qp.point(k)
		<< std::endl;
    }
}

template <int dim>
inline void
check_matrices (FiniteElement<dim>& fe, const char* name)
{
  deallog << name << '<' << dim << '>' << " constraint " << std::endl;
  print_formatted (fe.constraints(), 7, 10);

  for (unsigned int i=0;i<GeometryInfo<dim>::max_children_per_cell;++i)
    {
      deallog << name << '<' << dim << '>' << " restriction " << i << std::endl;
      if (fe.isotropic_restriction_is_implemented())
	print_formatted (fe.get_restriction_matrix(i), 3, 6);
      deallog << name << '<' << dim << '>' << " embedding " << i << std::endl;
      if (fe.isotropic_prolongation_is_implemented())
	print_formatted (fe.get_prolongation_matrix(i), 3, 6);
    }
}


#define CHECK_S(EL,deg,dim)   { FE_ ## EL<dim> EL(deg); check_support(EL, #EL #deg); }
#define CHECK_M(EL,deg,dim)   { FE_ ## EL<dim> EL(deg); check_matrices(EL, #EL #deg); }
#define CHECK_ALL(EL,deg,dim) { FE_ ## EL<dim> EL(deg); check_support(EL, #EL #deg); \
                                                        check_matrices(EL,#EL #deg); }
#define CHECK_SYS1(sub1,N1,dim) { FESystem<dim> q(sub1, N1); check_support(q, #sub1 #N1); \
                                                             check_matrices(q,#sub1 #N1); }
#define CHECK_SYS2(sub1,N1,sub2,N2,dim) { FESystem<dim> q(sub1, N1, sub2, N2); \
                                          check_support(q, #sub1 #N1 #sub2 #N2); \
                                          check_matrices(q,#sub1 #N1 #sub2 #N2); }
#define CHECK_SYS3(sub1,N1,sub2,N2,sub3,N3,dim) { FESystem<dim> q(sub1, N1, sub2, N2, sub3, N3); \
                                          check_support(q, #sub1 #N1 #sub2 #N2 #sub3 #N3); \
                                          check_matrices(q,#sub1 #N1 #sub2 #N2 #sub3 #N3); }

int
main()
{
  std::ofstream logfile("internals/output");
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  CHECK_M(DGQ,0,2);
  CHECK_M(DGQ,1,2);
  CHECK_M(DGQ,2,2);
  CHECK_M(DGQ,3,2);
  CHECK_M(DGQ,4,2);

                                   // DGP elements presently have no
                                   // restriction matrices, so cannot
                                   // be tested
  CHECK_M(DGP,0,2);
  CHECK_M(DGP,1,2);
  CHECK_M(DGP,2,2);
  CHECK_M(DGP,3,2);
  CHECK_M(DGP,4,2);

  CHECK_ALL(Q,1,2);
  CHECK_ALL(Q,2,2);
  CHECK_ALL(Q,3,2);

  CHECK_ALL(Q_Hierarchical,1,2);
  CHECK_ALL(Q_Hierarchical,2,2);
  CHECK_ALL(Q_Hierarchical,3,2);

  CHECK_M(DGQ,0,3);
  CHECK_M(DGQ,1,3);
  CHECK_M(DGQ,2,3);

                                   // see above
  CHECK_M(DGP,0,3);
  CHECK_M(DGP,1,3);
  CHECK_M(DGP,2,3);

  CHECK_ALL(Q,1,3);
  CHECK_ALL(Q,2,3);

  CHECK_ALL(Q_Hierarchical,1,3);
  CHECK_ALL(Q_Hierarchical,2,3);

  CHECK_ALL(Nedelec, 0, 2);
  CHECK_ALL(Nedelec, 0, 3);
  CHECK_ALL(Nedelec, 1, 2);
  CHECK_ALL(Nedelec, 1, 3);
  
  CHECK_ALL(RaviartThomas, 0, 2);
  CHECK_ALL(RaviartThomas, 0, 3);
  CHECK_ALL(RaviartThomas, 1, 2);
  CHECK_ALL(RaviartThomas, 1, 3);
  
  CHECK_ALL(BDM, 1, 2);
  CHECK_ALL(BDM, 2, 2);
  
  CHECK_SYS1(FE_Q<2>(1),3,2);
  CHECK_SYS1(FE_DGQ<2>(2),2,2);
//  CHECK_SYS1(FE_DGP<2>(3),1,2);

  CHECK_SYS2(FE_Q<2>(1),3,FE_DGQ<2>(2),2,2);
//   CHECK_SYS2(FE_DGQ<2>(2),2,FE_DGP<2>(3),1,2);
//   CHECK_SYS2(FE_DGP<2>(3),1,FE_DGQ<2>(2),2,2);

//   CHECK_SYS3(FE_Q<2>(1),3,FE_DGP<2>(3),1,FE_Q<2>(1),3,2);
  CHECK_SYS3(FE_DGQ<2>(2),2,FE_DGQ<2>(2),2,FE_Q<2>(3),3,2);
//   CHECK_SYS3(FE_DGP<2>(3),1,FE_DGP<2>(3),1,FE_Q<2>(2),3,2);

				   // systems of systems  
  CHECK_SYS3((FESystem<2>(FE_Q<2>(1),3)), 3,
	     FE_DGQ<2>(3), 1,
	     FE_Q<2>(1), 3,
	     2);
  CHECK_SYS3(FE_DGQ<2>(3), 1,
 	     FESystem<2>(FE_DGQ<2>(3),3), 1,
 	     FESystem<2>(FE_Q<2>(2),3,
 			 FE_DGQ<2>(0),1),2,
 	     2);

                                   // systems with Nedelec elements
  CHECK_SYS2 (FE_DGQ<2>(3), 1,
              FE_Nedelec<2>(0), 2,
              2);
  CHECK_SYS3(FE_Nedelec<2>(0), 1,
 	     FESystem<2>(FE_DGQ<2>(3),3), 1,
 	     FESystem<2>(FE_Q<2>(2),3,
 			 FE_Nedelec<2>(0),2),2,
 	     2);
  CHECK_SYS2 (FE_DGQ<2>(3), 1,
              FE_Nedelec<2>(1), 2,
              2);
  CHECK_SYS3(FE_Nedelec<2>(1), 1,
 	     FESystem<2>(FE_DGQ<2>(3),3), 1,
 	     FESystem<2>(FE_Q<2>(2),3,
 			 FE_Nedelec<2>(1),2),2,
 	     2);
  
  return 0;
}
