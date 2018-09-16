//------------------------------------------------------------------------------
/*
  https://github.com/vinniefalco/LuaBridge
  
  Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
  Copyright 2008, Nigel Atkinson <suprapilot+LuaCode@gmail.com>

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

#pragma once

#include <LuaBridge/detail/LuaException.h>
#include <LuaBridge/detail/Stack.h>

#include <iostream>
#include <string>
#include <vector>
#include <map>

namespace luabridge {

//------------------------------------------------------------------------------
/**
    Type tag for representing LUA_TNIL.

    Construct one of these using `Nil()` to represent a Lua nil. This is faster
    than creating a reference in the registry to nil. Example:

        LuaRef t (LuaRef::createTable (L));
        ...
        t ["k"] = Nil(); // assign nil
*/
struct Nil
{
};

//------------------------------------------------------------------------------
/**
    Lightweight reference to a Lua object.

    The reference is maintained for the lifetime of the C++ object.
*/
class LuaRef
{
  //----------------------------------------------------------------------------
  /**
      Pop the Lua stack.

      Pops the specified number of stack items on destruction. We use this
      when returning objects, to avoid an explicit temporary variable, since
      the destructor executes after the return statement. For example:

          template <class U>
          U cast (lua_State* L)
          {
            StackPop p (L, 1);
            ...
            return U (); // dtor called after this line
          }

      @note The `StackPop` object must always be a named local variable.
  */
  class StackPop
  {
  public:
    /** Create a StackPop object.

        @param count The number of stack entries to pop on destruction.
    */
    StackPop (lua_State* L, int count)
      : m_L (L)
      , m_count (count)
    {
    }

    ~StackPop ()
    {
      lua_pop (m_L, m_count);
    }

  private:
    lua_State* m_L;
    int m_count;
  };

  friend struct Stack <LuaRef>;

  //----------------------------------------------------------------------------
  /**
      Type tag for stack construction.
  */
  struct FromStack { };

  //----------------------------------------------------------------------------
  /**
      Create a reference to an object at the top of the Lua stack and pop it.

      This constructor is private and not invoked directly.
      Instead, use the `fromStack` function.

      @note The object is popped.
  */
  LuaRef (lua_State* L, FromStack)
    : m_L (L)
  {
    m_ref = luaL_ref (m_L, LUA_REGISTRYINDEX);
  }

  //----------------------------------------------------------------------------
  /**
      Create a reference to an object on the Lua stack.

      This constructor is private and not invoked directly.
      Instead, use the `fromStack` function.

      @note The object is not popped.
  */
  LuaRef (lua_State* L, int index, FromStack)
    : m_L (L)
  {
    lua_pushvalue (m_L, index);
    m_ref = luaL_ref (m_L, LUA_REGISTRYINDEX);
  }

  //----------------------------------------------------------------------------
  /**
      Create a reference to this ref.

      This is used internally.
  */
  int createRef () const
  {
    if (m_ref != LUA_REFNIL)
    {
      push (m_L);
      return luaL_ref (m_L, LUA_REGISTRYINDEX);
    }
    else
    {
      return LUA_REFNIL;
    }
  }

public:
  //----------------------------------------------------------------------------
  /**
      Create a nil reference.

      The LuaRef may be assigned later.
  */
  LuaRef (lua_State* L)
    : m_L (L)
    , m_ref (LUA_REFNIL)
  {
  }

  //----------------------------------------------------------------------------
  /**
      Create a reference to a value.
  */
  template <class T>
  LuaRef (lua_State* L, T v)
    : m_L (L)
  {
    Stack <T>::push (m_L, v);
    m_ref = luaL_ref (m_L, LUA_REGISTRYINDEX);
  }

  //----------------------------------------------------------------------------
  /**
      Create a new reference to an existing reference.
  */
  LuaRef (LuaRef const& other)
    : m_L (other.m_L)
    , m_ref (other.createRef ())
  {
  }

