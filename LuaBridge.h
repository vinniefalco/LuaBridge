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
#include <stdint.h>
#include <string.h>

//==============================================================================
/**
  @mainpage LuaBridge: Simple C++ to Lua bindings.

  @details

  # LuaBridge

  [LuaBridge][3] is a lightweight, dependency-free library for making C++ data,
  functions, and classes available to Lua. It works with Lua revisions starting
  from 5.1.2. [Lua][5] is a powerful, fast, lightweight, embeddable scripting
  language.

  LuaBridge offers the following features:

  - Nothing to compile, just include headers!

  - Simple, light, and nothing else needed (like Boost).

  - Supports different object lifetime management models.

  - Convenient, type-safe access to the Lua stack.

  - Automatic function parameter type binding.

  - Does not require C++11.

  LuaBridge is distributed as set of header files. You simply add 
  `#include "LuaBridge.h"` where you want to bind your functions, classes, and
  variables. There are no additional source files, no compilation settings, and
  no Makefiles or IDE-specific project files. LuaBridge is easy to integrate.

  C++ concepts like variables and classes are made available to Lua through
  a process called _registration_. Because Lua is weakly typed, the resulting
  structure is not rigid. The API is based on C++ template metaprogramming.
  It contains template code to automatically generate at compile-time the
  various Lua C API calls necessary to export your program's classes and
  functions to the Lua environment.

  ## LuaBridge Demo and Tests

  LuaBridge provides both a command line program and a stand-alone graphical
  program for compiling and running the test suite. The graphical program brings
  up an interactive window where you can enter execute Lua statements in a
  persistent environment. This application is cross platform and works on
  Windows, Mac OS, iOS, Android, and GNU/Linux systems with X11. The stand-alone
  program should work anywhere. Both of these applications include LuaBridge,
  Lua version 5.2, and the code necessary to produce a cross platform graphic
  application. They are all together in a separate repository, with no
  additional dependencies, available here:

  [LuaBridge Demo and Tests][4]

  <a href="https://github.com/vinniefalco/LuaBridgeDemo">
  <img src="http://vinniefalco.github.com/LuaBridgeDemo/LuaBridgeDemoScreenshot.png">
  </a><br>

  ## Registration

  There are four types of objects that LuaBridge can register:

  - **Data**: Global varaibles, static class data members, and class data
              members.

  - **Functions**: Regular functions, static class members, and class member
                   functions

  - **Namespaces**: A namespace is simply a table containing registrations of
                    functions, data, properties, and other namespaces.

  - **Properties**: Global properties, static class properties, and class member
                    properties. These appear like data to Lua, but are
                    implemented using get and set functions on the C++ side.

  Both data and properties can be marked as _read-only_ at the time of
  registration. This is different from `const`; the values of these objects can
  be modified on the C++ side, but Lua scripts cannot change them. For brevity
  of exposition, in the examples that follow the C++ code samples assume that a
  `using namespace luabridge` declaration is in effect. A code sample is either
  C++ or Lua, depending on the context.

  ### Namespaces

  All LuaBridge registrations take place in a _namespace_, which loosely
  resembles a C++ namespace. When we refer to a _namespace_ we are always
  talking about a namespace in the Lua sense, which is implemented using tables.
  The namespace does not need to correspond to a C++ namespace; in fact no C++
  namespaces need to exist at all unless you want them to. LuaBridge namespaces
  are visible only to Lua scripts; they are used as a logical grouping tool.
  To obtain access to the global namespace for a `lua_State* L` we use:

      getGlobalNamespace (L);

  This returns an object on which further registrations can be performed.
  The subsequent registrations will go into the global namespace, a practice
  which is not recommended. Instead, we can add a single global namespace like
  this:

      getGlobalNamespace (L)
        .beginNamespace ("test");

  This creates a table in `_G` (the global namespace in Lua) called "test".
  Since we have not performed any registrations, this table will be mostly
  empty, except for some necessary bookkeeping fields. LuaBridge reserves all
  identifiers that start with a double underscore. So `__test` would be an
  invalid name (although LuaBridge will silently accept it). Functions like
  `beginNamespace` return the corresponding object on which we can make more
  registrations. Given:

      getGlobalNamespace (L)
        .beginNamespace ("test")
          .beginNamespace ("detail")
          .endNamespace ()
          .beginNamespace ("utility")
          .endNamespace ()
        .endNamespace ();

  The following nested tables are produced: `_G["test"]`, `test["detail"]`, and
  `test["utility"]`. The results are accessible to Lua as `test.detail` and
  `test.utility`. We also used the `endNamespace` function; it returns an object
  representing the original enclosing namespace. It is undefined behavior to
  use `endNamespace` on the global namespace. All LuaBridge functions which 
  create registrations return an object upon which subsequent registrations can
  be made, allowing for an unlimited number of registrations to be chained
  together using the dot operator `.`.

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

  Adding two objects with the same name, in the same namespace, results in
  undefined behavior (although LuaBridge will silently accept it).

  ### Data, Properties, and Functions

  Data, properties, and functions are registered into a namespace using
  `addVariable`, `addProperty`, and `addFunction`. When registered functions
  are called by scripts, LuaBridge automatically takes care of the conversion of
  arguments into the appropriate data type when doing so is possible. This
  automated system works for the function's return value, and up to 8 parameters
  although more can be added by extending the templates. Pointers, references,
  and objects of class type as parameters are treated specially, and explained
  in a later section. Given the following:

      int globalVar;
      static float staticVar;

      std::string stringProperty;
      std::string getString () { return stringProperty; }
      void setString (std::string s) { return s; }

      int foo () { return 42; }
      void bar (char const*) { }

  The will register everything into the namespace "test":

      getGlobalNamespace (L)
        .beginNamespace ("test")
          .addVariable ("var1", &globalVar)
          .addVariable ("var2", &staticVar, false)     // read-only
          .addProperty ("prop1", getString, setString)
          .addProperty ("prop2", getString)            // read only
          .addFunction ("foo", foo)
          .addFunction ("bar", bar)
        .endNamespace ();

  Variables can be marked _read-only_ by passing `false` in the second optional
  parameter. If the parameter is omitted, `true` is used making the variable
  read/write. Properties are marked read-only by omitting the set function or
  passing it as 0 (or `nullptr`). After the registrations above, the following
  Lua identifiers are valid:

      test        -- a namespace
      test.var1   -- a lua_Number variable
      test.var2   -- a read-only lua_Number variable
      test.prop1  -- a lua_String property
      test.prop2  -- a read-only lua_String property
      test.foo    -- a function returning a lua_Number
      test.bar    -- a function taking a lua_String as a parameter

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

  ### Classes

  A class registration is opened using either `beginClass` or `deriveClass` and
  ended using `endClass`. Once registered, a class can later be re-opened for
  more registrations using `beginClass`. However, `deriveClass` should only be
  used once. To add more registrations to an already registered derived class,
  use `beginClass`. We use the word _method_ as an unambiguous synonym for
  _member function_ - static or otherwise. Given:

      struct A {
        static int staticData;
        static float staticProperty;
        
        static float getStaticProperty () { return staticProperty; }
        static void setStaticProperty (float f) { staticProperty = f; }
        static void staticFunc () { }

        std::string dataMember;

        char dataProperty;
        char getProperty () const { return dataProperty; }
        void setProperty (char v) { dataProperty = v; }

        void func1 () { }
        virtual void virtualFunc () { }
      };

      struct B : public A {
        double dataMember2;

        void func1 () { }
        void func2 () { }
        void virtualFunc () { }
      };

      int A::staticData;
      float A::staticProperty;

  The statement to register everything is:

      getGlobalNamespace (L)
        .beginNamespace ("test")
          .beginClass <A> ("A")
            .addStaticData ("staticData", &A::staticData)
            .addStaticProperty ("staticProperty", &A::staticProperty)
            .addStaticMethod ("staticFunc", &A::staticFunc)
            .addData ("data", &A::dataMember)
            .addProperty ("prop", &A::getProperty, &A::setProperty)
            .addMethod ("func1", &A::func1)
            .addMethod ("virtualFunc", &A::virtualFunc)
          .endClass ()
          .deriveClass <B, A> ("B")
            .addData ("data", &B::dataMember2)
            .addMethod ("func1", &B::func1)
            .addMethod ("func2", &B::func2)
          .endClass ()
        .endClass ();

  As with regular variables and properties, class data and properties can
  be marked read-only by passing false in the second parameter, or omitting
  the set function respectively. The `deriveClass` takes two template arguments:
  the class to be registered, and its base class.  Inherited methods do not have
  to be re-declared and will function normally in Lua. If a class has a base
  class that is **not** registered with Lua, there is no need to declare it as a
  subclass.

  Given two Lua objects `a` and `b` of class types `A` and `B respectively, the
  following effects are observed in Lua scripts (the mechanism for passing or
  creating these objects is explained a later section):

      print (test.A.staticData)       -- prints the static data member
      print (test.A.staticProperty)   -- prints the static property member
      test.A.staticFunc ()            -- calls the static method

      print (a.data)                  -- prints the data member
      print (a.prop)                  -- prints the property member
      a:func1 ()                      -- calls A::func1 ()
      test.A.func1 (a)                -- equivalent to a:func1 ()
      test.A.func1 ("hello")          -- error: "hello" is not a class A
      a:virtualFunc ()                -- calls A::virtualFunc ()

      print (b.data)                  -- prints B::dataMember
      print (b.prop)                  -- prints inherited property member
      b:func1 ()                      -- calls B::func1 ()
      b:func2 ()                      -- calls B::func2 ()
      test.B.func2 (a)                -- error: a is not a class B
      test.A.func1 (b)                -- calls A::func1 ()
      b:virtualFunc ()                -- calls B::virtualFunc ()
      test.B.virtualFunc (b)          -- calls B::virtualFunc ()
      test.A.virtualFunc (b)          -- calls B::virtualFunc ()
      test.B.virtualFunc (a)          -- error: a is not a class B

  ## The Stack

  In the Lua C API, all manipulation of the `lua_State` is performed through
  the Lua stack. In order to pass parameters back and forth between C++ and
  Lua, including the return value of functions, LuaBridge uses a class template
  specialized for each possible C++ data type.

  ### Data Types

  Basic types for supported variables, and function arguments and returns, are:
  
  - `bool`
  - `char`, converted to a string of length one.
  - Integers, `float`, and `double`, converted to Lua_number.
  - Strings: `char const*` and `std::string`

  Of course, LuaBridge supports passing objects of class type, in a variety of
  ways including dynamically allocated objects created with `new`. The behavior
  of the object with respect to lifetime management depends on the manner in
  which the object is passed. Given `class T`, these argument types are
  supported:

  - `T`, `T const` : Pass `T` by value. The lifetime is managed by Lua.
  - `T*`, `T&`, `T const*`, `T const&` : Pass `T` by reference. The lifetime
     is managed by C++.

  Furthermore, LuaBridge supports a "shared lifetime" model: dynamically
  allocated and reference counted objects whose ownership is shared by both
  Lua and C++. The object remains in existence until there are no remaining
  C++ or Lua references, and Lua performs its usual garbage collection cycle.
  LuaBridge comes with a few varieties of containers that support this
  shared lifetime model, or you can use your own (subject to some restrictions).

  Mixing object lifetime models is entirely possible, subject to the usual
  caveats of holding references to objects which could get deleted. For
  example, C++ can be called from Lua with a pointer to an object of class
  type; the function can modify the object or call non-const data members.
  These modifications are visible to Lua (since they both refer to the same
  object).

  ### Pointers, References, and Object Parameters

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

  In the example above, All functions can read the data members and property
  members of `a`, or call const member functions of `a`. Only `func0`, `func1`
  and `func3` can modify the data members and data properties, or call
  non-const member functions of `a`.

  Except for func0(), a problem can arise when C++ receives pointers or
  references to objects seen by Lua. Undefined behavior can result if the Lua
  garbage collector destroys the class while the C++ code is keeping a pointer
  to the object somewhere. In the body of the function above, access is safe
  because there is always a reference to the object on the Lua stack. To
  prevent undefined behavior, do not store pointers to objects that come from
  C++ functions called by Lua. Instead, use a _Container_. This is explained in
  a later section.

  The usual C++ inheritance and pointer assignment rules apply. Given:

      void func5 (B b);
      void func6 (B* b);

  The following Lua effects are observed:

      func5 (b)   - Passes a copy of b, using B's copy constructor.
      func6 (b)   - Passes a pointer to b.
      func6 (a)   - Error: Pointer to B expected.
      func1 (b)   - Okay, b is a subclass of a.

  When Lua manages the lifetime of the object, it is subjected to all of the
  normal garbage collection rules. C++ functions and member functions can
  receive pointers and references to these objects, but care must be taken
  to make sure that no attempt is made to access the object after it is
  garbage collected. Usually this is done by simply not storing a pointer to
  the object somewhere inside your C++.

  When C++ manages the lifetime of the object, the Lua garbage collector has
  no effect on it. Care must be taken to make sure that the C++ code does not
  destroy the object while Lua still has a reference to it.

  ### Lua Stack

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

  ## Security

  The metatables and userdata that LuaBridge creates in the `lua_State` are
  protected using a security system, to eliminate the possibility of undefined
  behavior resulting from scripted manipulation of the environment. This
  security system can be bypassed if scripts are given access to the debug
  library.

  When a class member function is called, or class property member accessed,
  the `this` pointer is type-checked. This is because member functions exposed
  to Lua are just plain functions that usually get called with the Lua colon
  notation, which passes the object in question as the first parameter.

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

  ## Development

  [Github][3] is the new official home for LuaBridge. The old SVN repository is
  deprecated since it is no longer used, or maintained. The original author has
  graciously passed the reins to Vinnie Falco for maintaining and improving the
  project. To obtain the older official releases, checkout the tags from 2.1
  and earlier.

  We welcome contributions to LuaBridge. Feel free to fork the repository and
  issue a Pull Request. All questions, comments, suggestions, and/or proposed
  changes will be handled by the new maintainer.

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
*/

