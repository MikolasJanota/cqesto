#!/bin/bash
#
# File:  setup-cms.sh
# Author:  mikolas
# Created on:  Thu Nov 28 10:50:48 WET 2019
# Copyright (C) 2019, Mikolas Janota
#

set -e
mkdir cms
cd cms
VER=5.11.11
CMSDIR=`pwd`
wget https://github.com/msoos/cryptominisat/archive/refs/tags/${VER}.tar.gz
tar xzvf ${VER}.tar.gz
ln -s cryptominisat-${VER} cms
cd cms
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=${CMSDIR} \
  -DSTATICCOMPILE=ON \
  -DIPASIR=ON \
  -DONLY_SIMPLE=ON -DNOZLIB=ON -DSTATS=OFF -DNOVALGRIND=ON -DENABLE_TESTING=OFF \
  ..
make -j4
make install
cd ..
cp ${CMSDIR}/cms/src/ipasir.h ${CMSDIR}/include/
