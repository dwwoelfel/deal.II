// $Id$


#include <numerics/time-dependent.h>
#include <grid/tria.h>
#include <grid/tria_accessor.h>
#include <grid/tria_iterator.h>
#include <lac/vector.h>

#include <functional>
#include <algorithm>
#include <numeric>


TimeDependent::TimeSteppingData::TimeSteppingData (const unsigned int look_ahead,
						   const unsigned int look_back)
		:
		look_ahead (look_ahead),
		look_back (look_back)
{};




TimeDependent::TimeDependent (const TimeSteppingData &data_primal,
			      const TimeSteppingData &data_dual,
			      const TimeSteppingData &data_postprocess):
		sweep_no (static_cast<unsigned int>(-1)),
		timestepping_data_primal (data_primal),
		timestepping_data_dual (data_dual),
		timestepping_data_postprocess (data_postprocess)
{};




TimeDependent::~TimeDependent ()
{
  while (timesteps.size() != 0)
    delete_timestep (0);
};



void
TimeDependent::insert_timestep (TimeStepBase      *new_timestep,
				const unsigned int position) 
{
  Assert (position<=timesteps.size(),
	  ExcInvalidPosition(position, timesteps.size()));

				   // lock this timestep from deletion
  new_timestep->subscribe();

				   // first insert the new time step
				   // into the doubly linked list
				   // of timesteps
  if (position == timesteps.size())
    {
				       // at the end
      new_timestep->set_next_timestep (0);
      if (timesteps.size() > 0)
	{
	  timesteps.back()->set_next_timestep (new_timestep);
	  new_timestep->set_previous_timestep (timesteps.back());
	}
      else
	new_timestep->set_previous_timestep (0);
    }
  else
    if (position == 0)
      {
					 // at the beginning
	new_timestep->set_previous_timestep (0);
	if (timesteps.size() > 0)
	  {
	    timesteps[0]->set_previous_timestep (new_timestep);
	    new_timestep->set_next_timestep (timesteps[0]);
	  }
	else
	  new_timestep->set_next_timestep (0);
      }
    else
      {
					 // inner time step
	timesteps[position-1]->set_next_timestep (new_timestep);
	new_timestep->set_next_timestep (timesteps[position]);
	timesteps[position]->set_previous_timestep (new_timestep);
      };

				   // finally enter it into the
				   // array
  timesteps.insert (&timesteps[position], new_timestep);
};



void
TimeDependent::add_timestep (TimeStepBase *new_timestep)
{
  insert_timestep (new_timestep, timesteps.size());
};



void TimeDependent::delete_timestep (const unsigned int position)
{
  Assert (position<timesteps.size(),
	  ExcInvalidPosition(position, timesteps.size()));

  timesteps[position]->unsubscribe();
  delete timesteps[position];
  timesteps.erase (&timesteps[position]);

				   // reset "next" pointer of previous
				   // time step if possible
				   //
				   // note that if now position==size,
				   // then we deleted the last time step
  if (position != 0)
    timesteps[position-1]->set_next_timestep ((position<timesteps.size()) ?
					      timesteps[position] :
					      0);

				   // same for "previous" pointer of next
				   // time step
  if (position<timesteps.size())
    timesteps[position]->set_previous_timestep ((position!=0) ?
						timesteps[position-1] :
						0);
};



void
TimeDependent::solve_primal_problem () 
{
  do_loop (mem_fun(&TimeStepBase::init_for_primal_problem),
	   mem_fun(&TimeStepBase::solve_primal_problem),
	   timestepping_data_primal);
};



void
TimeDependent::solve_dual_problem () 
{
  do_loop (mem_fun(&TimeStepBase::init_for_dual_problem),
	   mem_fun(&TimeStepBase::solve_dual_problem),
	   timestepping_data_dual);
};



void
TimeDependent::postprocess () 
{
  do_loop (mem_fun(&TimeStepBase::init_for_postprocessing),
	   mem_fun(&TimeStepBase::postprocess_timestep),
	   timestepping_data_postprocess);
};





