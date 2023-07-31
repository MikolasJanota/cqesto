#!/usr/bin/env python3
# File:  genqcir.py
# Author:  mikolas
# Created on:  Sat Jul 29 12:36:05 CEST 2023
# Copyright (C) 2023, Mikolas Janota
import sys, random
NT_CNT=5
(LITERAL,AND,OR,XOR,ITE)=range(NT_CNT)

false_n = 0
defs = []
gates = []

def define(v, s):
    global defs
    defs.append((v,s))
    return v

def make_false():
    global false_n
    if false_n != 0:
        return false_n
    false_n = new_id()
    return define(false_n, 'or()')

def new_id():
    global last_id
    last_id += 1
    return last_id

def mkLit(free, used):
   assert(free)
   v = random.choice(free)
   used.add(v)
   return v if (random.random()<0.5) else -v

def gn(free, used, mxd) :
    global MXDEPTH,MNDEPTH,defs
    if not free:
        return make_false()
    if mxd < MNDEPTH:
        # ntypes = [AND,OR,XOR,ITE]
        ntypes = [AND,OR]
        if mxd >= MNDEPTH:
            ntypes.append(LITERAL)
    else:
        ntypes = [LITERAL]
    ntype = random.choice(ntypes)
    if ntype==LITERAL:
        return mkLit(free, used)
    arg_count = {AND:3,OR:3,XOR:2,ITE:3}[ntype]
    name={AND:'and',OR:'or',XOR:'xor',ITE:'ite'}[ntype]
    lits = [gn(free, used, mxd+1) for _ in range(arg_count)]
    retv = new_id()
    arg = ','.join(map(str,lits))
    return define(retv, f'{name}({arg})')



def main():
    global last_id, N, MAXQLEV, MNDEPTH, MXDEPTH
    if len(sys.argv)!=5:
        print('Unexpected number of options!')
        print('USAGE: number-of-input-variables qlevs minimum-depth maximum-depth')
        sys.exit(100)
    N, MAXQLEV, MNDEPTH, MXDEPTH = map(int, sys.argv[1:5])
    if MNDEPTH > MXDEPTH:
        sys.exit(100)
        print('ERROR: minimum depth must be less than or equal to the maximum depth!')

    last_id = N
    free = range(1, N+1)
    used = set()
    output = gn(free, used, 0)
    print('#QCIR-14')
    print('# LastVar', max(used) if used else 0)
    prefix=[[] for _ in range(MAXQLEV)]
    for var in used:
        prefix[random.randint(0,MAXQLEV-1)].append(var)
    ex = True
    for quant in prefix:
        qtype = 'exists' if ex else 'forall'
        qvars = ','.join(map(str,quant))
        print(f'{qtype}({qvars})')
        ex = not ex
    print(f'output({output})')
    for v,s in defs:
        print(v,'=', s)

if __name__ == "__main__":
    sys.exit(main())
