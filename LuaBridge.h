//------------------------------------------------------------------------------
/*
  https://github.com/vinniefalco/LuaBridge
  
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
*/
//==============================================================================

#ifndef LUABRIDGE_LUABRIDGE_HEADER
#define LUABRIDGE_LUABRIDGE_HEADER

#include <cassert>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>

namespace luabridge
{

#include "LuaHelpers.h"

#include "TypeInfo.h"
#include "TypeList.h"
#include "FuncTraits.h"
#include "Constructor.h"
#include "Stack.h"

class LuaRef;

#include "LuaException.h"
#include "LuaPop.h"
#include "LuaVal.h"
#include "LuaRef.h"

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
