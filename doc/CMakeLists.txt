#
# Based on:
# https://majewsky.wordpress.com/2010/08/14/tip-of-the-day-cmake-and-doxygen/
#

option(BUILD_DOC
  "Set ON|OFF (default=OFF) to build user's documentation" ON)

if (BUILD_DOC)
  message(STATUS "Build user's documentation.")
  # Required packages for generating the documentation
  find_package(DocBook REQUIRED)
  # find_package(docbook-xsl REQUIRED)

  # Optional packages for validating and generating the documentation
  # The validation with xmllint
  find_package(LibXml2)
  # The documentation in HTML, PDF, or EPUB will be generated
  # depending on the available packages
  find_package(LibXslt)
  find_package(DBLATEX)
  find_package(DBTOEPUB)
  # The .po and .pot files for the translations will be generated
  find_package(Gettext)

  if(LIBXML2_FOUND)
    add_custom_target(xmllint
      ${LIBXML2_XMLLINT_EXECUTABLE} --valid mobilitydb-manual.xml
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc
      COMMENT "Validating the XML files of the user's documentation" VERBATIM
    )
  endif()
  if(LIBXSLT_FOUND)
    add_custom_target(dbhtml
      ${LIBXSLT_XSLTPROC_EXECUTABLE} --stringparam html.stylesheet ${CMAKE_CURRENT_BINARY_DIR}/doc/docbook.css --xinclude
        -o ${CMAKE_CURRENT_BINARY_DIR}/doc/index.html ${DOCBOOK_XSL_FILE}/html/chunk.xsl mobilitydb-manual.xml
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc
      COMMENT "Generating user's documentation in HTML format" VERBATIM
    )
  endif()
  if(DBLATEX_FOUND)
    add_custom_target(dblatex
      ${DBLATEX_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc
      COMMENT "Generating user's documentation in PDF format" VERBATIM
    )
  endif()
  if(DBTOEPUB_FOUND)
    add_custom_target(dbtoepub
      ${DBTOEPUB_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc
      COMMENT "Generating user's documentation in EPUB format" VERBATIM
    )
  endif()
  if(GETTEXT_FOUND)
    # add_custom_target(translate
      # ${DBTOEPUB_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
      # WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc
      # COMMENT "Translating the user's documentation" VERBATIM
    # )
  endif()

endif()
