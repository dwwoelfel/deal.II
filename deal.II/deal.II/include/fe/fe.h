/*----------------------------   fe.h     ---------------------------*/
/*      $Id$                 */
#ifndef __fe_H
#define __fe_H
/*----------------------------   fe.h     ---------------------------*/

#include <base/exceptions.h>
#include <grid/point.h>
#include <grid/dof.h>
#include <lac/dfmatrix.h>



template <int dim> class Boundary;
template <int dim> struct FiniteElementData;



/**
  Dimension dependent data for finite elements. See the #FiniteElementBase#
  class for more information.
  */
struct FiniteElementData<1> {
				     /**
				      * Number of degrees of freedom on
				      * a vertex.
				      */
    const unsigned int dofs_per_vertex;

				     /** Number of degrees of freedom
				      *  on a line.
				      */
    const unsigned int dofs_per_line;

				     /**
				      * Number of degrees of freedom on a
				      * face. This information is
				      * redundant to some fields in the
				      * derived classes but makes
				      * writing dimension independant
				      * programs easier.
				      */
    const unsigned int dofs_per_face;
    
				     /**
				      * Total number of degrees of freedom
				      * on a cell. This information is
				      * redundant to some fields in the
				      * derived classes but makes
				      * writing dimension independant
				      * programs easier.
				      */
    const unsigned int total_dofs;

    				     /**
				      * Default constructor. Constructs an element
				      * which is not so useful. Checking
				      * #total_dofs# is therefore a good way to
				      * check if something went wrong.
				      */
    FiniteElementData () :
		    dofs_per_vertex(0),
		    dofs_per_line(0),
		    dofs_per_face(0),
		    total_dofs(0) {};

				     /**
				      * A more useful version to construct
				      * an object of this type.
				      */
    FiniteElementData (const unsigned int dofs_per_vertex,
		       const unsigned int dofs_per_line) :
		    dofs_per_vertex(dofs_per_vertex),
		    dofs_per_line(dofs_per_line),
		    dofs_per_face(dofs_per_vertex),
		    total_dofs (2*dofs_per_vertex+dofs_per_line) {};

				     /**
				      * Comparison operator. It is not clear to
				      * me why we have to declare and implement
				      * this one explicitely.
				      */
    bool operator == (const FiniteElementData<1> &) const;
};




/**
  Dimension dependent data for finite elements. See the #FiniteElementBase#
  class for more information.
  */
struct FiniteElementData<2> {
				     /**
				      * Number of degrees of freedom on
				      * a vertex.
				      */
    const unsigned int dofs_per_vertex;

				     /** Number of degrees of freedom
				      *  on a line.
				      */
    const unsigned int dofs_per_line;

				     /** Number of degrees of freedom
				      *  on a quad.
				      */
    const unsigned int dofs_per_quad;

				     /**
				      * Number of degrees of freedom on a
				      * face. This information is
				      * redundant to some fields in the
				      * derived classes but makes
				      * writing dimension independant
				      * programs easier.
				      */
    const unsigned int dofs_per_face;
    
				     /**
				      * Total number of degrees of freedom
				      * on a cell. This information is
				      * redundant to some fields in the
				      * derived classes but makes
				      * writing dimension independant
				      * programs easier.
				      */
    const unsigned int total_dofs;

    				     /**
				      * Default constructor. Constructs an element
				      * which is not so useful. Checking
				      * #total_dofs# is therefore a good way to
				      * check if something went wrong.
				      */
    FiniteElementData () :
		    dofs_per_vertex(0),
		    dofs_per_line(0),
		    dofs_per_quad(0),
		    dofs_per_face(0),
		    total_dofs(0) {};

				     /**
				      * A more useful version to construct
				      * an object of this type.
				      */
    FiniteElementData (const unsigned int dofs_per_vertex,
		       const unsigned int dofs_per_line,
		       const unsigned int dofs_per_quad) :
		    dofs_per_vertex(dofs_per_vertex),
		    dofs_per_line(dofs_per_line),
		    dofs_per_quad(dofs_per_quad),
		    dofs_per_face(2*dofs_per_vertex+
				  dofs_per_line),
		    total_dofs (4*dofs_per_vertex+
				4*dofs_per_quad+
				dofs_per_line) {};

