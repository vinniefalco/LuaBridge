Luabridge v0.2
Readme - 28 July 2007
Copyright (C) 2007 by Nathan Reed.  All rights and priveleges reserved.

Introduction
------------

Luabridge is a lightweight, dependency-free library for binding Lua to C++,
written against Lua 5.1.2.

Compiling
---------

Luabridge compiles correctly in g++ 4, MSVC 7.1, and MSVC 8.0.  Because of the
advanced template features it uses, I can't guarantee Luabridge will compile
correctly with anything else, but it is written in standard-compliant C++, so
if you have a compliant compiler you *should* be fine.

Compiling should be very simple.  Ensure that Lua is installed and its headers
are in your include path.  If you are using MSVC, load the provided solution
file and click build.  Otherwise, a Makefile is provided; just enter the
directory where you have decompressed Luabridge and type `make'.

One option you may have to set manually is the name of the Lua library.  On
some systems it is called 'lua' (liblua.a, liblua.so) and on others 'lua5.1'
(liblua5.1.a, liblua5.1.so).  Setting the make variable LUA_NAME lets you
specify this.  The default is 'lua'.

The distributed Luabridge source includes a test application, which serves to
verify that everything works as expected, and also demonstrates the code
necessary to use each of Luabridge's features.  You can run it by clicking
Execute (Ctrl+F5) in MSVC after building, or by typing `make test` at the
command line.  (In either case the command should be executed from the
directory in which Luabridge was decompressed.)  The application will print
'All test succeeded' if everything worked as expected, or an error message
otherwise.

Using Luabridge
---------------

Luabridge is based on C++ template metaprogramming.  It contains template code
to automatically generate at compile-time the various Lua API calls necessary
to export your program's classes and functions to the Lua environment.

You will need to ensure that Luabridge's include directory is in your include
path.  The only file that needs to be included is luabridge.hpp; it will
include all the implementation files.  You will also need to link with
libluabridge (release version) or libluabridged (debug version), which will be
made in the lib directory.  These are static libraries containing the small
amount of common binary code in Luabridge.

If L is a pointer to an instance of lua_State, the following code creates a
Luabridge module for registering C++ functions and classes to L:

	luabridge::module m(L);

Functions can then be registered as follows:

	m	.function("foo", &foo)
		.function("bar", &bar);

The 'function' function returns a reference to m, so you can chain many
function definitions together.  The first argument is the name by which the
function will be available in Lua, and the second is the function's address.
Luabridge will automatically detect the number (up to 8, by default) and type
of the parameters.  Functions registered this way will be available at the
global scope to Lua scripts executed by L.  Overloading of function names is
not supported, nor is it likely to be supported in the future.

Supported types for parameters and returns are:
 * Primitive types: int, float, double (all converted to/from
	  Lua_number, since Lua does not distinguish different number types),
	  bool
 * Strings: const char * and std::string
 * Objects: pointers, references, and shared_ptrs to objects of registered
	  classes (more about shared_ptrs later)

C++ classes can be registered with Lua as follows:

	m.class_<MyClass>("MyClass")
		.constructor<void (*) (/* parameter types */)>()
		.method("method1", &MyClass::method1)
		.method("method2", &MyClass::method2);

	m.subclass<MySubclass, MyBaseClass>("MySubclass")
		.constructor<...>
		...

The "class_" function registers a class; its constructor will be available as
a global function with name given as argument to class_.  The object returned
can then be used to register the constructor (no overloading is supported, so
there can only be one constructor) and methods.

Luabridge cannot automatically determine the number and types of constructor
parameters like it can for functions and methods, so you must provide them.
This is done by letting the 'constructor' function take a template parameter,
which must be a function pointer type.  The parameter types will be extracted
from this (the return type is ignored).  For example, to register a
constructor taking two parameters, one int and one const char *, you would
write:

	m.class_...
		.constructor<void (*) (int, const char *)>()

Method registration works just like function registration.  Virtual methods
work normally; no special syntax is needed.  Const methods are detected and
const-correctness is enforced, so if a function returns a const object (or
rather, a shared_ptr to a const object) to Lua, that reference to the object
will be considered const and only const methods will be called on it.
Destructors are registered automatically for each class.

Static methods may be registered using the 'static_method' function:

	m.class_...
		.static_method("method3", &MyClass::method3)

Luabridge also supports properties, which allow class data members to be read
and written from Lua as if they were variables.  You can expose a 'bare' C++
member variable to Lua, or wrap it in getter and setter functions.  The syntax
for registering properties is as follows:

	m.class_...
		.property_rw("property1", &MyClass::property1)
		.property_rw("property2", &MyClass::getter2, &MyClass::setter2)
		.property_ro("property3", &MyClass::property3)
		.property_ro("property4", &MyClass::getter4)

The first registration above gives Lua direct access to the 'property1' member
variable of MyClass objects; the second creates a property which acts like any
other variable in Lua but is retrieved and set through the 'getter2' and
'setter2' methods of MyClass.  The getter must take no parameters and return
a value, and the setter must take a value of the same type and return nothing.
The 'property_rw' function creates a readable and writeable property, while
'property_ro' creates a read-only one.

Static properties on classes are also supported, using 'static_property_rw'
and 'static_property_ro'.  Properties on modules, i.e. global properties, are
not yet supported, but will be in a future Luabridge release.

To register a subclass of another class that has been registered with Lua, use
the "subclass" function.  This takes two template arguments: the class to be
registered, and its base class.  Inherited methods do not have to be
re-declared and will function normally in Lua.  If a class has a base class
that is *not* registered with Lua, there is no need to declare it as a
subclass.

shared_ptr
----------

Luabridge uses a built-in reference counted smart pointer implementation 
called shared_ptr for memory management.  It is necessary that all objects
that are created by Lua or exposed to Lua are referred to using shared_ptr
in C++, since C++ code may not be able to predict how long a Lua reference
to an object may last.  shared_ptr is declared in the luabridge namespace and
implements a strict subset of boost::shared_ptr's functionality.  If desired,
Luabridge will use another shared_ptr implementation rather than its own;
simply #define USE_OTHER_SHARED_PTR before including luabridge.hpp to enable
this.  shared_ptr must be visible to the luabridge namespace, i.e. you will
need to write

	using boost::shared_ptr;

or

	namespace luabridge {
		using boost::shared_ptr;
	}

to make the other shared_ptr visible to Luabridge.

Limitations of Luabridge
------------------------

Luabridge v0.2 does not support:
 * More than 8 parameters on a function or method (although this can be
   increased by editing include/impl/typelist.hpp)
 * Returning objects from functions other than through a shared_ptr
 * Passing objects to functions by value
 * Overloading operators
 * Overloaded functions, methods, or constructors
 * Global properties
 * Packages/namespaces
 * Automatic conversion between STL container types and Lua tables
 * Inheriting Lua classes from C++ classes

Development is continuing, and new releases will be published at the project
website: http://luabridge.sourceforge.net
The latest (unstable) version is always available for SVN checkout at:
https://luabridge.svn.sourceforge.net/svnroot/luabridge/trunk

If you are interested in contributing to Luabridge, please contact me at:
nathaniel dot reed at gmail dot com, or send email to me through sourceforge.
