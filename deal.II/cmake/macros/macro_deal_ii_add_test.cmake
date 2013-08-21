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
# A Macro to set up tests for thes testsuite
#
# This macro assumes that a test
#
#   ./tests/category/test_name.cc
#   ./tests/category/test_name.*.output
#
# is available in the testsuite.
#
# [configurations] is a list of configurations against this test should be
# run. Possible values are an empty list, DEBUG, RELEASE or
# "DEBUG;RELEASE".
#
# The following variables must be set:
#
# DEAL_II_TEST_DIFF
#   - specifying the executable and command line of the diff command to use
#
# DEAL_II_TEST_TIME_LIMIT
#   - specifying the maximal wall clock time in seconds a test is allowed
#     to run
#
# TODO: LABEL and MEASUREMENT
#
# Usage:
#     DEAL_II_ADD_TEST(category test_name [configurations])
#

MACRO(DEAL_II_ADD_TEST _category _test_name)

  FOREACH(_build ${DEAL_II_BUILD_TYPES})

    ITEM_MATCHES(_match "${_build}" ${ARGN})
    IF(_match OR "${ARGN}" STREQUAL "")

      STRING(TOLOWER ${_build} _build_lowercase)
      SET(_test ${_test_name}.${_build_lowercase})

      ADD_EXECUTABLE(${_test} EXCLUDE_FROM_ALL ${_test_name}.cc)

      SET_TARGET_PROPERTIES(${_test} PROPERTIES
        LINK_FLAGS "${DEAL_II_LINKER_FLAGS} ${DEAL_II_LINKER_FLAGS_${_build}}"
        COMPILE_DEFINITIONS "${DEAL_II_DEFINITIONS};${DEAL_II_DEFINITIONS_${_build}}"
        COMPILE_FLAGS "${DEAL_II_CXX_FLAGS_${_build}}"
        LINKER_LANGUAGE "CXX"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${_test}"
        )
      SET_PROPERTY(TARGET ${_test} APPEND PROPERTY
        INCLUDE_DIRECTORIES
          "${CMAKE_BINARY_DIR}/include"
          "${CMAKE_SOURCE_DIR}/include"
          "${CMAKE_SOURCE_DIR}/include/deal.II/"
        )
      SET_PROPERTY(TARGET ${_test} APPEND PROPERTY
        COMPILE_DEFINITIONS
          SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        )
      TARGET_LINK_LIBRARIES(${_test_name}.${_build_lowercase}
        ${DEAL_II_BASE_NAME}${DEAL_II_${_build}_SUFFIX}
        )


      ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_test}/output
        COMMAND ${_test}
        # TODO: Refactor:
        COMMAND ${PERL_EXECUTABLE} -pi
            ${CMAKE_SOURCE_DIR}/cmake/scripts/normalize.pl
            ${CMAKE_CURRENT_BINARY_DIR}/${_test}/output
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${_test}
        DEPENDS ${_test} ${CMAKE_SOURCE_DIR}/cmake/scripts/normalize.pl
        )
      ADD_CUSTOM_TARGET(${_test}.run
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${_test}/output
        )


      SET(_comparison ${CMAKE_CURRENT_SOURCE_DIR}/${_test_name})
      IF(EXISTS ${_comparison}.${_build_lowercase}.output)
        SET(_comparison ${_comparison}.${_build_lowercase}.output)
      ELSE()
        SET(_comparison ${_comparison}.output)
      ENDIF()

      ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_test}/diff
        COMMAND ${DEAL_II_TEST_DIFF}
          ${CMAKE_CURRENT_BINARY_DIR}/${_test}/output
          ${_comparison}
          | tee ${CMAKE_CURRENT_BINARY_DIR}/${_test}/diff
        DEPENDS
          ${CMAKE_CURRENT_BINARY_DIR}/${_test}/output
          ${_comparison}
        )
      ADD_CUSTOM_TARGET(${_test}.diff
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${_test}/diff
        )


      ADD_TEST(NAME ${_test}
        COMMAND ${CMAKE_COMMAND}
          -DTEST=${_test}
          -DDEAL_II_BINARY_DIR=${CMAKE_BINARY_DIR}
          -P ${CMAKE_SOURCE_DIR}/cmake/scripts/run_test.cmake
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${_test}
        )
      SET_TESTS_PROPERTIES(${_test} PROPERTIES
        LABEL "${_category}"
        TIMEOUT ${DEAL_II_TEST_TIME_LIMIT}
        )

    ENDIF()

  ENDFOREACH()

ENDMACRO()

