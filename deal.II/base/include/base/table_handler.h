//----------------------------  table_handler.h  ---------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 1998, 1999, 2000, 2001, 2002 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  table_handler.h  ---------------------------
#ifndef __deal2__table_handler_h
#define __deal2__table_handler_h


#include <base/config.h>
#include <base/exceptions.h>

#include <map>
#include <vector>
#include <string>
#include <fstream>

// we only need output streams, but older compilers did not provide
// them in a separate include file
#ifdef HAVE_STD_OSTREAM_HEADER
#  include <ostream>
#else
#  include <iostream>
#endif



/**
 * Abstract base class for the @p{TableEntry} class. See there.
 * This class is not to be used by the user.
 *
 * @author Ralf Hartmann, 1999
 */
class TableEntryBase 
{
  public:
                                     /**
                                      * Constructor.
                                      */
    TableEntryBase();
    
                                     /**
                                      * Virtual destructor.
                                      */    
    virtual ~TableEntryBase();
    
                                     /**
                                      * Write the table entry as text.
                                      */
    virtual void write_text (std::ostream &) const =0;
    
                                     /**
                                      * Write the table entry in tex format.
                                      */
    virtual void write_tex (std::ostream &) const =0;
};


/**
 * A @p{TableEntry} stores the value of a table entry.
 * The value type of this table entry is arbitrary. For
 * a @p{TableEntry<typename value_type>} with a non-common value
 * type you may want to specialize the output functions in order
 * to get nicer output. This class is not to be used by the user.
 *
 * For more detail see the @p{TableHandler} class.
 *
 * @author Ralf Hartmann, 1999
 */
template <typename value_type>
class TableEntry : public TableEntryBase 
{
  public:
                                     /**
                                      * Constructor.
                                      */
    TableEntry(const value_type value);

                                     /**
                                      * Destructor.
                                      */
    virtual ~TableEntry();

                                     /**
                                      * Returns the value of this
                                      * table entry.
                                      */
    value_type value() const;

                                     /**
                                      * Write the table entry as text.
                                      */
    virtual void write_text (std::ostream &out) const;

                                     /**
                                      * Write the table entry in tex
                                      * format.
                                      */
    virtual void write_tex (std::ostream &out) const;

  private:
                                     /**
                                      * Stored value.
                                      */
    const value_type val;
};


/**
 * The @p{TableHandler} stores @p{TableEntries} of arbitrary value
 * type and writes the table as text or in tex format to an output
 * stream. The value type actually may vary from column to column and
 * from row to row.
 *
 * @sect3{Usage}
 *
 * The most important function is the templatized function
 * @p{add_value(const std::string &key, const value_type value)}, that
 * adds a column with the name @p{key} to the table if this column
 * does not yet exist and adds the value of @p{value_type}
 * (e.g. @p{unsigned int}, @p{double}, @p{std::string}, ...) to this
 * column.  After the table is complete there are different
 * possibilities of output, e.g.  into a tex file with
 * @p{write_tex(std::ofstream &file)} or as text with @p{write_text
 * (std::ostream &out)}.
 *
 * Two (or more) columns may be merged into a "supercolumn" by twice
 * (or multiple) calling @p{add_column_to_supercolumn(...)}, see
 * there. Additionally there is a function to set for each column the
 * precision of the output of numbers, and there are several functions
 * to prescribe the format and the captions the columns are written
 * with in tex mode.
 *
 * @sect3{Example}
 * 
 * This is a simple example demonstrating the usage of this class. The
 * first column includes the numbers @p{i=1..n}, the second
 * $1^2$...$n^2$, the third $sqrt(1)...sqrt(n)$, where the second and
 * third columns are merged into one supercolumn with the superkey
 * @p{squares and roots}. Additionally the first column is aligned to
 * the right (the default was @p{centered}) and the precision of the
 * square roots are set to be 6 (instead of 4 as default).
 *
 * @begin{verbatim}
 * TableHandler table();
 *
 * for (unsigned int i=1; i<=n; ++i)
 *   {
 *     table.add_value("numbers", i);
 *     table.add_value("squares", i*i);
 *     table.add_value("square roots", sqrt(i));
 *   }
 *                                  // merge the second and third column
 * table.add_column_to_supercolumn("squares", "squares and roots");
 * table.add_column_to_supercolumn("square roots", "squares and roots");
 *
 *                                  // additional settings
 * table.set_tex_format("numbers", "r");
 * table.set_precision("square roots", 6);
 *
 *                                  // output
 * std::ofstream out_file("number_table.tex");
 * table.write_tex(out_file);
 * out_file.close();
 * @end{verbatim}
 *
 * @author Ralf Hartmann, 1999
 */
