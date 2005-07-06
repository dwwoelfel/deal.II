//---------------------------------------------------------------------------
//   grid_reordering.cc,v 1.27 2002/05/28 07:43:22 wolf Exp
//   Version: 
//
//   Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005 by the deal.II authors
//
//   This file is subject to QPL and may not be  distributed
//   without copyright and license information. Please refer
//   to the file deal.II/doc/license.html for the  text  and
//   further information on this license.
//
//---------------------------------------------------------------------------

#include <grid/grid_reordering.h>
#include <grid/grid_reordering_internal.h>
#include <grid/grid_tools.h>

#include <algorithm>
#include <set>
#include <iostream>
#include <fstream>


#if deal_II_dimension == 1

template<>
void
GridReordering<1>::reorder_cells (std::vector<CellData<1> > &)
{
				   // there should not be much to do
				   // in 1d...
}


template<>
void
GridReordering<1>::invert_all_cells_of_negative_grid(const std::vector<Point<1> > &,
						     std::vector<CellData<1> > &)
{
				   // nothing to be done in 1d
}

#endif



#if deal_II_dimension == 2

namespace internal
{
  namespace GridReordering2d
  {
// -- Definition of connectivity information --
    const int ConnectGlobals::EdgeToNode[4][2] = 
    { {0,1},{1,2},{2,3},{3,0} };

    const int ConnectGlobals::NodeToEdge[4][2] = 
    { {3,0},{0,1},{1,2},{2,3} };

    const int ConnectGlobals::DefaultOrientation[4][2] = 
    {{0,1},{1,2},{3,2},{0,3}};


                                     /**
                                      * Simple data structure denoting
                                      * an edge, i.e. the ordered pair
                                      * of its vertices. This is only
                                      * used in the is_consistent
                                      * function.
                                      */
    struct Edge
    {
        Edge (const unsigned int v0,
              const unsigned int v1)
                        :
                        v0(v0), v1(v1)
          {}
        
        const unsigned int v0, v1;
        bool operator < (const Edge &e) const
          {
            return ((v0 < e.v0) || ((v0 == e.v0) && (v1 < e.v1)));
          }
    };

    
    bool
    is_consistent  (const std::vector<CellData<2> > &cells)
    {
      std::set<Edge> edges;

      std::vector<CellData<2> >::const_iterator c = cells.begin();
      for (; c != cells.end(); ++c)
        {
                                           // construct the four edges
	                                   // in reverse order
          const Edge reverse_edges[4] = { Edge (c->vertices[1],
                                                c->vertices[0]),
                                          Edge (c->vertices[2],
                                                c->vertices[1]),
                                          Edge (c->vertices[2],
                                                c->vertices[3]),
                                          Edge (c->vertices[3],
                                                c->vertices[0]) };
                                           // for each of them, check
                                           // whether they are already
                                           // in the set
	                                   //
	                                   // unroll the loop by hand to
	                                   // avoid a nasty compiler error
	                                   // in gcc2.95 that generated
	                                   // duplicate	assembler labels
	                                   // otherwise	  
	  if ((edges.find (reverse_edges[0]) != edges.end()) ||
	      (edges.find (reverse_edges[1]) != edges.end()) ||
	      (edges.find (reverse_edges[2]) != edges.end()) ||
	      (edges.find (reverse_edges[3]) != edges.end()))
            return false;
                                           // ok, not. insert them
	                                   // in the order in which
	                                   // we want them
                                           // (std::set eliminates
                                           // duplicated by itself)
          for (unsigned int i = 0; i<4; ++i)
	    {
	      const Edge e(reverse_edges[i].v1, reverse_edges[i].v0);
	      edges.insert (e);
	    }
                                           // then go on with next
                                           // cell
        }
                                       // no conflicts found, so
                                       // return true
      return true;
    }
    


    struct MSide::SideRectify : public std::unary_function<MSide,void>
    {
	void operator() (MSide &s) const
	  {
	    if (s.v0>s.v1)
	      std::swap (s.v0, s.v1);
	  }	    
    };


    struct MSide::SideSortLess : public std::binary_function<MSide,MSide,bool>
    {
	bool operator()(const MSide &s1, const MSide &s2) const
	  {
	    int s1vmin,s1vmax;
	    int s2vmin,s2vmax;
	    if (s1.v0<s1.v1)
	      {
		s1vmin = s1.v0;
		s1vmax = s1.v1;
	      }
	    else
	      {
		s1vmin = s1.v1;
		s1vmax = s1.v0;
	      }
	    if (s2.v0<s2.v1)
	      {
		s2vmin = s2.v0;
		s2vmax = s2.v1;
	      }
	    else
	      {
		s2vmin = s2.v1;
		s2vmax = s2.v0;
	      }

	    if (s1vmin<s2vmin)
	      return true;
	    if (s1vmin>s2vmin)
	      return false;
	    return s1vmax<s2vmax;
	  }
    };
    

/**
 * Returns an MSide corresponding to the
 * specified side of a deal.II CellData<2> object.
 */
    MSide quadside(const CellData<2> &q, unsigned int i)
    {
      Assert (i<4, ExcInternalError());
      return MSide(q.vertices[ConnectGlobals::EdgeToNode[i][0]],
		   q.vertices[ConnectGlobals::EdgeToNode[i][1]]);
    }


/**
 * Wrapper class for the quadside() function
 */
    struct QuadSide: public std::binary_function<CellData<2>,int,MSide>
    {
	MSide operator()(const CellData<2>& q, int i) const
	  {
	    return quadside(q,i);
	  }
    };
 
    

