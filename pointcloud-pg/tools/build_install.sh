#!/bin/bash

set -e

if [[ -f config.mk ]]; then
    make clean maintainer-clean
fi

./autogen.sh
./configure CFLAGS="-Wall -Werror -O2 -g" $@
make
sudo make install