				     /**
				      * Comparison operator. It is not clear to
				      * me why we have to declare and implement
				      * this one explicitely.
				      */
    bool operator == (const FiniteElementData<2> &) const;
};

    



/**
  Base class for finite elements in arbitrary dimensions. This class provides
  several fields which describe a specific finite element and which are filled
  by derived classes. It more or less only offers the fields and access
  functions which makes it possible to copy finite elements without knowledge
  of the actual type (linear, quadratic, etc).

  The implementation of this base class is split into two parts: those fields
  which are not common to all dimensions (#dofs_per_quad# for example are only
  useful for #dim>=2#) are put into the #FiniteElementData<dim># class which
  is explicitely specialized for all used dimensions, while those fields which
  may be formulated in a dimension-independent way are put into the present
  class.

  The different matrices are initialized with the correct size, such that in
  the derived (concrete) finite element classes, their entries must only be
  filled in; no resizing is needed.
  */
template <int dim>
struct FiniteElementBase : public FiniteElementData<dim> {
  public:
				     /**
				      * Construct an object of this type.
				      * You have to set the
				      * matrices explicitely after calling
				      * this base class' constructor. For
				      * #dim==1#, #dofs_per_quad# must be
				      * zero.
				      */
    FiniteElementBase (const unsigned int dofs_per_vertex,
		       const unsigned int dofs_per_line,
		       const unsigned int dofs_per_quad=0);
    
				     /**
				      * Return a readonly reference to the
				      * matrix which describes the transfer of a
				      * child with the given number to the
				      * mother cell.
				      */
    const dFMatrix & restrict (const unsigned int child) const;

				     /**
				      * Return a readonly reference to the
				      * matrix which describes the transfer of a
				      * mother cell to the child with the given
				      * number.
				      */
    const dFMatrix & prolongate (const unsigned int child) const;

				     /**
				      * Return a readonly reference to the
				      * matrix which describes the constraints
				      * at the interface between a refined and
				      * an unrefined cell.
				      *
				      * The matrix is obviously empty in only
				      * one space dimension, since there are no
				      * constraints then.
				      */
    const dFMatrix & constraints () const;
    
				     /**
				      * Comparison operator. We also check for
				      * equality of the constraint matrix,
				      * which is quite an expensive operation.
				      * Do therefore
				      * use this function with care, if possible
				      * only for debugging purposes.
				      *
				      * Since this function is not that important,
				      * we avoid an implementational question
				      * about comparing arrays and do not compare
				      * the matrix arrays #restriction# and
				      * #prolongation#.
				      */
    bool operator == (const FiniteElementBase<dim> &) const;

				     /**
				      * Exception
				      */
    DeclException1 (ExcInvalidIndex,
		    int,
		    << "Invalid index " << arg1);
				     /**
				      * Exception
				      */
    DeclException2 (ExcWrongFieldDimension,
		    int, int,
		    << "The field has not the assumed dimension " << arg2
		    << ", but has " << arg1 << " elements.");
    DeclException2 (ExcWrongInterfaceMatrixSize,
		    int, int,
		    << "The interface matrix has a size of " << arg1
		    << "x" << arg2
		    << ", which is not reasonable in the present dimension.");
				     /**
				      * Exception
				      */
    DeclException0 (ExcInternalError);
    
  protected:
				     /**
				      * Have #N=2^dim# matrices keeping the
				      * restriction constants for the transfer
				      * of the #i#th child to the mother cell.
				      * The numbering conventions for the
				      * degree of freedom indices are descibed
				      * in the derived classes.
				      * In this matrix, the row indices belong
				      * to the destination cell, i.e. the
				      * unrefined one, while the column indices
				      * are for the refined cell's degrees of
				      * freedom.
				      *
				      * Upon assembling the transfer matrix
				      * between cells using this matrix array,
				      * zero elements in the restriction
				      * matrix are discarded and will not fill
				      * up the transfer matrix.
				      */
    dFMatrix restriction[(1<<dim)];

