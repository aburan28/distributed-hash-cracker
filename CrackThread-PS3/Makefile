# Makefile for CrackThread-PS3

###############################################################################
# System configuration
SHELL = /bin/sh
CC = g++
AS = nasm
.SUFFIXES:
.SUFFIXES: .asm .c .cpp .h .o

###############################################################################
# Configurable generic tool flags
CFLAGS = -Wall -O3 -g -fno-inline
ASFLAGS = -f elf
LDFLAGS = -shared -module

###############################################################################
# Architecture-specific tool flags
PS3_CFLAGS = -DPS3 -m32
PS3_LDFLAG = 

SPU_CFLAGS = -O3

###############################################################################
# Required libraries
LIBS = -lm -lpthread -ldl -lspe2

###############################################################################
# Sources and outputs

# Generic stuff
GEN_CPPFILES = CrackThread-PS3.cpp
GEN_HFILES = CrackThread-PS3.h
GEN_OBJS = CrackThread-PS3.o
OUTFILE = ../bin/crackthread-ps3.so

#SPU stuff
GENERATE_CFILE = Generate.c
GENERATE_OUTFILE = ../bin/Generate.spu
TEST_CFILE = Test.c
TEST_OUTFILE = ../bin/Test.spu

###############################################################################
# Combine generic flags with architecture-specific ones
ALL_CFLAGS = -I. -DPOSIX -fPIC $(CFLAGS) $(PS3_CFLAGS)
ALL_LDFLAGS = $(LDFLAGS) $(PS3_LDFLAGS)
CPPFILES = $(GEN_CPPFILES)
HFILES = $(GEN_HFILES) $(PS3_HFILES)
OBJS = $(GEN_OBJS) $(PS3_OBJS)

###############################################################################
# Targets
.PHONY: sanitycheck

#Default: build the output file
all: sanitycheck $(OUTFILE) $(TEST_OUTFILE) $(GENERATE_OUTFILE)

sanitycheck:
ifneq ($(ARCH),PS3)
	@echo "CrackThread-PS3 can only be compiled for the PS3 architecture."
	@false
endif

#Our actual executable
$(OUTFILE): $(OBJS)
	$(CC) $(OBJS) $(ALL_CFLAGS) $(LDFLAGS) $(LIBS) -o $(OUTFILE)

#Our guess-generation module
$(GENERATE_OUTFILE): $(GENERATE_CFILE)
	spu-gcc $(SPU_CFLAGS) $(GENERATE_CFILE) -o $(GENERATE_OUTFILE)

#Our testing module
$(TEST_OUTFILE): $(TEST_CFILE)
	spu-gcc $(SPU_CFLAGS) $(TEST_CFILE) -o $(TEST_OUTFILE)

#Rules for compiling various types of source	
.cpp.o: $(CPPFILES) $(HFILES)
	$(CC) -c $(ALL_CFLAGS) $< -o $@

#Simple - delete all of the temp files
clean:
	@rm -rf *.o $(OUTFILE) $(GENERATE_OUTFILE) $(TEST_OUTFILE)