  //----------------------------------------------------------------------------
  /**
      Destroy a reference.

      The corresponding Lua registry reference will be released.

      @note If the state refers to a thread, it is the responsibility of the
            caller to ensure that the thread still exists when the LuaRef
            is destroyed.
  */
  ~LuaRef ()
  {
    luaL_unref (m_L, LUA_REGISTRYINDEX, m_ref);
  }

  //----------------------------------------------------------------------------
  /**
      Return a LuaRef from a stack item.

      The stack item is not popped.
  */
  static LuaRef fromStack (lua_State* L, int index)
  {
    lua_pushvalue (L, index);
    return LuaRef (L, FromStack ());
  }

  //----------------------------------------------------------------------------
  /**
      Create a new empty table and return a reference to it.

      It is also possible to use the free function `newTable`.

      @see ::getGlobal
  */
  static LuaRef newTable (lua_State* L)
  {
    lua_newtable (L);
    return LuaRef (L, FromStack ());
  }

  //----------------------------------------------------------------------------
  /**
      Return a reference to a named global.

      It is also possible to use the free function `getGlobal`.

      @see ::getGlobal
  */
  static LuaRef getGlobal (lua_State *L, char const* name)
  {
    lua_getglobal (L, name);
    return LuaRef (L, FromStack ());
  }

  //----------------------------------------------------------------------------
  /**
  Assign nil to this LuaRef.
  */
  LuaRef& operator= (Nil const&)
  {
    LuaRef ref (m_L);
    swap (ref);
    return *this;
  }

  //----------------------------------------------------------------------------
  /**
      Assign a different value to this LuaRef.
  */
  template <class T>
  LuaRef& operator= (T rhs)
  {
    LuaRef ref (rhs);
    swap (ref);
    return *this;
  }

  //----------------------------------------------------------------------------
  /**
      Assign another LuaRef to this LuaRef.
  */
  LuaRef& operator= (LuaRef const& rhs)
  {
    LuaRef ref (rhs);
    swap (ref);
    return *this;
  }

  //----------------------------------------------------------------------------
  /**
      converts to a string using luas tostring function
  */
  std::string tostring() const
  {
    lua_getglobal (m_L, "tostring");
    push (m_L);
    lua_call (m_L, 1, 1);
    const char* str = lua_tostring(m_L, -1);
    lua_pop(m_L, 1);
    return std::string(str);
  }

  //----------------------------------------------------------------------------
  /**
      Print a text description of the value to a stream.

      This is used for diagnostics.
  */
  void print (std::ostream& os) const
  {
    switch (type ())
    {
    case LUA_TNIL:
      os << "nil";
      break;

    case LUA_TNUMBER:
      os << cast <lua_Number> ();
      break;

    case LUA_TBOOLEAN:
      os << (cast <bool> () ? "true" : "false");
      break;

    case LUA_TSTRING:
      os << '"' << cast <std::string> () << '"';
      break;

    case LUA_TTABLE:
      os << "table: " << tostring();
      break;

    case LUA_TFUNCTION:
      os << "function: " << tostring();
      break;

    case LUA_TUSERDATA:
      os << "userdata: " << tostring();
      break;

    case LUA_TTHREAD:
      os << "thread: " << tostring();
      break;

    case LUA_TLIGHTUSERDATA:
      os << "lightuserdata: " << tostring();
      break;

    default:
      os << "unknown";
      break;
    }
  }

  /**
      Retrieve the lua_State associated with the reference.
  */
  lua_State* state () const
  {
    return m_L;
  }

  //----------------------------------------------------------------------------
  /**
      Place the object onto the Lua stack.
  */
  void push () const
  {
    lua_rawgeti (m_L, LUA_REGISTRYINDEX, m_ref);
  }

  void push (lua_State* L) const
  {
    assert (equalstates (L, m_L));
    push ();
  }