    MQuad::MQuad (const unsigned int v0,
		  const unsigned int v1,
		  const unsigned int v2,
		  const unsigned int v3,
		  const unsigned int s0,
		  const unsigned int s1,
		  const unsigned int s2,
		  const unsigned int s3,
		  const CellData<2>  &cd)
		    :
		    original_cell_data (cd)
    {
      v[0] = v0;
      v[1] = v1;
      v[2] = v2;
      v[3] = v3;
      side[0] = s0;
      side[1] = s1;
      side[2] = s2;
      side[3] = s3;
    }


    MSide::MSide (const unsigned int initv0,
		  const unsigned int initv1)
		    :
		    v0(initv0), v1(initv1),
		    Q0(deal_II_numbers::invalid_unsigned_int),
                    Q1(deal_II_numbers::invalid_unsigned_int),
		    lsn0(deal_II_numbers::invalid_unsigned_int),
                    lsn1(deal_II_numbers::invalid_unsigned_int),
		    Oriented(false)
    {}

    
    
    bool
    MSide::operator == (const MSide& s2) const
    {
      if ((v0 == s2.v0)&&(v1 == s2.v1)) {return true;}
      if ((v0 == s2.v1)&&(v1 == s2.v0)) {return true;}
      return false;
    }


    bool
    MSide::operator != (const MSide& s2) const
    {
      return !(*this == s2);
    }
    
    
    struct MQuad::MakeQuad : public std::binary_function<CellData<2>,
		  std::vector<MSide>,
		  MQuad>
    {
	MQuad operator()(const CellData<2> &q,
			 const std::vector<MSide> &elist) const
	  {
					     //Assumes that the sides
					     //are in the vector.. Bad
					     //things will happen if
					     //they are not!
	    return MQuad(q.vertices[0],q.vertices[1], q.vertices[2], q.vertices[3],
			 std::distance(elist.begin(),
				       std::lower_bound(elist.begin(), elist.end(),
							quadside(q,0),
							MSide::SideSortLess() )),
			 std::distance(elist.begin(),
				       std::lower_bound(elist.begin(), elist.end(),
							quadside(q,1),
							MSide::SideSortLess() )),
			 std::distance(elist.begin(),
				       std::lower_bound(elist.begin(), elist.end(),
							quadside(q,2),
							MSide::SideSortLess() )),
			 std::distance(elist.begin(),
				       std::lower_bound(elist.begin(), elist.end(),
							quadside(q,3),
							MSide::SideSortLess() )),
			 q);
	  }
	    
    };


    
    void
    GridReordering::reorient(std::vector<CellData<2> > &quads)
    {
      build_graph(quads);
      orient();
      get_quads(quads);
    }


    void
    GridReordering::build_graph (const std::vector<CellData<2> > &inquads)
    {
				       //Reserve some space 
      sides.reserve(4*inquads.size());
      mquads.reserve(inquads.size());
  
				       //Insert all the sides into the side vector
      for (int i = 0;i<4;++i)
	{
	  std::transform(inquads.begin(),inquads.end(),
			 std::back_inserter(sides), std::bind2nd(QuadSide(),i));
	}
  
				       //Change each edge so that v0<v1
      std::for_each(sides.begin(),sides.end(),
		    MSide::SideRectify() );
  
				       //Sort them by Sidevertices.
      std::sort(sides.begin(),sides.end(),
		MSide::SideSortLess());
  
				       //Remove duplicates 
      sides.erase(std::unique(sides.begin(),sides.end()),
		  sides.end());

				       // Swap trick to shrink the
				       // side vector
      std::vector<MSide>(sides).swap(sides);
  
				       //Assigns the correct sides to
				       //each quads
      transform(inquads.begin(),inquads.end(), std::back_inserter(mquads),
		std::bind2nd(MQuad::MakeQuad(),sides) );
  
				       // Assign the quads to their sides also.
      int qctr = 0;
      for (std::vector<MQuad>::iterator it = mquads.begin(); it != mquads.end(); ++it)
	{
	  for (unsigned int i = 0;i<4;++i)
	    {
	      MSide &ss = sides[(*it).side[i]];
	      if (ss.Q0 == deal_II_numbers::invalid_unsigned_int)
		{
		  ss.Q0 = qctr;
		  ss.lsn0 = i;
		}
	      else if (ss.Q1 == deal_II_numbers::invalid_unsigned_int)
		{
		  ss.Q1 = qctr;
		  ss.lsn1 = i;
		}
	      else
		AssertThrow (false, ExcInternalError());
	    }
	  qctr++;
	}
    }


    void GridReordering::orient()
    {
				       // do what the comment in the
				       // class declaration says
      unsigned int qnum = 0;
      while(get_unoriented_quad(qnum))
	{
	  unsigned int lsn = 0;
	  while(get_unoriented_side(qnum,lsn))
	    {
	      orient_side(qnum,lsn);
	      unsigned int qqnum = qnum;
	      while(side_hop(qqnum,lsn))
		{
						   // switch this face
		  lsn = (lsn+2)%4;
		  if (!is_oriented_side(qqnum,lsn))
		    orient_side(qqnum,lsn);
		  else
						     //We've found a
						     //cycle.. and
						     //oriented all
						     //quads in it.
		    break;
		}
	    }
	}
    }