void TimeDependent::start_sweep (const unsigned int s) 
{
  sweep_no = s;

				   // reset the number each
				   // time step has, since some time
				   // steps might have been added since
				   // the last time we visited them
				   //
				   // also set the sweep we will
				   // process in the sequel
  for (unsigned int step=0; step<timesteps.size(); ++step)
    {
      timesteps[step]->set_timestep_no (step);
      timesteps[step]->set_sweep_no (sweep_no);
    };

  for (unsigned int step=0; step<timesteps.size(); ++step)
    timesteps[step]->init_for_sweep ();
};



/* --------------------------------------------------------------------- */



TimeStepBase::TimeStepBase (const double time) :
		previous_timestep(0),
		next_timestep (0),
		sweep_no (static_cast<unsigned int>(-1)),
		timestep_no (static_cast<unsigned int>(-1)),
		time (time)
{};



TimeStepBase::~TimeStepBase () 
{};



void
TimeStepBase::wake_up (const unsigned )
{};



void
TimeStepBase::sleep (const unsigned)
{};



void
TimeStepBase::init_for_sweep () 
{};



void
TimeStepBase::init_for_primal_problem () 
{
  next_action = primal_problem;
};



void
TimeStepBase::init_for_dual_problem () 
{
  next_action = dual_problem;
};



void
TimeStepBase::init_for_postprocessing () 
{
  next_action = postprocess;
};




void
TimeStepBase::solve_dual_problem () 
{
  Assert (false, ExcPureVirtualFunctionCalled());
};



void
TimeStepBase::postprocess_timestep () 
{
  Assert (false, ExcPureVirtualFunctionCalled());
};



double
TimeStepBase::get_backward_timestep () const
{
  Assert (previous_timestep != 0, ExcCantComputeTimestep());
  return time - previous_timestep->time;
};



double
TimeStepBase::get_forward_timestep () const
{
  Assert (next_timestep != 0, ExcCantComputeTimestep());
  return next_timestep->time - time;
};



void
TimeStepBase::set_previous_timestep (const TimeStepBase *previous)
{
  previous_timestep = previous;
};



void
TimeStepBase::set_next_timestep (const TimeStepBase *next)
{
  next_timestep     = next;
};



void
TimeStepBase::set_timestep_no (const unsigned int step_no)
{
  timestep_no = step_no;
};



void
TimeStepBase::set_sweep_no (const unsigned int sweep)
{
  sweep_no = sweep;
};



/* ------------------------------------------------------------------------- */





template <int dim>
TimeStepBase_Tria<dim>::TimeStepBase_Tria() :
		TimeStepBase (0),
		tria (0),
		coarse_grid (*reinterpret_cast<Triangulation<dim>*>(0))
{
  Assert (false, ExcPureVirtualFunctionCalled());
};



template <int dim>
TimeStepBase_Tria<dim>::TimeStepBase_Tria (const double              time,
					   const Triangulation<dim> &coarse_grid,
					   const Flags              &flags,
					   const RefinementFlags    &refinement_flags) :
		TimeStepBase (time),
		tria(0),
		coarse_grid (coarse_grid),
		flags (flags),
		refinement_flags (refinement_flags)
{
  coarse_grid.subscribe();
};



template <int dim>
TimeStepBase_Tria<dim>::~TimeStepBase_Tria () 
{
  if (!flags.delete_and_rebuild_tria)
    {
      tria->unsubscribe ();
      delete tria;
    }
  else
    Assert (tria==0, ExcInternalError());

  coarse_grid.unsubscribe();
};



template <int dim>
void
TimeStepBase_Tria<dim>::wake_up (const unsigned wakeup_level) {
  TimeStepBase::wake_up (wakeup_level);
  
  if (wakeup_level == flags.wakeup_level_to_build_grid)
    if (flags.delete_and_rebuild_tria || !tria)
      restore_grid ();
};



template <int dim>
void
TimeStepBase_Tria<dim>::sleep (const unsigned sleep_level)
{
  if (sleep_level == flags.sleep_level_to_delete_grid)
    {
      Assert (tria!=0, ExcInternalError());
      
      if (flags.delete_and_rebuild_tria)
	{
	  tria->unsubscribe();
	  delete tria;
	  tria = 0;
	};
    };

  TimeStepBase::sleep (sleep_level);
};




template <int dim>
void TimeStepBase_Tria<dim>::save_refine_flags ()
{
  				   // for any of the non-initial grids
				   // store the refinement flags
  refine_flags.push_back (vector<bool>());
  coarsen_flags.push_back (vector<bool>());
  tria->save_refine_flags (refine_flags.back());
  tria->save_coarsen_flags (coarsen_flags.back());
};