  //----------------------------------------------------------------------------
  /**
      Pop the top of Lua stack and assign the ref to m_ref
  */
  void pop ()
  {
    luaL_unref (m_L, LUA_REGISTRYINDEX, m_ref);
    m_ref = luaL_ref (m_L, LUA_REGISTRYINDEX);
  }

  void pop (lua_State* L)
  {
    assert (equalstates (L, m_L));
    pop ();
  }

  //----------------------------------------------------------------------------
  /**
      Determine the object type.

      The return values are the same as for `lua_type`.
  */
  /** @{ */
  int type () const
  {
    int result;
    if (m_ref != LUA_REFNIL)
    {
      push (m_L);
      result = lua_type (m_L, -1);
      lua_pop (m_L, 1);
    }
    else
    {
      result = LUA_TNIL;
    }

    return result;
  }

  // should never happen
  // bool isNone () const { return m_ref == LUA_NOREF; }

  bool isNil () const { return type () == LUA_TNIL; }
  bool isBool () const { return type () == LUA_TBOOLEAN; }
  bool isNumber () const { return type () == LUA_TNUMBER; }
  bool isString () const { return type () == LUA_TSTRING; }
  bool isTable () const { return type () == LUA_TTABLE; }
  bool isFunction () const { return type () == LUA_TFUNCTION; }
  bool isUserdata () const { return type () == LUA_TUSERDATA; }
  bool isThread () const { return type () == LUA_TTHREAD; }
  bool isLightUserdata () const { return type () == LUA_TLIGHTUSERDATA; }

  /** @} */

  //----------------------------------------------------------------------------
  /**
      Perform an explicit conversion.
  */
  template <class T>
  T cast () const
  {
    StackPop p (m_L, 1);
    push (m_L);

    // lua_gettop is used because Userdata::getClass() doesn't handle
    // negative stack indexes.
    //
    return Stack <T>::get (m_L, lua_gettop (m_L));
  }

  //----------------------------------------------------------------------------
  /**
      Universal implicit conversion operator.

      NOTE: Visual Studio 2010 and 2012 have a bug where this function
            is not used. See:

      http://social.msdn.microsoft.com/Forums/en-US/vcgeneral/thread/e30b2664-a92d-445c-9db2-e8e0fbde2014
      https://connect.microsoft.com/VisualStudio/feedback/details/771509/correct-code-doesnt-compile

          // This code snippet fails to compile in vs2010,vs2012
          struct S {
            template <class T> operator T () const { return T (); }
          };
          int main () {
            S () || false;
            return 0;
          }
  */
  template <class T>
  operator T () const
  {
    return cast <T> ();
  }

  //----------------------------------------------------------------------------
  /**
      Universal comparison operators.
  */
  /** @{ */
  template <class T>
  bool operator== (T rhs) const
  {
    StackPop p (m_L, 2);
    push (m_L);
    Stack <T>::push (m_L, rhs);
    return lua_compare (m_L, -2, -1, LUA_OPEQ) == 1;
  }

  template <class T>
  bool operator< (T rhs) const
  {
    StackPop p (m_L, 2);
    push (m_L);
    Stack <T>::push (m_L, rhs);
    int lhsType = lua_type (m_L, -2);
    int rhsType = lua_type (m_L, -1);
    if (lhsType != rhsType)
    {
      return lhsType < rhsType;
    }
    return lua_compare (m_L, -2, -1, LUA_OPLT) == 1;
  }

  template <class T>
  bool operator<= (T rhs) const
  {
    StackPop p (m_L, 2);
    push (m_L);
    Stack <T>::push (m_L, rhs);
    int lhsType = lua_type (m_L, -2);
    int rhsType = lua_type (m_L, -1);
    if (lhsType != rhsType)
    {
      return lhsType <= rhsType;
    }
    return lua_compare (m_L, -2, -1, LUA_OPLE) == 1;
  }

