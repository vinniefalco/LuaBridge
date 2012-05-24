//==============================================================================

/** Utility class to wrap a reference stored in the registry.

    These are reference counted, so multiple ref objects may point to the
    same item in the registry. When the last ref is deleted, the registry
    reference is unrefed (via luaL_unref).

    @note The implementation of the reference counting is not thread safe,
          since this would require C++11 or platform-specifics. This should not
          be a problem, since a lua_State is generally not thread safe either.
*/
class Ref
{
private:
  /** Holds a reference counted registry reference.
  */
  struct Holder
  {
    lua_State* const L;
    int const ref;
    int const type;

  public:
    /** Create the holder from a Lua stack index.
    */
    Holder (lua_State* L_, int index)
      : L(L_)
      , ref ((lua_pushvalue (L, index), luaL_ref (L, LUA_REGISTRYINDEX)))
      , type (lua_type (L, index))
      , m_count (1)
    {
    }

    /** Destroy the reference in the registry.

        @note The Lua object will be eligible for collection if no other
              Lua objects, stack variables, or upvalues are referencing it.
    */
    ~Holder ()
    {
      luaL_unref (L, LUA_REGISTRYINDEX, ref);
    }

    /** Increment the reference count.

        @note This is not thread safe.
    */
    inline void addref ()
    {
      ++m_count;
    }

    /** Decrement the reference count.

        @note This is not thread safe.
    */
    inline void release ()
    {
      if (--m_count == 0)
        delete this;
    }

  private:
    Holder& operator= (Holder const&);

    int m_count;
  };

private:
  Holder* m_holder;

public:
  /** Create a reference to nothing.
  */
  Ref () : m_holder (0)
  {
  }

  /** Construct from a Lua stack element.
  */
  Ref (lua_State* L, int index) : m_holder (new Holder (L, index))
  {
  }

  /** Create an additional reference.
  */
  Ref (Ref const& other) : m_holder (other.m_holder)
  {
    if (m_holder != 0)
      m_holder->addref ();
  }

  /** Release the reference.
  */
  ~Ref ()
  {
    if (m_holder != 0)
      m_holder->release ();
  }

  /** Change this to point to a different reference.
  */
  Ref& operator= (Ref const& other)
  {
    if (m_holder != other.m_holder)
    {
      if (m_holder != 0)
        m_holder->release ();

      m_holder = other.m_holder;

      if (m_holder != 0)
        m_holder->addref ();
    }

    return *this;
  }

  /** Compare reference for equality.
  */
  inline bool operator== (Ref const& other) const
  {
    return m_holder == other.m_holder;
  }

  /** Retrieve the Lua type of the value.
  */
  inline int type () const
  {
    return m_holder != 0 ? m_holder->type : LUA_TNONE;
  }

  /** Retrieve the lua_State associated with the reference.
  */
  inline lua_State* L () const
  {
    assert (m_holder != 0);
    return m_holder->L;
  }

  /** Push a reference to the value onto the stack.
  */
  inline void push () const
  {
    assert (m_holder != 0);
    lua_rawgeti (m_holder->L, LUA_REGISTRYINDEX, m_holder->ref);
  }
};

//==============================================================================

/**
  Wraps a Lua function in the registry.
*/
class function
{
private:
  Ref m_ref;

public:
  /** Create a function with no reference.
  */
  function ()
  {
  }

  /** Create the function from an argument.
  */
  function (lua_State* L, int index)
    : m_ref ((luaL_checktype (L, index, LUA_TFUNCTION), Ref (L, index)))
  {
  }

  /** Push a reference to the function onto the stack.
  */
  void push ()  const
  {
    m_ref.push ();
  }

  /** Call the function with up to 8 arguments and a possible return value.
  */

  template <class R>
  R call () const
  {
    m_ref.push ();
    lua_call (m_ref.L (), 0, 1);
    return tdstack <R>::get (m_ref.L (), -1);
  }

  template <class R, class T1>
  R call (T1 t1) const
  {
    m_ref.push ();
    tdstack <T1>::push (m_ref.L (), t1);
    lua_call (m_ref.L (), 1, 1);
    return tdstack <R>::get (m_ref.L (), -1);
  }

  template <class R, class T1, class T2>
  R call (T1 t1, T2 t2) const
  {
    m_ref.push ();
    tdstack <T1>::push (m_ref.L (), t1);
    tdstack <T2>::push (m_ref.L (), t2);
    lua_call (m_ref.L (), 2, 1);
    return tdstack <R>::get (m_ref.L (), -1);
  }

  template <class R, class T1, class T2, class T3>
  R call (T1 t1, T2 t2, T3 t3) const
  {
    m_ref.push ();
    tdstack <T1>::push (m_ref.L (), t1);
    tdstack <T2>::push (m_ref.L (), t2);
    tdstack <T3>::push (m_ref.L (), t3);
    lua_call (m_ref.L (), 3, 1);
    return tdstack <R>::get (m_ref.L (), -1);
  }

