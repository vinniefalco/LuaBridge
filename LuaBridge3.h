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

#ifndef LUABRIDGE_LUABRIDGE3_HEADER
#define LUABRIDGE_LUABRIDGE3_HEADER

#include <typeinfo>
#include <stdint.h>
#include <string.h>

#ifndef LUABRIDGE_USE_APICHECK
#ifdef LUA_USE_APICHECK
#define LUABRIDGE_USE_APICHECK 1
#else
#define LUABRIDGE_USE_APICHECK 0
#endif
#endif

/** This turns on code that enforces const-correctness for member functions
    but I can't get it to compile - Vinnie
*/
#define LUABRIDGE_STRICT_CONST 0

//==============================================================================
/**
  @mainpage LuaBridge: Simple C++ to Lua bindings.

  @details

  # LuaBridge

  [LuaBridge][3] is a lightweight, dependency-free library for binding to C++.
  It works with Lua revisions starting from 5.1.2. [Lua][4] is a powerful, fast,
  lightweight, embeddable scripting language.

  ## Compiling

  LuaBridge is distributed as a single header file "LuaBridge.h" that you simply
  include where you want to register your functions, classes, and variables.
  There are no additional source files, no compilation settings, and no
  Makefiles or IDE-specific project files. LuaBridge is easy to integrate.

  ## Usage

  LuaBridge is based on C++ template metaprogramming.  It contains template code
  to automatically generate at compile-time the various Lua API calls necessary
  to export your program's classes and functions to the Lua environment.

  ### Registering functions

  If `L` is a pointer to an instance of `lua_State`, the following code creates
  a LuaBridge scope for registering C++ functions and classes to `L`:

      luabridge::scope s (L);

  Functions can then be registered as follows:

      s .function ("foo", &foo)
        .function ("bar", &bar);

  The `function` function returns a reference to `s`, so you can chain many
  definitions together.  The first argument is the name by which the function
  will be available in Lua, and the second is the function's address. LuaBridge
  will automatically detect the number (up to 8, by default) and type of the
  parameters.  Functions registered this way will be available at the global
  scope to Lua scripts executed by `L`.  Overloading of function names is not
  supported, nor is it likely to be supported in the future.

  ### Registering data

  Variables can also be registered.  You can expose a 'bare' variable to Lua, or
  wrap it in getter and setter functions:

      s .variable_rw ("var1", &var1)
        .variable_rw ("var2", &getter2, &setter2)
        .variable_ro ("var3", &var3)
        .variable_ro ("var4", &getter4)

  The first registration above gives Lua direct access to the `var1` variable.
  The second creates a variable which appears like any other variable to Lua
  code, but is retrieved and set through the `getter2` and `setter2` function.
  The getter must take no parameters and return a value, and the setter must
  take a value of the same type and return nothing.  The `variable_rw` function
  creates a readable and writeable variable, while `variable_ro` creates a
  read-only one. Obviously, there is no setter for the read-only variable.

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
  - `C <T>`, `C <T const>` : Pass `T` by container. The lifetime
     is managed by the container.

  When Lua manages the lifetime of the object, it is subjected to all of the
  normal garbage collection rules. C++ functions and member functions can
  receive pointers and references to these objects, but care must be taken
  to make sure that no attempt is made to access the object after it is
  garbage collected. Usually this is done by simply not storing a pointer to
  the object somewhere inside your C++.

  When C++ manages the lifetime of the object, the Lua garbage collector has
  no effect on it. Care must be taken to make sure that the C++ code does not
  destroy the object while Lua still has a reference to it.

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
        .constructor <void (*) (void)> ()
        .method ("method1", &MyClass::method1)
        .method ("method2", &MyClass::method2);

      s .subclass <MySubclass, MyBaseClass> ("MySubclass")
        .constructor <...>
        ...

  The `class_` function registers a class; its constructor will be available as
  a global function with name given as argument to `class_`.  The object
  returned can then be used to register the constructor (no overloading is
  supported, so there can only be one constructor) and methods.

  LuaBridge cannot automatically determine the number and types of constructor
  parameters like it can for functions and methods, so you must provide them.
  This is done by letting the `constructor` function take a template parameter,
  which must be a function pointer type.  The parameter types will be extracted
  from this (the return type is ignored).  For example, to register a
  constructor taking two parameters, one `int` and one `char const*`, you would
  write:

      s .class_ <MyClass> ()
        .constructor <void (*) (int, const char *)> ()

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

  LuaBridge also supports properties, which allow class data members to be read
  and written from Lua as if they were variables.  Properties work much like
  variables do, and the syntax for registering them is as follows:

      s .class_ <MyClass> ()
        .property_rw ("property1", &MyClass::property1)
        .property_rw ("property2", &MyClass::getter2, &MyClass::setter2)
        .property_ro ("property3", &MyClass::property3)
        .property_ro ("property4", &MyClass::getter4)

  Static properties on classes are also supported, using `static_property_rw`
  and `static_property_ro`, which are simply aliases for `variable_rw` and
  `variable_ro`.

  To register a subclass of another class that has been registered with Lua, use
  the `subclass` function.  This takes two template arguments: the class to be
  registered, and its base class.  Inherited methods do not have to be
  re-declared and will function normally in Lua.  If a class has a base class
  that is *not* registered with Lua, there is no need to declare it as a
  subclass.

  ## Limitations 

  LuaBridge does not support:

  - More than 8 parameters on a function or method (although this can be
    increased by adding more `typelist` specializations).
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
  [4]: http://lua.org "The Lua Programming Language"
*/

