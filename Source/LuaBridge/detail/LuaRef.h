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

struct fromStack
{
  lua_State *m_L;

  fromStack( lua_State *L ) : m_L(L)
  {
  }
};

/** Lightweight reference to a Lua object.
*/
class LuaRef
{
  lua_State* m_L;
  int m_ref;

  /** A proxy for representing table values.
  */
  class Proxy
  {
    friend class LuaRef;

    lua_State *m_L;
    std::string m_key;
    int m_ref;

    Proxy (lua_State *L, int ref, char const *key )
      : m_L (L), m_key (key), m_ref (ref)
    {
    }

  public:
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

    double operator = (double f )
    {
      LuaPop p(m_L);
      lua_rawgeti (m_L, LUA_REGISTRYINDEX, m_ref);
      lua_pushnumber (m_L, f);
      lua_setfield (m_L, -2, m_key.c_str ());

      return f;
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
      fromStack fs (m_L);

      LuaRef ref(fs);

      return ref;
    }

    operator LuaVal ()
    {
      LuaPop p (m_L, 2);

      push ();
      fromStack fs (m_L);

      LuaRef ref(fs);

      LuaVal val(&ref);

      return val;
    }

  };

public:

  LuaRef (lua_State *L ) : m_L (L ), m_ref (LUA_NOREF )			// For "new" objects, that will be assigned a value later.
  {;}

  LuaRef (lua_State *L, const char *name ) : m_L (L )			// Reference an existing object.  Note that numbers and strings
  {															// will be copied.  Any changes will not change the original.
    lua_getglobal (m_L, name);
    m_ref = luaL_ref (m_L, LUA_REGISTRYINDEX);
  }

  LuaRef (fromStack fs ) : m_L (fs.m_L )						// Reference an existing object that is on top of the Lua stack.
  {															// Note same cavet as above.
    m_ref = luaL_ref (m_L, LUA_REGISTRYINDEX);				// Removes from stack.
  }

  LuaRef (Proxy & te ) : m_L (te.m_L )					// Reference a table element.
  {
    lua_rawgeti (te.m_L, LUA_REGISTRYINDEX, te.m_ref);
    lua_getfield (te.m_L, -1, te.m_key.c_str ());
    m_ref = luaL_ref (m_L, LUA_REGISTRYINDEX);
  }

  LuaRef (const LuaRef &obj ) : m_L (obj.m_L )
  {
    obj.push ();
    m_ref = luaL_ref (m_L, LUA_REGISTRYINDEX);
  }

  LuaRef (LuaRef *obj ) : m_L (obj->m_L )
  {
    obj->push ();
    m_ref = luaL_ref (m_L, LUA_REGISTRYINDEX);
  }

  ~LuaRef ()													// We're gone.  Release reference to object.
  {
    luaL_unref (m_L, LUA_REGISTRYINDEX, m_ref);
  }

  // Place object on top of Lua stack.
  void push () const										
  {
    lua_rawgeti (m_L, LUA_REGISTRYINDEX, m_ref);
  }

  // Return the 'type' of the object
  int type () const
  {
    int ret;

    push ();
    ret = lua_type (m_L, -1);
    lua_pop (m_L, 1);

    return ret;
  }

  // Create a new table.
  void createTable ()
  {
    lua_newtable (m_L);
    m_ref = luaL_ref (m_L, LUA_REGISTRYINDEX);
  }

  // Create a new table and associate a global variable with it.
  void createTable (const char *name )
  {
    createTable ();
    store (name);
  }

  // Return a proxy to a table element givin an index.
  Proxy operator []  (const char *key )
  {
    push ();

    if (! lua_istable (m_L, -1 ) )
      throw LuaException (m_L, "LuaRef operator [] used on a non Lua table", __FILE__, __LINE__);

    lua_pop (m_L, 1);

    return Proxy (m_L, m_ref, key);
  }

