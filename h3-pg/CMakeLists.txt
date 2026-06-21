# Copyright 2022 Zacharias Knudsen
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Almost all CMake files should start with this
# You should always specify a range with the newest
# and oldest tested versions of CMake. This will ensure
# you pick up the best policies.

# Keep minimum version in sync with:
# https://github.com/uber/h3/blob/master/CMakeLists.txt
cmake_minimum_required(VERSION 3.20..3.24)

if(POLICY CMP0135)
  # Set the timestamps of all extracted contents to the time of the extraction
  cmake_policy(SET CMP0135 NEW)
endif()

# This is your project statement. You should always list languages;
# Listing the version is nice here since it sets lots of useful variables
project(
  h3-pg
  VERSION 4.2.3
  LANGUAGES C
)
# set this to "${PROJECT_VERSION}" on release
set(INSTALL_VERSION "${PROJECT_VERSION}")
#set(INSTALL_VERSION "unreleased")
set(H3_CORE_VERSION 4.2.0)
set(H3_CORE_SHA256 438db46fc2b388785d2a0d8e26aa5509739240a7b50b2510c416778d871a4e11)

# If you set any CMAKE_ variables, that can go here.
# (But usually don't do this, except maybe for C++ standard)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Allow forcing PostgreSQL version
set(POSTGRESQL_VERSION $ENV{POSTGRESQL_VERSION} CACHE STRING "PostgreSQL version major")

# Find packages go here.
find_package(PostgreSQL ${POSTGRESQL_VERSION} REQUIRED
  OPTIONAL_COMPONENTS PostGIS
)
include(AddPostgreSQLExtension)

add_definitions(-DPOSTGRESQL_VERSION_MAJOR=${PostgreSQL_VERSION_MAJOR})

# https://cliutils.gitlab.io/modern-cmake/chapters/testing.html
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include(CTest)
endif()

# Include core library
add_subdirectory(cmake/h3)

# Include extensions and their shared code
add_subdirectory(include)
add_subdirectory(h3)
add_subdirectory(h3_postgis)

# Add target that bundles for pgxn
configure_file(META.json.in META.json)
add_custom_target(pgxn
  COMMAND git archive --format zip
                      --prefix="h3-${INSTALL_VERSION}/"
                      --add-file ${CMAKE_BINARY_DIR}/META.json
                      -o h3-${INSTALL_VERSION}.zip HEAD
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
