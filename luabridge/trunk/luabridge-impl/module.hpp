/*
 * module.hpp - Copyright (C) 2007 by Nathan Reed
 * Implementation of the module class from luabridge.hpp.
 */

/*
 * C functions that can be registered to Lua, wrapped in struct templates
 * so as to allow partial specialization for the void return type.  These act
 * as proxies for functions that we want to register, performing the mechanics
 * of dynamic type checking and so forth.  They are registered to Lua as
 * C closures with the actual function pointer to call as an upvalue.
 */

template <typename Ret>
struct dispatch0
{
	typedef Ret (*func_ptr_type) ();
	static int apply (lua_State *L)
	{
		func_ptr_type f =
			(func_ptr_type)lua_touserdata(L, lua_upvalueindex(1));
		tdstack<Ret>::push(L, f());
		return 1;
	}
};
template <>
struct dispatch0<void>
{
	typedef void (*func_ptr_type) ();
	static int apply (lua_State *L)
	{
		func_ptr_type f =
			(func_ptr_type)lua_touserdata(L, lua_upvalueindex(1));
		f();
		return 0;
	}
};

template <typename Ret, typename P1>
struct dispatch1
{
	typedef Ret (*func_ptr_type) (P1);
	static int apply (lua_State *L)
	{
		func_ptr_type f =
			(func_ptr_type)lua_touserdata(L, lua_upvalueindex(1));
		tdstack<Ret>::push(L, f(tdstack<P1>::get(L, 1)));
		return 1;
	}
};
template <typename P1>
struct dispatch1<void, P1>
{
	typedef void (*func_ptr_type) (P1);
	static int apply (lua_State *L)
	{
		func_ptr_type f =
			(func_ptr_type)lua_touserdata(L, lua_upvalueindex(1));
		f(tdstack<P1>::get(L, 1));
		return 0;
	}
};

template <typename Ret, typename P1, typename P2>
struct dispatch2
{
	typedef Ret (*func_ptr_type) (P1, P2);
	static int apply (lua_State *L)
	{
		func_ptr_type f =
			(func_ptr_type)lua_touserdata(L, lua_upvalueindex(1));
		tdstack<Ret>::push(L, f(tdstack<P1>::get(L, 1),
			tdstack<P2>::get(L, 2)));
		return 1;
	}
};
template <typename P1, typename P2>
struct dispatch2<void, P1, P2>
{
	typedef void (*func_ptr_type) (P1, P2);
	static int apply (lua_State *L)
	{
		func_ptr_type f =
			(func_ptr_type)lua_touserdata(L, lua_upvalueindex(1));
		f(tdstack<P1>::get(L, 1), tdstack<P2>::get(L, 2));
		return 0;
	}
};

/*
 * module's member functions that perform function registration.
 */

template <typename Ret>
module& module::function (const char *name, Ret (*func_ptr)())
{
	lua_pushlightuserdata(L, (void *)func_ptr);
	lua_pushcclosure(L, &dispatch0<Ret>::apply, 1);
	lua_setglobal(L, name);
	return *this;
}

template <typename Ret, typename P1>
module& module::function (const char *name, Ret (*func_ptr)(P1))
{
	lua_pushlightuserdata(L, (void *)func_ptr);
	lua_pushcclosure(L, &dispatch1<Ret, P1>::apply, 1);
	lua_setglobal(L, name);
	return *this;
}

template <typename Ret, typename P1, typename P2>
module& module::function (const char *name, Ret (*func_ptr)(P1, P2))
{
	lua_pushlightuserdata(L, (void *)func_ptr);
	lua_pushcclosure(L, &dispatch2<Ret, P1, P2>::apply, 1);
	lua_setglobal(L, name);
	return *this;
}

/*
 * module's member functions that perform class registration.
 */

// Lua-registerable C function template for destructors.  Objects are stored
// in Lua as userdata containing a shared_ptr, and this is registered as the
// __gc metamethod.  The expected classname is passed as an upvalue so that
// we can ensure that we are destructing the right kind of object.
template <typename T>
int destructor_dispatch (lua_State *L)
{
	((shared_ptr<T>*)luaL_checkudata(L, 1,
		lua_tostring(L, lua_upvalueindex(1))))->release();
	return 0;
}

template <typename T>
class__<T> module::class_ (const char *name)
{
	// Create metatable for this class.  The metatable is stored in the Lua
	// registry, keyed by the given class name.  If the metatable already
	// exists, we re-open the same table, so we can append new methods.
	luaL_newmetatable(L, name);
	// Set the metatable as its own __index element - this will allow index
	// operations on the userdata to be translated into index operations on
	// the metatable
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	// Set the garbage collection function to call the class destructor
	lua_pushstring(L, name);
	lua_pushcclosure(L, &destructor_dispatch<T>, 1);
	lua_setfield(L, -2, "__gc");
	lua_pop(L, 1);

	return class__<T>(L, name);
}
