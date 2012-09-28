#
# Configuration for the ARPACK library:
#

MACRO(FEATURE_ARPACK_FIND_EXTERNAL var)
  FIND_PACKAGE(ARPACK)

  IF(ARPACK_FOUND)
    SET(${var} TRUE)
  ENDIF()
ENDMACRO()


MACRO(FEATURE_ARPACK_CONFIGURE_EXTERNAL var)
  INCLUDE_DIRECTORIES(${ARPACK_INCLUDE_DIR})
  LIST(APPEND DEAL_II_EXTERNAL_LIBRARIES ${ARPACK_LIBRARY})

  SET(DEAL_II_USE_ARPACK TRUE)

  SET(${var} TRUE)
ENDMACRO()


CONFIGURE_FEATURE(ARPACK)

