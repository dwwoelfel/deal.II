#
# Set up deal.II specific definitions
#


SET(DEAL_II_PACKAGE_NAME "deal.II")
SET(DEAL_II_PACKAGE_VERSION ${VERSION})
SET(DEAL_II_PACKAGE_STRING
  "${DEAL_II_PACKAGE_NAME} ${DEAL_II_PACKAGE_VERSION}"
  )

STRING(REGEX REPLACE
  "^([0-9]+)\\..*" "\\1" DEAL_II_MAJOR "${VERSION}"
  )
STRING(REGEX REPLACE
  "^[0-9]+\\.([0-9]+).*" "\\1" DEAL_II_MINOR "${VERSION}"
  )


SET(DEAL_II_PATH ${CMAKE_INSTALL_PREFIX})

IF(DEAL_II_COMPONENT_COMPAT_FILES)
  #
  # The good, old directory structure:
  #
  SET(DEAL_II_DOCUMENTATION_RELDIR "doc")
  SET(DEAL_II_EXAMPLES_RELDIR "examples")
  SET(DEAL_II_INCLUDE_RELDIR "include")
  SET(DEAL_II_LIBRARY_RELDIR "lib")
  SET(DEAL_II_PROJECT_CONFIG_RELDIR ".")

ELSE()
  #
  # IF DEAL_II_COMPONENT_COMPAT_FILES is not set, we assume that we have to
  # obey the FSHS...
  #
  SET_IF_EMPTY(DEAL_II_DOCUMENTATION_RELDIR "share/doc/deal.II/html")
  SET_IF_EMPTY(DEAL_II_EXAMPLES_RELDIR "share/doc/deal.II/examples")
  SET_IF_EMPTY(DEAL_II_INCLUDE_RELDIR "include")
  SET_IF_EMPTY(DEAL_II_LIBRARY_RELDIR "lib${LIB_SUFFIX}")
  SET_IF_EMPTY(DEAL_II_PROJECT_CONFIG_RELDIR "${DEAL_II_LIBRARY_RELDIR}/cmake/deal.II")
ENDIF()

SET(DEAL_II_PACKAGE_BUGREPORT "dealii@dealii.org")
SET(DEAL_II_PACKAGE_TARNAME ${DEAL_II_PACKAGE_NAME}) #TODO
SET(DEAL_II_PACKAGE_URL "http://www.dealii.org")

LIST(APPEND DEAL_II_INCLUDE_DIRS
  "${CMAKE_INSTALL_PREFIX}/${DEAL_II_INCLUDE_RELDIR}"
  )

#
# TODO: This is decoupled from the actual library target atm...
#
IF(CMAKE_BUILD_TYPE MATCHES "Debug")
  SET(DEAL_II_LIBRARY_NAME "deal_II.g")
ELSE()
  SET(DEAL_II_LIBRARY_NAME "deal_II")
ENDIF()
IF(BUILD_SHARED_LIBS)
  SET(DEAL_II_LIBRARY_NAME
    ${CMAKE_SHARED_LIBRARY_PREFIX}deal_II${CMAKE_SHARED_LIBRARY_SUFFIX}
    )
ELSE()
  SET(DEAL_II_LIBRARY_NAME
    ${CMAKE_STATIC_LIBRARY_PREFIX}deal_II${CMAKE_STATIC_LIBRARY_SUFFIX}
    )
ENDIF()

LIST(APPEND DEAL_II_LIBRARIES
  "${CMAKE_INSTALL_PREFIX}/${DEAL_II_LIBRARY_RELDIR}/${DEAL_II_LIBRARY_NAME}"
  )

