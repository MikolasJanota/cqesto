/*
 * File:  options.h
 * Author:  mikolas
 * Created on:  Thu Jul 27 14:43:08 CEST 2023
 * Copyright (C) 2023, Mikolas Janota
 */
#pragma once
#include <string>
class Options {
  public:
    Options() {}

  public:
    std::string file_name;
    bool aig;
    bool simplify;
    bool flatten;
    bool unit;
    bool polarities;
    bool full;
    bool luby_restart;
    bool enumerate;
    int verbose;

    bool has_free = false;
};