    void
    GridReordering::orient_side(const unsigned int quadnum,
				const unsigned int localsidenum)
    {
      MQuad &quad = mquads[quadnum];
      int op_side_l = (localsidenum+2)%4;
      MSide &side = sides[mquads[quadnum].side[localsidenum]];
      const MSide &op_side = sides[mquads[quadnum].side[op_side_l]]; 
  
				       //is the opposite side oriented?    
      if (op_side.Oriented)
	{
					   //YES - Make the orientations match
					   //Is op side in default orientation?
	  if (op_side.v0 == quad.v[ConnectGlobals::DefaultOrientation[op_side_l][0]])
	    {
					       //YES
	      side.v0 = quad.v[ConnectGlobals::DefaultOrientation[localsidenum][0]];
	      side.v1 = quad.v[ConnectGlobals::DefaultOrientation[localsidenum][1]];
	    }
	  else
	    {
					       //NO, its reversed
	      side.v0 = quad.v[ConnectGlobals::DefaultOrientation[localsidenum][1]];
	      side.v1 = quad.v[ConnectGlobals::DefaultOrientation[localsidenum][0]];
	    }
	}
      else
	{
					   //NO
					   //Just use the default orientation      
	  side.v0 = quad.v[ConnectGlobals::DefaultOrientation[localsidenum][0]];
	  side.v1 = quad.v[ConnectGlobals::DefaultOrientation[localsidenum][1]];
	}
      side.Oriented = true;  
    }



    bool
    GridReordering::is_fully_oriented_quad(const unsigned int quadnum) const
    {
      return (
	(sides[mquads[quadnum].side[0]].Oriented)&&
	(sides[mquads[quadnum].side[1]].Oriented)&&
	(sides[mquads[quadnum].side[2]].Oriented)&&
	(sides[mquads[quadnum].side[3]].Oriented) 
      );
    }



    bool
    GridReordering::is_oriented_side(const unsigned int quadnum,
				     const unsigned int lsn) const
    {
      return (sides[mquads[quadnum].side[lsn]].Oriented);
    }




    bool
    GridReordering::get_unoriented_quad(unsigned int &UnOrQLoc) const
    {
      while( (UnOrQLoc<mquads.size()) &&
	     is_fully_oriented_quad(UnOrQLoc) )
	UnOrQLoc++;
      return (UnOrQLoc != mquads.size());
    }



    bool
    GridReordering::get_unoriented_side (const unsigned int quadnum,
					 unsigned int &lsn) const
    {
      const MQuad &mq = mquads[quadnum];
      if (!sides[mq.side[0]].Oriented)
	{
	  lsn = 0;
	  return true;
	}
      if (!sides[mq.side[1]].Oriented)
	{
	  lsn = 1;
	  return true;
	}
      if (!sides[mq.side[2]].Oriented)
	{
	  lsn = 2;
	  return true;
	}
      if (!sides[mq.side[3]].Oriented)
	{
	  lsn = 3;
	  return true;
	}
      return false;
    }


    bool
    GridReordering::side_hop (unsigned int &qnum, unsigned int &lsn) const
    {
      const MQuad &mq = mquads[qnum];
      const MSide &s = sides[mq.side[lsn]];
      unsigned int opquad = 0;
      if (s.Q0 == qnum)
	{
	  opquad = s.Q1;
	  lsn = s.lsn1;
	}
      else
	{
	  opquad = s.Q0;
	  lsn = s.lsn0;
	}
  
      if (opquad != deal_II_numbers::invalid_unsigned_int)
	{
	  qnum = opquad;
	  return true;
	}
  
      return false;
    }


    void
    GridReordering::get_quads (std::vector<CellData<2> > &outquads) const
    {
      outquads.clear();
      outquads.reserve(mquads.size());
      for (unsigned int qn = 0;qn<mquads.size();++qn)
	{
					   // initialize CellData object with
					   // previous contents, and the
					   // overwrite all the fields that
					   // might have changed in the
					   // process of rotating things
	  CellData<2> q = mquads[qn].original_cell_data;
	  
					   // Are the sides oriented? 
	  Assert (is_fully_oriented_quad(qn), ExcInternalError());
	  bool s[4]; //whether side 1 ,2, 3, 4 are in the default orientation
	  for (int sn = 0;sn<4;sn++)
	    {
	      s[sn] = is_side_default_oriented(qn,sn);
	    }
					   // Are they oriented in the "deal way"?
	  Assert (s[0] == s[2], ExcInternalError());
	  Assert (s[1] == s[3], ExcInternalError());
					   // How much we rotate them by.
	  int rotn = 2*(s[0]?1:0)+ ((s[0]^s[1])?1:0);

	  for (int i = 0;i<4;++i)
	    {
	      q.vertices[(i+rotn)%4] = mquads[qn].v[i];
	    }
	  outquads.push_back(q);
	}

    }

    bool
    GridReordering::is_side_default_oriented (const unsigned int qnum,
					      const unsigned int lsn) const
    {
      return (sides[mquads[qnum].side[lsn]].v0 == 
	      mquads[qnum].v[ConnectGlobals::DefaultOrientation[lsn][0]]);
    }
  } // namespace GridReordering2
} // namespace internal