#include <cassert>
#include <string>

namespace luabridge3
{

//==============================================================================
/**
  Container traits.

  The default template supports any shared_ptr compatible interface.
  
  Specializations are provided for containers of type T, T*, T&, and T const&.

  If you need to use an incompatible container, specialize this
  template for your type and provide the required fields.
*/
template <class T>
struct ContainerInfo
{
  typedef T Type;

  static inline void* get (T& t)
  {
    return &t;
  }
};

template <class T>
struct ContainerInfo <T*>
{
  typedef typename T Type;

  static inline void* get (T* p)
  {
    return p;
  }
};

template <class T>
struct ContainerInfo <T const*>
{
  typedef typename T Type;

  static inline void* get (T const* p)
  {
    return const_cast <T*> (p);
  }
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

struct Detail
{
protected:
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
    static void* getStaticKey ()
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
    static void* getClassKey ()
    {
      static char value;
      return &value;
    }

    /**
      Get the key for the const table.

      The const table holds read-only data members and properties, and const
      member functions of a class.
    */
    static void* getConstKey ()
    {
      static char value;
      return &value;
    }
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
    A userdata wrapping a class container.

    The container type C controls the object lifetime.
  */
  template <class C>
  class UserdataType : public Userdata
  {
  private:
    UserdataType (UserdataType <C> const&);
    UserdataType <C>& operator= (UserdataType <C> const&);

    typedef typename ContainerInfo <C>::Type T;

    C m_c;

  private:
    ~UserdataType ()
    {
    }

    void* getPointer ()
    {
      return ContainerInfo <C>::get (m_c);
    }

  public:
    UserdataType ()
    {
    }

    explicit UserdataType (T const& t) : m_c (t)
    {
    }

    template <class U>
    explicit UserdataType (U const& u) : m_c (u)
    {
    }

    /**
      Create the userdata on the stack and return the object storage.

      The return value is the uninitialized storage area for the
      UserdataType object. The caller will invoke placement new.
    */
    static void* push (lua_State* L, bool makeObjectConst)
    {
      void* const ud = lua_newuserdata (L, sizeof (UserdataType <C>));
      if (makeObjectConst)
        lua_rawgetp (L, LUA_REGISTRYINDEX, ClassInfo <T>::getConstKey ());
      else
        lua_rawgetp (L, LUA_REGISTRYINDEX, ClassInfo <T>::getClassKey ());
      assert (lua_istable (L, -1));
      lua_setmetatable (L, -2);
      return ud;
    }
  };
};

//==============================================================================
/**
  Lua stack objects with value semantics.

  Lifetime is managed by Lua. A C++ function which accesses a pointer or
  reference to an object outside the activation record in which it was
  retrieved may result in undefined behavior if Lua garbage collected it.
*/
template <class T>
struct Stack : Detail
{
public:
  static inline void push (lua_State* L, T const& t, bool makeObjectConst = false)
  {
    new (UserdataType <T>::push (L, makeObjectConst)) UserdataType <T> (t);
  }

  static inline T const& get (lua_State* L, int index)
  {
    return *Userdata::get <T> (L, index, true);
  }
};

//------------------------------------------------------------------------------
/**
  Lua stack objects with pointer semantics.

  Lifetime is managed by C++. Lua code which remembers a reference to the value
  may result in undefined behavior if C++ destroys the object.
*/
template <class T>
struct Stack <T*> : Detail
{
  static inline void push (lua_State* L, T* const p)
  {
    new (UserdataType <T*>::push (L, false)) UserdataType <T*> (p);
  }

  template <class U>
  static inline void push (lua_State* L, U* const p)
  {
    new (UserdataType <T*>::push (L, false)) UserdataType <T*> (p);
  }

