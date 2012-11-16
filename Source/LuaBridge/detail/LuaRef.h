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

  lua_State* const getLuaState () const
  {
    return m_L;
  }
};

/** Lightweight reference to a Lua object.
*/
class LuaRef
{
private:
  lua_State* m_L;
  int m_ref;

private:
  /** A proxy for representing table values.
  */
  class Proxy
  {
  private:
    lua_State* m_L;
    std::string m_key;
    int m_ref;          // registry ref to the table

  public:
    Proxy (lua_State *L, int ref, char const *key )
      : m_L (L), m_key (key), m_ref (ref)
    {
    }

    /** Retrieve the lua_State associated with the table value.
    */
    lua_State* getLuaState () const
    {
      return m_L;
    }

    /** Create a reference to this table value.
    */
    int createRef () const
    {
      lua_rawgeti (m_L, LUA_REGISTRYINDEX, m_ref);
      lua_getfield (m_L, -1, m_key.c_str ());
      return luaL_ref (m_L, LUA_REGISTRYINDEX);
    }

    void push ()
    {
      lua_rawgeti (m_L, LUA_REGISTRYINDEX, m_ref);
      lua_getfield (m_L, -1, m_key.c_str ());
    }

    /** Retrieve the Lua type of this value.

        The return values are the same as those from lua_type().
    */
    int type ()
    {
      int result;
      push ();
      result = lua_type (m_L, -1);
      lua_pop (m_L, 2);
      return result;
    }

    /** Assign a new value to this table key.
    */
    template <class U>
    void operator= (U u)
    {
      LuaPop p (m_L);
      lua_rawgeti (m_L, LUA_REGISTRYINDEX, m_ref);
      Stack <U>::push (m_L, u);
      lua_setfield (m_L, -2, m_key.c_str ());
    }

#if 0
    double operator= (double value)
    {
      LuaPop p(m_L);
      lua_rawgeti (m_L, LUA_REGISTRYINDEX, m_ref);
      lua_pushnumber (m_L, value);
      lua_setfield (m_L, -2, m_key.c_str ());
      return value;
    }

    std::string & operator = (std::string & str )
    {
      operator = (str.c_str ());			// Just re-use the const char * code.

      return str;
    }

    const char * operator = (const char *str )
    {
      LuaPop p(m_L);
      lua_rawgeti (m_L, LUA_REGISTRYINDEX, m_ref);
      lua_pushstring (m_L, str);
      lua_setfield (m_L, -2, m_key.c_str ());

      return str;
    }

    lua_CFunction operator = (lua_CFunction func )
    {
      LuaPop p(m_L);
      lua_rawgeti (m_L, LUA_REGISTRYINDEX, m_ref);
      lua_pushcfunction (m_L, func);
      lua_setfield (m_L, -2, m_key.c_str ());

      return func;
    }
#endif

    LuaRef & operator = (LuaRef &obj )
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
  };

protected:
  /** Create a reference to this ref.

      This is used internally for the copy constructor.
  */
  int createRef () const
  {
    push ();
    return luaL_ref (m_L, LUA_REGISTRYINDEX);
  }

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
    : m_L (fs.getLuaState ())
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
    : m_L (te.getLuaState ())
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

  /** Create a new reference from a pointer to an existing reference.

      VF: WHY IS THIS NEEDED?
  */
  LuaRef (LuaRef* pOther)
    : m_L (pOther->m_L)
    , m_ref (pOther->createRef ())
  {
  }

  /** Destroy a reference.

      The corresponding Lua registry reference will be released.
  */
  ~LuaRef ()
  {
    luaL_unref (m_L, LUA_REGISTRYINDEX, m_ref);
  }

  /** Place the object onto the Lua stack.
  */
  void push () const										
  {
    lua_rawgeti (m_L, LUA_REGISTRYINDEX, m_ref);
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

  Proxy operator[] (const char *key)
  {
    return Proxy (m_L, m_ref, key);
  }

  Proxy operator[] (int key )
  {
    push ();

    std::stringstream ss;
    ss << key;
    return Proxy (m_L, m_ref, ss.str ().c_str ());
  }

  // Reference an object referenced by "obj"
  LuaRef & operator = (const LuaRef &obj )
  {
    obj.push ();
    m_L = obj.m_L;
    m_ref = luaL_ref (m_L, LUA_REGISTRYINDEX);

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

  // Associate referenced object with a name, or global variable.
  void store (const char *name )
  {
    push ();		
    lua_setglobal (m_L, name);
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

  operator LuaVal ()
  {
    LuaVal val(this);

    return val;
  }

  // The next few overloads of operator () allow calling a callable referenced Lua object (provided it's a function),
  // with the same syntax as calling a C/C++ function.  Only the types LuaVal has conversions for can be used.
  // Upto 4 parameters, but more can be added.  Returns true on succesfull call.  No results are returned.
  /** @{ */
  LuaRef operator() () const
  {
    push ();
    LuaException::pcall (m_L, 0, 1);
    return LuaRef (FromStack (m_L));
  }

  LuaRef operator() (LuaVal p1) const
  {
    push ();
    p1.push (m_L);
    LuaException::pcall (m_L, 1, 1);
    return LuaRef (FromStack (m_L));
  }

#if 1
  template <class P1, class P2>
  LuaRef operator() (P1 p1, P2 p2) const
  {
    push ();
    Stack <P1>::push (m_L, p1);
    Stack <P2>::push (m_L, p2);
    LuaException::pcall (m_L, 2, 1);
    return LuaRef (FromStack (m_L));
  }
#else
  LuaRef operator() (LuaVal p1, LuaVal p2) const
  {
    push ();
    p1.push (m_L);
    p2.push (m_L);
    LuaException::pcall (m_L, 2, 1);
    return LuaRef (FromStack (m_L));
  }
#endif

  LuaRef operator() (LuaVal p1, LuaVal p2, LuaVal p3) const
  {
    push ();
    p1.push(m_L);
    p2.push(m_L);
    p3.push(m_L);
    LuaException::pcall (m_L, 3, 1);
    return LuaRef (FromStack (m_L));
  }

  LuaRef operator() (LuaVal p1, LuaVal p2, LuaVal p3, LuaVal p4) const
  {
    push ();
    p1.push (m_L);
    p2.push (m_L);
    p3.push (m_L);
    p4.push (m_L);
    LuaException::pcall (m_L, 4, 1);
    return LuaRef (FromStack (m_L));
  }
  /** @} */
};