template<>
void
GridReordering<2>::reorder_cells (std::vector<CellData<2> > &original_cells)
{
                                   // check if grids are already
                                   // consistent. if so, do
                                   // nothing. if not, then do the
                                   // reordering
  if (internal::GridReordering2d::is_consistent (original_cells))
    return;
  
  internal::GridReordering2d::GridReordering().reorient(original_cells);
}


template<>
void
GridReordering<2>::invert_all_cells_of_negative_grid(const std::vector<Point<2> > &,
						     std::vector<CellData<2> > &)
{
				   // nothing to be done in 2d
}

#endif



#if deal_II_dimension == 3

namespace internal
{
  namespace GridReordering3d
  {
    DeclException1 (ExcGridOrientError,
		    char *,
		    <<  "Grid Orientation Error: " << arg1);

    const EdgeOrientation unoriented_edge = {'u'};
    const EdgeOrientation forward_edge    = {'f'};
    const EdgeOrientation backward_edge   = {'b'};

    inline
    bool
    EdgeOrientation::
    operator == (const EdgeOrientation &edge_orientation) const
    {
      Assert ((orientation == 'u') || (orientation == 'f') || (orientation == 'b'),
	      ExcInternalError());
      return orientation == edge_orientation.orientation;
    }



    inline
    bool
    EdgeOrientation::
    operator != (const EdgeOrientation &edge_orientation) const
    {
      return ! (*this == edge_orientation);
    }

    

    namespace ElementInfo
    {
				       /**
					* The numbers of the edges
					* coming into node i are
					* given by
					* edge_to_node[i][k] where
					* k=0,1,2.
					*/    
      static const unsigned int edge_to_node[8][3] = 
      { 
	    {0,4,8},
	    {0,5,9},
	    {3,5,10},
	    {3,4,11},
	    {1,7,8},
	    {1,6,9},
	    {2,6,10},
	    {2,7,11}
      };


				       /**
					* The orientation of edge
					* coming into node i is
					* given by
					* edge_to_node_orient[i][k]
					* where k=0,1,2. 1 means the
					* given node is the start of
					* the edge -1 means the end
					* of the edge.
					*/
//TODO: we should really initialize these entries as forward_edge or backward_edge, but gcc2.95 has a bug where it zero-initializes these data elements instead, leading to aborts later on.
      static const EdgeOrientation edge_to_node_orient[8][3] = 
      {
	    {{'f'},{'f'},{'f'}},
	    {{'b'},{'f'},{'f'}},
	    {{'b'},{'b'},{'f'}},
	    {{'f'},{'b'},{'f'}},
	    {{'f'},{'f'},{'b'}},
	    {{'b'},{'f'},{'b'}},
	    {{'b'},{'b'},{'b'}}, 
	    {{'f'},{'b'},{'b'}}
      };	    
    
				       /**
					* nodesonedge[i][0] is the
					* start node for edge i.
					* nodesonedge[i][1] is the
					* end node for edge i.
					*/
      static const unsigned int nodes_on_edge[12][2] = 
      {
	    {0,1},
	    {4,5},
	    {7,6},
	    {3,2},
	    {0,3},
	    {1,2},
	    {5,6},
	    {4,7},
	    {0,4},
	    {1,5},
	    {2,6},
	    {3,7}
      };	  
    }
    
    
    CheapEdge::CheapEdge (const unsigned int n0,
			  const unsigned int n1)
		    :
                                                     // sort the
                                                     // entries so
                                                     // that
                                                     // node0<node1
		    node0(std::min (n0, n1)),
                    node1(std::max (n0, n1))
    {}


    
    bool CheapEdge::operator< (const CheapEdge & e2) const
    {
      if (node0 < e2.node0) return true;
      if (node0 > e2.node0) return false;
      if (node1 < e2.node1) return true;
      return false;
    }

  
    Edge::Edge (const unsigned int n0,
		const unsigned int n1)
		    :
		    orientation_flag (unoriented_edge),
		    group (deal_II_numbers::invalid_unsigned_int)
    {
      nodes[0] = n0;
      nodes[1] = n1;
    }



    Cell::Cell () 
    {
      for (unsigned int i=0; i<GeometryInfo<3>::lines_per_cell; ++i)
	{
	  edges[i] = deal_II_numbers::invalid_unsigned_int;
	  local_orientation_flags[i] = forward_edge;
	}
      
      for (unsigned int i=0; i<GeometryInfo<3>::vertices_per_cell; ++i)
	nodes[i] = deal_II_numbers::invalid_unsigned_int;
      
      waiting_to_be_processed = false;
    }



    Mesh::Mesh (const std::vector<CellData<3> > &incubes)
    {
                                       // copy the cells into our own
                                       // internal data format.
      const unsigned int numelems = incubes.size();
      for (unsigned int i=0; i<numelems; ++i)
        {
          Cell the_cell;
          std::copy (&incubes[i].vertices[0],
                     &incubes[i].vertices[GeometryInfo<3>::vertices_per_cell],
                     &the_cell.nodes[0]);
	
          cell_list.push_back(the_cell);
        }

                                       // then build edges and
                                       // connectivity
      build_connectivity ();
    }
    

    
    void
    Mesh::sanity_check () const
    {
      for (unsigned int i=0; i<cell_list.size(); ++i)
	for (unsigned int j=0; j<8; ++j)
	  sanity_check_node (cell_list[i], j);
    }



