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

class FromStack
{
private:
  lua_State* m_L;

public:
  explicit FromStack (lua_State* L)
    : m_L (L)
  {
  }

  lua_State* const state () const
  {
    return m_L;
  }
};

/** Lightweight reference to a Lua object.

    The reference is maintained for the lifetime of the C++ object.
*/
class LuaRef
{
private:
  lua_State* m_L;
  int m_ref;

private:
  class Proxy;
  friend struct Stack <Proxy>;

  //----------------------------------------------------------------------------
  /**
      A proxy for representing table values.
  */
  class Proxy
  {
  private:
    lua_State* m_L;
    int m_tableRef;
    int m_keyRef;

  public:
    /** Construct a Proxy from a table value.

        The table is in the registry, and the key is at the top of the stack.
    */
    Proxy (lua_State* L, int tableRef)
      : m_L (L)
      , m_tableRef (tableRef)
      , m_keyRef (luaL_ref (L, LUA_REGISTRYINDEX))
    {
    }

    ~Proxy ()
    {
      luaL_unref (m_L, LUA_REGISTRYINDEX, m_keyRef);
    }

    /** Retrieve the lua_State associated with the table value.
    */
    lua_State* state () const
    {
      return m_L;
    }

    /** Push the value onto the Lua stack.
    */
    void push () const
    {
      lua_rawgeti (m_L, LUA_REGISTRYINDEX, m_tableRef);
      lua_rawgeti (m_L, LUA_REGISTRYINDEX, m_keyRef);
      lua_gettable (m_L, -2);
      lua_remove (m_L, -2); // ditch the table
    }

    /** Return a reference to the table value.
    */
    int createRef () const
    {
      push ();
      return luaL_ref (m_L, LUA_REGISTRYINDEX);
    }

    /** Retrieve the Lua type of this value.

        The return values are the same as those from lua_type().
    */
    int type ()
    {
      int result;
      push ();
      result = lua_type (m_L, -1);
      lua_pop (m_L, 1);
      return result;
    }

    /** Assign a new value to this table key.
    */
    template <class U>
    void operator= (U u)
    {
      LuaPop p (m_L);
      lua_rawgeti (m_L, LUA_REGISTRYINDEX, m_tableRef);
      lua_rawgeti (m_L, LUA_REGISTRYINDEX, m_keyRef);
      Stack <U>::push (m_L, u);
      lua_settable (m_L, -3);
    }

#if 0
    LuaRef& operator= (LuaRef &obj)
    {
      LuaPop p(m_L);

      lua_rawgeti (m_L, LUA_REGISTRYINDEX, m_ref);
      obj.push ();
      lua_setfield (m_L, -2, m_key.c_str ());

      return obj;
    }

    operator double ()
    {
      //luaPop p1(m_L,2);		// Removes index and table from Lua stack once out of scope.

      push ();	

      return lua_tonumber (m_L, -1);
    }

    operator std::string ()
    {
      push ();	
      std::string str (lua_tostring (m_L, -1 ));
      lua_pop (m_L, 2);		// Removes index and table from Lua stack

      return str;
    }

    operator LuaRef ()
    {
      LuaPop p (m_L, 2);

      push ();
      FromStack fs (m_L);

      LuaRef ref(fs);

      return ref;
    }

    operator LuaVal ()
    {
      LuaPop p (m_L, 2);

      push ();
      FromStack fs (m_L);

      LuaRef ref(fs);

      LuaVal val(&ref);

      return val;
    }
#endif
  };

public:
  /** Create an empty reference.

      The LuaRef can be assigned a value later.
  */
  explicit LuaRef (lua_State *L)
    : m_L (L)
    , m_ref (LUA_NOREF)
  {
  }

  /** Create a reference to a named global.

      Numbers and strings will be copied. Changes to the referenced value
      will not change the original. (VF: NEEDS VERIFICATION!)
  */
  LuaRef (lua_State *L, char const* name)
    : m_L (L)
  {
    lua_getglobal (m_L, name);
    m_ref = luaL_ref (m_L, LUA_REGISTRYINDEX);
  }

  /** Create a reference to an object at the top of the Lua stack.

      The stack element is popped.
  */
  explicit LuaRef (FromStack fs)
    : m_L (fs.state ())
  {
    if (lua_gettop (m_L) >= 1)
    {
      m_ref = luaL_ref (m_L, LUA_REGISTRYINDEX);
    }
    else
    {
      // An argument can be made for either LUA_NOREF and LUA_REFNIL.
      //
      m_ref = LUA_NOREF;
    }
  }

  /** Create a reference to a table value.
  */
  LuaRef (Proxy const& te)
    : m_L (te.state ())
    , m_ref (te.createRef ())
  {
  }

