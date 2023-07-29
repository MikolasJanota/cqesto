#!/bin/bash
#
# File:  genNtest.sh
# Author:  mikolas
# Created on:  Fri Apr 5 12:31:58 WEST 2013
# Copyright (C) 2013, Mikolas Janota
#
if [[ $# != 3 ]] ; then
    echo "Usage: $0 <solver1> <solver2> <iters>"
    exit 100;
fi

S1=$1
S2=$2

LIMIT="ulimit -t 20"
# LIMIT=
mkdir -p bugs
N=$3
TOTAL_RES=0
UNSAT=0
while [[ $N > 0 ]]; do
  echo $N
  let N="${N} - 1"
  HS=`cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 8 | head -n 1`
  STAMP=`date +%s`_${HS}
  I=/tmp/gNt_${STAMP}.qcir
  ./genqcir.py 5 3 2 4 >$I
  chmod -w $I
  echo $STAMP
  ( $LIMIT ; /usr/bin/time -f"%U %S %e %M" bash -c "$S1 $I >/dev/null 2>/dev/null" 2>&1 )
  R1=$?
  ( $LIMIT ; /usr/bin/time -f"%U %S %e %M" bash -c "$S2 $I >/dev/null 2>/dev/null" 2>&1 )
  R2=$?
  chmod u+w $I
  if [ $R1 -eq $R2 ]; then
    echo "ec:"${R1}
    if [ $R1 -eq 20 ]; then let UNSAT=${UNSAT}+1; fi
    rm -f $I
  else
    mv -v $I ./bugs/bug_${STAMP}.qcir
    chmod -w ./bugs/bug_${STAMP}.qcir
    echo '++FAIL'
    echo "${S1}" ":" "${R1}"
    echo "${S2}" ":" "${R2}"
    echo '++FAIL'
    let TOTAL_RES="${TOTAL_RES}+1"
  fi
done
echo 'UNSAT:' ${UNSAT}
if [ $TOTAL_RES -eq 0 ]; then
  echo 'OK'
else
  echo 'FAIL', $TOTAL_RES
fi
exit $TOTAL_RES