  template <class T>
  bool operator> (T rhs) const
  {
    StackPop p (m_L, 2);
    push (m_L);
    Stack <T>::push (m_L, rhs);
    int lhsType = lua_type (m_L, -2);
    int rhsType = lua_type (m_L, -1);
    if (lhsType != rhsType)
    {
      return lhsType > rhsType;
    }
    return lua_compare (m_L, -1, -2, LUA_OPLT) == 1;
  }

  template <class T>
  bool operator>= (T rhs) const
  {
    StackPop p (m_L, 2);
    push (m_L);
    Stack <T>::push (m_L, rhs);
    int lhsType = lua_type (m_L, -2);
    int rhsType = lua_type (m_L, -1);
    if (lhsType != rhsType)
    {
      return lhsType >= rhsType;
    }
    return lua_compare (m_L, -1, -2, LUA_OPLE) == 1;
  }

  template <class T>
  bool rawequal (T rhs) const
  {
    StackPop p (m_L, 2);
    push (m_L);
    Stack <T>::push (m_L, rhs);
    return lua_rawequal (m_L, -1, -2) == 1;
  }
  /** @} */

  //----------------------------------------------------------------------------
  /**
      Append a value to the table.

      If the table is a sequence this will add another element to it.
  */
  template <class T>
  void append (T v) const
  {
    push (m_L);
    Stack <T>::push (m_L, v);
    luaL_ref (m_L, -2);
    lua_pop (m_L, 1);
  }

  //----------------------------------------------------------------------------
  /**
      Call the length operator.

      This is identical to applying the Lua # operator.
  */
  int length () const
  {
    StackPop p (m_L, 1);
    push (m_L);
    return get_length (m_L, -1);
  }

  //----------------------------------------------------------------------------
  /**
      Access a table value using a key.

      This invokes metamethods.
  */
  template <class T>
  LuaRef operator[] (T key) const
  {
    lua_rawgeti (m_L, LUA_REGISTRYINDEX, m_ref);
    StackPop p (m_L, 1);
    Stack <T>::push (m_L, key);
    lua_gettable (m_L, -2);
    return LuaRef (m_L, FromStack ());
  }

  //----------------------------------------------------------------------------
  /**
      Call Lua code.

      These overloads allow Lua code to be called with up to 8 parameters.
      The return value is provided as a LuaRef (which may be LUA_REFNIL).
      If an error occurs, a LuaException is thrown.
  */
  /** @{ */
  LuaRef const operator() () const
  {
    push (m_L);
    LuaException::pcall (m_L, 0, 1);
    return LuaRef (m_L, FromStack ());
  }

  template <class P1>
  LuaRef const operator() (P1 p1) const
  {
    push (m_L);
    Stack <P1>::push (m_L, p1);
    LuaException::pcall (m_L, 1, 1);
    return LuaRef (m_L, FromStack ());
  }

  template <class P1, class P2>
  LuaRef const operator() (P1 p1, P2 p2) const
  {
    push (m_L);
    Stack <P1>::push (m_L, p1);
    Stack <P2>::push (m_L, p2);
    LuaException::pcall (m_L, 2, 1);
    return LuaRef (m_L, FromStack ());
  }

  template <class P1, class P2, class P3>
  LuaRef const operator() (P1 p1, P2 p2, P3 p3) const
  {
    push (m_L);
    Stack <P1>::push (m_L, p1);
    Stack <P2>::push (m_L, p2);
    Stack <P3>::push (m_L, p3);
    LuaException::pcall (m_L, 3, 1);
    return LuaRef (m_L, FromStack ());
  }

  template <class P1, class P2, class P3, class P4>
  LuaRef const operator() (P1 p1, P2 p2, P3 p3, P4 p4) const
  {
    push (m_L);
    Stack <P1>::push (m_L, p1);
    Stack <P2>::push (m_L, p2);
    Stack <P3>::push (m_L, p3);
    Stack <P4>::push (m_L, p4);
    LuaException::pcall (m_L, 4, 1);
    return LuaRef (m_L, FromStack ());
  }