template <int dim>
void TimeStepBase_Tria<dim>::restore_grid () {
  Assert (tria == 0, ExcGridNotDeleted());
  Assert (refine_flags.size() == coarsen_flags.size(),
	  ExcInternalError());

				   // create a virgin triangulation and
				   // set it to a copy of the coarse grid
  tria = new Triangulation<dim> ();
  tria->subscribe();
  tria->copy_triangulation (coarse_grid);

				   // for each of the previous refinement
				   // sweeps
  for (unsigned int previous_sweep=0; previous_sweep<refine_flags.size();
       ++previous_sweep) 
    {
				       // get flags
      tria->load_refine_flags  (refine_flags[previous_sweep]);
      tria->load_coarsen_flags (coarsen_flags[previous_sweep]);

				       // limit refinement depth if the user
				       // desired so
//       if (flags.max_refinement_level != 0)
// 	{
// 	  Triangulation<dim>::active_cell_iterator cell, endc;
// 	  for (cell = tria->begin_active(),
// 	       endc = tria->end();
// 	       cell!=endc; ++cell)
// 	    if (static_cast<unsigned int>(cell->level()) >=
// 		flags.max_refinement_level)
// 	      cell->clear_refine_flag();
// 	};

      tria->execute_coarsening_and_refinement ();
    };
};




