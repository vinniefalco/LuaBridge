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

	// module performs registration tasks in a given Lua state
	class module
	{
		lua_State *L;
	public:
		module (lua_State *L_): L(L_) {}

		// Function registration

		template <typename FnPtr>
		module& function (const char *name, FnPtr fp);

		// !!!UNDONE: support variables (global properties)

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

		// !!!UNDONE: support namespaces
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

		// Method registration
		template <typename FnPtr>
		class__<T>& method (const char *name, FnPtr fp);

		// Property registration.  Properties can be read/write (rw)
		// or read-only (ro).  Varieties that access member pointers directly
		// and varieties that access through function calls are provided.
		template <typename U>
		class__<T>& property_ro (const char *name, U T::* mp);
		template <typename U>
		class__<T>& property_ro (const char *name, U (T::*get) () const);
		template <typename U>
		class__<T>& property_rw (const char *name, U T::* mp);
		template <typename U>
		class__<T>& property_rw (const char *name,
			U (T::*get) () const, void (T::*set) (U));

		// Static method registration
		template <typename FnPtr>
		class__<T>& static_method (const char *name, FnPtr fp);

		// Static property registration
		template <typename U>
		class__<T>& static_property_ro (const char *name, U *data);
		template <typename U>
		class__<T>& static_property_ro (const char *name, U (*get) ());
		template <typename U>
		class__<T>& static_property_rw (const char *name, U *data);
		template <typename U>
		class__<T>& static_property_rw (const char *name, U (*get) (),
		                                void (*set) (U));

		// !!!UNDONE: allow inheriting Lua classes from C++ classes
	};

	// Convenience functions: like lua_getfield and lua_setfield, but raw
	inline void rawgetfield (lua_State *L, int idx, const char *key)
	{
		lua_pushstring(L, key);
		if (idx < 0) --idx;
		lua_rawget(L, idx);
	}
	inline void rawsetfield (lua_State *L, int idx, const char *key)
	{
		lua_pushstring(L, key);
		lua_insert(L, -2);
		if (idx < 0) --idx;
		lua_rawset(L, idx);
	}

	// Prototypes for implementation functions implemented in luabridge.cpp
	void *checkclass (lua_State *L, int idx, const char *tname,
		bool exact = false);
	int indexer (lua_State *L);
	int newindexer (lua_State *L);
	int m_newindexer (lua_State *L);

	// Predeclare classname struct since several implementation files use it
	template <typename T>
	struct classname;
	extern const char *classname_unknown;

	// Include implementation files
#	include "impl/typelist.hpp"
#	include "impl/stack.hpp"
#	include "impl/module.hpp"
#	include "impl/class.hpp"
}


#endif
