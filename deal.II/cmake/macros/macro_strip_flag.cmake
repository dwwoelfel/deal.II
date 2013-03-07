#####
##
## Copyright (C) 2012 by the deal.II authors
##
## This file is part of the deal.II library.
##
## <TODO: Full License information>
## This file is dual licensed under QPL 1.0 and LGPL 2.1 or any later
## version of the LGPL license.
##
## Author: Matthias Maier <matthias.maier@iwr.uni-heidelberg.de>
##
#####

#
# Remove all occurences of "${flag}" in the string variable.
#
# Usage:
#     STRIP_FLAG(variable flag)
#

MACRO(STRIP_FLAG _variable _flag)
  SET(${_variable} " ${${_variable}} ")
  STRING(REPLACE " ${_flag} " "" "${_variable}" ${${_variable}})
  STRING(STRIP "${${_variable}}" ${_variable})
ENDMACRO()

