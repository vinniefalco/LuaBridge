// luabridge.hpp
// Implementation of template functions from luabridge.hpp
// Copyright (C) 2007 by Nathan Reed.  All rights and priveleges reserved.

#include <cassert>
#include <iostream>
#include <string>

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
const char *classname_unknown = "(unknown type)";
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
 * Compile-time dispatchable function templates to manipulate the Lua stack
 */

// These totally generic functions are unimplemented, causing a compiler
// error if they are called.  We don't know how to send objects of arbitrary
// types to and from Lua.  Following are specializations of this structure
// that allow specific types to be handled in specific ways.
template <typename T>
struct generic
{
	static void push (lua_State *L, T data);
	static T get (lua_State *L, int index);
};

// Pointers and references: getting is done by retrieving the address from
// the Lua-owned shared_ptr, but pushing is not allowed since luabridge
// has no idea of the ownership semantics of these objects.  You can only
// push shared_ptrs, not naked pointers and references.

template <typename T>
struct generic<T*>
{
	static void push (lua_State *L, T *data);
	static T* get (lua_State *L, int index)
	{
		return ((shared_ptr<T> *)
			luaL_checkudata(L, index, classname<T>::name()))->get();
	}
};

template <typename T>
struct generic<T* const>
{
	static void push (lua_State *L, T * const data);
	static T* const get (lua_State *L, int index)
	{
		return ((shared_ptr<T> *)
			luaL_checkudata(L, index, classname<T>::name()))->get();
	}
};

template <typename T>
struct generic<T&>
{
	static void push (lua_State *L, T &data);
	static T& get (lua_State *L, int index)
	{
		return *((shared_ptr<T> *)
			luaL_checkudata(L, index, classname<T>::name()))->get();
	}
};

// shared_ptr: we can push these

template <typename T>
struct generic<shared_ptr<T> >
{
	static void push (lua_State *L, shared_ptr<T> data)
	{
		// Make sure we don't try to push ptrs to objects of
		// unregistered classes or primitive types
		assert(classname<T>::name() != classname_unknown);

		// Allocate a new userdata and construct the pointer in-place there
		void *block = lua_newuserdata(L, sizeof(shared_ptr<T>));
		new(block) shared_ptr<T>(data);

		// Set the userdata's metatable
		luaL_getmetatable(L, classname<T>::name());
		lua_setmetatable(L, -2);
	}
	static shared_ptr<T> get (lua_State *L, int index)
	{
		return *(shared_ptr<T> *)
			luaL_checkudata(L, index, classname<T>::name());
	}
};

// Primitive types, including const char * and std::string

template <>
struct generic<float>
{
	static void push (lua_State *L, float data)
	{
		lua_pushnumber(L, (lua_Number)data);
	}
	static float get (lua_State *L, int index)
	{
		return (float)luaL_checknumber(L, index);
	}
};

template <>
struct generic<double>
{
	static void push (lua_State *L, double data)
	{
		lua_pushnumber(L, (lua_Number)data);
	}
	static double get (lua_State *L, int index)
	{
		return (double)luaL_checknumber(L, index);
	}
};

template <>
struct generic<int>
{
	static void push (lua_State *L, int data)
	{
		lua_pushinteger(L, data);
	}
	static int get (lua_State *L, int index)
	{
		return luaL_checkint(L, index);
	}
};

template <>
struct generic<bool>
{
	static void push (lua_State *L, bool data)
	{
		lua_pushboolean(L, data ? 1 : 0);
	}
	static bool get (lua_State *L, int index)
	{
		luaL_checktype(L, index, LUA_TBOOLEAN);
		// In MSC, disable "bool to int conversion" warning
#		ifdef _MSC_VER
#			pragma warning (push)
#			pragma warning (disable: 4800)
#		endif

		return (bool)lua_toboolean(L, index);

#		ifdef _MSC_VER
#			pragma warning (pop)
#		endif
	}
};

template <>
struct generic<const char *>
{
	static void push (lua_State *L, const char *data)
	{
		lua_pushstring(L, data);
	}
	static const char *get (lua_State *L, int index)
	{
		return luaL_checkstring(L, index);
	}
};

template <>
struct generic<std::string>
{
	static void push (lua_State *L, const std::string &data)
	{
		lua_pushstring(L, data.c_str());
	}
	static std::string get (lua_State *L, int index)
	{
		return std::string(luaL_checkstring(L, index));
	}
};
template <>
struct generic<const std::string &>
{
	static void push (lua_State *L, const std::string &data)
	{
		lua_pushstring(L, data.c_str());
	}
	static std::string get (lua_State *L, int index)
	{
		return std::string(luaL_checkstring(L, index));
	}
};

