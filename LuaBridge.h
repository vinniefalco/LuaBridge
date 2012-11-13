//==============================================================================
/*
  https://github.com/vinniefalco/LuaBridge
  https://github.com/vinniefalco/LuaBridgeDemo
  
  Copyright (C) 2012, Vinnie Falco <vinnie.falco@gmail.com>
  Copyright (C) 2007, Nathan Reed

  License: The MIT License (http://www.opensource.org/licenses/mit-license.php)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

  This file incorporates work covered by the following copyright and
  permission notice:  

    The Loki Library
    Copyright (c) 2001 by Andrei Alexandrescu
    This code accompanies the book:
    Alexandrescu, Andrei. "Modern C++ Design: Generic Programming and Design 
        Patterns Applied". Copyright (c) 2001. Addison-Wesley.
    Permission to use, copy, modify, distribute and sell this software for any 
        purpose is hereby granted without fee, provided that the above copyright 
        notice appear in all copies and that both that copyright notice and this 
        permission notice appear in supporting documentation.
    The author or Addison-Welsey Longman make no representations about the 
        suitability of this software for any purpose. It is provided "as is" 
        without express or implied warranty.
*/
//==============================================================================

#ifndef LUABRIDGE_LUABRIDGE_HEADER
#define LUABRIDGE_LUABRIDGE_HEADER

#include <stdexcept>
#include <typeinfo>
#include <string.h>

//==============================================================================
/**
  @mainpage LuaBridge: Simple C++ to Lua bindings.

  @details

  <a href="http://lua.org">
  <img src="http://vinniefalco.github.com/LuaBridgeDemo/powered-by-lua.png">
  </a><br>

  # LuaBridge 1.0.3

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
        .endNameSpace ();

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
          .endClass ()
        .endNamespace ();

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
*/

#include <cassert>
#include <string>

namespace luabridge
{

/**
  Since the throw specification is part of a function signature, the FuncTraits
  family of templates needs to be specialized for both types. The LUABRIDGE_THROWSPEC
  macro controls whether we use the 'throw ()' form, or 'noexcept' (if C++11
  is available) to distinguish the functions.
*/
#if defined (__APPLE_CPP__) || defined(__APPLE_CC__) || defined(__clang__) || defined(__GNUC__) || \
    (defined (_MSC_VER) && (_MSC_VER >= 1700))
// Do not define LUABRIDGE_THROWSPEC since the Xcode and gcc  compilers do not
// distinguish the throw specification in the function signature.
#else
#define LUABRIDGE_THROWSPEC throw()
#endif

//==============================================================================
/**
  Templates for extracting type information.

  These templates are used for extracting information about types used in
  various ways.
*/

//==============================================================================

template <typename T>
struct TypeInfo
{
  typedef T Type;
  static bool const is_const     = false;
  static bool const is_pointer   = false;
  static bool const is_reference = false;
};

template <typename T>
struct TypeInfo <T const>
{
  typedef T Type;
  static bool const is_const     = true;
  static bool const is_pointer   = false;
  static bool const is_reference = false;
};

template <typename T>
struct TypeInfo <T*>
{
  typedef T Type;
  static bool const is_const     = false;
  static bool const is_pointer   = true;
  static bool const is_reference = false;
};

template <typename T>
struct TypeInfo <T const*>
{
  typedef T Type;
  static bool const is_const     = true;
  static bool const is_pointer   = true;
  static bool const is_reference = false;
};

template <typename T>
struct TypeInfo <T&>
{
  typedef T Type;
  static bool const is_const     = false;
  static bool const is_pointer   = false;
  static bool const is_reference = true;
};

template <typename T>
struct TypeInfo <T const&>
{
  typedef T Type;
  static bool const is_const     = true;
  static bool const is_pointer   = false;
  static bool const is_reference = true;
};

//==============================================================================
//
// TypeList
//
//==============================================================================

/**
  None type means void parameters or return value.
*/
typedef void None;

template <typename Head, typename Tail = None>
struct TypeList
{
};

/**
  A TypeList with actual values.
*/
template <typename List>
struct TypeListValues
{
  static std::string const tostring (bool)
  {
    return "";
  }
};

/**
  TypeListValues recursive template definition.
*/
template <typename Head, typename Tail>
struct TypeListValues <TypeList <Head, Tail> >
{
  Head hd;
  TypeListValues <Tail> tl;

  TypeListValues (Head hd_, TypeListValues <Tail> const& tl_)
    : hd (hd_), tl (tl_)
  {
  }

  static std::string const tostring (bool comma = false)
  {
    std::string s;

    if (comma)
      s = ", ";

    s = s + typeid (Head).name ();

    return s + TypeListValues <Tail>::tostring (true);
  }
};

// Specializations of type/value list for head types that are references and
// const-references.  We need to handle these specially since we can't count
// on the referenced object hanging around for the lifetime of the list.

template <typename Head, typename Tail>
struct TypeListValues <TypeList <Head&, Tail> >
{
  Head hd;
  TypeListValues <Tail> tl;

  TypeListValues (Head& hd_, TypeListValues <Tail> const& tl_)
    : hd (hd_), tl (tl_)
  {
  }

  static std::string const tostring (bool comma = false)
  {
    std::string s;

    if (comma)
      s = ", ";

    s = s + typeid (Head).name () + "&";

    return s + TypeListValues <Tail>::tostring (true);
  }
};

template <typename Head, typename Tail>
struct TypeListValues <TypeList <Head const&, Tail> >
{
  Head hd;
  TypeListValues <Tail> tl;

  TypeListValues (Head const& hd_, const TypeListValues <Tail>& tl_)
    : hd (hd_), tl (tl_)
  {
  }

  static std::string const tostring (bool comma = false)
  {
    std::string s;

    if (comma)
      s = ", ";

    s = s + typeid (Head).name () + " const&";

    return s + TypeListValues <Tail>::tostring (true);
  }
};

//==============================================================================
/**
  Traits for function pointers.

  There are three types of functions: global, non-const member, and const member.
  These templates determine the type of function, which class type it belongs to
  if it is a class member, the const-ness if it is a member function, and the
  type information for the return value and argument list.

  Expansions are provided for functions with up to 8 parameters. This can be
  manually extended, or expanded to an arbitrary amount using C++11 features.
*/
template <typename MemFn, typename D = MemFn>
struct FuncTraits
{
};

/* Ordinary function pointers. */

template <typename R, typename D>
struct FuncTraits <R (*) (), D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef None Params;
  static R call (DeclType fp, TypeListValues <Params>)
  {
    return fp ();
  }
};

template <typename R, typename P1, typename D>
struct FuncTraits <R (*) (P1), D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1> Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd);
  }
};

template <typename R, typename P1, typename P2, typename D>
struct FuncTraits <R (*) (P1, P2), D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2> > Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd, tvl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename D>
struct FuncTraits <R (*) (P1, P2, P3), D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3> > > Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename D>
struct FuncTraits <R (*) (P1, P2, P3, P4), D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4> > > > Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename D>
struct FuncTraits <R (*) (P1, P2, P3, P4, P5), D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5> > > > > Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename D>
struct FuncTraits <R (*) (P1, P2, P3, P4, P5, P6), D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5,  TypeList <P6> > > > > > Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename D>
struct FuncTraits <R (*) (P1, P2, P3, P4, P5, P6, P7), D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6, TypeList <P7> > > > > > > Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename D>
struct FuncTraits <R (*) (P1, P2, P3, P4, P5, P6, P7, P8), D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6, TypeList <P7, TypeList <P8> > > > > > > > Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
};

/* Non-const member function pointers. */

template <class T, typename R, typename D>
struct FuncTraits <R (T::*) (), D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef None Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> const&)
  {
    return (obj->*fp)();
  }
};

template <class T, typename R, typename P1, typename D>
struct FuncTraits <R (T::*) (P1), D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1> Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename D>
struct FuncTraits <R (T::*) (P1, P2), D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2> > Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3), D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3> > > Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4), D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4> > > > Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5), D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5> > > > > Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6), D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6> > > > > > Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6, P7), D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6, TypeList <P7> > > > > > > Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6, P7, P8), D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6, TypeList <P7, TypeList <P8> > > > > > > > Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
};

/* Const member function pointers. */

template <class T, typename R, typename D>
struct FuncTraits <R (T::*) () const, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef None Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> const&)
  {
    return (obj->*fp)();
  }
};

template <class T, typename R, typename P1, typename D>
struct FuncTraits <R (T::*) (P1) const, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1> Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename D>
struct FuncTraits <R (T::*) (P1, P2) const, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2> > Params;
  static R call (T const* const obj, R (T::*fp) (P1, P2) const,
    TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3) const, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3> > > Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4) const, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4> > > > Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5) const, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5> > > > > Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6) const, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6> > > > > > Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6, P7) const, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6, TypeList <P7> > > > > > > Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6, P7, P8) const, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6, TypeList <P7, TypeList <P8> > > > > > > > Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
};

#if defined (LUABRIDGE_THROWSPEC)

/* Ordinary function pointers. */

template <typename R, typename D>
struct FuncTraits <R (*) () LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef None Params;
  static R call (DeclType fp, TypeListValues <Params> const&)
  {
    return fp ();
  }
};

template <typename R, typename P1, typename D>
struct FuncTraits <R (*) (P1) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1> Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd);
  }
};

template <typename R, typename P1, typename P2, typename D>
struct FuncTraits <R (*) (P1, P2) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2> > Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd, tvl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename D>
struct FuncTraits <R (*) (P1, P2, P3) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3> > > Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename D>
struct FuncTraits <R (*) (P1, P2, P3, P4) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4> > > > Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename D>
struct FuncTraits <R (*) (P1, P2, P3, P4, P5) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5> > > > > Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename D>
struct FuncTraits <R (*) (P1, P2, P3, P4, P5, P6) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5,  TypeList <P6> > > > > > Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename D>
struct FuncTraits <R (*) (P1, P2, P3, P4, P5, P6, P7) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6, TypeList <P7> > > > > > > Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename D>
struct FuncTraits <R (*) (P1, P2, P3, P4, P5, P6, P7, P8) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6, TypeList <P7, TypeList <P8> > > > > > > > Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
};

/* Non-const member function pointers. */

template <class T, typename R, typename D>
struct FuncTraits <R (T::*) () LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef None Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> const&)
  {
    return (obj->*fp)();
  }
};

template <class T, typename R, typename P1, typename D>
struct FuncTraits <R (T::*) (P1) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1> Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename D>
struct FuncTraits <R (T::*) (P1, P2) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2> > Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3> > > Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4> > > > Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5> > > > > Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6> > > > > > Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6, P7) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6, TypeList <P7> > > > > > > Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6, P7, P8) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6, TypeList <P7, TypeList <P8> > > > > > > > Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
};

/* Const member function pointers. */

template <class T, typename R, typename D>
struct FuncTraits <R (T::*) () const LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef None Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> const&)
  {
    (void)tvl;
    return (obj->*fp)();
  }
};