  template <class P1, class P2, class P3, class P4, class P5>
  LuaRef const operator() (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) const
  {
    push (m_L);
    Stack <P1>::push (m_L, p1);
    Stack <P2>::push (m_L, p2);
    Stack <P3>::push (m_L, p3);
    Stack <P4>::push (m_L, p4);
    Stack <P5>::push (m_L, p5);
    LuaException::pcall (m_L, 5, 1);
    return LuaRef (m_L, FromStack ());
  }

  template <class P1, class P2, class P3, class P4, class P5, class P6>
  LuaRef const operator() (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) const
  {
    push (m_L);
    Stack <P1>::push (m_L, p1);
    Stack <P2>::push (m_L, p2);
    Stack <P3>::push (m_L, p3);
    Stack <P4>::push (m_L, p4);
    Stack <P5>::push (m_L, p5);
    Stack <P6>::push (m_L, p6);
    LuaException::pcall (m_L, 6, 1);
    return LuaRef (m_L, FromStack ());
  }

  template <class P1, class P2, class P3, class P4, class P5, class P6, class P7>
  LuaRef const operator() (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7) const
  {
    push (m_L);
    Stack <P1>::push (m_L, p1);
    Stack <P2>::push (m_L, p2);
    Stack <P3>::push (m_L, p3);
    Stack <P4>::push (m_L, p4);
    Stack <P5>::push (m_L, p5);
    Stack <P6>::push (m_L, p6);
    Stack <P7>::push (m_L, p7);
    LuaException::pcall (m_L, 7, 1);
    return LuaRef (m_L, FromStack ());
  }

  template <class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
  LuaRef const operator() (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8) const
  {
    push (m_L);
    Stack <P1>::push (m_L, p1);
    Stack <P2>::push (m_L, p2);
    Stack <P3>::push (m_L, p3);
    Stack <P4>::push (m_L, p4);
    Stack <P5>::push (m_L, p5);
    Stack <P6>::push (m_L, p6);
    Stack <P7>::push (m_L, p7);
    Stack <P8>::push (m_L, p8);
    LuaException::pcall (m_L, 8, 1);
    return LuaRef (m_L, FromStack ());
  }
  /** @} */

  //============================================================================

private:
  void swap (LuaRef& other)
  {
    std::swap (m_L, other.m_L);
    std::swap (m_ref, other.m_ref);
  }

  lua_State* m_L;
  int m_ref;
};

//------------------------------------------------------------------------------
/**
    Stack specialization for Nil
*/
template <>
struct Stack <Nil>
{
  static void push (lua_State* L, Nil)
  {
    lua_pushnil (L);
  }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for LuaRef.
*/
template <>
struct Stack <LuaRef>
{
  // The value is const& to prevent a copy construction.
  //
  static void push (lua_State* L, LuaRef const& v)
  {
    v.push (L);
  }

  static LuaRef get (lua_State* L, int index)
  {
    return LuaRef (L, index, LuaRef::FromStack ());
  }
};

//------------------------------------------------------------------------------
/**
    Create a reference to a new, empty table.

    This is a syntactic abbreviation for LuaRef::newTable().
*/
inline LuaRef newTable (lua_State* L)
{
  return LuaRef::newTable (L);
}

//------------------------------------------------------------------------------
/**
    Create a reference to a value in the global table.

    This is a syntactic abbreviation for LuaRef::getGlobal().
*/
inline LuaRef getGlobal (lua_State *L, char const* name)
{
  return LuaRef::getGlobal (L, name);
}

//------------------------------------------------------------------------------
/**
    Write a LuaRef to a stream.

    This allows LuaRef and table proxies to work with streams.
*/
inline std::ostream& operator<< (std::ostream& os, LuaRef const& ref)
{
  ref.print (os);
  return os;
}

//------------------------------------------------------------------------------

// more C++-like cast syntax
//
template<class T>
T LuaRef_cast(LuaRef const& lr)
{
  return lr.cast<T>();
}

} // namespace luabridge
