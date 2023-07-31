#!/bin/bash
#
# File:  build_configs.sh
# Author:  mikolas
# Created on:  Thu Nov 28 11:12:06 WET 2019
# Copyright (C) 2019, Mikolas Janota
#
program_name=cqesto

set -e

for g in '-g' '' ; do
    for s in '-s' '' ; do
        for sat in '-r' '-c' ''; do
            if  [[ ! -z "${s}" && ! -z "${g}"  ]]; then 
              continue
            fi
            echo '===== Trying' ${sat} ${g} ${s}
            # oname=${program_name}_${sat}
            # if [[ ! -z "${s}" ]]; then oname=${oname}'_static'; fi
            # if [[ ! -z "${g}" ]]; then oname=${oname}'_dbg'; fi
            ./configure $g $s $sat 
            cd build
            make -j 4
            if [ -s ${program_name} ]
            then
                echo "===== Compiled: ${oname}"
            fi
            rm -fv ${program_name}
            cd ..
        done
    done
done
