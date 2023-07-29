-include makeenv
PROGRAM_NAME=cqesto

GENF = ./generated/Options.cpp ./generated/Options.h qcir_lex.c qcir_parse.tab.c qcir_parse.tab.h
CSRCS    = $(wildcard *.cpp) $(wildcard ./generated/*.cpp)
YACC     = $(wildcard *.y)
LEX      = $(wildcard *.l)
COBJS    = $(CSRCS:.cpp=.o) $(YACC:.y=.tab.o) $(LEX:.l=.o)

LIBD =
LIBS =
CXX?=g++
CC=g++

#DBG=1

ifdef PROF
	CFLAGS+=-pg
	LNFLAGS+=-pg
endif

ifdef EXPERT # allows using giving options, without options the solver's fairly dumb
	CFLAGS+=-DEXPERT
endif

ifdef DBG
	CFLAGS+=-O0
	CFLAGS+=-ggdb
	CFLAGS+=-DDBG
	MINISAT_DIR=./minisat/build/debug/lib
	MSAT=ld
else
	CFLAGS+=-DNDEBUG
	CFLAGS+=-O3
	MSAT=lr
	MINISAT_DIR=./minisat/build/release/lib
endif

ifdef NOO
	CFLAGS+=-O0
endif

CFLAGS += -Wall -Wextra
CFLAGS += -std=c++17
CFLAGS += -pedantic
CFLAGS += -MMD
LIBS += -lz
CFLAGS += -I./generated
#STATIC = 1

ifdef STATIC
	CFLAGS+=-static
	CFLAGS+=-DSTATICLN
	LNFLAGS+=-static
endif

ifdef NON_INCR
	CFLAGS+=-DNON_INCR
endif

# =================== Decide which sat solver to use
#USE_MINISAT=1 # minisat is the default
USE_SMS=1 # minisat is the default

CFLAGS+=-I./minisat # we use literals from minisat in any case

########### CADICAL
ifdef USE_IPASIR
	undefine USE_MINISAT
	CFLAGS+=-DUSE_IPASIR
	LIBS+=-lcadical
	CFLAGS+=-I./cadical
	LIBD+=-L./cadical
endif

########### MINISAT
ifdef USE_MINISAT
	LIBD+=-L$(MINISAT_DIR)
	MINISAT_LIB=$(MINISAT_DIR)/libminisat.a
	LIBS+=-lminisat
	CFLAGS+=-DUSE_MINISAT
endif

########### SMS
ifdef USE_SMS
	LIBD+=-L.
	LIBS+=-lipasir -lcadical
	CFLAGS+=-DUSE_SMS
	CFLAGS+=-I../../sms-cpp-impl/src # SMS headers
endif

$(info USE_MINISAT : $(USE_MINISAT))
$(info USE_IPASIR : $(USE_IPASIR))

all:  defs.h
	$(MAKE) source
	$(MAKE) $(PROGRAM_NAME)

ifdef USE_MINISAT
$(PROGRAM_NAME): $(MINISAT_LIB)
$(MINISAT_LIB):
	@cd ./minisat ; $(MAKE) CXX=$(CXX) $(MSAT)
endif

## Make definition file based on git info
defs.h:
	@./update_defs.sh

## Make source files
source:  $(GENF)

## Build program
$(PROGRAM_NAME):  $(COBJS)
	@echo Linking: $@
	@$(CXX) -o $@ $(COBJS) $(LNFLAGS) $(LIBD) $(LIBS)

## Build rule
%.o:	%.cpp
	@echo Compiling: $@
	@$(CXX) $(CFLAGS) -c -o $@ $<

%.o:	%.c Makefile
	@echo Compiling: $@
	@$(CXX) $(CFLAGS) -c -o $@ $<

## Build c from bison
%.tab.h %.tab.c: %.y
	@echo Generating: $*
	@bison $<

## Build c from lex
%.c:	%.l Makefile
	@echo Generating: $*
	@flex -o$@ $<

## Generate option parser
./generated/Options.h ./generated/Options.cpp: option_generation
	@echo Generating: $@
	mkdir -p generated
	./option_generation  >./generated/Options.h 2>./generated/Options.cpp

## Clean rule
clean:
	@rm -f $(PROGRAM_NAME) $(PROGRAM_NAME).exe $(COBJS) *.tab.[ch]
	cd ./minisat/; $(MAKE) clean

-include ${DEPENDS}
