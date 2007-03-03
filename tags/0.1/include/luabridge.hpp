/*
 * luabridge.hpp - Copyright (C) 2007 by Nathan Reed
 * Lightweight C++ to Lua binding library, allowing C++ functions and classes
 * to be made available to Lua scripts.
 */

#ifndef LUABRIDGE_HPP
#define LUABRIDGE_HPP

#include <cassert>
#include <string>
#include <lua.hpp>

#ifndef USE_OTHER_SHARED_PTR
#	include "shared_ptr.hpp"
#endif

namespace luabridge
{
	template <typename T> class class__;

	// module encapsulates a Lua state and performs registration tasks
	class module
	{
		lua_State *L;
	public:
		module (lua_State *L_): L(L_) {}

		// !!!UNDONE: support variables (global properties)
		// !!!UNDONE: support packages

		// Class registration

		// For registering a class that hasn't been registered before
		template <typename T>
		class__<T> class_ (const char *name);
		// For registering subclasses (the base class must also be registered)
		template <typename T, typename Base>
		class__<T> subclass (const char *name);
		// For adding new methods to a previously registered class, if desired
		template <typename T>
		class__<T> class_ ();

		// Function registration
		// !!!UNDONE: support overloading (maybe)

		template <typename FnPtr>
		module& function (const char *name, FnPtr fp);
	};

	// class__ performs registration for members of a class
	template <typename T>
	class class__
	{
		lua_State *L;
	public:
		class__ (lua_State *L_);
		class__ (lua_State *L_, const char *name);
		class__ (lua_State *L_, const char *name, const char *basename);

		// Constructor registration.  The template parameter should be passed
		// a function pointer type; only the argument list will be used (since
		// you can't take the address of a ctor).
		template <typename FnPtr>
		class__<T>& constructor ();

		// !!!UNDONE: handle const and static methods, and properties
		// !!!UNDONE: allow inheriting Lua classes from C++ classes

		// Method registration
		template <typename FnPtr>
		class__<T>& method (const char *name, FnPtr fp);

	};

	// Prototypes for implementation functions implemented in luabridge.cpp
	void *checkclass (lua_State *L, int idx, const char *tname);
	int subclass_indexer (lua_State *L);

	// Predeclare the classname struct since several implementation files use it
	template <typename T>
	struct classname;

	// Include implementation files
#	include "impl/typelist.hpp"
#	include "impl/stack.hpp"
#	include "impl/module.hpp"
#	include "impl/class.hpp"
}


#endif
