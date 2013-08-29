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

#
# A Macro to pick up all tests in a test subdirectory
#
# If DEAL_II_PICKUP_REGEX is set, only tests matching the regex will be
# processed.
#
# Usage:
#     DEAL_II_PICKUP_TESTS()
#

MACRO(DEAL_II_PICKUP_TESTS)
  GET_FILENAME_COMPONENT(_category ${CMAKE_CURRENT_SOURCE_DIR} NAME)

  FILE(GLOB _tests "*.output")

  FOREACH(_test ${_tests})

    IF( "${DEAL_II_PICKUP_REGEX}" STREQUAL "" OR
        _test MATCHES "${DEAL_II_PICKUP_REGEX}" )
      GET_FILENAME_COMPONENT(_test ${_test} NAME)

      IF(_test MATCHES debug)
        SET(_configuration DEBUG)
      ELSEIF(_test MATCHES release)
        SET(_configuration RELEASE)
      ELSE()
        SET(_configuration)
      ENDIF()

      STRING(REGEX REPLACE "\\..*" "." _test ${_test})
      DEAL_II_ADD_TEST(${_category} ${_test} ${_configuration})
    ENDIF()

  ENDFOREACH()