class TableHandler
{
  public:

                                     /**
                                      * Constructor.
                                      */
    TableHandler();
    
                                     /**
                                      * Adds a column (if not yet
                                      * existent) with the key @p{key}
                                      * and adds the value of
                                      * @p{value_type} to the column.
                                      */
    template <typename value_type>
    void add_value (const std::string &key,
                    const value_type   value);
    
                                     /**
                                      * Creates a supercolumn (if not
                                      * yet existent) and includes
                                      * column to it.  The keys of the
                                      * column and the supercolumn are
                                      * @p{key} and @p{superkey},
                                      * respectively.  To merge two
                                      * columns @p{c1} and @p{c2} to a
                                      * supercolumn @p{sc} hence call
                                      * @p{add_column_to_supercolumn(c1,sc)}
                                      * and
                                      * @p{add_column_to_supercolumn(c2,sc)}.
                                      *
                                      * Concerning the order of the
                                      * columns, the supercolumn
                                      * replaces the first column that
                                      * is added to the supercolumn.
                                      * Within the supercolumn the
                                      * order of output follows the
                                      * order the columns are added to
                                      * the supercolumn.
                                      */
    void add_column_to_supercolumn (const std::string &key,
                                    const std::string &superkey);

                                     /**
                                      * Change the order of columns and
                                      * supercolumns in the table.
                                      *
                                      * @p{new_order} includes the
                                      * keys and superkeys of the
                                      * columns and supercolumns in
                                      * the order the user like to.
                                      * If a superkey is included the
                                      * keys of the subcolumns need
                                      * not to be additionally
                                      * mentioned in this vector.  The
                                      * order of subcolumns within a
                                      * supercolumn is not changeable
                                      * and keeps the order in which
                                      * the columns are added to the
                                      * supercolumn.
                                      *
                                      * This function may also be used
                                      * to break big tables with to
                                      * many columns into smaller
                                      * ones. Call this function with
                                      * the first e.g. five columns
                                      * and then @p{write_*}.
                                      * Afterwards call this function
                                      * with the next e.g. five
                                      * columns and again @p{write_*},
                                      * and so on.
                                      */
    void set_column_order (const std::vector<std::string> &new_order);
    
                                     /**
                                      * Sets the @p{precision}
                                      * e.g. double or float variables
                                      * are written
                                      * with. @p{precision} is the
                                      * same as in calling
                                      * @p{out<<setprecision(precision)}.
                                      */
    void set_precision (const std::string &key,
                        const unsigned int precision);

                                     /**
                                      * Sets the
                                      * @p{scientific_flag}. True
                                      * means scientific, false means
                                      * fixed point notation.
                                      */
    void set_scientific (const std::string &key,
                         const bool         scientific);

                                     /**
                                      * Sets the caption of the column
                                      * @p{key} for tex output. You
                                      * may want to chose this
                                      * different from @p{key}, if it
                                      * contains formulas or similar
                                      * constructs.
                                      */
    void set_tex_caption (const std::string &key,
                          const std::string &tex_caption);

                                     /**
                                      * Sets the caption the the
                                      * supercolumn @p{superkey} for
                                      * tex output. You may want to
                                      * chose this different from
                                      * @p{superkey}, if it contains
                                      * formulas or similar
                                      * constructs.
                                      */
    void set_tex_supercaption (const std::string &superkey,
                               const std::string &tex_supercaption);

                                     /**
                                      * Sets the tex output format of
                                      * a column, e.g. @p{c}, @p{r},
                                      * @p{l}, or @p{p{3cm}}. The
                                      * default is @p{c}. Also if this
                                      * function is not called for a
                                      * column, the default is preset
                                      * to be @p{c}.
                                      */
    void set_tex_format (const std::string &key,
                         const std::string &format="c");

                                     /**
                                      * Write table as formatted text,
                                      * e.g.  to the standard output.
                                      */
    void write_text (std::ostream &out) const;

                                     /**
                                      * Write table as a tex file.
                                      */
    void write_tex (std::ofstream &file) const;
   
                                     /**
                                      * Exception
                                      */
    DeclException1 (ExcColumnNotExistent,
                    std::string,
                    << "Column <" << arg1 << "> does not exist.");

                                     /**
                                      * Exception
                                      */
    DeclException1 (ExcSuperColumnNotExistent,
                    std::string,
                    << "Supercolumn <" << arg1 << "> does not exist.");

