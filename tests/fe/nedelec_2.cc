// nedelec_2.cc,v 1.4 2003/04/09 15:49:55 wolf Exp
// (c) Wolfgang Bangerth
//
// Some more tests with the Nedelec element, basically using
// multi-element triangulations and check whether interface conditions
// are ok

#include "../tests.h"
#include <base/quadrature_lib.h>
#include <base/logstream.h>
#include <lac/vector.h>
#include <grid/tria.h>
#include <grid/tria_iterator.h>
#include <dofs/dof_accessor.h>
#include <lac/constraint_matrix.h>
#include <dofs/dof_tools.h>
#include <grid/grid_generator.h>
#include <grid/grid_tools.h>
#include <fe/fe_nedelec.h>
#include <fe/fe_values.h>

#include <vector>
#include <fstream>
#include <string>

#define PRECISION 2


template <int dim>
void
plot (const Triangulation<dim> &tr, const unsigned int p)
{
  deallog << dim << "d, "
          << tr.n_active_cells() << " CELLS" << std::endl;

  FE_Nedelec<dim> fe_ned (p);
  
  DoFHandler<dim> dof(tr);
  dof.distribute_dofs(fe_ned);

                                   // generate some numbers for the
                                   // degrees of freedom on this mesh
  Vector<double> values (dof.n_dofs());
  for (unsigned int i=0; i<values.size(); ++i)
    values(i) = i;
                                   // then make sure that hanging node
                                   // constraints are taken care of
  ConstraintMatrix cm;
  DoFTools::make_hanging_node_constraints (dof, cm);
  cm.close ();
  cm.distribute (values);

                                   // now take these values, and print
                                   // the values of this so defined
                                   // function on each cell and on
                                   // each quadrature point
  QTrapez<dim> quadrature;
  std::vector<Vector<double> > shape_values (quadrature.n_quadrature_points,
                                             Vector<double>(dim));
  FEValues<dim> fe(fe_ned, quadrature,
                   update_values|update_q_points);

  for (typename DoFHandler<dim>::active_cell_iterator
         c = dof.begin_active();
       c!=dof.end(); ++c)
    {
      deallog << "  CELL " << c << std::endl;
      fe.reinit(c);
      fe.get_function_values (values, shape_values);

      for (unsigned int q=0; q<quadrature.n_quadrature_points; ++q)
        {
          deallog << ", xq=" << fe.quadrature_point(q)
                  << ", f=[";
          for (unsigned int d=0; d<dim; ++d)
            deallog << (d==0 ? "" : " ")
                    << shape_values[q](d);
          
          deallog << "]" << std::endl;
        };
                
      deallog << std::endl;
    }
}



template<int dim>
inline void
check (const unsigned int p)
{
  Triangulation<dim> tr;
  GridGenerator::hyper_cube(tr, 0., 1.);

                                   // first everything on a
                                   // once-refined grid
  tr.refine_global (1);
  plot (tr, p);

                                   // then same with one cell refined
  tr.begin_active()->set_refine_flag ();
  tr.execute_coarsening_and_refinement ();
  plot (tr, p);
}


int
main()
{
  std::ofstream logfile ("nedelec_2/output");
  deallog << std::setprecision(PRECISION);
  deallog << std::fixed;  
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);
  deallog << "Degree 0:" << std::endl;
  check<2> (0);
  check<3> (0);
  deallog << "Degree 1:" << std::endl;
  check<2> (1);
  check<3> (1);
  
  return 0;
}



