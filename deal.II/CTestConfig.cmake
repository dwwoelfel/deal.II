## ---------------------------------------------------------------------
## $Id$
##
## Copyright (C) 2013 by the deal.II authors
##
## This file is part of the deal.II library.
##
## The deal.II library is free software; you can use it, redistribute
## it, and/or modify it under the terms of the GNU Lesser General
## Public License as published by the Free Software Foundation; either
## version 2.1 of the License, or (at your option) any later version.
## The full text of the license can be found in the file LICENSE at
## the top level of the deal.II distribution.
##
## ---------------------------------------------------------------------

SET(CTEST_PROJECT_NAME "deal.II")

SET(CTEST_NIGHTLY_START_TIME "1:00:00 UTC")

SET(CTEST_CUSTOM_COVERAGE_EXCLUDE
  "/bundled"
  "/CMakeFiles/CMakeTmp/"
  "/contrib"
  )

SET(CTEST_USE_LAUNCHERS 1)
