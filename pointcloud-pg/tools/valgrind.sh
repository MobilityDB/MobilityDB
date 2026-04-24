#!/bin/bash

set -e

valgrind --leak-check=full --error-exitcode=1 lib/cunit/cu_tester
