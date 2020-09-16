
#---------------------------------------------
#  MOBILITYDB_SOURCE_NAMES
#---------------------------------------------
#
# Name of the directories that have
# - C/C++ code
# - SQL code
# - documentation code
#
# structure:
#
# directory | code | sql | doc
#
# where:
#
# directory: is the name of the directory
# code: Y / N value, when "Y" C/C++ code will be looked for
# sql: Y / N value, when "Y" SQL code will be looked for
# doc: Y / N value, when "Y" Documentation code will be looked for
#----------------------
configure_file("configuration.conf" "configuration.conf")
file(STRINGS configuration.conf MOBILITYDB_CONFIGURATION_FILE)

set(MOBILITYDB_SOURCE_NAMES "")
set(MOBILITYDB_SQL_DIRECTORIES "")
set(MOBILITYDB_DOC_DIRECTORIES "")
foreach(line ${MOBILITYDB_CONFIGURATION_FILE})
    string(REGEX REPLACE "^(#).*" "\\1" comment ${line})
    if("${comment}" MATCHES "#")
        continue()
    endif()
    string(REGEX REPLACE "^(.*)\\|(.*)\\|(.*)\\|(.*)" "\\1" directory ${line})
    string(REGEX REPLACE "^(.*)\\|(.*)\\|(.*)\\|(.*)" "\\2" has_code ${line})
    string(REGEX REPLACE "^(.*)\\|(.*)\\|(.*)\\|(.*)" "\\3" has_sql ${line})
    string(REGEX REPLACE "^(.*)\\|(.*)\\|(.*)\\|(.*)" "\\4" has_doc ${line})

    string(STRIP ${directory} directory)
    string(STRIP ${has_code} has_code)
    string(STRIP ${has_sql} has_sql)
    string(STRIP ${has_doc} has_doc)


    if( ${has_code} MATCHES "Y")
        list(APPEND MOBILITYDB_SOURCE_NAMES "${directory}")
    endif()
    if( ${has_sql} MATCHES "Y")
        list(APPEND MOBILITYDB_SQL_DIRECTORIES "${directory}")
    endif()
    if( ${has_doc} MATCHES "Y")
        list(APPEND MOBILITYDB_DOC_DIRECTORIES "${directory}")
    endif()
endforeach()

if (MOBILITYDB_DEBUG)
    message(STATUS "MOBILITYDB_SOURCE_NAMES ${MOBILITYDB_SOURCE_NAMES}")
    message(STATUS "MOBILITYDB_SQL_DIRECTORIES ${MOBILITYDB_SQL_DIRECTORIES}")
    message(STATUS "MOBILITYDB_DOC_DIRECTORIES ${MOBILITYDB_DOC_DIRECTORIES}")
endif()
