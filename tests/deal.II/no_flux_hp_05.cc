// check the creation of no-flux boundary conditions for a finite
// element that consists of more than dim components and where
// therefore we have to pick the vector components from somewhere in
// the middle
//
// similar to no_flux_hp_02, but for a Q^dim \times DGP element as
// used for Stokes at times. The problem here is that the DGP element
// does not have support points, which caused problems when this test
// was written


#include "../tests.h"
#include <deal.II/base/logstream.h>
#include <deal.II/base/function.h>
#include <deal.II/base/quadrature_lib.h>
#include <deal.II/lac/vector.h>
#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/tria_boundary_lib.h>
#include <deal.II/hp/dof_handler.h>
#include <deal.II/dofs/dof_renumbering.h>
#include <deal.II/lac/constraint_matrix.h>
#include <deal.II/hp/fe_collection.h>
#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_dgp.h>
#include <deal.II/fe/fe_system.h>
#include <deal.II/fe/mapping_q1.h>
#include <deal.II/numerics/vector_tools.h>

#include <fstream>



template<int dim>
void test (const Triangulation<dim>& tr,
		      const hp::FECollection<dim>& fe)
{
  hp::DoFHandler<dim> dof(tr);
  dof.distribute_dofs(fe);

  DoFRenumbering::component_wise (dof);

  for (unsigned int i=0; i<GeometryInfo<dim>::faces_per_cell; ++i)
    {
      deallog << "FE=" << fe[0].get_name()
	      << ", case=" << i
	      << std::endl;

      std::set<types::boundary_id> boundary_ids;
      for (unsigned int j=0; j<=i; ++j)
	boundary_ids.insert (j);

      ConstraintMatrix cm;
      VectorTools::compute_no_normal_flux_constraints (dof, 0, boundary_ids, cm);

      cm.print (deallog.get_file_stream ());
    }
}


template<int dim>
void test_hyper_cube()
{
  Triangulation<dim> tr;
  GridGenerator::hyper_ball(tr);

  static const HyperBallBoundary<dim> boundary;
  tr.set_boundary (0, boundary);

  tr.refine_global(1);

  for (unsigned int degree=1; degree<4; ++degree)
    {
      hp::FECollection<dim> fe (FESystem<dim> (FE_Q<dim>(degree), dim,
			                                   FE_DGP<dim>(degree+1), 1));
      test(tr, fe);
    }
}


int main()
{
  std::ofstream logfile ("no_flux_hp_05/output");
  deallog << std::setprecision (2);
  deallog << std::fixed;
  deallog.attach(logfile);
  deallog.depth_console (0);
  deallog.threshold_double(1.e-12);

  test_hyper_cube<2>();
  test_hyper_cube<3>();
}
