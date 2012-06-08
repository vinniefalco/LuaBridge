<a href="http://lua.org">
<img src="http://vinniefalco.github.com/LuaBridgeDemo/powered-by-lua.png">
</a><br>

# LuaBridge 1.0.2

[LuaBridge][3] is a lightweight, dependency-free library for making C++ data,
functions, and classes available to [Lua][5]: A powerful, fast, lightweight,
embeddable scripting language. LuaBridge has been tested and works with Lua
revisions starting from 5.1.5., although it should work in any version of Lua
from 5.1.0 and later.

LuaBridge offers the following features:

- Nothing to compile, just include one header file!

- Simple, light, and nothing else needed (like Boost).

- Supports different object lifetime management models.

- Convenient, type-safe access to the Lua stack.

- Automatic function parameter type binding.

- Does not require C++11.

LuaBridge is distributed as a single header file. You simply add 
`#include "LuaBridge.h"` where you want to bind your functions, classes, and
variables. There are no additional source files, no compilation settings, and
no Makefiles or IDE-specific project files. LuaBridge is easy to integrate.
A few additional header files provide optional features. Like the main header
file, these are simply used via `#include`. No additional source files need
to be compiled.

C++ concepts like variables and classes are made available to Lua through a
process called _registration_. Because Lua is weakly typed, the resulting
structure is not rigid. The API is based on C++ template metaprogramming. It
contains template code to automatically generate at compile-time the various
Lua C API calls necessary to export your program's classes and functions to
the Lua environment.

### Version

LuaBridge repository branches are as follows:

- **[master][7]**: Tagged, stable release versions.

- **[release][8]**: Tagged candidates for imminent release.

- **[develop][9]**: Work in progress.

## LuaBridge Demo and Tests

LuaBridge provides both a command line program and a stand-alone graphical
program for compiling and running the test suite. The graphical program brings
up an interactive window where you can enter execute Lua statements in a
persistent environment. This application is cross platform and works on
Windows, Mac OS, iOS, Android, and GNU/Linux systems with X11. The stand-alone
program should work anywhere. Both of these applications include LuaBridge,
Lua version 5.2, and the code necessary to produce a cross platform graphic
application. They are all together in a separate repository, with no
additional dependencies, available on Github at [LuaBridge Demo and Tests][4].
This is what the GUI application looks like, along with the C++ code snippet
for registering the two classes:
  
<a href="https://github.com/vinniefalco/LuaBridgeDemo">
<img src="http://vinniefalco.github.com/LuaBridgeDemo/LuaBridgeDemoScreenshot1.0.2.png">
</a><br>

## Registration

There are five types of objects that LuaBridge can register:

- **Data**: Global variables, data members, and static data members.

- **Functions**: Global functions, member functions, and static member
                  functions.

- **CFunctions**: A regular function, member function, or static member
                  function that uses the `lua_CFunction` calling convention.

- **Namespaces**: A namespace is simply a table containing registrations of
                  functions, data, properties, and other namespaces.

- **Properties**: Global properties, property members, and static property
                  members. These appear like data to Lua, but are implemented
                  using get and set functions on the C++ side.

Both data and properties can be marked as _read-only_ at the time of
registration. This is different from `const`; the values of these objects can
be modified on the C++ side, but Lua scripts cannot change them. Code samples
that follow are in C++ or Lua, depending on context. For brevity of exposition
code samples in C++ assume the traditional variable `lua_State* L` is defined,
and that a `using namespace luabridge` using-directive is in effect.

### Namespaces

All LuaBridge registrations take place in a _namespace_. When we refer to a
_namespace_ we are always talking about a namespace in the Lua sense, which is
implemented using tables. The namespace need not correspond to a C++ namespace;
in fact no C++ namespaces need to exist at all unless you want them to.
LuaBridge namespaces are visible only to Lua scripts; they are used as a
logical grouping tool. To obtain access to the global namespace we write:

    getGlobalNamespace (L);

This returns an object on which further registrations can be performed. The
subsequent registrations will go into the global namespace, a practice which
is not recommended. Instead, we can add our own namespace by writing:

    getGlobalNamespace (L)
      .beginNamespace ("test");

