# Makefile for luabridge
# Copyright (C) 2007 by Nathan Reed.  All rights and priveleges reserved.

all: luabridge test

CXX := g++
CXXFLAGS := -Wall -W -Wno-unknown-pragmas -Wno-ctor-dtor-privacy \
    -Iinclude -I$(HOME)/include
REL_FLAGS := -O3
DBG_FLAGS := -g

LDFLAGS := -L$(HOME)/lib
VPATH := $(HOME)/lib
LDLIBS := -llua

# Dependencies for libluabridge
LUABRIDGE_SOURCES := src/luabridge.cpp src/shared_ptr.cpp
LUABRIDGE_DBG_OBJS := $(LUABRIDGE_SOURCES:src/%.cpp=src/debug/%.o)
LUABRIDGE_REL_OBJS := $(LUABRIDGE_SOURCES:src/%.cpp=src/release/%.o)

luabridge: lib/. src/debug/. src/release/. \
    lib/libluabridged.a lib/libluabridge.a
lib/libluabridged.a: $(LUABRIDGE_DBG_OBJS)
lib/libluabridge.a: $(LUABRIDGE_REL_OBJS)

# Dependencies for test
TEST_SOURCES := src/test.cpp
TEST_DBG_OBJS := $(TEST_SOURCES:src/%.cpp=src/debug/%.o)
TEST_REL_OBJS := $(TEST_SOURCES:src/%.cpp=src/release/%.o)

test: src/debug/. src/release/. src/testd src/test
src/testd: $(TEST_DBG_OBJS) $(LDLIBS) lib/libluabridged.a
src/test: $(TEST_REL_OBJS) $(LDLIBS) lib/libluabridge.a

# Dependencies for source files on headers
# The sed command replaces "%.o" with "src/debug/%.o src/release/%.o"
include depend
depend: $(LUABRIDGE_SOURCES) $(TEST_SOURCES)
	@set -e; rm -f $@; \
	$(CXX) $(CPPFLAGS) -MM $^ > $@.$$$$; \
	sed 's,\([a-z]*\.o\)[ :]*,src/debug/\1 src/release/\1: ,g' \
	< $@.$$$$ > $@; \
	rm -f $@.$$$$

# Rule for making directories
%/.:
	mkdir -p $@

# Rules for making objects
src/debug/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(DBG_FLAGS) -c -o $@ $<
src/release/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(REL_FLAGS) -c -o $@ $<

# Rule for archives
%.a:
	$(AR) $(ARFLAGS) $@ $^

# Rule for executables
src/testd src/test:
	$(CXX) $(LDFLAGS) -o $@ $^

# Cleanup
clean:
	rm -rf lib/libluabridged.a lib/libluabridge.a src/debug src/release \
	*~ src/*~ include/*~ include/impl/*~ depend

.PHONY: all clean luabridge test
