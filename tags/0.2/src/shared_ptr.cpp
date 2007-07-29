/*
 * shared_ptr.cpp - Copyright (C) 2007 by Nathan Reed
 * Source for shared_ptr class template.
 */

#include "../include/shared_ptr.hpp"

// Definition of the container for refcounts - has to be in a source file,
// not the header
luabridge::refcounts_t luabridge::refcounts_;
