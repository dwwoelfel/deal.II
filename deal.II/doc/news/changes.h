/**
// * @page changes_after_8_0 Changes after Version 8.0

<p>
This is the list of changes made after the release of
deal.II version 8.0.0.
All entries are signed with the names of the authors.
</p>



<!-- ----------- INCOMPATIBILITIES ----------------- -->

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
  Removed: The member function face_to_equivalent_cell_index() in
  FiniteElementData has been removed. It had been deprecated a while
  back already. Please use FiniteElement::face_to_cell_index() instead.
  <br>
  (Wolfgang Bangerth, 2013/08/09)
  </li>

  <li>
  Changed: The typedefs DataOut::cell_iterator and
  DataOut::active_cell_iterator were previously defined as
  DoFHandler::(active)_cell_iterator, while they are now
  Triangulation::(active)_cell_iterator. This is necessary to support DataOut
  on multiple DoFHandler objects. This affects possible overloading of
  DataOut::next_cell(cell_iterator). Use the typedef
  DataOut::(active)_cell_iterator as argument type instead.
  <br>
  (Martin Kronbichler, 2013/07/24)
  </li>
</ol>


<!-- ----------- GENERAL IMPROVEMENTS ----------------- -->

<a name="general"></a>
<h3>General</h3>


<ol>
  <li>
  New: It is now possible to compile and link deal.II against LLVM's libcxx. For
  this, a few issues with C++ standard violations are resolved.
  <br>
  (Matthias Maier, 2013/08/09)
  </li>
</ol>


<!-- ----------- SPECIFIC IMPROVEMENTS ----------------- -->

<a name="specific"></a>
<h3>Specific improvements</h3>

<ol>
  <li>
  Fixed: <code>common/Make.global_options</code> now exports enable-threads
  correctly, furthermore, <code>lib-suffix</code>, <code>shared-lib-suffix</code>
  and <code>static-lib-suffix</code> are now exported as well for better legacy
  support.
  <br>
  (Matthias Maier, 2013/08/30)
  </li>

  <li>
  New: The ParameterHandler class can now deal with including one parameter
  file from another.
  <br>
  (Wolfgang Bangerth, 2013/08/25)
  </li>

  <li>
  New: The method VectorTools::compute_normal_flux_constraints can be used to
  force a vector finite element function to be normal to the boundary.
  <br>
  (Martin Kronbichler, 2013/08/23)
  </li>

  <li>
  Improved: MappingQ now uses the points of the Gauss-Lobatto quadrature
  formula as support points instead of equispaced ones. This allows its use
  for high polynomial orders and also gives better interpolation of circular
  boundaries. Beware that mappings of order three and higher will behave
  slightly differently now (usually better).
  <br>
  (Martin Kronbichler, 2013/08/23)
  </li>

  <li>
  Improved: Several .cc files in the deal.II directory have been split in
  order to better utilize multiple processors when compiling in parallel and
  reduce memory requirements of the compilation stage.
  <br>
  (Martin Kronbichler, 2013/08/22)
  </li>

  <li>
  Fixed: The ParameterHandler::declare_entry() did not check that the
  default value of a parameter indeed satisfies the pattern given for this
  parameter (despite a statement in the documentation that this checking
  would happen). This is now fixed.
  <br>
  (Wolfgang Bangerth, 2013/08/21)
  </li>

  <li>
  New: Patterns::List and Patterns::Map now accept a string
  different than the default comma that denotes the separator
  between entries of the list or map.
  <br>
  (Wolfgang Bangerth, 2013/08/21)
  </li>

  <li>
  Fixed: Some operations in the MappingQ class are now done in higher
  precision arithmetic to mitigate the ill-conditioning that appears
  when using mappings of high order (say, order 6 or 8 or 10).
  <br>
  (Juan Carlos Araujo Cabarcas, 2013/08/20)
  </li>

  <li>
  Fixed: The SLEPcWrappers classes could not be compiled for 64-bit
  indices. This is now fixed.
  <br>
  (Denis Davydov, Wolfgang Bangerth, 2013/08/20)
  </li>

  <li>
  Fixed: SolutionTransfer used to crash whenever one transfered in the hp
  context between cells that use FE_Nothing and FE_Q. This is now fixed.
  <br>
  (Krzyszof Bzowski, Wolfgang Bangerth, 2013/08/18)
  </li>

  <li>
  Fixed: Under some circumstances (see http://code.google.com/p/dealii/issues/detail?id=82)
  the DoFTools::make_periodicity_constraints() function could create cycles in
  the ConstraintMatrix object. This is now fixed.
  <br>
  (David Emerson, Wolfgang Bangerth, 2013/08/16)
  </li>

  <li>
  New: There is now a function ConstraintMatrix::are_identity_constrained().
  <br>
  (Wolfgang Bangerth, 2013/08/16)
  </li>

  <li>
  New: TableHandler::write_text() now also supports output in
  org-mode (http://orgmode.org/) format via a new entry in the
  TableHandler::TextOutputFormat enumeration.
  <br>
  (Oleh Krehel, 2013/08/15)
  </li>

  <li>
  New: There are now global functions <code>scalar_product</code>
  that compute the scalar product (double contraction) between
  tensors of rank 2.
  <br>
  (Scott Miller, 2013/08/14)
  </li>

  <li>
  Fixed: Creating objects of type MappingQ was previously only possible
  for low order polynomials. For orders higher than around 6, one ran
  into assertions that tested for internal consistency. These assertions
  have now been appropriately relaxes for the growth of round-off errors
  with growing polynomial degrees.
  <br>
  (Juan Carlos Araujo Cabarcas, Wolfgang Bangerth, 2013/08/14)
  </li>

  <li>
  New: MappingQEulerian is now also instantiated for vector elements
  of type TrilinosWrappers::Vector as well as the MPI and block
  variants.
  <br>
  (Armin Ghajar Jazi, 2013/08/14)
  </li>

  <li>
  Fixed: The FiniteElement::face_to_cell_index() function had a bug
  that made it work incorrectly for elements that have more than one
  degree of freedom per line (in 2d) or per quad (in 3d). This is now
  fixed for the most common cases, namely the FE_Q elements as well
  as elements composed of FESystem elements. For all other cases, an
  exception is generated reporting that this case is not implemented.
  If you run into this, let us know.
  <br>
  (Wolfgang Bangerth, 2013/08/10)
  </li>

  <li>
  New: DataOutBase::VtkFlags now has a flag
  DataOutBase::VtkFlags::print_date_and_time that can be used to suppress output
  of date and time in output files. This is useful in test suites where a newer
  run at a different time produces differences against previously stored files,
  even though the actual data is exactly the same.
  <br>
  (Oleh Krehel, 2013/08/06)
  </li>

  <li>
  Fixed: The various block matrix classes are all derived from BlockMatrixBase
  which had race conditions when the set() or add() functions were called from
  different threads. This is now fixed.
  <br>
  (Wolfgang Bangerth, 2013/08/05)
  </li>

  <li>
  Fixed: various fixes with assignment and reinit of PETScWrappers::MPI::Vector.
  <br>
  (Timo Heister, 2013/08/05)
  </li>

  <li>Fixed: An assertion wrongly triggered in
  DoFTools::make_hanging_node_constraints when used with a particular
  combination of FESystem elements containing FE_Nothing. This is now fixed.
  <br>
  (Denis Davydov, Wolfgang Bangerth, 2013/08/01)
  </li>

  <li>
  New: Add has_ghost_elements() for PETScWrappers::MPI::BlockVector and
  TrilinosWrappers::MPI::BlockVector.
  <br>
  (Timo Heister, 2013/08/01)
  </li>

  <li>
  SparsityTools::distribute_sparsity_pattern did not work correctly for
  block systems, this has been fixed (function has a different signature).
  <br>
  (Timo Heister, 2013/07/31)
  </li>

  <li>Fixed: When typing <code>make run</code> in the step-32 directory,
  the program was executed with <code>mpirun -np 2 ./step-32</code>. This
  assumes that a program <code>mpirun</code> exists, but also does that
  deal.II was in fact compiled with MPI support on. Neither was intended.
  This is now fixed.
  <br>
  (Wolfgang Bangerth, 2013/07/24)
  </li>

  <li>New: The DataOut, DataOutFaces, and DataOutRotation classes now allow
  the output of data vectors using different DoFHandler objects (based on the
  same triangulation), by new functions add_data_vector. This is used in the
  step-31 tutorial program which avoids creating a joint DoFHandler just for
  output.
  <br>
  (Martin Kronbichler, 2013/07/24)
  </li>

  <li>Changed: GridGenerator used to be a class with only static members
  but is now a namespace, like all other similar constructs in deal.II.
  <br>
  (Wolfgang Bangerth, 2013/07/24)
  </li>

  <li>Changed: In GridGenerator, several functions had erroneously been changed
  to take an argument of type <code>size_type</code> rather than <code>unsigned
  int</code>. <code>GridGenerator::size_type</code> was a typedef to
  types::global_dof_index, which for most users was <code>unsigned int</code>
  anyway, but could also be set to be a 64-bit integer type. In any case, the
  change has been reverted and these functions take just a regular
  <code>unsigned int</code> again.
  <br>
  (Wolfgang Bangerth, 2013/07/24)
  </li>
</ol>


*/
