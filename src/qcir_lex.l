%{
#include "qcir_parse.tab.h"
#include <iostream>
#include <zlib.h>
using namespace std;
void start_qcir_lex(const string& filename);

#define VAL qcir_lval
#define YY_INPUT(buf, res, size) (res = yy_input(buf, size))

extern int qcir_line;

typedef int yy_input_t(char* buf, int size);

yy_input_t *yy_input;


%}

%option noyywrap nounput
%option prefix="qcir_"

%x NONINITIAL

%%

<INITIAL>"#QCIR-G14"  { return FORMAT_ID; }
<INITIAL>[-+]?[0-9]+  {
  VAL.val = strtol(yytext, 0, 10);
  if (errno == ERANGE) return ERROR_TOKEN;
  return NUM;
  }
<INITIAL>\n           { ++qcir_line; BEGIN(NONINITIAL); return NL; }
<INITIAL>\n#[^\n]*    { ++qcir_line; BEGIN(NONINITIAL); return NL; }
<NONINITIAL>\n#[^\n]* { ++qcir_line; return NL; }
<NONINITIAL>\n        { ++qcir_line; return NL; }
<NONINITIAL>[oO][uU][tT][pP][uU][tT]  { return OUTPUT; }
<NONINITIAL>[eE][xX][iI][sS][tT][sS]  { return EXISTS; }
<NONINITIAL>[fF][oO][rR][aA][lL][lL]  { return FORALL; }
<NONINITIAL>[xX][oO][rR]     { return XOR; }
<NONINITIAL>[aA][nN][dD]     { return AND; }
<NONINITIAL>[oO][rR]      { return OR; }
<NONINITIAL>[iI][tT][eE]     { return ITE; }
<NONINITIAL>"("       { return LP; }
<NONINITIAL>")"       { return RP; }
<NONINITIAL>"="       { return EQ; }
<NONINITIAL>","       { return COMMA; }
<NONINITIAL>";"       { return SEMI; }
<NONINITIAL>"+"       { return PLUS; }
<NONINITIAL>"-"       { return MINUS; }
<NONINITIAL>[a-zA-Z0-9_!?]+  { VAL.str=strdup(yytext); return VAR_ID; }
<INITIAL,NONINITIAL><<EOF>>  { return 0; }
<INITIAL,NONINITIAL>[ \t]+   { }
<INITIAL,NONINITIAL>.        { cerr << "Unrecognized token "<<yytext << endl; exit(1); }

%%

gzFile gzin;
int yy_input_stdin(char *buf, int size) { return fread(buf, 1, size, stdin); }
int yy_input_gz(char *buf, int size) { return gzread(gzin, buf, size); }

void start_qcir_lex(const string& filename) {
  if (filename == "-") yy_input = yy_input_stdin;
  else {
    gzin = gzopen(filename.c_str(), "rb");
    yy_input = yy_input_gz;
    if (!gzin) {
      std::cerr << "E: Can't read file " << filename << std::endl;
      exit(100);
    }
  }
}