  /** Create a new reference to an existing reference.
  */
  LuaRef (LuaRef const& other)
    : m_L (other.m_L)
    , m_ref (other.createRef ())
  {
  }

  /** Destroy a reference.

      The corresponding Lua registry reference will be released.
  */
  ~LuaRef ()
  {
    luaL_unref (m_L, LUA_REGISTRYINDEX, m_ref);
  }

protected:
  /** Create a reference to this ref.

      This is used internally.
  */
  int createRef () const
  {
    push ();
    return luaL_ref (m_L, LUA_REGISTRYINDEX);
  }

  //----------------------------------------------------------------------------

public:
  /** Create a new empty table and return a reference to it.
  */
  static LuaRef createTable (lua_State* L)
  {
    lua_newtable (L);
    return LuaRef (FromStack (L));
  }

  /* VF: This function seems extraneous.
  /*
  void createTable (const char *name )
  {
    createTable ();
    store (name);
  }
  */

  //----------------------------------------------------------------------------

  /** Retrieve the lua_State associated with the reference.
  */
  lua_State* state () const
  {
    return m_L;
  }

  /** Determine the object type.

      The return values are the same as for lua_type().
  */
  int type () const
  {
    int ret;
    push ();
    ret = lua_type (m_L, -1);
    lua_pop (m_L, 1);
    return ret;
  }

  /** Place the object onto the Lua stack.
  */
  void push () const										
  {
    lua_rawgeti (m_L, LUA_REGISTRYINDEX, m_ref);
  }

  /** Print a text description of the value to a stream.
  */
  void print (std::ostream& os)
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
      os << cast <bool> () ? "true" : "false";
      break;

    case LUA_TSTRING:
      os << cast <std::string> ();
      break;

    case LUA_TTABLE:
      os << "table";
      break;

    case LUA_TFUNCTION:
      os << "function";
      break;

    case LUA_TUSERDATA:
      os << "userdata";
      break;

    case LUA_TTHREAD:
      os << "thread";
      break;

    case LUA_TLIGHTUSERDATA:
      os << "lightuserdata";
      break;

    default:
      os << "unknown";
      break;
    }
  }

  /** Access a table value using a key.

      This invokes metamethods.
  */
  template <class U>
  Proxy operator[] (U key) const
  {
    Stack <U>::push (m_L, key);
    return Proxy (m_L, m_ref);
  }

  /** Convert the referenced value to a different type.
  */
  template <class U>
  U cast () const
  {
    push ();
    return Stack <U>::get (m_L, -1);
  }

  /** Reference a different object.
  */
  template <class U>
  LuaRef& operator= (U u)
  {
    luaL_unref (m_L, LUA_REGISTRYINDEX, m_ref);
    Stack <U>::push (m_L, u);
    luaL_ref (m_L, LUA_REGISTRYINDEX);
    return *this;
  }

#if 0
  /** Create a reference to an existing object.
  */
  LuaRef& operator= (LuaRef const& rhs)
  {
    luaL_unref (m_L, LUA_REGISTRYINDEX, m_ref);
    m_L = rhs.m_L;
    m_ref = rhs.createRef ();
    return *this;
  }

  // Reference a number as a new object.
  double operator = (double f )
  {
    lua_pushnumber (m_L, f);
    m_ref = luaL_ref (m_L, LUA_REGISTRYINDEX);
    return f;
  }

  // Reference a string as a new object.
  std::string& operator = (std::string& str)
  {
    operator= (str.c_str ());
    return str;
  }

  // Reference a string.
  char const* operator= (const char *str )
  {
    lua_pushstring (m_L, str);
    m_ref = luaL_ref (m_L, LUA_REGISTRYINDEX);		
    return str;
  }

  // Reference a function.
  lua_CFunction operator = (lua_CFunction func )
  {
    lua_pushcfunction (m_L, func);
    m_ref = luaL_ref (m_L, LUA_REGISTRYINDEX);
    return func;
  }
