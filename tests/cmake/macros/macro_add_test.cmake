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
# A Macro to set up tests for the testsuite
#
# The following variables must be set:
#
# TEST_DIFF
#   - specifying the executable and command line of the diff command to use
#
# TEST_TIME_LIMIT
#   - specifying the maximal wall clock time in seconds a test is allowed
#     to run
#
# TEST_DIR
#   - pointing to the ./tests folder where normalize.pl and run_test.cmake are
#     located under cmake/scripts
#
#
# Usage:
#     DEAL_II_ADD_TEST(category test_name comparison_file n_cpu [configurations])
#
# This macro assumes that a source file
#
#   ./tests/category/<test_name>.cc
#
# as well as the comparison file
#
#   ./tests/category/<comparison_file>
#
# is available in the testsuite.
#
# If <n_cpu> is equal to 0, the plain, generated executable will be run. If
# <n_cpu> is a number larger than 0, the mpirun loader will be used to
# launch the executable
#
# [configurations] is a list of configurations against this test should be
# run. Possible values are an empty list, DEBUG, RELEASE or
# "DEBUG;RELEASE".
#
# The output of <test_name>.cc is compared against the file
# <comparison_file>.
#

MACRO(DEAL_II_ADD_TEST _category _test_name _comparison_file _n_cpu)

  IF(NOT DEAL_II_PROJECT_CONFIG_INCLUDED)
    MESSAGE(FATAL_ERROR
      "\nDEAL_II_ADD_TEST can only be called in external test subprojects after "
      "the inclusion of deal.IIConfig.cmake. It is not intended for "
      "internal use.\n\n"
      )
  ENDIF()

  FOREACH(_build ${DEAL_II_BUILD_TYPES})

    ITEM_MATCHES(_match "${_build}" ${ARGN})
    IF(_match OR "${ARGN}" STREQUAL "")

      #
      # Setup a bunch of variables describing the test:
      #
      STRING(TOLOWER ${_build} _build_lowercase)
      SET(_target ${_test_name}.${_build_lowercase}) # target name

      # If _n_cpu is equal to "0", a normal, sequental test will be run,
      # otherwise run the test with mpirun:
      IF("${_n_cpu}" STREQUAL "0")

        SET(_diff_target ${_target}.diff) # diff target name
        SET(_test_full ${_category}/${_test_name}.${_build_lowercase}) # full test name
        SET(_test_directory ${CMAKE_CURRENT_BINARY_DIR}/${_target}) # directory to run the test in
        SET(_run_command ${_target}) # the command to issue

      ELSE()

        SET(_diff_target ${_test_name}.mpirun${_n_cpu}.${_build_lowercase}.diff) # diff target name
        SET(_test_full ${_category}/${_test_name}.mpirun=${_n_cpu}.${_build_lowercase}) # full test name
        SET(_test_directory ${CMAKE_CURRENT_BINARY_DIR}/${_target}/mpirun=${_n_cpu}) # directory to run the test in
        SET(_run_command mpirun -np ${_n_cpu} ${CMAKE_CURRENT_BINARY_DIR}/${_target}/${_target}) # the command to issue

      ENDIF()

      FILE(MAKE_DIRECTORY ${_test_directory})

      #
      # Add an executable for the current test and set up compile
      # definitions and the full link interface:
      #
      IF(NOT TARGET ${_target})
        # only add the target once

        ADD_EXECUTABLE(${_target} EXCLUDE_FROM_ALL ${_test_name}.cc)

        SET_TARGET_PROPERTIES(${_target} PROPERTIES
          LINK_FLAGS "${DEAL_II_LINKER_FLAGS} ${DEAL_II_LINKER_FLAGS_${_build}}"
          COMPILE_DEFINITIONS "${DEAL_II_USER_DEFINITIONS};${DEAL_II_USER_DEFINITIONS_${_build}}"
          COMPILE_FLAGS "${DEAL_II_CXX_FLAGS} ${DEAL_II_CXX_FLAGS_${_build}}"
          LINKER_LANGUAGE "CXX"
          RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${_target}"
          )
        SET_PROPERTY(TARGET ${_target} APPEND PROPERTY
          INCLUDE_DIRECTORIES "${DEAL_II_INCLUDE_DIRS}"
          )
        SET_PROPERTY(TARGET ${_target} APPEND PROPERTY
          COMPILE_DEFINITIONS
            SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
          )
        TARGET_LINK_LIBRARIES(${_target} ${DEAL_II_TARGET_${_build}})
      ENDIF()

      #
      # Add a top level target to run and compare the test:
      #

      ADD_CUSTOM_COMMAND(OUTPUT ${_test_directory}/output
        COMMAND
          ${_run_command}
          || (mv ${_test_directory}/output
                 ${_test_directory}/failing_output
              && echo "${_test_full}: BUILD successful."
              && echo "${_test_full}: RUN failed. Output:"
              && cat ${_test_directory}/failing_output
              && exit 1)
        COMMAND
          ${PERL_EXECUTABLE} -pi ${TEST_DIR}/cmake/scripts/normalize.pl
                                 ${_test_directory}/output
        WORKING_DIRECTORY
          ${_test_directory}
        DEPENDS
          ${_target}
          ${TEST_DIR}/cmake/scripts/normalize.pl
        )
      ADD_CUSTOM_COMMAND(OUTPUT ${_test_directory}/diff
        COMMAND
          ${TEST_DIFF}
            ${_test_directory}/output
            ${_comparison_file}
            > ${_test_directory}/diff
          || (mv ${_test_directory}/diff
                 ${_test_directory}/failing_diff
              && echo "${_test_full}: BUILD successful."
              && echo "${_test_full}: RUN successful."
              && echo "${_test_full}: DIFF failed. Output:"
              && cat ${_test_directory}/failing_diff
              && exit 1)
        WORKING_DIRECTORY
          ${_test_directory}
        DEPENDS
          ${_test_directory}/output
          ${_comparison_file}
        )

      ADD_CUSTOM_TARGET(${_diff_target} DEPENDS ${_test_directory}/diff
        COMMAND
             echo "${_test_full}: BUILD successful."
          && echo "${_test_full}: RUN successful."
          && echo "${_test_full}: DIFF successful."
          && echo "${_test_full}: PASSED."
        )

      #
      # And finally add the test:
      #

      ADD_TEST(NAME ${_test_full}
        COMMAND ${CMAKE_COMMAND}
          -DTRGT=${_diff_target}
          -DTEST=${_test_full}
          -DDEAL_II_BINARY_DIR=${CMAKE_BINARY_DIR}
          -P ${TEST_DIR}/cmake/scripts/run_test.cmake
        WORKING_DIRECTORY ${_test_directory}
        )
      SET_TESTS_PROPERTIES(${_test_full} PROPERTIES
        LABEL "${_category}"
        TIMEOUT ${TEST_TIME_LIMIT}
        )

      #
      # We have to be careful not to run different mpirun settings for the
      # same executable in parallel because this triggers a race condition
      # when compiling the not yet existent executable that is shared
      # between the different tests.
      #
      # Luckily CMake has a mechanism to force a test to be run when
      # another has finished (and both are scheduled):
      #
      IF(NOT "${_n_cpu}" STREQUAL "0")
        IF(DEFINED TEST_DEPENDENCIES_${_target})
          SET_TESTS_PROPERTIES(${_test_full} PROPERTIES
            DEPENDS ${TEST_DEPENDENCIES_${_target}}
            )
        ENDIF()
        SET(TEST_DEPENDENCIES_${_target} ${_test_full})
      ENDIF()

    ENDIF()
  ENDFOREACH()
ENDMACRO()
