#
# Copyright (c) 2021 Esteban Zimanyi
#
# Based on:
# https://github.com/pgRouting/pgrouting/blob/main/doc/CMakeLists.txt
#

#---------------------------------------------
# User's documentation options
#---------------------------------------------

option(ALL_DOC
  "Set ON|OFF (default=OFF) to build the user's documentation in all formats: HTML, PDF, and EPUB

  When OFF and want to build specific format, use the format code in capital letters:
  -D HTML=ON
  "
  OFF
)

option(HTML
  "Set ON|OFF (default=OFF) to build the user's documentation in HTML format" OFF)

option(PDF
  "Set ON|OFF (default=OFF) to build the user's documentation in PDF format" OFF)

option(EPUB
  "Set ON|OFF (default=OFF) to build the user's documentation in EPUB format" OFF)

# Defining the user's documentation format options
foreach(format HTML PDF EPUB)
  string(TOUPPER ${format} opt)
  if(${opt} OR ALL_DOC)
    list(APPEND PROJECT_DOC_TARGETS ${opt})
    set(${opt} ON)
  endif()
endforeach()

message(STATUS "PROJECT_DOC_TARGETS = ${PROJECT_DOC_TARGETS}")

#---------------------------------------------
# Available Languages
# English is always built
#---------------------------------------------

set(PROJECT_SUPPORTED_LANGUAGES "es")
set(PROJECT_SUPPORTED_LANGUAGES_STRING "Spanish")
set(PROJECT_ENGLISH "en")
set(PROJECT_ENGLISH_STRING "English")

#---------------------------------------------
# Language options
#---------------------------------------------

option(ALL_LANG
  "Set ON|OFF (default=OFF) to build all the supported languages:
  Main language will always be built:
  ${PROJECT_ENGLISH_STRING}
  Optional languages:
  ${PROJECT_SUPPORTED_LANGUAGES_STRING}

  When OFF and want to build specific optional language, use the language code in capital letters:
  -D ES=ON
  "
  OFF
)

# Defining the available language options
foreach(lang ${PROJECT_SUPPORTED_LANGUAGES})
  list(FIND PROJECT_SUPPORTED_LANGUAGES lang idx)
  list(GET PROJECT_SUPPORTED_LANGUAGES_STRING idx lang_str)
  string(TOUPPER ${lang} val)
  option(${val}
    "Set ON|OFF (default=OFF) build the ${lang_str} user's documentation" OFF)
endforeach()

# Determining the language to be build. English is always built.
list(APPEND PROJECT_BUILD_LANGUAGES ${PROJECT_ENGLISH})
list(APPEND PROJECT_BUILD_LANGUAGES_STRING ${PROJECT_ENGLISH_STRING})
foreach(lang ${PROJECT_SUPPORTED_LANGUAGES})
  string(TOUPPER ${lang} opt)
  if(${opt} OR ALL_LANG)
    list(FIND PROJECT_SUPPORTED_LANGUAGES lang idx)
    list(GET PROJECT_SUPPORTED_LANGUAGES_STRING idx lang_str)
    list(APPEND PROJECT_BUILD_LANGUAGES ${lang})
    list(APPEND PROJECT_BUILD_LANGUAGES_STRING ${lang_str})
  endif()
endforeach()
message(STATUS "PROJECT_BUILD_LANGUAGES = ${PROJECT_BUILD_LANGUAGES}")
message(STATUS "PROJECT_BUILD_LANGUAGES_STRING = ${PROJECT_BUILD_LANGUAGES_STRING}")

#---------------------------------------------
# Processing
#---------------------------------------------