#endif

  /** Set the referenced object as a named global.
  */
  LuaRef& setGlobal (char const* name )
  {
    push ();		
    lua_setglobal (m_L, name);
    return *this;
  }

  // Associate referenced object as a member of a table.
  void store (const char *name, LuaRef const& table)
  {
    table.push ();
    push ();
    lua_setfield (m_L, -2, name);	// Pops value
    lua_pop (m_L, 1);	// Pops table
  }

  // Return as a number.
  operator double ()
  {
    double ret = 0.0;
    LuaPop p(m_L);
    push ();

    if (lua_isnumber (m_L, -1 ) )
      ret = lua_tonumber (m_L, -1);
    else
      throw LuaException (m_L, "LuaRef referenced object is not a number.", __FILE__, __LINE__);

    return ret;
  }

  // Return as a string
  operator std::string ()
  {
    std::string ret;
    LuaPop p(m_L);

    push ();

    if (lua_isstring (m_L, -1 ) )
      ret = lua_tostring (m_L, -1);
    else
      throw LuaException (m_L, "LuaRef referenced object is not a string.", __FILE__, __LINE__);

    return ret;
  }

  /** Call Lua code.

      These overloads allow Lua code to be called with up to 8 parameters.
      The return value is provided as a LuaRef (which may be LUA_REFNIL).
      If an error occurs, a LuaException is thrown.
  */
  /** @{ */
  LuaRef operator() () const
  {
    push ();
    LuaException::pcall (m_L, 0, 1);
    return LuaRef (FromStack (m_L));
  }

  template <class P1>
  LuaRef operator() (P1 p1) const
  {
    push ();
    Stack <P1>::push (m_L, p1);
    LuaException::pcall (m_L, 1, 1);
    return LuaRef (FromStack (m_L));
  }

  template <class P1, class P2>
  LuaRef operator() (P1 p1, P2 p2) const
  {
    push ();
    Stack <P1>::push (m_L, p1);
    Stack <P2>::push (m_L, p2);
    LuaException::pcall (m_L, 2, 1);
    return LuaRef (FromStack (m_L));
  }

  template <class P1, class P2, class P3>
  LuaRef operator() (P1 p1, P2 p2, P3 p3) const
  {
    push ();
    Stack <P1>::push (m_L, p1);
    Stack <P2>::push (m_L, p2);
    Stack <P3>::push (m_L, p3);
    LuaException::pcall (m_L, 3, 1);
    return LuaRef (FromStack (m_L));
  }

  template <class P1, class P2, class P3, class P4, class P5>
  LuaRef operator() (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) const
  {
    push ();
    Stack <P1>::push (m_L, p1);
    Stack <P2>::push (m_L, p2);
    Stack <P3>::push (m_L, p3);
    Stack <P4>::push (m_L, p4);
    Stack <P5>::push (m_L, p5);
    LuaException::pcall (m_L, 5, 1);
    return LuaRef (FromStack (m_L));
  }

  template <class P1, class P2, class P3, class P4, class P5, class P6>
  LuaRef operator() (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) const
  {
    push ();
    Stack <P1>::push (m_L, p1);
    Stack <P2>::push (m_L, p2);
    Stack <P3>::push (m_L, p3);
    Stack <P4>::push (m_L, p4);
    Stack <P5>::push (m_L, p5);
    Stack <P6>::push (m_L, p6);
    LuaException::pcall (m_L, 6, 1);
    return LuaRef (FromStack (m_L));
  }

  template <class P1, class P2, class P3, class P4, class P5, class P6, class P7>
  LuaRef operator() (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7) const
  {
    push ();
    Stack <P1>::push (m_L, p1);
    Stack <P2>::push (m_L, p2);
    Stack <P3>::push (m_L, p3);
    Stack <P4>::push (m_L, p4);
    Stack <P5>::push (m_L, p5);
    Stack <P6>::push (m_L, p6);
    Stack <P7>::push (m_L, p7);
    LuaException::pcall (m_L, 7, 1);
    return LuaRef (FromStack (m_L));
  }

  template <class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
  LuaRef operator() (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8) const
  {
    push ();
    Stack <P1>::push (m_L, p1);
    Stack <P2>::push (m_L, p2);
    Stack <P3>::push (m_L, p3);
    Stack <P4>::push (m_L, p4);
    Stack <P5>::push (m_L, p5);
    Stack <P6>::push (m_L, p6);
    Stack <P7>::push (m_L, p7);
    Stack <P8>::push (m_L, p8);
    LuaException::pcall (m_L, 8, 1);
    return LuaRef (FromStack (m_L));
  }
  /** @} */
};

//------------------------------------------------------------------------------

/** Stack specialization for LuaRef.
*/
template <>
struct Stack <LuaRef>
{
public:
  static inline void push (lua_State* L, LuaRef v)
  {
    assert (L == v.state ());
    v.push ();
  }

  /*
    Have to make this work with a provided stack index
  static inline LuaRef get (lua_State* L, int index)
  {
    return LuaRef (FromStack (L, ));
  }
  */
};

/** Stack specialization for Proxy.
*/
template <>
struct Stack <LuaRef::Proxy>
{
public:
  static inline void push (lua_State* L, LuaRef::Proxy v)
  {
    assert (L == v.state ());
    v.push ();
  }
};

//------------------------------------------------------------------------------

inline std::ostream& operator<< (std::ostream &os, LuaRef& ref)
{
  ref.print (os);
  return os;
}
