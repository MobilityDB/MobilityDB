#!/bin/bash

export PATH=/usr/lib/postgresql/14/bin:$PATH

sudo service postgresql stop
echo "=== Building MobilityDB ==="
cd ..
rm -rf build
mkdir build
cd build
cmake ..
make -j$(nproc)  # Compilation rapide avec tous les coeurs CPU
sudo make install

cd ..
cd 'danish_ais_scripts'
./script_compil2.sh