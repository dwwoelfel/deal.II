#ifndef LOCAL_ASSEMBLE_MASS_MATRIX
#define LOCAL_ASSEMBLE_MASS_MATRIX

#include "local_assemble.h"

#include <deal.II/base/logstream.h>
#include <deal.II/base/smartpointer.h>
#include <deal.II/lac/vector.h>
#include <deal.II/lac/full_matrix.h>
#include <deal.II/fe/fe_values.h>
#include <deal.II/fe/fe.h>

template <int dim, typename DH>
class LocalAssembleMass : 
public LocalAssembleBase<dim, DH>
{
 public:
  
  LocalAssembleMass(); 
  
  ~LocalAssembleMass(); 

  virtual void  assemble_cell_term
    (const typename DH::active_cell_iterator& cell, 
     FullMatrix<double> &);
  
  virtual void assemble_rhs_term
    (const typename DH::active_cell_iterator&,
     Vector<double> &);
  
  void reinit(FiniteElement<dim>&,
	      Function<dim> &);
	
 private:
  /** A pointer to the finite element.*/
  SmartPointer<FiniteElement<dim> > fe;

  /** A pointer to the rhs function.*/
  SmartPointer<Function<dim> > rhs;

  /** A pointer to fe_values objects. */
  SmartPointer<FEValues<dim> > fe_v;
};

#endif
