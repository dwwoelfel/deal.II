//----------------------------  data_out_faces_04.cc  ---------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2004, 2004, 2006, 2007 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  data_out_faces_04.cc  ---------------------------


// write the data in deal.II intermediate form, read it back in, and
// make sure that the result is the same

// this is as the _03 test except that it tests our ability to read and write
// files with vector components in them

#include "../tests.h"
#include "data_out_common.cc"
#include <lac/sparsity_pattern.h>
#include <numerics/data_out_faces.h>


std::string output_file_name = "data_out_faces_04/output";


// have a class that makes sure we can get at the patches and data set
// names that the base class generates
template <int dim>
class XDataOut : public DataOutFaces<dim>
{
  public:
    const std::vector<typename ::DataOutBase::Patch<dim-1,dim> > &
    get_patches() const
      { return DataOutFaces<dim>::get_patches(); }

    std::vector<std::string>
    get_dataset_names () const    
      { return DataOutFaces<dim>::get_dataset_names();  }

    std::vector<boost::tuple<unsigned int, unsigned int, std::string> >
    get_vector_data_ranges () const
      {
	return DataOutFaces<dim>::get_vector_data_ranges ();
      }
};

// have a class that makes sure we can get at the patches and data set
// names that the base class generates
template <int dim>
class XDataOutReader : public DataOutReader<dim-1,dim>
{
  public:
    const std::vector<typename ::DataOutBase::Patch<dim-1,dim> > &
    get_patches() const
      { return DataOutReader<dim-1,dim>::get_patches(); }

    std::vector<std::string>
    get_dataset_names () const    
      { return DataOutReader<dim-1,dim>::get_dataset_names();  }

    std::vector<boost::tuple<unsigned int, unsigned int, std::string> >
    get_vector_data_ranges () const
      {
	return DataOutReader<dim-1,dim>::get_vector_data_ranges ();
      }
};



void
my_check_this (const DoFHandler<1> &,
            const Vector<double>  &,
            const Vector<double>  &)
{
				   // don't check in 1d
}




template <int dim>
void
my_check_this (const DoFHandler<dim> &dof_handler,
            const Vector<double>  &v_node,
            const Vector<double>  &v_cell)
{
  XDataOut<dim> data_out;
  data_out.attach_dof_handler (dof_handler);
  data_out.add_data_vector (v_node, "node_data");
  data_out.add_data_vector (v_cell, "cell_data");
  data_out.build_patches ();
  
  {
    std::ofstream tmp ("data_out_faces_04.tmp");
    data_out.write_deal_II_intermediate (tmp);
  }

  XDataOutReader<dim> reader;
  {
    std::ifstream tmp ("data_out_faces_04.tmp");
    reader.read (tmp);
  }  

				   // finally make sure that we have
				   // read everything back in
				   // correctly
  Assert (data_out.get_dataset_names() == reader.get_dataset_names(),
	  ExcInternalError());
  
  Assert (data_out.get_patches().size() == reader.get_patches().size(),
 	  ExcInternalError());

  for (unsigned int i=0; i<reader.get_patches().size(); ++i)
    Assert (data_out.get_patches()[i] == reader.get_patches()[i],
	    ExcInternalError());

  deallog << data_out.get_vector_data_ranges().size()
	  << std::endl;
  Assert (data_out.get_vector_data_ranges().size() ==
	  reader.get_vector_data_ranges().size(),
	  ExcInternalError());
  for (unsigned int i=0; i<data_out.get_vector_data_ranges().size(); ++i)
    {
      deallog << data_out.get_vector_data_ranges()[i].template get<0>()
	      << ' '
	      << data_out.get_vector_data_ranges()[i].template get<1>()
	      << ' '
	      << data_out.get_vector_data_ranges()[i].template get<2>()
	      << std::endl;
      Assert (data_out.get_vector_data_ranges()[i].template get<0>()
	      ==
	      reader.get_vector_data_ranges()[i].template get<0>(),
	      ExcInternalError());
      Assert (data_out.get_vector_data_ranges()[i].template get<1>()
	      ==
	      reader.get_vector_data_ranges()[i].template get<1>(),
	      ExcInternalError());
      Assert (data_out.get_vector_data_ranges()[i].template get<2>()
	      ==
	      reader.get_vector_data_ranges()[i].template get<2>(),
	      ExcInternalError());
    }

				   // for good measure, delete tmp file
  remove ("data_out_faces_04.tmp");
  
  deallog << "OK" << std::endl;
}




template <int dim>
void
check_this (const DoFHandler<dim> &dof_handler,
            const Vector<double>  &v_node,
            const Vector<double>  &v_cell)
{
				   // since we can't forward declare
				   // check_this in this file (it is forward
				   // declared in data_out_common.cc), we
				   // also can't make the driver file aware of
				   // the overload for 1d. to avoid linker
				   // errors, we can consequently not overload
				   // check_this, and need this forwarder
				   // function
  my_check_this (dof_handler, v_node, v_cell);
}
