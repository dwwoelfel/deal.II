## ---------------------------------------------------------------------
## $Id$
##
## Copyright (C) 2012 - 2014 by the deal.II authors
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
# Configuration for the boost library:
#

#
# This configure script has to be included after configure_threads.
# We need some of the variables defined in SETUP_THREADING for
# the setup of the bundled boost library (if used)
#
IF(NOT FEATURE_THREADS_PROCESSED)
  MESSAGE(FATAL_ERROR "\n"
    "Internal build system error:\n"
    "configure_boost.cmake included before configure_1_threads.cmake\n\n"
    )
ENDIF()


SET(DEAL_II_WITH_BOOST ON # Always true. We need it :-]
  CACHE BOOL "Build deal.II with support for boost." FORCE
  )


MACRO(FEATURE_BOOST_FIND_EXTERNAL var)

  #
  # This mumbo jumbo is necessary because CMake won't let us test against
  # BOOST_DIR directly. WTF?!
  #
  IF(NOT DEFINED BOOST_DIR)
    SET(BOOST_DIR "$ENV{BOOST_DIR}")
  ELSE()
    SET_IF_EMPTY(BOOST_DIR "$ENV{BOOST_DIR}")
  ENDIF()

  IF(NOT "${BOOST_DIR}" STREQUAL "")
    SET(BOOST_ROOT "${BOOST_DIR}")
  ENDIF()

  IF(DEAL_II_WITH_THREADS)
    SET(_boost_components iostreams serialization system thread)
  ELSE()
    SET(_boost_components iostreams serialization system)
  ENDIF()

  #
  # Prefer static libs if BUILD_SHARED_LIBS=OFF:
  #
  IF(NOT BUILD_SHARED_LIBS)
    SET(Boost_USE_STATIC_LIBS TRUE)
  ENDIF()

  FIND_PACKAGE(Boost 1.44 COMPONENTS ${_boost_components})

  #
  # Fall back to dynamic libraries if no static libraries could be found:
  #
  IF(NOT Boost_FOUND AND Boost_USE_STATIC_LIBS)
    SET(Boost_USE_STATIC_LIBS FALSE)
    FIND_PACKAGE(Boost 1.44 COMPONENTS ${_boost_components})
  ENDIF()

  MARK_AS_ADVANCED(Boost_DIR)

  IF(Boost_FOUND)
    SET(BOOST_VERSION_MAJOR "${Boost_MAJOR_VERSION}")
    SET(BOOST_VERSION_MINOR "${Boost_MINOR_VERSION}")
    SET(BOOST_VERSION_SUBMINOR "${Boost_SUBMINOR_VERSION}")
    SET(BOOST_VERSION
      "${BOOST_VERSION_MAJOR}.${BOOST_VERSION_MINOR}.${BOOST_VERSION_SUBMINOR}"
      )

    #
    # Remove "pthread" from Boost_LIBRARIES. Threading, if necessary, is
    # already set up via configure_1_threads.cmake.
    #
    LIST(REMOVE_ITEM Boost_LIBRARIES "pthread")

    SET(BOOST_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
    SET(BOOST_LIBRARIES ${Boost_LIBRARIES})

    MARK_AS_ADVANCED(BOOST_DIR)

    SET(${var} TRUE)

  ELSE()

    SET(BOOST_DIR "" CACHE PATH "An optional hint to a boost directory")
  ENDIF()
ENDMACRO()


MACRO(FEATURE_BOOST_CONFIGURE_BUNDLED)
  SET(BOOST_BUNDLED_INCLUDE_DIRS ${BOOST_FOLDER}/include)
ENDMACRO()


CONFIGURE_FEATURE(BOOST)

#
# The user has to know the location of the boost headers as well:
#
SET(BOOST_USER_INCLUDE_DIRS ${BOOST_INCLUDE_DIRS})

#
# DEAL_II_WITH_BOOST is always required.
#
IF(NOT DEAL_II_WITH_BOOST)
  IF(DEAL_II_FEATURE_AUTODETECTION)
    FEATURE_ERROR_MESSAGE("BOOST")
  ELSE()
    MESSAGE(FATAL_ERROR "\n"
      "Unmet configuration requirements: "
      "DEAL_II_WITH_BOOST required, but set to OFF!.\n\n"
      )
  ENDIF()
ENDIF()
