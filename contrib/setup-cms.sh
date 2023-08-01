#!/bin/bash
#
# File:  setup-cms.sh
# Author:  mikolas
# Created on:  Thu Nov 28 10:50:48 WET 2019
# Copyright (C) 2019, Mikolas Janota
#

set -e
static=no

usage () {
cat <<EOF
usage: $0 [-h|--help][-s|--static]

-h | --help    print this command line option summary
-s | --static compile static
EOF
}

msg () {
  echo "[setup-cms] $*"
}

while [ $# -gt 0 ]
do
  case $1 in
    -h|--help) usage; exit 0;;
    -s|--static) static=yes;;
    *) die "invalid option '$1' (try '-h')";;
  esac
  shift
done

CMSFLAGS="-DONLY_SIMPLE=ON -DNOZLIB=ON -DSTATS=OFF -DNOVALGRIND=ON -DENABLE_TESTING=OFF"

CONTRIB_ROOT=`pwd`
CMSROOT=${CONTRIB_ROOT}/cms
if [ $static = yes ]
then
  msg "static compilation"
  CMSFLAGS="$CMSFLAGS -DSTATICCOMPILE=ON"
  CMSROOT=${CMSROOT}_static
else
  CMSROOT=${CMSROOT}_dyn
fi

if [ -d $CMSROOT ]; then
  msg "$CMSROOT directory exists, assuming that cms is already set up"
  cd $CONTRIB_ROOT
  rm -f cms
  ln -fvs ${CMSROOT} cms
  exit 0
fi

mkdir $CMSROOT
cd $CMSROOT
VER=5.11.11
wget https://github.com/msoos/cryptominisat/archive/refs/tags/${VER}.tar.gz
tar xzvf ${VER}.tar.gz
cd cryptominisat-${VER}
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=${CMSROOT} ${CMSFLAGS} ..
make -j4
make install
cd $CONTRIB_ROOT
rm -f cms
ln -vs ${CMSROOT} cms
