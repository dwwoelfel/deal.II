//----------------------------  hp_dof_levels.cc  ---------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 2003, 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  hp_dof_levels.cc  ------------------------


#include <base/memory_consumption.h>
#include <dofs/hp_dof_levels.h>
#include <dofs/hp_dof_handler.h>
#include <fe/fe_collection.h>


namespace internal
{
  namespace hp
  {
    unsigned int
    DoFLevel<1>::memory_consumption () const
    {
      return (MemoryConsumption::memory_consumption (line_dofs) +
              MemoryConsumption::memory_consumption (dof_line_index_offset));
    }



    unsigned int
    DoFLevel<2>::memory_consumption () const
    {
      return (DoFLevel<1>::memory_consumption () +
              MemoryConsumption::memory_consumption (quad_dofs) +
              MemoryConsumption::memory_consumption (dof_quad_index_offset));
    }



    unsigned int
    DoFLevel<3>::memory_consumption () const
    {
      return (DoFLevel<2>::memory_consumption () +
              MemoryConsumption::memory_consumption (hex_dofs) +
              MemoryConsumption::memory_consumption (dof_hex_index_offset));
    }



    
    template <int dim>
    unsigned int
    DoFLevel<0>::
    get_vertex_dof_index (const ::hp::DoFHandler<dim> &dof_handler,
                          const unsigned int           vertex_index,
                          const unsigned int           fe_index,
                          const unsigned int           local_index)
    {
      Assert (fe_index != ::hp::DoFHandler<dim>::default_fe_index,
              ExcMessage ("You need to specify a FE index when working "
                          "with hp DoFHandlers"));
      Assert (&dof_handler != 0,
              ExcMessage ("No DoFHandler is specified for this iterator"));
      Assert (&dof_handler.get_fe() != 0,
              ExcMessage ("No finite element collection is associated with "
                          "this DoFHandler"));
      Assert (local_index < dof_handler.get_fe()[fe_index].dofs_per_vertex,
              ExcIndexRange(local_index, 0,
                            dof_handler.get_fe()[fe_index].dofs_per_vertex));

                                       // hop along the list of index
                                       // sets until we find the one
                                       // with the correct fe_index, and
                                       // then poke into that
                                       // part. trigger an exception if
                                       // we can't find a set for this
                                       // particular fe_index
      const unsigned int starting_offset
        = dof_handler.vertex_dofs_offsets[vertex_index];
      const unsigned int *pointer
        = &dof_handler.vertex_dofs[starting_offset];
      while (true)
        {
          Assert (*pointer != deal_II_numbers::invalid_unsigned_int,
                  ExcInternalError());
          if (*pointer == fe_index)
            return *(pointer + 1 + local_index);
          else
            pointer += dof_handler.get_fe()[*pointer].dofs_per_vertex;
        }
    }  



    template <int dim>
    void
    DoFLevel<0>::
    set_vertex_dof_index (::hp::DoFHandler<dim> &dof_handler,
                          const unsigned int           vertex_index,
                          const unsigned int           fe_index,
                          const unsigned int           local_index,
                          const unsigned int           global_index)
    {
      Assert (fe_index != ::hp::DoFHandler<dim>::default_fe_index,
              ExcMessage ("You need to specify a FE index when working "
                          "with hp DoFHandlers"));
      Assert (&dof_handler != 0,
              ExcMessage ("No DoFHandler is specified for this iterator"));
      Assert (&dof_handler.get_fe() != 0,
              ExcMessage ("No finite element collection is associated with "
                          "this DoFHandler"));
      Assert (local_index < dof_handler.get_fe()[fe_index].dofs_per_vertex,
              ExcIndexRange(local_index, 0,
                            dof_handler.get_fe()[fe_index].dofs_per_vertex));

                                       // hop along the list of index
                                       // sets until we find the one
                                       // with the correct fe_index, and
                                       // then poke into that
                                       // part. trigger an exception if
                                       // we can't find a set for this
                                       // particular fe_index
      const unsigned int starting_offset
        = dof_handler.vertex_dofs_offsets[vertex_index];
      unsigned int *pointer
        = &dof_handler.vertex_dofs[starting_offset];
      while (true)
        {
          Assert (*pointer != deal_II_numbers::invalid_unsigned_int,
                  ExcInternalError());
          if (*pointer == fe_index)
            {
              *(pointer + 1 + local_index) = global_index;
              return;
            }
          else
            pointer += dof_handler.get_fe()[*pointer].dofs_per_vertex;
        }
    }  