    void
    Mesh::sanity_check_node (const Cell         &c,
                             const unsigned int local_node_num) const
    {
                                       // check that every edge
                                       // coming into a node has the
                                       // same node value

                                       // Get the Local Node Numbers
				       // of the incoming edges
      const unsigned int e0 = ElementInfo::edge_to_node[local_node_num][0];
      const unsigned int e1 = ElementInfo::edge_to_node[local_node_num][1]; 
      const unsigned int e2 = ElementInfo::edge_to_node[local_node_num][2];

				       // Global Edge Numbers
      const unsigned int ge0 = c.edges[e0];
      const unsigned int ge1 = c.edges[e1];
      const unsigned int ge2 = c.edges[e2];

      const EdgeOrientation or0 = ElementInfo::edge_to_node_orient[local_node_num][0] ==
				  c.local_orientation_flags[e0] ?
				  forward_edge : backward_edge;
      const EdgeOrientation or1 = ElementInfo::edge_to_node_orient[local_node_num][1] ==
				  c.local_orientation_flags[e1] ?
				  forward_edge : backward_edge;
      const EdgeOrientation or2 = ElementInfo::edge_to_node_orient[local_node_num][2] ==
		      c.local_orientation_flags[e2] ?
		      forward_edge : backward_edge;

				       // Make sure that edges agree
				       // what the current node should
				       // be.
      Assert ((edge_list[ge0].nodes[or0 == forward_edge ? 0 : 1] ==
	       edge_list[ge1].nodes[or1 == forward_edge ? 0 : 1])
	      &&
	      (edge_list[ge1].nodes[or1 == forward_edge ? 0 : 1] ==
	       edge_list[ge2].nodes[or2 == forward_edge ? 0 : 1]),
	      ExcMessage ("This message does not satisfy the internal "
			  "consistency check"));
    }    



				     // This is the guts of the matter...
    void Mesh::build_connectivity ()
    {
      const unsigned int n_cells = cell_list.size();
      
      unsigned int n_edges = 0;
				       // Correctly build the edge
				       // list
      {
					 // edge_map stores the
					 // edge_number associated
					 // with a given CheapEdge
	std::map<CheapEdge,unsigned int> edge_map;
	unsigned int ctr = 0;
	for (unsigned int cur_cell_id = 0; 
	     cur_cell_id<n_cells; 
	     ++cur_cell_id)
	  {
					     // Get the local node
					     // numbers on edge
					     // edge_num
	    const Cell & cur_cell = cell_list[cur_cell_id];

	    for (unsigned short int edge_num = 0; 
		 edge_num<12; 
		 ++edge_num)
	      {
		unsigned int gl_edge_num = 0;
		EdgeOrientation l_edge_orient = forward_edge;
		
						 // Construct the
						 // CheapEdge
		const unsigned int
		  node0 = cur_cell.nodes[ElementInfo::nodes_on_edge[edge_num][0]],
		  node1 = cur_cell.nodes[ElementInfo::nodes_on_edge[edge_num][1]];
		const CheapEdge cur_edge (node0, node1);
		
		if (edge_map.count(cur_edge) == 0)
						   // Edge not in map
		  {
						     // put edge in
						     // hash map with
						     // ctr value;
		    edge_map[cur_edge] = ctr;
		    gl_edge_num = ctr;

						     // put the edge
						     // into the
						     // global edge
						     // list
		    edge_list.push_back(Edge(node0,node1));
		    ctr++;
		  }
		else
		  {
						     // get edge_num
						     // from hash_map
		    gl_edge_num = edge_map[cur_edge]; 
		    if (edge_list[gl_edge_num].nodes[0] != node0)
		      l_edge_orient = backward_edge;
		  }
						 // set edge number to
						 // edgenum
		cell_list[cur_cell_id].edges[edge_num] = gl_edge_num; 
		cell_list[cur_cell_id].local_orientation_flags[edge_num]
		  = l_edge_orient;
	      }
	  }
	n_edges = ctr;
      }

				       // Count each of the edges.
      {
	std::vector<int> edge_count(n_edges,0);
      

					 // Count every time an edge
					 // occurs in a cube.
	for (unsigned int cur_cell_id=0; cur_cell_id<n_cells; ++cur_cell_id)
          for (unsigned short int edge_num = 0; edge_num<12; ++edge_num)
            ++edge_count[cell_list[cur_cell_id].edges[edge_num]];

					 // So we now know how many
					 // cubes contain a given
					 // edge. Just need to store
					 // the list of cubes in the
					 // edge

					 // Allocate the space for the
					 // neighbor list
	for (unsigned int cur_edge_id=0; cur_edge_id<n_edges; ++cur_edge_id)
          edge_list[cur_edge_id].neighboring_cubes
            .resize (edge_count[cur_edge_id]);
      
					 // Store the position of the
					 // current neighbor in the
					 // edge's neighbor list
	std::vector<int> cur_cell_edge_list_posn(n_edges,0);
	for (unsigned int cur_cell_id=0; cur_cell_id<n_cells; ++cur_cell_id)
          for (unsigned short int edge_num=0; edge_num<12; ++edge_num)
            {
              const unsigned int
                gl_edge_id = cell_list[cur_cell_id].edges[edge_num];
              Edge & cur_edge = edge_list[gl_edge_id];
              cur_edge.neighboring_cubes[cur_cell_edge_list_posn[gl_edge_id]]
                = cur_cell_id;
              cur_cell_edge_list_posn[gl_edge_id]++;
            }
      }
    }