This creates a table in `_G` called "test". Since we have not performed any
registrations, this table will be empty except for some bookkeeping key/value
pairs. LuaBridge reserves all identifiers that start with a double underscore.
So `__test` would be an invalid name (although LuaBridge will silently accept
it). Functions like `beginNamespace` return the corresponding object on which
we can make more registrations. Given:

    getGlobalNamespace (L)
      .beginNamespace ("test")
        .beginNamespace ("detail")
        .endNamespace ()
        .beginNamespace ("utility")
        .endNamespace ()
      .endNamespace ();

The results are accessible to Lua as `test`, `test.detail`, and
`test.utility`. Here we introduce the `endNamespace` function; it returns an
object representing the original enclosing namespace. All LuaBridge functions
which  create registrations return an object upon which subsequent
registrations can be made, allowing for an unlimited number of registrations
to be chained together using the dot operator `.`. Adding two objects with the
same name, in the same namespace, results in undefined behavior (although
LuaBridge will silently accept it).

A namespace can be re-opened later to add more functions. This lets you split
up the registration between different source files. These are equivalent:

    getGlobalNamespace (L)
      .beginNamespace ("test")
        .addFunction ("foo", foo)
      .endNamespace ();

    getGlobalNamespace (L)
      .beginNamespace ("test")
        .addFunction ("bar", bar)
      .endNamespace ();

and

    getGlobalNamespace (L)
      .beginNamespace ("test")
        .addFunction ("foo", foo)
        .addFunction ("bar", bar)
      .endNamespace ();


### Data, Properties, Functions, and CFunctions.

These are registered into a namespace using `addVariable`, `addProperty`,
`addFunction`, and `addCFunction`. When registered functions are called by
scripts, LuaBridge automatically takes care of the conversion of arguments
into the appropriate data type when doing so is possible. This automated
system works for the function's return value, and up to 8 parameters although
more can be added by extending the templates. Pointers, references, and
objects of class type as parameters are treated specially, and explained
later. If we have:

    int globalVar;
    static float staticVar;

    std::string stringProperty;
    std::string getString () { return stringProperty; }
    void setString (std::string s) { return s; }

    int foo () { return 42; }
    void bar (char const*) { }
    int cFunc (lua_State* L) { return 0; }

  These are registered with:

    getGlobalNamespace (L)
      .beginNamespace ("test")
        .addVariable ("var1", &globalVar)
        .addVariable ("var2", &staticVar, false)     // read-only
        .addProperty ("prop1", getString, setString)
        .addProperty ("prop2", getString)            // read only
        .addFunction ("foo", foo)
        .addFunction ("bar", bar)
        .addCFunction ("cfunc", cFunc)
      .endNamespace ();

Variables can be marked _read-only_ by passing `false` in the second optional
parameter. If the parameter is omitted, `true` is used making the variable
read/write. Properties are marked read-only by omitting the set function.
After the registrations above, the following Lua identifiers are valid:

    test        -- a namespace
    test.var1   -- a lua_Number variable
    test.var2   -- a read-only lua_Number variable
    test.prop1  -- a lua_String property
    test.prop2  -- a read-only lua_String property
    test.foo    -- a function returning a lua_Number
    test.bar    -- a function taking a lua_String as a parameter
    test.cfunc  -- a function with a variable argument list and multi-return

Note that `test.prop1` and `test.prop2` both refer to the same value. However,
since `test.prop2` is read-only, assignment does not work. These Lua
statements have the stated effects:

    test.var1 = 5         -- okay
    test.var2 = 6         -- error: var2 is not writable
    test.prop1 = "Hello"  -- okay
    test.prop1 = 68       -- okay, Lua converts the number to a string.
    test.prop2 = "bar"    -- error: prop2 is not writable

    test.foo ()           -- calls foo and discards the return value
    test.var1 = foo ()    -- calls foo and stores the result in var1
    test.bar ("Employee") -- calls bar with a string
    test.bar (test)       -- error: bar expects a string not a table

