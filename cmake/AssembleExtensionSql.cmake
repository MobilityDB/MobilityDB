#-----------------------------------------------------------------------------
# Assembling a PostgreSQL extension script from many SQL sources
#
# Both MobilityDB extensions (mobilitydb and mobilitydb_datagen) ship as a
# single generated <name>--<version>.sql that is the concatenation of dozens of
# per-module SQL sources. Every source carries the full licence block, which
# would otherwise land in the generated script once per source; and the script
# needs the psql guard and the DO NOT EDIT banner exactly once, at the top.
# This module centralises that assembly so both extensions get the same
# treatment.
#-----------------------------------------------------------------------------

# The licence block that opens every SQL source, matched structurally rather
# than by exact text: the copyright years are not uniform across sources, and
# the codegen-generated sources carry a provenance banner ahead of the licence.
# Anchoring on the invariant first three lines and closing on the first `***/`
# line is what keeps the match from also swallowing the `/**** ... ****/`
# section separators that appear throughout the file bodies.
set(SQL_LICENCE_REGEX
  "/\\*\\*+\n \\*\n \\* This MobilityDB code is provided under The PostgreSQL License\\.\n( \\*[^\n]*\n)* \\*+/\n")

# Strip the licence block from the SQL text held in the variable named `var`.
# A source with no recognisable licence block is left untouched.
function(strip_sql_licence var)
  string(REGEX REPLACE "${SQL_LICENCE_REGEX}" "" _stripped "${${var}}")
  set(${var} "${_stripped}" PARENT_SCOPE)
endfunction()

# assemble_extension_sql(OUTPUT <file> HEADER <file> BASE_DIR <dir>
#                        SOURCES <file>...)
#
# Write OUTPUT as HEADER followed by every file in SOURCES, with each source's
# licence block removed and a marker naming the source it came from. Markers
# are reported relative to BASE_DIR.
#
# OUTPUT is rewritten from scratch on every configure. Re-running cmake in an
# existing build directory must not append a second copy of every rule on top
# of the previous pass (#794) — seeding from HEADER with file(WRITE) rather
# than file(APPEND) is what guarantees that, and it is why HEADER is read and
# written here instead of being configure_file'd straight onto OUTPUT.
function(assemble_extension_sql)
  cmake_parse_arguments(ARG "" "OUTPUT;HEADER;BASE_DIR" "SOURCES" ${ARGN})

  file(READ ${ARG_HEADER} _header)
  file(WRITE ${ARG_OUTPUT} "${_header}")

  foreach(f ${ARG_SOURCES})
    file(READ ${f} _contents)
    strip_sql_licence(_contents)
    file(RELATIVE_PATH _label ${ARG_BASE_DIR} ${f})
    # The leading newline doubles as the separator between two sources, so a
    # source that does not end in a newline cannot glue its last statement onto
    # the next source's first line.
    file(APPEND ${ARG_OUTPUT} "\n-- Begin contents of ${_label}\n\n${_contents}\n")
  endforeach()
endfunction()

# Register SQL sources read with file(READ) as configure dependencies.
#
# file(READ) creates no dependency of its own, so without this cmake never
# re-runs when a SQL source is edited: the extension target reports itself up
# to date and `make install` silently ships the previous contents.
function(add_sql_configure_depends)
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${ARGN})
endfunction()
