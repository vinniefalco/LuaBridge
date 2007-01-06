/*
 * class.hpp - Copyright (C) 2007 by Nathan Reed
 * Implementation of the class__ class from luabridge.hpp.
 */

/*
 * Container for registered class names, with disambiguation for const types
 */

template <typename T>
struct classname
{
	static const char *name_;
	static const char *name ()
	{
		return classname<T>::name_;
	}
	static void set_name (const char *name)
	{
		classname<T>::name_ = name;
	}
};

// Initial type names are unknown
extern const char *classname_unknown;
template <typename T>
const char *classname<T>::name_ = classname_unknown;

// Specialization for const types, mapping to same names
template <typename T>
struct classname<const T>
{
	static const char *name ()
	{
		return classname<T>::name_;
	}
	static void set_name (const char *name)
	{
		classname<T>::name_ = name;
	}
};

/*
 * Lua-registerable C function template for destructors.  Objects are stored
 * in Lua as userdata containing a shared_ptr, and this is registered as the
 * __gc metamethod.  The expected classname is passed as an upvalue so that
 * we can ensure that we are destructing the right kind of object.
 */

template <typename T>
int destructor_dispatch (lua_State *L)
{
	((shared_ptr<T>*)checkclass(L, 1,
		lua_tostring(L, lua_upvalueindex(1))))->release();
	return 0;
}

/*
 * class__ constructors
 */

template <typename T>
class__<T>::class__ (lua_State *L_): L(L_)
{
	assert(classname<T>::name() != classname_unknown);
}

template <typename T>
class__<T>::class__ (lua_State *L_, const char *name): L(L_)
{
	// Create metatable for this class.  The metatable is stored in the Lua
	// registry, keyed by the given class name.
	luaL_newmetatable(L, name);
	// Set subclass_indexer as the __index metamethod
	lua_pushcfunction(L, &subclass_indexer);
	lua_setfield(L, -2, "__index");
	// Set the __gc metamethod to call the class destructor
	lua_pushstring(L, name);
	lua_pushcclosure(L, &destructor_dispatch<T>, 1);
	lua_setfield(L, -2, "__gc");
	// Set the __type metafield to the name of the class
	lua_pushstring(L, name);
	lua_setfield(L, -2, "__type");
	lua_pop(L, 1);

	classname<T>::set_name(name);
}

template <typename T>
class__<T>::class__ (lua_State *L_, const char *name,
	const char *basename): L(L_)
{
	// Create metatable for this class
	luaL_newmetatable(L, name);
	// Set subclass_indexer as the __index metamethod
	lua_pushcfunction(L, &subclass_indexer);
	lua_setfield(L, -2, "__index");
	// Set the __gc metamethod to call the class destructor
	lua_pushstring(L, name);
	lua_pushcclosure(L, &destructor_dispatch<T>, 1);
	lua_setfield(L, -2, "__gc");
	// Set the __type metafield to the name of the class
	lua_pushstring(L, name);
	lua_setfield(L, -2, "__type");
	// Set the __parent metafield to the base class's metatable
	luaL_getmetatable(L, basename);
	lua_setfield(L, -2, "__parent");
	lua_pop(L, 1);

	classname<T>::set_name(name);
}

/*
 * Lua-registerable C function templates for constructor proxies.  These are
 * registered to Lua as global functions with the name of the class, with the
 * appropriate metatable passed as an upvalue.  They allocate a new userdata,
 * initialize it with a shared_ptr to an appropriately constructed new class
 * object, and set the metatable so that Lua can use the object.
 */

template <typename T>
int constructor_dispatch0 (lua_State *L)
{
	// Allocate a new userdata and construct a T in-place there
	void *block = lua_newuserdata(L, sizeof(shared_ptr<T>));
	new(block) shared_ptr<T>(new T);

	// Set the userdata's metatable
	lua_pushvalue(L, lua_upvalueindex(1));
	lua_setmetatable(L, -2);

	return 1;
}

template <typename T, typename P1>
int constructor_dispatch1 (lua_State *L)
{
	// Allocate a new userdata and construct a T in-place there
	void *block = lua_newuserdata(L, sizeof(shared_ptr<T>));
	new(block) shared_ptr<T>(new T(tdstack<P1>::get(L, 1)));

	// Set the userdata's metatable
	lua_pushvalue(L, lua_upvalueindex(1));
	lua_setmetatable(L, -2);

	return 1;
}

template <typename T, typename P1, typename P2>
int constructor_dispatch2 (lua_State *L)
{
	// Allocate a new userdata and construct an A in-place there
	void *block = lua_newuserdata(L, sizeof(shared_ptr<T>));
	new(block) shared_ptr<T>(new T(tdstack<P1>::get(L, 1),
		tdstack<P2>::get(L, 2)));

	// Set the userdata's metatable
	lua_pushvalue(L, lua_upvalueindex(1));
	lua_setmetatable(L, -2);

	return 1;
}

/*
 * class__'s member functions to register constructors.
 */

template <typename T>
class__<T>& class__<T>::constructor ()
{
	luaL_getmetatable(L, classname<T>::name());
	lua_pushcclosure(L, &constructor_dispatch0<T>, 1);
	lua_setglobal(L, classname<T>::name());
	return *this;
}

template <typename T>
template <typename P1>
class__<T>& class__<T>::constructor ()
{
	luaL_getmetatable(L, classname<T>::name());
	lua_pushcclosure(L, &constructor_dispatch1<T, P1>, 1);
	lua_setglobal(L, classname<T>::name());
	return *this;
}

template <typename T>
template <typename P1, typename P2>
class__<T>& class__<T>::constructor ()
{
	luaL_getmetatable(L, classname<T>::name());
	lua_pushcclosure(L, &constructor_dispatch2<T, P1, P2>, 1);
	lua_setglobal(L, classname<T>::name());
	return *this;
}

