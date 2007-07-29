/*
 * class.hpp - Copyright (C) 2007 by Nathan Reed
 * Implementation of the class__ class from luabridge.hpp.
 */

/*
 * Container for registered class names, with awareness of const types
 */

template <typename T>
struct classname
{
	static const char *name_;
	static const char *name ()
	{
		return classname<T>::name_;
	}
	static bool is_const ()
	{
		return false;
	}
	static void set_name (const char *name)
	{
		classname<T>::name_ = name;
	}
};

// Initial type names are unknown
template <typename T>
const char *classname<T>::name_ = classname_unknown;

// Specialization for const types, mapping to same names
template <typename T>
struct classname <const T>
{
	static const char *name ()
	{
		return classname<T>::name_;
	}
	static bool is_const ()
	{
		return true;
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
	void *obj = checkclass(L, 1, lua_tostring(L, lua_upvalueindex(1)), true);
	shared_ptr<T> &ptr = *((shared_ptr<T> *)obj);
	ptr.~shared_ptr<T>();
	return 0;
}

/*
 * Functions for metatable construction.  These functions create a metatable and
 * leave it in the top element of the Lua stack (in addition to registering it
 * wherever it needs to be registered).
 */

template <typename T>
void create_metatable (lua_State *L, const char *name)
{
	luaL_newmetatable(L, name);
	// Set indexer as the __index metamethod
	lua_pushcfunction(L, &indexer);
	rawsetfield(L, -2, "__index");
	// Set m_newindexer as the __newindex metamethod
	lua_pushcfunction(L, &m_newindexer);
	rawsetfield(L, -2, "__newindex");
	// Set the __gc metamethod to call the class destructor
	lua_pushstring(L, name);
	lua_pushcclosure(L, &destructor_dispatch<T>, 1);
	rawsetfield(L, -2, "__gc");
	// Set the __type metafield to the name of the class
	lua_pushstring(L, name);
	rawsetfield(L, -2, "__type");
	// Create the __propget and __propset metafields as empty tables
	lua_newtable(L);
	rawsetfield(L, -2, "__propget");
	lua_newtable(L);
	rawsetfield(L, -2, "__propset");
}

template <typename T>
void create_const_metatable (lua_State *L, const char *name)
{
	std::string constname = std::string("const ") + name;
	luaL_newmetatable(L, constname.c_str());
	lua_pushcfunction(L, &indexer);
	rawsetfield(L, -2, "__index");
	lua_pushcfunction(L, &m_newindexer);
	rawsetfield(L, -2, "__newindex");
	lua_pushstring(L, constname.c_str());
	lua_pushcclosure(L, &destructor_dispatch<T>, 1);
	rawsetfield(L, -2, "__gc");
	lua_pushstring(L, constname.c_str());
	rawsetfield(L, -2, "__type");
	lua_newtable(L);
	rawsetfield(L, -2, "__propget");
}

template <typename T>
void create_static_table (lua_State *L, const char *name)
{
	lua_newtable(L);
	// Set it as its own metatable
	lua_pushvalue(L, -1);
	lua_setmetatable(L, -2);
	// Set indexer as the __index metamethod
	lua_pushcfunction(L, &indexer);
	rawsetfield(L, -2, "__index");
	// Set newindexer as the __newindex metamethod
	lua_pushcfunction(L, &newindexer);
	rawsetfield(L, -2, "__newindex");
	// Create the __propget and __propset metafields as empty tables
	lua_newtable(L);
	rawsetfield(L, -2, "__propget");
	lua_newtable(L);
	rawsetfield(L, -2, "__propset");
	// Install it in the global environment
	lua_pushvalue(L, -1);
	lua_setglobal(L, name);
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
	assert(!classname<T>::is_const());
	classname<T>::set_name(name);

	// Create metatable for this class.  The metatable is stored in the Lua
	// registry, keyed by the given class name.
	create_metatable<T>(L, name);

	// Create const metatable for this class.  This is identical to the
	// previous metatable, except that it has "const " prepended to the __type
	// field, and has no __propset field.  Const methods will be added to the
	// const metatable, non-const methods to the normal metatable.
	create_const_metatable<T>(L, name);

	// Set __const metafield to point to the const metatable
	rawsetfield(L, -2, "__const");
	// Pop the original metatable
	lua_pop(L, 1);

	// Create static table for this class.  This is stored in the global
	// environment, keyed by the given class name.  Its __call metamethod
	// will be the constructor, and it will also contain static members.
	create_static_table<T>(L, name);
	lua_pop(L, 1);
}

template <typename T>
class__<T>::class__ (lua_State *L_, const char *name,
	const char *basename): L(L_)
{
	assert(!classname<T>::is_const());
	classname<T>::set_name(name);

	// Create metatable for this class
	create_metatable<T>(L, name);
	// Set the __parent metafield to the base class's metatable
	luaL_getmetatable(L, basename);
	rawsetfield(L, -2, "__parent");

	// Create const metatable for this class.  Its __parent field will point
	// to the const metatable of the parent class.
	create_const_metatable<T>(L, name);
	std::string base_constname = std::string("const ") + basename;
	luaL_getmetatable(L, base_constname.c_str());
	rawsetfield(L, -2, "__parent");

	// Set __const metafield to point to the const metatable
	rawsetfield(L, -2, "__const");
	// Pop the original metatable
	lua_pop(L, 1);

	// Create static table for this class
	create_static_table<T>(L, name);
	// Set the __parent metafield to the base class's static table
	lua_getglobal(L, basename);
	rawsetfield(L, -2, "__parent");
	lua_pop(L, 1);
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
	// Allocate a new userdata and construct a shared_ptr<T> in-place there
	void *block = lua_newuserdata(L, sizeof(shared_ptr<T>));
	arglist<Params, 2> args(L);
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
	// Get a reference to the class's static table
	lua_getglobal(L, classname<T>::name());
	// Push the constructor proxy, with the class's metatable as an upvalue
	luaL_getmetatable(L, classname<T>::name());
	lua_pushcclosure(L,
		&constructor_proxy<T, typename fnptr<FnPtr>::params>, 1);
	// Set the constructor proxy as the __call metamethod of the static table
	rawsetfield(L, -2, "__call");
	lua_pop(L, 1);
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
		return 0;
	}
};

