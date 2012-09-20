#
# Try to find the METIS library
#

INCLUDE(FindPackageHandleStandardArgs)

FIND_PATH(METIS_INCLUDE_DIR metis.h
  PATH_SUFFIXES metis
  )

FIND_LIBRARY(METIS_LIBRARY
  NAMES metis
  PATH_SUFFIXES lib64 lib
  )

#
# Extract the version number out of metis.h
#
FILE(STRINGS "${METIS_INCLUDE_DIR}/metis.h" METIS_MAJOR_STRING
  REGEX "METIS_VER_MAJOR")
STRING(REGEX REPLACE "^.*METIS_VER_MAJOR.* ([0-9]+).*" "\\1" METIS_MAJOR "${METIS_MAJOR_STRING}")

FILE(STRINGS "${METIS_INCLUDE_DIR}/metis.h" METIS_MINOR_STRING
  REGEX "METIS_VER_MINOR")
STRING(REGEX REPLACE "^.*METIS_VER_MINOR.* ([0-9]+).*" "\\1" METIS_MINOR "${METIS_MINOR_STRING}")


FIND_PACKAGE_HANDLE_STANDARD_ARGS(METIS DEFAULT_MSG METIS_LIBRARY METIS_INCLUDE_DIR)

IF(METIS_FOUND)
  MARK_AS_ADVANCED(
    METIS_LIBRARY
    METIS_INCLUDE_DIR
  )
ENDIF()