/*
 * Lua-registerable C function templates for method proxies.  These are
 * registered with the expected classname as upvalue 1 and the member function
 * pointer as upvalue 2.  When called from Lua, they will receive the object
 * on which they are called as argument 1 and all the method arguments as
 * args 2 and up.
 */

template <typename T, typename Ret>
struct method_dispatch0
{
	typedef Ret (T::*func_ptr_type) ();
	static int apply (lua_State *L)
	{
		T *t = ((shared_ptr<T> *)checkclass(L, 1,
			lua_tostring(L, lua_upvalueindex(1))))->get();
		void *vp = lua_touserdata(L, lua_upvalueindex(2));
		func_ptr_type f = *((func_ptr_type *)vp);
		tdstack<Ret>::push(L, (t->*f)());
		return 1;
	}
};
template <typename T>
struct method_dispatch0<T, void>
{
	typedef void (T::*func_ptr_type) ();
	static int apply (lua_State *L)
	{
		T *t = ((shared_ptr<T> *)checkclass(L, 1,
			lua_tostring(L, lua_upvalueindex(1))))->get();
		void *vp = lua_touserdata(L, lua_upvalueindex(2));
		func_ptr_type f = *((func_ptr_type *)vp);
		t->*f();
		return 0;
	}
};

template <typename T, typename Ret, typename P1>
struct method_dispatch1
{
	typedef Ret (T::*func_ptr_type) (P1);
	static int apply (lua_State *L)
	{
		T *t = ((shared_ptr<T> *)checkclass(L, 1,
			lua_tostring(L, lua_upvalueindex(1))))->get();
		void *vp = lua_touserdata(L, lua_upvalueindex(2));
		func_ptr_type f = *((func_ptr_type *)vp);
		tdstack<Ret>::push(L, (t->*f)(tdstack<P1>::get(L, 2)));
		return 1;
	}
};
template <typename T, typename P1>
struct method_dispatch1<T, void, P1>
{
	typedef void (T::*func_ptr_type) (P1);
	static int apply (lua_State *L)
	{
		T *t = ((shared_ptr<T> *)checkclass(L, 1,
			lua_tostring(L, lua_upvalueindex(1))))->get();
		void *vp = lua_touserdata(L, lua_upvalueindex(2));
		func_ptr_type f = *((func_ptr_type *)vp);
		t->*f(tdstack<P1>::get(L, 2));
		return 0;
	}
};

template <typename T, typename Ret, typename P1, typename P2>
struct method_dispatch2
{
	typedef Ret (T::*func_ptr_type) (P1, P2);
	static int apply (lua_State *L)
	{
		T *t = ((shared_ptr<T> *)checkclass(L, 1,
			lua_tostring(L, lua_upvalueindex(1))))->get();
		void *vp = lua_touserdata(L, lua_upvalueindex(2));
		func_ptr_type f = *((func_ptr_type *)vp);
		tdstack<Ret>::push(L, (t->*f)(tdstack<P1>::get(L, 2),
			tdstack<P2>::get(L, 3)));
		return 1;
	}
};
template <typename T, typename P1, typename P2>
struct method_dispatch2<T, void, P1, P2>
{
	typedef void (T::*func_ptr_type) (P1, P2);
	static int apply (lua_State *L)
	{
		T *t = ((shared_ptr<T> *)checkclass(L, 1,
			lua_tostring(L, lua_upvalueindex(1))))->get();
		void *vp = lua_touserdata(L, lua_upvalueindex(2));
		func_ptr_type f = *((func_ptr_type *)vp);
		t->*f(tdstack<P1>::get(L, 2), tdstack<P2>::get(L, 3));
		return 0;
	}
};

/*
 * class__'s member functions to register methods.  The method proxies are all
 * registered as values in the class's metatable.  Since the class's metatable
 * is listed as its own __index member, index operations on the userdata will
 * lookup the correct functions in the metatable.
 */

template <typename T>
template <typename Ret>
class__<T>& class__<T>::method (const char *name, Ret (T::*func_ptr)())
{
	luaL_getmetatable(L, classname<T>::name());
	lua_pushstring(L, classname<T>::name());
	void *v = lua_newuserdata(L, sizeof(func_ptr));
	memcpy(v, &func_ptr, sizeof(func_ptr));
	lua_pushcclosure(L, &method_dispatch0<T, Ret>::apply, 2);
	lua_setfield(L, -2, name);
	lua_pop(L, 1);
	return *this;
}

template <typename T>
template <typename Ret, typename P1>
class__<T>& class__<T>::method (const char *name, Ret (T::*func_ptr)(P1))
{
	luaL_getmetatable(L, classname<T>::name());
	lua_pushstring(L, classname<T>::name());
	void *v = lua_newuserdata(L, sizeof(func_ptr));
	memcpy(v, &func_ptr, sizeof(func_ptr));
	lua_pushcclosure(L, &method_dispatch1<T, Ret, P1>::apply, 2);
	lua_setfield(L, -2, name);
	lua_pop(L, 1);
	return *this;
}

template <typename T>
template <typename Ret, typename P1, typename P2>
class__<T>& class__<T>::method (const char *name, Ret (T::*func_ptr)(P1, P2))
{
	luaL_getmetatable(L, classname<T>::name());
	lua_pushstring(L, classname<T>::name());
	void *v = lua_newuserdata(L, sizeof(func_ptr));
	memcpy(v, &func_ptr, sizeof(func_ptr));
	lua_pushcclosure(L, &method_dispatch2<T, Ret, P1, P2>::apply, 2);
	lua_setfield(L, -2, name);
	lua_pop(L, 1);
	return *this;
}