template <int dim>
void TimeStepBase_Tria<dim>::refine_grid (const RefinementData &refinement_data)
{
				   // copy the following two values since
				   // we may need modified values in the
				   // process of this function
  double refinement_threshold = refinement_data.refinement_threshold,
	 coarsening_threshold = refinement_data.coarsening_threshold;

				   // prepare an array where the partial
				   // sums of the criteria are stored
				   // we need this if cell number correction
				   // is switched on
  Vector<float> partial_sums;
				   // two pointers into this array denoting
				   // the position where the two thresholds
				   // are assumed (sorry for the stupid
				   // naming of these variables, they were
				   // chosen when the whole function had about
				   // 30 lines of code and I am now too lazy
				   // to change it...)
  Vector<float>::const_iterator p=0, q=0;
  
  if ((timestep_no != 0) &&
      (sweep_no>=refinement_flags.first_sweep_with_correction) &&
      (refinement_flags.cell_number_correction_steps > 0))
    {
      partial_sums = refinement_data.criteria;
      sort (partial_sums.begin(), partial_sums.end(),
	    greater<float>());
      partial_sum (partial_sums.begin(), partial_sums.end(),
		   partial_sums.begin());
      p = upper_bound (partial_sums.begin(), partial_sums.end(),
		       coarsening_threshold);
      q = lower_bound (partial_sums.begin(), partial_sums.end(),
		       refinement_threshold);
    };

  
  
				   // actually flag cells the first time
  tria->refine (refinement_data.criteria, refinement_threshold);
  tria->coarsen (refinement_data.criteria, coarsening_threshold);

				   // store this number for the following
				   // since its computation is rather
				   // expensive and since it doesn't change
  const unsigned int n_active_cells = tria->n_active_cells ();
      
				   // if not on first time level: try to
				   // adjust the number of resulting
				   // cells to those on the previous
				   // time level. Only do the cell number
				   // correction for higher sweeps and if
				   // there are sufficiently many cells
				   // already to avoid "grid stall" i.e.
				   // that the grid's evolution is hindered
				   // by the correction (this usually
				   // happens if there are very few cells,
				   // since then the number of cells touched
				   // by the correction step may exceed the
				   // number of cells which are flagged for
				   // refinement; in this case it often
				   // happens that the number of cells
				   // does not grow between sweeps, which
				   // clearly is not the wanted behaviour)
				   //
				   // however, if we do not do anything, we
				   // can get into trouble somewhen later.
				   // therefore, we also use the correction
				   // step for the first sweep or if the
				   // number of cells is between 100 and 300
				   // (unlike in the first version of the
				   // algorithm), but relax the conditions
				   // for the correction to allow deviations
				   // which are three times as high than
				   // allowed (sweep==1 || cell number<200)
				   // or twice as high (sweep==2 ||
				   // cell number<300). Also, since
				   // refinement never does any harm other
				   // than increased work, we allow for
				   // arbitrary growth of cell number if
				   // the estimated cell number is below
				   // 200.
				   //
				   // repeat this loop several times since
				   // the first estimate may not be totally
				   // correct
  if ((timestep_no != 0) && (sweep_no>=refinement_flags.first_sweep_with_correction))
    for (unsigned int loop=0;
	 loop<refinement_flags.cell_number_correction_steps; ++loop)
      {
	Triangulation<dim> *previous_tria
	  = dynamic_cast<const TimeStepBase_Tria<dim>*>(previous_timestep)->tria;

					 // do one adaption step if desired
					 // (there are more coming below then
					 // also)
	if (refinement_flags.adapt_grids)
	  adapt_grids (*previous_tria, *tria);
	
					 // perform flagging of cells
					 // needed to regularize the
					 // triangulation
	tria->prepare_coarsening ();
	tria->prepare_refinement ();
	previous_tria->prepare_coarsening ();
	previous_tria->prepare_refinement ();

      
      
					     // now count the number of elements
					     // which will result on the previous
					     // grid after it will be refined. The
					     // number which will really result
					     // should be approximately that that we
					     // compute here, since we already
					     // performed most of the prepare*
					     // steps for the previous grid
					     //
					     // use a double value since for each
					     // four cells (in 2D) that we flagged
					     // for coarsening we result in one
					     // new. but since we loop over flagged
					     // cells, we have to subtract 3/4 of
					     // a cell for each flagged cell
	double previous_cells = previous_tria->n_active_cells();
	Triangulation<dim>::active_cell_iterator cell, endc;
	cell = previous_tria->begin_active();
	endc = previous_tria->end();
	for (; cell!=endc; ++cell)
	  if (cell->refine_flag_set())
	    previous_cells += (GeometryInfo<dim>::children_per_cell-1);
	  else
	    if (cell->coarsen_flag_set())
	      previous_cells -= (GeometryInfo<dim>::children_per_cell-1) /
				GeometryInfo<dim>::children_per_cell;
	    
					 // #previous_cells# now gives the
					 // number of cells which would result
					 // from the flags on the previous grid
					 // if we refined it now. However, some
					 // more flags will be set when we adapt
					 // the previous grid with this one
					 // after the flags have been set for
					 // this time level; on the other hand,
					 // we don't account for this, since the
					 // number of cells on this time level
					 // will be changed afterwards by the
					 // same way, when it is adapted to the
					 // next time level

					     // now estimate the number of cells which
					     // will result on this level
	double estimated_cells = n_active_cells;
	cell = tria->begin_active();
	endc = tria->end();
	for (; cell!=endc; ++cell)
	  if (cell->refine_flag_set())
	    estimated_cells += (GeometryInfo<dim>::children_per_cell-1);
	  else
	    if (cell->coarsen_flag_set())
	      estimated_cells -= (GeometryInfo<dim>::children_per_cell-1) /
				 GeometryInfo<dim>::children_per_cell;

					 // calculate the allowed delta in
					 // cell numbers; be more lenient
					 // if there are few cells
	double delta_up = refinement_flags.cell_number_corridor_top,
	     delta_down = refinement_flags.cell_number_corridor_bottom;

	const vector<pair<unsigned int,double> > &relaxations
	  = (sweep_no > refinement_flags.correction_relaxations.size() ?
	     refinement_flags.correction_relaxations.back() :
	     refinement_flags.correction_relaxations[sweep_no]);
	for (unsigned int r=0; r!=relaxations.size(); ++r)
	  if (n_active_cells < relaxations[r].first)
	    {
	      delta_up   *= relaxations[r].second;
	      delta_down *= relaxations[r].second;
	      break;
	    };
	  
					 // now, if the number of estimated
					 // cells exceeds the number of cells
					 // on the old time level by more than
					 // delta: cut the top threshold
					 //
					 // note that for each cell that
					 // we unflag we have to diminish the
					 // estimated number of cells by
					 // #children_per_cell#.
	if (estimated_cells > previous_cells*(1.+delta_up)) 
	  {
					     // only limit the cell number
					     // if there will not be less
					     // than some number of cells
					     //
					     // also note that when using the
					     // dual estimator, the initial
					     // time level is not refined
					     // on its own, so we may not
					     // limit the number of the second
					     // time level on the basis of
					     // the initial one; since for
					     // the dual estimator, we
					     // mirror the refinement
					     // flags, the initial level
					     // will be passively refined
					     // later on.
	    if (estimated_cells>refinement_flags.min_cells_for_correction)
	      {
						 // number of cells by which the
						 // new grid is to be diminished
		double delta_cells = estimated_cells -
				     previous_cells*(1.+delta_up);
		    
		for (unsigned int i=0; i<delta_cells;
		     i += (GeometryInfo<dim>::children_per_cell-1))
		  if (q!=partial_sums.begin())
		    --q;
		  else
		    break;
	      }
	    else
					       // too many cells, but we
					       // won't do anything about
					       // that
	      break;
	  }
	else
					   // likewise: if the estimated number
					   // of cells is less than 90 per cent
					   // of those at the previous time level:
					   // raise threshold by refining
					   // additional cells. if we start to
					   // run into the area of cells
					   // which are to be coarsened, we
					   // raise the limit for these too
	  if (estimated_cells < previous_cells*(1.-delta_down))
	    {
					       // number of cells by which the
					       // new grid is to be enlarged
	      double delta_cells = previous_cells*(1.-delta_down)-estimated_cells;
					       // heuristics: usually, if we
					       // add #delta_cells# to the
					       // present state, we end up
					       // with much more than only
					       // (1-delta_down)*prev_cells
					       // because of the effect of
					       // regularization and because
					       // of adaption to the
					       // following grid. Therefore,
					       // if we are not in the last
					       // correction loop, we try not
					       // to add as many cells as seem
					       // necessary at first and hope
					       // to get closer to the limit
					       // this way. Only in the last
					       // loop do we have to take the
					       // full number to guarantee the
					       // wanted result.
					       //
					       // The value 0.9 is taken from
					       // practice, as the additional
					       // number of cells introduced
					       // by regularization is
					       // approximately 10 per cent
					       // of the flagged cells.
	      if (loop != refinement_flags.cell_number_correction_steps-1)
		delta_cells *= 0.9;
		  

	      for (unsigned int i=0; i<delta_cells;
		   i += (GeometryInfo<dim>::children_per_cell-1))
		if (q!=p)
		  ++q;
		else
		  if (p!=partial_sums.end())
		    ++p, ++q;
		  else
		    break;
	    }
	  else
					     // estimated cell number is ok,
					     // stop correction steps
	    break;
	    

	if (p==partial_sums.end())
	  coarsening_threshold = 0;
	else
	  coarsening_threshold = *p - *(p-1);
      
	if (q==partial_sums.begin())
	  refinement_threshold = *q;
	else
	  refinement_threshold = *q - *(q-1);
	if (coarsening_threshold>=refinement_threshold)
	  coarsening_threshold = 0.999*refinement_threshold;

				       // now that we have re-adjusted
				       // thresholds: clear all refine and
				       // coarsening flags and do it all
				       // over again
	cell = tria->begin_active();
	endc  = tria->end();
	for (; cell!=endc; ++cell)
	  {
	    cell->clear_refine_flag ();
	    cell->clear_coarsen_flag ();
	  };

  
					 // flag cells finally
	tria->refine (refinement_data.criteria, refinement_threshold);
	tria->coarsen (refinement_data.criteria, coarsening_threshold);
      };


  
				   // if step number is greater than
				   // one: adapt this and the previous
				   // grid to each other. Don't do so
				   // for the initial grid because
				   // it is always taken to be the first
				   // grid and needs therefore no
				   // treatment of its own.
  if ((timestep_no >= 1) && (refinement_flags.adapt_grids))
    {
      Triangulation<dim> *previous_tria
	= dynamic_cast<const TimeStepBase_Tria<dim>*>(previous_timestep)->tria;
	

				       // if we used the dual estimator, we
				       // computed the error information on
				       // a time slab, rather than on a level
				       // of its own. we then mirror the
				       // refinement flags we determined for
				       // the present level to the previous
				       // one
				       //
				       // do this mirroring only, if cell number
				       // adjustment is on, since otherwise
				       // strange things may happen
      if (refinement_flags.mirror_flags_to_previous_grid)
	{
	  adapt_grids (*previous_tria, *tria);
	  
	  Triangulation<dim>::cell_iterator old_cell, new_cell, endc;
	  old_cell = previous_tria->begin(0);
	  new_cell = tria->begin(0);
	  endc     = tria->end(0);
	  for (; new_cell!=endc; ++new_cell, ++old_cell)
	    mirror_refinement_flags (new_cell, old_cell);
	};
      
      tria->prepare_coarsening ();
      tria->prepare_refinement ();
      previous_tria->prepare_coarsening ();
      previous_tria->prepare_refinement ();
      
				       // adapt present and previous grids
				       // to each other: flag additional
				       // cells to avoid the previous grid
				       // to have cells refined twice more
				       // than the present one and vica versa.
      adapt_grids (*tria, *tria);
      tria->prepare_coarsening ();
      tria->prepare_refinement ();
      previous_tria->prepare_coarsening ();
      previous_tria->prepare_refinement ();
    };
};






