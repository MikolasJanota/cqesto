#!/bin/bash
#
# File:  setmeup.sh
# Author:  mikolas
# Created on:  Thu Nov 28 10:50:48 WET 2019
# Copyright (C) 2019, Mikolas Janota
#

set -e
mkdir cadical 
cd cadical 
git clone https://github.com/arminbiere/cadical.git
cd cadical 
./configure
make -j4
cd -
ln -s cadical/build/libcadical.a .
ln -s cadical/src/ipasir.h .