LuaBridge does not support overloaded functions nor is it likely to in the
future. Since Lua is dynamically typed, any system that tries to resolve a set
of parameters passed from a script will face considerable ambiguity when
trying to choose an appropriately matching C++ function signature.

### Classes

A class registration is opened using either `beginClass` or `deriveClass` and
ended using `endClass`. Once registered, a class can later be re-opened for
more registrations using `beginClass`. However, `deriveClass` should only be
used once. To add more registrations to an already registered derived class,
use `beginClass`. These declarations:

    struct A {
      static int staticData;
      static float staticProperty;
        
      static float getStaticProperty () { return staticProperty; }
      static void setStaticProperty (float f) { staticProperty = f; }
      static void staticFunc () { }

      static int staticCFunc () { return 0; }

      std::string dataMember;

      char dataProperty;
      char getProperty () const { return dataProperty; }
      void setProperty (char v) { dataProperty = v; }

      void func1 () { }
      virtual void virtualFunc () { }

      int cfunc (lua_State* L) { return 0; }
    };

    struct B : public A {
      double dataMember2;

      void func1 () { }
      void func2 () { }
      void virtualFunc () { }
    };

    int A::staticData;
    float A::staticProperty;

Are registered using:

    getGlobalNamespace (L)
      .beginNamespace ("test")
        .beginClass <A> ("A")
          .addStaticData ("staticData", &A::staticData)
          .addStaticProperty ("staticProperty", &A::staticProperty)
          .addStaticFunction ("staticFunc", &A::staticFunc)
          .addStaticCFunction ("staticCFunc", &A::staticCFunc)
          .addData ("data", &A::dataMember)
          .addProperty ("prop", &A::getProperty, &A::setProperty)
          .addFunction ("func1", &A::func1)
          .addFunction ("virtualFunc", &A::virtualFunc)
          .addCFunction ("cfunc", &A::cfunc)
        .endClass ()
        .deriveClass <B, A> ("B")
          .addData ("data", &B::dataMember2)
          .addFunction ("func1", &B::func1)
          .addFunction ("func2", &B::func2)
        .endClass ()
      .endClass ();

Method registration works just like function registration.  Virtual methods
work normally; no special syntax is needed. const methods are detected and
const-correctness is enforced, so if a function returns a const object (or
a container holding to a const object) to Lua, that reference to the object
will be considered const and only const methods can be called on it.
Destructors are registered automatically for each class.

As with regular variables and properties, class data and properties can be
marked read-only by passing false in the second parameter, or omitting the set
set function respectively. The `deriveClass` takes two template arguments: the
class to be registered, and its base class.  Inherited methods do not have to
be re-declared and will function normally in Lua. If a class has a base class
that is **not** registered with Lua, there is no need to declare it as a
subclass.

### Property Member Proxies

Sometimes when registering a class which comes from a third party library, the
data is not exposed in a way that can be expressed as a pointer to member,
there are no get or set functions, or the get and set functons do not have the
right function signature. Since the class declaration is closed for changes,
LuaBridge provides allows a _property member proxy_. This is a pair of get
and set flat functions which take as their first parameter a pointer to
the object. This is easily understood with the following example:

    // Third party declaration, can't be changed
    struct Vec 
    {
      float coord [3];
    };

Taking the address of an array element, e.g. `&Vec::coord [0]` results in an
error instead of a pointer-to-member. The class is closed for modifications,
but we want to export Vec objects to Lua using the familiar object notation.
To do this, first we add a "helper" class:

    struct VecHelper
    {
      template <unsigned index>
      static float get (Vec const* vec)
      {
        return vec->coord [index];
      }

      template <unsigned index>
      static void set (Vec* vec, float value)
      {
        vec->coord [index] = value;
      }
    };

