//---------------------------------------------------------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 2008 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------


#include <lac/trilinos_vector.h>

#include <cmath>

#ifdef DEAL_II_USE_TRILINOS

#include <Epetra_Import.h>


DEAL_II_NAMESPACE_OPEN

namespace TrilinosWrappers
{
  namespace MPI
  {


    Vector::Vector ()
                   :
#ifdef DEAL_II_COMPILER_SUPPORTS_MPI
                   map (0,0,Epetra_MpiComm(MPI_COMM_WORLD))
#else
		   map (0,0,Epetra_SerialComm())
#endif
    {
      last_action = Zero;
      vector = std::auto_ptr<Epetra_FEVector> (new Epetra_FEVector(map));
    }


  
    Vector::Vector (const Epetra_Map &InputMap)
                    :
		    map (InputMap)
    {
      last_action = Zero;
      vector = std::auto_ptr<Epetra_FEVector> (new Epetra_FEVector(map));
    }
  

  
    Vector::Vector (const Vector &v)
                    :
                    VectorBase(),
		    map (v.map)
    {
      last_action = Zero;
      vector = std::auto_ptr<Epetra_FEVector> (new Epetra_FEVector(*v.vector));
    }



    Vector::Vector (const Epetra_Map &InputMap,
		    const VectorBase &v)
                    :
                    VectorBase(),
		    map (InputMap)
    {
      AssertThrow (map.NumGlobalElements() == v.vector->Map().NumGlobalElements(),
		   ExcDimensionMismatch (map.NumGlobalElements(),
					 v.vector->Map().NumGlobalElements()));

      last_action = Zero;
      
      if (map.SameAs(v.vector->Map()) == true)
	vector = std::auto_ptr<Epetra_FEVector> (new Epetra_FEVector(*v.vector));
      else
	{
	  vector = std::auto_ptr<Epetra_FEVector> (new Epetra_FEVector(map));
	  reinit (v, false, true);
	}

    }



    void
    Vector::reinit (const Epetra_Map &input_map,
		    const bool        fast)
    {
      vector.reset();
      map = input_map;
      (void)fast;
      
      vector = std::auto_ptr<Epetra_FEVector> (new Epetra_FEVector(map));
      last_action = Zero;
    }



    void
    Vector::reinit (const VectorBase &v,
		    const bool        fast,
		    const bool        allow_different_maps)
    {
					// In case we do not allow to
					// have different maps, this
					// call means that we have to
					// reset the vector. So clear
					// the vector, initialize our
					// map with the map in v, and
					// generate the vector.
      if (allow_different_maps == false)
        {
	  vector.reset();
	  if (map.SameAs(v.vector->Map()) == false)
	    map = Epetra_Map(v.vector->Map().NumGlobalElements(),
			     v.vector->Map().NumMyElements(),
			     v.vector->Map().IndexBase(),
			     v.vector->Map().Comm());

	  vector = std::auto_ptr<Epetra_FEVector> (new Epetra_FEVector(map));
	  last_action = Zero;
	}

					// Otherwise, we have to check
					// that the two vectors are
					// already of the same size,
					// create an object for the data
					// exchange and then insert all
					// the data. The first assertion
					// is only a check whether the
					// user knows what she is doing.
      else
        {
	  Assert (fast == false,
		  ExcMessage ("It is not possible to exchange data with the "
			      "option fast set, which would not write "
			      "elements."));

	  AssertThrow (size() == v.size(),
		       ExcDimensionMismatch (size(), v.size()));

	  Epetra_Import data_exchange (vector->Map(), v.vector->Map());

	  const int ierr = vector->Import(*v.vector, data_exchange, Insert);
	  AssertThrow (ierr == 0, ExcTrilinosError(ierr));

	  last_action = Insert;
	}

    }



    Vector &
    Vector::operator = (const Vector &v)
    {
      if (vector->Map().SameAs(v.vector->Map()) == true)
	*vector = *v.vector;
      else
	{
	  vector.reset();
	  map = Epetra_Map(v.vector->Map().NumGlobalElements(),
			   v.vector->Map().NumMyElements(),
			   v.vector->Map().IndexBase(),
			   v.vector->Map().Comm());
	  vector = std::auto_ptr<Epetra_FEVector> (new Epetra_FEVector(*v.vector));
	}

      return *this;
    }



    Vector &
    Vector::operator = (const TrilinosWrappers::Vector &v)
    {
      Assert (size() == v.size(), ExcDimensionMismatch(size(), v.size()));

      Epetra_Import data_exchange (vector->Map(), v.vector->Map());
      const int ierr = vector->Import(*v.vector, data_exchange, Insert);

      AssertThrow (ierr == 0, ExcTrilinosError(ierr));

      last_action = Insert;

      return *this;
    }



    void
    Vector::do_data_exchange (const TrilinosWrappers::SparseMatrix &m,
			      const Vector                         &v)
    {
      Assert (m.matrix->Filled() == true,
	      ExcMessage ("Matrix is not compressed. "
			  "Cannot find exchange information!"));
      Assert (v.vector->Map().UniqueGIDs() == true,
	      ExcMessage ("The input vector has overlapping data, "
			  "which is not allowed."));

      if (vector->Map().SameAs(m.matrix->ColMap()) == false)
	{
	  map = m.matrix->ColMap();
	  vector = std::auto_ptr<Epetra_FEVector> (new Epetra_FEVector(map));
	}

      Epetra_Import data_exchange (vector->Map(), v.vector->Map());
      const int ierr = vector->Import(*v.vector, data_exchange, Insert);

      AssertThrow (ierr == 0, ExcTrilinosError(ierr));

      last_action = Insert;
    }

  } /* end of namespace MPI */




