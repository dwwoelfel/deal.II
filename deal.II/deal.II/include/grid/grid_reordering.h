//----------------------------  grid_reordering.h  ---------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  grid_reordering.h  ---------------------------
#ifndef __deal2__grid_reordering_h
#define __deal2__grid_reordering_h


#include <base/config.h>
#include <grid/tria.h>

#include <vector>




/**
 * This class reorders the vertices of cells such that they meet the
 * requirements of the @ref{Triangulation} class when creating
 * grids. This class is mainly used when reading in grids from files
 * and converting them to deal.II triangulations.
 *
 *
 * @sect3{Statement of problems}
 *
 * Triangulations in deal.II have a special structure, in that there
 * are not only cells, but also faces, and in 3d also edges, that are
 * objects of their own right. Faces and edges have unique
 * orientations, and they have a specified orientation also with
 * respect to the cells that are adjacent. Thus, a line that separates
 * two cells in two space dimensions does not only have a direction,
 * but it must also have a well-defined orientation with respect to
 * the other lines bounding the two quadrilaterals adjacent to the
 * first line. Likewise definitions hold for three dimensional cells
 * and the objects (lines, quads) that separate them.
 *
 * For example, in two dimensions, a quad consists of four lines which
 * have a direction, which is by definition as follows:
 * @begin{verbatim}
 *   3-->--2
 *   |     |
 *   ^     ^
 *   |     |
 *   0-->--1
 * @end{verbatim}
 * Now, two adjacent cells must have a vertex numbering such that the direction
 * of the common side is the same. For example, the following two quads
 * @begin{verbatim}
 *   3---4---5
 *   |   |   |
 *   0---1---2
 * @end{verbatim}
 * may be characterised by the vertex numbers @p{(0 1 4 3)} and
 * @p{(1 2 5 4)}, since the middle line would get the direction @p{1->4}
 * when viewed from both cells.  The numbering @p{(0 1 4 3)} and
 * @p{(5 4 1 2)} would not be allowed, since the left quad would give the
 * common line the direction @p{1->4}, while the right one would want
 * to use @p{4->1}, leading to an ambiguity.
 *
 * As a sidenote, we remark that if one adopts the idea that having
 * directions of faces is useful, then the orientation of the four
 * faces of a cell as shown above is almost necessary. In particular,
 * it is not possible to orient them such that they represent a
 * (counter-)clockwise sense, since then we couldn't already find a
 * valid orientation of the following patch of three cells:
 * @begin{verbatim}
 *       o
 *     /   \
 *   o       o 
 *   | \   / |
 *   |   o   |    
 *   |   |   |
 *   o---o---o
 * @end{verbatim}
 * (The reader is asked to try to find a conforming choice of line
 * directions; it will soon be obvious that there can't exists such a
 * thing, even if we allow that there might be cells with clockwise
 * and counterclockwise orientation of the lines at the same time.)
 * 
 * One might argue that the definition of unique directions for faces
 * and edges, and the definition of directions relative to the cells
 * they bound, is a misfeature of deal.II. In fact, it makes reading
 * in grids created by mesh generators rather difficult, as they
 * usually don't follow these conventions when generating their
 * output. On the other hand, there are good reasons to introduce such
 * conventions, as they can make programming much simpler in many
 * cases, leading to an increase in speed of some computations as one
 * can avoid expensive checks in many places because the orientation
 * of faces is known by assumption that it is guaranteed by the
 * triangulation.
 *
 * The purpose of this class is now to find an ordering for a given
 * set of cells such that the generated triangulation satisfies all
 * the requirements stated above. To this end, we will first show some
 * examples why this is a difficult problem, and then develop
 * algorithms that finds such a reordering. Note that the algorithm
 * operates on a set of @ref{CellData} objects that are used to
 * describe a mesh to the triangulation class. These objects are, for
 * example, generated by the @ref{GridIn} class, when reading in grids
 * from input files.
 *
 * As a last question for this first section: is it guaranteed that
 * such orientations of faces always exist for a given subdivision of
 * a domain into cells? The linear complexity algorithm described
 * below for 2d also proves that the answer is yes for 2d. For 3d, the
 * answer is no (which also underlines that using such orientations
 * might be an -- unfortunately uncurable -- misfeature of deal.II). A
 * simple counter-example in 3d illustrates this: take a string of 3d
 * cells and bend it together to a torus. Since opposing lines in a
 * cell need to have the same direction, there is a simple ordering
 * for them, for example all lines radially outward, tangentially
 * clockwise, and axially upward. However, if before joining the two
 * ends of the string of cells, the string is twisted by 180 degrees,
 * then no such orientation is possible any more, as can easily be
 * checked. In effect, some meshes cannot be used in deal.II,
 * unfortunately.
 *
 *
 * @sect3{Examples of problems}
 *
 * As noted, reordering the vertex lists of cells such that the
 * resulting grid is not a trivial problem. In particular, it is often
 * not sufficient to only look at the neighborhood of a cell that
 * cannot be added to a set of other cells without violating the
 * requirements stated above. We will show two examples where this is
 * obvious.
 *
 * The first such example is the following, which we will call the
 * ``four cells at the end'' because of the four cells that close of
 * the right end of a row of three vertical cells each (in the
 * following picture we only show one such column of three cells at
 * the left, but we will indicate what happens if we prolong this
 * list):
 * @begin{verbatim}
 *   9---10-----11
 *   |   |    / |
 *   6---7---8  |
 *   |   |   |  |
 *   3---4---5  |
 *   |   |    \ |
 *   0---1------2
 * @end{verbatim}
 * Assume that you had numbered the vertices in the cells at the left boundary
 * in a way, that the following line directions are induced:
 * @begin{verbatim}
 *   9->-10-----11
 *   ^   ^    / |
 *   6->-7---8  |
 *   ^   ^   |  |
 *   3->-4---5  |
 *   ^   ^    \ |
 *   0->-1------2
 * @end{verbatim}
 * (This could for example be done by using the indices @p{(0 1 4 3)}, @p{(3 4 7 6)},
 * @p{(6 7 10 9)} for the three cells). Now, you will not find a way of giving
 * indices for the right cells, without introducing either ambiguity for
 * one line or other, or without violating that within each cells, there must be
 * one vertex from which both lines are directed away and the opposite one to
 * which both adjacent lines point to.
 *
 * The solution in this case is to renumber one of the three left cells, e.g.
 * by reverting the sense of the line between vertices 7 and 10 by numbering
 * the top left cell by @p{(9 6 7 10)}:
 * @begin{verbatim}
 *   9->-10-----11
 *   v   v    / |
 *   6->-7---8  |
 *   ^   ^   |  |
 *   3->-4---5  |
 *   ^   ^    \ |
 *   0->-1------2
 * @end{verbatim}
 *
 * The point here is the following: assume we wanted to prolong the grid to 
 * the left like this:
 * @begin{verbatim}
 *   o---o---o---o---o------o
 *   |   |   |   |   |    / |
 *   o---o---o---o---o---o  |
 *   |   |   |   |   |   |  |
 *   o---o---o---o---o---o  |
 *   |   |   |   |   |    \ |
 *   o---o---o---o---o------o
 * @end{verbatim}
 * Then we run into the same problem as above if we order the cells at
 * the left uniformly, thus forcing us to revert the ordering of one
 * cell (the one which we could order as @p{(9 6 7 10)}
 * above). However, since opposite lines have to have the same
 * direction, this in turn would force us to rotate the cell left of
 * it, and then the one left to that, and so on until we reach the
 * left end of the grid. This is therefore an example we we have to
 * track back right until the first column of three cells to find a
 * consistent ordering, if we had initially ordered them uniformly.
 *
 * As a second example, consider the following simple grid, where the
 * order in which the cells are numbered is important:
 * @begin{verbatim}
 *   3-----2-----o-----o ... o-----7-----6
 *   |     |     |     |     |     |     |
 *   |  0  |  N  | N-1 | ... |  2  |  1  |
 *   |     |     |     |     |     |     |
 *   0-----1-----o-----o ... o-----4-----5
 * @end{verbatim}
 * We have here only indicated the numbers of the vertices that are
 * relevant. Assume that the user had given the cells 0 and 1 by the
 * vertex indices @p{0 1 2 3} and @p{6 7 4 5}. Then, if we follow this
 * orientation, the grid after creating the lines for these two cells
 * would look like this:
 * @begin{verbatim}
 *   3-->--2-----o-----o ... o-----7--<--6
 *   |     |     |     |     |     |     |
 *   ^  0  ^  N  | N-1 | ... |  2  v  1  v
 *   |     |     |     |     |     |     |
 *   0-->--1-----o-----o ... o-----4--<--5
 * @end{verbatim}
 * Now, since opposite lines must point in the same direction, we can
 * only add the cells 2 through N-1 to cells 1 such that all vertical
 * lines point down. Then, however, we cannot add cell N in any
 * direction, as it would have two opposite lines that do not point in
 * the same direction. We would have to rotate either cell 0 or 1 in
 * order to be able to add all the other cells such that the
 * requirements of deal.II triangulations are met.
 *
 * These two examples demonstrate that if we have added a certain
 * number of cells in some orientation of faces and can't add the next
 * one without introducing faces that had already been added in another
 * direction, then it might not be sufficient to only rotate cells in
 * the neighborhood of the the cell that we failed to add. It might be
 * necessary to go back a long way and rotate cells that have been
 * entered long ago.
 *
 *
 * @sect3{Solution}
 *
 * From the examples above, it is obvious that if we encounter a cell
 * that cannot be added to the cells which have already been entered,
 * we can not usually point to a cell that is the culprit and that
 * must be entered in a different oreintation. Furthermore, even if we
 * knew which cell, there might be large number of cells that would
 * then cease to fit into the grid and which we would have to find a
 * different orientation as well (in the second example above, if we
 * rotated cell 1, then we would have to rotate the cells 1 through
 * N-1 as well).
 *
 * A brute force approach to this problem is the following: if
 * cell N can't be added, then try to rotate cell N-1. If we can't
 * rotate cell N-1 any more, then try to rotate cell N-2 and try to
 * add cell N with all orientations of cell N-1. And so
 * on. Algorithmically, we can visualize this by a tree structure,
 * where node N has as many children as there are possible
 * orientations of node N+1 (in two space dimensions, there are four
 * orientations in which each cell can be constructed from its four
 * vertices; for example, if the vertex indicaes are @p{(0 1 2 3)},
 * then the four possibilities would be @p{(0 1 2 3)}, @p{(1 2 3 0)},
 * @p{(2 3 0 1)}, and @p{(3 0 1 2)}). When adding one cell after the
 * other, we traverse this tree in a depth-first (pre-order)
 * fashion. When we encounter that one path from the root (cell 0) to
 * a leaf (the last cell) is not allowed (i.e. that the orientations
 * of the cells which are encoded in the path through the tree does
 * not lead to a valid triangulation), we have to track back and try
 * another path through the tree.
 *
 * In practice, of course, we do not follow each path to a final node
 * and then find out whether a path leads to a valid triangulation,
 * but rather use an inductive argument: if for all previously added
 * cells the triangulation is a valid one, then we can find out
 * whether a path through the tree can yield a valid triangulation by
 * checking whether entering the present cell would introduce any
 * faces that have a nonunique direction; if that is so, then we can
 * stop following all paths below this point and track back
 * immediately.
 *
 * Nevertheless, it is already obvious that the tree has @p{4**N}
 * leaves in two space dimensions, since each of the N cells can be
 * added in four orientations. Most of these nodes can be discarded
 * rapidly, since firstly the orientation of the first cell is
 * irrelevant, and secondly if we add one cell that has a neighbor
 * that has already been added, then there are already only two
 * possible orientations left, so the total number of checks we have
 * to make until we find a valid way is significantly smaller than
 * @p{4**N}. However, the algorithm is still exponential in time and
 * linear in memory (we only have to store the information for the
 * present path in form of a stack of orientations of cells that have
 * already been added).
 *
 * In fact, the two examples above show that the exponential estimate
 * is not a pessimized one: we indeed have to track back to one of the
 * very first cells there to find a way to add all cells in a
 * consistent fashion.
 *
 * This discouraging situation is greatly improved by the fact that we
 * have an alternative algorithm for 2d that is always linear in
 * runtime (discovered and implemented by Michael Anderson of TICAM,
 * University of Texas, in 2003), and that for 3d we can find an
 * algorithm that in practice is usually only roughly linear in time
 * and memory. We will describe these algorithms in the following.
 *
 *
 * @sect3{The 2d linear complexity algorithm}
 *
 * The algorithm uses the fact that opposite faces of a cell need to
 * have the same orientation. So you start with one arbitrary line,
 * choose an orientation. Then the orientation of the opposite face is
 * already fixed. Then go to the two cells across the two faces we
 * have fixed: for them, one face is fixed, so we can also fix the
 * opposite face. Go on with doing so. Eventually, we have done this
 * for a string of cells. Then take one of the non-fixed faces of a
 * cell which has already two fixed faces and do all this again.
 *
 * In more detail, the algorithm is best illustrated using an
 * example. We consider the mesh below:
 * @begin{verbatim}
 *   9------10-------11
 *   |      |        /|
 *   |      |       / |
 *   |      |      /  |
 *   6------7-----8   |
 *   |      |     |   |
 *   |      |     |   |
 *   |      |     |   |
 *   3------4-----5   |
 *   |      |      \  |
 *   |      |       \ |
 *   |      |        \|
 *   0------1---------2
 * @end{verbatim}
 * First a cell is chosen ( (0,1,4,3) in this case). A single side of the cell
 * is oriented arbitrarily (3->4). This choice of orientation is then propogated
 * through the mesh, across sides and elements. (0->1), (6->7) and (9->10).
 * The involves edge-hopping and face hopping, giving a path through the mesh
 * shown in dots.
 * @begin{verbatim}
 *   9-->--10-------11
 *   |  .  |        /|
 *   |  .  |       / |
 *   |  .  |      /  |
 *   6-->--7-----8   |
 *   |  .  |     |   |
 *   |  .  |     |   |
 *   |  .  |     |   |
 *   3-->--4-----5   |
 *   |  .  |      \  |
 *   |  X  |       \ |
 *   |  .  |        \|
 *   0-->--1---------2
 * @end{verbatim}
 * This is then repeated for the other sides of the chosen element, orienting
 * more sides of the mesh.
 * @begin{verbatim}
 *   9-->--10-------11
 *   |     |        /|
 *   v.....v.......V |
 *   |     |      /. |
 *   6-->--7-----8 . |
 *   |     |     | . |
 *   |     |     | . |
 *   |     |     | . |
 *   3-->--4-----5 . |
 *   |     |      \. |
 *   ^..X..^.......^ |
 *   |     |        \|
 *   0-->--1---------2
 * @end{verbatim}
 * Once an element has been completely oriented it need not be considered
 * further. These elements are filled with o's in the diagrams. We then move
 * to the next element.
 * @begin{verbatim}
 *   9-->--10->-----11
 *   | ooo |  .     /|
 *   v ooo v  .    V |
 *   | ooo |  .   /  |
 *   6-->--7-->--8   |
 *   |     |  .  |   |
 *   |     |  .  |   |
 *   |     |  .  |   |
 *   3-->--4-->--5   |
 *   | ooo |  .   \  |
 *   ^ ooo ^  X    ^ |
 *   | ooo |  .     \|
 *   0-->--1-->------2
 * @end{verbatim}
 * Repeating this gives
 * @begin{verbatim}
 *   9-->--10->-----11
 *   | ooo | oooooo /|
 *   v ooo v ooooo V |
 *   | ooo | oooo /  |
 *   6-->--7-->--8   |
 *   |     |     |   |
 *   ^.....^..X..^...^
 *   |     |     |   |
 *   3-->--4-->--5   |
 *   | ooo | oooo \  |
 *   ^ ooo ^ ooooo ^ |
 *   | ooo | oooooo \|
 *   0-->--1-->------2
 * @end{verbatim}
 * and the final oriented mesh is
 * @begin{verbatim}
 *   9-->--10->-----11
 *   |     |        /|
 *   v     v       V |
 *   |     |      /  |
 *   6-->--7-->--8   |
 *   |     |     |   |
 *   ^     ^     ^   ^
 *   |     |     |   |
 *   3-->--4-->--5   |
 *   |     |      \  |
 *   ^     ^       ^ |
 *   |     |        \|
 *   0-->--1-->-------2
 * @end{verbatim}
 * It is obvious that this algorithm has linear run-time, since it
 * only ever touches each face exactly once.
 *
 * The algorithm just described is implemented in a specialization of this
 * class for the 2d case. A similar, but slightly more complex algorithm is
 * implemented in a specialization for 3d. It using sheets instead of strings
 * of cells to work on. If a grid is orientable, then the algorithm is able to
 * do its work in linear time; if it is not orientable, then it aborts in
 * linear time as well. Both algorithms are described in a paper by Agelek,
 * Anderson, Bangerth and Barth, see the publications page of the deal.II
 * library.
 *
 *
 * @sect3{For the curious}
 *
 * Prior to the implementation of the algorithms developed by Michael Anderson
 * and described above, we used a branch-and-cut algorithm initially
 * implemented in 2000 by Wolfgang Bangerth. Although it is no longer used
 * here is how it works, and why it doesn't always work for large meshes since
 * its run-time can exponential in bad cases.
 * 
 * The first observation is that although there are counterexamples,
 * problems are usually local. For example, in the second example
 * mentioned above, if we had numbered the cells in a way that
 * neighboring cells have similar cell numbers, then the amount of
 * backtracking needed is greatly reduced. Therefore, in the
 * implementation of the algorithm, the first step is to renumber the
 * cells in a Cuthill-McKee fashion: start with the cell with the
 * least number of neighbors and assign to it the cell number
 * zero. Then find all neighbors of this cell and assign to them
 * consecutive further numbers. Then find their neighbors that have
 * not yet been numbered and assign to them numbers, and so
 * on. Graphically, this represents finding zones of cells
 * consecutively further away from the initial cells and number them
 * in this front-marching way. This already greatly improves locality
 * of problems and consequently reduced the necessary amount of
 * backtracking.
 *
 * The second point is that we can use some methods to prune the tree,
 * which usually lead to a valid orientation of all cells very
 * quickly.
 *
 * The first such method is based on the observation that if we
 * fail to insert one cell with number N, then this may not be due to
 * cell N-1 unless N-1 is a direct neighbor of N. The reason is
 * abvious: the chosen orientation of cell M could only affect the
 * possibilities to add cell N if either it were a direct neighbor or
 * if there were a sequence of cells that were added after M and that
 * connected cells M and N. Clearly, for M=N-1, the latter cannot be
 * the case. Conversely, if we fail to add cell N, then it is not
 * necessary to track back to cell N-1, but we can track back to the
 * neighbor of N with the largest cell index and which has already
 * been added.
 *
 * Unfortunately, this method can fail to yield a valid path through
 * the tree if not applied with care. Consider the following
 * situation, initially extracted from a mesh of 950 cells generated
 * automatically by the program BAMG (this program usually generates
 * meshes that are quite badly balanced, often have many -- sometimes
 * 10 or more -- neighbors of one vertex, and exposed several problems
 * in the initial algorithm; note also that the example is in 2d where
 * we now have the much better algorithm described above, but the same
 * observations also apply to 3d):
 * @begin{verbatim}
 * 13----------14----15
 * | \         |     |
 * |  \    4   |  5  |
 * |   \       |     |
 * |    12-----10----11
 * |     |     |     |
 * |     |     |  7  |
 * |     |     |     |
 * |  3  |     8-----9
 * |     |     |     |
 * |     |     |  6  |
 * |     |     |     |
 * 4-----5-----6-----7
 * |     |     |     |
 * |  2  |  1  |  0  |
 * |     |     |     |
 * 0-----1-----2-----3
 * @end{verbatim}
 * Note that there is a hole in the middle. Assume now that the user
 * described the first cell 0 by the vertex numbers @p{2 3 7 6}, and
 * cell 5 by @p{15 14 10 11}, and assume that cells 1, 2, 3, and 4 are
 * numbered such that 5 can be added in initial rotation. All other
 * cells are numbered in the usual way, i.e. starting at the bottom
 * left and counting counterclockwise. Given this description of
 * cells, the algorithm will start with cell zero and add one cell
 * after the other, up until the sixth one. Then the situation will be
 * the following:
 * @begin{verbatim}
 * 13----->---14--<--15
 * | \         |     |
 * |  >    4   v  5  v
 * |   \       |     |
 * |    12->--10--<--11
 * |     |     |     |
 * ^     |     |  7  |
 * |     |     |     |
 * |  3  ^     8-->--9
 * |     |     |     |
 * |     |     ^  6  ^
 * |     |     |     |
 * 4-->--5-->--6-->--7
 * |     |     |     |
 * ^  2  ^  1  ^  0  ^
 * |     |     |     |
 * 0-->--1-->--2-->--3
 * @end{verbatim}
 * Coming now to cell 7, we see that the two opposite lines at its top
 * and bottom have different directions; we will therefore find no
 * orientation of cell 7 in which it can be added without violation of
 * the consistency of the triangulation. According to the rule stated
 * above, we track back to the neighbor with greatest index, which is
 * cell 6, but since its bottom line is to the right, its top line
 * must be to the right as well, so we won't be able to find an
 * orientation of cell 6 such that 7 will fit into the
 * triangulation. Then, if we have finished all possible orientations
 * of cell 6, we track back to the neighbor of 6 with the largest
 * index and which has been added already. This would be cell
 * 0. However, we know that the orientation of cell 0 can't be
 * important, so we conclude that there is no possible way to orient
 * all the lines of the given cells such that they satisfy the
 * requirements if deal.II triangulations. We know that this can't be,
 * so it results in an exception be thrown.
 *
 * The bottom line of this example is that when we looked at all
 * possible orientations of cell 6, we couldn't find one such that
 * cell 7 could be added, and then decided to track back to cell 0. We
 * did not even attempt to turn cell 5, after which it would be simple
 * to add cell 7. Thus, the algorithm described above has to be
 * modified: we are only allowed to track back to that neighbor that
 * has already been added, with the largest cell index, if we fail to
 * add a cell in any orientation. If we track back further because we
 * have exhausted all possible orientations but could add the cell
 * (i.e. we track back since another cell, further down the road
 * couldn't be added, irrespective of the orientation of the cell
 * which we are presently considering), then we are not allowed to
 * track back to one of its neighbors, but have to track back only one
 * cell index.
 *
 * The second method to prune the tree is that usually we cannot add a
 * new cell since the orientation of one of its neighbors that have
 * already been added is wrong. Thus, if we may try to rotate one of
 * the neighbors (of course making sure that rotating that neighbor
 * does not violate the consistency of the triangulation) in order to
 * allow the present cell to be added.
 *
 * While the first method could be explained in terms of backtracking
 * in the tree of orientations more than one step at once, turning a
 * neighbor means jumping to a totally different place in the
 * tree. For both methods, one can find arguments that they will never
 * miss a path that is valid and only skip paths that are invalid
 * anyway.
 *
 * These two methods have proven extremely efficient. We have been
 * able to read very large grids (several ten thousands of cells)
 * without the need to track back much. In particular, the time to find
 * an ordering of the cells was found to be mostly linear in the
 * number of cells, and the time to reorder them is usually much
 * smaller (for example by one order of magnitude) than the time
 * needed to read the data from a file, and also to actually generate
 * the triangulation from this data using the
 * @ref{Triangulation}@p{<dim>::create_triangulation} function.
 *
 * @author Wolfgang Bangerth, 2000, Michael Anderson 2003
 */
template <int dim>
class GridReordering
{
};



/**
 * This is the specialization of the general template for 1d. In 1d,
 * there is actually nothing to be done.
 *
 * @author Wolfgang Bangerth, 2000
 */
template <>
class GridReordering<1>
{
  public:
				     /**
				      * Do nothing, since in 1d no
				      * reordering is necessary.
				      */
    static void reorder_cells (const std::vector<CellData<1> > &);
};



/**
 * This specialization of the general template implements the
 * 2d-algorithm described in the documentation of the general
 * template.
 *
 * @author Michael Anderson, 2003
 */
template <>
class GridReordering<2>
{
  public:
				     /**
				      *  This is the main function,
				      *  doing what is announced in
				      *  the general documentation of
				      *  this class.
				      */
    static void reorder_cells (std::vector<CellData<2> > &original_cells);
};



/**
 * This specialization of the general template implements the
 * 3d-algorithm described in the documentation of the general
 * template.
 *
 * @author Michael Anderson, 2003
 */
template <>
class GridReordering<3>
{
  public:
				     /**
				      *  This is the main function,
				      *  doing what is announced in
				      *  the general documentation of
				      *  this class.
				      */
    static void reorder_cells (std::vector<CellData<3> > &original_cells);
};




#endif
