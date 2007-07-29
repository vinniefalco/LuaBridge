# Makefile for luabridge
# Copyright (C) 2007 by Nathan Reed.  All rights and priveleges reserved.

default: release
all: release debug
release: luabridge-release test-release
debug: luabridge-debug test-debug

# Target for running the test program
test: test-release
	src/test

# C++ compiler options
CXX := g++
CXXFLAGS := -Wall -W -Wno-unknown-pragmas -Wno-ctor-dtor-privacy -Iinclude
REL_FLAGS := -O3
DBG_FLAGS := -g

# Linker options
LDFLAGS :=			# No linker flags at present
# Let the name of the lua library be given as an environment variable.
# UNDONE: figure out how to determine this automatically.
LUA_NAME ?= lua
LDLIBS := -l$(LUA_NAME)
VPATH := $(LIBRARY_PATH) $(LD_LIBRARY_PATH)

# Dependencies for libluabridge
LUABRIDGE_SOURCES := src/luabridge.cpp src/shared_ptr.cpp
LUABRIDGE_DBG_OBJS := $(LUABRIDGE_SOURCES:src/%.cpp=src/debug/%.o)
LUABRIDGE_REL_OBJS := $(LUABRIDGE_SOURCES:src/%.cpp=src/release/%.o)

luabridge-release: lib/. src/release/. lib/libluabridge.a
luabridge-debug: lib/. src/debug/. lib/libluabridged.a
lib/libluabridge.a: $(LUABRIDGE_REL_OBJS)
lib/libluabridged.a: $(LUABRIDGE_DBG_OBJS)

# Dependencies for test
TEST_SOURCES := src/test.cpp
TEST_DBG_OBJS := $(TEST_SOURCES:src/%.cpp=src/debug/%.o)
TEST_REL_OBJS := $(TEST_SOURCES:src/%.cpp=src/release/%.o)

test-release: src/release/. src/test
test-debug: src/debug/. src/testd
src/test: $(TEST_REL_OBJS) $(LDLIBS) lib/libluabridge.a
src/testd: $(TEST_DBG_OBJS) $(LDLIBS) lib/libluabridged.a

# Dependencies for source files on headers
# The sed command replaces "%.o" with "src/debug/%.o src/release/%.o"
-include depend
depend: $(LUABRIDGE_SOURCES) $(TEST_SOURCES)
	$(CXX) $(CPPFLAGS) -MM $^ | \
	sed 's,\([a-zA-Z0-9._-]*\)\.o:,src/debug/\1.o src/release/\1.o:,' > $@

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
src/test src/testd:
	$(CXX) $(LDFLAGS) -o $@ $^

# Cleanup
clean:
	rm -rf lib/libluabridged.a lib/libluabridge.a src/debug src/release \
	*~ src/*~ include/*~ include/impl/*~ depend

.PHONY: default all release debug clean luabridge-release luabridge-debug \
	test-release test-debug test
