/*----------------------------   convergence_table.cc     ---------------------------*/
/*      $Id$                 */


#include <base/convergence_table.h>
#include <math.h>


ConvergenceTable::ConvergenceTable()  
{}


void ConvergenceTable::evaluate_convergence_rates(const string &data_column_key,
						  const string &reference_column_key,
						  const RateMode rate_mode)
{
  Assert(columns.count(data_column_key),
	 ExcColumnNotExistent(data_column_key));
  Assert(columns.count(reference_column_key),
	 ExcColumnNotExistent(reference_column_key));
 
  if (rate_mode==none)
    return;
 
  vector<TableEntryBase *> &entries=columns[data_column_key].entries;
  string rate_key=data_column_key;

  const unsigned int n=entries.size();
  
  vector<double> values(n);
  for (unsigned int i=0; i<n; ++i)
    {
      if (dynamic_cast<TableEntry<double>*>(entries[i]) != 0)
	values[i]=dynamic_cast<TableEntry<double>*>(entries[i])->value();
      else if (dynamic_cast<TableEntry<float>*>(entries[i]) != 0)
	values[i]=dynamic_cast<TableEntry<float>*>(entries[i])->value();
      else if (dynamic_cast<TableEntry<int>*>(entries[i]) != 0)
	values[i]=dynamic_cast<TableEntry<int>*>(entries[i])->value();
      else if (dynamic_cast<TableEntry<unsigned int>*>(entries[i]) != 0)
	values[i]=dynamic_cast<TableEntry<unsigned int>*>(entries[i])->value();
      else
	Assert(false, ExcWrongValueType());
    }
  
  switch (rate_mode)
    {
      case none:
	    break;
      case reduction_rate:
      case reduction_rate_log2:
      default:
	    Assert(false, ExcNotImplemented());  
    }

  Assert(columns.count(rate_key), ExcInternalError());  
  columns[rate_key].flag=1;
  set_precision(rate_key, 2);

  string superkey=data_column_key;
  if (!supercolumns.count(superkey))
    {
      add_column_to_supercolumn(data_column_key, superkey);
      set_tex_supercaption(superkey, columns[data_column_key].tex_caption);
    }

  add_column_to_supercolumn(rate_key, superkey);
}


void ConvergenceTable::evaluate_convergence_rates(const string &data_column_key,
						  const RateMode rate_mode)
{
  Assert(columns.count(data_column_key), ExcColumnNotExistent(data_column_key));
  
  vector<TableEntryBase *> &entries=columns[data_column_key].entries;
  string rate_key=data_column_key+"...";

  const unsigned int n=entries.size();
  
  vector<double> values(n);
  for (unsigned int i=0; i<n; ++i)
    {
      if (dynamic_cast<TableEntry<double>*>(entries[i]) != 0)
	values[i]=dynamic_cast<TableEntry<double>*>(entries[i])->value();
      else if (dynamic_cast<TableEntry<float>*>(entries[i]) != 0)
	values[i]=dynamic_cast<TableEntry<float>*>(entries[i])->value();
      else if (dynamic_cast<TableEntry<int>*>(entries[i]) != 0)
	values[i]=dynamic_cast<TableEntry<int>*>(entries[i])->value();
      else if (dynamic_cast<TableEntry<unsigned int>*>(entries[i]) != 0)
	values[i]=dynamic_cast<TableEntry<unsigned int>*>(entries[i])->value();
      else
	Assert(false, ExcWrongValueType());
    }
  
  switch (rate_mode)
    {
      case none:
	    break;
      case reduction_rate:
	    rate_key+="red.rate";
	    Assert(columns.count(rate_key)==0, ExcRateColumnAlreadyExists(rate_key));
					     // no value available for the
					     // first row
	    add_value(rate_key, string("-"));
	    for (unsigned int i=1; i<n; ++i)
	      add_value(rate_key, values[i-1]/values[i]);
	    break;
      case reduction_rate_log2:
	    rate_key+="red.rate.log2";
	    Assert(columns.count(rate_key)==0, ExcRateColumnAlreadyExists(rate_key));
					     // no value availble for the
					     // first row
	    add_value(rate_key, string("-"));
	    for (unsigned int i=1; i<n; ++i)
	      add_value(rate_key, log(values[i-1]/values[i])/log(2));
	    break;
      default:
	    ExcNotImplemented();  
    }

  Assert(columns.count(rate_key), ExcInternalError());  
  columns[rate_key].flag=1;
  set_precision(rate_key, 2);
  
				   // set the superkey equal to the key
  string superkey=data_column_key;
				   // and set the tex caption of the supercolumn
				   // to the tex caption of the data_column.
  if (!supercolumns.count(superkey))
    {
      add_column_to_supercolumn(data_column_key, superkey);
      set_tex_supercaption(superkey, columns[data_column_key].tex_caption);
    }

  add_column_to_supercolumn(rate_key, superkey);
}

void ConvergenceTable::omit_column_from_convergence_rate_evaluation(const string &key)
{
  Assert(columns.count(key), ExcColumnNotExistent(key));
  
  const map<string, Column>::iterator col_iter=columns.find(key);
  col_iter->second.flag=1;
}



void ConvergenceTable::evaluate_all_convergence_rates(const string &reference_column_key,
						      const RateMode rate_mode)
{
  for (map<string, Column>::const_iterator col_iter=columns.begin();
       col_iter!=columns.end(); ++col_iter)
    if (!col_iter->second.flag)
      evaluate_convergence_rates(col_iter->first, reference_column_key, rate_mode);
}


void ConvergenceTable::evaluate_all_convergence_rates(const RateMode rate_mode)
{
  for (map<string, Column>::const_iterator col_iter=columns.begin();
       col_iter!=columns.end(); ++col_iter)
    if (!col_iter->second.flag)
      evaluate_convergence_rates(col_iter->first, rate_mode);
}


/*----------------------------   convergence_table.cc     ---------------------------*/