    template <int dim>
    unsigned int
    DoFLevel<1>::
    get_line_dof_index (const ::hp::DoFHandler<dim> &dof_handler,
                        const unsigned int           line_index,
                        const unsigned int           fe_index,
                        const unsigned int           local_index) const
    {
      Assert (fe_index != ::hp::DoFHandler<dim>::default_fe_index,
              ExcMessage ("You need to specify a FE index when working "
                          "with hp DoFHandlers"));
      Assert (&dof_handler != 0,
              ExcMessage ("No DoFHandler is specified for this iterator"));
      Assert (&dof_handler.get_fe() != 0,
              ExcMessage ("No finite element collection is associated with "
                          "this DoFHandler"));
      Assert (local_index < dof_handler.get_fe()[fe_index].dofs_per_line,
              ExcIndexRange(local_index, 0,
                            dof_handler.get_fe()[fe_index].dofs_per_line));

                                       // if we are in 1d, then the
                                       // only set of indices we store
                                       // is the one for the cell,
                                       // which is unique. then
                                       // fe_index must be
                                       // active_fe_index
      if (dim == 1)
        {
          Assert (fe_index == this->active_fe_indices[line_index],
                  ExcMessage ("FE index does not match that of the present cell"));
          return line_dofs[dof_line_index_offset[line_index]+local_index];
        }
      else
        {
                                           // we are in higher space
                                           // dimensions, so there may
                                           // be multiple finite
                                           // elements associated with
                                           // this object. hop along
                                           // the list of index sets
                                           // until we find the one
                                           // with the correct
                                           // fe_index, and then poke
                                           // into that part. trigger
                                           // an exception if we can't
                                           // find a set for this
                                           // particular fe_index
          const unsigned int starting_offset = dof_line_index_offset[line_index];
          const unsigned int *pointer        = &line_dofs[starting_offset];
          while (true)
            {
              Assert (*pointer != deal_II_numbers::invalid_unsigned_int,
                      ExcInternalError());
              if (*pointer == fe_index)
                return *(pointer + 1 + local_index);
              else
                pointer += dof_handler.get_fe()[*pointer].dofs_per_line;
            }
        }
    }



    template <int dim>
    void
    DoFLevel<1>::
    set_line_dof_index (const ::hp::DoFHandler<dim> &dof_handler,
                        const unsigned int           line_index,
                        const unsigned int           fe_index,
                        const unsigned int           local_index,
                        const unsigned int           global_index)
    {
      Assert (fe_index != ::hp::DoFHandler<dim>::default_fe_index,
              ExcMessage ("You need to specify a FE index when working "
                          "with hp DoFHandlers"));
      Assert (&dof_handler != 0,
              ExcMessage ("No DoFHandler is specified for this iterator"));
      Assert (&dof_handler.get_fe() != 0,
              ExcMessage ("No finite element collection is associated with "
                          "this DoFHandler"));
      Assert (local_index < dof_handler.get_fe()[fe_index].dofs_per_line,
              ExcIndexRange(local_index, 0,
                            dof_handler.get_fe()[fe_index].dofs_per_line));

                                       // if we are in 1d, then the
                                       // only set of indices we store
                                       // is the one for the cell,
                                       // which is unique. then
                                       // fe_index must be
                                       // active_fe_index
      if (dim == 1)
        {
          Assert (fe_index == this->active_fe_indices[line_index],
                  ExcMessage ("FE index does not match that of the present cell"));
          line_dofs[dof_line_index_offset[line_index]+local_index] = global_index;
        }
      else
        {
                                           // we are in higher space
                                           // dimensions, so there may
                                           // be multiple finite
                                           // elements associated with
                                           // this object.  hop along
                                           // the list of index sets
                                           // until we find the one
                                           // with the correct
                                           // fe_index, and then poke
                                           // into that part. trigger
                                           // an exception if we can't
                                           // find a set for this
                                           // particular fe_index
          const unsigned int starting_offset = dof_line_index_offset[line_index];
          unsigned int      *pointer         = &line_dofs[starting_offset];
          while (true)
            {
              Assert (*pointer != deal_II_numbers::invalid_unsigned_int,
                      ExcInternalError());
              if (*pointer == fe_index)
                {
                  *(pointer + 1 + local_index) = global_index;
                  return;
                }
              else
                pointer += dof_handler.get_fe()[*pointer].dofs_per_line;
            }
        }  
    }