/****************************************************************************
 * module
 ****************************************************************************/

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
		generic<Ret>::push(L, f());
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
		generic<Ret>::push(L, f(generic<P1>::get(L, 1)));
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
		f(generic<P1>::get(L, 1));
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
		generic<Ret>::push(L, f(generic<P1>::get(L, 1),
			generic<P2>::get(L, 2)));
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
		f(generic<P1>::get(L, 1), generic<P2>::get(L, 2));
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

/****************************************************************************
 * class__
 ****************************************************************************/

// class__ constructor: sets classname for newly registered class
template <typename T>
class__<T>::class__ (lua_State *L_, const char *classname_): L(L_)
{
	classname<T>::set_name(classname_);
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
	new(block) shared_ptr<T>(new T(generic<P1>::get(L, 1)));

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
	new(block) shared_ptr<T>(new T(generic<P1>::get(L, 1),
		generic<P2>::get(L, 2)));

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
		T *t = ((shared_ptr<T> *)luaL_checkudata(L, 1,
			lua_tostring(L, lua_upvalueindex(1))))->get();
		void *vp = lua_touserdata(L, lua_upvalueindex(2));
		func_ptr_type f = *((func_ptr_type *)vp);
		generic<Ret>::push(L, (t->*f)());
		return 1;
	}
};
template <typename T>
struct method_dispatch0<T, void>
{
	typedef void (T::*func_ptr_type) ();
	static int apply (lua_State *L)
	{
		T *t = ((shared_ptr<T> *)luaL_checkudata(L, 1,
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
		T *t = ((shared_ptr<T> *)luaL_checkudata(L, 1,
			lua_tostring(L, lua_upvalueindex(1))))->get();
		void *vp = lua_touserdata(L, lua_upvalueindex(2));
		func_ptr_type f = *((func_ptr_type *)vp);
		generic<Ret>::push(L, (t->*f)(generic<P1>::get(L, 2)));
		return 1;
	}
};
template <typename T, typename P1>
struct method_dispatch1<T, void, P1>
{
	typedef void (T::*func_ptr_type) (P1);
	static int apply (lua_State *L)
	{
		T *t = ((shared_ptr<T> *)luaL_checkudata(L, 1,
			lua_tostring(L, lua_upvalueindex(1))))->get();
		void *vp = lua_touserdata(L, lua_upvalueindex(2));
		func_ptr_type f = *((func_ptr_type *)vp);
		t->*f(generic<P1>::get(L, 2));
		return 0;
	}
};

template <typename T, typename Ret, typename P1, typename P2>
struct method_dispatch2
{
	typedef Ret (T::*func_ptr_type) (P1, P2);
	static int apply (lua_State *L)
	{
		T *t = ((shared_ptr<T> *)luaL_checkudata(L, 1,
			lua_tostring(L, lua_upvalueindex(1))))->get();
		void *vp = lua_touserdata(L, lua_upvalueindex(2));
		func_ptr_type f = *((func_ptr_type *)vp);
		generic<Ret>::push(L, (t->*f)(generic<P1>::get(L, 2),
			generic<P2>::get(L, 3)));
		return 1;
	}
};
template <typename T, typename P1, typename P2>
struct method_dispatch2<T, void, P1, P2>
{
	typedef void (T::*func_ptr_type) (P1, P2);
	static int apply (lua_State *L)
	{
		T *t = ((shared_ptr<T> *)luaL_checkudata(L, 1,
			lua_tostring(L, lua_upvalueindex(1))))->get();
		void *vp = lua_touserdata(L, lua_upvalueindex(2));
		func_ptr_type f = *((func_ptr_type *)vp);
		t->*f(generic<P1>::get(L, 2), generic<P2>::get(L, 3));
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

/*struct None {};
template <typename T, typename U>
struct Pair {};
typedef Pair<int, Pair<float, Pair<char, None> > > myTypeList;

template <typename T> struct Length {};
template <> struct Length<None> { static const int val = 0; };
template <typename U, typename V> struct Length<Pair<U, V> > { static const int val = Length<V>::val + 1; };

template <int n> struct Fact { static const int val = n * Fact<n-1>::val; };
template <> struct Fact<0> { static const int val = 1; };
typedef Fact<-1> breakme;*/
