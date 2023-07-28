/*
 * File:  qcir_parser.cpp
 * Author:  mikolas
 * Created on:  Fri Jul 28 09:10:49 CEST 2023
 * Copyright (C) 2023, Mikolas Janota
 */
#include "qcir_parser.h"
#include "auxiliary.h"
#include "fmtutils.hh"
#include <cctype>

void QCIRParser::parse() { qcir_file(); }
void QCIRParser::qcir_file() {
    format_id();
    skip();
    __PL;
    qblock_prefix();
    __PL;
    output_stmt();
    __PL;
    while (!is_end()) {
        gate_stmt();
        nl();
    }
    cb_file_closed();
}

void QCIRParser::format_id() {
    match_string("#QCIR-14");
    nl();
}

void QCIRParser::output_stmt() {
    match_string("output");
    match_char('(');
    cb_output_lit(lit());
    match_char(')');
    nl();
}

void QCIRParser::gate_stmt() {
    cb_gate_stmt_var(var());
    match_char('=');
    switch (*d_buf) {
    case 'a':
        match_string("and");
        cb_gate_stmt_gt(GType::AND);
        break;
    case 'o':
        match_string("or");
        cb_gate_stmt_gt(GType::OR);
        break;
    case 'x':
        match_string("xor");
        cb_gate_stmt_gt(GType::XOR);
        break;
    case 'i':
        match_string("ite");
        cb_gate_stmt_gt(GType::ITE);
        break;
    default:
        err() << "expecting gate operator" << std::endl;
        exit(EXIT_FAILURE);
    }
    match_char('(');
    while (*d_buf != ')') {
        cb_gate_stmt_lit(lit());
        skip();
        if (*d_buf == ',') {
            match_char(',');
            skip();
        }
    }
    match_char(')');
    cb_gate_closed();
}

void QCIRParser::qblock_prefix() {
    while (!is_end()) {
        if (!qblock_quant())
            break;
    }
}

bool QCIRParser::qblock_quant() {
    skip();

    switch (*d_buf) {
    case 'e':
        match_string("exists");
        cb_qblock_quant(QType::EXIST);
        break;
    case 'f':
        match_string("forall");
        cb_qblock_quant(QType::FORALL);
        break;
    default: return false;
    }
    match_char('(');
    while (true) {
        cb_qblock_var(var());
        skip();
        if (*d_buf != ',')
            break;
    }
    match_char(')');
    nl();
    cb_quant_closed();
    return true;
}

static inline bool is_var_char(char c) { return isalnum(c) || c == '_'; }

std::string QCIRParser::var() {
    skip();
    std::stringstream buf;
    if (!is_var_char(*d_buf)) {
        err() << "expecting variable, found '" << static_cast<char>(*d_buf)
              << "'" << std::endl;
        exit(EXIT_FAILURE);
    }
    while (is_var_char(*d_buf)) {
        buf << static_cast<char>(*d_buf);
        ++d_buf;
    }
    return buf.str();
}

void QCIRParser::lit_list() { skip(); }

QCIRParser::Lit QCIRParser::lit() {
    skip();
    const bool sign = *d_buf == '-';
    if (sign)
        ++d_buf;
    return {sign, var()};
}

void QCIRParser::nl() {
    skip();
    switch (*d_buf) {
    case '\n': ++d_buf; break;
    case '\r':
        ++d_buf;
        if (*d_buf == '\n')
            ++d_buf;
        break;
    default: err() << "expecting" << std::endl; exit(EXIT_FAILURE);
    }
    d_ln++;
}

void QCIRParser::match_string(const char *s) {
    skip(s[0] != '#');
    const auto olds = s;
    for (; *s; ++d_buf, s++) {
        if (*d_buf == EOF) {
            err() << "End of file when looking for '" << olds << "'."
                  << std::endl;
            exit(EXIT_FAILURE);
        }
        const char rc = *d_buf;
        if (rc != *s) {
            err() << "Unexpected character '" << rc << "' when looking for '"
                  << *s << "' in " << olds << "'." << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

void QCIRParser::match_char(char c) {
    skip();
    if (*d_buf != c) {
        std::cerr << c << " expected" << std::endl;
        exit(EXIT_FAILURE);
    }
    ++d_buf;
}
std::ostream &QCIRParser::err() { return std::cerr << "ERROR:" << d_ln << ":"; }

void QCIRParser::skip_end_of_line() {
    while (true) {
        if (*d_buf == EOF)
            return;
        if (*d_buf == '\r' || *d_buf == '\n') {
            nl();
            return;
        }
        ++d_buf;
    }
}

void QCIRParser::skip(bool skipComments) {
    while (1) {
        skipTrueWhitespace(d_buf);
        if (!skipComments)
            return;
        if (*d_buf == '#')
            skipLine(d_buf);
        else
            break;
    }
}