  static inline T* const get (lua_State* L, int index)
  {
    return Userdata::get <T> (L, index, false);
  }
};

//------------------------------------------------------------------------------
/**
  Lua stack objects with const pointer semantics.

  Lifetime is managed by C++. Lua code which remembers a reference to the value
  may result in undefined behavior if C++ destroys the object.
*/
template <class T>
struct Stack <T const*> : Detail
{
  static inline void push (lua_State* L, T const* const p)
  {
    new (UserdataType <T const*>::push (L, true)) UserdataType <T const*> (p);
  }

  template <class U>
  static inline void push (lua_State* L, U const* const p)
  {
    new (UserdataType <T const*>::push (L, true)) UserdataType <T const*> (p);
  }

  static inline T const* const get (lua_State* L, int index)
  {
    return Userdata::get <T> (L, index, true);
  }
};

//------------------------------------------------------------------------------
/**
  Lua stack objects with reference semantics.

  Lifetime is managed by C++. Lua code which remembers a reference to the value
  may result in undefined behavior if C++ destroys the object.
*/
template <class T>
struct Stack <T&> : Detail
{
  static inline void push (lua_State* L, T& t)
  {
    new (UserdataType <T*>::push (L, false)) UserdataType <T*> (&t);
  }

  template <class U>
  static inline void push (lua_State* L, U& u)
  {
    new (UserdataType <T*>::push (L, false)) UserdataType <T*> (&u);
  }

  static T& get (lua_State* L, int index)
  {
    return *Userdata::get <T> (L, index, false);
  }
};

//------------------------------------------------------------------------------
/**
  Lua stack objects with const reference semantics.

  Lifetime is managed by C++. Lua code which remembers a reference to the value
  may result in undefined behavior if C++ destroys the object.
*/
template <class T>
struct Stack <T const&> : Detail
{
  static inline void push (lua_State* L, T const& t)
  {
    new (UserdataType <T const*>::push (L, true)) UserdataType <T const*> (&t);
  }

  template <class U>
  static inline void push (lua_State* L, U const& u)
  {
    new (UserdataType <T const*>::push (L, true)) UserdataType <T const*> (&u);
  }

  static T const& get (lua_State* L, int index)
  {
    return *Userdata::get <T> (L, index, true);
  }
};

//------------------------------------------------------------------------------

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
//
// typelist
//
//==============================================================================
/*
  Implementation of C++ template type lists and related tools.
  Type list and definition of nil type list, which is void.
*/

typedef void nil;

template <typename Head, typename Tail = nil>
struct typelist
{
  /*
  static std::string const tostring ()
  {
    std::string s = ", " + typeid (Head).name ();
    return s;
  }
  */
};

/*
* Type/value list.
*/

template <typename Typelist>
struct typevallist
{
  static std::string const tostring (bool)
  {
    return "";
  }
};

template <typename Head, typename Tail>
struct typevallist <typelist <Head, Tail> >
{
  Head hd;
  typevallist <Tail> tl;

  typevallist (Head hd_, typevallist <Tail> const& tl_)
    : hd (hd_), tl (tl_)
  {
  }

  static std::string const tostring (bool comma = false)
  {
    std::string s;

    if (comma)
      s = ", ";

    s = s + typeid (Head).name ();

    return s + typevallist <Tail>::tostring (true);
  }
};

// Specializations of type/value list for head types that are references and
// const-references.  We need to handle these specially since we can't count
// on the referenced object hanging around for the lifetime of the list.

template <typename Head, typename Tail>
struct typevallist <typelist <Head&, Tail> >
{
  Head hd;
  typevallist <Tail> tl;

  typevallist (Head& hd_, typevallist <Tail> const& tl_)
    : hd (hd_), tl (tl_)
  {
  }

  static std::string const tostring (bool comma = false)
  {
    std::string s;

    if (comma)
      s = ", ";

    s = s + typeid (Head).name () + "&";

    return s + typevallist <Tail>::tostring (true);
  }
};

template <typename Head, typename Tail>
struct typevallist <typelist <Head const&, Tail> >
{
  Head hd;
  typevallist <Tail> tl;

  typevallist (Head const& hd_, const typevallist <Tail>& tl_)
    : hd (hd_), tl (tl_)
  {
  }

