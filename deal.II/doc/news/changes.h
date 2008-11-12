/**
 * @page changes_after_6.1 Changes after Version 6.1

<p>
This is the list of changes made after the release of 
deal.II version 6.1. It is subdivided into changes
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
  <li>
  <p> 
  Changed: The function DoFTools::get_subdomain_association function used
  to assign degrees of freedom to the subdomain of the last cell on which
  the degree of freedom is a part. This introduced a bias for degrees of
  freedom's subdomains located on boundaries of subdomains, and
  consequently to unequal numbers of DoFs per subdomain even if the
  number of cells is roughly equal across subdomains. This behavior has
  been changed by assigning degrees of freedom pseudo-randomly to any of
  the subdomains on which they are located. This is a deviation from
  previous behavior, however.
  <br>
  (Timo Heister, WB 2008/11/02)
  </p>

  <li>
  <p> 
  Changed: The way we set up threads in the Threads::spawn functions
  and friends has been completely written, using the boost::bind and
  boost::function libraries. This has made things significantly simpler
  and allowed us to remove some 4,100 lines of code. The only
  user-visible side effect is that you can now no longer spawn 
  functions with 10 arguments (this was the maximum before) whereas 9
  arguments continues to work; the reason for this is a limitation in
  the boost::bind library. This limit will be lifted with the next
  C++ standard, however, using variadic templates.
  <br>
  (WB 2008/10/10)
  </p>

  <li>
  <p> 
  Changed: The SolutionTransfer class used to take a type as second
  template argument that denoted the scalar upon which Vector objects
  were built, in order to allow interpolating Vector@<float@> objects,
  for example. This argument has now been changed to a vector type,
  and been given a default of Vector@<double@>; however, one can
  now also pass in BlockVector objects, or objects of type
  PETScWrappers::Vector, etc. On the downside, the old
  SolutionTransfer::refine_interpolate function with only a single
  argument has been deleted since there is no reliable way to resize
  a vector unless it is a plain Vector@<double@>.
  <br>
  (WB 2008/08/28)
  </p>

  <li>
  <p> 
  Changed: The FiniteElement::get_prolongation_matrix and
  FiniteElement::get_restriction_matrix functions now have an
  additional argument of type RefinementCase to enable a
  meaningful use of these matrices in case of anisotropic
  refinement. These matrices are used for solution transfer and
  multigrid operations, but there should be no need to use them
  in user codes directly. Note, that the matrices are not fully
  implemented for all finite elements up to now.
  <br>
  (Tobias Leicht 2008/07/08 as of branch_anisotropic at 2006/07/04)
  </p>
  
  <li>
  <p>
  Changed: <code>GeometryInfo::children_per_cell</code> has been
  replaced by GeometryInfo::max_children_per_cell, which
  represents the maximum number of children a cell might have,
  i.e. the number of children in the case of isotropic
  refinement. But note, that this number will rarely be needed in
  user codes. In general, the number of children of a cell varies
  from cell to cell and can be obtained by
  <code>cell->n_children()</code>, which gives the number of
  children of a specific <code>cell</code> which is refined iso-
  or anisotropically.
  <br>
  (RH 2008/07/08 as of branch_anisotropic at 2005/02/26)
  </p>

  <li>
  <p>
  Changed: The analogous change applies to the number of children
  of a face. <code>GeometryInfo::subfaces_per_face</code> has
  been replaced by GeometryInfo::max_children_per_face, which
  represents the maximum number of children a face might have,
  i.e. the number of children in the case of an isotropic refined
  face.  But note, that this number will rarely be needed in user
  codes. In general, the number of children of a face varies from
  face to face and can be obtained by
  <code>face->n_children()</code>, which gives the number of
  children of a specific <code>face</code> which is refined iso-
  or anisotropically.
  <br>
  (RH 2008/07/08 as of branch_anisotropic at 2005/02/26)
  </p>
       
  <li>
  <p>
  Changed: The GeometryInfo::child_cell_on_face function is
  generalized to anisotropic refinement and has now an additional
  RefinementCase argument. This function will rarely be used in
  user codes, as e.g. the neighbor's child at a specific face and
  subface should be accessed through
  <code>cell->neighbor_child_on_subface</code> rather than
  through the GeometryInfo::child_cell_on_face function.
  <br>
  (RH 2008/07/08 as of branch_anisotropic at 2005/02/27)
  </p>
</ol>


<a name="general"></a>
<h3>General</h3>

<ol>
  <li>
  <p>
  Updated: In the @ref step_22 "step-22" tutorial program the generation of
  the sparsity pattern using the class BlockCompressedSetSparsityPattern has 
  been replaced by a pattern of the class BlockCompressedSimpleSparsityPattern, 
  which uses far less memory and is slightly faster.
  <br>
  (Martin Kronbichler 2008/11/12)
  </p>
  </li>

  <li>
  <p>
  New: The shared libraries we create are now versioned, i.e. they have
  the form <code>libdeal_II_2d.g.so.6.2.0</code>. The library without
  the version number is simply a link to the versioned library name.
  <br>
  (WB 2008/10/07)
  </p>
  </li>

  <li>
  <p>
  Updated: The version of the <a href="http://www.boost.org/">BOOST</a>
  library that is in the <code>contrib/</code> directory and is used in
  various places of the library has been upgraded to 1.36.
  <br>
  (WB 2008/09/19)
  </p>
  </li>

  <li>
  <p>
  Fixed: In the @ref step_23 "step-23" tutorial program the terms corresponding
  to the external force were computed incorrectly. The error wasn't visible
  right away since in the program the right hand side is assumed to be zero.
  This has now been fixed.
  <br>
  (Johan Lorentzon, WB 2008/09/12)
  </p>
  </li>

  <li>
  <p>
  Fixed: In the new @ref step_33 "step-33" tutorial program there was
  a place where we incorrectly passed the diameter of a cell, rather
  than a face, to a function. This is now fixed.
  <br>
  (Chih-Che Chueh, WB 2008/06/28)
  </p>
  </li>

  <li>
  <p>
  Fixed: The new @ref step_33 "step-33" tutorial program had a place where
  we didn't release some memory. This is now fixed.
  <br>
  (Chih-Che Chueh, WB 2008/06/02)
  </p>
  </li>

  <li>
  <p>
  Fixed: A missing include file prevented the <code>./configure</code> script
  from detecting the presence of the demangler with recent versions of the
  gcc compiler. The result is that backtraces after failed assertions only
  show the mangles function names, not their plain text equivalent. This is
  now fixed.
  <br>
  (WB 2008/05/27)
  </p>
  </li>
</ol>



<a name="base"></a>
<h3>base</h3>

<ol>
  <li>
  <p>
  New: The Utilities::reverse_permutation and Utilities::invert_permutation
  compute the reverse and inverse of a given permutation of indices.
  <br> 
  (WB 2008/10/31)
  </p>

  <li>
  <p>
  Fixed: The PolynomialsRaviartThomas class had a bug that led to random
  results when used from multiple threads. As a consequence the FE_RaviartThomas
  class was unusable in a multithreaded context. This has now been fixed.
  <br> 
  (WB 2008/10/13)
  </p>

  <li>
  <p>
  New: There is a new function scalar_product(const Tensor<2,dim> &,
  const Tensor<2,dim> &) that computes the scalar product
  $a:b=\sum_{i,j} a_{ij}b_{ij}$ between two tensors of rank 2.
  <br> 
  (WB 2008/08/15)
  </p>

  <li>
  <p>
  New: If the compiler allows to do <code>\#include @<mpi.h@></code>, then
  the preprocessor flag <code>DEAL_II_COMPILER_SUPPORTS_MPI</code> is now set in
  <code>base/include/base/config.h</code>. This also fixes a problem in
  <code>base/include/base/utilities.h</code> if a compiler capable of
  including <code>mpi.h</code> was used but not PETSc.
  <br> 
  (WB 2008/08/15)
  </p>

  <li>
  <p>
  Fixed: A misplaced <code>\#include</code> directive prevented the file
  <code>base/source/data_out_base.cc</code> from being compilable by
  the PGI C++ compiler. This is now fixed.
  <br> 
  (WB 2008/08/05)
  </p>

  <li>
  <p>
  New: There are now a new
  GeometryInfo::min_cell_refinement_case_for_face_refinement
  (resp. GeometryInfo::min_cell_refinement_case_for_line_refinement)
  function which returns the RefinementCase representing the
  smallest refinement case of a cell for a given refinement of
  one of its faces (resp. lines). In 2D for example a cell has to
  be refined at least with RefinementCase::cut_y if the left line
  (line 0) shall be refined. Another refinement possibility for
  the cell would be <code>RefinementCase::cut_xy</code>, but that
  is not the minimal case.
  <br> 
  (Tobias Leicht 2008/07/08 as of branch_anisotropic at 2006/06/28)
  </p>

  <li>
  <p>
  New: There is now a new GeometryInfo::line_refinement_case
  function which returns the RefinementCase representing the
  refinement case of a line for a given refinement case of the
  corresponding cell.
  <br> 
  (Tobias Leicht 2008/07/08 as of branch_anisotropic at 2006/06/28)
  </p>

  <li>
  <p>
  New: The new
  <tt>GeometryInfo<dim>::n_children(refinement_case)</tt>
  function returns the number of children a cell/face has when
  refined with the RefinementCase <tt>refinement_case</tt>.
  <br> 
  (RH 2008/07/08 as of branch_anisotropic at 2005/11/07)
  </p>

  <li>
  <p>
  New: Given a RefinementCase of a cell the new
  GeometryInfo::face_refinement_case function returns the
  RefinementCase of a face.
  <br>
  (RH 2008/07/08 as of branch_anisotropic at 2005/11/07)
  </p>

  <li>
  <p>
  New: There is now a new RefinementCase
  GeometryInfo::isotropic_refinement representing the isotropic
  refinement case in <code>dim</code> dimensions,
  i.e. <tt>GeometryInfo<1>::isotropic_refinement=RefinementCase::cut_x</tt>,
  <tt>GeometryInfo<2>::isotropic_refinement=RefinementCase::cut_xy</tt>
  and
  <tt>GeometryInfo<3>::isotropic_refinement=RefinementCase::cut_xyz</tt>.
  <br> 
  (RH 2008/07/08 as of branch_anisotropic at 2005/03/03)
  </p>
</ol>



<a name="lac"></a>
<h3>lac</h3>

<ol>
  <li>
  <p>
  New: The class LAPACKFullMatrix can now invert full matrices using 
  the (optimized) LAPACK functions getrf and getri. The speedup over 
  the FullMatrix::gauss_jordan() function is a factor of two for matrices
  with 100 rows and columns, and grows with matrix size.
  <br>
  (Martin Kronbichler 2008/11/11)
  </p>

  <li>
  <p>
  Fixed: The BlockMatrixBase::clear() function that is used by all other
  block matrix type classes had a memory leak in that the memory
  allocated by all sub-objects was not freed. This is now fixed.
  <br>
  (WB 2008/11/05)
  </p>

  <li>
  <p>
  New: The function SparsityTools::reorder_Cuthill_McKee reorders
  the nodes of a graph based on their connectivity to other nodes.
  <br>
  (WB 2008/10/31)
  </p>

  <li>
  <p>
  New: The function GridTools::get_face_connectivity_of_cells produces a
  sparsity pattern that describes the connectivity of cells of a
  triangulation based on whether they share common faces.
  <br>
  (WB 2008/10/31)
  </p>

  <li>
  <p>
  Changed: The function SparsityPattern::partition has been deprecated. It
  is now available in a new namespace SparsityTools that collects algorithms
  that work on sparsity patterns or connectivity graphs.
  <br>
  (WB 2008/10/31)
  </p>

  <li>
  <p>
  Fixed: Whereas the Vector class copy operator resized the left hand side
  operand whenever necessary, the corresponding operator of the BlockVector
  class did not. This is now fixed.
  <br>
  (Christian Cornelssen, WB 2008/10/28)
  </p>

  <li>
  <p>
  Changed: The SparseDirectUMFPACK class now calls the umfpack_dl_* routines 
  instead of umfpack_di_*. On machines with 64-bit longs this allows the 
  UMFPACK solver to allocate more than 2GB of memory for large problems.
  <br>
  (Moritz Allmaras 2008/10/16)
  </p>

  <li>
  <p>
  Improved: The SparseILU::initialize function, for some reason, required
  the second argument, SparseILU::AdditionalParameters, to be present even
  if it is a default constructed object. This argument now has a default value
  that equates to a default constructed object that can therefore be omitted
  when so desired.
  <br>
  (WB 2008/09/23)
  </p>

  <li>
  <p>
  New: Added the CompressedSimpleSparsityPattern as an alternative to 
  CompressedSparsityPattern and CompressedSetSparsityPattern, which
  should be faster in most cases but its memory usage is somewhere
  inbetween.
  <br>
  (Timo Heister 2008/09/03)
  </p>

  <li>
  <p>
  Improved: The CompressedSparsityPattern can now elide some operations
  upon entering entries that may have been added before already.
  <br>
  (Timo Heister, WB 2008/08/28)
  </p>

  <li>
  <p>
  Fixed: There are now functions CompressedSparsityPattern::print and
  CompressedSetSparsityPattern::print, in analogy
  to SparsityPattern::print .
  <br>
  (WB 2008/07/31)
  </p>

  <li>
  <p>
  Fixed: The FullMatrix::copy_from(FullMatrix,bool) function had a bug
  that resulted in an exception being triggered whenever the given matrix
  had rows where the diagonal element is zero and the second parameter
  to the function was set to <code>true</code>. This is now fixed.
  <br>
  (WB 2008/07/30)
  </p>
</ol>



<a name="deal.II"></a>
<h3>deal.II</h3>

<ol>
  <li>
  <p>
  Fixed: The GridIn::read_msh function had a bug that made it reject
  MSH input files if they contained type-15 cells with more than one
  associated vertex.
  <br>
  (WB 2008/11/05)
  </p>

  <li>
  <p>
  Fixed: The DoFRenumbering::Cuthill_McKee algorithm had a bug when applied
  to MGDoFHandler objects and if the <code>reverse</code> flag was set. This
  should now be fixed.
  <br>
  (WB 2008/10/31)
  </p>

  <li>
  <p>
  New: MatrixTools::apply_boundary_values() also for PETScWrappers::MPI::BlockSparseMatrix.
  <br>
  (Timo Heister 2008/10/27)
  </p>

  <li>
  <p>
  New: When calling function DoFTools::make_sparsity_pattern with a ConstraintMatrix, it is now possible to set a bool argument keep_constrained_dofs. When this flag is set to false, constrained rows and columns will not be part of the sparsity pattern, which increases the performance of matrix operations and decrease memory consumption in case there are many constraints.
  <br>
  (Martin Kronbichler 2008/10/21)
  </p> 

  <li>
  <p>
  New: There is now a second DoFTools::count_dofs_with_subdomain_association function that
  calculates the number of degrees of freedom associated with a certain subdomain and
  splits the result up according to the vector component of each degree of freedom. This
  function is needed when splitting block matrices in parallel computations.
  <br>
  (WB 2008/10/07)
  </p> 

  <li>
  <p>
  Fixed: The GridOut::write_gnuplot function had a bug that made it output only the
  very first cell. This is now fixed.
  <br>
  (WB 2008/10/09)
  </p>

  <li>
  <p>
  New: The GridIn::read_msh function can now read version 2 of the MSH format described
  <a target="_top" href="http://www.geuz.org/gmsh/doc/texinfo/gmsh_10.html">here</a>.
  <br>
  (WB 2008/10/07)
  </p> 

  <li>
  <p>
  Fixed: In rare cases, when Triangulation::limit_level_difference_at_vertices
  is passed to the constructor of the Triangulation class, meshes could be
  generated that do not honor this flag. This is now fixed.
  <br>
  (WB 2008/10/06)
  </p>

  <li>
  <p>
  New: The class FE_Q can now alternatively be constructed based on 
  support points from a given one-dimensional quadrature rule.
  <br>
  (Katharina Kormann, Martin Kronbichler, 2008/09/07)
  </p> 

  <li>
  <p>
  Fixed: Using the ConstraintMatrix class, when a degree of freedom was
  constrained against another DoF, and that other DoF was constrained to
  be equal to zero (i.e. the trivial linear combination of a third set of
  degrees of freedom), an exception resulted. This is now fixed.
  <br>
  (WB 2008/08/15)
  </p>

  <li>
  <p>
  New: It is now possible to get the inverse of the Jacobian
  matrix from the transformation form the real to the unit cell by
  using FEValues::inverse_jacobian.
  <br>
  (Martin Kronbichler 2008/08/13)
  </p>
  
  <li>
  <p>
  New: There is a second GridTools::partition_triangulation
  function that takes a cell connectivity pattern as argument, rather
  than computing it itself as the existing function. Use cases are
  discussed in the documentation of the new function.
  <br>
  (WB 2008/08/06)
  </p>

  <li>
  <p>
  Fixed: GridTools::find_cells_adjacent_to_vertex had a bug that 
  prevented its correct functioning in three dimensions. Some 
  cases were left out due to uncorrect assumptions on the various 
  refinement possibilities.
  <br>
  (Luca Heltai 2008/07/17)
  </p>

  <p>
  New: There is now a new
  Triangulation::prevent_distorted_boundary_cells function which is
  only useful in case of anisotropic refinement. At the boundary
  of the domain, the new point on the face may be far inside the
  current cell, if the boundary has a strong curvature. If we
  allow anistropic refinement here, the resulting cell may be
  strongly distorted, especially if it is refined again later
  on. To prevent this problem, this function flags such cells for
  isotropic refinement. It is called automatically from
  Triangulation::prepare_coarsening_and_refinement. Therefore
  this should have no effect on user codes.
  <br>
  (Tobias Leicht 2008/07/08 as of branch_anisotropic at 2006/08/02)
  </p>

  <li>
  <p>
  New: There is now a new Triangulation::create_children function
  which actually sets up the children of a cell and updates the
  neighbor information. This work has been done in
  Triangulation::execute_refinement so far. Memory allocation has
  to be done prior to the function call. However, this is really
  something internal to the library and there should never be the
  need to use this function in user_codes. Calling
  Triangulation::execute_coarsening_and_refinement will be all
  you need.
  <br> 
  (Tobias Leicht 2008/07/08 as of branch_anisotropic at 2006/06/29)
  </p>

  <li>
  <p>
  New: A part of the functionality of Triangulation::execute_coarsening has been implemented in a new way
  and shifted to the new function Triangulation::coarsening_allowed. This function decides,
  depending on the refinement situation of all a cells neighbors, if the
  cell may be coarsened, in which case the bool value <code>true</code> is
  returned. This is a functionality which is partly dimension dependend.
  <br>	
  However, there should never be any reason to use this function in user codes.
  <br> 
  (Tobias Leicht 2008/07/08 as of branch_anisotropic at 2006/06/28)
  </p>
  
  <li>
  <p>
  New: There is now a new CellAccessor::neighbor_is_coarser function,
  which returns a bool value <code>true</code>, if the requested
  neighbor is a coarser one and <code>false</code> if the
  neighbor is as refined as the current cell. This new
  functionality is needed in the internals of the library to
  decide, if <code>neighbor_of_neighbor</code> or
  <code>neighbor_of_coarser_neighbor</code> has to be
  called. This decision is trivial in case of isotropic
  refinement, where the level of the cell and its neighbor is all
  the information needed. In case of aniostropic refinement, this
  new function can be used.
  <br> 
  (Tobias Leicht 2008/07/08 as of branch_anisotropic at 2006/06/28)
  </p>

  <li>
  <p>
  New: There is now a new
  <code>Triangulation::MeshSmoothing::allow_anisotropic_smoothing</code>
  smoothing flag for triangulations. An important part of the
  smoothing process for refinement and coarsen flags is to
  ensure, that no double refinement takes places at any face. If
  a cell is refined twice, its neighbor has to be refined at
  least once. However, technically it is not always necessary to
  refine the neighbor isotropically, it may be sufficient to
  choose only one direction for the refinement. While this allows
  to reduce the number of newly created cells to a minimum, it is
  incompatible with older versions of the library, as anisotropic
  refinement takes place, even if all the explicitly set
  refinement flags ask for isotropic refinement.
  <br>
  Therefore this functionality is off by default. However, in order to use
  the new feature of anisotropic refinement to full extend, this flag should
  be set explicitly.
  <br>
  Note, that for reasons of backwards compatibility this flag is
  NOT included in the general <code>MeshSmoothing::maximum_smoothing</code>
  flag.
  <br> 
  (Tobias Leicht 2008/07/08 as of branch_anisotropic at 2006/06/28)
  </p>
  
  <li>
  <p>
  New: There is now a new
  TriaObjectAccessor::clear_refinement_case function.
  <br> 
  (RH 2008/07/08 as of branch_anisotropic at 2005/12/19)
  </p>

  <li>
  <p>
  Extended: The CellAccessor::neighbor_of_neighbor function is
  now extended to anisotropic refinement.
  <br> 
  (RH 2008/07/08 as of branch_anisotropic at 2005/12/15)
  </p>

  <li>
  <p>
  Extended (internal): Lines on <code>level>0</code> have always
  been stored pairwise. In order to allow the creation of single
  interior lines we now also allow storage of single lines. The
  <tt>TriangulationLevel<1>::reserve_space</tt> function
  now takes an additional <code>n_consecutive_lines</code>
  parameter which allows to create new single as well as pairs
  for lines <code>n_consecutive_lines=1</code> or
  <code>n_consecutive_lines=2</code>, respectively.
  <br> 
  (RH 2008/07/08 as of branch_anisotropic at 2005/12/15)
  </p>

  <li>
  <p>
  Changed: When allowing anisotropic refinement it cannot be
  guaranteed that all children of a cell are stored in
  consecutive components of the TriaLevel::cells vector,
  respectively. It is only known that children are stored at
  least in pairs. Therefore, it is not sufficient any more to
  store the index of the first child of a cell, only. Now the
  indices of every second child, i.e. of the even numbered
  children, must be stored.  For this, the
  TriaObjectAccessor::set_children function for cells now has a
  new argument representing the number of the child for which the
  index shall be set. This function can only be called for even
  numbered children. The same applies to the respective function
  for faces.
  <br> 
  Finally, we note that these functions are for internal use,
  only.
  <br> 
  (RH 2008/07/08 as of branch_anisotropic at 2005/03/03)
  </p>

  <li>
  <p>
  Changed: The CellAccessor::refine_flag_set function now returns
  a RefinementCase argument instead of a boolean. Be aware, that
  you now still can ask <code>if(!cell->refine_flag_set())</code>
  , <code>if(cell->refine_flag_set()==false)</code> and
  <code>if(cell->refine_flag_set())</code> , but you cannot ask
  <code>if(cell->refine_flag_set()==true)</code> any more.
  <br>
  (RH 2008/07/08 as of branch_anisotropic at 2005/03/03)
  </p>

  <li>
  <p>
  Extended: The CellAccessor::set_refine_flag function now has a
  new RefinementCase argument which defaults to
  <tt>GeometryInfo<dim>::isotropic_refinement</tt>.
  <br>
  (RH 2008/07/08 as of branch_anisotropic at 2005/03/03)
  </p>

  <li>
  <p>
  New: There is now a new class RefinementCase defined in
  <code>geometry_info.h</code>, which allows to describe all
  possible (anisotropic and isotropic) refinement cases in (1,) 2
  and 3 dimensions.
  <br> 
  (RH 2008/07/08 as of branch_anisotropic at 2005/02/26)
  </p>

  <li>
  <p>
  New: There is now a new TriaObjectAccessor::n_children function
  which returns the number of children of the cell or face it was
  invoked for.
  <br> 
  (RH 2008/07/08 as of branch_anisotropic at 2005/02/26)
  </p>

  <li>
  <p>
  New: There is now a new TriaObjectAccessor::refinement_case
  function which returns the RefinementCase a cell or face is
  refined with.
  <br> 
  (RH 2008/07/08 as of branch_anisotropic at 2005/02/26)
  </p>

  <li>
  <p>
  New (for internal use only): There is now a new
  TriaObjectAccessor::set_refinement_case function. This function
  is mainly for internal use (required by
  Triangulation::execute_coarsening_and_refinement). It is not
  provided for <code>dim=1</code> as there the refinement case
  defaults to isotropic refinement.
  <br>
  Note, that users should still use the
  CellAccessor::set_refine_flag function for setting the
  RefinementCase of cells to be refined within the next
  Triangulation::execute_coarsening_and_refinement function.
  <br>
  (RH 2008/07/08 as of branch_anisotropic at 2005/02/26)
  </p>

  <li>
  <p>
  New: New <code>vector@<RefinementCase@> refinement_cases</code>
  vectors have been introduced in TriaObjects. For memory
  efficiency (but with a penalty on run-time) they might be
  replaced by <code>vector@<vector@<bool@> @> (dim, vector@<bool@>
  (n_quads/n_hexes))</code>, later.
  <br> 
  (RH 2008/07/08 as of branch_anisotropic at 2005/02/26)
  </p>

  <li>
  <p>
  Fixed: When using a higher order mapping of degree at least 3 (i.e.
  the MappingQ class) on meshes that have cells with non-standard
  face orientation and that touch the boundary of the domain, then
  some interpolation points were wrongly computed.
  <br>
  (Tobias Leicht, Timo Heister, WB 2008/06/10)
  </p>
  </li>

  <li>
  <p>
  Fixed: The ParameterHandler::get_integer function now throws an
  exception when called for non-integer parameters. This exception has
  been commented out some time ago but is now reincluded.
  <br>
  (RH 2008/06/11)
  </p>
  </li>
</ol>


*/