  template <class R, class T1, class T2, class T3, class T4>
  R call (T1 t1, T2 t2, T3 t3, T4 t4) const
  {
    m_ref.push ();
    tdstack <T1>::push (m_ref.L (), t1);
    tdstack <T2>::push (m_ref.L (), t2);
    tdstack <T3>::push (m_ref.L (), t3);
    tdstack <T4>::push (m_ref.L (), t4);
    lua_call (m_ref.L (), 4, 1);
    return tdstack <R>::get (m_ref.L (), -1);
  }

  template <class R, class T1, class T2, class T3, class T4, class T5>
  R call (T1 t1, T2 t2, T3 t3, T4 t4, T5 t5) const
  {
    m_ref.push ();
    tdstack <T1>::push (m_ref.L (), t1);
    tdstack <T2>::push (m_ref.L (), t2);
    tdstack <T3>::push (m_ref.L (), t3);
    tdstack <T4>::push (m_ref.L (), t4);
    tdstack <T5>::push (m_ref.L (), t5);;
    lua_call (m_ref.L (), 5, 1);
    return tdstack <R>::get (m_ref.L (), -1);
  }

  template <class R, class T1, class T2, class T3, class T4,
                     class T5, class T6>
  R call (T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6) const
  {
    m_ref.push ();
    tdstack <T1>::push (m_ref.L (), t1);
    tdstack <T2>::push (m_ref.L (), t2);
    tdstack <T3>::push (m_ref.L (), t3);
    tdstack <T4>::push (m_ref.L (), t4);
    tdstack <T5>::push (m_ref.L (), t5);
    tdstack <T6>::push (m_ref.L (), t6);
    lua_call (m_ref.L (), 6, 1);
    return tdstack <R>::get (m_ref.L (), -1);
  }

  template <class R, class T1, class T2, class T3, class T4,
                     class T5, class T6, class T7>
  R call (T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7) const
  {
    m_ref.push ();
    tdstack <T1>::push (m_ref.L (), t1);
    tdstack <T2>::push (m_ref.L (), t2);
    tdstack <T3>::push (m_ref.L (), t3);
    tdstack <T4>::push (m_ref.L (), t4);
    tdstack <T5>::push (m_ref.L (), t5);
    tdstack <T6>::push (m_ref.L (), t6);
    tdstack <T7>::push (m_ref.L (), t7);
    return tdstack <R>::get (m_ref.L (), -1);
  }

  template <class R, class T1, class T2, class T3, class T4,
                     class T5, class T6, class T7, class T8>
  R call (T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8) const
  {
    m_ref.push ();
    tdstack <T1>::push (m_ref.L (), t1);
    tdstack <T2>::push (m_ref.L (), t2);
    tdstack <T3>::push (m_ref.L (), t3);
    tdstack <T4>::push (m_ref.L (), t4);
    tdstack <T5>::push (m_ref.L (), t5);
    tdstack <T6>::push (m_ref.L (), t6);
    tdstack <T7>::push (m_ref.L (), t7);
    tdstack <T8>::push (m_ref.L (), t8);
    lua_call (m_ref.L (), 8, 1);
    return tdstack <R>::get (m_ref.L (), -1);
  }

  // void return

  void call () const
  {
    m_ref.push ();
    lua_call (m_ref.L (), 0, 0);
  }

  template <class T1>
  void call (T1 t1) const
  {
    m_ref.push ();
    tdstack <T1>::push (m_ref.L (), t1);
    lua_call (m_ref.L (), 1, 0);
  }

  template <class T1, class T2>
  void call (T1 t1, T2 t2) const
  {
    m_ref.push ();
    tdstack <T1>::push (m_ref.L (), t1);
    tdstack <T2>::push (m_ref.L (), t2);
    lua_call (m_ref.L (), 2, 0);
  }

  template <class T1, class T2, class T3>
  void call (T1 t1, T2 t2, T3 t3) const
  {
    m_ref.push ();
    tdstack <T1>::push (m_ref.L (), t1);
    tdstack <T2>::push (m_ref.L (), t2);
    tdstack <T3>::push (m_ref.L (), t3);
    lua_call (m_ref.L (), 3, 0);
  }

  template <class T1, class T2, class T3, class T4>
  void call (T1 t1, T2 t2, T3 t3, T4 t4) const
  {
    m_ref.push ();
    tdstack <T1>::push (m_ref.L (), t1);
    tdstack <T2>::push (m_ref.L (), t2);
    tdstack <T3>::push (m_ref.L (), t3);
    tdstack <T4>::push (m_ref.L (), t4);
    lua_call (m_ref.L (), 4, 0);
  }

  template <class T1, class T2, class T3, class T4, class T5>
  void call (T1 t1, T2 t2, T3 t3, T4 t4, T5 t5) const
  {
    m_ref.push ();
    tdstack <T1>::push (m_ref.L (), t1);
    tdstack <T2>::push (m_ref.L (), t2);
    tdstack <T3>::push (m_ref.L (), t3);
    tdstack <T4>::push (m_ref.L (), t4);
    tdstack <T5>::push (m_ref.L (), t5);
    lua_call (m_ref.L (), 5, 0);
  }