  static std::string const tostring (bool comma = false)
  {
    std::string s;

    if (comma)
      s = ", ";

    s = s + typeid (Head).name () + " const&";

    return s + typevallist <Tail>::tostring (true);
  }
};

//==============================================================================
/*
* Containers for function pointer types.  We have three kinds of containers:
* one for global functions, one for non-const member functions, and one for
* const member functions.  These containers allow the function pointer types
* to be broken down into their components.
*
* Of course, because of the limitations of C++ templates, we can only define
* this for up to a constant number of function parameters.  We give the
* definitions for up to 8 parameters here, though this can be easily
* increased if necessary.
*/

template <typename MemFn>
struct FunctionPointer {};

/* Ordinary function pointers. */

template <typename Ret>
struct FunctionPointer <Ret (*) ()>
{
  static const bool mfp = false;
  typedef Ret resulttype;
  typedef nil params;
  static Ret call (Ret (*fp) (), const typevallist<params> &tvl)
  {
    (void)tvl;
    return fp();
  }
};

template <typename Ret, typename P1>
struct FunctionPointer <Ret (*) (P1)>
{
  static const bool mfp = false;
  typedef Ret resulttype;
  typedef typelist<P1> params;
  static Ret call (Ret (*fp) (P1), const typevallist<params> &tvl)
  {
    return fp(tvl.hd);
  }
};

template <typename Ret, typename P1, typename P2>
struct FunctionPointer <Ret (*) (P1, P2)>
{
  static const bool mfp = false;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2> > params;
  static Ret call (Ret (*fp) (P1, P2), const typevallist<params> &tvl)
  {
    return fp(tvl.hd, tvl.tl.hd);
  }
};

template <typename Ret, typename P1, typename P2, typename P3>
struct FunctionPointer <Ret (*) (P1, P2, P3)>
{
  static const bool mfp = false;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3> > > params;
  static Ret call (Ret (*fp) (P1, P2, P3), const typevallist<params> &tvl)
  {
    return fp(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <typename Ret, typename P1, typename P2, typename P3, typename P4>
struct FunctionPointer <Ret (*) (P1, P2, P3, P4)>
{
  static const bool mfp = false;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4> > > > params;
  static Ret call (Ret (*fp) (P1, P2, P3, P4),
    const typevallist<params> &tvl)
  {
    return fp(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <typename Ret, typename P1, typename P2, typename P3, typename P4,
  typename P5>
struct FunctionPointer <Ret (*) (P1, P2, P3, P4, P5)>
{
  static const bool mfp = false;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4,
    typelist<P5> > > > > params;
  static Ret call (Ret (*fp) (P1, P2, P3, P4, P5),
    const typevallist<params> &tvl)
  {
    return fp(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
};

template <typename Ret, typename P1, typename P2, typename P3, typename P4,
  typename P5, typename P6>
struct FunctionPointer <Ret (*) (P1, P2, P3, P4, P5, P6)>
{
  static const bool mfp = false;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5, 
    typelist<P6> > > > > > params;
  static Ret call (Ret (*fp) (P1, P2, P3, P4, P5, P6),
    const typevallist<params> &tvl)
  {
    return fp(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <typename Ret, typename P1, typename P2, typename P3, typename P4,
  typename P5, typename P6, typename P7>
struct FunctionPointer <Ret (*) (P1, P2, P3, P4, P5, P6, P7)>
{
  static const bool mfp = false;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5,
    typelist<P6, typelist<P7> > > > > > > params;
  static Ret call (Ret (*fp) (P1, P2, P3, P4, P5, P6, P7),
    const typevallist<params> &tvl)
  {
    return fp(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <typename Ret, typename P1, typename P2, typename P3, typename P4,
  typename P5, typename P6, typename P7, typename P8>
struct FunctionPointer <Ret (*) (P1, P2, P3, P4, P5, P6, P7, P8)>
{
  static const bool mfp = false;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5,
    typelist<P6, typelist<P7, typelist<P8> > > > > > > > params;
  static Ret call (Ret (*fp) (P1, P2, P3, P4, P5, P6, P7, P8),
    const typevallist<params> &tvl)
  {
    return fp(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
};

/* Non-const member function pointers. */

template <class T, typename Ret>
struct FunctionPointer <Ret (T::*) ()>
{
  static const bool mfp = true;
  static const bool const_mfp = false;
  typedef T classtype;
  typedef Ret resulttype;
  typedef nil params;
  static Ret call (T *obj, Ret (T::*fp) (), const typevallist<params> &tvl)
  {
    (void)tvl;
    return (obj->*fp)();
  }
};

template <class T, typename Ret, typename P1>
struct FunctionPointer <Ret (T::*) (P1)>
{
  static const bool mfp = true;
  static const bool const_mfp = false;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1> params;
  static Ret call (T *obj, Ret (T::*fp) (P1),
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd);
  }
};

template <class T, typename Ret, typename P1, typename P2>
struct FunctionPointer <Ret (T::*) (P1, P2)>
{
  static const bool mfp = true;
  static const bool const_mfp = false;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2> > params;
  static Ret call (T *obj, Ret (T::*fp) (P1, P2),
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd);
  }
};

template <class T, typename Ret, typename P1, typename P2, typename P3>
struct FunctionPointer <Ret (T::*) (P1, P2, P3)>
{
  static const bool mfp = true;
  static const bool const_mfp = false;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3> > > params;
  static Ret call (T *obj, Ret (T::*fp) (P1, P2, P3),
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <class T, typename Ret, typename P1, typename P2, typename P3,
  typename P4>
struct FunctionPointer <Ret (T::*) (P1, P2, P3, P4)>
{
  static const bool mfp = true;
  static const bool const_mfp = false;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4> > > > params;
  static Ret call (T *obj, Ret (T::*fp) (P1, P2, P3, P4),
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <class T, typename Ret, typename P1, typename P2, typename P3,
  typename P4, typename P5>
struct FunctionPointer <Ret (T::*) (P1, P2, P3, P4, P5)>
{
  static const bool mfp = true;
  static const bool const_mfp = false;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4,
    typelist<P5> > > > > params;
  static Ret call (T *obj, Ret (T::*fp) (P1, P2, P3, P4, P5),
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename Ret, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6>
struct FunctionPointer <Ret (T::*) (P1, P2, P3, P4, P5, P6)>
{
  static const bool mfp = true;
  static const bool const_mfp = false;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5,
    typelist<P6> > > > > > params;
  static Ret call (T *obj, Ret (T::*fp) (P1, P2, P3, P4, P5, P6),
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename Ret, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6, typename P7>
struct FunctionPointer <Ret (T::*) (P1, P2, P3, P4, P5, P6, P7)>
{
  static const bool mfp = true;
  static const bool const_mfp = false;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5,
    typelist<P6, typelist<P7> > > > > > > params;
  static Ret call (T *obj, Ret (T::*fp) (P1, P2, P3, P4, P5, P6, P7),
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename Ret, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6, typename P7, typename P8>
struct FunctionPointer <Ret (T::*) (P1, P2, P3, P4, P5, P6, P7, P8)>
{
  static const bool mfp = true;
  static const bool const_mfp = false;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5,
    typelist<P6, typelist<P7, typelist <P8> > > > > > > > params;
  static Ret call (T *obj, Ret (T::*fp) (P1, P2, P3, P4, P5, P6, P7, P8),
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
};

/* Const member function pointers. */

template <class T, typename Ret>
struct FunctionPointer <Ret (T::*) () const>
{
  static const bool mfp = true;
  static const bool const_mfp = true;
  typedef T classtype;
  typedef Ret resulttype;
  typedef nil params;
  static Ret call (T const* const obj, Ret (T::*fp) () const,
    const typevallist<params> &tvl)
  {
    (void)tvl;
    return (obj->*fp)();
  }
};

template <class T, typename Ret, typename P1>
struct FunctionPointer <Ret (T::*) (P1) const>
{
  static const bool mfp = true;
  static const bool const_mfp = true;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1> params;
  static Ret call (T const* const obj, Ret (T::*fp) (P1) const,
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd);
  }
};

template <class T, typename Ret, typename P1, typename P2>
struct FunctionPointer <Ret (T::*) (P1, P2) const>
{
  static const bool mfp = true;
  static const bool const_mfp = true;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2> > params;
  static Ret call (T const* const obj, Ret (T::*fp) (P1, P2) const,
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd);
  }
};

template <class T, typename Ret, typename P1, typename P2, typename P3>
struct FunctionPointer <Ret (T::*) (P1, P2, P3) const>
{
  static const bool mfp = true;
  static const bool const_mfp = true;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3> > > params;
  static Ret call (T const* const obj, Ret (T::*fp) (P1, P2, P3) const,
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <class T, typename Ret, typename P1, typename P2, typename P3,
  typename P4>
struct FunctionPointer <Ret (T::*) (P1, P2, P3, P4) const>
{
  static const bool mfp = true;
  static const bool const_mfp = true;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4> > > > params;
  static Ret call (T const* const obj, Ret (T::*fp) (P1, P2, P3, P4) const,
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <class T, typename Ret, typename P1, typename P2, typename P3,
  typename P4, typename P5>
struct FunctionPointer <Ret (T::*) (P1, P2, P3, P4, P5) const>
{
  static const bool mfp = true;
  static const bool const_mfp = true;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4,
    typelist<P5> > > > > params;
  static Ret call (T const* const obj, Ret (T::*fp) (P1, P2, P3, P4, P5) const,
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename Ret, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6>
struct FunctionPointer <Ret (T::*) (P1, P2, P3, P4, P5, P6) const>
{
  static const bool mfp = true;
  static const bool const_mfp = true;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5,
    typelist<P6> > > > > > params;
  static Ret call (T const* const obj,
    Ret (T::*fp) (P1, P2, P3, P4, P5, P6) const,
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename Ret, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6, typename P7>
struct FunctionPointer <Ret (T::*) (P1, P2, P3, P4, P5, P6, P7) const>
{
  static const bool mfp = true;
  static const bool const_mfp = true;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5,
    typelist<P6, typelist<P7> > > > > > > params;
  static Ret call (T const* const obj,
    Ret (T::*fp) (P1, P2, P3, P4, P5, P6, P7) const,
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename Ret, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6, typename P7, typename P8>
struct FunctionPointer <Ret (T::*) (P1, P2, P3, P4, P5, P6, P7, P8) const>
{
  static const bool mfp = true;
  static const bool const_mfp = true;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5,
    typelist<P6, typelist<P7, typelist<P8> > > > > > > > params;
  static Ret call (T const* const obj,
    Ret (T::*fp) (P1, P2, P3, P4, P5, P6, P7, P8) const,
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
};

/*
* Constructor generators.  These templates allow you to call operator new and
* pass the contents of a type/value list to the constructor.  Like the
* function pointer containers, these are only defined up to 8 parameters.
*/

/** Constructor generators.

    These templates call operator new with the contents of a type/value
    list passed to the constructor with up to 8 parameters. Two versions
    of call() are provided. One performs a regular new, the other performs
    a placement new.
*/
template <class T, typename Typelist>
struct constructor {};

template <class T>
struct constructor <T, nil>
{
  static T* call (typevallist <nil> const&)
  {
    return new T;
  }
  static T* call (void* mem, typevallist <nil> const&)
  {
    return new (mem) T;
  }
};

template <class T, class P1>
struct constructor <T, typelist<P1> >
{
  static T* call (const typevallist<typelist<P1> > &tvl)
  {
    return new T(tvl.hd);
  }
  static T* call (void* mem, const typevallist<typelist<P1> > &tvl)
  {
    return new (mem) T(tvl.hd);
  }
};

template <class T, class P1, class P2>
struct constructor <T, typelist<P1, typelist<P2> > >
{
  static T* call (const typevallist<typelist<P1, typelist<P2> > > &tvl)
  {
    return new T(tvl.hd, tvl.tl.hd);
  }
  static T* call (void* mem, const typevallist<typelist<P1, typelist<P2> > > &tvl)
  {
    return new (mem) T(tvl.hd, tvl.tl.hd);
  }
};

template <class T, class P1, class P2, class P3>
struct constructor <T, typelist<P1, typelist<P2, typelist<P3> > > >
{
  static T* call (const typevallist<typelist<P1, typelist<P2,
    typelist<P3> > > > &tvl)
  {
    return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
  static T* call (void* mem, const typevallist<typelist<P1, typelist<P2,
    typelist<P3> > > > &tvl)
  {
    return new (mem) T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <class T, class P1, class P2, class P3, class P4>
struct constructor <T, typelist<P1, typelist<P2, typelist<P3,
  typelist<P4> > > > >
{
  static T* call (const typevallist<typelist<P1, typelist<P2,
    typelist<P3, typelist<P4> > > > > &tvl)
  {
    return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
  static T* call (void* mem, const typevallist<typelist<P1, typelist<P2,
    typelist<P3, typelist<P4> > > > > &tvl)
  {
    return new (mem) T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <class T, class P1, class P2, class P3, class P4,
  class P5>
struct constructor <T, typelist<P1, typelist<P2, typelist<P3,
  typelist<P4, typelist<P5> > > > > >
{
  static T* call (const typevallist<typelist<P1, typelist<P2,
    typelist<P3, typelist<P4, typelist<P5> > > > > > &tvl)
  {
    return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
  static T* call (void* mem, const typevallist<typelist<P1, typelist<P2,
    typelist<P3, typelist<P4, typelist<P5> > > > > > &tvl)
  {
    return new (mem) T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
};

template <class T, class P1, class P2, class P3, class P4,
  class P5, class P6>
struct constructor <T, typelist<P1, typelist<P2, typelist<P3,
  typelist<P4, typelist<P5, typelist<P6> > > > > > >
{
  static T* call (const typevallist<typelist<P1, typelist<P2,
    typelist<P3, typelist<P4, typelist<P5, typelist<P6> > > > > > > &tvl)
  {
    return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
  static T* call (void* mem, const typevallist<typelist<P1, typelist<P2,
    typelist<P3, typelist<P4, typelist<P5, typelist<P6> > > > > > > &tvl)
  {
    return new (mem) T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, class P1, class P2, class P3, class P4,
  class P5, class P6, class P7>
struct constructor <T, typelist<P1, typelist<P2, typelist<P3,
  typelist<P4, typelist<P5, typelist<P6, typelist<P7> > > > > > > >
{
  static T* call (const typevallist<typelist<P1, typelist<P2,
    typelist<P3, typelist<P4, typelist<P5, typelist<P6,
    typelist<P7> > > > > > > > &tvl)
  {
    return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
  static T* call (void* mem, const typevallist<typelist<P1, typelist<P2,
    typelist<P3, typelist<P4, typelist<P5, typelist<P6,
    typelist<P7> > > > > > > > &tvl)
  {
    return new (mem) T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, class P1, class P2, class P3, class P4,
  class P5, class P6, class P7, class P8>
struct constructor <T, typelist<P1, typelist<P2, typelist<P3,
  typelist<P4, typelist<P5, typelist<P6, typelist<P7, 
  typelist<P8> > > > > > > > >
{
  static T* call (const typevallist<typelist<P1, typelist<P2,
    typelist<P3, typelist<P4, typelist<P5, typelist<P6,
    typelist<P7, typelist<P8> > > > > > > > > &tvl)
  {
    return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
  static T* call (void* mem, const typevallist<typelist<P1, typelist<P2,
    typelist<P3, typelist<P4, typelist<P5, typelist<P6,
    typelist<P7, typelist<P8> > > > > > > > > &tvl)
  {
    return new (mem) T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
};

//==============================================================================

/*
* Subclass of a type/value list, constructable from the Lua stack.
*/

template <typename Typelist, int start = 1>
struct arglist
{
};

template <int start>
struct arglist <nil, start> : public typevallist <nil>
{
  arglist (lua_State*)
  {
  }
};

template <typename Head, typename Tail, int start>
struct arglist <typelist <Head, Tail>, start>
  : public typevallist <typelist <Head, Tail> >
{
  arglist (lua_State* L)
    : typevallist <typelist <Head, Tail> > (Stack <Head>::get (L, start),
                                            arglist <Tail, start + 1> (L))
  {
  }
};

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
    T* const data = static_cast <T*> (lua_touserdata (L, lua_upvalueindex (1)));
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
  template <class Function,
            class ReturnType = typename FunctionPointer <Function>::resulttype>
  struct functionProxy
  {
    typedef typename FunctionPointer <Function>::params params;
    static int f (lua_State* L)
    {
      assert (lua_islightuserdata (L, lua_upvalueindex (1)));
      Function fp = reinterpret_cast <Function> (lua_touserdata (L, lua_upvalueindex (1)));
      assert (fp != 0);
      arglist <params> args (L);
      Stack <ReturnType>::push (L, FunctionPointer <Function>::call (fp, args));
      return 1;
    }
  };

  //----------------------------------------------------------------------------
  /**
    lua_CFunction to call a function with no return value.

    This is used for global functions, global properties, class static methods,
    and class static properties.
  */
  template <class Function>
  struct functionProxy <Function, void>
  {
    typedef typename FunctionPointer <Function>::params params;
    static int f (lua_State* L)
    {
      assert (lua_islightuserdata (L, lua_upvalueindex (1)));
      Function fp = reinterpret_cast <Function> (lua_touserdata (L, lua_upvalueindex (1)));
      assert (fp != 0);
      arglist <params> args (L);
      FunctionPointer <Function>::call (fp, args);
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
            class ReturnType = typename FunctionPointer <MemFn>::resulttype>
  struct methodProxy
  {
    typedef typename ContainerInfo <typename FunctionPointer <MemFn>::classtype>::Type T;
    typedef typename FunctionPointer <MemFn>::params params;

    static int callMethod (lua_State* L)
    {
      T* const t = Userdata::get <T> (L, 1, false);
      MemFn fp = *static_cast <MemFn*> (lua_touserdata (L, lua_upvalueindex (1)));
      arglist <params, 2> args (L);
      Stack <ReturnType>::push (L, FunctionPointer <MemFn>::call (t, fp, args));
      return 1;
    }

    static int callConstMethod (lua_State* L)
    {
      T const* const t = Userdata::get <T> (L, 1, true);
      MemFn fp = *static_cast <MemFn*> (lua_touserdata (L, lua_upvalueindex (1)));
      arglist <params, 2> args(L);
      Stack <ReturnType>::push (L, FunctionPointer <MemFn>::call (t, fp, args));
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
    typedef typename ContainerInfo <typename FunctionPointer <MemFn>::classtype>::Type T;
    typedef typename FunctionPointer <MemFn>::params params;

    static int callMethod (lua_State* L)
    {
      T* const t = Userdata::get <T> (L, 1, false);
      MemFn fp = *static_cast <MemFn*> (lua_touserdata (L, lua_upvalueindex (1)));
      arglist <params, 2> args (L);
      FunctionPointer <MemFn>::call (t, fp, args);
      return 0;
    }

    static int callConstMethod (lua_State* L)
    {
      T const* const t = Userdata::get <T> (L, 1, true);
      MemFn fp = *static_cast <MemFn*> (lua_touserdata (L, lua_upvalueindex (1)));
      arglist <params, 2> args (L);
      FunctionPointer <MemFn>::call (t, fp, args);
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
            "attempt to set '%s', which isn't a property", lua_tostring (L, 2));
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
      lua_pushvalue (L, -1);
      lua_setmetatable (L, -2);
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
      lua_pushvalue (L, -1);
      rawsetfield (L, -5, name);
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
      Copy constructor.
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

    The Lua stack holds these objects:
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

    //==========================================================================
    /**
      lua_CFunction to construct a class object.
    */
    template <class Params, class C>
    static int ctorProxy (lua_State* L)
    {
      typedef typename ContainerInfo <C>::Type T;
      arglist <Params, 2> args (L);
      T* const p = constructor <T, Params>::call (args);
      new (UserdataType <C>::push (L, false)) UserdataType <C> (p);
      return 1;
    }

    //--------------------------------------------------------------------------
    /**
      __gc metamethod for a class.
    */
    template <class T>
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
        lua_pushcfunction (L, &gcMetaMethod <T>);
        rawsetfield (L, -2, "__gc");

        createClassTable (name);
        lua_pushcfunction (L, &gcMetaMethod <T>);
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
    Class (char const* name, Namespace const* parent, void* staticKey)
      : ClassBase (parent->L)
    {
      m_stackSize = parent->m_stackSize + 3;
      parent->m_stackSize = 0;

      assert (lua_istable (L, -1));

      createConstTable (name);
      lua_pushcfunction (L, &gcMetaMethod <T>);
      rawsetfield (L, -2, "__gc");

      createClassTable (name);
      lua_pushcfunction (L, &gcMetaMethod <T>);
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
      assert (lua_istable (L, -1));

      rawgetfield (L, -1, "__propget");
      assert (lua_istable (L, -1));
      lua_pushlightuserdata (L, get);
      lua_pushcclosure (L, &functionProxy <U (*) (void)>::f, 1);
      rawsetfield (L, -2, name);
      lua_pop (L, 1);

      rawgetfield (L, -1, "__propset");
      assert (lua_istable (L, -1));
      if (set != 0)
      {
        lua_pushlightuserdata (L, set);
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
      lua_pushlightuserdata (L, fp);
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
      rawgetfield (L, -2, "__propget");
      rawgetfield (L, -4, "__propget");
      typedef U (T::*MemFn) () const;
      void* const v = lua_newuserdata (L, sizeof (MemFn));
      memcpy (v, &get, sizeof (MemFn));
      lua_pushcclosure (L, &methodProxy <MemFn>::callConstMethod, 1);
      lua_pushvalue (L, -1);
      rawsetfield (L, -4, name);
      rawsetfield (L, -2, name);
      lua_pop (L, 2);

      if (set != 0)
      {
        // Add to __propset in class table.
        rawgetfield (L, -2, "__propset");
        assert (lua_istable (L, -1));
        typedef void (T::* MemFn) (U);
        void* const v = lua_newuserdata (L, sizeof (MemFn));
        memcpy (v, &set, sizeof (MemFn));
        lua_pushcclosure (L, &methodProxy <MemFn>::callMethod, 1);
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
      methodHelper <MemFn, FunctionPointer <MemFn>::const_mfp>::add (L, name, mf);
      return *this;
    }

    //--------------------------------------------------------------------------
    /**
      Add or replace a primary constructor.

      The primary constructor is invoked when calling the class type table
      like a function.

      The template parameter should be a function pointer type that matches
      the desired constructor (since you can't take the address of a constructor
      and pass it as an argument).
    */
    template <class MemFn, class C>
    Class <T>& addConstructor ()
    {
      lua_pushcclosure (L, &ctorProxy <typename FunctionPointer <MemFn>::params, C>, 0);
      rawsetfield(L, -2, "__call");

      return *this;
    }
    //--------------------------------------------------------------------------
  };

private:
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
    Copy constructor.

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
    lua_pushlightuserdata (L, fp);
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
