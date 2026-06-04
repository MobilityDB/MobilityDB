cmake_minimum_required(VERSION 3.13)
project(pdal-mobilitydb-tpcpatch CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(PDAL 2.6 REQUIRED CONFIG)
find_package(PostgreSQL REQUIRED)
find_package(LibXml2 REQUIRED)

add_subdirectory(reader)
add_subdirectory(writer)
