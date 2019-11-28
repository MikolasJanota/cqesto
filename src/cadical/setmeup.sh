#!/bin/bash
#
# File:  setmeup.sh
# Author:  mikolas
# Created on:  Thu Nov 28 10:50:48 WET 2019
# Copyright (C) 2019, Mikolas Janota
#
git clone https://github.com/arminbiere/cadical.git
cd cadical && ./configure\
	&& make\
	&& cd -\
	&& ln -s cadical/build/libcadical.a .\
	&& ln -s cadical/src/ipasir.h .