    void
    Mesh::export_to_deal_format (std::vector<CellData<3> > &outcubes) const 
    {
      Assert (outcubes.size() == cell_list.size(),
              ExcInternalError());

                                       // simply overwrite the output
                                       // array with the new
                                       // information
      for (unsigned int i=0; i<cell_list.size(); ++i)
        std::copy (&cell_list[i].nodes[0],
                   &cell_list[i].nodes[GeometryInfo<3>::vertices_per_cell],
                   &outcubes[i].vertices[0]);
    }
    

    
    Orienter::Orienter (const std::vector<CellData<3> > &incubes)
                    :
                    mesh (incubes),
                    cur_posn (0),
                    marker_cube (0),
                    cur_edge_group  (0)
    {  
      for (unsigned int i = 0; i<12; ++i)
	edge_orient_array[i] = false;
    }


    
    void Orienter::orient_mesh (std::vector<CellData<3> > &incubes)
    {
      Orienter orienter (incubes);
      
				       // First check that the mesh is
				       // sensible
      orienter.mesh.sanity_check ();

                                       // Orient the mesh
      orienter.orient_edges ();

                                       // Now we have a bunch of oriented
                                       // edges int the structure we only
                                       // have to turn the cubes so they
                                       // match the edge orientation.
      orienter.orient_cubes ();
        
                                       // Copy the elements from our
                                       // internal structure back into
                                       // their original location.
      orienter.mesh.export_to_deal_format (incubes);
    }

