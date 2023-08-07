/*
 * File:  qcir_parser.h
 * Author:  mikolas
 * Created on:  Fri Jul 28 08:58:20 CEST 2023
 * Copyright (C) 2023, Mikolas Janota
 */
#pragma once
#include "stream_buffer.h" // for StreamBuffer
#include <cstddef>         // for size_t
#include <sstream>         // for ostream, stringstream
#include <string>          // for string, basic_string
#include <utility>         // for pair
class QCIRParser {
  public:
    QCIRParser(StreamBuffer &buf) : d_buf(buf) {}
    void parse();
    std::string d_filename; // only err reporting
    bool found_header() const { return d_found_header; }

  protected:
    size_t d_ln = 1;
    bool d_found_header = false;
    enum QType { FORALL, EXIST, FREE };
    enum GType { AND, OR, XOR, ITE };
    typedef std::pair<bool, std::string> Lit;
    StreamBuffer &d_buf;
    bool is_end() { return next() == EOF; }
    virtual void cb_qblock_quant(QType qt) = 0;
    virtual void cb_qblock_var(std::string v) = 0;
    virtual void cb_gate_stmt_gt(GType gt) = 0;
    virtual void cb_gate_stmt_var(const std::string &v) = 0;
    virtual void cb_gate_stmt_lit(const Lit &l) = 0;
    virtual void cb_output_lit(const Lit &l) = 0;
    virtual void cb_gate_closed() = 0;
    virtual void cb_quant_closed() = 0;
    virtual void cb_file_closed() = 0;

    void qcir_file();
    void output_stmt();
    void gate_stmt();
    void format_id();
    void qblock_prefix();
    bool qblock_quant();
    bool free_quant();
    void var_list();
    std::string var();
    void lit_list();
    Lit lit();
    void nltoken();
    void nlchar();
    int skip();
    void match_string(const char *s, bool run_skip = true);
    void match_char_token(char c);
    void match_char(char c);
    int next() {
        skip();
        return *d_buf;
    }
    void skip_line();
    std::ostream &err();
    std::stringstream d_varbuf;
};