/*
 * Perform method registration in a class.  The method proxies are all
 * registered as values in the class's metatable, which is searched by the
 * indexer function we've installed as __index metamethod.
 */

template <typename T>
template <typename FnPtr>
class__<T>& class__<T>::method (const char *name, FnPtr fp)
{
	assert(fnptr<FnPtr>::mfp);
	std::string metatable_name = classname<T>::name();
	if (fnptr<FnPtr>::const_mfp)
		metatable_name = "const " + metatable_name;
	luaL_getmetatable(L, metatable_name.c_str());
	lua_pushstring(L, metatable_name.c_str());
	void *v = lua_newuserdata(L, sizeof(FnPtr));
	memcpy(v, &fp, sizeof(FnPtr));
	lua_pushcclosure(L, &method_proxy<FnPtr>::f, 2);
	rawsetfield(L, -2, name);
	lua_pop(L, 1);
	return *this;
}

/*
 * Lua-registerable C function templates for getting and setting the value of
 * an object member through a member pointer; similiar to the global property
 * proxies, but they take both the expected classname for type-checking and
 * the member pointer as upvalues.
 */

template <typename T, typename U>
int m_propget_proxy (lua_State *L)
{
	T *obj = ((shared_ptr<T> *)checkclass(L, 1,
		lua_tostring(L, lua_upvalueindex(1))))->get();
	U T::* mp = *(U T::**)lua_touserdata(L, lua_upvalueindex(2));
	tdstack<U>::push(L, obj->*mp);
	return 1;
}

template <typename T, typename U>
int m_propset_proxy (lua_State *L)
{
	T *obj = ((shared_ptr<T> *)checkclass(L, 1,
		lua_tostring(L, lua_upvalueindex(1))))->get();
	U T::* mp = *(U T::**)lua_touserdata(L, lua_upvalueindex(2));
	obj->*mp = tdstack<U>::get(L, 2);
	return 0;
}

/* Property registration.  Properties are stored in the class's __propget
 * metafield, with the property name as the get-function and property name
 * + "_set" as the set-function.  Note that property getters are stored
 * both in the regular metatable and the const metatable.
 */

template <typename T>
template <typename U>
class__<T>& class__<T>::property_ro (const char *name, U T::* mp)
{
	luaL_getmetatable(L, classname<T>::name());
	std::string constname = std::string("const ") + classname<T>::name();
	luaL_getmetatable(L, constname.c_str());
	rawgetfield(L, -2, "__propget");
	rawgetfield(L, -2, "__propget");
	lua_pushstring(L, constname.c_str());
	void *v = lua_newuserdata(L, sizeof(U T::*));
	memcpy(v, &mp, sizeof(U T::*));
	lua_pushcclosure(L, &m_propget_proxy<T, U>, 2);
	lua_pushvalue(L, -1);
	rawsetfield(L, -3, name);
	rawsetfield(L, -3, name);
	lua_pop(L, 4);
	return *this;
}