This helper class is only used to provide property member proxies. `Vec`
continues to be used in the C++ code as it was before. Now we can register
the `Vec` class with property member proxies for `x`, `y`, and `z`:

    getGlobalNamespace (L)
      .beginNamespace ("test")
        .beginClass <Vec> ("Vec")
          .addProperty ("x", &VecHelper::get <0>, &VecHelper::set <0>)
          .addProperty ("y", &VecHelper::get <1>, &VecHelper::set <1>)
          .addProperty ("z", &VecHelper::get <2>, &VecHelper::set <2>)
        .endClass ()
      .endNamespace ();

### Constructors

A single constructor may be added for a class using `addConstructor`.
LuaBridge cannot automatically determine the number and types of constructor
parameters like it can for functions and methods, so you must provide them.
This is done by specifying the signature of the desired constructor function
as the first template parameter to `addConstructor`. The parameter types will
be extracted from this (the return type is ignored).  For example, these
statements register constructors for the given classes:

    struct A {
      A ();
    };

    struct B {
      explicit B (char const* s, int nChars);
    };

    getGlobalNamespace (L)
      .beginNamespace ("test")
        .beginClass <A> ("A")
          .addConstructor <void (*) (void)> ()
        .endClass ()
        .beginClass <B> ("B")
          .addConstructor <void (*) (char const*, int)> ()
        .endClass ();
      .endNamespace ()

Constructors added in this fashion are called from Lua using the fully
qualified name of the class. This Lua code will create instances of `A` and
`B`

    a = test.A ()           -- Create a new A.
    b = test.B ("hello", 5) -- Create a new B.
    b = test.B ()           -- Error: expected string in argument 1

## The Lua Stack

In the Lua C API, all operations on the `lua_State` are performed through the
Lua stack. In order to pass parameters back and forth between C++ and Lua,
LuaBridge uses specializations of this template class concept:

    template <class T>
    struct Stack
    {
      static void push (lua_State* L, T t);
      static T get (lua_State* L, int index);
    };

The Stack template class specializations are used automatically for variables,
properties, data members, property members, function arguments and return
values. These basic types are supported:

- `bool`
- `char`, converted to a string of length one.
- `char const*` and `std::string` strings.
- Integers, `float`, and `double`, converted to `Lua_number`.

User-defined types which are convertible to one of the basic types are
possible, simply provide a `Stack <>` specialization in the `luabridge`
namespace for your user-defined type, modeled after the existing types.
For example, here is a specialization for a [juce::String][6]:

    template <>
    struct Stack <juce::String>
    {
      static void push (lua_State* L, juce::String s)
      {
        lua_pushstring (L, s.toUTF8 ());
      }

      static juce::String get (lua_State* L, int index)
      {
        return juce::String (luaL_checkstring (L, index));
      }
    };

### The `lua_State*`

Sometimes it is convenient from within a bound function or member function
to gain access to the `lua_State*` normally available to a `lua_CFunction`.
With LuaBridge, all you need to do is add a `lua_State*` as the last
parameter of your bound function:

    void useState (lua_State* L);

    getGlobalNamespace (L).addFunction ("useState", &useState);

You can still include regular arguments while receiving the state:

    void useStateAndArgs (int i, std::string s, lua_State* L);

    getGlobalNamespace (L).addFunction ("useStateAndArgs", &useStateAndArgs);

When a script calls `useStateAndArgs`, it passes only the integer and string
parameters. LuaBridge takes care of inserting the `lua_State*` into the
argument list for the corresponding C++ function. This will work correctly
even for the state created by coroutines. Undefined behavior results if
the `lua_State*` is not the last parameter.

### Class Object Types

An object of a registered class `T` may be passed to Lua as:

- `T*` or `T&`: Passed by reference, with _C++ lifetime_.
- `T const*` or `T const&`: Passed by const reference, with _C++ lifetime_.
- `T` or `T const`: Passed by value (a copy), with _Lua lifetime_.

### C++ Lifetime

