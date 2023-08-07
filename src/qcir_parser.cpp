/*
 * File:  qcir_parser.cpp
 * Author:  mikolas
 * Created on:  Fri Jul 28 09:10:49 CEST 2023
 * Copyright (C) 2023, Mikolas Janota
 */
#include "qcir_parser.h"
#include "auxiliary.h"
#include "stream_buffer.h"
#include <cassert>
#include <cctype>   // for isalnum
#include <cstdlib>  // for exit
#include <iostream> // for cerr

void QCIRParser::parse() { qcir_file(); }
void QCIRParser::qcir_file() {
    format_id();
    qblock_prefix();
    output_stmt();
    while (!is_end()) {
        gate_stmt();
        nltoken();
    }
    cb_file_closed();
}

void QCIRParser::format_id() {
    d_found_header = false;
    // even though it's not permitted in the format, we try to avoid comments
    // before the header, we also allow files without headers, because this is
    // apparently the practice
    while (!d_found_header && *d_buf == '#') {
        match_char('#');
        if (*d_buf == 'Q') {
            match_string("qcir-", false);
            if (*d_buf == 'g' or *d_buf == 'G')
                ++d_buf;
            match_string("14", false);
            d_found_header = true;
        }
        skip_line();
    }
}

void QCIRParser::skip_line() {
    while (*d_buf != '\n' && *d_buf != '\r' && *d_buf != EOF)
        ++d_buf;
    nlchar();
}

void QCIRParser::output_stmt() {
    match_string("output");
    match_char_token('(');
    cb_output_lit(lit());
    match_char_token(')');
    nltoken();
}

void QCIRParser::gate_stmt() {
    cb_gate_stmt_var(var());
    match_char_token('=');
    switch (next()) {
    case 'a':
    case 'A':
        match_string("and");
        cb_gate_stmt_gt(GType::AND);
        break;
    case 'o':
    case 'O':
        match_string("or");
        cb_gate_stmt_gt(GType::OR);
        break;
    case 'x':
    case 'X':
        match_string("xor");
        cb_gate_stmt_gt(GType::XOR);
        break;
    case 'i':
    case 'I':
        match_string("ite");
        cb_gate_stmt_gt(GType::ITE);
        break;
    default:
        err() << "expecting gate operator at '" << static_cast<char>(*d_buf)
              << "'" << std::endl;
        exit(1);
    }
    match_char_token('(');
    while (*d_buf != ')') {
        cb_gate_stmt_lit(lit());
        skip();
        if (*d_buf == ',') {
            match_char_token(',');
            skip();
        }
    }
    match_char_token(')');
    cb_gate_closed();
}

void QCIRParser::qblock_prefix() {
    free_quant();
    while (!is_end()) {
        if (!qblock_quant())
            break;
    }
}

void QCIRParser::var_list() {
    match_char_token('(');
    while (next() != ')') {
        cb_qblock_var(var());
        if (next() != ')')
            match_char_token(',');
    }
    match_char_token(')');
    nltoken();
}

bool QCIRParser::free_quant() {
    skip();
    if (*d_buf != 'f' && *d_buf != 'F')
        return false;
    match_string("free");
    cb_qblock_quant(QType::FREE);
    var_list();
    cb_quant_closed();
    return true;
}

bool QCIRParser::qblock_quant() {
    skip();
    switch (*d_buf) {
    case 'e':
    case 'E':
        match_string("exists");
        cb_qblock_quant(QType::EXIST);
        break;
    case 'f':
    case 'F':
        match_string("forall");
        cb_qblock_quant(QType::FORALL);
        break;
    default: return false;
    }
    var_list();
    cb_quant_closed();
    return true;
}

static inline bool is_var_char(char c) { return isalnum(c) || c == '_'; }

std::string QCIRParser::var() {
    d_varbuf.str(std::string());
    d_varbuf.clear();
    if (!is_var_char(next())) {
        err() << "expecting variable, found '" << static_cast<char>(*d_buf)
              << "'" << std::endl;
        exit(1);
    }
    while (is_var_char(*d_buf)) {
        d_varbuf.put(static_cast<char>(*d_buf));
        ++d_buf;
    }
    return d_varbuf.str();
}

void QCIRParser::lit_list() { skip(); }

QCIRParser::Lit QCIRParser::lit() {
    const bool sign = next() == '-';
    if (sign)
        ++d_buf;
    return {sign, var()};
}

void QCIRParser::match_string(const char *s, bool run_skip) {
    if (run_skip)
        skip();
    const auto olds = s;
    for (; *s; ++d_buf, s++) {
        if (*d_buf == EOF) {
            err() << "End of file when looking for '" << olds << "'."
                  << std::endl;
            exit(1);
        }
        char rc = *d_buf;
        if ('A' <= rc && rc <= 'Z')
            rc = (rc - 'A') + 'a';
        if (rc != *s) {
            err() << "Unexpected character '" << rc << "' when looking for '"
                  << *s << "' in " << olds << "'." << std::endl;
            exit(1);
        }
    }
}

void QCIRParser::match_char(char c) {
    if (*d_buf != c) {
        err() << "'" << c << "' expected instead of '"
              << static_cast<char>(*d_buf) << "'" << std::endl;
        exit(1);
    }
    ++d_buf;
}

void QCIRParser::match_char_token(char c) {
    skip();
    match_char(c);
}

std::ostream &QCIRParser::err() {
    if (d_filename.empty())
        return std::cerr << "ERROR on line " << d_ln << ":";
    return std::cerr << d_filename << ":" << d_ln << ":";
}

int QCIRParser::skip() {
    int nls = 0;
    bool comment = false;
    for (bool stop = false; !stop;) {
        switch (*d_buf) {
        case EOF: stop = true; break;
        case ' ':
        case '\t': ++d_buf; break;
        case '\r':
        case '\n':
            if (comment) {
                nlchar();
                comment = false;
                nls++;
            } else
                stop = true;
            break;
        case '#':
            comment = true;
            ++d_buf;
            break;
        default:
            if (comment)
                ++d_buf;
            else
                stop = true;
        }
    }
    return nls;
}

void QCIRParser::nltoken() {
    if (skip() > 0)
        return;
    switch (*d_buf) {
    case EOF: break;
    case '\r':
    case '\n': nlchar(); break;
    default:
        err() << "new line expected instead of '" << static_cast<char>(*d_buf)
              << "'" << std::endl;
        exit(1);
    }
}

void QCIRParser::nlchar() {
    switch (*d_buf) {
    case '\n': ++d_buf; break;
    case '\r':
        ++d_buf;
        if (*d_buf == '\n')
            ++d_buf;
        break;
    default: assert(false);
    }
    d_ln++;
}