template <int dim>
TimeStepBase_Tria<dim>::Flags::Flags () {
  Assert (false, ExcInternalError());
};



template <int dim>
TimeStepBase_Tria<dim>::Flags::Flags (const bool delete_and_rebuild_tria,
				      const unsigned int wakeup_level_to_build_grid,
				      const unsigned int sleep_level_to_delete_grid):
		delete_and_rebuild_tria (delete_and_rebuild_tria),
		wakeup_level_to_build_grid (wakeup_level_to_build_grid),
		sleep_level_to_delete_grid (sleep_level_to_delete_grid)
{
//   Assert (!delete_and_rebuild_tria || (wakeup_level_to_build_grid>=1),
// 	  ExcInvalidParameter(wakeup_level_to_build_grid));
//   Assert (!delete_and_rebuild_tria || (sleep_level_to_delete_grid>=1),
// 	  ExcInvalidParameter(sleep_level_to_delete_grid));
};



template <int dim>
TimeStepBase_Tria<dim>::RefinementFlags::
RefinementFlags (const unsigned int max_refinement_level,
		 const double cell_number_corridor_top,
		 const double cell_number_corridor_bottom,
		 const unsigned int cell_number_correction_steps) :
		max_refinement_level(max_refinement_level),
		cell_number_corridor_top(cell_number_corridor_top),
		cell_number_corridor_bottom(cell_number_corridor_bottom),
		cell_number_correction_steps(cell_number_correction_steps)
{
  Assert (cell_number_corridor_top>=0, ExcInvalidValue (cell_number_corridor_top));
  Assert (cell_number_corridor_bottom>=0, ExcInvalidValue (cell_number_corridor_bottom));
  Assert (cell_number_corridor_bottom<=1, ExcInvalidValue (cell_number_corridor_bottom));
};