The creation and deletion of objects with _C++ lifetime_ is controlled by
the C++ code. Lua does nothing when it garbage collects a reference to such an
object. Specifically, the object's destructor is not called (since C++ owns
it). Care must be taken to ensure that objects with C++ lifetime are not
deleted while still being referenced by a `lua_State*`, or else undefined
behavior results. In the previous examples, an instance of `A` can be passed
to Lua with C++ lifetime, like this:

    A a;

    push (L, &a);             // pointer to 'a', C++ lifetime
    lua_setglobal (L, "a");

    push (L, (A const*)&a);   // pointer to 'a const', C++ lifetime
    lua_setglobal (L, "ac");

    push <A const*> (L, &a);  // equivalent to push (L, (A const*)&a)
    lua_setglobal (L, "ac2");

    push (L, new A);          // compiles, but will leak memory
    lua_setglobal (L, "ap");

### Lua Lifetime

When an object of a registered class is passed by value to Lua, it will have
_Lua lifetime_. A copy of the passed object is constructed inside the
userdata. When Lua has no more references to the object, it becomes eligible
for garbage collection. When the userdata is collected, the destructor for
the class will be called on the object. Care must be taken to ensure that
objects with Lua lifetime are not accessed by C++ after they are garbage
collected, or else undefined behavior results. An instance of `B` can be
passed to Lua with Lua lifetime this way:

    B b;

    push (L, b);                    // Copy of b passed, Lua lifetime.
    lua_setglobal (L, "b");

Given the previous code segments, these Lua statements are applicable:

    print (test.A.staticData)       -- Prints the static data member.
    print (test.A.staticProperty)   -- Prints the static property member.
    test.A.staticFunc ()            -- Calls the static method.

    print (a.data)                  -- Prints the data member.
    print (a.prop)                  -- Prints the property member.
    a:func1 ()                      -- Calls A::func1 ().
    test.A.func1 (a)                -- Equivalent to a:func1 ().
    test.A.func1 ("hello")          -- Error: "hello" is not a class A.
    a:virtualFunc ()                -- Calls A::virtualFunc ().

    print (b.data)                  -- Prints B::dataMember.
    print (b.prop)                  -- Prints inherited property member.
    b:func1 ()                      -- Calls B::func1 ().
    b:func2 ()                      -- Calls B::func2 ().
    test.B.func2 (a)                -- Error: a is not a class B.
    test.A.func1 (b)                -- Calls A::func1 ().
    b:virtualFunc ()                -- Calls B::virtualFunc ().
    test.B.virtualFunc (b)          -- Calls B::virtualFunc ().
    test.A.virtualFunc (b)          -- Calls B::virtualFunc ().
    test.B.virtualFunc (a)          -- Error: a is not a class B.

    a = nil; collectgarbage ()      -- 'a' still exists in C++.
    b = nil; collectgarbage ()      -- Lua calls ~B() on the copy of b.

When Lua script creates an object of class type using a registered
constructor, the resulting value will have Lua lifetime. After Lua no longer
references the object, it becomes eligible for garbage collection. You can
still pass these to C++, either by reference or by value. If passed by
reference, the usual warnings apply about accessing the reference later,
after it has been garbage collected.

### Pointers, References, and Pass by Value

When C++ objects are passed from Lua back to C++ as arguments to functions,
or set as data members, LuaBridge does its best to automate the conversion.
Using the previous definitions, the following functions may be registered
to Lua:

    void func0 (A a);
    void func1 (A* a);
    void func2 (A const* a);
    void func3 (A& a);
    void func4 (A const& a);

Executing this Lua code will have the prescribed effect:

    func0 (a)   -- Passes a copy of a, using A's copy constructor.
    func1 (a)   -- Passes a pointer to a.
    func2 (a)   -- Passes a pointer to a const a.
    func3 (a)   -- Passes a reference to a.
    func4 (a)   -- Passes a reference to a const a.

In the example above, all functions can read the data members and property
members of `a`, or call const member functions of `a`. Only `func0`, `func1`
and `func3` can modify the data members and data properties, or call
non-const member functions of `a`.

The usual C++ inheritance and pointer assignment rules apply. Given:

    void func5 (B b);
    void func6 (B* b);

These Lua statements hold:

    func5 (b)   - Passes a copy of b, using B's copy constructor.
    func6 (b)   - Passes a pointer to b.
    func6 (a)   - Error: Pointer to B expected.
    func1 (b)   - Okay, b is a subclass of a.

