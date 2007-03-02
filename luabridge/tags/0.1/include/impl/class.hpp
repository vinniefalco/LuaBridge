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
		lua_tostring(L, lua_upvalueindex(1))))->reset();
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
 * Lua-registerable C function template for constructor proxies.  These are
 * registered to Lua as global functions with the name of the class, with the
 * appropriate metatable passed as an upvalue.  They allocate a new userdata,
 * initialize it with a shared_ptr to an appropriately constructed new class
 * object, and set the metatable so that Lua can use the object.
 */

template <typename T, typename Params>
int constructor_proxy (lua_State *L)
{
	// Allocate a new userdata and construct a T in-place there
	void *block = lua_newuserdata(L, sizeof(shared_ptr<T>));
	arglist<Params> args(L);
	new(block) shared_ptr<T>(constructor<T, Params>::apply(args));

	// Set the userdata's metatable
	lua_pushvalue(L, lua_upvalueindex(1));
	lua_setmetatable(L, -2);

	return 1;
}

/*
 * Perform constructor registration in a class.
 */

template <typename T>
template <typename FnPtr>
class__<T>& class__<T>::constructor ()
{
	luaL_getmetatable(L, classname<T>::name());
	lua_pushcclosure(L,
		&constructor_proxy<T, typename fnptr<FnPtr>::params>, 1);
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

template <typename FnPtr, typename Ret = typename fnptr<FnPtr>::resulttype>
struct method_proxy
{
	typedef typename fnptr<FnPtr>::classtype classtype;
	typedef typename fnptr<FnPtr>::params params;
	static int f (lua_State *L)
	{
		classtype *obj = ((shared_ptr<classtype> *)checkclass(L, 1,
			lua_tostring(L, lua_upvalueindex(1))))->get();
		FnPtr fp = *(FnPtr *)lua_touserdata(L, lua_upvalueindex(2));
		arglist<params, 2> args(L);
		tdstack<Ret>::push(L, fnptr<FnPtr>::apply(obj, fp, args));
		return 1;
	}
};

template <typename FnPtr>
struct method_proxy <FnPtr, void>
{
	typedef typename fnptr<FnPtr>::classtype classtype;
	typedef typename fnptr<FnPtr>::params params;
	static int f (lua_State *L)
	{
		classtype *obj = ((shared_ptr<classtype> *)checkclass(L, 1,
			lua_tostring(L, lua_upvalueindex(1))))->get();
		FnPtr fp = *(FnPtr *)lua_touserdata(L, lua_upvalueindex(2));
		arglist<params, 2> args(L);
		fnptr<FnPtr>::apply(obj, fp, args);
		return 1;
	}
};

/*
 * Perform method registration in a class.  The method proxies are all
 * registered as values in the class's metatable, which is searched by the
 * subclass_indexer function we've installed as __index metamethod.
 */

template <typename T>
template <typename FnPtr>
class__<T>& class__<T>::method (const char *name, FnPtr fp)
{
	luaL_getmetatable(L, classname<T>::name());
	lua_pushstring(L, classname<T>::name());
	void *v = lua_newuserdata(L, sizeof(FnPtr));
	memcpy(v, &fp, sizeof(FnPtr));
	lua_pushcclosure(L, &method_proxy<FnPtr>::f, 2);
	lua_setfield(L, -2, name);
	lua_pop(L, 1);
	return *this;
}