  // Return a proxy to a table element givin an index.
  Proxy operator []  (int key )
  {
    push ();

    if (! lua_istable (m_L, -1 ) )
      throw LuaException (m_L, "LuaRef operator [] used on a non Lua table", __FILE__, __LINE__);

    lua_pop (m_L, 1);

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
  std::string & operator = (std::string & str )
  {
    operator = (str.c_str ());
    return str;
  }

  // Reference a string.
  const char * operator = (const char *str )
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
  void store (const char *name, LuaRef & table )
  {
    if (table.type () != LUA_TTABLE )
      throw LuaException (m_L, "given object is not a table.", __FILE__, __LINE__);

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

  // The next few overloads of operator () allow calling a referenced Lua object (provided it's a function),
  // with the same syntax as calling a C/C++ function.  Only the types LuaVal has conversions for can be used.
  // Upto 4 parameters, but more can be added.  Returns true on succesfull call.  No results are returned.

  LuaRef operator () ()
  {
    push ();

    if (lua_isfunction (m_L, -1 ) )
    {
      if (lua_pcall (m_L, 0, 1, 0 ) != 0 )
        throw LuaException (m_L, "Error running function in LuaRef operator ()", __FILE__, __LINE__);
    }
    else
    {
      lua_pop (m_L, 1);
      throw LuaException (m_L, "LuaRef operator () called but does not reference a function", __FILE__, __LINE__);
    }

    fromStack fs (m_L);

    LuaRef ref (fs);

    return ref;
  }

  LuaRef operator ()  (LuaVal p1 )
  {
    push ();

    if (lua_isfunction (m_L, -1 ) )
    {
      p1.push(m_L);

      if (lua_pcall (m_L, 1, 1, 0 ) != 0 )
        throw LuaException (m_L, "Error running function in LuaRef operator ()", __FILE__, __LINE__);
    }
    else
    {
      lua_pop (m_L, 1);
      throw LuaException (m_L, "LuaRef operator () called but does not reference a function", __FILE__, __LINE__);
    }

    fromStack fs (m_L);

    LuaRef ref (fs);

    return ref;
  }

  LuaRef operator ()  (LuaVal p1, LuaVal p2 )
  {
    push ();

    if (lua_isfunction (m_L, -1 ) )
    {
      p1.push(m_L);
      p2.push(m_L);

      if (lua_pcall (m_L, 2, 1, 0 ) != 0 )
        throw LuaException (m_L, "Error running function in LuaRef operator ()", __FILE__, __LINE__);
    }
    else
    {
      lua_pop (m_L, 1);
      throw LuaException (m_L, "LuaRef operator () called but does not reference a function", __FILE__, __LINE__);
    }

    fromStack fs (m_L);

    LuaRef ref (fs);

    return ref;
  }

  LuaRef operator ()  (LuaVal p1, LuaVal p2, LuaVal p3 )
  {
    push ();

    if (lua_isfunction (m_L, -1 ) )
    {
      p1.push(m_L);
      p2.push(m_L);
      p3.push(m_L);

      if (lua_pcall (m_L, 3, 1, 0 ) != 0 )
        throw LuaException (m_L,"Error running function in LuaRef operator ()" , __FILE__, __LINE__);
    }
    else
    {
      lua_pop (m_L, 1);
      throw LuaException (m_L, "LuaRef operator () called but does not reference a function", __FILE__, __LINE__);
    }

    fromStack fs (m_L);

    LuaRef ref (fs);

    return ref;
  }

  LuaRef operator ()  (LuaVal p1, LuaVal p2, LuaVal p3, LuaVal p4 )
  {
    push ();

    if (lua_isfunction (m_L, -1 ) )
    {
      p1.push(m_L);
      p2.push(m_L);
      p3.push(m_L);
      p4.push(m_L);

      if (lua_pcall (m_L, 4, 1, 0 ) != 0 )
        throw LuaException (m_L, "Error running function in LuaRef operator ()", __FILE__, __LINE__);
    }
    else
    {
      lua_pop (m_L, 1);
      throw LuaException (m_L,"LuaRef operator () called but does not reference a function" , __FILE__, __LINE__);
    }

    fromStack fs (m_L);

    LuaRef ref (fs);

    return ref;
  }
};