    				     /**
				      * Have #N=2^dim# matrices keeping the
				      * prolongation constants for the transfer
				      * of the mother cell to the #i#th child.
				      * The numbering conventions for the
				      * degree of freedom indices are descibed
				      * in the derived classes.
				      * In this matrix, the row indices belong
				      * to the destination cell, i.e. the
				      * refined one, while the column indices
				      * are for the unrefined cell's degrees of
				      * freedom.
				      *
				      * Upon assembling the transfer matrix
				      * between cells using this matrix array,
				      * zero elements in the prolongation
				      * matrix are discarded and will not fill
				      * up the transfer matrix.
				      */
    dFMatrix prolongation[(1<<dim)];

    				     /**
				      * Specify the constraints which the
				      * dofs on the two sides of a cell interface
				      * underly if the line connects two
				      * cells of which one is refined once.
				      *
				      * For further details see the general
				      * description of the derived class.
				      *
				      * This field is obviously useless in one
				      * space dimension.
				      */
    dFMatrix interface_constraints;
};







/**
  Finite Element in any dimension. This class declares the functionality
  to fill the fields of the #FiniteElementBase# class. Since this is
  something that depends on the actual finite element, the functions are
  declared virtual if it is not possible to provide a reasonable standard
  implementation.


  {\bf Finite Elements in one dimension}

  Finite elements in one dimension need only set the #restriction# and
  #prolongation# matrices in #FiniteElementBase<1>#. The constructor of
  this class in one dimension presets the #interface_constraints# matrix
  by the unit matrix with dimension one. Changing this behaviour in
  derived classes is generally not a reasonable idea and you risk getting
  in terrible trouble.
  
  
  {\bf Finite elements in two dimensions}
  
  In addition to the fields already present in 1D, a constraint matrix
  is needed in case two quads meet at a common line of which one is refined
  once more than the other one. Then there are constraints referring to the
  hanging nodes on that side of the line which is refined. These constraints
  are represented by a $n\times m$-matrix #line_constraints#, where $n$ is the
  number of degrees of freedom on the refined side (those dofs on the middle
  vertex plus those on the two lines), and $m$ is that of the unrefined side
  (those dofs on the two vertices plus those on the line). The matrix is thus
  a rectangular one.

  The mapping of the dofs onto the indices of the matrix is as follows:
  let $d_v$ be the number of dofs on a vertex, $d_l$ that on a line, then
  $m=0...d_v-1$ refers to the dofs on vertex zero of the unrefined line,
  $m=d_v...2d_v-1$ to those on vertex one,
  $m=2d_v...2d_v+d_l-1$ to those on the line.

  Similarly, $n=0...d_v-1$ refers to the dofs on the middle vertex
  (vertex one of child line zero, vertex zero of child line one),
  $n=d_v...d_v+d_l-1$ refers to the dofs on child line zero,
  $n=d_v+d_l...d_v+2d_l-1$ refers to the dofs on child line one.
  Please note that we do not need to reserve space for the dofs on the
  end vertices of the refined lines, since these must be mapped one-to-one
  to the appropriate dofs of the vertices of the unrefined line.

  It should be noted that it is not possible to distribute a constrained
  degree of freedom to other degrees of freedom which are themselves
  constrained. Only one level of indirection is allowed. It is not known
  at the time of this writing whether this is a constraint itself.
  */
template <int dim>
class FiniteElement : public FiniteElementBase<dim> {
  public:
				     /**
				      * Constructor
				      */
    FiniteElement (const unsigned int dofs_per_vertex,
		   const unsigned int dofs_per_line,
		   const unsigned int dofs_per_quad=0) :
		    FiniteElementBase<dim> (dofs_per_vertex,
					    dofs_per_line,
					    dofs_per_quad) {};

				     /**
				      * Destructor. Only declared to have a
				      * virtual destructor which the compiler
				      * wants to have.
				      */
    virtual ~FiniteElement () {};
    