    template <int dim>
    unsigned int
    DoFLevel<2>::
    get_quad_dof_index (const ::hp::DoFHandler<dim> &dof_handler,
                        const unsigned int           quad_index,
                        const unsigned int           fe_index,
                        const unsigned int           local_index) const
    {
      Assert (dim >= 2, ExcMessage ("You can only access quads in 2d or higher"));
      Assert (fe_index != ::hp::DoFHandler<dim>::default_fe_index,
              ExcMessage ("You need to specify a FE index when working "
                          "with hp DoFHandlers"));
      Assert (&dof_handler != 0,
              ExcMessage ("No DoFHandler is specified for this iterator"));
      Assert (&dof_handler.get_fe() != 0,
              ExcMessage ("No finite element collection is associated with "
                          "this DoFHandler"));
      Assert (local_index < dof_handler.get_fe()[fe_index].dofs_per_quad,
              ExcIndexRange(local_index, 0,
                            dof_handler.get_fe()[fe_index].dofs_per_quad));

                                       // if we are in 1d, then the
                                       // only set of indices we store
                                       // is the one for the cell,
                                       // which is unique. then
                                       // fe_index must be
                                       // active_fe_index
      if (dim == 2)
        {
          Assert (fe_index == this->active_fe_indices[quad_index],
                  ExcMessage ("FE index does not match that of the present cell"));
          return quad_dofs[dof_quad_index_offset[quad_index]+local_index];
        }
      else
        {
                                           // we are in higher space
                                           // dimensions, so there may
                                           // be multiple finite
                                           // elements associated with
                                           // this object. hop along
                                           // the list of index sets
                                           // until we find the one
                                           // with the correct
                                           // fe_index, and then poke
                                           // into that part. trigger
                                           // an exception if we can't
                                           // find a set for this
                                           // particular fe_index
          const unsigned int starting_offset = dof_quad_index_offset[quad_index];
          const unsigned int *pointer        = &quad_dofs[starting_offset];
          while (true)
            {
              Assert (*pointer != deal_II_numbers::invalid_unsigned_int,
                      ExcInternalError());
              if (*pointer == fe_index)
                return *(pointer + 1 + local_index);
              else
                pointer += dof_handler.get_fe()[*pointer].dofs_per_quad;
            }
        }
    }



    template <int dim>
    void
    DoFLevel<2>::
    set_quad_dof_index (const ::hp::DoFHandler<dim> &dof_handler,
                        const unsigned int           quad_index,
                        const unsigned int           fe_index,
                        const unsigned int           local_index,
                        const unsigned int           global_index)
    {
      Assert (dim >= 2, ExcMessage ("You can only access quads in 2d or higher"));
      Assert (fe_index != ::hp::DoFHandler<dim>::default_fe_index,
              ExcMessage ("You need to specify a FE index when working "
                          "with hp DoFHandlers"));
      Assert (&dof_handler != 0,
              ExcMessage ("No DoFHandler is specified for this iterator"));
      Assert (&dof_handler.get_fe() != 0,
              ExcMessage ("No finite element collection is associated with "
                          "this DoFHandler"));
      Assert (local_index < dof_handler.get_fe()[fe_index].dofs_per_quad,
              ExcIndexRange(local_index, 0,
                            dof_handler.get_fe()[fe_index].dofs_per_quad));

                                       // if we are in 1d, then the
                                       // only set of indices we store
                                       // is the one for the cell,
                                       // which is unique. then
                                       // fe_index must be
                                       // active_fe_index
      if (dim == 2)
        {
          Assert (fe_index == this->active_fe_indices[quad_index],
                  ExcMessage ("FE index does not match that of the present cell"));
          quad_dofs[dof_quad_index_offset[quad_index]+local_index] = global_index;
        }
      else
        {
                                           // we are in higher space
                                           // dimensions, so there may
                                           // be multiple finite
                                           // elements associated with
                                           // this object.  hop along
                                           // the list of index sets
                                           // until we find the one
                                           // with the correct
                                           // fe_index, and then poke
                                           // into that part. trigger
                                           // an exception if we can't
                                           // find a set for this
                                           // particular fe_index
          const unsigned int starting_offset = dof_quad_index_offset[quad_index];
          unsigned int      *pointer         = &quad_dofs[starting_offset];
          while (true)
            {
              Assert (*pointer != deal_II_numbers::invalid_unsigned_int,
                      ExcInternalError());
              if (*pointer == fe_index)
                {
                  *(pointer + 1 + local_index) = global_index;
                  return;
                }
              else
                pointer += dof_handler.get_fe()[*pointer].dofs_per_quad;
            }
        }  
    }