if(HTML OR PDF OR EPUB OR ALL_DOC)
  message(STATUS "Build the user's documentation.")
  # Required package for generating the user's documentation
  find_package(DocBook REQUIRED)
  # Required package for validating the XML files of the user's documentation with xmllint
  find_package(LibXml2 REQUIRED)

  # Optional packages for generating the user's documentation in
  # HTML, PDF, or EPUB depending on the selected options

  # Set the message level
  # - to WARNING for finding at least one target when building all formats
  # - to FATAL_ERROR when the requested target cannot be built
  if(ALL_DOC)
    set(msg_level "WARNING")
  else()
    set(msg_level "FATAL_ERROR")
  endif()

  if(HTML)
    find_package(LibXslt)
    if(NOT LIBXSLT_FOUND)
      message(${msg_level} "No package found for generating the user's documentation in HTML format")
    endif()
  endif()

  if(PDF)
    find_package(DBLATEX)
    if(NOT DBLATEX_FOUND)
      message(${msg_level} "No package found for generating the user's documentation in PDF format")
    endif()
  endif()

  if(EPUB)
    find_package(DBTOEPUB)
    if(NOT DBTOEPUB_FOUND)
      message(${msg_level} "No package found for generating the user's documentation in EPUB format")
    endif()
  endif()

  if(ALL_DOC)
    if(NOT LIBXSLT_FOUND AND NOT DBLATEX_FOUND AND NOT DBTOEPUB_FOUND)
      message(FATAL_ERROR "No package found for generating the user's documentation")
    endif()
    # Add an empty target that depends on the targets of all formats
    add_custom_target(doc
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
      COMMENT "Generating the user's documentation" VERBATIM
    )
  endif()

  # Optional packages for generating the .po and .pot files for the translations
  # find_package(Gettext)
  # find_package(POXML)

  # Function that validates the XML files of the user's documentation
  function(validate_xml_file file lang)
    message(STATUS "Validating the XML files of the ${lang_str} user's documentation")
    execute_process(
      COMMAND ${LIBXML2_XMLLINT_EXECUTABLE} --valid --noent --noout ${file}
      RESULT_VARIABLE STATUS
      ERROR_QUIET
    )
    if(STATUS AND NOT STATUS EQUAL 0)
      message(FATAL_ERROR "The XML files are not valid. Please run xmllint to validate them.")
    endif()
    return()
  endfunction(validate_xml_file)

  # Validates the XML files of the user's documentation for all build languages
  foreach (lang ${PROJECT_BUILD_LANGUAGES})
    list(FIND PROJECT_BUILD_LANGUAGES "${lang}" idx)
    list(GET PROJECT_BUILD_LANGUAGES_STRING "${idx}" lang_str)
    if("${lang}" STREQUAL "en")
      validate_xml_file(${CMAKE_CURRENT_SOURCE_DIR}/mobilitydb-manual.xml lang lang_str)
    else()
      validate_xml_file(${CMAKE_CURRENT_SOURCE_DIR}/${lang}/mobilitydb-manual.xml lang lang_str)
    endif()
  endforeach()

  # Generating the user's documentation in HTML format
  if(LIBXSLT_FOUND)
    # Add an empty target that depends on the targets of all build languages
    add_custom_target(doc_html
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}${locale}"
      COMMENT "Generating the user's documentation in HTML format" VERBATIM
    )
    # Add dependency on the doc target of all formats
    if(ALL_DOC)
      add_dependencies(doc doc_html)
    endif()
    # Copy the images before building the doc_html target
    add_custom_target(doc_html_img
      COMMAND ${CMAKE_COMMAND} -E copy_directory
         ${CMAKE_CURRENT_SOURCE_DIR}/images ${CMAKE_CURRENT_BINARY_DIR}/images
      COMMENT "Copying the image files of the user's documentation" VERBATIM
    )
    # Add target of all build languages
    add_dependencies(doc_html doc_html_img)
    foreach (lang ${PROJECT_BUILD_LANGUAGES})
      if("${lang}" STREQUAL "en")
        set(locale "")
      else()
        set(locale "${lang}/")
      endif()
      list(FIND PROJECT_BUILD_LANGUAGES "${lang}" idx)
      list(GET PROJECT_BUILD_LANGUAGES_STRING "${idx}" lang_str)
      add_custom_target(doc_html_${lang}
        ${LIBXSLT_XSLTPROC_EXECUTABLE} --stringparam html.stylesheet ${CMAKE_CURRENT_BINARY_DIR}/${locale}docbook.css
          # Make section numbers to include the chapter number. This assumes chapter numbering is turned on
          --stringparam section.autolabel 1 --stringparam section.label.includes.component.label 1
          # Make generated text for chapters, sections, ... is only the number, instead the default number and title
          --stringparam xref.with.number.and.title 0
          --xinclude -o ${CMAKE_CURRENT_BINARY_DIR}/${locale}index.html ${DOCBOOK_XSL}/html/chunk.xsl
          ${CMAKE_CURRENT_SOURCE_DIR}/${locale}mobilitydb-manual.xml
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating the ${lang_str} user's documentation in HTML format" VERBATIM
      )
      add_dependencies(doc_html doc_html_${lang})
    endforeach()
 endif()

  # Generating the user's documentation in PDF format
  if(DBLATEX_FOUND)
    # Add an empty target that depends on the targets of all build languages
    add_custom_target(doc_pdf
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}${locale}"
      COMMENT "Generating the user's documentation in PDF format" VERBATIM
    )
    # Add dependency on the doc target of all formats
    if(ALL_DOC)
      add_dependencies(doc doc_pdf)
    endif()
    # Add target of all build languages
    foreach (lang ${PROJECT_BUILD_LANGUAGES})
      if("${lang}" STREQUAL "en")
        set(locale "")
      else()
        set(locale "${lang}/")
      endif()
      list(FIND PROJECT_BUILD_LANGUAGES "${lang}" idx)
      list(GET PROJECT_BUILD_LANGUAGES_STRING "${idx}" lang_str)
      add_custom_target(doc_pdf_${lang}
        ${DBLATEX_COMMAND} -s ${CMAKE_CURRENT_SOURCE_DIR}/${locale}texstyle.sty
          ${CMAKE_CURRENT_SOURCE_DIR}/${locale}mobilitydb-manual.xml
          -o ${CMAKE_CURRENT_BINARY_DIR}/${locale}mobilitydb-manual.pdf
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating the ${lang_str} user's documentation in PDF format" VERBATIM
      )
      if(NOT "${lang}" STREQUAL "en")
        add_custom_target(doc_pdf_${lang}_dir
          ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${locale}
          COMMENT "Creating directory ${CMAKE_CURRENT_BINARY_DIR}/${locale}" VERBATIM
        )
        add_dependencies(doc_pdf_${lang} doc_pdf_${lang}_dir)
      endif()
      add_dependencies(doc_pdf doc_pdf_${lang})
    endforeach()
  endif()

  # Generating the user's documentation in EPUB format
  if(DBTOEPUB_FOUND)
    # Add an empty target that depends on the targets of all build languages
    add_custom_target(doc_epub
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}${locale}"
        COMMENT "Generating the user's documentation in EPUB format" VERBATIM
    )
    # Add dependency on the doc target of all formats
    if(ALL_DOC)
      add_dependencies(doc doc_epub)
    endif()
    # Add target of all build languages
    foreach (lang ${PROJECT_BUILD_LANGUAGES})
      if("${lang}" STREQUAL "en")
        set(locale "")
      else()
        set(locale "${lang}/")
      endif()
      list(FIND PROJECT_BUILD_LANGUAGES "${lang}" idx)
      list(GET PROJECT_BUILD_LANGUAGES_STRING "${idx}" lang_str)
      add_custom_target(doc_epub_${lang}
        ${DBTOEPUB_COMMAND} ${CMAKE_CURRENT_SOURCE_DIR}/${locale}mobilitydb-manual.xml
          -o ${CMAKE_CURRENT_BINARY_DIR}/${locale}mobilitydb-manual.epub
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating the ${lang_str} user's documentation in EPUB format" VERBATIM
      )
      if(NOT "${lang}" STREQUAL "en")
        add_custom_target(doc_epub_${lang}_dir
          ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${locale}
          COMMENT "Creating directory ${CMAKE_CURRENT_BINARY_DIR}/${locale}" VERBATIM
        )
        add_dependencies(doc_epub_${lang} doc_epub_${lang}_dir)
      endif()
      add_dependencies(doc_epub doc_epub_${lang})
    endforeach()
  endif()

endif()
