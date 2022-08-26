//  https://github.com/vinniefalco/LuaBridge
// Copyright 2022, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

// This determines which version of Lua to use.
// The value is the same as LUA_VERSION_NUM in lua.h

#if LUABRIDGE_TEST_LUA_VERSION <= 501

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

#elif LUABRIDGE_TEST_LUA_VERSION <= 504

#include "lua.hpp"

#else

#error "LUABRIDGE_TEST_LUA_VERSION is not defined"

#endif
