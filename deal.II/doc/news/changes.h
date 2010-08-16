/**
 * @page changes_after_6_3 Changes after Version 6.3

<p>
This is the list of changes made after the release of
deal.II version 6.3.0. It is subdivided into changes
made to the three sub-libraries <a href="#base">base</a>,
<a href="#lac">lac</a>, and <a href="#deal.II">deal.II</a>, as well as
changes to the <a href="#general">general infrastructure,
documentation, etc</a>.
</p>

<p>
All entries are signed with the names of the author. Regular
contributor's names are abbreviated by WB (Wolfgang Bangerth), GK
(Guido Kanschat), RH (Ralf Hartmann).
</p>


<a name="incompatible"></a>
<h3 style="color:red">Incompatibilities</h3>

<p style="color:red">
Following are a few modifications to the library that unfortunately
are incompatible with previous versions of the library, but which we
deem necessary for the future maintainability of the
library. Unfortunately, some of these changes will require
modifications to application programs. We apologize for the
inconvenience this causes.
</p>

<ol>

<li> The fields DoFHandler::tria and DoFHandler::selected_fe are now
private instead of protected. Inheriting classes can only access them
through DoFHandler::get_tria() and DoFHandler::get_fe(), respectively.
<br>
(GK 2010/08/16)</li>

</ol>


<a name="general"></a>
<h3>General</h3>

<ol>
  <li>
  <p>Fixed: When using Trilinos and deal.II both with static libraries,
  a linker error would occur whenever a program linked both the 2d and
  3d libraries of deal.II. This is now fixed.
  <br>
  (WB 2010/07/23)
  </p>

  <li>
  <p>Fixed: On all non-linux platforms, if static libraries were selected
  and <code>./configure</code> was instructed to use Trilinos (which also
  was compiled with static libraries), a failure would occur. This should
  now be fixed.
  <br>
  (WB 2010/07/23)
  </p>

  <li>
  <p>Fixed: The step-33 tutorial program can not be built with GCC versions
  4.5.x. There are in fact two problems, one that pertains to uses of
  <code>std::make_pair</code> that don't work any more with the upcoming
  C++ 1x standard that GCC 4.5.x already follows, and some in which the
  Trilinos package Sacado is incompatible with GCC 4.5.x, at least up to
  Trilinos version 10.4.0. While the latter problem can only be fixed in
  future Trilinos versions, at least the former problem is solved in step-33.
  <br>
  (WB 2010/07/18)
  </p>

  <li>
  <p>Fixed: GCC version 3.4.0 failed to compile the file
  <code>deal.II/source/numerics/matrices.cc</code> with
  an internal compiler error. This has
  now been worked around.
  <br>
  (WB 2010/07/15)
  </p>

  <li>
  <p>Fixed: A problem in the Makefiles caused error messages when
  building under CygWin.
  <br>
  (GK 2010/07/12)
  </p>

  <li>
  <p>Fixed: GCC version 3.3.x failed to compile the files
  <code>lac/include/lac/precondition_block.h</code>,
  <code>deal.II/source/multigrid/mg_dof_handler.cc</code> and
  <code>examples/step-34/step-34.cc</code>. These problems have
  now been worked around.
  <br>
  (WB 2010/07/12)
  </p>

  <li>
  <p>Fixed: Some older 3.x versions of GCC crashed compiling the functions in
  namespace DoFRenumbering::boost. There is now a configuration time test
  that checks that the compiler accepts the constructs in question. If the
  compiler does not, then these functions are disabled.
  <br>
  (WB 2010/07/01)
  </p>

  <li>
  <p>Fixed: Linking with more than one of the deal.II 1d, 2d, or 3d libraries
  when using static libraries did not work. This is now fixed. However, due to
  GCC bug <a href="http://gcc.gnu.org/bugzilla/show_bug.cgi?id=10591"
  target="_top">10591</a>, GCC versions prior to and including 4.1.x will
  still not work. Working with shared libraries was not and is not affected
  by this problem.
  <br>
  (WB 2010/07/01)
  </p>

  <li>
  <p>Updated: The version of <a href="http://www.boost.org/">boost</a>
   included in the <code>contrib/</code> directory has been updated
   to 1.43.0.
  <br>
  (WB 2010/06/30)
  </p>

  <li>
  <p>Fixed: GCC version 4.0.1 had a bug that prevented it from compiling
  release 6.3.0 because it apparently had an infinite loop allocating
  memory when compiling <code>fe_values.cc</code> in optimized mode. This
  problem had been fixed in GCC 4.0.2, but some versions of Mac OS X still use
  this GCC version in their Xcode environment. In any case, the code in
  deal.II has been changed to avoid this problem.
  <br>
  (WB 2010/06/30)
  </p>

  <li>
  <p>Fixed: Configuring with an external BOOST version did not work when
  using shared libraries since the test ran in the wrong order with respect
  to another configure test. This is now fixed.
  <br>
  (Bradley Froehle 2010/06/29)
  </p>

  <li>
  <p>Updated: The conversion tool in <code>contrib/mesh_conversion</code> that
  can read CUBIT output and convert it into something that is readable by
  deal.II has been updated.
  <br>
  (Jean-Paul Pelteret 2010/06/28)
  </p>

  <li>
  <p>
  Fixed: deal.II release 6.3.0 did not compile with Trilinos versions 9.x and
  10.0. This is now fixed.
  <br>
  (Martin Kronbichler, WB 2010/06/28)
  </p>
</ol>



<a name="base"></a>
<h3>base</h3>


<ol>
  <li><p> Fixed: The computation of quadrature points in the QGaussLobatto
  class uses a Newton method that was wrongly implemented. While the
  results were correct (at least for moderate orders), it required more
  iterations than necessary. This is now fixed.
  <br>
  (Andrea Bonito 2010/08/12)
  </p>

  <li><p> Changed: The DataOutBase::write_vtu function now writes data
  as 64-bit values, rather than 32-bit values.
  <br>
  (Scott Miller 2010/08/5)
  </p>

  <li><p> New: MappingQ and MappingQEulerian now support order > 1 also in
  codimension one. Step-34 has been modified to show how this works.
  <br>
  (Luca Heltai 2010/07/23-27)
  </p>

  <li><p> New: QGaussOneOverR now has a new constructor for arbitrary quadrature
  points and not only the vertices of the reference cell.
  <br>
  (Luca Heltai 2010/07/21)
  </p>
</ol>


<a name="lac"></a>
<h3>lac</h3>

<ol>
  <li><p>Updated: Changes to the <code>SLEPcWrappers</code> to preserve
  compatibility with SLEPc version 3.1. Main new updated features are
  new solver classes: (i) Power; and (ii) Generalized Davidson.
  <br>
  (Toby D. Young 2010/08/04)
  </p></li>

  <li><p>Fixed: SparseMatrix::precondition_SSOR and
  SparseMatrixEZ::precondition_SSOR had a bug, which made the SSOR
  method inaccurate for relaxation parameters different from unity.
  <br>
  (GK 2010/07/20)
  </p></li>
</ol>



<a name="deal.II"></a>
<h3>deal.II</h3>

<ol>

  <li>
  <p>
  Improved: DoFHandler has a default constructor, so that it can be used in containers. 	
  <br>
  (GK 2010/08/16)
  </p>

  <li>
  <p>
  New: The functions VectorTools::project_boundary_values_curl_conforming
  are added. They can compute Dirichlet boundary conditions for Nedelec
  elements. 	
  <br>
  (Markus Buerg 2010/08/13)
  </p>

  <li>
  <p>
  Fixed: The function FEValuesViews::Vector::get_function_curls produced
  wrong results in some cases, because it erased the given vector first.
  This is now fixed.	
  <br>
  (Markus Buerg 2010/08/13)
  </p>

  <li>
  <p>
  New: Ability to project second-order SymmetricTensor and first-order Tensor objects from the quadrature points to the support points of the cell using  FETools::compute_projection_from_quadrature_points
  <br>
  (Andrew McBride 2010/07/29)
  </p>

  <li>
  <p>
  Fixed: In some rather pathological cases, the function
  Triangulation::prepare_coarsening_and_refinement (which is called from
  Triangulation::execute_coarsening_and_refinement) could take very long
  if the flag Triangulation::eliminate_unrefined_islands was given in
  the mesh smoothing flags upon construction of the triangulation. This is
  now fixed.
  <br>
  (WB 2010/07/27)
  </p>

  <li><p> New: Brezzi-Douglas-Marini elements of arbitrary order in FE_BDM.
  <br>
  (GK 2010/07/19)
  </p>

  <li>
  <p>
  Fixed: The FEValues::get_cell() function was unusable from user code
  since its implementation used a class that was only forward declared
  and not visible at the point of instantiations in user code. This is now
  fixed.
  <br>
  (WB 2010/07/16)
  </p>

  <li>
  <p>New: The multilevel matrices for continuous elements
  can be built with the MeshWorker now.
  <br>
  (B&auml;rbel Janssen 2010/07/01)
  </p></li>

  <li>
  <p>
  Fixed: On some systems and compilers, the library could not be compiled
  because of a duplicate symbol in <code>MeshWorker::LocalResults</code>.
  This is now fixed.
  <br>
  (WB 2010/06/28)
  </p>

  <li>
  <p>
  Fixed: The output of the function
  FE_Q::adjust_quad_dof_index_for_face_orientation
  was wrong in 3d for polynomial orders of three or greater. This is now
  fixed.
  <br>
  (WB 2010/06/28)
  </p>
</ol>


*/
