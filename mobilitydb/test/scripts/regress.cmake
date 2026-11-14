#
# This MobilityDB code is provided under The PostgreSQL License.
# Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
# contributors
#
# MobilityDB includes portions of PostGIS version 3 source code released
# under the GNU General Public License (GPLv2 or later).
# Copyright (c) 2001-2026, PostGIS contributors
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose, without fee, and without a written
# agreement is hereby granted, provided that the above copyright notice and
# this paragraph and the following two paragraphs appear in all copies.
#
# IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
# DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
# LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
# EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
# OF SUCH DAMAGE.
#
# UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
# AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
# PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
#

# Run the regression suite with pg_regress. It creates a temporary instance,
# creates a database with the extensions of the build, runs the tests of the
# schedule and compares the output of each with its expected file. CMake
# configures this script with the modules of the build, see ../CMakeLists.txt.

# A script sets no policies of its own, and file(CREATE_LINK) dates from 3.14
cmake_minimum_required(VERSION 3.14)

set(PG_REGRESS "@PG_REGRESS@")
set(POSTGRESQL_BIN_DIR "@POSTGRESQL_BIN_DIR@")
set(SOURCE_DIR "@CMAKE_SOURCE_DIR@")
set(REGRESS_DIR "@MOBILITYDB_REGRESS_DIR@")
set(TEST_DIRS "@MOBILITYDB_TEST_DIRS@")
set(SETUPS "@MOBILITYDB_TEST_SETUPS@")
set(EXTENSIONS "@MOBILITYDB_TEST_EXTENSIONS@")
set(TEST_TMPDIR "@MOBILITYDB_TEST_TMPDIR@")
set(TEST_ENCODING "@MOBILITYDB_TEST_ENCODING@")

if(NOT EXISTS "${PG_REGRESS}")
  message(FATAL_ERROR "pg_regress is not at ${PG_REGRESS}. It is installed "
    "with the files for building extensions (PGXS), which some distributions "
    "package apart from the server, e.g., postgresql-server-devel.")
endif()

# pg_regress reads every test from one directory, so the tests of the modules
# are linked into it at each run, which keeps them those of the source tree.
# Where no link can be created, as on Windows without the privilege, they are
# copied.
file(REMOVE_RECURSE ${REGRESS_DIR}/sql ${REGRESS_DIR}/expected
  ${REGRESS_DIR}/results ${REGRESS_DIR}/exchange)
file(MAKE_DIRECTORY ${REGRESS_DIR}/sql ${REGRESS_DIR}/expected
  ${REGRESS_DIR}/exchange)
foreach(dir ${TEST_DIRS})
  file(GLOB files ${dir}/sql/*.sql ${dir}/expected/*.out)
  foreach(file ${files})
    get_filename_component(kind ${file} DIRECTORY)
    get_filename_component(kind ${kind} NAME)
    get_filename_component(name ${file} NAME)
    file(CREATE_LINK ${file} ${REGRESS_DIR}/${kind}/${name} SYMBOLIC COPY_ON_ERROR)
  endforeach()
endforeach()

# A regular expression in the environment variable MOBILITYDB_TESTS limits the
# run to the tests whose names match it, after the setup scripts, e.g.,
#   MOBILITYDB_TESTS=tcbuffer ctest -R regress
set(schedule ${REGRESS_DIR}/schedule)
if(DEFINED ENV{MOBILITYDB_TESTS})
  file(STRINGS ${schedule} lines)
  set(filtered "")
  foreach(line ${lines})
    string(REGEX REPLACE "^test: *" "" names "${line}")
    separate_arguments(names)
    set(keep "")
    foreach(name ${names})
      if(name IN_LIST SETUPS OR name MATCHES "$ENV{MOBILITYDB_TESTS}")
        list(APPEND keep ${name})
      endif()
    endforeach()
    if(keep)
      list(JOIN keep " " keep)
      string(APPEND filtered "test: ${keep}\n")
    endif()
  endforeach()
  set(schedule ${REGRESS_DIR}/schedule.filtered)
  file(WRITE ${schedule} "${filtered}")
endif()

# The setup scripts read the data files of their module in the source tree
set(ENV{MOBILITYDB_SOURCE_DIR} ${SOURCE_DIR})

set(load_extensions "")
foreach(ext ${EXTENSIONS})
  list(APPEND load_extensions --load-extension=${ext})
endforeach()

# Explanation of the parameters
# --temp-instance=<directory> Create a temporary instance there, which also
#   holds the log of its server
# --temp-config=<file> Add these settings to its postgresql.conf
# --no-locale --encoding=<encoding> The C locale, which the expected files are
#   written in, and the encoding MOBILITYDB_TEST_ENCODING names
# --load-extension=<extension> Create the extension in the test database. The
#   extensions are listed in the order of their dependencies, since pg_regress
#   creates them without CASCADE
# --schedule=<file> The tests and the groups of tests that run together
# The tests write the files they read back with \copy in the directory psql
# runs in, the working directory of pg_regress.
execute_process(
  COMMAND ${PG_REGRESS}
    --bindir=${POSTGRESQL_BIN_DIR}
    --temp-instance=${TEST_TMPDIR}
    --temp-config=${REGRESS_DIR}/regress.conf
    --inputdir=${REGRESS_DIR}
    --outputdir=${REGRESS_DIR}
    --no-locale --encoding=${TEST_ENCODING}
    ${load_extensions}
    --schedule=${schedule}
  WORKING_DIRECTORY ${REGRESS_DIR}/exchange
  RESULT_VARIABLE result
)

# A failure writes the differences to regression.diffs, which a CI log only
# shows if they are written here
if(result)
  if(EXISTS ${REGRESS_DIR}/regression.diffs)
    file(READ ${REGRESS_DIR}/regression.diffs diffs)
    message("${diffs}")
  endif()
  message(FATAL_ERROR "pg_regress failed, see ${REGRESS_DIR}/regression.diffs")
endif()