/**
  ### Shared Pointers

  A `C` container template allows for object lifetime management that
  behaves like `std::shared_ptr`. That is, objects are dynamically allocated
  and reference counted. The object is not destroyed until the reference count
  drops to zero. Such objects are safe to store in C++ and Lua code. A
  garbage collection will only decrement the reference count.

  LuaBridge registraton templates will automatically detect arguments which
  behave like containers. For example, this registration is valid:

      extern void func (shared_ptr <T> p);

      s.function ("func", &func);

  Any container may be used. LuaBridge expects that the container in question
  is a class template with one template argument, and a member function called
  get() which returns a pointer to the underlying object. If you need to use
  a container with a different interface, you can specialize the `C`
  class for your container type and provide an extraction function. Your
  specialization needs to be in the `luabridge` namespace. Here's an example
  specialization for a container called `ReferenceCountedObject` which provides
  a member function `getObject` that retrieves the pointer.

      namespace luabridge
      {
          template <>
          struct C <ReferenceCountedObjectPtr>
          {
            template <class T>
            static T* get (ReferenceCountedObjectPtr <T> const& p)
            {
              return p.getObject ();
            }
         };
      }

  ### Registering classes

  C++ classes can be registered with Lua as follows:

      s .class_ <MyClass> ("MyClass")
        .Constructor <void (*) (void)> ()
        .method ("method1", &MyClass::method1)
        .method ("method2", &MyClass::method2);

      s .subclass <MySubclass, MyBaseClass> ("MySubclass")
        .Constructor <...>
        ...

  The `class_` function registers a class; its Constructor will be available as
  a global function with name given as argument to `class_`.  The object
  returned can then be used to register the Constructor (no overloading is
  supported, so there can only be one Constructor) and methods.

  LuaBridge cannot automatically determine the number and types of Constructor
  parameters like it can for functions and methods, so you must provide them.
  This is done by letting the `Constructor` function take a template parameter,
  which must be a function pointer type.  The parameter types will be extracted
  from this (the return type is ignored).  For example, to register a
  Constructor taking two parameters, one `int` and one `char const*`, you would
  write:

      s .class_ <MyClass> ()
        .Constructor <void (*) (int, const char *)> ()

  Note that in the example above, the name of the class was ommitted from the
  argument to the `class` function. This allows you to add additional
  registrations to a class that has already been registered.

  Method registration works just like function registration.  Virtual methods
  work normally; no special syntax is needed. const methods are detected and
  const-correctness is enforced, so if a function returns a const object (or
  rather, a `shared_ptr` to a const object) to Lua, that reference to the object
  will be considered const and only const methods will be called on it.
  Destructors are registered automatically for each class.

  Static methods may be registered using the `static_method` function, which is
  simply an alias for the `function` function:

      s .class_ <MyClass> ()
        .static_method ("method3", &MyClass::method3)

  ### The `lua_State`

  Multiple lua_State registrations

  Sometimes it is convenient from within a bound function or member function
  to gain access to the `lua_State` normally available to a `lua_CFunction`.
  With LuaBridge, all you need to do is add a `lua_State*` parameter at any
  position in your bound function:

      void useState (lua_State* L);

      s.function ("useState", &useState);

  You can still include regular arguments while receiving the state:

      void useStateAndArgs (lua_State* L, int i, std::string s);

      s.function ("useStateAndArgs", &useStateAndArgs);


  ## Implementation Details

  - Nested tables

  - Security system

  - The registry

  - Multiple lua_state
*/

