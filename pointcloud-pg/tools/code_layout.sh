#! /bin/bash

clang-format -i -style=file pgsql/*.c pgsql/*.h lib/*.c lib/*.h lib/cunit/*.c lib/*.cpp lib/*.hpp lib/cunit/*.h