				     /**
				      * Return the value of the #i#th shape
				      * function at the point #p#.
				      * #p# is a point on the reference element.
				      */
    virtual double shape_value (const unsigned int i,
			        const Point<dim>& p) const = 0;

				     /**
				      * Return the gradient of the #i#th shape
				      * function at the point #p#.
				      * #p# is a point on the reference element,
				      */
    virtual Point<dim> shape_grad (const unsigned int i,
				   const Point<dim>& p) const = 0;

				     /**
				      * Compute the Jacobian matrix and the
				      * quadrature points as well as the ansatz
				      * function locations on the real cell in
				      * real space from the given cell
				      * and the given quadrature points on the
				      * unit cell. The Jacobian matrix is to
				      * be computed at every quadrature point.
				      * This function has to be in the finite
				      * element class, since different finite
				      * elements need different transformations
				      * of the unit cell to a real cell.
				      *
				      * The computation of the three fields may
				      * share some common code, which is why we
				      * put it in one function. However, it may
				      * not always be necessary to really
				      * compute all fields, so there are
				      * bool flags which tell the function which
				      * of the fields to actually compute.
				      *
				      * Refer to the documentation of the
				      * \Ref{FEValues} class for a definition
				      * of the Jacobi matrix and of the various
				      * structures to be filled.
				      *
				      * It is provided for the finite element
				      * class in one space dimension, but for
				      * higher dimensions, it depends on the
				      * present fe and needs reimplementation
				      * by the user. This is due to the fact
				      * that the user may want to use
				      * iso- or subparametric mappings of the
				      * unit cell to the real cell, which
				      * makes things much more complicated.
				      *
				      * The function assumes that the fields
				      * already have the right number of
				      * elements.
				      *
				      * This function is more or less an
				      * interface to the #FEValues# class and
				      * should not be used by users unless
				      * absolutely needed.
				      */
    virtual void fill_fe_values (const DoFHandler<dim>::cell_iterator &cell,
				 const vector<Point<dim> >            &unit_points,
				 vector<dFMatrix>    &jacobians,
				 const bool           compute_jacobians,
				 vector<Point<dim> > &ansatz_points,
				 const bool           compute_ansatz_points,
				 vector<Point<dim> > &q_points,
				 const bool           compute_q_points,
				 const Boundary<dim> &boundary) const;