#include <cassert>
#include <string>

namespace luabridge
{

#if defined (__APPLE_CPP__) || defined(__APPLE_CC__)
// Do not define THROWSPEC since the Xcode compilers do not distinguish
// the throw specification in the function signature.
#else
#define THROWSPEC throw()
#endif
#include "TypeTraits.h"

//==============================================================================
/**
  Container traits.

  Unspecialized Container sets isContainer to false for enable_if. All user
  defined containers must supply an appropriate specialization for Container.
  The containers that come with LuaBridge also come with specializations for
  their types, see the corresponding container declarations for details.
*/
template <class T>
struct ContainerTraits
{
  typedef bool isNotContainer;
};

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

// Forward declaration required.
template <class T>
struct Stack;

//------------------------------------------------------------------------------

struct Detail
{
//protected:
  //----------------------------------------------------------------------------
  /**
    Return the identity pointer for our lightuserdata tokens.

    LuaBridge metatables are tagged with a security "token." The token is a
    lightuserdata created from the identity pointer, used as a key in the
    metatable. The value is a boolean = true, although any value could have been
    used.

    Because of Lua's weak typing and our improvised system of imposing C++
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
  static inline void* const getIdentityKey ()
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
    static void const* const  getStaticKey ()
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
    static void const* const getClassKey ()
    {
      static char value;
      return &value;
    }

    /**
      Get the key for the const table.

      The const table holds read-only data members and properties, and const
      member functions of a class.
    */
    static void const* const getConstKey ()
    {
      static char value;
      return &value;
    }
  };

