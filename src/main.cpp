/*
 * File:  main.cpp
 * Author:  mikolas
 * Created on:  Fri, May 15, 2015 2:40:53 PM
 * Copyright (C) 2015, Mikolas Janota
 */
#include <signal.h>
#include <string>
#include "qcir_parse.tab.h"
#include "Expressions.h"
#include "VariableManager.h"
#include "Visitors.h"
#include "qtypes.h"
#include "ZigZag.h"
#include "Options.h"
#include "defs.h"
using namespace std;
void read_qcir(string filename);
using qesto::ZigZag;
extern qesto::Expressions* exf;
extern qesto::QFla qcir_qfla;
extern qesto::VariableManager qcir_vmng;
extern unordered_map<const char*,int,cstrHash,cstrEq> name2var;
namespace qesto { NiceExpressionPrinter* dprn; }

ZigZag* ps=NULL;
static void SIG_handler(int signum);
ostream& print_usage(const Options& options,ostream& o);

unordered_map<int,const char*> var2name;
int main(int argc, char** argv) {
#ifndef NDEBUG
  cout << "c DEBUG version." << endl;
#endif
#ifdef STATIC
  cout << "c STATIC version." << endl;
#else
  cout << "c DYNLINK version." << endl;
#endif
#ifndef __MINGW32__
  signal(SIGHUP, SIG_handler);
  signal(SIGUSR1, SIG_handler);
#else
  cout << "c MINGW version." << endl;
#endif
  cout<<"c cqesto, v00.0, "<<GITHEAD<<endl;
  signal(SIGTERM, SIG_handler);
  signal(SIGINT, SIG_handler);
  signal(SIGABRT, SIG_handler);
#ifndef EXPERT
  // prepare nonexpert options
  const int nargc = 3;
  char* nargv[nargc];
  nargv[0] = argv[0];
  nargv[1] = strdup("-es");
  nargv[2] = argc>=2 ? argv[1] : strdup("-");
  if (argc>2) {
    cerr<<"ERROR: ingoring some options after FILENAME"<<std::endl;
    return 100;
  }
  argv=nargv;
  argc=nargc;
#else
  cout<<"c WARNING: running in the EXPERT mode, I'm very stupid without any options."<<std::endl;
#endif
  Options options;
  if (!options.parse(argc, argv)) {
    cerr << "ERROR: processing options." << endl;
    print_usage(options,cerr);
    return 100;
  }
  auto& rest = options.get_rest();
  if (rest.size()>1)
    cerr<<"WARNING: garbage at the end of command line."<<endl;

  if (options.get_help()){
    print_usage(options,cout);
    return 0;
  }

  const string flafile(rest.size() ? rest[0] : "-");
  exf=new qesto::Expressions(options);
  read_qcir(flafile);
  FOR_EACH(i,name2var)var2name[i->second]=i->first;
  qesto::dprn=new qesto::NiceExpressionPrinter(*exf,var2name,cerr);
  ps=new ZigZag(options,*exf,qcir_qfla);
  const bool r=ps->solve();
  std::cout<<"c solved "<<read_cpu_time()<<std::endl;
  ps->print_stats(cerr);
  std::cout<<"s cnf "<<(r?'1':'0')<<std::endl;
  exit(r ? 10 : 20);
  return r ? 10 : 20;
}

static void SIG_handler(int signum) {
  if(ps) ps->print_stats(cerr);
  else cerr<<"c Solver not yet initialized."<<endl;
  cerr<<"# received external signal " << signum << endl;
  cerr<<"Terminating ..." << endl;
  exit(0);
}

ostream& print_usage(const Options& options,ostream& o) {
  o << "USAGE: [OPTIONS] [FILENAME]" << endl;
  return options.print(o);
}