				     /**
				      * Do the same thing that the other
				      * #fill_fe_values# function does,
				      * exception that a face rather than
				      * a cell is considered. The #face_no#
				      * parameter denotes the number of the
				      * face to the given cell to be
				      * considered.
				      *
				      * The unit points for the quadrature
				      * formula are given on the unit face
				      * which is a mannifold of dimension
				      * one less than the dimension of the
				      * cell. The #global_unit_points# 
				      * denote the position of the unit points
				      * on the selected face on the unit cell.
				      * This additional information is passed
				      * since the #FEFaceValues# class can
				      * compute them once and for all,
				      * eliminating the need to recompute it
				      * each time #FEFaceValues::reinit# is
				      * called.
				      *
				      * The jacobian matrix is evaluated at
				      * each of the quadrature points on the
				      * given face. The matrix is the
				      * transformation matrix of the unit cell
				      * to the real cell, not from the unit
				      * face to the real face. This is the
				      * necessary matrix to compute the real
				      * gradients.
				      *
				      * Conversely, the Jacobi determinants
				      * are the determinants of the
				      * transformation from the unit face to
				      * the real face. This information is
				      * needed to actually perform integrations
				      * along faces. Note that we here return
				      * the inverse of the determinant of the
				      * jacobi matrices as explained in the
				      * documentation of the #FEValues# class.
				      * 
				      * The ansatz points are the
				      * off-points of those ansatz functions
				      * located on the given face; this
				      * information is taken over from the
				      * #get_face_ansatz_points# function.
				      *
				      * The order of ansatz functions is the
				      * same as if it were a cell of dimension
				      * one less than the present. E.g. in
				      * two dimensions, the order is first
				      * the vertex functions (using the
				      * direction of the face induced by the
				      * given cell) then the interior functions.
				      * The same applies for the quadrature
				      * points which also use the standard
				      * direction of faces as laid down by
				      * the #Triangulation# class.
				      *
				      * There is a standard implementation for
				      * dimensions greater than one. It
				      * uses the #fill_fe_values()#
				      * function to retrieve the wanted
				      * information. Since this operation acts
				      * only on unit faces and cells it does
				      * not depend on a specific finite element
				      * transformation and is thus applicable
				      * for all finite elements and uses tha
				      * same mapping from the unit to the real
				      * cell as used for the other operations
				      * performed by the specific finite element
				      * class.
				      *
				      * Three fields remain to be finite element
				      * specific in this standard implementation:
				      * The jacobi determinants of the
				      * transformation from the unit face to the
				      * real face, the ansatz points
				      * and the outward normal vectors. For
				      * these fields, there exist pure
				      * virtual functions, #get_face_jacobians#,
				      * #get_face_ansatz_points# and
				      * #get_normal_vectors#.
				      *
				      * Though there is a standard
				      * implementation, there
				      * may be room for optimizations which is
				      * why this function is made virtual.
				      *
				      * Since any implementation for one
				      * dimension would be senseless, all
				      * derived classes should throw an error
				      * when called with #dim==1#.
				      *
				      * The function assumes that the fields
				      * already have the right number of
				      * elements.
				      *
				      * This function is more or less an
				      * interface to the #FEFaceValues# class
				      * and should not be used by users unless
				      * absolutely needed.
				      */
    virtual void fill_fe_face_values (const DoFHandler<dim>::cell_iterator &cell,
				      const unsigned int           face_no,
				      const vector<Point<dim-1> > &unit_points,
				      const vector<Point<dim> >   &global_unit_points,
				      vector<dFMatrix>    &jacobians,
				      const bool           compute_jacobians,
				      vector<Point<dim> > &ansatz_points,
				      const bool           compute_ansatz_points,
				      vector<Point<dim> > &q_points,
				      const bool           compute_q_points,
				      vector<double>      &face_jacobi_determinants,
				      const bool           compute_face_jacobians,
				      vector<Point<dim> > &normal_vectors,
				      const bool           compute_normal_vectors,
				      const Boundary<dim> &boundary) const;

				     /**
				      * This function does almost the same as
				      * the above one, with the difference that
				      * it considers the restriction of a finite
				      * element to a subface (the child of a
				      * face) rather than to a face. The number
				      * of the subface in the face is given by
				      * the #subface_no# parameter. The meaning
				      * of the other parameters is the same as
				      * for the #fill_fe_face_values# function.
				      *
				      * Since the usage of ansatz points on
				      * subfaces is not useful, it is excluded
				      * from the interface to this function.
				      *
				      * Like for the #fill_fe_face_values#
				      * function, there is a default
				      * implementation, using the
				      * #fill_fe_values# function. There may
				      * be better and more efficient solutions
				      * for a special finite element, which is
				      * why this function is made virtual.
				      *
				      * This function is more or less an
				      * interface to the #FESubfaceValues# class
				      * and should not be used by users unless
				      * absolutely needed.
				      */				       
    virtual void fill_fe_subface_values (const DoFHandler<dim>::cell_iterator &cell,
					 const unsigned int           face_no,
					 const unsigned int           subface_no,
					 const vector<Point<dim-1> > &unit_points,
					 const vector<Point<dim> >   &global_unit_points,
					 vector<dFMatrix>    &jacobians,
					 const bool           compute_jacobians,
					 vector<Point<dim> > &q_points,
					 const bool           compute_q_points,
					 vector<double>      &face_jacobi_determinants,
					 const bool           compute_face_jacobians,
					 vector<Point<dim> > &normal_vectors,
					 const bool           compute_normal_vectors,
					 const Boundary<dim> &boundary) const;

