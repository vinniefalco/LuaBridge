/*
 * stack.hpp - Copyright (C) 2007 by Nathan Reed
 * Type-dispatch functions for manipulating the Lua stack.
 */

/*
 * These totally generic functions are unimplemented, causing a compiler
 * error if they are called.  We don't know how to send objects of arbitrary
 * types to and from Lua.  Following are specializations of this structure
 * that allow specific types to be handled in specific ways.
 */

template <typename T>
struct tdstack
{
private:
	static void push (lua_State *L, T data);
	static T get (lua_State *L, int index);
};

/*
 * Pointers and references: getting is done by retrieving the address from
 * the Lua-owned shared_ptr, but pushing is not allowed since luabridge
 * has no idea of the ownership semantics of these objects.  You can only
 * push shared_ptrs, not naked pointers and references.
 */

template <typename T>
struct tdstack <T*>
{
private:
	static void push (lua_State *L, T *data);
public:
	static T* get (lua_State *L, int index)
	{
		return ((shared_ptr<T> *)
			checkclass(L, index, classname<T>::name()))->get();
	}
};

template <typename T>
struct tdstack <const T *>
{
private:
	static void push (lua_State *L, const T *data);
public:
	static const T* get (lua_State *L, int index)
	{
		std::string constname = std::string("const ") + classname<T>::name();
		return ((shared_ptr<const T> *)
			checkclass(L, index, constname.c_str()))->get();
	}
};

template <typename T>
struct tdstack <T* const>
{
private:
	static void push (lua_State *L, T * const data);
public:
	static T* const get (lua_State *L, int index)
	{
		return ((shared_ptr<T> *)
			checkclass(L, index, classname<T>::name()))->get();
	}
};

template <typename T>
struct tdstack <const T* const>
{
private:
	static void push (lua_State *L, const T * const data);
public:
	static const T* const get (lua_State *L, int index)
	{
		std::string constname = std::string("const ") + classname<T>::name();
		return ((shared_ptr<const T> *)
			checkclass(L, index, constname.c_str()))->get();
	}
};

template <typename T>
struct tdstack <T&>
{
private:
	static void push (lua_State *L, T &data);
public:
	static T& get (lua_State *L, int index)
	{
		return *((shared_ptr<T> *)
			checkclass(L, index, classname<T>::name()))->get();
	}
};

template <typename T>
struct tdstack <const T&>
{
private:
	static void push (lua_State *L, const T &data);
public:
	static const T& get (lua_State *L, int index)
	{
		std::string constname = std::string("const ") + classname<T>::name();
		return *((shared_ptr<const T> *)
			checkclass(L, index, constname.c_str()))->get();
	}
};

/*
 * shared_ptr: we can push these.
 * There is a specialization for const types, which produces a Lua userdata
 * whose metatable is the class's const metatable.
 */

template <typename T>
struct tdstack <shared_ptr<T> >
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
			checkclass(L, index, classname<T>::name());
	}
};

template <typename T>
struct tdstack <shared_ptr<const T> >
{
	static void push (lua_State *L, shared_ptr<const T> data)
	{
		// Make sure we don't try to push ptrs to objects of
		// unregistered classes or primitive types
		assert(classname<T>::name() != classname_unknown);

		// Allocate a new userdata and construct the pointer in-place there
		void *block = lua_newuserdata(L, sizeof(shared_ptr<const T>));
		new(block) shared_ptr<const T>(data);

		// Set the userdata's metatable
		std::string constname = std::string("const ") + classname<T>::name();
		luaL_getmetatable(L, constname.c_str());
		lua_setmetatable(L, -2);
	}
	static shared_ptr<const T> get (lua_State *L, int index)
	{
		std::string constname = std::string("const ") + classname<T>::name();
		return *(shared_ptr<const T> *)
			checkclass(L, index, constname.c_str());
	}
};

/*
 * Primitive types, including const char * and std::string
 */

template <>
struct tdstack <float>
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
struct tdstack <double>
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
struct tdstack <int>
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
struct tdstack <bool>
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
struct tdstack <const char *>
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
struct tdstack <std::string>
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
struct tdstack <const std::string &>
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

/*
 * Subclass of a type/value list, constructable from the Lua stack.
 */

template <typename Typelist, int start = 1>
struct arglist {};

template <int start>
struct arglist <nil, start>:
	public typevallist<nil>
{
	arglist (lua_State *L) { (void)L; }
};

template <typename Head, typename Tail, int start>
struct arglist <typelist<Head, Tail>, start>:
	public typevallist<typelist<Head, Tail> >
{
	arglist (lua_State *L):
		typevallist<typelist<Head, Tail> >
			(tdstack<Head>::get(L, start),
			arglist<Tail, start + 1>(L))
	{}
};