    template <int dim>
    unsigned int
    DoFLevel<3>::
    get_hex_dof_index (const ::hp::DoFHandler<dim> &dof_handler,
                       const unsigned int           hex_index,
                       const unsigned int           fe_index,
                       const unsigned int           local_index) const
    {
      Assert (dim >= 3, ExcMessage ("You can only access hexs in 3d or higher"));
      Assert (fe_index != ::hp::DoFHandler<dim>::default_fe_index,
              ExcMessage ("You need to specify a FE index when working "
                          "with hp DoFHandlers"));
      Assert (&dof_handler != 0,
              ExcMessage ("No DoFHandler is specified for this iterator"));
      Assert (&dof_handler.get_fe() != 0,
              ExcMessage ("No finite element collection is associated with "
                          "this DoFHandler"));
      Assert (local_index < dof_handler.get_fe()[fe_index].dofs_per_hex,
              ExcIndexRange(local_index, 0,
                            dof_handler.get_fe()[fe_index].dofs_per_hex));

                                       // if we are in 1d, then the
                                       // only set of indices we store
                                       // is the one for the cell,
                                       // which is unique. then
                                       // fe_index must be
                                       // active_fe_index
      if (dim == 3)
        {
          Assert (fe_index == this->active_fe_indices[hex_index],
                  ExcMessage ("FE index does not match that of the present cell"));
          return hex_dofs[dof_hex_index_offset[hex_index]+local_index];
        }
      else
        {
                                           // we are in higher space
                                           // dimensions, so there may
                                           // be multiple finite
                                           // elements associated with
                                           // this object. hop along
                                           // the list of index sets
                                           // until we find the one
                                           // with the correct
                                           // fe_index, and then poke
                                           // into that part. trigger
                                           // an exception if we can't
                                           // find a set for this
                                           // particular fe_index
          const unsigned int starting_offset = dof_hex_index_offset[hex_index];
          const unsigned int *pointer        = &hex_dofs[starting_offset];
          while (true)
            {
              Assert (*pointer != deal_II_numbers::invalid_unsigned_int,
                      ExcInternalError());
              if (*pointer == fe_index)
                return *(pointer + 1 + local_index);
              else
                pointer += dof_handler.get_fe()[*pointer].dofs_per_hex;
            }
        }
    }