template <typename T>
template <typename U>
class__<T>& class__<T>::property_ro (const char *name, U (T::*get) () const)
{
	luaL_getmetatable(L, classname<T>::name());
	std::string constname = std::string("const ") + classname<T>::name();
	luaL_getmetatable(L, constname.c_str());
	rawgetfield(L, -2, "__propget");
	rawgetfield(L, -2, "__propget");
	lua_pushstring(L, constname.c_str());
	typedef U (T::*FnPtr) () const;
	void *v = lua_newuserdata(L, sizeof(FnPtr));
	memcpy(v, &get, sizeof(FnPtr));
	lua_pushcclosure(L, &method_proxy<FnPtr>::f, 2);
	lua_pushvalue(L, -1);
	rawsetfield(L, -3, name);
	rawsetfield(L, -3, name);
	lua_pop(L, 4);
	return *this;
}

template <typename T>
template <typename U>
class__<T>& class__<T>::property_rw (const char *name, U T::* mp)
{
	property_ro(name, mp);
	luaL_getmetatable(L, classname<T>::name());
	rawgetfield(L, -1, "__propset");
	lua_pushstring(L, classname<T>::name());
	void *v = lua_newuserdata(L, sizeof(U T::*));
	memcpy(v, &mp, sizeof(U T::*));
	lua_pushcclosure(L, &m_propset_proxy<T, U>, 2);
	rawsetfield(L, -2, name);
	lua_pop(L, 2);
	return *this;
}

template <typename T>
template <typename U>
class__<T>& class__<T>::property_rw (const char *name,
	U (T::*get) () const, void (T::*set) (U))
{
	property_ro(name, get);
	luaL_getmetatable(L, classname<T>::name());
	rawgetfield(L, -1, "__propset");
	lua_pushstring(L, classname<T>::name());
	typedef void (T::*FnPtr) (U);
	void *v = lua_newuserdata(L, sizeof(FnPtr));
	memcpy(v, &set, sizeof(FnPtr));
	lua_pushcclosure(L, &method_proxy<FnPtr>::f, 2);
	rawsetfield(L, -2, name);
	lua_pop(L, 2);
	return *this;
}

/*
 * Static method registration.  Method proxies are registered as values
 * in the class's static table.  We use the global function proxies defined
 * in module.hpp, since static methods are really just hidden globals.
 */

template <typename T>
template <typename FnPtr>
class__<T>& class__<T>::static_method (const char *name, FnPtr fp)
{
	assert(!fnptr<FnPtr>::mfp);
	lua_getglobal(L, classname<T>::name());
	lua_pushlightuserdata(L, (void *)fp);
	lua_pushcclosure(L, &function_proxy<FnPtr>::f, 1);
	rawsetfield(L, -2, name);
	lua_pop(L, 1);
	return *this;
}

/*
 * Static property registration.  Works the same way as class properties,
 * but the proxy functions are stored in the static __propget and __propset
 * tables, and the proxies are the same as for global properties.
 */

template <typename T>
template <typename U>
class__<T>& class__<T>::static_property_ro (const char *name, U *data)
{
	lua_getglobal(L, classname<T>::name());
	rawgetfield(L, -1, "__propget");
	lua_pushlightuserdata(L, data);
	lua_pushcclosure(L, &propget_proxy<U>, 1);
	rawsetfield(L, -2, name);
	lua_pop(L, 2);
	return *this;
}

template <typename T>
template <typename U>
class__<T>& class__<T>::static_property_ro (const char *name, U (*get) ())
{
	lua_getglobal(L, classname<T>::name());
	rawgetfield(L, -1, "__propget");
	lua_pushlightuserdata(L, (void *)get);
	lua_pushcclosure(L, &function_proxy<U (*) ()>::f, 1);
	rawsetfield(L, -2, name);
	lua_pop(L, 2);
	return *this;
}

template <typename T>
template <typename U>
class__<T>& class__<T>::static_property_rw (const char *name, U *data)
{
	static_property_ro(name, data);
	lua_getglobal(L, classname<T>::name());
	rawgetfield(L, -1, "__propset");
	lua_pushlightuserdata(L, data);
	lua_pushcclosure(L, &propset_proxy<U>, 1);
	rawsetfield(L, -2, name);
	lua_pop(L, 2);
	return *this;
}

template <typename T>
template <typename U>
class__<T>& class__<T>::static_property_rw (const char *name, U (*get) (),
                                            void (*set) (U))
{
	static_property_ro(name, get);
	lua_getglobal(L, classname<T>::name());
	rawgetfield(L, -1, "__propset");
	lua_pushlightuserdata(L, (void *)set);
	lua_pushcclosure(L, &function_proxy<void (*) (U)>::f, 1);
	rawsetfield(L, -2, name);
	lua_pop(L, 2);
	return *this;
}