				     /**
				      * This assigns an orientation
				      * to each edge so that every
				      * cube is a rotated Deal.II
				      * cube.
				      */
    void Orienter::orient_edges ()
    {
				       // While there are still cubes
				       // to orient
      while (get_next_unoriented_cube())
                                         // And there are edges in
                                         // the cube to orient
        while (orient_next_unoriented_edge())
          {
                                             // Make all the sides
                                             // in the current set
                                             // match
            orient_edges_in_current_cube();

                                             // Add the adjacent
                                             // cubes to the list
                                             // for processing
            get_adjacent_cubes();
                                             // Start working on
                                             // this list of cubes
            while (get_next_active_cube())
              {
                                                 // Make sure the
                                                 // Cube doesn't
                                                 // have a
                                                 // contradiction
                AssertThrow(cell_is_consistent(cur_posn),
                            ExcGridOrientError("Mesh is Unorientable"));

                                                 // If we needed to
                                                 // orient any edges
                                                 // in the current
                                                 // cube then we may
                                                 // have to process
                                                 // the neighbor.
                if (orient_edges_in_current_cube())
                  get_adjacent_cubes();
              }

                                             // start the next sheet
                                             // (equivalence class
                                             // of edges)
            ++cur_edge_group;
          }	
    }


    
    bool Orienter::get_next_unoriented_cube ()
    {
				       // The last cube in the list
      const unsigned int n_cubes = mesh.cell_list.size();
				       // Keep shifting along the list
				       // until we find a cube which
				       // is not fully oriented or the
				       // end.
      while( (marker_cube<n_cubes) &&
             (is_oriented(marker_cube)) )
	++marker_cube;
      cur_posn = marker_cube;
				       // Return true if we now point
				       // at a valid cube.
      return (cur_posn < n_cubes);
    }


    
    bool Orienter::is_oriented (const unsigned int cell_num) const
    {
      for (unsigned int i=0; i<12; ++i)
	if (mesh.edge_list[mesh.cell_list[cell_num].edges[i]].orientation_flag
            == unoriented_edge)
	  return false;
      return true;
    }


    
    bool
    Orienter::cell_is_consistent(const unsigned int cell_num) const
    {

      const Cell& c = mesh.cell_list[cell_num];
  
				       // Checks that all oriented
				       // edges in the group are
				       // oriented consistently.
      for (unsigned int group=0; group<3; ++group)
	{
					   // When a nonzero
					   // orientation is first
					   // encountered in the group
					   // it is stored in this
	  EdgeOrientation value = unoriented_edge;
					   // Loop over all parallel
					   // edges
	  for (unsigned int i=4*group; i<4*(group+1); ++i)
	    {
					       // If the edge has
					       // orientation
	      if ((c.local_orientation_flags[i] !=
		   unoriented_edge)
		  &&
		  (mesh.edge_list[c.edges[i]].orientation_flag !=
		   unoriented_edge))
		{
		  const EdgeOrientation this_edge_direction
		    = (c.local_orientation_flags[i] 
		       == mesh.edge_list[c.edges[i]].orientation_flag  ?
		       forward_edge : backward_edge);

						   // If we haven't
						   // seen an oriented
						   // edge before,
						   // then store its
						   // value:
		  if (value == unoriented_edge) 
		    value = this_edge_direction;
		  else
						     // If we have
						     // seen an
						     // oriented edge
						     // in this group
						     // we'd better
						     // have the same
						     // orientation.
		    if (value != this_edge_direction)
		      return false;
		}
	    }
	}
      return true;
    }


    
    bool Orienter::orient_next_unoriented_edge ()
    {
      cur_posn = marker_cube;
      const Cell& c = mesh.cell_list[cur_posn];
      unsigned int edge = 0;
 
				       // search for the unoriented
				       // side
      while ((edge<12) &&
	     (mesh.edge_list[c.edges[edge]].orientation_flag !=
	     unoriented_edge))
	++edge;
  
				       // if we found none then return
				       // false
      if (edge == 12)
	return false;
  
				       // Which edge group we're in.
      const unsigned int edge_group = edge/4;

				       // A sanity check that none of
				       // the other edges in the group
				       // have been oriented yet Each
				       // of the edges in the group
				       // should be un-oriented
      for (unsigned int j = edge_group*4; j<edge_group*4+4; ++j)
	Assert (mesh.edge_list[c.edges[j]].orientation_flag ==
		unoriented_edge,
		ExcGridOrientError("Tried to orient edge when other edges "
                                   "in group are already oriented!"));

				       // Make the edge alignment
				       // match that of the local
				       // cube.
      mesh.edge_list[c.edges[edge]].orientation_flag
	= c.local_orientation_flags[edge];
      mesh.edge_list[c.edges[edge]].group = cur_edge_group;

                                       // Remember that we have oriented 
                                       // this edge in the current cell.
      edge_orient_array[edge] = true;

      return true;
    }


    
    bool Orienter::orient_edges_in_current_cube ()
    {
      for (unsigned int edge_group=0; edge_group<3; ++edge_group)
	if (orient_edge_set_in_current_cube(edge_group) == true)
	  return true;
      
      return false;
    }


    
    bool
    Orienter::orient_edge_set_in_current_cube (const unsigned int n)
    {
      const Cell& c = mesh.cell_list[cur_posn];
  
				       // Check if any edge is
				       // oriented
      unsigned int n_oriented = 0;
      EdgeOrientation glorient   = unoriented_edge;
      unsigned int edge_flags = 0;
      unsigned int cur_flag   = 1;
      for (unsigned int i = 4*n; i<4*(n+1); ++i, cur_flag<<=1)
	{
	  if ((mesh.edge_list[c.edges[i]].orientation_flag !=
	      unoriented_edge)
	      &&
	      (c.local_orientation_flags[i] !=
	       unoriented_edge))
	    {
	      ++n_oriented;

	      const EdgeOrientation orient
		= (mesh.edge_list[c.edges[i]].orientation_flag ==
		   c.local_orientation_flags[i] ?
		   forward_edge : backward_edge);

	      if (glorient == unoriented_edge)
		glorient = orient;
	      else
		AssertThrow(orient == glorient,
			    ExcGridOrientError("Attempted to Orient Misaligned cube"));
	    }
	  else
	    edge_flags |= cur_flag;
	}

				       // were any of the sides
				       // oriented?  were they all
				       // already oriented?
      if ((glorient == unoriented_edge) || (n_oriented == 4))
	return false;

				       // If so orient all edges
				       // consistently.
      cur_flag = 1;
      for (unsigned int i=4*n; i<4*(n+1); ++i, cur_flag<<=1)
	if ((edge_flags & cur_flag) != 0)
	  {
	    mesh.edge_list[c.edges[i]].orientation_flag 
	      = (c.local_orientation_flags[i] == glorient ?
		 forward_edge : backward_edge);
	    
	    mesh.edge_list[c.edges[i]].group = cur_edge_group;
                                       // Remember that we have oriented 
                                       // this edge in the current cell.
	    edge_orient_array[i] = true;
	  }
	
      return true;
    }


    
    void Orienter::get_adjacent_cubes ()
    {
      const Cell &c = mesh.cell_list[cur_posn];
      for (unsigned int e=0; e<12; ++e)
                               	       // Only need to add the adjacent 
	                               // cubes for edges we recently 
	                               // oriented
	if (edge_orient_array[e] == true)
	  {
	    const Edge & the_edge = mesh.edge_list[c.edges[e]];
	    for (unsigned int local_cube_num = 0; 
		 local_cube_num < the_edge.neighboring_cubes.size();
		 ++local_cube_num)
	      {
		const unsigned int
		  global_cell_num = the_edge.neighboring_cubes[local_cube_num];
		Cell &ncell = mesh.cell_list[global_cell_num];
	
 	                               // If the cell is waiting to be 
		                       // processed we dont want to add 
		                       // it to the list a second time.	
		if (!ncell.waiting_to_be_processed)
		  {
		    sheet_to_process.push_back(global_cell_num);
		    ncell.waiting_to_be_processed = true;
		  }
	      }
	  }
                                       // we're done with this cube so 
                                       // clear its processing flags.
      for (unsigned int e=0; e<12; ++e)
	edge_orient_array[e] = false;
	    
    }


    
    bool Orienter::get_next_active_cube ()
    {
				       // Mark the curent Cube as
				       // finished with.
      Cell &c = mesh.cell_list[cur_posn];
      c.waiting_to_be_processed = false;
      if (sheet_to_process.size() != 0)
	{
	  cur_posn = sheet_to_process.back();
	  sheet_to_process.pop_back();
	  return true;
	}
      return false;
    }

    
    void Orienter::orient_cubes ()
    {
				       // We assume that the mesh has
				       // all edges oriented already.
  
				       // This is a list of
				       // permutations that take node
				       // 0 to node i but only rotate
				       // the cube.  (This set is far
				       // from unique (there are 3 for
				       // each node - for our
				       // algorithm it doesn't matter
				       // which of the three we use)
      static const unsigned int CubePermutations[8][8] = {
	    {0,1,2,3,4,5,6,7},
	    {1,2,3,0,5,6,7,4},
	    {2,3,0,1,6,7,4,5},
	    {3,0,1,2,7,4,5,6},
	    {4,7,6,5,0,3,2,1},
	    {5,4,7,6,1,0,3,2},
	    {6,5,4,7,2,1,0,3},
	    {7,6,5,4,3,2,1,0}
      };
  
				       // So now we need to work out
				       // which node needs to be
				       // mapped to the zero node.
				       // The trick is that the node
				       // that should be the local
				       // zero node has three edges
				       // coming into it.
      for (unsigned int i=0; i<mesh.cell_list.size(); ++i)
	{
	  Cell& the_cell = mesh.cell_list[i];
    
					   // This stores whether the
					   // global oriented edge
					   // points in the same
					   // direction as it's local
					   // edge on the current
					   // cube. (for each edge on
					   // the curent cube)
	  EdgeOrientation local_edge_orientation[12];
	  for (unsigned int j = 0; j<12; ++j)
	    {
					       // get the global edge
	      const Edge& the_edge = mesh.edge_list[the_cell.edges[j]];
					       // All edges should be
					       // oriented at this
					       // stage..
	      Assert (the_edge.orientation_flag != unoriented_edge,
		      ExcGridOrientError ("Unoriented edge encountered"));
					       // calculate whether it
					       // points the right way
					       // or not
	      local_edge_orientation[j] = (the_cell.local_orientation_flags[j] ==
					   the_edge.orientation_flag ?
					   forward_edge : backward_edge);
	    }

					   // Here the number of
					   // incoming edges is
					   // tallied for each node.
	  unsigned int perm_num = deal_II_numbers::invalid_unsigned_int;
	  for (unsigned int node_num=0; node_num<8; ++node_num)
	    {
					       // The local edge
					       // numbers coming into
					       // the node
	      const unsigned int e0 = ElementInfo::edge_to_node[node_num][0];
	      const unsigned int e1 = ElementInfo::edge_to_node[node_num][1];
	      const unsigned int e2 = ElementInfo::edge_to_node[node_num][2];

					       // The local
					       // orientation of the
					       // edge coming into the
					       // node.
	      const EdgeOrientation sign0 = ElementInfo::edge_to_node_orient[node_num][0];
	      const EdgeOrientation sign1 = ElementInfo::edge_to_node_orient[node_num][1];
	      const EdgeOrientation sign2 = ElementInfo::edge_to_node_orient[node_num][2];

					       // Add one to the total
					       // for each edge
					       // pointing in
	      Assert (local_edge_orientation[e0] != unoriented_edge,
		      ExcInternalError());
	      Assert (local_edge_orientation[e1] != unoriented_edge,
		      ExcInternalError());
	      Assert (local_edge_orientation[e2] != unoriented_edge,
		      ExcInternalError());
	      
	      const unsigned int
		total  = (((local_edge_orientation[e0] == sign0) ? 1 : 0)
			  +((local_edge_orientation[e1] == sign1) ? 1 : 0)
			  +((local_edge_orientation[e2] == sign2) ? 1 : 0));
      
	      if (total == 3) 
		{
		  Assert (perm_num == deal_II_numbers::invalid_unsigned_int,
			  ExcGridOrientError("More than one node with 3 incoming "
					     "edges found in curent hex.")); 
		  perm_num = node_num;
		}
	    }
					   // We should now have a
					   // valid permutation number
	  Assert (perm_num != deal_II_numbers::invalid_unsigned_int,
		  ExcGridOrientError("No node having 3 incoming edges found in curent hex.")); 

					   // So use the apropriate
					   // rotation to get the new
					   // cube
	  unsigned int temp[8];
	  for (unsigned int i=0; i<8; ++i)
	    temp[i] = the_cell.nodes[CubePermutations[perm_num][i]];
	  for (unsigned int i=0; i<8; ++i)
	    the_cell.nodes[i] = temp[i];
	}
    }
  }
}