    template <int dim>
    void
    DoFLevel<3>::
    set_hex_dof_index (const ::hp::DoFHandler<dim> &dof_handler,
                       const unsigned int           hex_index,
                       const unsigned int           fe_index,
                       const unsigned int           local_index,
                       const unsigned int           global_index)
    {
      Assert (dim >= 3, ExcMessage ("You can only access hexs in 3d or higher"));
      Assert (fe_index != ::hp::DoFHandler<dim>::default_fe_index,
              ExcMessage ("You need to specify a FE index when working "
                          "with hp DoFHandlers"));
      Assert (&dof_handler != 0,
              ExcMessage ("No DoFHandler is specified for this iterator"));
      Assert (&dof_handler.get_fe() != 0,
              ExcMessage ("No finite element collection is associated with "
                          "this DoFHandler"));
      Assert (local_index < dof_handler.get_fe()[fe_index].dofs_per_hex,
              ExcIndexRange(local_index, 0,
                            dof_handler.get_fe()[fe_index].dofs_per_hex));

                                       // if we are in 1d, then the
                                       // only set of indices we store
                                       // is the one for the cell,
                                       // which is unique. then
                                       // fe_index must be
                                       // active_fe_index
      if (dim == 3)
        {
          Assert (fe_index == this->active_fe_indices[hex_index],
                  ExcMessage ("FE index does not match that of the present cell"));
          hex_dofs[dof_hex_index_offset[hex_index]+local_index] = global_index;
        }
      else
        {
                                           // we are in higher space
                                           // dimensions, so there may
                                           // be multiple finite
                                           // elements associated with
                                           // this object.  hop along
                                           // the list of index sets
                                           // until we find the one
                                           // with the correct
                                           // fe_index, and then poke
                                           // into that part. trigger
                                           // an exception if we can't
                                           // find a set for this
                                           // particular fe_index
          const unsigned int starting_offset = dof_hex_index_offset[hex_index];
          unsigned int      *pointer         = &hex_dofs[starting_offset];
          while (true)
            {
              Assert (*pointer != deal_II_numbers::invalid_unsigned_int,
                      ExcInternalError());
              if (*pointer == fe_index)
                {
                  *(pointer + 1 + local_index) = global_index;
                  return;
                }
              else
                pointer += dof_handler.get_fe()[*pointer].dofs_per_hex;
            }
        }  
    }
    
    

// explicit instantiations
    template
    unsigned int
    DoFLevel<0>::
    get_vertex_dof_index (const ::hp::DoFHandler<deal_II_dimension> &dof_handler,
                          const unsigned int           vertex_index,
                          const unsigned int           fe_index,
                          const unsigned int           local_index);
    
    template
    void
    DoFLevel<0>::
    set_vertex_dof_index (::hp::DoFHandler<deal_II_dimension> &dof_handler,
                          const unsigned int           vertex_index,
                          const unsigned int           fe_index,
                          const unsigned int           local_index,
                          const unsigned int           global_index);

    template
    unsigned int
    DoFLevel<1>::
    get_line_dof_index (const ::hp::DoFHandler<deal_II_dimension> &dof_handler,
                        const unsigned int           line_index,
                        const unsigned int           fe_index,
                        const unsigned int           local_index) const;
    
    template
    void
    DoFLevel<1>::
    set_line_dof_index (const ::hp::DoFHandler<deal_II_dimension> &dof_handler,
                        const unsigned int           line_index,
                        const unsigned int           fe_index,
                        const unsigned int           local_index,
                        const unsigned int           global_index);

#if deal_II_dimension >= 2

    template
    unsigned int
    DoFLevel<2>::
    get_quad_dof_index (const ::hp::DoFHandler<deal_II_dimension> &dof_handler,
                        const unsigned int           quad_index,
                        const unsigned int           fe_index,
                        const unsigned int           local_index) const;
    
    template
    void
    DoFLevel<2>::
    set_quad_dof_index (const ::hp::DoFHandler<deal_II_dimension> &dof_handler,
                        const unsigned int           quad_index,
                        const unsigned int           fe_index,
                        const unsigned int           local_index,
                        const unsigned int           global_index);

#endif

#if deal_II_dimension >= 3
    
    template
    unsigned int
    DoFLevel<3>::
    get_hex_dof_index (const ::hp::DoFHandler<deal_II_dimension> &dof_handler,
                       const unsigned int           hex_index,
                       const unsigned int           fe_index,
                       const unsigned int           local_index) const;
    
    template
    void
    DoFLevel<3>::
    set_hex_dof_index (const ::hp::DoFHandler<deal_II_dimension> &dof_handler,
                       const unsigned int           hex_index,
                       const unsigned int           fe_index,
                       const unsigned int           local_index,
                       const unsigned int           global_index);

#endif
  }
}
