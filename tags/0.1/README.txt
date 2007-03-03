luabridge v0.1
Readme - 1 March 2007
Copyright (C) 2007 by Nathan Reed.  All rights and priveleges reserved.

Introduction
------------

luabridge is a lightweight, dependency-free library for binding Lua to C++.
It is written against Lua 5.1.1 and compiles in Microsoft Visual C++ 7.1 or
higher and in g++ 3.3.5 or higher.

Compiling
---------

Compiling should be very simple.  Ensure that Lua is installed and its headers
are in your include path.  If you are using MSVC, load the provided solution
file and click build.  Otherwise, a Makefile is provided; just enter the
directory where you have installed luabridge and type `make'.

Using luabridge
---------------

luabridge is based on C++ template metaprogramming.  It contains template code
to automatically generate at compile-time the various Lua API calls necessary
to export your program's classes and functions to the Lua environment.

You will need to ensure that luabridge's include directory is in your include
path.  The only file that needs to be included is luabridge.hpp; it will
include all the implementation files.  You will also need to link with
libluabridge (release version) or libluabridged (debug version), which will be
made in the lib directory.  These are static libraries containing the small
amount of common binary code in luabridge.

If L is a pointer to an instance of lua_State, the following code creates a
luabridge module for registering C++ functions and classes to L:

	luabridge::module m(L);

Functions can then be registered as follows:

	m	.function("foo", &foo)
		.function("bar", &bar);

The "function" function returns a reference to m, so you can chain many
function definitions together.  The first argument is the name by which the
function will be available in Lua, and the second is the function's address.
luabridge will automatically detect the number (up to 5, by default) and type
of the parameters.  Functions registered this way will be available at the
global scope to Lua scripts executed by L.  Overloading of function names is
currently not supported.

Supported types for parameters and returns are:
 * Primitive types: int, bool, float, double (all converted to/from
	  Lua_number, since Lua does not distinguish different number types
 * Strings: const char * and std::string
 * Objects: pointers, references, and shared_ptrs to objects of registered
	  classes (more about shared_ptrs later)

C++ classes can be registered with Lua as follows:

	m	.class_<MyClass>("MyClass")
		.constructor<void (*) (/* parameter types */)>()
		.method("method1", &MyClass::method1)
		.method("method2", &MyClass::method2);

	m	.subclass_<MySubclass, MyBaseClass>("MySubclass")
		.constructor<...>
		...

The "class_" function registers a class; its constructor will be available as
a global function with name given as argument to class_.  The object returned
can then be used to register the constructor (no overloading is supported, so
there can only be one constructor) and methods.

luabridge cannot automatically determine the number and types of constructor
parameters like it can for functions and methods, so you must provide them.
This is done by letting the "constructor" function take a template parameter,
which must be a function pointer type.  The parameter types will be extracted
from this (the return type is ignored).

Method registration works just like function registration.  Virtual methods
work as expected.  Static methods are not yet supported.  Destructors are
registered automatically for each class.

To register a subclass of another class that has been registered to Lua, use
the "subclass" function.  This takes two template arguments: the class to be
registered, and its base class.  Inherited methods do not have to be
re-declared and will function normally in Lua.  If a class has a base class
that is *not* registered to Lua, there is no need to declare it as a subclass.

luabridge uses a built-in reference counted smart pointer implementation 
called shared_ptr for memory management.  It is necessary that all objects
that are created by Lua or exposed to Lua are referred to using shared_ptr
in C++, since C++ code may not be able to predict how long a Lua reference
to an object may last.  shared_ptr is declared in the luabridge namespace and
implements a strict subset of boost::shared_ptr's functionality.  If desired,
luabridge will use another shared_ptr implementation rather than its own;
simply #define USE_OTHER_SHARED_PTR before including luabridge.hpp to enable
this.  shared_ptr must be visible to the luabridge namespace, i.e. you will
need to write

	using boost::shared_ptr;

or

	namespace luabridge {
		using boost:;shared_ptr;
	}

to make the other shared_ptr visible to luabridge.

Limitations of luabridge
------------------------

luabridge v0.1 does not support:
 * More than 5 parameters on a function or method (although this can be
   increased by editing include/impl/typelist.hpp)
 * Static methods on classes
 * const methods on classes
 * Overloading operators
 * Overloaded functions, methods, or constructors
 * Properties
 * Packages (Lua namespaces)
 * STL list types, like std::vector, std::list, or std::map
 * Inheriting Lua classes from C++ classes

Development is continuing, and new releases will be published at:
https://svn.cs.pomona.edu/reedbeta/luabridge/tags/
The latest unstable version is always available at:
https://svn.cs.pomona.edu/reedbeta/luabridge/trunk
The subversion repository is world-readable, so you can checkout and even view
logs and prior revisions if you are so inclined.

If you are interested in contributing to luabridge, please contact me at:
nathaniel dot reed at gmail dot com.