template<>
void
GridReordering<3>::reorder_cells (std::vector<CellData<3> > &incubes)
{

  Assert (incubes.size() != 0,
	  ExcMessage("List of elements to orient was of zero length"));
  
				   // This does the real work
  internal::GridReordering3d::Orienter::orient_mesh (incubes);
}


template<>
void
GridReordering<3>::invert_all_cells_of_negative_grid(
  const std::vector<Point<3> > &all_vertices,
  std::vector<CellData<3> > &cells)
{
  unsigned int n_negative_cells=0;
  for (unsigned int cell_no=0; cell_no<cells.size(); ++cell_no)
    if (GridTools::cell_measure(all_vertices, cells[cell_no].vertices) < 0)
      {
	++n_negative_cells;
	for (unsigned int i=0; i<4; ++i)
	  std::swap(cells[cell_no].vertices[i], cells[cell_no].vertices[i+4]);
	
					 // check whether the
					 // resulting cell is now ok.
					 // if not, then the grid is
					 // seriously broken and
					 // should be sticked into the
					 // bin
	AssertThrow(GridTools::cell_measure(all_vertices, cells[cell_no].vertices) > 0,
		    ExcInternalError());
      }

				   // We assuming that all cells of a
				   // grid have either positive or
				   // negative volumes but not both
				   // mixed. Although above reordering
				   // might work also on single cells,
				   // grids with both kind of cells
				   // are very likely to be
				   // broken. Check for this here.
  AssertThrow(n_negative_cells==0 || n_negative_cells==cells.size(), ExcInternalError());
}



	

#endif // deal_II_dimension == 3