template <class T, typename R, typename P1, typename D>
struct FuncTraits <R (T::*) (P1) const LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1> Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename D>
struct FuncTraits <R (T::*) (P1, P2) const LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2> > Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3) const LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3> > > Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4) const LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4> > > > Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5) const LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5> > > > > Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6) const LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6> > > > > > Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6, P7) const LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6, TypeList <P7> > > > > > > Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6, P7, P8) const LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6, TypeList <P7, TypeList <P8> > > > > > > > Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
};

#endif

/*
* Constructor generators.  These templates allow you to call operator new and
* pass the contents of a type/value list to the Constructor.  Like the
* function pointer containers, these are only defined up to 8 parameters.
*/

/** Constructor generators.

    These templates call operator new with the contents of a type/value
    list passed to the Constructor with up to 8 parameters. Two versions
    of call() are provided. One performs a regular new, the other performs
    a placement new.
*/
template <class T, typename List>
struct Constructor {};

template <class T>
struct Constructor <T, None>
{
  static T* call (TypeListValues <None> const&)
  {
    return new T;
  }
  static T* call (void* mem, TypeListValues <None> const&)
  {
    return new (mem) T;
  }
};

template <class T, class P1>
struct Constructor <T, TypeList <P1> >
{
  static T* call (const TypeListValues<TypeList <P1> > &tvl)
  {
    return new T(tvl.hd);
  }
  static T* call (void* mem, const TypeListValues<TypeList <P1> > &tvl)
  {
    return new (mem) T(tvl.hd);
  }
};

template <class T, class P1, class P2>
struct Constructor <T, TypeList <P1, TypeList <P2> > >
{
  static T* call (const TypeListValues<TypeList <P1, TypeList <P2> > > &tvl)
  {
    return new T(tvl.hd, tvl.tl.hd);
  }
  static T* call (void* mem, const TypeListValues<TypeList <P1, TypeList <P2> > > &tvl)
  {
    return new (mem) T(tvl.hd, tvl.tl.hd);
  }
};

