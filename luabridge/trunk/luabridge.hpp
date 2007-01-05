// luabridge.hpp
// Lightweight C++ to Lua binding library, allowing C++ functions and classes
// to be made available to Lua scripts
// Copyright (C) 2007 by Nathan Reed.  All rights and priveleges reserved.

#ifndef LUABRIDGE_HPP
#define LUABRIDGE_HPP

#include <lua.hpp>
#include "shared_ptr.hpp"

namespace luabridge
{
	template <typename T> class class__;

	// module encapsulates a Lua state and performs registration tasks
	class module
	{
		lua_State *L;
	public:
		module (lua_State *L_): L(L_) {}

		template <typename T>
		class__<T> class_ (const char *name);

		template <typename Ret>
		module& function (const char *name, Ret (*func_ptr)());
		template <typename Ret, typename P1>
		module& function (const char *name, Ret (*func_ptr)(P1));
		template <typename Ret, typename P1, typename P2>
		module& function (const char *name, Ret (*func_ptr)(P1, P2));
	};

	// class__ performs registration for members of a class
	template <typename T>
	class class__
	{
		lua_State *L;
	public:
		class__ (lua_State *L_, const char *classname_);

		class__<T>& constructor ();
		template <typename P1>
		class__<T>& constructor ();
		template <typename P1, typename P2>
		class__<T>& constructor ();

		template <typename Ret>
		class__<T>& method (const char *name, Ret (T::*func_ptr)());
		template <typename Ret, typename P1>
		class__<T>& method (const char *name, Ret (T::*func_ptr)(P1));
		template <typename Ret, typename P1, typename P2>
		class__<T>& method (const char *name, Ret (T::*func_ptr)(P1, P2));
	};

#	include "luabridge_impl.hpp"
}


#endif