  //============================================================================
  /**
    Determine if type T is a container.

    To be considered a container, there must be a specialization of
    ContainerTraits with the required fields.
  */
  template <class T>
  struct isContainer
  {
    // Types "yes" and "no" are guaranteed to have different sizes,
    // specifically sizeof(yes) == 1 and sizeof(no) == 2.
    typedef char yes[1];
    typedef char no [2];

    template <typename C>
    static yes& test (typename C::isNotContainer*);
 
    template <typename>
    static no& test (...);
 
    // If the "sizeof" the result of calling test<T>(0) would be equal to the sizeof(yes),
    // the first overload worked and T has a nested type named foobar.
    static const bool value = sizeof (test <ContainerTraits <T> >(0)) == sizeof (no);
  };

  //----------------------------------------------------------------------------
  /**
    Strip the const qualifier from T.
  */
  template <class T>
  struct withoutConst
  {
    typedef typename T Type;
  };

  template <class T>
  struct withoutConst <T const>
  {
    typedef typename T Type;
  };

  //----------------------------------------------------------------------------
  /**
    Determine if T is const qualified.
  */
  template <class T>
  struct isConstQualified
  {
    static bool const value = false;
  };

  template <class T>
  struct isConstQualified <T const>
  {
    static bool const value = true;
  };

  //============================================================================
  /**
    Class wrapped in a Lua userdata.
  */
  class Userdata
  {
  private:
    //--------------------------------------------------------------------------
    /**
      Validate and retrieve a Userdata on the stack.

      The Userdata must exactly match the corresponding class table or
      const table, or else a Lua error is raised. This is used for the
      __gc metamethod.
    */
    static Userdata* const getExactClass (lua_State* L, int narg, void const* const classKey)
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
    static Userdata* const getClass (
      lua_State* L, int narg, void const* const baseClassKey, bool const canBeConst)
    {
      Userdata* ud = 0;
      int const index = lua_absindex (L, narg);

      bool mismatch = false;
      char const* got = 0;

      lua_rawgetp (L, LUA_REGISTRYINDEX, baseClassKey);
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
              if (narg > 0)
                luaL_argerror (L, narg, "cannot be const");
              else
                luaL_error (L, "cannot be const");
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
      Get an untyped pointer to the contained class.
    */
    virtual void* getPointer () = 0;

  public:
    virtual ~Userdata () { }

    //--------------------------------------------------------------------------
    /**
      Returns the Userdata* if the class on the Lua stack matches.

      If the class does not match, a Lua error is raised.
    */
    template <class T>
    static Userdata* getExact (lua_State* L, int index)
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
    static T* get (lua_State* L, int index, bool canBeConst)
    {
      Userdata* const ud = getClass (L, index, ClassInfo <T>::getClassKey (), canBeConst);
      return static_cast <T*> (ud->getPointer ());
    }
  };

  //----------------------------------------------------------------------------
  /**
    Wraps a container thet references a class object.

    The template argument C is the container type, for which a specialization
    in ContainerTraits must exist.
  */
  template <class C>
  class UserdataShared : public Userdata
  {
  private:
    UserdataShared (UserdataShared <C> const&);
    UserdataShared <C>& operator= (UserdataShared <C> const&);

    typedef typename withoutConst <typename ContainerTraits <C>::Type>::Type T;

