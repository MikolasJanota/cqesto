/*
 * File:  main.cpp
 * Author:  mikolas
 * Created on:  Fri, May 15, 2015 2:40:53 PM
 * Copyright (C) 2015, Mikolas Janota
 */
#include "CLI11.hpp"
#include "expressions.h"
#include "options.h"
#include "qesto_qcir_parser.h"
#include "qtypes.h"
#include "variable_manager.h"
#include "version.h"
#include "visitor.h"
#include "zigzag.h"
#include <signal.h>
#include <string>
using namespace std;

qesto::ZigZag *ps = NULL;
static void SIG_handler(int signum);

unordered_map<int, std::string> var2name;
int main(int argc, char **argv) {
#ifndef NDEBUG
    cout << "c DEBUG version." << endl;
#endif
#ifdef STATICLN
    cout << "c STATIC version." << endl;
#else
    cout << "c DYNLINK version." << endl;
#endif

#ifdef USE_IPASIR
    cout << "c solver IPASIR (cadical)" << endl;
#endif /*USE_IPASIR*/
#ifdef USE_MINISAT
    cout << "c solver MINISAT" << endl;
#endif /*USE_MINISAT*/

#ifndef __MINGW32__
    signal(SIGHUP, SIG_handler);
    signal(SIGUSR1, SIG_handler);
#else
    cout << "c MINGW version." << endl;
#endif
    cout << "c cqesto, v00.0, " << Version::GIT_SHA1 << ", "
         << Version::GIT_DATE << endl;
    cout << "c (C) 2015 Mikolas Janota, mikolas.janota@gmail.com" << endl;
    signal(SIGTERM, SIG_handler);
    signal(SIGINT, SIG_handler);
    signal(SIGABRT, SIG_handler);

    CLI::App app("cqesto non-CNF QBF solver.");
    Options options;
    app.add_option("file_name", options.file_name,
                   "Input file name, use - or empty for stdin.")
        ->default_val("-");
    app.add_flag("-a, !--no-a", options.aig, "Use only AND gates.")
        ->default_val(false);
    app.add_flag("-s, !--no-s", options.simplify, "Use simplification.")
        ->default_val(true);
    app.add_flag("-u, !--no-u", options.unit, "Use unit propagation.")
        ->default_val(true);
    app.add_flag("-f, !--no-f", options.flatten, "Use flattening.")
        ->default_val(false);
    app.add_flag("-l, !--no-l", options.luby_restart, "Use  luby restarts.")
        ->default_val(false);
    app.add_flag("-p, !--no-p", options.polarities, "Set variable polarities.")
        ->default_val(false);
    app.add_flag("-e, !--no-e", options.full,
                 "Initialize all abstractions with the input formula right at "
                 "the beginning.")
        ->default_val(true);
    app.add_flag("-v", options.verbose, "Add verbosity.")->default_val(0);
    CLI11_PARSE(app, argc, argv);

    const bool use_std = options.file_name == "-";
    gzFile in =
        use_std ? gzdopen(0, "rb") : gzopen(options.file_name.c_str(), "rb");
    if (in == nullptr) {
        cerr << "ERROR! Could not open file: " << options.file_name << endl;
        exit(EXIT_FAILURE);
    }

    using qesto::ZigZag;
    qesto::Expressions factory(options);
    StreamBuffer buf(in);
    qesto::QestoQCIRParser parser(buf, factory);
    if (options.file_name != "-")
        parser.d_filename = options.file_name;
    parser.parse();

    for (auto i : parser.name2var())
        var2name[i.second] = i.first;
    qesto::NiceExpressionPrinter *dprn =
        new qesto::NiceExpressionPrinter(factory, var2name, cerr);
    ps = new qesto::ZigZag(options, factory, parser.formula());
    ps->dprn = dprn;
    const bool r = ps->solve();
    std::cout << "c solved " << read_cpu_time() << std::endl;
    ps->print_stats(cerr);
    std::cout << "s cnf " << (r ? '1' : '0') << std::endl;
#ifndef NDEBUG
    delete ps;
    delete dprn;
#endif
    exit(r ? 10 : 20);
    return r ? 10 : 20;
}

static void SIG_handler(int signum) {
    if (ps)
        ps->print_stats(cerr);
    else
        cerr << "c Solver not yet initialized." << endl;
    cerr << "# received external signal " << signum << endl;
    cerr << "Terminating ..." << endl;
    exit(0);
}
