/*
 * File:  stream_buffer.h
 * Author:  mikolas
 * Created on:  Sat Jul 29 18:49:31 CEST 2023
 * Copyright (C) 2023, Mikolas Janota
 */
#pragma once

/* this is adapted from minisat's parser */

#define CHUNK_LIMIT 1048576

#include <cstdio>
#include <zlib.h>
class StreamBuffer {
  protected:
    gzFile in;
    char *buf;
    size_t pos;
    size_t size;

    void assureLookahead() {
        if (pos >= size) {
            pos = 0;
            size = gzread(in, buf, sizeof(buf));
        }
    }

  public:
    explicit StreamBuffer(gzFile i) : in(i), pos(0), size(0) {
        buf = new char[CHUNK_LIMIT];
        assureLookahead();
    }

    virtual ~StreamBuffer() { delete[] buf; }

    int operator*() { return (pos >= size) ? EOF : buf[pos]; }
    void operator++() {
        pos++;
        assureLookahead();
    }
};
