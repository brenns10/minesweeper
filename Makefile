#-------------------------------------------------------------------------------
#
# File:         Makefile
#
# Author:       Stephen Brennan
#
# Date Created: Monday, 30 March 2015
#
# Description:  Makefile for Minesweeper program.
#
#-------------------------------------------------------------------------------

# Compiler Variable Declarations
CC=gcc
FLAGS=
INC=-Isrc/
CFLAGS=$(FLAGS) -c -g -Wall --std=c99 $(SMB_CONF) $(INC)
LFLAGS=$(FLAGS)
DIR_GUARD=@mkdir -p $(@D)

# Build configurations.
CFG=release
ifeq ($(CFG),debug)
FLAGS += -g -DDEBUG -DSMB_DEBUG
endif
ifeq ($(CFG),coverage)
CFLAGS += -fprofile-arcs -ftest-coverage
LFLAGS += -fprofile-arcs -lgcov
endif
ifneq ($(CFG),debug)
ifneq ($(CFG),release)
ifneq ($(CFG),coverage)
	@echo "Invalid configuration "$(CFG)" specified."
	@echo "You must specify a configuration when running make, e.g."
	@echo "  make CFG=debug"
	@echo "Choices are 'release', 'debug', and 'coverage'."
	@exit 1
endif
endif
endif

# Sources and Objects
SOURCES=$(shell find src/ -type f -name "*.c")
SOURCEDIRS=$(shell find src/ -type d)

OBJECTS=$(patsubst src/%.c,obj/$(CFG)/%.o,$(SOURCES))

# Main targets
.PHONY: all clean clean_all clean_docs clean_cov docs gcov

all: bin/$(CFG)/main

gcov:
	lcov --capture --directory . --output-file coverage.info
	genhtml coverage.info --output-directory cov/
	rm coverage.info

clean:
	rm -rf bin/$(CFG)/* obj/$(CFG)/* src/*.gch

clean_all: clean clean_docs clean_cov
	rm -rf bin/* obj/*

clean_docs:
	rm -rf doc/*

clean_cov:
	rm -rf cov/*

docs: src/*
	doxygen

# Dependencies.
src/minesweeper.c: src/minesweeper.h

# --- Compile Rule
obj/$(CFG)/%.o: src/%.c
	$(DIR_GUARD)
	$(CC) $(CFLAGS) $< -o $@

# --- Link Rule
bin/$(CFG)/main: $(OBJECTS)
	$(DIR_GUARD)
	$(CC) $(LFLAGS) $(OBJECTS) -o bin/$(CFG)/main