template <class T, class P1, class P2, class P3>
struct Constructor <T, TypeList <P1, TypeList <P2, TypeList <P3> > > >
{
  static T* call (const TypeListValues<TypeList <P1, TypeList <P2,
    TypeList <P3> > > > &tvl)
  {
    return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
  static T* call (void* mem, const TypeListValues<TypeList <P1, TypeList <P2,
    TypeList <P3> > > > &tvl)
  {
    return new (mem) T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <class T, class P1, class P2, class P3, class P4>
struct Constructor <T, TypeList <P1, TypeList <P2, TypeList <P3,
  TypeList <P4> > > > >
{
  static T* call (const TypeListValues<TypeList <P1, TypeList <P2,
    TypeList <P3, TypeList <P4> > > > > &tvl)
  {
    return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
  static T* call (void* mem, const TypeListValues<TypeList <P1, TypeList <P2,
    TypeList <P3, TypeList <P4> > > > > &tvl)
  {
    return new (mem) T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <class T, class P1, class P2, class P3, class P4,
  class P5>
struct Constructor <T, TypeList <P1, TypeList <P2, TypeList <P3,
  TypeList <P4, TypeList <P5> > > > > >
{
  static T* call (const TypeListValues<TypeList <P1, TypeList <P2,
    TypeList <P3, TypeList <P4, TypeList <P5> > > > > > &tvl)
  {
    return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
  static T* call (void* mem, const TypeListValues<TypeList <P1, TypeList <P2,
    TypeList <P3, TypeList <P4, TypeList <P5> > > > > > &tvl)
  {
    return new (mem) T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
};

template <class T, class P1, class P2, class P3, class P4,
  class P5, class P6>
struct Constructor <T, TypeList <P1, TypeList <P2, TypeList <P3,
  TypeList <P4, TypeList <P5, TypeList <P6> > > > > > >
{
  static T* call (const TypeListValues<TypeList <P1, TypeList <P2,
    TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6> > > > > > > &tvl)
  {
    return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
  static T* call (void* mem, const TypeListValues<TypeList <P1, TypeList <P2,
    TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6> > > > > > > &tvl)
  {
    return new (mem) T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, class P1, class P2, class P3, class P4,
  class P5, class P6, class P7>
struct Constructor <T, TypeList <P1, TypeList <P2, TypeList <P3,
  TypeList <P4, TypeList <P5, TypeList <P6, TypeList <P7> > > > > > > >
{
  static T* call (const TypeListValues<TypeList <P1, TypeList <P2,
    TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6,
    TypeList <P7> > > > > > > > &tvl)
  {
    return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
  static T* call (void* mem, const TypeListValues<TypeList <P1, TypeList <P2,
    TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6,
    TypeList <P7> > > > > > > > &tvl)
  {
    return new (mem) T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, class P1, class P2, class P3, class P4,
  class P5, class P6, class P7, class P8>
struct Constructor <T, TypeList <P1, TypeList <P2, TypeList <P3,
  TypeList <P4, TypeList <P5, TypeList <P6, TypeList <P7, 
  TypeList <P8> > > > > > > > >
{
  static T* call (const TypeListValues<TypeList <P1, TypeList <P2,
    TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6,
    TypeList <P7, TypeList <P8> > > > > > > > > &tvl)
  {
    return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
  static T* call (void* mem, const TypeListValues<TypeList <P1, TypeList <P2,
    TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6,
    TypeList <P7, TypeList <P8> > > > > > > > > &tvl)
  {
    return new (mem) T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
};

//==============================================================================

// Forward declaration required.
template <class T>
struct Stack;

//------------------------------------------------------------------------------
/**
  Container traits.

  Unspecialized ContainerTraits has the isNotContainer typedef for SFINAE. All
  user defined containers must supply an appropriate specialization for
  ContinerTraits (without the typedef isNotContainer). The containers that come
  with LuaBridge also come with the appropriate ContainerTraits specialization.
  See the corresponding declaration for details.

  A specialization of ContainerTraits for some generic type ContainerType
  looks like this:

  template <class T>
  struct ContainerTraits <ContainerType <T> >
  {
    typedef typename T Type;

    static T* get (ContainerType <T> const& c)
    {
      return c.get (); // Implementation-dependent on ContainerType
    }
  };
*/
template <class T>
struct ContainerTraits
{
  typedef bool isNotContainer;
};

//==============================================================================

#if LUA_VERSION_NUM < 502
/**
  Helpers for Lua versions prior to 5.2.0.
*/

inline int lua_absindex (lua_State *L, int idx)
{
  if (idx > LUA_REGISTRYINDEX && idx < 0)
    return lua_gettop (L) + idx + 1;
  else
    return idx;
}

inline void lua_rawgetp (lua_State *L, int idx, void const* p)
{
  idx = lua_absindex (L, idx);
  lua_pushlightuserdata (L, const_cast <void*> (p));
  lua_rawget (L,idx);
}

inline void lua_rawsetp (lua_State *L, int idx, void const* p)
{
  idx = lua_absindex (L, idx);
  lua_pushlightuserdata (L, const_cast <void*> (p));
  // put key behind value
  lua_insert (L, -2);
  lua_rawset (L, idx);
}
#endif

//------------------------------------------------------------------------------
/**
  Get a table value, bypassing metamethods.
*/  
inline void rawgetfield (lua_State* const L, int index, char const* const key)
{
  assert (lua_istable (L, index));
  index = lua_absindex (L, index);
  lua_pushstring (L, key);
  lua_rawget (L, index);
}

//------------------------------------------------------------------------------
/**
  Set a table value, bypassing metamethods.
*/  
inline void rawsetfield (lua_State* const L, int index, char const* const key)
{
  assert (lua_istable (L, index));
  index = lua_absindex (L, index);
  lua_pushstring (L, key);
  lua_insert (L, -2);
  lua_rawset (L, index);
}

//==============================================================================

namespace Detail
{
  //----------------------------------------------------------------------------
  /**
    Control security options.
  */
  class Security
  {
  public:
    static bool hideMetatables ()
    {
      return getSettings().hideMetatables;
    }

    static void setHideMetatables (bool shouldHide)
    {
      getSettings().hideMetatables = shouldHide;
    }

  private:
    struct Settings
    {
      Settings ()
        : hideMetatables (true)
      {
      }

      bool hideMetatables;
    };

    static Settings& getSettings ()
    {
      static Settings settings;
      return settings;
    }
  };

  //----------------------------------------------------------------------------
  struct TypeTraits
  {
    //--------------------------------------------------------------------------
    /**
      Determine if type T is a container.

      To be considered a container, there must be a specialization of
      ContainerTraits with the required fields.
    */
    template <typename T>
    class isContainer
    {
      typedef char yes[1]; // sizeof (yes) == 1
      typedef char no [2]; // sizeof (no)  == 2

      template <typename C>
      static no& test (typename C::isNotContainer*);
 
      template <typename>
      static yes& test (...);
 
    public:
      static const bool value = sizeof (test <ContainerTraits <T> >(0)) == sizeof (yes);
    };

    //--------------------------------------------------------------------------
    /**
      Determine if T is const qualified.
    */
    template <class T>
    struct isConst
    {
      static bool const value = false;
    };

    template <class T>
    struct isConst <T const>
    {
      static bool const value = true;
    };

    //--------------------------------------------------------------------------
    /**
      Strip the const qualifier from T.
    */
    template <class T>
    struct removeConst
    {
      typedef T Type;
    };

    template <class T>
    struct removeConst <T const>
    {
      typedef T Type;
    };
  };

  //============================================================================
  /**
    Return the identity pointer for our lightuserdata tokens.

    LuaBridge metatables are tagged with a security "token." The token is a
    lightuserdata created from the identity pointer, used as a key in the
    metatable. The value is a boolean = true, although any value could have been
    used.

    Because of Lua's dynamic typing and our improvised system of imposing C++
    class structure, there is the possibility that executing scripts may
    knowingly or unknowingly cause invalid data to get passed to the C functions
    created by LuaBridge. In particular, our security model addresses the
    following:

    Problem:

      Prove that a userdata passed to a LuaBridge C function was created by us.

    An attempt to access the memory of a foreign userdata through a pointer
    of our own type will result in undefined behavior. Our verification
    strategy is based on the security of the token used to tag our metatables.
    We will now reason about the security model.

    Axioms:

      1. Scripts cannot create a userdata (ignoring the debug lib).
      2. Scripts cannot create a lightuserdata (ignoring the debug lib).
      3. Scripts cannot set the metatable on a userdata.
      4. Our identity key is a unique pointer in the process.
      5. Our metatables have a lightuserdata identity key / value pair.
      6. Our metatables have "__metatable" set to a boolean = false.

    Lemma:

      7. Our lightuserdata is unique.

          This follows from #4.

    Lemma:

    - Lua scripts cannot read or write metatables created by LuaBridge.
      They cannot gain access to a lightuserdata

    Therefore, it is certain that if a Lua value is a userdata, the userdata
    has a metatable, and the metatable has a value for a lightuserdata key
    with this identity pointer address, that LuaBridge created the userdata.
  */
  static inline void* getIdentityKey ()
  {
    static char value;
    return &value;
  }

  //----------------------------------------------------------------------------
  /**
    Unique registry keys for a class.

    Each registered class inserts three keys into the registry, whose
    values are the corresponding static, class, and const metatables. This
    allows a quick and reliable lookup for a metatable from a template type.
  */
  template <class T>
  class ClassInfo
  {
  public:
    /**
      Get the key for the static table.

      The static table holds the static data members, static properties, and
      static member functions for a class.
    */
    static void const* getStaticKey ()
    {
      static char value;
      return &value;
    }

    /**
      Get the key for the class table.

      The class table holds the data members, properties, and member functions
      of a class. Read-only data and properties, and const member functions are
      also placed here (to save a lookup in the const table).
    */
    static void const* getClassKey ()
    {
      static char value;
      return &value;
    }

    /**
      Get the key for the const table.

      The const table holds read-only data members and properties, and const
      member functions of a class.
    */
    static void const* getConstKey ()
    {
      static char value;
      return &value;
    }
  };

  //============================================================================
  /**
    Interface to a class poiner retrievable from a userdata.
  */
  class Userdata
  {
  protected:
    void* m_p; // subclasses must set this

    //--------------------------------------------------------------------------
    /**
      Get an untyped pointer to the contained class.
    */
    inline void* const getPointer ()
    {
      return m_p;
    }

  private:
    //--------------------------------------------------------------------------
    /**
      Validate and retrieve a Userdata on the stack.

      The Userdata must exactly match the corresponding class table or
      const table, or else a Lua error is raised. This is used for the
      __gc metamethod.
    */
    static Userdata* getExactClass (lua_State* L, int narg, void const* const classKey)
    {
      Userdata* ud = 0;
      int const index = lua_absindex (L, narg);

      bool mismatch = false;
      char const* got = 0;

      lua_rawgetp (L, LUA_REGISTRYINDEX, classKey);
      assert (lua_istable (L, -1));

      // Make sure we have a userdata.
      if (!mismatch && !lua_isuserdata (L, index))
        mismatch = true;

      // Make sure it's metatable is ours.
      if (!mismatch)
      {
        lua_getmetatable (L, index);
        lua_rawgetp (L, -1, getIdentityKey ());
        if (lua_isboolean (L, -1))
        {
          lua_pop (L, 1);
        }
        else
        {
          lua_pop (L, 2);
          mismatch = true;
        }      
      }

      if (!mismatch)
      {
        if (lua_rawequal (L, -1, -2))
        {
          // Matches class table.
          lua_pop (L, 2);
          ud = static_cast <Userdata*> (lua_touserdata (L, index));
        }
        else
        {
          rawgetfield (L, -2, "__const");
          if (lua_rawequal (L, -1, -2))
          {
            // Matches const table
            lua_pop (L, 3);
            ud = static_cast <Userdata*> (lua_touserdata (L, index));
          }
          else
          {
            // Mismatch, but its one of ours so get a type name.
            rawgetfield (L, -2, "__type");
            lua_insert (L, -4);
            lua_pop (L, 2);
            got = lua_tostring (L, -2);
            mismatch = true;
          }
        }
      }

      if (mismatch)
      {
        rawgetfield (L, -1, "__type");
        assert (lua_type (L, -1) == LUA_TSTRING);
        char const* const expected = lua_tostring (L, -1);

        if (got == 0)
          got = lua_typename (L, lua_type (L, index));

        char const* const msg = lua_pushfstring (
          L, "%s expected, got %s", expected, got);

        if (narg > 0)
          luaL_argerror (L, narg, msg);
        else
          lua_error (L);
      }

      return ud;
    }

    //--------------------------------------------------------------------------
    /**
      Validate and retrieve a Userdata on the stack.

      The Userdata must be derived from or the same as the given base class,
      identified by the key. If canBeConst is false, generates an error if
      the resulting Userdata represents to a const object. We do the type check
      first so that the error message is informative.
    */
    static Userdata* getClass (
      lua_State* L, int const index, void const* const baseClassKey, bool const canBeConst)
    {
      assert (index > 0);
      Userdata* ud = 0;

      bool mismatch = false;
      char const* got = 0;

      lua_rawgetp (L, LUA_REGISTRYINDEX, baseClassKey);
      assert (lua_istable (L, -1));

      // Make sure we have a userdata.
      if (lua_isuserdata (L, index))
      {
        // Make sure it's metatable is ours.
        lua_getmetatable (L, index);
        lua_rawgetp (L, -1, getIdentityKey ());
        if (lua_isboolean (L, -1))
        {
          lua_pop (L, 1);

          // If __const is present, object is NOT const.
          rawgetfield (L, -1, "__const");
          assert (lua_istable (L, -1) || lua_isnil (L, -1));
          bool const isConst = lua_isnil (L, -1);
          lua_pop (L, 1);

          // Replace the class table with the const table if needed.
          if (isConst)
          {
            rawgetfield (L, -2, "__const");
            assert (lua_istable (L, -1));
            lua_replace (L, -3);
          }

          for (;;)
          {
            if (lua_rawequal (L, -1, -2))
            {
              lua_pop (L, 2);

              // Match, now check const-ness.
              if (isConst && !canBeConst)
              {
                luaL_argerror (L, index, "cannot be const");
              }
              else
              {
                ud = static_cast <Userdata*> (lua_touserdata (L, index));
                break;
              }
            }
            else
            {
              // Replace current metatable with it's base class.
              rawgetfield (L, -1, "__parent");
              lua_remove (L, -2);

              if (lua_isnil (L, -1))
              {
                // Mismatch, but its one of ours so get a type name.
                rawgetfield (L, -2, "__type");
                lua_insert (L, -4);
                lua_pop (L, 2);
                got = lua_tostring (L, -2);
                mismatch = true;
                break;
              }
            }
          }
        }
        else
        {
          lua_pop (L, 2);
          mismatch = true;
        }      
      }
      else
      {
        mismatch = true;
      }

      if (mismatch)
      {
        rawgetfield (L, -1, "__type");
        assert (lua_type (L, -1) == LUA_TSTRING);
        char const* const expected = lua_tostring (L, -1);

        if (got == 0)
          got = lua_typename (L, lua_type (L, index));

        char const* const msg = lua_pushfstring (
          L, "%s expected, got %s", expected, got);

        luaL_argerror (L, index, msg);
      }

      return ud;
    }

  public:
    virtual ~Userdata () { }

    //--------------------------------------------------------------------------
    /**
      Returns the Userdata* if the class on the Lua stack matches.

      If the class does not match, a Lua error is raised.
    */
    template <class T>
    static inline Userdata* getExact (lua_State* L, int index)
    {
      return getExactClass (L, index, ClassInfo <T>::getClassKey ());
    }

    //--------------------------------------------------------------------------
    /**
      Get a pointer to the class from the Lua stack.

      If the object is not the class or a subclass, or it violates the
      const-ness, a Lua error is raised.
    */
    template <class T>
    static inline T* get (lua_State* L, int index, bool canBeConst)
    {
      if (lua_isnil (L, index))
        return 0;
      else
        return static_cast <T*> (getClass (L, index,
          ClassInfo <T>::getClassKey (), canBeConst)->getPointer ());
    }
  };

  //----------------------------------------------------------------------------
  /**
    Wraps a class object stored in a Lua userdata.

    The lifetime of the object is managed by Lua. The object is constructed
    inside the userdata using placement new.
  */
  template <class T>
  class UserdataValue : public Userdata
  {
  private:
    UserdataValue <T> (UserdataValue <T> const&);
    UserdataValue <T> operator= (UserdataValue <T> const&);

    char m_storage [sizeof (T)];

    inline T* getObject ()
    {
      // If this fails to compile it means you forgot to provide
      // a Container specialization for your container!
      //
      return reinterpret_cast <T*> (&m_storage [0]);
    }

  private:
    /**
      Used for placement construction.
    */
    UserdataValue ()
    {
      m_p = getObject ();
    }

    ~UserdataValue ()
    {
      getObject ()->~T ();
    }

  public:
    /**
      Push a T via placement new.

      The caller is responsible for calling placement new using the
      returned uninitialized storage.
    */
    static void* place (lua_State* const L)
    {
      UserdataValue <T>* const ud = new (
        lua_newuserdata (L, sizeof (UserdataValue <T>))) UserdataValue <T> ();
      lua_rawgetp (L, LUA_REGISTRYINDEX, ClassInfo <T>::getClassKey ());
      // If this goes off it means you forgot to register the class!
      assert (lua_istable (L, -1));
      lua_setmetatable (L, -2);
      return ud->getPointer ();
    }

    /**
      Push T via copy construction from U.
    */
    template <class U>
    static inline void push (lua_State* const L, U const& u)
    {
      new (place (L)) U (u);
    }
  };

  //----------------------------------------------------------------------------
  /**
    Wraps a pointer to a class object inside a Lua userdata.

    The lifetime of the object is managed by C++.
  */
  class UserdataPtr : public Userdata
  {
  private:
    UserdataPtr (UserdataPtr const&);
    UserdataPtr operator= (UserdataPtr const&);

  private:
    /** Push non-const pointer to object using metatable key.
    */
    static void push (lua_State* L, void* const p, void const* const key)
    {
      if (p)
      {
        new (lua_newuserdata (L, sizeof (UserdataPtr))) UserdataPtr (p);
        lua_rawgetp (L, LUA_REGISTRYINDEX, key);
        // If this goes off it means you forgot to register the class!
        assert (lua_istable (L, -1));
        lua_setmetatable (L, -2);
      }
      else
      {
        lua_pushnil (L);
      }
    }

    /** Push const pointer to object using metatable key.
    */
    static void push (lua_State* L, void const* const p, void const* const key)
    {
      if (p)
      {
        new (lua_newuserdata (L, sizeof (UserdataPtr)))
          UserdataPtr (const_cast <void*> (p));
        lua_rawgetp (L, LUA_REGISTRYINDEX, key);
        // If this goes off it means you forgot to register the class!
        assert (lua_istable (L, -1));
        lua_setmetatable (L, -2);
      }
      else
      {
        lua_pushnil (L);
      }
    }

    explicit UserdataPtr (void* const p)
    {
      m_p = p;

      // Can't construct with a null pointer!
      //
      assert (m_p != 0);
    }

  public:
    /** Push non-const pointer to object.
    */
    template <class T>
    static inline void push (lua_State* const L, T* const p)
    {
      if (p)
        push (L, p, ClassInfo <T>::getClassKey ());
      else
        lua_pushnil (L);
    }

    /** Push const pointer to object.
    */
    template <class T>
    static inline void push (lua_State* const L, T const* const p)
    {
      if (p)
        push (L, p, ClassInfo <T>::getConstKey ());
      else
        lua_pushnil (L);
    }
  };

  //============================================================================
  /**
    Wraps a container thet references a class object.

    The template argument C is the container type, ContainerTraits must be
    specialized on C or else a compile error will result.
  */
  template <class C>
  class UserdataShared : public Userdata
  {
  private:
    UserdataShared (UserdataShared <C> const&);
    UserdataShared <C>& operator= (UserdataShared <C> const&);

    typedef typename TypeTraits::removeConst <
      typename ContainerTraits <C>::Type>::Type T;

    C m_c;

  private:
    ~UserdataShared ()
    {
    }

  public:
    /**
      Construct from a container to the class or a derived class.
    */
    template <class U>
    explicit UserdataShared (U const& u) : m_c (u)
    {
      m_p = const_cast <void*> (reinterpret_cast <void const*> (
          (ContainerTraits <C>::get (m_c))));
    }

    /**
      Construct from a pointer to the class or a derived class.
    */
    template <class U>
    explicit UserdataShared (U* u) : m_c (u)
    {
      m_p = const_cast <void*> (reinterpret_cast <void const*> (
          (ContainerTraits <C>::get (m_c))));
    }
  };

  //----------------------------------------------------------------------------
  //
  // SFINAE helpers.
  //

  // non-const objects
  template <class C, bool makeObjectConst>
  struct UserdataSharedHelper
  {
    typedef typename TypeTraits::removeConst <
      typename ContainerTraits <C>::Type>::Type T;

    static void push (lua_State* L, C const& c)
    {
      if (ContainerTraits <C>::get (c) != 0)
      {
        new (lua_newuserdata (L, sizeof (UserdataShared <C>))) UserdataShared <C> (c);
        lua_rawgetp (L, LUA_REGISTRYINDEX, ClassInfo <T>::getClassKey ());
        // If this goes off it means the class T is unregistered!
        assert (lua_istable (L, -1));
        lua_setmetatable (L, -2);
      }
      else
      {
        lua_pushnil (L);
      }
    }

    static void push (lua_State* L, T* const t)
    {
      if (t)
      {
        new (lua_newuserdata (L, sizeof (UserdataShared <C>))) UserdataShared <C> (t);
        lua_rawgetp (L, LUA_REGISTRYINDEX, ClassInfo <T>::getClassKey ());
        // If this goes off it means the class T is unregistered!
        assert (lua_istable (L, -1));
        lua_setmetatable (L, -2);
      }
      else
      {
        lua_pushnil (L);
      }
    }
  };

  // const objects
  template <class C>
  struct UserdataSharedHelper <C, true>
  {
    typedef typename TypeTraits::removeConst <
      typename ContainerTraits <C>::Type>::Type T;

    static void push (lua_State* L, C const& c)
    {
      if (ContainerTraits <C>::get (c) != 0)
      {
        new (lua_newuserdata (L, sizeof (UserdataShared <C>))) UserdataShared <C> (c);
        lua_rawgetp (L, LUA_REGISTRYINDEX, ClassInfo <T>::getConstKey ());
        // If this goes off it means the class T is unregistered!
        assert (lua_istable (L, -1));
        lua_setmetatable (L, -2);
      }
      else
      {
        lua_pushnil (L);
      }
    }

    static void push (lua_State* L, T* const t)
    {
      if (t)
      {
        new (lua_newuserdata (L, sizeof (UserdataShared <C>))) UserdataShared <C> (t);
        lua_rawgetp (L, LUA_REGISTRYINDEX, ClassInfo <T>::getConstKey ());
        // If this goes off it means the class T is unregistered!
        assert (lua_istable (L, -1));
        lua_setmetatable (L, -2);
      }
      else
      {
        lua_pushnil (L);
      }
    }
  };

  /**
    Pass by container.

    The container controls the object lifetime. Typically this will be a
    lifetime shared by C++ and Lua using a reference count. Because of type
    erasure, containers like std::shared_ptr will not work. Containers must
    either be of the intrusive variety, or in the style of the RefCountedPtr
    type provided by LuaBridge (that uses a global hash table).
  */
  template <class C, bool byContainer>
  struct StackHelper
  {
    static inline void push (lua_State* L, C const& c)
    {
      UserdataSharedHelper <C,
        TypeTraits::isConst <typename ContainerTraits <C>::Type>::value>::push (L, c);
    }

    typedef typename TypeTraits::removeConst <
      typename ContainerTraits <C>::Type>::Type T;

    static inline C get (lua_State* L, int index)
    {
      return Detail::Userdata::get <T> (L, index, true);
    }
  };

  /**
    Pass by value.

    Lifetime is managed by Lua. A C++ function which accesses a pointer or
    reference to an object outside the activation record in which it was
    retrieved may result in undefined behavior if Lua garbage collected it.
  */
  template <class T>
  struct StackHelper <T, false>
  {
    static inline void push (lua_State* L, T const& t)
    {
      Detail::UserdataValue <T>::push (L, t);
    }

    static inline T const& get (lua_State* L, int index)
    {
      return *Detail::Userdata::get <T> (L, index, true);
    }
  };
}

//==============================================================================

/**
  Lua stack conversions for class objects passed by value.
*/
template <class T>
struct Stack
{
public:
  static inline void push (lua_State* L, T const& t)
  {
    Detail::StackHelper <T,
      Detail::TypeTraits::isContainer <T>::value>::push (L, t);
  }

  static inline T get (lua_State* L, int index)
  {
    return Detail::StackHelper <T,
      Detail::TypeTraits::isContainer <T>::value>::get (L, index);
  }
};

//------------------------------------------------------------------------------
/**
  Lua stack conversions for pointers and references to class objects.

  Lifetime is managed by C++. Lua code which remembers a reference to the
  value may result in undefined behavior if C++ destroys the object. The
  handling of the const and volatile qualifiers happens in UserdataPtr.
*/

// pointer
template <class T>
struct Stack <T*>
{
  static inline void push (lua_State* L, T* const p)
  {
    Detail::UserdataPtr::push (L, p);
  }

  static inline T* const get (lua_State* L, int index)
  {
    return Detail::Userdata::get <T> (L, index, false);
  }
};

// Strips the const off the right side of *
template <class T>
struct Stack <T* const>
{
  static inline void push (lua_State* L, T* const p)
  {
    Detail::UserdataPtr::push (L, p);
  }

  static inline T* const get (lua_State* L, int index)
  {
    return Detail::Userdata::get <T> (L, index, false);
  }
};

// pointer to const
template <class T>
struct Stack <T const*>
{
  static inline void push (lua_State* L, T const* const p)
  {
    Detail::UserdataPtr::push (L, p);
  }

  static inline T const* const get (lua_State* L, int index)
  {
    return Detail::Userdata::get <T> (L, index, true);
  }
};

// Strips the const off the right side of *
template <class T>
struct Stack <T const* const>
{
  static inline void push (lua_State* L, T const* const p)
  {
    Detail::UserdataPtr::push (L, p);
  }

  static inline T const* const get (lua_State* L, int index)
  {
    return Detail::Userdata::get <T> (L, index, true);
  }
};

// reference
template <class T>
struct Stack <T&>
{
  static inline void push (lua_State* L, T& t)
  {
    Detail::UserdataPtr::push (L, &t);
  }

  static T& get (lua_State* L, int index)
  {
    T* const t = Detail::Userdata::get <T> (L, index, false);
    if (!t)
      luaL_error (L, "nil passed to reference");
    return *t;
  }
};

// reference to const
template <class T>
struct Stack <T const&>
{
  static inline void push (lua_State* L, T const& t)
  {
    Detail::UserdataPtr::push (L, &t);
  }

  static T const& get (lua_State* L, int index)
  {
    T const* const t = Detail::Userdata::get <T> (L, index, true);
    if (!t)
      luaL_error (L, "nil passed to reference");
    return *t;
  }
};

//------------------------------------------------------------------------------
/**
  Receive the lua_State* as an argument.
*/
template <>
struct Stack <lua_State*>
{
  static lua_State* get (lua_State* L, int)
  {
    return L;
  }
};

//------------------------------------------------------------------------------
/**
  Lua stack conversions for basic types.
*/

// int
template <> struct Stack <
  int > { static inline void push (lua_State* L,
  int value) { lua_pushnumber (L, static_cast <lua_Number> (value)); } static inline
  int get (lua_State* L, int index) { return static_cast <
  int > (luaL_checknumber (L, index)); } };

// unsigned int
template <> struct Stack <
  unsigned int > { static inline void push (lua_State* L,
  unsigned int value) { lua_pushnumber (L, static_cast <lua_Number> (value)); } static inline
  unsigned int get (lua_State* L, int index) { return static_cast <
  unsigned int > (luaL_checknumber (L, index)); } };

// unsigned char
template <> struct Stack <
  unsigned char > { static inline void push (lua_State* L,
  unsigned char value) { lua_pushnumber (L, static_cast <lua_Number> (value)); } static inline
  unsigned char get (lua_State* L, int index) { return static_cast <
  unsigned char > (luaL_checknumber (L, index)); } };

// short
template <> struct Stack <
  short > { static inline void push (lua_State* L,
  short value) { lua_pushnumber (L, static_cast <lua_Number> (value)); } static inline
  short get (lua_State* L, int index) { return static_cast <
  short > (luaL_checknumber (L, index)); } };

// unsigned short
template <> struct Stack <
  unsigned short > { static inline void push (lua_State* L,
  unsigned short value) { lua_pushnumber (L, static_cast <lua_Number> (value)); } static inline
  unsigned short get (lua_State* L, int index) { return static_cast <
  unsigned short > (luaL_checknumber (L, index)); } };

// long
template <> struct Stack <
  long > { static inline void push (lua_State* L,
  long value) { lua_pushnumber (L, static_cast <lua_Number> (value)); } static inline
  long get (lua_State* L, int index) { return static_cast <
  long > (luaL_checknumber (L, index)); } };

// unsigned long
template <> struct Stack <
  unsigned long > { static inline void push (lua_State* L,
  unsigned long value) { lua_pushnumber (L, static_cast <lua_Number> (value)); } static inline
  unsigned long get (lua_State* L, int index) { return static_cast <
  unsigned long > (luaL_checknumber (L, index)); } };

// float
template <> struct Stack <
  float > { static inline void push (lua_State* L,
  float value) { lua_pushnumber (L, static_cast <lua_Number> (value)); } static inline
  float get (lua_State* L, int index) { return static_cast <
  float > (luaL_checknumber (L, index)); } };

// double
template <> struct Stack <
  double > { static inline void push (lua_State* L,
  double value) { lua_pushnumber (L, static_cast <lua_Number> (value)); } static inline
  double get (lua_State* L, int index) { return static_cast <
  double > (luaL_checknumber (L, index)); } };

// bool
template <>
struct Stack <bool>
{
  static inline void push (lua_State* L, bool value)
  {
    lua_pushboolean (L, value ? 1 : 0);
  }

  static inline bool get (lua_State* L, int index)
  {
    luaL_checktype (L, index, LUA_TBOOLEAN);

    return lua_toboolean (L, index) ? true : false;
  }
};

// char
template <>
struct Stack <char>
{
  static inline void push (lua_State* L, char value)
  {
    char str [2] = { value, 0 };
    lua_pushstring (L, str);
  }

  static inline char get (lua_State* L, int index)
  {
    return luaL_checkstring (L, index) [0];
  }
};

// null terminated string
template <>
struct Stack <char const*>
{
  static inline void push (lua_State* L, char const* str)
  {
    if (str)
      lua_pushstring (L, str);
    else
      lua_pushnil (L);
  }

  static inline char const* get (lua_State* L, int index)
  {
    if (lua_isnil (L, index))
      return 0;
    else
      return luaL_checkstring (L, index);
  }
};

// std::string
template <>
struct Stack <std::string>
{
  static inline void push (lua_State* L, std::string const& str)
  {
    lua_pushstring (L, str.c_str ());
  }

  static inline std::string get (lua_State* L, int index)
  {
    return std::string (luaL_checkstring (L, index));
  }
};

// std::string const&
template <>
struct Stack <std::string const&>
{
  static inline void push (lua_State* L, std::string const& str)
  {
    lua_pushstring (L, str.c_str());
  }

  static inline std::string get (lua_State* L, int index)
  {
    return std::string (luaL_checkstring (L, index));
  }
};

//==============================================================================
/**
  Subclass of a TypeListValues constructable from the Lua stack.
*/

template <typename List, int Start = 1>
struct ArgList
{
};

template <int Start>
struct ArgList <None, Start> : public TypeListValues <None>
{
  ArgList (lua_State*)
  {
  }
};

template <typename Head, typename Tail, int Start>
struct ArgList <TypeList <Head, Tail>, Start>
  : public TypeListValues <TypeList <Head, Tail> >
{
  ArgList (lua_State* L)
    : TypeListValues <TypeList <Head, Tail> > (Stack <Head>::get (L, Start),
                                            ArgList <Tail, Start + 1> (L))
  {
  }
};

//=============================================================================

/**
  Provides a namespace registration in a lua_State.
*/
class Namespace
{
private:
  Namespace& operator= (Namespace const& other);

  lua_State* const L;
  int mutable m_stackSize;

private:
  //============================================================================
  /**
    Error reporting.
  */
  static int luaError (lua_State* L, std::string message)
  {
    assert (lua_isstring (L, lua_upvalueindex (1)));
    std::string s;

    // Get information on the caller's caller to format the message,
    // so the error appears to originate from the Lua source.
    lua_Debug ar;
    int result = lua_getstack (L, 2, &ar);
    if (result != 0)
    {
      lua_getinfo (L, "Sl", &ar);
      s = ar.short_src;
      if (ar.currentline != -1)
      {
        // poor mans int to string to avoid <strstrream>.
        lua_pushnumber (L, ar.currentline);
        s = s + ":" + lua_tostring (L, -1) + ": ";
        lua_pop (L, 1);
      }
    }

    s = s + message;

    return luaL_error (L, s.c_str ());
  }
  
  //----------------------------------------------------------------------------
  /**
    lua_CFunction to report an error writing to a read-only value.

    The name of the variable is in the first upvalue.
  */
  static int readOnlyError (lua_State* L)
  {
    std::string s;
    
    s = s + "'" + lua_tostring (L, lua_upvalueindex (1)) + "' is read-only";

    return luaL_error (L, s.c_str ());
  }
  
  //============================================================================
  /**
    __index metamethod for a namespace or class static members.

    This handles:
      - Retrieving functions and class static methods, stored in the metatable.
      - Reading global and class static data, stored in the __propget table.
      - Reading global and class properties, stored in the __propget table.
  */
  static int indexMetaMethod (lua_State* L)
  {
    int result = 0;
    lua_getmetatable (L, 1);                // push metatable of arg1
    for (;;)
    {
      lua_pushvalue (L, 2);                 // push key arg2
      lua_rawget (L, -2);                   // lookup key in metatable
      if (lua_isnil (L, -1))                // not found
      {
        lua_pop (L, 1);                     // discard nil
        rawgetfield (L, -1, "__propget");   // lookup __propget in metatable
        lua_pushvalue (L, 2);               // push key arg2
        lua_rawget (L, -2);                 // lookup key in __propget
        lua_remove (L, -2);                 // discard __propget
        if (lua_iscfunction (L, -1))
        {
          lua_remove (L, -2);               // discard metatable
          lua_pushvalue (L, 1);             // push arg1
          lua_call (L, 1, 1);               // call cfunction
          result = 1;
          break;
        }
        else
        {
          assert (lua_isnil (L, -1));
          lua_pop (L, 1);                   // discard nil and fall through
        }
      }
      else
      {
        assert (lua_istable (L, -1) || lua_iscfunction (L, -1));
        lua_remove (L, -2);
        result = 1;
        break;
      }

      rawgetfield (L, -1, "__parent");
      if (lua_istable (L, -1))
      {
        // Remove metatable and repeat the search in __parent.
        lua_remove (L, -2);
      }
      else
      {
        // Discard metatable and return nil.
        assert (lua_isnil (L, -1));
        lua_remove (L, -2);
        result = 1;
        break;
      }
    }

    return result;
  }

  //----------------------------------------------------------------------------
  /**
    __newindex metamethod for a namespace or class static members.

    The __propset table stores proxy functions for assignment to:
      - Global and class static data.
      - Global and class properties.
  */
  static int newindexMetaMethod (lua_State* L)
  {
    int result = 0;
    lua_getmetatable (L, 1);                // push metatable of arg1
    for (;;)
    {
      rawgetfield (L, -1, "__propset");     // lookup __propset in metatable
      assert (lua_istable (L, -1));
      lua_pushvalue (L, 2);                 // push key arg2
      lua_rawget (L, -2);                   // lookup key in __propset
      lua_remove (L, -2);                   // discard __propset
      if (lua_iscfunction (L, -1))          // ensure value is a cfunction
      {
        lua_remove (L, -2);                 // discard metatable
        lua_pushvalue (L, 3);               // push new value arg3
        lua_call (L, 1, 0);                 // call cfunction
        result = 0;
        break;
      }
      else
      {
        assert (lua_isnil (L, -1));
        lua_pop (L, 1);
      }

      rawgetfield (L, -1, "__parent");
      if (lua_istable (L, -1))
      {
        // Remove metatable and repeat the search in __parent.
        lua_remove (L, -2);
      }
      else
      {
        assert (lua_isnil (L, -1));
        lua_pop (L, 2);
        result = luaL_error (L,"no writable variable '%s'", lua_tostring (L, 2));
      }
    }

    return result;
  }

  //----------------------------------------------------------------------------
  /**
    lua_CFunction to get a variable.

    This is used for global variables or class static data members.
  */
  template <class T>
  static int getVariable (lua_State* L)
  {
    assert (lua_islightuserdata (L, lua_upvalueindex (1)));
    T const* const data = static_cast <T const*> (lua_touserdata (L, lua_upvalueindex (1)));
    assert (data != 0);
    Stack <T>::push (L, *data);
    return 1;
  }

  //----------------------------------------------------------------------------
  /**
    lua_CFunction to set a variable.

    This is used for global variables or class static data members.
  */
  template <class T>
  static int setVariable (lua_State* L)
  {
    assert (lua_islightuserdata (L, lua_upvalueindex (1)));
    T* const data = static_cast <T*> (lua_touserdata (L, lua_upvalueindex (1)));
    assert (data != 0);
    *data = Stack <T>::get (L, 1);
    return 0;
  }

  //----------------------------------------------------------------------------
  /**
    lua_CFunction to call a function with a return value.

    This is used for global functions, global properties, class static methods,
    and class static properties.
  */
  template <class Func,
            class ReturnType = typename FuncTraits <Func>::ReturnType>
  struct CallFunction
  {
    typedef typename FuncTraits <Func>::Params Params;
    static int call (lua_State* L)
    {
      assert (lua_isuserdata (L, lua_upvalueindex (1)));
      Func const& fp = *static_cast <Func const*> (
        lua_touserdata (L, lua_upvalueindex (1)));
      assert (fp != 0);
      ArgList <Params> args (L);
      Stack <typename FuncTraits <Func>::ReturnType>::push (
        L, FuncTraits <Func>::call (fp, args));
      return 1;
    }
  };

  //----------------------------------------------------------------------------
  /**
    lua_CFunction to call a function with no return value.

    This is used for global functions, global properties, class static methods,
    and class static properties.
  */
  template <class Func>
  struct CallFunction <Func, void>
  {
    typedef typename FuncTraits <Func>::Params Params;
    static int call (lua_State* L)
    {
      assert (lua_isuserdata (L, lua_upvalueindex (1)));
      Func const& fp = *static_cast <Func const*> (lua_touserdata (L, lua_upvalueindex (1)));
      assert (fp != 0);
      ArgList <Params> args (L);
      FuncTraits <Func>::call (fp, args);
      return 0;
    }
  };

  //============================================================================
  /**
    lua_CFunction to call a class member function with a return value.
  */
  template <class MemFn,
            class ReturnType = typename FuncTraits <MemFn>::ReturnType>
  struct CallMemberFunction
  {
    typedef typename FuncTraits <MemFn>::ClassType T;
    typedef typename FuncTraits <MemFn>::Params Params;

    static int call (lua_State* L)
    {
      assert (lua_isuserdata (L, lua_upvalueindex (1)));
      T* const t = Detail::Userdata::get <T> (L, 1, false);
      MemFn fp = *static_cast <MemFn*> (lua_touserdata (L, lua_upvalueindex (1)));
      ArgList <Params, 2> args (L);
      Stack <ReturnType>::push (L, FuncTraits <MemFn>::call (t, fp, args));
      return 1;
    }

    static int callConst (lua_State* L)
    {
      assert (lua_isuserdata (L, lua_upvalueindex (1)));
      T const* const t = Detail::Userdata::get <T> (L, 1, true);
      MemFn fp = *static_cast <MemFn*> (lua_touserdata (L, lua_upvalueindex (1)));
      ArgList <Params, 2> args(L);
      Stack <ReturnType>::push (L, FuncTraits <MemFn>::call (t, fp, args));
      return 1;
    }
  };

  //----------------------------------------------------------------------------
  /**
    lua_CFunction to call a class member function with no return value.
  */
  template <class MemFn>
  struct CallMemberFunction <MemFn, void>
  {
    typedef typename FuncTraits <MemFn>::ClassType T;
    typedef typename FuncTraits <MemFn>::Params Params;

    static int call (lua_State* L)
    {
      T* const t = Detail::Userdata::get <T> (L, 1, false);
      MemFn const fp = *static_cast <MemFn*> (lua_touserdata (L, lua_upvalueindex (1)));
      ArgList <Params, 2> args (L);
      FuncTraits <MemFn>::call (t, fp, args);
      return 0;
    }

    static int callConst (lua_State* L)
    {
      T const* const t = Detail::Userdata::get <T> (L, 1, true);
      MemFn const fp = *static_cast <MemFn*> (lua_touserdata (L, lua_upvalueindex (1)));
      ArgList <Params, 2> args (L);
      FuncTraits <MemFn>::call (t, fp, args);
      return 0;
    }
  };

  //----------------------------------------------------------------------------
  /**
    lua_CFunction to call a class member lua_CFunction
  */
  template <class T>
  struct CallMemberCFunction
  {
    static int call (lua_State* L)
    {
      typedef int (T::*MFP)(lua_State* L);
      T* const t = Detail::Userdata::get <T> (L, 1, false);
      MFP const mfp = *static_cast <MFP*> (lua_touserdata (L, lua_upvalueindex (1)));
      return (t->*mfp) (L);
    }

    static int callConst (lua_State* L)
    {
      typedef int (T::*MFP)(lua_State* L);
      T const* const t = Detail::Userdata::get <T> (L, 1, true);
      MFP const mfp = *static_cast <MFP*> (lua_touserdata (L, lua_upvalueindex (1)));
      return (t->*mfp) (L);
    }
  };

  //----------------------------------------------------------------------------

  // SFINAE Helpers

  template <class MemFn, bool isConst>
  struct CallMemberFunctionHelper
  {
    static void add (lua_State* L, char const* name, MemFn mf)
    {
      new (lua_newuserdata (L, sizeof (MemFn))) MemFn (mf);
      lua_pushcclosure (L, &CallMemberFunction <MemFn>::callConst, 1);
      lua_pushvalue (L, -1);
      rawsetfield (L, -5, name); // const table
      rawsetfield (L, -3, name); // class table
    }
  };

  template <class MemFn>
  struct CallMemberFunctionHelper <MemFn, false>
  {
    static void add (lua_State* L, char const* name, MemFn mf)
    {
      new (lua_newuserdata (L, sizeof (MemFn))) MemFn (mf);
      lua_pushcclosure (L, &CallMemberFunction <MemFn>::call, 1);
      rawsetfield (L, -3, name); // class table
    }
  };

  //----------------------------------------------------------------------------
  /**
    Pop the Lua stack.
  */
  void pop (int n) const
  {
    if (m_stackSize >= n && lua_gettop (L) >= n)
    {
      lua_pop (L, n);
      m_stackSize -= n;
    }
    else
    {
      throw std::logic_error ("invalid stack");
    }
  }

private:
  //============================================================================
  //
  // ClassBase
  //
  //============================================================================
  /**
    Factored base to reduce template instantiations.
  */
  class ClassBase
  {
  private:
    ClassBase& operator= (ClassBase const& other);

  protected:
    friend class Namespace;

    lua_State* const L;
    int mutable m_stackSize;

  protected:
    //--------------------------------------------------------------------------
    /**
      __index metamethod for a class.

      This implements member functions, data members, and property members.
      Functions are stored in the metatable and const metatable. Data members
      and property members are in the __propget table.

      If the key is not found, the search proceeds up the hierarchy of base
      classes.
    */
    static int indexMetaMethod (lua_State* L)
    {
      int result = 0;

      assert (lua_isuserdata (L, 1));               // warn on security bypass
      lua_getmetatable (L, 1);                      // get metatable for object
      for (;;)
      {
        lua_pushvalue (L, 2);                       // push key arg2
        lua_rawget (L, -2);                         // lookup key in metatable
        if (lua_iscfunction (L, -1))                // ensure its a cfunction
        {
          lua_remove (L, -2);                       // remove metatable
          result = 1;
          break;
        }
        else if (lua_isnil (L, -1))
        {
          lua_pop (L, 1);
        }
        else
        {
          lua_pop (L, 2);
          throw std::logic_error ("not a cfunction");
        }

        rawgetfield (L, -1, "__propget");           // get __propget table
        if (lua_istable (L, -1))                    // ensure it is a table
        {
          lua_pushvalue (L, 2);                     // push key arg2
          lua_rawget (L, -2);                       // lookup key in __propget
          lua_remove (L, -2);                       // remove __propget
          if (lua_iscfunction (L, -1))              // ensure its a cfunction
          {
            lua_remove (L, -2);                     // remove metatable
            lua_pushvalue (L, 1);                   // push class arg1
            lua_call (L, 1, 1);
            result = 1;
            break;
          }
          else if (lua_isnil (L, -1))
          {
            lua_pop (L, 1);
          }
          else
          {
            lua_pop (L, 2);

            // We only put cfunctions into __propget.
            throw std::logic_error ("not a cfunction");
          }
        }
        else
        {
          lua_pop (L, 2);

          // __propget is missing, or not a table.
          throw std::logic_error ("missing __propget table");
        }

        // Repeat the lookup in the __parent metafield,
        // or return nil if the field doesn't exist.
        rawgetfield (L, -1, "__parent");
        if (lua_istable (L, -1))
        {
          // Remove metatable and repeat the search in __parent.
          lua_remove (L, -2);
        }
        else if (lua_isnil (L, -1))
        {
          result = 1;
          break;
        }
        else
        {
          lua_pop (L, 2);

          throw std::logic_error ("__parent is not a table");
        }
      }

      return result;
    }

    //--------------------------------------------------------------------------
    /**
      __newindex metamethod for classes.

      This supports writable variables and properties on class objects. The
      corresponding object is passed in the first parameter to the set function.
    */
    static int newindexMetaMethod (lua_State* L)
    {
      int result = 0;

      lua_getmetatable (L, 1);

      for (;;)
      {
        // Check __propset
        rawgetfield (L, -1, "__propset");
        if (!lua_isnil (L, -1))
        {
          lua_pushvalue (L, 2);
          lua_rawget (L, -2);
          if (!lua_isnil (L, -1))
          {
            // found it, call the setFunction.
            assert (lua_isfunction (L, -1));
            lua_pushvalue (L, 1);
            lua_pushvalue (L, 3);
            lua_call (L, 2, 0);
            result = 0;
            break;
          }
          lua_pop (L, 1);
        }
        lua_pop (L, 1);

        // Repeat the lookup in the __parent metafield.
        rawgetfield (L, -1, "__parent");
        if (lua_isnil (L, -1))
        {
          // Either the property or __parent must exist.
          result = luaL_error (L,
            "no member named '%s'", lua_tostring (L, 2));
        }
        lua_remove (L, -2);
      }

      return result;
    }

    //--------------------------------------------------------------------------
    /**
      Create the const table.
    */
    void createConstTable (char const* name)
    {
      lua_newtable (L);
      lua_pushvalue (L, -1);
      lua_setmetatable (L, -2);
      lua_pushboolean (L, 1);
      lua_rawsetp (L, -2, Detail::getIdentityKey ());
      lua_pushstring (L, (std::string ("const ") + name).c_str ());
      rawsetfield (L, -2, "__type");
      lua_pushcfunction (L, &indexMetaMethod);
      rawsetfield (L, -2, "__index");
      lua_pushcfunction (L, &newindexMetaMethod);
      rawsetfield (L, -2, "__newindex");
      lua_newtable (L);
      rawsetfield (L, -2, "__propget");
      
      if (Detail::Security::hideMetatables ())
      {
        lua_pushnil (L);
        rawsetfield (L, -2, "__metatable");
      }
    }

    //--------------------------------------------------------------------------
    /**
      Create the class table.

      The Lua stack should have the const table on top.
    */
    void createClassTable (char const* name)
    {
      lua_newtable (L);
      lua_pushvalue (L, -1);
      lua_setmetatable (L, -2);
      lua_pushboolean (L, 1);
      lua_rawsetp (L, -2, Detail::getIdentityKey ());
      lua_pushstring (L, name);
      rawsetfield (L, -2, "__type");
      lua_pushcfunction (L, &indexMetaMethod);
      rawsetfield (L, -2, "__index");
      lua_pushcfunction (L, &newindexMetaMethod);
      rawsetfield (L, -2, "__newindex");
      lua_newtable (L);
      rawsetfield (L, -2, "__propget");
      lua_newtable (L);
      rawsetfield (L, -2, "__propset");

      lua_pushvalue (L, -2);
      rawsetfield (L, -2, "__const"); // point to const table

      lua_pushvalue (L, -1);
      rawsetfield (L, -3, "__class"); // point const table to class table

      if (Detail::Security::hideMetatables ())
      {
        lua_pushnil (L);
        rawsetfield (L, -2, "__metatable");
      }
    }

    //--------------------------------------------------------------------------
    /**
      Create the static table.

      The Lua stack should have:
        -1 class table
        -2 const table
        -3 enclosing namespace
    */
    void createStaticTable (char const* name)
    {
      lua_newtable (L);
      lua_newtable (L);
      lua_pushvalue (L, -1);
      lua_setmetatable (L, -3);
      lua_insert (L, -2);
      rawsetfield (L, -5, name);

#if 0
      lua_pushlightuserdata (L, this);
      lua_pushcclosure (L, &tostringMetaMethod, 1);
      rawsetfield (L, -2, "__tostring");
#endif
      lua_pushcfunction (L, &Namespace::indexMetaMethod);
      rawsetfield (L, -2, "__index");
      lua_pushcfunction (L, &Namespace::newindexMetaMethod);
      rawsetfield (L, -2, "__newindex");
      lua_newtable (L);
      rawsetfield (L, -2, "__propget");
      lua_newtable (L);
      rawsetfield (L, -2, "__propset");

      lua_pushvalue (L, -2);
      rawsetfield (L, -2, "__class"); // point to class table

      if (Detail::Security::hideMetatables ())
      {
        lua_pushnil (L);
        rawsetfield (L, -2, "__metatable");
      }
    }

    //==========================================================================
    /**
      lua_CFunction to construct a class object wrapped in a container.
    */
    template <class Params, class C>
    static int ctorContainerProxy (lua_State* L)
    {
      typedef typename ContainerTraits <C>::Type T;
      ArgList <Params, 2> args (L);
      T* const p = Constructor <T, Params>::call (args);
      Detail::UserdataSharedHelper <C, false>::push (L, p);
      return 1;
    }

    //--------------------------------------------------------------------------
    /**
      lua_CFunction to construct a class object in-place in the userdata.
    */
    template <class Params, class T>
    static int ctorPlacementProxy (lua_State* L)
    {
      ArgList <Params, 2> args (L);
      Constructor <T, Params>::call (Detail::UserdataValue <T>::place (L), args);
      return 1;
    }

    //--------------------------------------------------------------------------
    /**
      Pop the Lua stack.
    */
    void pop (int n) const
    {
      if (m_stackSize >= n && lua_gettop (L) >= n)
      {
        lua_pop (L, n);
        m_stackSize -= n;
      }
      else
      {
        throw std::logic_error ("invalid stack");
      }
    }

  public:
    //--------------------------------------------------------------------------
    explicit ClassBase (lua_State* L_)
      : L (L_)
      , m_stackSize (0)
    {
    }

    //--------------------------------------------------------------------------
    /**
      Copy Constructor.
    */
    ClassBase (ClassBase const& other)
      : L (other.L)
      , m_stackSize (0)
    {
      m_stackSize = other.m_stackSize;
      other.m_stackSize = 0;
    }

    ~ClassBase ()
    {
      pop (m_stackSize);
    }
  };

  //============================================================================
  //
  // Class
  //
  //============================================================================
  /**
    Provides a class registration in a lua_State.

    After contstruction the Lua stack holds these objects:
      -1 static table
      -2 class table
      -3 const table
      -4 (enclosing namespace)
  */
  template <class T>
  class Class : public ClassBase
  {
  private:
    //--------------------------------------------------------------------------
    /**
      __gc metamethod for a class.
    */
    static int gcMetaMethod (lua_State* L)
    {
      Detail::Userdata* ud = Detail::Userdata::getExact <T> (L, 1);
      ud->~Userdata ();
      return 0;
    }

    //--------------------------------------------------------------------------
    /**
      lua_CFunction to get a class data member.
    */
    template <typename U>
    static int getProperty (lua_State* L)
    {
      T const* const t = Detail::Userdata::get <T> (L, 1, true);
      U T::** mp = static_cast <U T::**> (lua_touserdata (L, lua_upvalueindex (1)));
      Stack <U>::push (L, t->**mp);
      return 1;
    }

    //--------------------------------------------------------------------------
    /**
      lua_CFunction to set a class data member.

      @note The expected class name is in upvalue 1, and the pointer to the
            data member is in upvalue 2.
    */
    template <typename U>
    static int setProperty (lua_State* L)
    {
      T* const t = Detail::Userdata::get <T> (L, 1, false);
      U T::** mp = static_cast <U T::**> (lua_touserdata (L, lua_upvalueindex (1)));
      t->**mp = Stack <U>::get (L, 2);
      return 0;
    }

 public:
    //==========================================================================
    /**
      Register a new class or add to an existing class registration.
    */
    Class (char const* name, Namespace const* parent) : ClassBase (parent->L)
    {
      m_stackSize = parent->m_stackSize + 3;
      parent->m_stackSize = 0;

      assert (lua_istable (L, -1));
      rawgetfield (L, -1, name);
      
      if (lua_isnil (L, -1))
      {
        lua_pop (L, 1);

        createConstTable (name);
        lua_pushcfunction (L, &gcMetaMethod);
        rawsetfield (L, -2, "__gc");

        createClassTable (name);
        lua_pushcfunction (L, &gcMetaMethod);
        rawsetfield (L, -2, "__gc");

        createStaticTable (name);

        // Map T back to its tables.
        lua_pushvalue (L, -1);
        lua_rawsetp (L, LUA_REGISTRYINDEX, Detail::ClassInfo <T>::getStaticKey ());
        lua_pushvalue (L, -2);
        lua_rawsetp (L, LUA_REGISTRYINDEX, Detail::ClassInfo <T>::getClassKey ());
        lua_pushvalue (L, -3);
        lua_rawsetp (L, LUA_REGISTRYINDEX, Detail::ClassInfo <T>::getConstKey ());
      }
      else
      {
        rawgetfield (L, -1, "__class");
        rawgetfield (L, -1, "__const");

        // Reverse the top 3 stack elements
        lua_insert (L, -3);
        lua_insert (L, -2);
      }
    }

    //==========================================================================
    /**
      Derive a new class.
    */
    Class (char const* name, Namespace const* parent, void const* const staticKey)
      : ClassBase (parent->L)
    {
      m_stackSize = parent->m_stackSize + 3;
      parent->m_stackSize = 0;

      assert (lua_istable (L, -1));

      createConstTable (name);
      lua_pushcfunction (L, &gcMetaMethod);
      rawsetfield (L, -2, "__gc");

      createClassTable (name);
      lua_pushcfunction (L, &gcMetaMethod);
      rawsetfield (L, -2, "__gc");

      createStaticTable (name);

      lua_rawgetp (L, LUA_REGISTRYINDEX, staticKey);
      assert (lua_istable (L, -1));
      rawgetfield (L, -1, "__class");
      assert (lua_istable (L, -1));
      rawgetfield (L, -1, "__const");
      assert (lua_istable (L, -1));

      rawsetfield (L, -6, "__parent");
      rawsetfield (L, -4, "__parent");
      rawsetfield (L, -2, "__parent");

      lua_pushvalue (L, -1);
      lua_rawsetp (L, LUA_REGISTRYINDEX, Detail::ClassInfo <T>::getStaticKey ());
      lua_pushvalue (L, -2);
      lua_rawsetp (L, LUA_REGISTRYINDEX, Detail::ClassInfo <T>::getClassKey ());
      lua_pushvalue (L, -3);
      lua_rawsetp (L, LUA_REGISTRYINDEX, Detail::ClassInfo <T>::getConstKey ());
    }

    //--------------------------------------------------------------------------
    /**
      Continue registration in the enclosing namespace.
    */
    Namespace endClass ()
    {
      return Namespace (this);
    }

    //--------------------------------------------------------------------------
    /**
      Add or replace a static data member.
    */
    template <class U>
    Class <T>& addStaticData (char const* name, U* pu, bool isWritable = true)
    {
      assert (lua_istable (L, -1));

      rawgetfield (L, -1, "__propget");
      assert (lua_istable (L, -1));
      lua_pushlightuserdata (L, pu);
      lua_pushcclosure (L, &getVariable <U>, 1);
      rawsetfield (L, -2, name);
      lua_pop (L, 1);

      rawgetfield (L, -1, "__propset");
      assert (lua_istable (L, -1));
      if (isWritable)
      {
        lua_pushlightuserdata (L, pu);
        lua_pushcclosure (L, &setVariable <U>, 1);
      }
      else
      {
        lua_pushstring (L, name);
        lua_pushcclosure (L, &readOnlyError, 1);
      }
      rawsetfield (L, -2, name);
      lua_pop (L, 1);

      return *this;
    }

    //--------------------------------------------------------------------------
    /**
      Add or replace a static property member.

      If the set function is null, the property is read-only.
    */
    template <class U>
    Class <T>& addStaticProperty (char const* name, U (*get)(), void (*set)(U) = 0)
    {
      typedef U (*get_t)();
      typedef void (*set_t)(U);
      
      assert (lua_istable (L, -1));

      rawgetfield (L, -1, "__propget");
      assert (lua_istable (L, -1));
      new (lua_newuserdata (L, sizeof (get))) get_t (get);
      lua_pushcclosure (L, &CallFunction <U (*) (void)>::call, 1);
      rawsetfield (L, -2, name);
      lua_pop (L, 1);

      rawgetfield (L, -1, "__propset");
      assert (lua_istable (L, -1));
      if (set != 0)
      {
        new (lua_newuserdata (L, sizeof (set))) set_t (set);
        lua_pushcclosure (L, &CallFunction <void (*) (U)>::call, 1);
      }
      else
      {
        lua_pushstring (L, name);
        lua_pushcclosure (L, &readOnlyError, 1);
      }
      rawsetfield (L, -2, name);
      lua_pop (L, 1);

      return *this;
    }

    //--------------------------------------------------------------------------
    /**
      Add or replace a static member function.
    */
    template <class FP>
    Class <T>& addStaticFunction (char const* name, FP const fp)
    {
      new (lua_newuserdata (L, sizeof (fp))) FP (fp);
      lua_pushcclosure (L, &CallFunction <FP>::call, 1);
      rawsetfield (L, -2, name);

      return *this;
    }

    //--------------------------------------------------------------------------
    /**
      Add or replace a lua_CFunction.
    */
    Class <T>& addStaticCFunction (char const* name, int (*const fp)(lua_State*))
    {
      lua_pushcfunction (L, fp);
      rawsetfield (L, -2, name);
      return *this;
    }

    //--------------------------------------------------------------------------
    /**
      Add or replace a data member.
    */
    template <class U>
    Class <T>& addData (char const* name, const U T::* mp, bool isWritable = true)
    {
      typedef const U T::*mp_t;

      // Add to __propget in class and const tables.
      {
        rawgetfield (L, -2, "__propget");
        rawgetfield (L, -4, "__propget");
        new (lua_newuserdata (L, sizeof (mp_t))) mp_t (mp);
        lua_pushcclosure (L, &getProperty <U>, 1);
        lua_pushvalue (L, -1);
        rawsetfield (L, -4, name);
        rawsetfield (L, -2, name);
        lua_pop (L, 2);
      }

      if (isWritable)
      {
        // Add to __propset in class table.
        rawgetfield (L, -2, "__propset");
        assert (lua_istable (L, -1));
        new (lua_newuserdata (L, sizeof (mp_t))) mp_t (mp);
        lua_pushcclosure (L, &setProperty <U>, 1);
        rawsetfield (L, -2, name);
        lua_pop (L, 1);
      }

      return *this;
    }

    //--------------------------------------------------------------------------
    /**
      Add or replace a property member.
    */
    template <class TG, class TS>
    Class <T>& addProperty (char const* name, TG (T::* get) () const, void (T::* set) (TS))
    {
      // Add to __propget in class and const tables.
      {
        rawgetfield (L, -2, "__propget");
        rawgetfield (L, -4, "__propget");
        typedef TG (T::*get_t) () const;
        new (lua_newuserdata (L, sizeof (get_t))) get_t (get);
        lua_pushcclosure (L, &CallMemberFunction <get_t>::callConst, 1);
        lua_pushvalue (L, -1);
        rawsetfield (L, -4, name);
        rawsetfield (L, -2, name);
        lua_pop (L, 2);
      }

      {
        // Add to __propset in class table.
        rawgetfield (L, -2, "__propset");
        assert (lua_istable (L, -1));
        typedef void (T::* set_t) (TS);
        new (lua_newuserdata (L, sizeof (set_t))) set_t (set);
        lua_pushcclosure (L, &CallMemberFunction <set_t>::call, 1);
        rawsetfield (L, -2, name);
        lua_pop (L, 1);
      }

      return *this;
    }

    // read-only
    template <class TG>
    Class <T>& addProperty (char const* name, TG (T::* get) () const)
    {
      // Add to __propget in class and const tables.
      rawgetfield (L, -2, "__propget");
      rawgetfield (L, -4, "__propget");
      typedef TG (T::*get_t) () const;
      new (lua_newuserdata (L, sizeof (get_t))) get_t (get);
      lua_pushcclosure (L, &CallMemberFunction <get_t>::callConst, 1);
      lua_pushvalue (L, -1);
      rawsetfield (L, -4, name);
      rawsetfield (L, -2, name);
      lua_pop (L, 2);

      return *this;
    }

    //--------------------------------------------------------------------------
    /**
      Add or replace a property member, by proxy.

      When a class is closed for modification and does not provide (or cannot
      provide) the function signatures necessary to implement get or set for
      a property, this will allow non-member functions act as proxies.

      Both the get and the set functions require a T const* and T* in the first
      argument respectively.
    */
    template <class TG, class TS>
    Class <T>& addProperty (char const* name, TG (*get) (T const*), void (*set) (T*, TS))
    {
      // Add to __propget in class and const tables.
      {
        rawgetfield (L, -2, "__propget");
        rawgetfield (L, -4, "__propget");
        typedef TG (*get_t) (T const*);
        new (lua_newuserdata (L, sizeof (get_t))) get_t (get);
        lua_pushcclosure (L, &CallFunction <get_t>::call, 1);
        lua_pushvalue (L, -1);
        rawsetfield (L, -4, name);
        rawsetfield (L, -2, name);
        lua_pop (L, 2);
      }

      if (set != 0)
      {
        // Add to __propset in class table.
        rawgetfield (L, -2, "__propset");
        assert (lua_istable (L, -1));
        typedef void (*set_t) (T*, TS);
        new (lua_newuserdata (L, sizeof (set_t))) set_t (set);
        lua_pushcclosure (L, &CallFunction <set_t>::call, 1);
        rawsetfield (L, -2, name);
        lua_pop (L, 1);
      }

      return *this;
    }

    // read-only
    template <class TG, class TS>
    Class <T>& addProperty (char const* name, TG (*get) (T const*))
    {
      // Add to __propget in class and const tables.
      rawgetfield (L, -2, "__propget");
      rawgetfield (L, -4, "__propget");
      typedef TG (*get_t) (T const*);
      new (lua_newuserdata (L, sizeof (get_t))) get_t (get);
      lua_pushcclosure (L, &CallFunction <get_t>::call, 1);
      lua_pushvalue (L, -1);
      rawsetfield (L, -4, name);
      rawsetfield (L, -2, name);
      lua_pop (L, 2);

      return *this;
    }

    //--------------------------------------------------------------------------
    /**
      Add or replace a member function.
    */
    template <class MemFn>
    Class <T>& addFunction (char const* name, MemFn mf)
    {
      CallMemberFunctionHelper <MemFn, FuncTraits <MemFn>::isConstMemberFunction>::add (L, name, mf);
      return *this;
    }

    //--------------------------------------------------------------------------
    /**
      Add or replace a member lua_CFunction.
    */
    Class <T>& addCFunction (char const* name, int (T::*mfp)(lua_State*))
    {
      typedef int (T::*MFP)(lua_State*);
      assert (lua_istable (L, -1));
      new (lua_newuserdata (L, sizeof (mfp))) MFP (mfp);
      lua_pushcclosure (L, &CallMemberCFunction <T>::call, 1);
      rawsetfield (L, -3, name); // class table

      return *this;
    }

    //--------------------------------------------------------------------------
    /**
      Add or replace a const member lua_CFunction.
    */
    Class <T>& addCFunction (char const* name, int (T::*mfp)(lua_State*) const)
    {
      typedef int (T::*MFP)(lua_State*) const;
      assert (lua_istable (L, -1));
      new (lua_newuserdata (L, sizeof (mfp))) MFP (mfp);
      lua_pushcclosure (L, &CallMemberCFunction <T>::callConst, 1);
      lua_pushvalue (L, -1);
      rawsetfield (L, -5, name); // const table
      rawsetfield (L, -3, name); // class table

      return *this;
    }

    //--------------------------------------------------------------------------
    /**
      Add or replace a primary Constructor.

      The primary Constructor is invoked when calling the class type table
      like a function.

      The template parameter should be a function pointer type that matches
      the desired Constructor (since you can't take the address of a Constructor
      and pass it as an argument).
    */
    template <class MemFn, class C>
    Class <T>& addConstructor ()
    {
      lua_pushcclosure (L,
        &ctorContainerProxy <typename FuncTraits <MemFn>::Params, C>, 0);
      rawsetfield(L, -2, "__call");

      return *this;
    }

    template <class MemFn>
    Class <T>& addConstructor ()
    {
      lua_pushcclosure (L,
        &ctorPlacementProxy <typename FuncTraits <MemFn>::Params, T>, 0);
      rawsetfield(L, -2, "__call");

      return *this;
    }
  };

private:
  //============================================================================
  //
  // Namespace (Cont.)
  //
  //============================================================================

  //----------------------------------------------------------------------------
  /**
    Opens the global namespace.
  */
  explicit Namespace (lua_State* L_)
    : L (L_)
    , m_stackSize (0)
  {
    lua_getglobal (L, "_G");
    ++m_stackSize;
  }

  //----------------------------------------------------------------------------
  /**
    Opens a namespace for registrations.

    The namespace is created if it doesn't already exist. The parent
    namespace is at the top of the Lua stack.
  */
  Namespace (char const* name, Namespace const* parent)
    : L (parent->L)
    , m_stackSize (0)
  {
    m_stackSize = parent->m_stackSize + 1;
    parent->m_stackSize = 0;

    assert (lua_istable (L, -1));
    rawgetfield (L, -1, name);
    if (lua_isnil (L, -1))
    {
      lua_pop (L, 1);

      lua_newtable (L);
      lua_pushvalue (L, -1);
      lua_setmetatable (L, -2);
      lua_pushcfunction (L, &indexMetaMethod);
      rawsetfield (L, -2, "__index");
      lua_pushcfunction (L, &newindexMetaMethod);
      rawsetfield (L, -2, "__newindex");
      lua_newtable (L);
      rawsetfield (L, -2, "__propget");
      lua_newtable (L);
      rawsetfield (L, -2, "__propset");
      lua_pushvalue (L, -1);
      rawsetfield (L, -3, name);
#if 0
      lua_pushcfunction (L, &tostringMetaMethod);
      rawsetfield (L, -2, "__tostring");
#endif
    }
  }

  //----------------------------------------------------------------------------
  /**
    Creates a continued registration from a child namespace.
  */
  explicit Namespace (Namespace const* child)
    : L (child->L)
    , m_stackSize (0)
  {
    m_stackSize = child->m_stackSize - 1;
    child->m_stackSize = 1;
    child->pop (1);

    // It is not necessary or valid to call
    // endNamespace() for the global namespace!
    //
    assert (m_stackSize != 0);
  }

  //----------------------------------------------------------------------------
  /**
    Creates a continued registration from a child class.
  */
  explicit Namespace (ClassBase const* child)
    : L (child->L)
    , m_stackSize (0)
  {
    m_stackSize = child->m_stackSize - 3;
    child->m_stackSize = 3;
    child->pop (3);
  }

public:
  //----------------------------------------------------------------------------
  /**
    Copy Constructor.

    Ownership of the stack is transferred to the new object. This happens when
    the compiler emits temporaries to hold these objects while chaining
    registrations across namespaces.
  */
  Namespace (Namespace const& other) : L (other.L)
  {
    m_stackSize = other.m_stackSize;
    other.m_stackSize = 0;
  }

  //----------------------------------------------------------------------------
  /**
    Closes this namespace registration.
  */
  ~Namespace ()
  {
    pop (m_stackSize);
  }

  //----------------------------------------------------------------------------
  /**
    Open the global namespace.
  */
  static Namespace getGlobalNamespace (lua_State* L)
  {
    return Namespace (L);
  }

  //----------------------------------------------------------------------------
  /**
    Open a new or existing namespace for registrations.
  */
  Namespace beginNamespace (char const* name)
  {
    return Namespace (name, this);
  }

  //----------------------------------------------------------------------------
  /**
    Continue namespace registration in the parent.

    Do not use this on the global namespace.
  */
  Namespace endNamespace ()
  {
    return Namespace (this);
  }

  //----------------------------------------------------------------------------
  /**
    Add or replace a variable.
  */
  template <class T>
  Namespace& addVariable (char const* const name, T* const pt, bool const isWritable = true)
  {
    assert (lua_istable (L, -1));

    rawgetfield (L, -1, "__propget");
    assert (lua_istable (L, -1));
    lua_pushlightuserdata (L, pt);
    lua_pushcclosure (L, &getVariable <T>, 1);
    rawsetfield (L, -2, name);
    lua_pop (L, 1);

    rawgetfield (L, -1, "__propset");
    assert (lua_istable (L, -1));
    if (isWritable)
    {
      lua_pushlightuserdata (L, pt);
      lua_pushcclosure (L, &setVariable <T>, 1);
    }
    else
    {
      lua_pushstring (L, name);
      lua_pushcclosure (L, &readOnlyError, 1);
    }
    rawsetfield (L, -2, name);
    lua_pop (L, 1);

    return *this;
  }
  
  //----------------------------------------------------------------------------
  /**
    Add or replace a property.

    If the set function is omitted or null, the property is read-only.
  */
  template <class TG, class TS>
  Namespace& addProperty (char const* name, TG (*get) (), void (*set)(TS) = 0)
  {
    assert (lua_istable (L, -1));

    rawgetfield (L, -1, "__propget");
    assert (lua_istable (L, -1));
    lua_pushlightuserdata (L, get);
    lua_pushcclosure (L, &CallFunction <TG (*) (void)>::call, 1);
    rawsetfield (L, -2, name);
    lua_pop (L, 1);

    rawgetfield (L, -1, "__propset");
    assert (lua_istable (L, -1));
    if (set != 0)
    {
      lua_pushlightuserdata (L, set);
      lua_pushcclosure (L, &CallFunction <void (*) (TS)>::call, 1);
    }
    else
    {
      lua_pushstring (L, name);
      lua_pushcclosure (L, &readOnlyError, 1);
    }
    rawsetfield (L, -2, name);
    lua_pop (L, 1);

    return *this;
  }

  //----------------------------------------------------------------------------
  /**
    Add or replace a function.
  */
  template <class FP>
  Namespace& addFunction (char const* name, FP const fp)
  {
    assert (lua_istable (L, -1));

    new (lua_newuserdata (L, sizeof (fp))) FP (fp);
    lua_pushcclosure (L, &CallFunction <FP>::call, 1);
    rawsetfield (L, -2, name);

    return *this;
  }

  //----------------------------------------------------------------------------
  /**
    Add or replace a lua_CFunction.
  */
  Namespace& addCFunction (char const* name, int (*const fp)(lua_State*))
  {
    lua_pushcfunction (L, fp);
    rawsetfield (L, -2, name);

    return *this;
  }

  //----------------------------------------------------------------------------
  /**
    Open a new or existing class for registrations.
  */
  template <class T>
  Class <T> beginClass (char const* name)
  {
    return Class <T> (name, this);
  }

  //----------------------------------------------------------------------------
  /**
    Derive a new class for registrations.

    To continue registrations for the class later, use beginClass().
    Do not call deriveClass() again.
  */
  template <class T, class U>
  Class <T> deriveClass (char const* name)
  {
    return Class <T> (name, this, Detail::ClassInfo <U>::getStaticKey ());
  }
};

//==============================================================================
/**
  Retrieve the global namespace.

  It is recommended to put your namespace inside the global namespace, and then
  add your classes and functions to it, rather than adding many classes and
  functions directly to the global namespace.
*/
inline Namespace getGlobalNamespace (lua_State* L)
{
  return Namespace::getGlobalNamespace (L);
}

//------------------------------------------------------------------------------
/**
  Push objects onto the Lua stack.
*/
template <class T>
inline void push (lua_State* L, T t)
{
  Stack <T>::push (L, t);
}

//------------------------------------------------------------------------------
/**
  Set a global value in the lua_State.
*/
template <class T>
inline void setglobal (lua_State* L, T t, char const* name)
{
  push (L, t);
  lua_setglobal (L, name);
}

//------------------------------------------------------------------------------
/**
  Change whether or not metatables are hidden (on by default).
*/
inline void setHideMetatables (bool shouldHide)
{
  Detail::Security::setHideMetatables (shouldHide);
}

}

//==============================================================================

#endif
