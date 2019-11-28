#!/bin/bash
#
# File:  build_configs.sh
# Author:  mikolas
# Created on:  Thu Nov 28 11:12:06 WET 2019
# Copyright (C) 2019, Mikolas Janota
#
if [ -s makeenv ]
then
        echo 'this should not be run if you have something in makeenv'
        exit 1
fi

program_name=cqesto

for d in 'DBG=1' '' ; do
    for s in 'STATIC=1' '' ; do
        for sat in 'USE_IPASIR' 'USE_MINISAT'; do
            echo '===== Trying' ${sat} ${s}
            oname=${program_name}_${sat}
            if [[ ! -z "${s}" ]]; then oname=${oname}'_static'; fi
            if [[ ! -z "${d}" ]]; then oname=${oname}'_dbg'; fi
            make ${s} ${d} ${sat}'=1' clean all && mv ${program_name} ${oname}
            if [ -s ${oname} ]
            then
                echo "===== Compiled: ${oname}"
            fi
        done
    done
done