  Vector::Vector ()
                 :
#ifdef DEAL_II_COMPILER_SUPPORTS_MPI
                 map (0, 0, Epetra_MpiComm(MPI_COMM_WORLD))
#else
		 map (0, 0, Epetra_SerialComm())
#endif
  {
    last_action = Zero;
    vector = std::auto_ptr<Epetra_FEVector> (new Epetra_FEVector(map));
  }



  Vector::Vector (const unsigned int n)
                 :
#ifdef DEAL_II_COMPILER_SUPPORTS_MPI
                 map ((int)n, 0, Epetra_MpiComm(MPI_COMM_WORLD))
#else
		 map ((int)n, 0, Epetra_SerialComm())
#endif
  {
    reinit (n);
  }



  Vector::Vector (const Epetra_Map &InputMap)
                 :
                 map (InputMap.NumGlobalElements(), 0, InputMap.Comm())
  {
    last_action = Zero;
    vector = std::auto_ptr<Epetra_FEVector> (new Epetra_FEVector(map));
  }



  Vector::Vector (const VectorBase &v)
                 :
                 map (v.vector->Map().NumGlobalElements(), 0, v.vector->Comm())
  {
    last_action = Zero;
    vector = std::auto_ptr<Epetra_FEVector> (new Epetra_FEVector(map));

    if (vector->Map().SameAs(v.vector->Map()) == true)
      *vector = *v.vector;
    else
      reinit (v, false, true);

  }



  void
  Vector::reinit (const unsigned int n,
		  const bool         fast)
  {
    if (size() != n)
      {
	vector.reset();

#ifdef DEAL_II_COMPILER_SUPPORTS_MPI
	map = Epetra_LocalMap ((int)n, 0, Epetra_MpiComm(MPI_COMM_WORLD));
#else
	map = Epetra_LocalMap ((int)n, 0, Epetra_SerialComm());
#endif

	last_action = Zero;

	vector = std::auto_ptr<Epetra_FEVector> (new Epetra_FEVector (map));
      }
  }



  void
  Vector::reinit (const Epetra_Map &input_map,
                  const bool        fast)
  {
    vector.reset();

    if (map.NumGlobalElements() != input_map.NumGlobalElements())
      {
	map = Epetra_LocalMap (input_map.NumGlobalElements(),
			       input_map.IndexBase(),
			       input_map.Comm());
      }

    last_action = Zero;

    (void)fast;
    vector = std::auto_ptr<Epetra_FEVector> (new Epetra_FEVector (map));
  }



  void
  Vector::reinit (const VectorBase &v,
		  const bool        fast,
		  const bool        allow_different_maps)
  {
					// In case we do not allow to
					// have different maps, this
					// call means that we have to
					// reset the vector. So clear
					// the vector, initialize our
					// map with the map in v, and
					// generate the vector.
    if (allow_different_maps == false)
      {
	vector.reset();
	if (map.SameAs(v.vector->Map()) == false)
	  map = Epetra_LocalMap (v.vector->GlobalLength(),
				 v.vector->Map().IndexBase(),
				 v.vector->Comm());

	vector = std::auto_ptr<Epetra_FEVector> (new Epetra_FEVector(map));
	last_action = Zero;
      }

					// Otherwise, we have to check
					// that the two vectors are
					// already of the same size,
					// create an object for the data
					// exchange and then insert all
					// the data.
    else
      {
	Assert (fast == false,
		ExcMessage ("It is not possible to exchange data with the "
			    "option fast set, which would not write "
			    "elements."));

	AssertThrow (size() == v.size(),
		     ExcDimensionMismatch (size(), v.size()));

	Epetra_Import data_exchange (vector->Map(), v.vector->Map());

	const int ierr = vector->Import(*v.vector, data_exchange, Insert);
	AssertThrow (ierr == 0, ExcTrilinosError(ierr));

	last_action = Insert;
      }

  }



  Vector &
  Vector::operator = (const MPI::Vector &v)
  {
    if (size() != v.size())
      {
	map = Epetra_LocalMap (v.vector->Map().NumGlobalElements(), 
			       v.vector->Map().IndexBase(),
			       v.vector->Comm());
	vector = std::auto_ptr<Epetra_FEVector> (new Epetra_FEVector(map));
      }

    reinit (v, false, true);
    return *this;
  }



  Vector &
  Vector::operator = (const Vector &v)
  {
    if (vector->Map().SameAs(v.vector->Map()) == false)
      {
	map = Epetra_LocalMap (v.vector->Map().NumGlobalElements(), 
			       v.vector->Map().IndexBase(),
			       v.vector->Comm());
	vector = std::auto_ptr<Epetra_FEVector> (new Epetra_FEVector(map));
      }

    *vector = *v.vector;

    return *this;
  }
  
}

DEAL_II_NAMESPACE_CLOSE

#endif // DEAL_II_USE_TRILINOS
