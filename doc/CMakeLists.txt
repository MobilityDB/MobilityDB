#
# Copyright (c) 2021 Esteban Zimanyi
#
# Based on:
# https://github.com/pgRouting/pgrouting/blob/main/doc/CMakeLists.txt
#

#---------------------------------------------
# User's documentation options
#---------------------------------------------

option(DOC_ALL
  "Set ON|OFF (default=OFF) to build the user's documentation in all formats: HTML, PDF, and EPUB

  When OFF and want to build specific format, use the format code in capital letters:
  -D DOC_HTML=ON
  "
  OFF
)
option(DOC_HTML
  "Set ON|OFF (default=OFF) to build the user's documentation in HTML format" OFF)
option(DOC_PDF
  "Set ON|OFF (default=OFF) to build the user's documentation in PDF format" OFF)
option(DOC_EPUB
  "Set ON|OFF (default=OFF) to build the user's documentation in EPUB format" OFF)

# Defining the user's documentation format options
foreach(format DOC_HTML DOC_PDF DOC_EPUB)
  string(TOUPPER ${format} opt)
  if(${opt} OR DOC_ALL)
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

option(LANG_ALL
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
  if(${opt} OR LANG_ALL)
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

if(DOC_HTML OR DOC_PDF OR DOC_EPUB OR DOC_ALL)
  message(STATUS "Build the user's documentation.")
  # Required package for generating the user's documentation in any format
  find_package(DocBook REQUIRED)
  # Required package for validating the XML files of the user's documentation with xmllint
  find_package(LibXml2 REQUIRED)

  # Add an empty main doc target that will be dependent on the main format targets
  add_custom_target(doc
    COMMENT "Generating the user's documentation" VERBATIM
  )

  # Optional packages for generating the user's documentation in
  # HTML, PDF, or EPUB depending on the selected options

  # Set the message level
  # - to WARNING for finding at least one target when building all formats
  # - to FATAL_ERROR when the requested target cannot be built
  if(DOC_ALL)
    set(msg_level "WARNING")
  else()
    set(msg_level "FATAL_ERROR")
  endif()

  # Required packages for generating the user's documentation in HTML format
  if(DOC_HTML)
    find_package(LibXslt)
    if(NOT LIBXSLT_FOUND)
      message(${msg_level} "No package found for generating the user's documentation in HTML format")
    endif()
  endif()

  # Required packages for generating the user's documentation in PDF format
  if(DOC_PDF)
    find_package(DBLATEX)
    if(NOT DBLATEX_FOUND)
      message(${msg_level} "No package found for generating the user's documentation in PDF format")
    endif()
  endif()

  # Required packages for generating the user's documentation in EPUB format
  if(DOC_EPUB)
    find_package(LibXslt)
    find_package(DBTOEPUB)
    if(NOT LIBXSLT_FOUND OR NOT DBTOEPUB_FOUND)
      message(${msg_level} "No package found for generating the user's documentation in EPUB format")
    endif()
  endif()

  # When building all formats verify that at least one format will be produced
  if(DOC_ALL)
    if(NOT LIBXSLT_FOUND AND NOT DBLATEX_FOUND AND NOT DBTOEPUB_FOUND)
      message(FATAL_ERROR "No package found for generating the user's documentation")
    endif()
  endif()

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

  # Validate the XML files of the user's documentation for all build languages
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
    # Add an empty main html target
    add_custom_target(doc_html
      COMMENT "Generating the user's documentation in HTML format" VERBATIM
    )
    # Make the main doc target dependent on the main html target
    if(DOC_ALL OR DOC_HTML)
      add_dependencies(doc doc_html)
    endif()
    # Add target copying the image directory and make the main html target dependent on it
    add_custom_target(doc_html_img
      COMMAND ${CMAKE_COMMAND} -E copy_directory
         ${CMAKE_CURRENT_SOURCE_DIR}/images ${CMAKE_CURRENT_BINARY_DIR}/images
      COMMENT "Copying the image files of the user's documentation" VERBATIM
    )
    add_dependencies(doc_html doc_html_img)
    # Loop for all build languages
    foreach (lang ${PROJECT_BUILD_LANGUAGES})
      # Set subdirectory variable for all languages excepted English
      if("${lang}" STREQUAL "en")
        set(locale "")
      else()
        set(locale "/${lang}")
      endif()
      # Get the message string for every language
      list(FIND PROJECT_BUILD_LANGUAGES "${lang}" idx)
      list(GET PROJECT_BUILD_LANGUAGES_STRING "${idx}" lang_str)
      # Add html target for every language and make the main html target dependent on it
      add_custom_target(doc_html_${lang}
        ${LIBXSLT_XSLTPROC_EXECUTABLE} --output ${CMAKE_CURRENT_BINARY_DIR}${locale}/index.html
          # Include the customized docbook.css
          --stringparam html.stylesheet docbook.css
          # Make section numbers to include the chapter number. This assumes chapter numbering is turned on
          --stringparam section.autolabel 1 --stringparam section.label.includes.component.label 1
          # Generated text for chapters, sections, figures... is only the number and not the tile
          --stringparam xref.with.number.and.title 0
          # Placement for the legend for figures is after
          --stringparam formal.title.placement "figure after table after"
          # Allow controlling individual cell borders
          --stringparam table.borders.with.css 1
          --xinclude ${DOCBOOK_XSL}/html/chunk.xsl ${CMAKE_CURRENT_SOURCE_DIR}${locale}/mobilitydb-manual.xml
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating the ${lang_str} user's documentation in HTML format" VERBATIM
      )
      add_dependencies(doc_html doc_html_${lang})
      # Add target creating the subdirectory for every language distinct than English
      # and make the html target for the language dependent on it
      if(NOT "${lang}" STREQUAL "en")
        add_custom_target(doc_html_${lang}_dir
          ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}${locale}
          COMMENT "Creating directory ${CMAKE_CURRENT_BINARY_DIR}${locale}" VERBATIM
        )
        add_dependencies(doc_html_${lang} doc_html_${lang}_dir)
      endif()
      # Add target copying the docbook.css and make the html target for each language dependent on it
      add_custom_target(doc_html_${lang}_css
        COMMAND ${CMAKE_COMMAND} -E copy
           ${CMAKE_CURRENT_SOURCE_DIR}/docbook.css ${CMAKE_CURRENT_BINARY_DIR}${locale}
        COMMENT "Copying the docbook.css file in the directory ${CMAKE_CURRENT_BINARY_DIR}${locale}" VERBATIM
      )
      if(NOT "${lang}" STREQUAL "en")
        add_dependencies(doc_html_${lang}_css doc_html_${lang}_dir)
      endif()
      add_dependencies(doc_html_${lang} doc_html_${lang}_css)
    endforeach()
 endif()

  # Generating the user's documentation in PDF format
  if(DBLATEX_FOUND)
    # Add an empty main pdf target
    add_custom_target(doc_pdf
      COMMENT "Generating the user's documentation in PDF format" VERBATIM
    )
    # Make the main doc target dependent on the main pdf target
    if(DOC_ALL OR DOC_PDF)
      add_dependencies(doc doc_pdf)
    endif()
    # Loop for all build languages
    foreach (lang ${PROJECT_BUILD_LANGUAGES})
      # Set subdirectory variable for all languages excepted English
      if("${lang}" STREQUAL "en")
        set(locale "")
      else()
        set(locale "/${lang}")
      endif()
      # Get the message string for every language
      list(FIND PROJECT_BUILD_LANGUAGES "${lang}" idx)
      list(GET PROJECT_BUILD_LANGUAGES_STRING "${idx}" lang_str)
      # Add pdf target for every language and make the main pdf target dependent on it
      add_custom_target(doc_pdf_${lang}
        ${DBLATEX_COMMAND} -s ${CMAKE_CURRENT_SOURCE_DIR}${locale}/texstyle.sty
          ${CMAKE_CURRENT_SOURCE_DIR}${locale}/mobilitydb-manual.xml
          -o ${CMAKE_CURRENT_BINARY_DIR}${locale}/mobilitydb-manual.pdf
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating the ${lang_str} user's documentation in PDF format" VERBATIM
      )
      add_dependencies(doc_pdf doc_pdf_${lang})
      # Add target creating the subdirectory for every language distinct than English
      # and make the pdf target for the language dependent on it
      if(NOT "${lang}" STREQUAL "en")
        add_custom_target(doc_pdf_${lang}_dir
          ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}${locale}
          COMMENT "Creating directory ${CMAKE_CURRENT_BINARY_DIR}${locale}" VERBATIM
        )
        add_dependencies(doc_pdf_${lang} doc_pdf_${lang}_dir)
      endif()
    endforeach()
  endif()

  # Generating the user's documentation in EPUB format
  if(DBTOEPUB_FOUND)
    # Set the ${DOCBOOK_XSL} variable in the epub.xsl file
    configure_file(epub.xsl ${CMAKE_CURRENT_BINARY_DIR}/epub.xsl)
    # Add an empty main epub target
    add_custom_target(doc_epub
        COMMENT "Generating the user's documentation in EPUB format" VERBATIM
    )
    # Make the main doc target dependent on the main epub target
    if(DOC_ALL OR DOC_EPUB)
      add_dependencies(doc doc_epub)
    endif()
    # Loop for all build languages
    foreach (lang ${PROJECT_BUILD_LANGUAGES})
      # Set subdirectory variable for all languages excepted English
      if("${lang}" STREQUAL "en")
        set(locale "")
      else()
        set(locale "/${lang}")
      endif()
      # Get the message string for every language
      list(FIND PROJECT_BUILD_LANGUAGES "${lang}" idx)
      list(GET PROJECT_BUILD_LANGUAGES_STRING "${idx}" lang_str)
      # Add epub target for every language and make the main epub target dependent on it
      add_custom_target(doc_epub_${lang}
        ${DBTOEPUB_COMMAND} -c ${CMAKE_CURRENT_SOURCE_DIR}/docbook.css
          -s ${CMAKE_CURRENT_BINARY_DIR}/epub.xsl
          -o ${CMAKE_CURRENT_BINARY_DIR}${locale}/mobilitydb-manual.epub
          ${CMAKE_CURRENT_SOURCE_DIR}${locale}/mobilitydb-manual.xml
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating the ${lang_str} user's documentation in EPUB format" VERBATIM
      )
      add_dependencies(doc_epub doc_epub_${lang})
      # Add target creating the subdirectory for every language distinct than English
      # and make the epub target for the language dependent on it
      if(NOT "${lang}" STREQUAL "en")
        add_custom_target(doc_epub_${lang}_dir
          ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}${locale}
          COMMENT "Creating directory ${CMAKE_CURRENT_BINARY_DIR}${locale}" VERBATIM
        )
        add_dependencies(doc_epub_${lang} doc_epub_${lang}_dir)
      endif()
    endforeach()
  endif()

endif()