    C m_c;

  private:
    ~UserdataShared ()
    {
    }

    void* getPointer ()
    {
      return const_cast <void*> (reinterpret_cast <void const*> (
        (ContainerTraits <C>::get (m_c))));
    }

    template <bool makeObjectConst>
    struct Helper
    {
      static void push (lua_State* L, C const& c)
      {
        new (lua_newuserdata (L, sizeof (UserdataShared <C>))) UserdataShared <C> (c);
        lua_rawgetp (L, LUA_REGISTRYINDEX, ClassInfo <T>::getClassKey ());
        // If this goes off it means the class T is unregistered!
        assert (lua_istable (L, -1));
        lua_setmetatable (L, -2);
      }

      static void push (lua_State* L, T* const t)
      {
        new (lua_newuserdata (L, sizeof (UserdataShared <C>))) UserdataShared <C> (t);
        lua_rawgetp (L, LUA_REGISTRYINDEX, ClassInfo <T>::getClassKey ());
        // If this goes off it means the class T is unregistered!
        assert (lua_istable (L, -1));
        lua_setmetatable (L, -2);
      }
    };

    template <>
    struct Helper <true>
    {
      static void push (lua_State* L, C const& c)
      {
        new (lua_newuserdata (L, sizeof (UserdataShared <C>))) UserdataShared <C> (c);
        lua_rawgetp (L, LUA_REGISTRYINDEX, ClassInfo <T>::getConstKey ());
        // If this goes off it means the class T is unregistered!
        assert (lua_istable (L, -1));
        lua_setmetatable (L, -2);
      }

      static void push (lua_State* L, T* const t)
      {
        new (lua_newuserdata (L, sizeof (UserdataShared <C>))) UserdataShared <C> (t);
        lua_rawgetp (L, LUA_REGISTRYINDEX, ClassInfo <T>::getConstKey ());
        // If this goes off it means the class T is unregistered!
        assert (lua_istable (L, -1));
        lua_setmetatable (L, -2);
      }
    };

  public:
    /**
      Construct from a container to the class or a derived class.
    */
    template <class U>
    explicit UserdataShared (U const& u) : m_c (u)
    {
    }

    /**
      Construct from a pointer to the class or a derived class.
    */
    template <class U>
    explicit UserdataShared (U* u) : m_c (u)
    {
    }

    static void push (lua_State* L, C const& c)
    {
      Helper <isConstQualified <typename ContainerTraits <C>::Type>::value>::push (L, c);
    }

