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
	  vector = std::auto_ptr<Epetra_FEVector> (new Epetra_FEVector(map, false));
	  reinit (v, false, true);
	}

    }



    void
    Vector::reinit (const Epetra_Map &input_map,
		    const bool        fast)
    {
      vector.reset();
      map = input_map;
      
      vector = std::auto_ptr<Epetra_FEVector> (new Epetra_FEVector(map,!fast));
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
	    map = Epetra_Map(v.vector->GlobalLength(),0,v.vector->Comm());

	  vector = std::auto_ptr<Epetra_FEVector> (new Epetra_FEVector(map,!fast));
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
	  map = v.map;
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

      return *this;
    }



    template <typename Number>
    Vector &
    Vector::operator = (const ::dealii::Vector<Number> &v)
    {
      Assert (size() == v.size(),
	      ExcDimensionMismatch(size(), v.size()));

      vector = std::auto_ptr<Epetra_FEVector> (new Epetra_FEVector(map, false));
      
      const int min_my_id = map.MinMyGID();
      const unsigned int size = map.NumMyElements();

      Assert (map.MaxLID() == size-1,
	      ExcDimensionMismatch(map.MaxLID(), size-1));

      std::vector<int> indices (size);
      for (unsigned int i=0; i<size; ++i)
	indices[i] = map.GID(i);


      const int ierr = vector->ReplaceGlobalValues(size, 0, v.begin()+min_my_id, 
						   &indices[0]);
      AssertThrow (ierr == 0, ExcTrilinosError());

      return *this;
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
    vector = std::auto_ptr<Epetra_FEVector>
	              (new Epetra_FEVector(map,false));
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
    vector = std::auto_ptr<Epetra_FEVector>
	              (new Epetra_FEVector(map,true));
  }



  Vector::Vector (const VectorBase &v)
                 :
                 map (v.vector->Map().NumGlobalElements(), 0, v.vector->Comm())
  {
    last_action = Zero;
    vector = std::auto_ptr<Epetra_FEVector> (new Epetra_FEVector(map,false));

    if (vector->Map().SameAs(v.vector->Map()) == true)
      *vector = *v.vector;
    else
      reinit (v, false, true);

  }



  void
  Vector::reinit (const unsigned int n)
  {
    vector.reset();

    if (map.NumGlobalElements() != (int)n)
      {
#ifdef DEAL_II_COMPILER_SUPPORTS_MPI
	map = Epetra_LocalMap ((int)n, 0, Epetra_MpiComm(MPI_COMM_WORLD));
#else
	map = Epetra_LocalMap ((int)n, 0, Epetra_SerialComm());
#endif
      }

    last_action = Zero;

    vector = std::auto_ptr<Epetra_FEVector> (new Epetra_FEVector (map,true));
  }



  void
  Vector::reinit (const Epetra_Map &input_map,
                  const bool        fast)
  {
    vector.reset();

    if (map.NumGlobalElements() != input_map.NumGlobalElements())
      {
	map = Epetra_LocalMap (input_map.NumGlobalElements(),0,
			       input_map.Comm());
      }

    last_action = Zero;

    vector = std::auto_ptr<Epetra_FEVector> (new Epetra_FEVector (map, !fast));
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
	  map = Epetra_LocalMap (v.vector->GlobalLength(),0,v.vector->Comm());

	vector = std::auto_ptr<Epetra_FEVector> (new Epetra_FEVector(map,!fast));
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
      }

  }



  Vector &
  Vector::operator = (const MPI::Vector &v)
  {
    if (vector->Map().SameAs(v.vector->Map()) == false)
      {
	map = Epetra_LocalMap (v.vector->Map().NumGlobalElements(), 0,
			       v.vector->Comm());
	vector = std::auto_ptr<Epetra_FEVector> (new Epetra_FEVector(map,false));
      }

    reinit (v, false, true);
    return *this;
  }



  Vector &
  Vector::operator = (const Vector &v)
  {
    if (vector->Map().SameAs(v.vector->Map()) == false)
      {
	map = Epetra_LocalMap (v.vector->Map().NumGlobalElements(), 0,
			       v.vector->Comm());
	vector = std::auto_ptr<Epetra_FEVector> (new Epetra_FEVector(map,false));
      }

    *vector = *v.vector;

    return *this;
  }



  template <typename Number>
  Vector &
  Vector::operator = (const ::dealii::Vector<Number> &v)
  {
    if (size() != v.size())
      {
	map = LocalMap (v.size(), 0, vector->Comm());
	vector = std::auto_ptr<Epetra_FEVector> (new Epetra_FEVector(map,false));
      }

    std::vector<int> indices (v.size());
    for (unsigned int i=0; i<v.size(); ++i)
      indices[i] = map.GID(i);

    const int ierr = vector->ReplaceGlobalValues(v.size(), 0, v.begin(), 
						 &indices[0]);
    AssertThrow (ierr == 0, ExcTrilinosError());

    return *this;
  }

}

DEAL_II_NAMESPACE_CLOSE

#endif // DEAL_II_USE_TRILINOS
