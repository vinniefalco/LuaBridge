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

template <typename T>
class__<T> module::class_ ()
{
	return class__<T>(L);
}

template <typename T, typename Base>
class__<T> module::subclass (const char *name)
{
	assert(classname<Base>::name() != classname_unknown);
	return class__<T>(L, name, classname<Base>::name());
}

template <typename T>
class__<T> module::class_ (const char *name)
{
	return class__<T>(L, name);
}