template <int dim>
TimeStepBase_Tria<dim>::RefinementData::
RefinementData (const Vector<float> &criteria,
		const double         _refinement_threshold,
		const double         _coarsening_threshold) :
		criteria (criteria),
		refinement_threshold(_refinement_threshold),
				   // in some rare cases it may happen that
				   // both thresholds are the same (e.g. if
				   // there are many cells with the same
				   // error indicator). That would mean that
				   // all cells will be flagged for
				   // refinement or coarsening, but some will
				   // be flagged for both, namely those for
				   // which the indicator equals the
				   // thresholds. This is forbidden, however.
				   //
				   // In some rare cases with very few cells
				   // we also could get integer round off
				   // errors and get problems with
				   // the top and bottom fractions.
				   //
				   // In these case we arbitrarily reduce the
				   // bottom threshold by one permille below
				   // the top threshold
		coarsening_threshold((_coarsening_threshold == _refinement_threshold ?
				      _coarsening_threshold :
				      0.999*_coarsening_threshold))
{
  Assert (*min_element(criteria.begin(), criteria.end()) >= 0,
	  ExcInvalidValue(*min_element(criteria.begin(), criteria.end())));
  
  Assert (refinement_threshold >= 0, ExcInvalidValue(refinement_threshold));
  Assert (coarsening_threshold >= 0, ExcInvalidValue(coarsening_threshold));
  Assert (coarsening_threshold < refinement_threshold,
	  ExcInvalidValue (coarsening_threshold));
};





// explicit instantiations
template class TimeStepBase_Tria<deal_II_dimension>;


