## ---------------------------------------------------------------------
## $Id$
##
## Copyright (C) 2012 - 2013 by the deal.II authors
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
# Try to find the SLEPC library
#
# This module exports:
#
#     SLEPC_FOUND
#     SLEPC_LIBRARIES
#     SLEPC_INCLUDE_DIRS
#     SLEPC_VERSION
#     SLEPC_VERSION_MAJOR
#     SLEPC_VERSION_MINOR
#     SLEPC_VERSION_SUBMINOR
#     SLEPC_VERSION_PATCH
#

INCLUDE(FindPackageHandleStandardArgs)

SET_IF_EMPTY(SLEPC_DIR "$ENV{SLEPC_DIR}")
SET_IF_EMPTY(PETSC_DIR "$ENV{PETSC_DIR}")
SET_IF_EMPTY(PETSC_ARCH "$ENV{PETSC_ARCH}")

#
# Luckily, SLEPc wants the same insanity as PETSc, so we can just copy the
# mechanism.
#

FIND_LIBRARY(SLEPC_LIBRARY
  NAMES slepc
  HINTS
    # SLEPC is special. Account for that
    ${SLEPC_DIR}
    ${SLEPC_DIR}/${PETSC_ARCH}
    ${PETSC_DIR}
  PATH_SUFFIXES lib${LIB_SUFFIX} lib64 lib
)

FIND_PATH(SLEPC_INCLUDE_DIR_ARCH slepcconf.h
  HINTS
    # SLEPC is special. Account for that
    ${SLEPC_DIR}
    ${SLEPC_DIR}/${PETSC_ARCH}
    ${SLEPC_INCLUDE_DIRS}
    ${PETSC_DIR}
  PATH_SUFFIXES slepc include include/slepc
)

FIND_PATH(SLEPC_INCLUDE_DIR_COMMON slepcversion.h
  HINTS
    ${SLEPC_DIR}
    ${SLEPC_DIR}/${PETSC_ARCH}
    ${SLEPC_INCLUDE_DIRS}
    ${PETSC_DIR}
  PATH_SUFFIXES slepc include include/slepc
)


FIND_PACKAGE_HANDLE_STANDARD_ARGS(SLEPC DEFAULT_MSG
  SLEPC_LIBRARY
  SLEPC_INCLUDE_DIR_ARCH
  SLEPC_INCLUDE_DIR_COMMON
  PETSC_FOUND
  )

MARK_AS_ADVANCED(
  SLEPC_DIR
  SLEPC_INCLUDE_DIR_ARCH
  SLEPC_INCLUDE_DIR_COMMON
  SLEPC_INCLUDE_DIRS
  SLEPC_LIBRARY
  )

IF(SLEPC_FOUND)
  SET(SLEPC_INCLUDE_DIRS
    ${SLEPC_INCLUDE_DIR_ARCH}
    ${SLEPC_INCLUDE_DIR_COMMON}
    )
  SET(SLEPC_LIBRARIES
    ${SLEPC_LIBRARY}
    ${PETSC_LIBRARIES}
    )

  SET(SLEPC_SLEPCVERSION_H "${SLEPC_INCLUDE_DIR_COMMON}/slepcversion.h")

  FILE(STRINGS "${SLEPC_SLEPCVERSION_H}" SLEPC_VERSION_MAJOR_STRING
    REGEX "#define.*SLEPC_VERSION_MAJOR")
  STRING(REGEX REPLACE "^.*SLEPC_VERSION_MAJOR.*([0-9]+).*" "\\1"
    SLEPC_VERSION_MAJOR "${SLEPC_VERSION_MAJOR_STRING}"
    )

  FILE(STRINGS "${SLEPC_SLEPCVERSION_H}" SLEPC_VERSION_MINOR_STRING
    REGEX "#define.*SLEPC_VERSION_MINOR")
  STRING(REGEX REPLACE "^.*SLEPC_VERSION_MINOR.*([0-9]+).*" "\\1"
    SLEPC_VERSION_MINOR "${SLEPC_VERSION_MINOR_STRING}"
    )

  FILE(STRINGS "${SLEPC_SLEPCVERSION_H}" SLEPC_VERSION_SUBMINOR_STRING
    REGEX "#define.*SLEPC_VERSION_SUBMINOR")
  STRING(REGEX REPLACE "^.*SLEPC_VERSION_SUBMINOR.*([0-9]+).*" "\\1"
    SLEPC_VERSION_SUBMINOR "${SLEPC_VERSION_SUBMINOR_STRING}"
    )

  FILE(STRINGS "${SLEPC_SLEPCVERSION_H}" SLEPC_VERSION_PATCH_STRING
    REGEX "#define.*SLEPC_VERSION_PATCH")
  STRING(REGEX REPLACE "^.*SLEPC_VERSION_PATCH.*([0-9]+).*" "\\1"
    SLEPC_VERSION_PATCH "${SLEPC_VERSION_PATCH_STRING}"
    )

  SET(SLEPC_VERSION
    "${SLEPC_VERSION_MAJOR}.${SLEPC_VERSION_MINOR}.${SLEPC_VERSION_SUBMINOR}.${SLEPC_VERSION_PATCH}"
    )

  MARK_AS_ADVANCED(SLEPC_DIR)
ELSE()
  SET(SLEPC_DIR "" CACHE PATH
    "An optional hint to a SLEPC directory"
    )
ENDIF()

