// shared_ptr.cpp
// Implementation of shared_ptr.hpp
// Copyright (C) 2007 by Nathan Reed.  All rights and priveleges reserved.

#include "shared_ptr.hpp"

// Definition of the container for refcounts - has to be in a source file, not the header
refcounts_t refcounts_;
