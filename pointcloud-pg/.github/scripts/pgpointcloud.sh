#! /bin/bash

set -e

sh ./tools/build_install.sh
sh ./tools/install_lazperf.sh
sh ./tools/build_install.sh --with-lazperf=/usr/local
make check
sh ./tools/valgrind.sh
make installcheck