  template <class T1, class T2, class T3, class T4,
            class T5, class T6>
  void call (T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6) const
  {
    m_ref.push ();
    tdstack <T1>::push (m_ref.L (), t1);
    tdstack <T2>::push (m_ref.L (), t2);
    tdstack <T3>::push (m_ref.L (), t3);
    tdstack <T4>::push (m_ref.L (), t4);
    tdstack <T5>::push (m_ref.L (), t5);
    tdstack <T6>::push (m_ref.L (), t6);
    lua_call (m_ref.L (), 6, 0);
  }

  template <class T1, class T2, class T3, class T4,
            class T5, class T6, class T7>
  void call (T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7) const
  {
    m_ref.push ();
    tdstack <T1>::push (m_ref.L (), t1);
    tdstack <T2>::push (m_ref.L (), t2);
    tdstack <T3>::push (m_ref.L (), t3);
    tdstack <T4>::push (m_ref.L (), t4);
    tdstack <T5>::push (m_ref.L (), t5);
    tdstack <T6>::push (m_ref.L (), t6);
    tdstack <T7>::push (m_ref.L (), t7);
    lua_call (m_ref.L (), 7, 0);
  }

  template <class T1, class T2, class T3, class T4,
            class T5, class T6, class T7, class T8>
  void call (T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8) const
  {
    m_ref.push ();
    tdstack <T1>::push (m_ref.L (), t1);
    tdstack <T2>::push (m_ref.L (), t2);
    tdstack <T3>::push (m_ref.L (), t3);
    tdstack <T4>::push (m_ref.L (), t4);
    tdstack <T5>::push (m_ref.L (), t5);
    tdstack <T6>::push (m_ref.L (), t6);
    tdstack <T7>::push (m_ref.L (), t7);
    tdstack <T8>::push (m_ref.L (), t8);
    lua_call (m_ref.L (), 8, 0);
  }
};

//------------------------------------------------------------------------------
/**
  A Lua function on the stack.

  @note To simplify the implementation, the function is immediately stored
        in the registry as long as it is referenced.
*/
template <>
struct tdstack <function>
{
  static void push (lua_State*, function f)
  {
    f.push ();
  }

  static function get (lua_State* L, int index)
  {
    return function (L, index);
  }
};

//==============================================================================

/**
  Wraps a Lua table in the registry.
*/
class Table
{
private:
  Ref m_ref;

public:
  Table ()
  {
  }

  /** Create the object from an argument.
  */
  Table (lua_State* L, int index)
    : m_ref ((luaL_checktype (L, index, LUA_TTABLE), Ref (L, index)))
  {
  }

  /** Retrieve the lua_State associated with the reference.
  */
  inline lua_State* L () const
  {
    return m_ref.L ();
  }

  /** Retrieve the Lua type of the value.
  */
  inline int type () const
  {
    return m_ref.type ();
  }

  /** Retrieve the value associated with a key by string.

      @note This may trigger metamethods.
  */
  template <class T>
  T operator[] (char const* key)
  {
    lua_State* const L (m_ref.L ());
    m_ref.push ();
    lua_getfield (L, -1, key);
    lua_remove (L, -2);
    T t (tdstack <T>::get (L, -1));
    lua_pop (L, 1);
    return t;
  }
 
  /** Retrieve the value associated with a key of arbitrary type.

      @note The type must be recognized by tdstack<>.
  */
  template <class T, class U>
  T operator[] (U key)
  {
    lua_State* const L (m_ref.L ());
    m_ref.push ();
    tdstack <U>::push (L, key);
    lua_gettable (L, -2);
    lua_remove (L, -2);
    T t (tdstack <T>::get (L, -1));
    lua_pop (L, 1);
    return t;
  }

  /** Push a reference to the table onto the stack.
  */
  void push ()
  {
    m_ref.push ();
  }
};

//------------------------------------------------------------------------------

/**
  A Lua table on the stack.
*/
template <>
struct tdstack <Table>
{
  static void push (lua_State*, Table table)
  {
    table.push ();
  }

  static Table get (lua_State* L, int index)
  {
    return Table (L, index);
  }
};

//==============================================================================

/**
  Wraps any Lua type in the registry.
*/
class Object
{
private:
  Ref m_ref;

public:
  Object ()
  {
  }

  /** Create the object from an argument.
  */
  Object (lua_State* L, int index)
    : m_ref (Ref (L, index))
  {
  }

  /** Retrieve the lua_State associated with the reference.
  */
  inline lua_State* L () const
  {
    return m_ref.L ();
  }

  /** Retrieve the Lua type of the value.
  */
  inline int type () const
  {
    return m_ref.type ();
  }

  /** Push a reference to the object onto the stack.
  */
  void push ()
  {
    m_ref.push ();
  }
};

//------------------------------------------------------------------------------

/**
  Any Lua type on the stack, as a variant.
*/
template <>
struct tdstack <Object>
{
  static void push (lua_State*, Object object)
  {
    object.push ();
  }

  static Object get (lua_State* L, int index)
  {
    return Object (L, index);
  }
};

//==============================================================================