    static void push (lua_State* L, T* const t)
    {
      Helper <isConstQualified <typename ContainerTraits <C>::Type>::value>::push (L, t);
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

  private:
    ~UserdataValue ()
    {
      getObject ()->~T ();
    }

    void* getPointer ()
    {
      return &m_storage [0];
    }

    T* getObject ()
    {
      // If this fails to compile it means you forgot to provide
      // a Container specialization for your container!
      //
      return static_cast <T*> (&m_storage [0]);
    }

  public:
    /**
      Push a T via placement new.

      The caller is responsible for calling placement new using the
      returned uninitialized storage.
    */
    static void* place (lua_State* const L)
    {
      Userdata* const ud = new (
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
      push (place (L)) T (u);
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

    void* const m_p;

  private:
    void* getPointer ()
    {
      return m_p;
    }

    /** Push non-const pointer to object using metatable key.
    */
    static void push (lua_State* L, void* const p, void const* const key)
    {
      new (lua_newuserdata (L, sizeof (UserdataPtr))) UserdataPtr (p);
      lua_rawgetp (L, LUA_REGISTRYINDEX, key);
      // If this goes off it means you forgot to register the class!
      assert (lua_istable (L, -1));
      lua_setmetatable (L, -2);
    }

    /** Push const pointer to object using metatable key.
    */
    static void push (lua_State* L, void const* const p, void const* const key)
    {
      new (lua_newuserdata (L, sizeof (UserdataPtr)))
        UserdataPtr (const_cast <void*> (p));
      lua_rawgetp (L, LUA_REGISTRYINDEX, key);
      // If this goes off it means you forgot to register the class!
      assert (lua_istable (L, -1));
      lua_setmetatable (L, -2);
    }

    explicit UserdataPtr (void* const p) : m_p (p)
    {
    }

  public:
    /** Push non-const pointer to object.
    */
    template <class T>
    static inline void push (lua_State* const L, T* const p)
    {
      push (L, p, ClassInfo <T>::getClassKey ());
    }

    /** Push const pointer to object.
    */
    template <class T>
    static inline void push (lua_State* const L, T const* const p)
    {
      push (L, p, ClassInfo <T>::getConstKey ());
    }
  };
};

//==============================================================================
/**
  Lua stack conversions for class objects passed by value.
*/
template <class T>
struct Stack : Detail
{
private:
  /**
    Pass by pointer wrapped in a container.

    The container controls the object lifetime. Typically this will be a
    lifetime shared by C++ and Lua using a reference count. Because of type
    erasure, containers like std::shared_ptr will not work. Containers must
    either be of the intrusive variety, or in the style of the RefCountedPtr
    type provided by LuaBridge (that uses a global hash table).
  */
  template <bool byContainer>
  struct Helper
  {
    static inline void push (lua_State* L, T const& t)
    {
      UserdataShared <T>::push (L, t);
    }

    static inline T get (lua_State* L, int index)
    {
      typedef typename ContainerTraits <T>::Type U;
      return Userdata::get <U> (L, index, true);
    }
  };

  /**
    Pass by value.

    Lifetime is managed by Lua. A C++ function which accesses a pointer or
    reference to an object outside the activation record in which it was
    retrieved may result in undefined behavior if Lua garbage collected it.
  */
  template <>
  struct Helper <false>
  {
    static inline void push (lua_State* L, T const& t)
    {
      UserdataValue <T>::push (L, t);
    }

    static inline T const& get (lua_State* L, int index)
    {
      return *Userdata::get <T> (L, index, true);
    }
  };

public:
  static inline void push (lua_State* L, T const& t)
  {
    Helper <isContainer <T>::value>::push (L, t);
  }

  static inline T get (lua_State* L, int index)
  {
    return Helper <isContainer <T>::value>::get (L, index);
  }
};

//------------------------------------------------------------------------------
/**
  Lua stack conversions for pointers and references to class objects.

  Lifetime is managed by C++. Lua code which remembers a reference to the
  value may result in undefined behavior if C++ destroys the object. The
  handling of the const and volatile qualifiers happens in UserdataPtr.
*/
template <class T>
struct Stack <T*> : Detail
{
  static inline void push (lua_State* L, T* const p)
  {
    UserdataPtr::push (L, p);
  }

  static inline T* const get (lua_State* L, int index)
  {
    return Userdata::get <T> (L, index, false);
  }
};

template <class T>
struct Stack <T const*> : Detail
{
  static inline void push (lua_State* L, T const* const p)
  {
    UserdataPtr::push (L, p);
  }

  static inline T const* const get (lua_State* L, int index)
  {
    return Userdata::get <T> (L, index, true);
  }
};

template <class T>
struct Stack <T&> : Detail
{
  static inline void push (lua_State* L, T& t)
  {
    UserdataPtr::push (L, &t);
  }

  static T& get (lua_State* L, int index)
  {
    return *Userdata::get <T> (L, index, false);
  }
};

template <class T>
struct Stack <T const&> : Detail
{
  static inline void push (lua_State* L, T const& t)
  {
    UserdataPtr::push (L, &t);
  }

  static T const& get (lua_State* L, int index)
  {
    return *Userdata::get <T> (L, index, true);
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
    lua_pushstring (L, str);
  }

  static inline char const* get (lua_State* L, int index)
  {
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
struct ArgList <nil, Start> : public TypeListValues <nil>
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

//==============================================================================
/**
  Push objects onto the Lua stack.
*/

template <class T>
inline void push (lua_State* L, T t)
{
  Stack <T>::push (L, t);
}

//==============================================================================

/**
  Provides a namespace registration in a lua_State.
*/
class Namespace : private Detail
{
private:
  Namespace& operator= (Namespace const& other);

  lua_State* const L;
  int mutable m_stackSize;

private:
  //----------------------------------------------------------------------------
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
    lua_CFunction to report an error writing to a read-only value.

    The name of the variable is in the first upvalue.
  */
  static int readOnlyError (lua_State* L)
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

    s = s + "'" + lua_tostring (L, lua_upvalueindex (1))
          + "' is read-only";

    return luaL_error (L, s.c_str ());
  }
  
  //----------------------------------------------------------------------------
  /**
    lua_CFunction to get a variable.

    This is used for global variables or class static data members.
  */
  template <class T>
  static int vargetProxy (lua_State* L)
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
  static int varsetProxy (lua_State* L)
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
  struct functionProxy
  {
    typedef typename FuncTraits <Func>::Params Params;
    static int f (lua_State* L)
    {
      assert (lua_isuserdata (L, lua_upvalueindex (1)));
      Func fp = *static_cast <Func*> (lua_touserdata (L, lua_upvalueindex (1)));
      assert (fp != 0);
      ArgList <Params> args (L);
      Stack <ReturnType>::push (L, FuncTraits <Func>::call (fp, args));
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
  struct functionProxy <Func, void>
  {
    typedef typename FuncTraits <Func>::Params Params;
    static int f (lua_State* L)
    {
      assert (lua_isuserdata (L, lua_upvalueindex (1)));
      Func fp = *static_cast <Func*> (lua_touserdata (L, lua_upvalueindex (1)));
      assert (fp != 0);
      ArgList <Params> args (L);
      FuncTraits <Func>::call (fp, args);
      return 0;
    }
  };

  //============================================================================
  /**
    lua_CFunction to call a class member function with a return value.

    The argument list contains the 'this' pointer followed by the method
    arguments.

    @note The expected class name is in upvalue 1, and the member function
          pointer is in upvalue 2.
  */
  template <class MemFn,
            class ReturnType = typename FuncTraits <MemFn>::ReturnType>
  struct methodProxy
  {
    //typedef typename Container <typename FuncTraits <MemFn>::ClassType>::Type T;
    typedef typename FuncTraits <MemFn>::ClassType T;
    typedef typename FuncTraits <MemFn>::Params Params;

    static int callMethod (lua_State* L)
    {
      assert (lua_isuserdata (L, lua_upvalueindex (1)));
      T* const t = Userdata::get <T> (L, 1, false);
      MemFn fp = *static_cast <MemFn*> (lua_touserdata (L, lua_upvalueindex (1)));
      ArgList <Params, 2> args (L);
      Stack <ReturnType>::push (L, FuncTraits <MemFn>::call (t, fp, args));
      return 1;
    }

    static int callConstMethod (lua_State* L)
    {
      assert (lua_isuserdata (L, lua_upvalueindex (1)));
      T const* const t = Userdata::get <T> (L, 1, true);
      MemFn fp = *static_cast <MemFn*> (lua_touserdata (L, lua_upvalueindex (1)));
      ArgList <Params, 2> args(L);
      Stack <ReturnType>::push (L, FuncTraits <MemFn>::call (t, fp, args));
      return 1;
    }
  };

  //----------------------------------------------------------------------------
  /**
    lua_CFunction to call a class member function with no return value.

    The argument list contains the 'this' pointer followed by the method
    arguments.

    @note The expected class name is in upvalue 1, and the member function
          pointer is in upvalue 2.
  */
  template <class MemFn>
  struct methodProxy <MemFn, void>
  {
    //typedef typename Container <typename FuncTraits <MemFn>::ClassType>::Type T;
    typedef typename FuncTraits <MemFn>::ClassType T;
    typedef typename FuncTraits <MemFn>::Params Params;

    static int callMethod (lua_State* L)
    {
      T* const t = Userdata::get <T> (L, 1, false);
      MemFn const fp = *static_cast <MemFn*> (lua_touserdata (L, lua_upvalueindex (1)));
      ArgList <Params, 2> args (L);
      FuncTraits <MemFn>::call (t, fp, args);
      return 0;
    }

    static int callConstMethod (lua_State* L)
    {
      T const* const t = Userdata::get <T> (L, 1, true);
      MemFn const fp = *static_cast <MemFn*> (lua_touserdata (L, lua_upvalueindex (1)));
      ArgList <Params, 2> args (L);
      FuncTraits <MemFn>::call (t, fp, args);
      return 0;
    }
  };

  //----------------------------------------------------------------------------
  /**
    Template to add class member functions.
  */
  template <class MemFn, bool isConst>
  struct methodHelper;

  /**
    Create a proxy for a const member function.
  */
  template <class MemFn>
  struct methodHelper <MemFn, true>
  {
    static void add (lua_State* L, char const* name, MemFn mf)
    {
      new (lua_newuserdata (L, sizeof (MemFn))) MemFn (mf);
      lua_pushcclosure (L, &methodProxy <MemFn>::callConstMethod, 1);
      lua_pushvalue (L, -1);
      rawsetfield (L, -5, name); // const table
      rawsetfield (L, -3, name); // class table
    }
  };

  /**
    Create a proxy for a non-const member function.
  */
  template <class MemFn>
  struct methodHelper <MemFn, false>
  {
    static void add (lua_State* L, char const* name, MemFn mf)
    {
      new (lua_newuserdata (L, sizeof (MemFn))) MemFn (mf);
      lua_pushcclosure (L, &methodProxy <MemFn>::callMethod, 1);
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

          // We only put cfunctions into the metatable.
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
      lua_rawsetp (L, -2, getIdentityKey ());
      lua_pushstring (L, (std::string ("const ") + name).c_str ());
      rawsetfield (L, -2, "__type");
      lua_pushcfunction (L, &indexMetaMethod);
      rawsetfield (L, -2, "__index");
      lua_pushcfunction (L, &newindexMetaMethod);
      rawsetfield (L, -2, "__newindex");
      lua_newtable (L);
      rawsetfield (L, -2, "__propget");
      lua_pushboolean (L, 0);
      rawsetfield (L, -2, "__metatable");
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
      lua_rawsetp (L, -2, getIdentityKey ());
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
      lua_pushboolean (L, 0);
      rawsetfield (L, -2, "__metatable");
      lua_pushvalue (L, -2);
      rawsetfield (L, -2, "__const"); // point to const table

      lua_pushvalue (L, -1);
      rawsetfield (L, -3, "__class"); // point const table to class table
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
      lua_pushboolean (L, 0);
      rawsetfield (L, -2, "__metatable");
      lua_pushvalue (L, -2);
      rawsetfield (L, -2, "__class"); // point to class table
    }

    //==========================================================================
    /**
      lua_CFunction to construct a class object.
    */
    template <class Params, class C>
    static int ctorProxy (lua_State* L)
    {
      typedef typename ContainerTraits <C>::Type T;
      ArgList <Params, 2> args (L);
      T* const p = Constructor <T, Params>::call (args);
      UserdataShared <C>::push (L, p);
      //new (UserdataType <C>::push (L, false)) UserdataType <C> (p);
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
      lua_CFunction to get a class data member.
    */
    template <typename U>
    static int propgetProxy (lua_State* L)
    {
      T const* const t = Userdata::get <T> (L, 1, true);
      U T::* mp = *static_cast <U T::**> (lua_touserdata (L, lua_upvalueindex (1)));
      Stack <U>::push (L, t->*mp);
      return 1;
    }

    //--------------------------------------------------------------------------
    /**
      lua_CFunction to set a class data member.

      @note The expected class name is in upvalue 1, and the pointer to the
            data member is in upvalue 2.
    */
    template <typename U>
    static int propsetProxy (lua_State* L)
    {
      T* const t = Userdata::get <T> (L, 1, false);
      U T::* mp = *static_cast <U T::**> (lua_touserdata (L, lua_upvalueindex (1)));
      t->*mp = Stack <U>::get (L, 2);
      return 0;
    }

    //--------------------------------------------------------------------------
    /**
      __gc metamethod for a class.
    */
    static int gcMetaMethod (lua_State* L)
    {
      Userdata::getExact <T> (L, 1)->~Userdata ();
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
        lua_rawsetp (L, LUA_REGISTRYINDEX, ClassInfo <T>::getStaticKey ());
        lua_pushvalue (L, -2);
        lua_rawsetp (L, LUA_REGISTRYINDEX, ClassInfo <T>::getClassKey ());
        lua_pushvalue (L, -3);
        lua_rawsetp (L, LUA_REGISTRYINDEX, ClassInfo <T>::getConstKey ());
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
      lua_rawsetp (L, LUA_REGISTRYINDEX, ClassInfo <T>::getStaticKey ());
      lua_pushvalue (L, -2);
      lua_rawsetp (L, LUA_REGISTRYINDEX, ClassInfo <T>::getClassKey ());
      lua_pushvalue (L, -3);
      lua_rawsetp (L, LUA_REGISTRYINDEX, ClassInfo <T>::getConstKey ());
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
      lua_pushcclosure (L, &vargetProxy <U>, 1);
      rawsetfield (L, -2, name);
      lua_pop (L, 1);

      rawgetfield (L, -1, "__propset");
      assert (lua_istable (L, -1));
      if (isWritable)
      {
        lua_pushlightuserdata (L, pu);
        lua_pushcclosure (L, &varsetProxy <U>, 1);
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
      lua_pushcclosure (L, &functionProxy <U (*) (void)>::f, 1);
      rawsetfield (L, -2, name);
      lua_pop (L, 1);

      rawgetfield (L, -1, "__propset");
      assert (lua_istable (L, -1));
      if (set != 0)
      {
        new (lua_newuserdata (L, sizeof (set))) set_t (set);
        lua_pushcclosure (L, &functionProxy <void (*) (U)>::f, 1);
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
    Class <T>& addStaticMethod (char const* name, FP const fp)
    {
      new (lua_newuserdata (L, sizeof (fp))) FP (fp);
      lua_pushcclosure (L, &functionProxy <FP>::f, 1);
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
      // Add to __propget in class and const tables.
      rawgetfield (L, -2, "__propget");
      rawgetfield (L, -4, "__propget");
      void* const v = lua_newuserdata (L, sizeof (U T::*));
      memcpy (v, &mp, sizeof (U T::*));
      lua_pushcclosure (L, &propgetProxy <U>, 1);
      lua_pushvalue (L, -1);
      rawsetfield (L, -4, name);
      rawsetfield (L, -2, name);
      lua_pop (L, 2);

      if (isWritable)
      {
        // Add to __propset in class table.
        rawgetfield (L, -2, "__propset");
        assert (lua_istable (L, -1));
        void* const v = lua_newuserdata (L, sizeof (U T::*));
        memcpy (v, &mp, sizeof (U T::*));
        lua_pushcclosure (L, &propsetProxy <U>, 1);
        rawsetfield (L, -2, name);
        lua_pop (L, 1);
      }

      return *this;
    }

    //--------------------------------------------------------------------------
    /**
      Add or replace a property member.

      If the set function is null, the property is read-only.
    */
    template <class U>
    Class <T>& addProperty (char const* name, U (T::* get) () const, void (T::* set) (U) = 0)
    {
      // Add to __propget in class and const tables.
      {
        rawgetfield (L, -2, "__propget");
        rawgetfield (L, -4, "__propget");
        typedef U (T::*get_t) () const;
        new (lua_newuserdata (L, sizeof (get_t))) get_t (get);
        lua_pushcclosure (L, &methodProxy <get_t>::callConstMethod, 1);
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
        typedef void (T::* set_t) (U);
        new (lua_newuserdata (L, sizeof (set_t))) set_t (set);
        lua_pushcclosure (L, &methodProxy <set_t>::callMethod, 1);
        rawsetfield (L, -2, name);
        lua_pop (L, 1);
      }

      return *this;
    }

    //--------------------------------------------------------------------------
    /**
      Add or replace a member function.
    */
    template <class MemFn>
    Class <T>& addMethod (char const* name, MemFn mf)
    {
      methodHelper <MemFn, FuncTraits <MemFn>::isConstMemberFunction>::add (L, name, mf);
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
      lua_pushcclosure (L, &ctorProxy <typename FuncTraits <MemFn>::Params, C>, 0);
      rawsetfield(L, -2, "__call");

      return *this;
    }
  };

protected:
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
    Creates a continued registration from a child namespace.
  */
  explicit Namespace (Namespace const* child)
    : L (child->L)
    , m_stackSize (0)
  {
    m_stackSize = child->m_stackSize - 1;
    child->m_stackSize = 1;
    child->pop (1);
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

#if 0
      lua_pushcfunction (L, &tostringMetaMethod);
      rawsetfield (L, -2, "__tostring");
#endif

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
    }
  }

public:
  //----------------------------------------------------------------------------
  /**
    Copy Constructor.

    Ownership of the stack is transferred to the new object. This happens
    when the compiler emits temporaries to hold these objects while chaining
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
    lua_pushcclosure (L, &vargetProxy <T>, 1);
    rawsetfield (L, -2, name);
    lua_pop (L, 1);

    rawgetfield (L, -1, "__propset");
    assert (lua_istable (L, -1));
    if (isWritable)
    {
      lua_pushlightuserdata (L, pt);
      lua_pushcclosure (L, &varsetProxy <T>, 1);
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
  template <class T>
  Namespace& addProperty (char const* name, T (*get) (), void (*set)(T) = 0)
  {
    assert (lua_istable (L, -1));

    rawgetfield (L, -1, "__propget");
    assert (lua_istable (L, -1));
    lua_pushlightuserdata (L, get);
    lua_pushcclosure (L, &functionProxy <T (*) (void)>::f, 1);
    rawsetfield (L, -2, name);
    lua_pop (L, 1);

    rawgetfield (L, -1, "__propset");
    assert (lua_istable (L, -1));
    if (set != 0)
    {
      lua_pushlightuserdata (L, set);
      lua_pushcclosure (L, &functionProxy <void (*) (T)>::f, 1);
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
    lua_pushcclosure (L, &functionProxy <FP>::f, 1);
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
    return Class <T> (name, this, ClassInfo <U>::getStaticKey ());
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

}

//==============================================================================

#endif