When a pointer or pointer to const is passed to Lua and the pointer is null
(zero), LuaBridge will pass Lua a `nil` instead. When Lua passes a `nil`
to C++ where a pointer is expected, a null (zero) is passed instead.
Attempting to pass a null pointer to a C++ function expecting a reference
results in `lua_error` being called.

## Shared Lifetime

LuaBridge supports a "shared lifetime" model: dynamically allocated and
reference counted objects whose ownership is shared by both Lua and C++.
The object remains in existence until there are no remaining C++ or Lua
references, and Lua performs its usual garbage collection cycle. A container
is recognized by a specialization of the `ContainerTraits` template class.
LuaBridge will automatically recognize when a data type is a container when
the correspoding specialization is present. Two styles of containers come with
LuaBridge, including the necessary specializations:

### The `RefCountedObjectPtr` Container

This is an intrusive style container. Your existing class declaration must be
changed to be also derived from `RefCountedObject`. Given `class T`, derived
from `RefCountedObject`, the container `RefCountedObjectPtr <T>` may be used.
In order for reference counts to be maintained properly, all C++ code must
store a container instead of the pointer. This is similar in style to
`std::shared_ptr` although there are slight differences. For example:

    // A is reference counted.
    struct A : public RefCountedObject
    {
      void foo () { }
    };

    struct B
    {
      RefCountedObjectPtr <A> a; // holds a reference to A
    };

    void bar (RefCountedObjectPtr <A> a)
    {
      a->foo ();
    }

### The `RefCountedPtr` Container

This is a non intrusive reference counted pointer. The reference counts are
kept in a global hash table, which does incur a small performance penalty.
However, it does not require changing any already existing class declarations.
This is especially useful when the classes to be registered come from a third
party library and cannot be modified. To use it, simply wrap all pointers
to class objects with the container instead:

    struct A
    {
      void foo () { }
    };

    struct B
    {
      RefCountedPtr <A> a;
    };

    RefCountedPtr <A> createA ()
    {
      return new A;
    }

    void bar (RefCountedPtr <A> a)
    {
      a->foo ();
    }

    void callFoo ()
    {
      bar (createA ());

      // The created A will be destroyed
      // when we leave this scope
    }

### Custom Containers

If you have your own container, you must provide a specialization of
`ContainerTraits` in the `luabridge` namespace for your type before it will be
recognized by LuaBridge (or else the code will not compile):

    template <class T>
    struct ContainerTraits <CustomContainer <T> >
    {
      typedef typename T Type;

      static T* get (CustomContainer <T> const& c)
      {
        return c.getPointerToObject ();
      }
    };

Standard containers like `std::shared_ptr` or `boost::shared_ptr` **will not
work**. This is because of type erasure; when the object goes from C++ to
Lua and back to C++, there is no way to associate the object with the
original container. The new container is constructed from a pointer to the
object instead of an existing container. The result is undefined behavior
since there are now two sets of reference counts.

### Container Construction

When a constructor is registered for a class, there is an additional
optional second template parameter describing the type of container to use.
If this parameter is specified, calls to the constructor will create the
object dynamically, via operator new, and place it a container of that
type. The container must have been previously specialized in
`ContainerTraits`, or else a compile error will result. This code will
register two objects, each using a constructor that creates an object
with Lua lifetime using the specified container:

    class C : public RefCountedObject
    {
      C () { }
    };

    class D
    {
      D () { }
    };

    getGlobalNamespace (L)
      .beginNamespace ("test")
        .beginClass <C> ("C")
          .addConstructor <void (*) (void), RefCountedObjectPtr <C> > ()
        .endClass ()
        .beginClass <D> ("D")
          .addConstructor <void (*) (void), RefCountedPtr <D> > ()
        .endClass ();
      .endNamespace ()

### Mixing Lifetimes

