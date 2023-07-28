/*
 * File:  qesto_qcir_parser.cpp
 * Author:  mikolas
 * Created on:  Fri Jul 28 14:41:04 CEST 2023
 * Copyright (C) 2023, Mikolas Janota
 */

#include "qesto_qcir_parser.h"

namespace qesto {
void QestoQCIRParser::semantic_error(std::string msg) {
    std::cerr << "ERROR:" << d_ln << ": " << msg << std::endl;
    exit(100);
}

} // namespace qesto