				     /**
				      * This function produces a subset of
				      * the information provided by the
				      * #fill_fe_face_values()# function,
				      * namely the ansatz function off-points
				      * of those ansatz functions located on
				      * the face. However, you should not try
				      * to implement this function using the
				      * abovementioned function, since usually
				      * that function uses this function to
				      * compute information.
				      *
				      * This function is excluded from the
				      * abovementioned one, since no information
				      * about the neighboring cell is needed,
				      * such that loops over faces alone are
				      * possible when using this function.
				      * This is useful for example if we want
				      * to interpolate boundary values to the
				      * finite element functions. If integration
				      * along faces is needed, we still need
				      * the #fill_fe_values# function.
				      *
				      * The function assumes that the
				      * #ansatz_points# array already has the
				      * right size. The order of points in
				      * the array matches that returned by
				      * the #face->get_dof_indices# function.
				      *
				      * Since any implementation for one
				      * dimension would be senseless, all
				      * derived classes should throw an error
				      * when called with #dim==1#.
				      */
    virtual void get_face_ansatz_points (const DoFHandler<dim>::face_iterator &face,
					 const Boundary<dim> &boundary,
					 vector<Point<dim> > &ansatz_points) const =0;

				     /**
				      * This is the second separated function
				      * described in the documentation of the
				      * #fill_fe_face_values# function. It
				      * returns the determinants of the
				      * transformation from the unit face to the
				      * real face at the
				      *
				      * Since any implementation for one
				      * dimension would be senseless, all
				      * derived classes should throw an error
				      * when called with #dim==1#.
				      */
    virtual void get_face_jacobians (const DoFHandler<dim>::face_iterator &face,
				     const Boundary<dim>         &boundary,
				     const vector<Point<dim-1> > &unit_points,
				     vector<double>      &face_jacobi_determinants) const =0;

				     /**
				      * Does the same as the above function,
				      * except that it computes the Jacobi
				      * determinant of the transformation from
				      * the unit face to the subface of #face#
				      * with number #subface_no#.
				      *
				      * The function needs not take special care
				      * about boundary approximation, since it
				      * must not be called for faces at the
				      * boundary.
				      */
    virtual void get_subface_jacobians (const DoFHandler<dim>::face_iterator &face,
					const unsigned int           subface_no,
					const vector<Point<dim-1> > &unit_points,
					vector<double>      &face_jacobi_determinants) const =0;

				     /**
				      * Compute the normal vectors to the cell
				      * at the quadrature points. See the
				      * documentation for the #fill_fe_face_values#
				      * function for more details. The function
				      * must guarantee that the length of the
				      * vectors be one.
				      *
				      * Since any implementation for one
				      * dimension would be senseless, all
				      * derived classes should throw an error
				      * when called with #dim==1#.
				      */
    virtual void get_normal_vectors (const DoFHandler<dim>::cell_iterator &cell,
				     const unsigned int          face_no,
				     const Boundary<dim>         &boundary,
				     const vector<Point<dim-1> > &unit_points,
				     vector<Point<dim> >         &normal_vectors) const =0;

				     /**
				      * Does the same as the above function,
				      * except that it refers to the
				      * subface #subface_no# of the given face.
				      *
				      * The function needs not take special care
				      * about boundary approximation, since it
				      * must not be called for faces at the
				      * boundary.
				      */
    virtual void get_normal_vectors (const DoFHandler<dim>::cell_iterator &cell,
				     const unsigned int           face_no,
				     const unsigned int           subface_no,
				     const vector<Point<dim-1> > &unit_points,
				     vector<Point<dim> >         &normal_vectors) const =0;

				     /**
				      * Exception
				      */
    DeclException0 (ExcPureFunctionCalled);
				     /**
				      * Exception
				      */
    DeclException0 (ExcNotImplemented);
				     /**
				      * Exception
				      */
    DeclException0 (ExcBoundaryFaceUsed);
};




  
/*----------------------------   fe.h     ---------------------------*/
/* end of #ifndef __fe_H */
#endif
/*----------------------------   fe.h     ---------------------------*/
