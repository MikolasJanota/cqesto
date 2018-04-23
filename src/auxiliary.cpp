/*
 * File:  auxiliary.cc
 * Author:  mikolas
 * Created on:  Wed, Dec 03, 2014 4:00:16 PM
 * Copyright (C) 2014, Mikolas Janota
 */
#include <math.h>
double luby(double y, int x){
    // Find the finite subsequence that contains index 'x', and the
    // size of that subsequence:
    int size, seq;
    for (size = 1, seq = 0; size < x+1; seq++, size = 2*size+1);

    while (size-1 != x){
        size = (size-1)>>1;
        seq--;
        x = x % size;
    }

    return pow(y, seq);
}
