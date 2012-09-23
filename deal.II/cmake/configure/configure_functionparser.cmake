MACRO(FEATURE_FUNCTIONPARSER_FIND_EXTERNAL var)
  MESSAGE(STATUS
    "No module available for finding functionparser externally."
    )
ENDMACRO()


MACRO(FEATURE_FUNCTIONPARSER_CONFIGURE_EXTERNAL var)
ENDMACRO()


SET(FEATURE_FUNCTIONPARSER_HAVE_CONTRIB TRUE)


MACRO(FEATURE_FUNCTIONPARSER_CONFIGURE_CONTRIB var)
  #
  # compile the necessary parts of functionparser out of ./contrib
  #

  INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/contrib/functionparser/
    )

  ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/contrib/functionparser)

  #
  # Add functionparser directly to the object files of deal.II
  #
  LIST(APPEND deal_ii_additional_object_files
    $<TARGET_OBJECTS:obj_functionparser>
    )

  SET(HAVE_FUNCTIONPARSER TRUE)

  SET(${var} TRUE)
ENDMACRO()


SET(FEATURE_FUNCTIONPARSER_CUSTOM_ERROR_MESSAGE TRUE)


MACRO(FEATURE_FUNCTIONPARSER_ERROR_MESSAGE)
  MESSAGE(SEND_ERROR "\n"
    "No module available for finding functionparser externally.\n"
    "Disable DEAL_II_WITH_FUNCTIONPARSER, or enable DEAL_II_ALLOW_CONTRIB.\n\n"
    )
ENDMACRO()


CONFIGURE_FEATURE(FUNCTIONPARSER)

