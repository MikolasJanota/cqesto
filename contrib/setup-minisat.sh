#!/bin/bash
#
# File:  setup-minisat.sh
# Author:  mikolas
# Created on:  Wed Dec 21 17:13:24 CET 2022
# Copyright (C) 2022, Mikolas Janota
#

set -e
mkdir minisat 
cd minisat 
MDIR=`pwd`
git clone https://github.com/agurfinkel/minisat.git
cd minisat 
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=${MDIR} ..
make -j4
make install