                                     /**
                                      * Exception
                                      */
    DeclException1 (ExcColumnOrSuperColumnNotExistent,
                    std::string,
                    << "Column or supercolumn <" << arg1 << "> does not exist.");
    
                                     /**
                                      * Exception
                                      */
    DeclException4 (ExcWrongNumberOfDataEntries,
                    std::string, int, std::string, int,
                    << "Column <" << arg1 << "> has got " << arg2
                    << " rows, but Column <" << arg3 << "> has got " << arg4 << ".");

                                     /**
                                      * Exception
                                      */
    DeclException1 (ExcUndefinedTexFormat,
                    std::string,
                    << "<" << arg1 << "> is not a tex column format. Use l,c,r.");

  protected:

                                     /**
                                      * Structure encapsulating all the data
                                      * that is needed to describe one column
                                      * of a table.
                                      */
    struct Column
    {
                                         /**
                                          * Constructor needed by STL maps.
                                          */
        Column();

                                         /**
                                          * Constructor.
                                          */
        Column(const std::string &tex_caption);
	
                                         /**
                                          * Destructor.
                                          */
        ~Column();
	
                                         /**
                                          * List of entries within
                                          * this column.  They may
                                          * actually be of very
                                          * different type, since we
                                          * use the templated
                                          * @p{TableEntry<T>} class
                                          * for actual values, which
                                          * is only a wrapper for
                                          * @p{T}, but is derived from
                                          * @p{TableEntryBase}.
                                          */
        std::vector<TableEntryBase *> entries;
	
                                         /**
                                          * The caption of the column
                                          * in tex output.  By
                                          * default, this is the key
                                          * string that is given to
                                          * the @p{TableHandler} by
                                          * @p{TableHandler::add_value(...)}. This
                                          * may be changed by calling
                                          * @p{TableHandler::set_tex_caption(...)}.
                                          */
        std::string tex_caption;

                                         /**
                                          * The column format in tex
                                          * output.  By default, this
                                          * is @p{"c"}, meaning
                                          * `centered'. This may be
                                          * changed by calling
                                          * @p{TableHandler::set_tex_format(...)}
                                          * with @p{"c", "r", "l"} for
                                          * centered, right or left.
                                          */
	
        std::string tex_format;

                                         /**
                                          * Double or float entries
                                          * are written with this
                                          * precision (set by the
                                          * user).  The default is 4.
                                          */
        unsigned int precision;

                                         /**
                                          * @p{scientific}=false means
                                          * fixed point notation.
                                          */
        bool scientific;

                                         /**
                                          * Flag that may be used by
                                          * derived classes for
                                          * arbitrary purposes.
                                          */
        unsigned int flag;
    };

                                     /**
                                      * Help function that gives a
                                      * vector of the keys of all
                                      * columns that are mentioned in
                                      * @p{column_order}, where each
                                      * supercolumn key is replaced by
                                      * its subcolumn keys.
                                      *
                                      * This function implicitly
                                      * checks the consistency of the
                                      * data. The result is returned
                                      * in @p{sel_columns}.
                                      */
    void get_selected_columns (std::vector<std::string> &sel_columns) const;
    
                                     /**
                                      * Builtin function, that gives
                                      * the number of rows in the
                                      * table and that checks if the
                                      * number of rows is equal in
                                      * every column. This function is
                                      * e.g. called before writing
                                      * output.
                                      */
    unsigned int n_rows() const;

                                     /**
                                      * Stores the column and
                                      * supercolumn keys in the order
                                      * desired by the user.  By
                                      * default this is the order of
                                      * adding the columns. This order
                                      * may be changed by
                                      * @p{set_column_order(...)}.
                                      */
    std::vector<std::string> column_order;

                                     /**
                                      * Maps the column keys to the
                                      * columns (not supercolumns).
                                      */
    std::map<std::string,Column> columns;

                                     /**
                                      * Maps each supercolumn key to
                                      * the the keys of its subcolumns
                                      * in the right order.  It is
                                      * allowed that a supercolumn has
                                      * got the same key as a column.
                                      *
                                      * Note that we do not use a @p{multimap}
                                      * here since the order of column
                                      * keys for each supercolumn key is
                                      * relevant.
                                      */
    std::map<std::string, std::vector<std::string> > supercolumns;

                                     /**
                                      * Maps the supercolumn keys to
                                      * the captions of the
                                      * supercolumns that are used in
                                      * tex output.
                                      *
                                      * By default these are just the
                                      * supercolumn keys but they may
                                      * be changed by
                                      * @p{set_tex_supercaptions(...)}.
                                      */
    std::map<std::string, std::string> tex_supercaptions;
};


#endif
