// $Id$

// JS.Wird das File ueberhaupt gebraucht ?

#include <base/function.h>

class BoundaryFct
  : public Function<2>
{
 public:
  virtual double operator()(const Point<2> &p) const;
};