Mixing object lifetime models is entirely possible, subject to the usual
caveats of holding references to objects which could get deleted. For
example, C++ can be called from Lua with a pointer to an object of class
type; the function can modify the object or call non-const data members.
These modifications are visible to Lua (since they both refer to the same
object). An object store in a container can be passed to a function expecting
a pointer. These conversion work seamlessly.

## Security

The metatables and userdata that LuaBridge creates in the `lua_State*` are
protected using a security system, to eliminate the possibility of undefined
behavior resulting from scripted manipulation of the environment. The
security system has these components:

- Class and const class tables use the 'table proxy' technique. The
  corresponding metatables have `__index` and `__newindex` metamethods,
  so these class tables are immutable from Lua.

- Metatables have `__metatable` set to a boolean value. Scripts cannot
  obtain the metatable from a LuaBridge object.

- Classes are mapped to metatables through the registry, which Lua scripts
  cannot access. The global environment does not expose metatables

- Metatables created by LuaBridge are tagged with a lightuserdata key which
  is unique in the process. Other libraries cannot forge a LuaBridge
  metatable.

This security system can be easily bypassed if scripts are given access to
the debug library (or functionality similar to it, i.e. a raw `getmetatable`).
The security system can also be defeated by C code in the host, either by
revealing the unique lightuserdata key to another module or by putting a
LuaBridge metatable in a place that can be accessed by scripts.

When a class member function is called, or class property member accessed,
the `this` pointer is type-checked. This is because member functions exposed
to Lua are just plain functions that usually get called with the Lua colon
notation, which passes the object in question as the first parameter. Lua's
dynamic typing makes this type-checking mandatory to prevent undefined
behavior resulting from improper use.

If a type check error occurs, LuaBridge uses the `lua_error` mechanism to
trigger a failure. A host program can always recover from an error through
the use of `lua_pcall`; proper usage of LuaBridge will never result in
undefined behavior.

## Limitations 

LuaBridge does not support:

- Enumerated constants
- More than 8 parameters on a function or method (although this can be
  increased by adding more `TypeListValues` specializations).
- Overloaded functions, methods, or constructors.
- Global variables (variables must be wrapped in a named scope).
- Automatic conversion between STL container types and Lua tables.
- Inheriting Lua classes from C++ classes.
- Passing nil to a C++ function that expects a pointer or reference.
- Standard containers like `std::shared_ptr`.

## Development

[Github][3] is the new official home for LuaBridge. The old SVN repository is
deprecated since it is no longer used, or maintained. The original author has
graciously passed the reins to Vinnie Falco for maintaining and improving the
project. To obtain the older official releases, checkout the tags from 0.2.1
and earlier.

If you are an existing LuaBridge user, a new LuaBridge user, or a potential
LuaBridge user, we welcome your input, feedback, and contributions. Feel
free to open Issues, or fork the repository. All questions, comments,
suggestions, and/or proposed changes will be handled by the new maintainer.

## License

Copyright (C) 2012, [Vinnie Falco][1] ([e-mail][0]) <br>
Copyright (C) 2007, Nathan Reed <br>
  
Portions from The Loki Library: <br>
Copyright (C) 2001 by Andrei Alexandrescu

License: The [MIT License][2]

Older versions of LuaBridge up to and including 0.2 are distributed under the
BSD 3-Clause License. See the corresponding license file in those versions
for more details.

[0]: mailto:vinnie.falco@gmail.com "Vinnie Falco (Email)"
[1]: http://www.vinniefalco.com "Vinnie Falco"
[2]: http://www.opensource.org/licenses/mit-license.html "The MIT License"
[3]: https://github.com/vinniefalco/LuaBridge "LuaBridge"
[4]: https://github.com/vinniefalco/LuaBridgeDemo "LuaBridge Demo"
[5]: http://lua.org "The Lua Programming Language"
[6]: http://www.rawmaterialsoftware.com/juce/api/classString.html "juce::String"
[7]: https://github.com/vinniefalco/LuaBridge "LuaBridge master branch"
[8]: https://github.com/vinniefalco/LuaBridge/tree/release "LuaBridge release branch"
[9]: https://github.com/vinniefalco/LuaBridge/tree/develop "LuaBridge develop branch"
