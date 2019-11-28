-include makeenv
GENF=./generated/Options.cpp ./generated/Options.h \
qcir_lex.c qcir_parse.tab.c qcir_parse.tab.h
CSRCS    = $(wildcard *.cpp) $(wildcard ./generated/*.cpp)
YACC     = $(wildcard *.y)
LEX      = $(wildcard *.l)
COBJS    = $(CSRCS:.cpp=.o) $(YACC:.y=.tab.o) $(LEX:.l=.o)
LIBD =
LIBS =
CXX?=g++
CC=g++

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
CFLAGS+=-D_GLIBCXX_DEBUG
MINISAT_DIR=./minisat/build/debug/lib
MSAT=ld
else
CFLAGS+=-DNDEBUG
CFLAGS+=-O3
MSAT=lr
MINISAT_DIR=./minisat/build/release/lib
endif

LIBD+=-L$(MINISAT_DIR)
MINISAT_LIB=$(MINISAT_DIR)/libminisat.a

ifdef NOO
CFLAGS+=-O0
endif

CFLAGS += -Wall -Wextra -DBUILDDATE='"$(BUILDDATE)"' -DDISTDATE='"$(DISTDATE)"'
CFLAGS += -DCHANGESET='"$(CHANGESET)"' -DRELDATE='"$(RELDATE)"'
CFLAGS+=-std=c++11
CFLAGS+=-pedantic
CFLAGS+=-MMD
LIBS+=-lz
CFLAGS+=-I./generated

LIBS+=-lminisat
CFLAGS+=-I./minisat/

ifdef STATIC
CFLAGS+=-static
CFLAGS+=-DSTATICLN
LNFLAGS+=-static
endif

ifdef USE_SIMP
CFLAGS+=-DUSE_SIMP
endif

ifdef NON_INCR
CFLAGS+=-DNON_INCR
endif

.PHONY: satmake

all:  make_defs satmake
	$(MAKE)  source
	$(MAKE)  cqesto

satmake:
	@cd ./minisat ; $(MAKE) CXX=$(CXX) $(MSAT)

make_defs:
	@./update_defs.sh

source:  $(GENF)

cqesto:  $(COBJS) $(MINISAT_LIB)
	@echo Linking: $@
	@$(CXX) -o $@ $(COBJS) $(LNFLAGS) $(LIBD) $(LIBS)

$(MINISAT_LIB): satmake

## Build rule
%.o:	%.cpp
	@echo Compiling: $@
	@$(CXX) $(CFLAGS) -c -o $@ $<

%.o:	%.c Makefile
	@echo Compiling: $@
	@$(CXX) $(CFLAGS) -c -o $@ $<

%.tab.h %.tab.c: %.y
	@echo Generating: $*
	@bison $<

%.c:	%.l Makefile
	@echo Generating: $*
	@flex -o$@ $<

./generated/Options.h ./generated/Options.cpp: option_generation
	@echo Generating: $@
	mkdir -p generated
	./option_generation  >./generated/Options.h 2>./generated/Options.cpp

## Clean rule
clean:
	@rm -f cqesto cqesto.exe $(COBJS) *.tab.[ch]
	cd ./minisat/; $(MAKE) clean

-include ${DEPENDS}
