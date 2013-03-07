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
# Try to find the Trilinos library
#
# This module exports:
#
#   TRILINOS_DIR (cached)
#   TRILINOS_INCLUDE_DIRS
#   TRILINOS_LIBRARIES
#   TRILINOS_VERSION_MAJOR
#   TRILINOS_VERSION_MINOR
#   TRILINOS_VERSION_SUBMINOR
#   TRILINOS_WITH_MPI
#

INCLUDE(FindPackageHandleStandardArgs)

SET_IF_EMPTY(TRILINOS_DIR "$ENV{TRILINOS_DIR}")

#
# Include the trilinos package configuration:
#
FIND_PACKAGE(TRILINOS
  QUIET CONFIG
  NAMES Trilinos TRILINOS
  HINTS
    ${TRILINOS_DIR}
  PATH_SUFFIXES
    lib${LIB_SUFFIX}/cmake/Trilinos
    lib64/cmake/Trilinos
    lib/cmake/Trilinos
  NO_SYSTEM_ENVIRONMENT_PATH
  )

SET(TRILINOS_INCLUDE_DIRS ${Trilinos_INCLUDE_DIRS})

#
# We'd like to have the full library names but the Trilinos package only
# exports a list with short names...
# So we check again for every lib and store the full path:
#
FOREACH(_library ${Trilinos_LIBRARIES})
  FIND_LIBRARY(TRILINOS_LIBRARY_${_library}
    NAMES ${_library}
    HINTS ${Trilinos_LIBRARY_DIRS}
    NO_DEFAULT_PATH
    NO_CMAKE_ENVIRONMENT_PATH
    NO_CMAKE_PATH
    NO_SYSTEM_ENVIRONMENT_PATH
    NO_CMAKE_SYSTEM_PATH
    NO_CMAKE_FIND_ROOT_PATH
    )

  LIST(APPEND TRILINOS_LIBRARIES ${TRILINOS_LIBRARY_${_library}})

  #
  # Remove the variables from the cache, so that updating TRILINOS_DIR will
  # find the new libraries..
  #
  UNSET(TRILINOS_LIBRARY_${_library} CACHE)
ENDFOREACH()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(TRILINOS DEFAULT_MSG
  TRILINOS_LIBRARIES # cosmetic: Gives nice output
  TRILINOS_FOUND
  )


IF(TRILINOS_FOUND)
  #
  # Extract the major and minor version numbers:
  #
  STRING(REGEX REPLACE
    "^([0-9]+).*$" "\\1"
    TRILINOS_VERSION_MAJOR "${Trilinos_VERSION}")

  STRING(REGEX REPLACE
    "^[0-9]+\\.([0-9]+).*$" "\\1"
    TRILINOS_VERSION_MINOR "${Trilinos_VERSION}")

  STRING(REGEX REPLACE
    "^[0-9]+\\.[0-9]+\\.([0-9]+).*$" "\\1"
    TRILINOS_VERSION_SUBMINOR "${Trilinos_VERSION}")

  #
  # Determine whether Trilinos was configured with MPI:
  #
  FIND_FILE(EPETRA_CONFIG_H Epetra_config.h
    HINTS ${TRILINOS_INCLUDE_DIRS}
    NO_DEFAULT_PATH
    NO_CMAKE_ENVIRONMENT_PATH
    NO_CMAKE_PATH
    NO_SYSTEM_ENVIRONMENT_PATH
    NO_CMAKE_SYSTEM_PATH
    NO_CMAKE_FIND_ROOT_PATH
    )
  FILE(STRINGS "${EPETRA_CONFIG_H}" EPETRA_MPI_STRING
    REGEX "#define HAVE_MPI")
  IF("${EPETRA_MPI_STRING}" STREQUAL "")
    SET(TRILINOS_WITH_MPI FALSE)
  ELSE()
    SET(TRILINOS_WITH_MPI TRUE)
  ENDIF()

  UNSET(EPETRA_CONFIG_H CACHE)

  MARK_AS_ADVANCED(
    TRILINOS_DIR
    )

ELSE()

  IF(TRILINOS_DIR MATCHES "-NOTFOUND")
    SET(TRILINOS_DIR "" CACHE STRING
      "A hint to a Trilinos installation"
      FORCE
      )
  ENDIF()
ENDIF()
