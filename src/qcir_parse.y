%{
#include <iostream>
#include <string>
#include <map>
#include <stdio.h>
#include <string.h>
#include <unordered_map>
#include "auxiliary.h"
#include "DataStructures.h"
#include "qtypes.h"
#include "Expressions.h"
#include "Visitors.h"
#include "VariableManager.h"
using namespace std;
using qesto::ID;
using qesto::ExpressionPrinter;
using qesto::Expressions;
using qesto::VariableManager;
using qesto::QFla;
using qesto::IDVector;
using SATSPC::Var;
using SATSPC::mkLit;

int yylex();
int qcir_line;  // incremented by the lexer
unordered_map<const char*,int,cstrHash,cstrEq> name2var;
unordered_map<const char*,ID,cstrHash,cstrEq> name2id;
void yyerror(const char *m);
void start_qcir_lex(const string& filename); // in lexer
static string filename("<ERR>");
Expressions *exf;
VariableManager qcir_vmng;
QFla qcir_qfla;
static bool output_gate_polarity;
static char* output_gate_str;

#define TI ID::toInt
#define FI ID::fromInt
std::vector<ID> qcir_ID_stack;
std::vector<Var> qcir_var_stack;

void semantic_error(string msg) {
  cerr<<filename<<":"<<qcir_line<<": "<<msg<<endl;
  exit(100);
}

int make_varid(const char* var_name) {
  const auto retv=qcir_vmng.new_var();
  const auto i=name2var.insert(std::pair<const char*,Var>(var_name,retv));
  if (!i.second) semantic_error("var redefinition '"+string(var_name)+"'");
  return retv;
}

ID define_gate(const char *s, ID def) {
  const auto i=name2id.insert( std::pair<const char*,ID>(s,def) );
  if (!i.second) semantic_error("gate redefinition");
  return def;
}

ID get_ID(const char* s, bool polarity) {
 const auto i=name2id.find(s);
 if(i!=name2id.end()) {
   const ID retv=i->second;
   return polarity ? retv : exf->make_not(retv);
  } else {
   const auto vi=name2var.find(s);
   if(vi==name2var.end()) semantic_error("undefined variable or gate '"+string(s)+"'");
   const Var v=vi->second;
   const ID retv=exf->make_lit(mkLit(v,!polarity));
   return retv;
 }
}

void set_output_gate() {
  const auto i=name2id.find(output_gate_str);
  if (i==name2id.end()) semantic_error("output gate not defined");
  qcir_qfla.output=output_gate_polarity?i->second:exf->make_not(i->second);
}

%}

//-- SYMBOL SEMANTIC VALUES -----------------------------
%defines
%name-prefix "qcir_"

%code requires {
}


%union {
  unsigned long long val;
  char* str;
}

%token <str> VAR_ID
%token FORMAT_ID NUM NL FORALL EXISTS XOR AND OR ITE OUTPUT
%token LP RP EQ SEMI PLUS MINUS COMMA
%token ERROR_TOKEN
%type <val> new_var
%type <val> lit
%type <val> quant
%type <val> gate_definition

//-- GRAMMAR RULES ---------------------------------------
%%

qcir_file: header qblocks output_stmt gates { set_output_gate(); }

nls: NL
   | nls NL
   ;

header : FORMAT_ID size nls { }
       ;

size:
    | NUM
    ;

qblocks:
       | qblocks qblock
       ;

qblock: quant LP var_list RP nls {
      const auto qt=($1)==0?qesto::EXISTENTIAL:qesto::UNIVERSAL;
      qcir_qfla.pref.push_back(qesto::Quantification(qt,qesto::VarVector(qcir_var_stack)));
      qcir_var_stack.clear();
      }
     ;

output_stmt : OUTPUT LP output_gate RP nls { };
output_gate: plus VAR_ID  {output_gate_str=$2;output_gate_polarity=true;}
           | MINUS VAR_ID {output_gate_str=$2;output_gate_polarity=false;}

gates:
  | gates gate
  ;

gate: VAR_ID EQ gate_definition {define_gate($1,FI($3));}
    ;

gate_definition:
      XOR LP lit COMMA lit RP nls {$$=TI(exf->make_and(
                                            exf->make_or(FI($3),FI($5)),
                                            exf->make_or(exf->make_not(FI($3)),exf->make_not(FI($5)))));
                                  }
    | AND LP lit_list RP nls      {$$=TI(exf->make_and(IDVector(qcir_ID_stack))); qcir_ID_stack.clear();}
    | OR LP lit_list RP nls      {$$=TI(exf->make_or(IDVector(qcir_ID_stack))); qcir_ID_stack.clear();}
    | ITE LP lit COMMA lit COMMA lit RP nls {
                                $$=TI(exf->make_or(exf->make_and(FI($3),FI($5)),
                                               exf->make_and(exf->make_not(FI($3)),FI($7))));
                                }
//    | quant LP var_list SEMI lit RP nls

quant: EXISTS  { $$=0; }
     | FORALL  { $$=1; }
     ;

var_list:
    | new_var                { qcir_var_stack.push_back($1); }
    | var_list COMMA new_var { qcir_var_stack.push_back($3); }
    ;

lit_list:
         | lit                  { qcir_ID_stack.push_back(FI($1)); }
         | lit_list COMMA lit   { qcir_ID_stack.push_back(FI($3)); }
         ;

plus:
    | PLUS
    ;

lit: plus VAR_ID   {$$=TI(get_ID($2,true));}
   | MINUS VAR_ID  {$$=TI(get_ID($2,false));}
    ;

new_var: VAR_ID { $$=make_varid($1); }

%%

void read_qcir(string _filename) {
  qcir_line=1;
  filename=_filename;
  start_qcir_lex(filename);
  qcir_parse();
}

void yyerror(const char *msg) {
  cerr<<filename<<":"<<qcir_line<<": "<<msg<<endl;
  exit(100);
}
