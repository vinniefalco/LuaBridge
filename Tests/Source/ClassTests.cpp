// https://github.com/vinniefalco/LuaBridge
//
// Copyright 2018, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include <exception>
#include <map>


struct ClassTests : TestBase
{
  template <class T>
  T variable (const std::string& name)
  {
    runLua ("result = " + name);
    return result ().cast <T>();
  }
};

namespace {

struct EmptyClass
{
};

template <class T, class BaseClass = EmptyClass>
struct Class : BaseClass
{
  Class ()
    : data ()
  {
  }

  Class (T data)
    : data (data)
  {
  }

  static Class <T, BaseClass> staticFunction (Class <T, BaseClass> value)
  {
    return value;
  }

  std::string toString () const
  {
    std::ostringstream stream;
    stream << data;
    return stream.str ();
  }

  bool operator== (const Class <T, BaseClass>& rhs) const
  {
    return data == rhs.data;
  }

  bool operator< (const Class <T, BaseClass>& rhs) const
  {
    return data < rhs.data;
  }

  bool operator<= (const Class <T, BaseClass>& rhs) const
  {
    return data <= rhs.data;
  }

  Class <T, BaseClass> operator+ (const Class <T, BaseClass>& rhs) const
  {
    return Class <T> (data + rhs.data);
  }

  Class <T, BaseClass> operator- (const Class <T, BaseClass>& rhs) const
  {
    return Class <T, BaseClass> (data - rhs.data);
  }

  Class <T, BaseClass> operator* (const Class <T, BaseClass>& rhs) const
  {
    return Class <T, BaseClass> (data * rhs.data);
  }

  Class <T, BaseClass> operator/ (const Class <T, BaseClass>& rhs) const
  {
    return Class <T, BaseClass> (data / rhs.data);
  }

  Class <T, BaseClass> operator% (const Class <T>& rhs) const
  {
    return Class <T, BaseClass> (data % rhs.data);
  }

  Class <T, BaseClass> operator() (T param)
  {
    return Class <T, BaseClass> (param);
  }

  int len () const
  {
    return data;
  }

  Class <T, BaseClass> negate () const
  {
    return Class <T, BaseClass> (-data);
  }

  T method (T value)
  {
    return value;
  }

  T constMethod (T value) const
  {
    return value;
  }

  static T getData (const Class <T, BaseClass>* object)
  {
    return object->data;
  }

  static void setData (Class <T, BaseClass>* object, T data)
  {
    object->data = data;
  }

  mutable T data;
  static T staticData;
};

template <class T, class BaseClass>
T Class <T, BaseClass>::staticData = {};

} // namespace

TEST_F (ClassTests, PassingUnregisteredClassToLuaThrows)
{
  using Unregistered = Class <int>;

  runLua ("function process_fn (value) end");

  auto process_fn = luabridge::getGlobal (L, "process_fn");
  ASSERT_TRUE (process_fn.isFunction ());

  Unregistered value (1);
  const Unregistered constValue (2);
  ASSERT_THROW (process_fn (value), std::exception);
  ASSERT_THROW (process_fn (constValue), std::exception);
  ASSERT_THROW (process_fn (&value), std::exception);
  ASSERT_THROW (process_fn (&constValue), std::exception);
}

namespace {

Class <int>& returnRef ()
{
  static Class <int> value (1);
  return value;
}

const Class <int>& returnConstRef ()
{
  return returnRef ();
}

Class <int>* returnPtr ()
{
  return &returnRef ();
}

const Class <int>* returnConstPtr ()
{
  return &returnConstRef ();
}

Class <int> returnValue ()
{
  return Class <int> (2);
}

} // namespace

TEST_F(ClassTests, PassingUnregisteredClassFromLuaThrows)
{
  using Unregistered = Class <int>;

  luabridge::getGlobalNamespace (L)
    .addFunction ("returnRef", &returnRef)
    .addFunction ("returnConstRef", &returnConstRef)
    .addFunction ("returnPtr", &returnPtr)
    .addFunction ("returnConstPtr", &returnConstPtr)
    .addFunction ("returnValue", &returnValue);

  ASSERT_THROW (runLua ("result = returnRef ()"), std::exception);
  ASSERT_THROW (runLua ("result = returnConstRef ()"), std::exception);
  ASSERT_THROW (runLua ("result = returnPtr ()"), std::exception);
  ASSERT_THROW (runLua ("result = returnConstPtr ()"), std::exception);
  ASSERT_THROW (runLua ("result = returnValue ()"), std::exception);
}

TEST_F (ClassTests, DeriveFromUnregisteredClassThrows)
{
  using Base = Class <int>;
  using Derived = Class <float, Base>;

  ASSERT_THROW (
    (luabridge::getGlobalNamespace (L).deriveClass <Derived, Base> ("Derived")),
    std::exception);

  luabridge::debug::dumpState (L, std::cout);
  ASSERT_EQ (1, lua_gettop (L));
}

TEST_F (ClassTests, SimpleFunctions)
{
  using Int = Class <int>;
  using AnyClass = Class <luabridge::LuaRef>;

  luabridge::getGlobalNamespace (L)
    .beginClass <Int> ("Int")
    .addConstructor <void (*) (int)> ()
    .addFunction ("theMethod", &Int::method)
    .addFunction ("constMethod", &Int::constMethod)
    .endClass ();

  runLua ("object = Int (501)");

  runLua ("result = object:theMethod (3)");
  ASSERT_EQ (3, result ().cast <int> ());

  runLua ("result = object:constMethod (5)");
  ASSERT_EQ (5, result ().cast <int> ());
}

TEST_F (ClassTests, ObjectConstness)
{
  using Int = Class <int>;

  luabridge::getGlobalNamespace (L)
    .beginClass <Int> ("Int")
    .addFunction ("theMethod", &Int::method)
    .addFunction ("constMethod", &Int::constMethod)
    .endClass ()
    .addFunction ("returnRef", &returnRef)
    .addFunction ("returnConstRef", &returnConstRef)
    .addFunction ("returnPtr", &returnPtr)
    .addFunction ("returnConstPtr", &returnConstPtr)
    .addFunction ("returnValue", &returnValue);

  runLua ("result = returnRef ():constMethod (10)");
  ASSERT_EQ (10, result ().cast <int> ());

  runLua ("result = returnRef ():theMethod (11)");
  ASSERT_EQ (11, result ().cast <int> ());

  runLua ("result = returnConstRef ():constMethod (12)");
  ASSERT_EQ (12, result ().cast <int> ());

  runLua ("result = returnConstRef ().theMethod"); // Don't call, just get
  ASSERT_TRUE (result ().isNil ());
}

TEST_F (ClassTests, Data)
{
  using Int = Class <int>;

  luabridge::getGlobalNamespace (L)
    .beginClass <Int> ("Int")
    .addConstructor <void (*) (int)> ()
    .addData ("data", &Int::data, true)
    .endClass ();

  runLua ("result = Int (501)");
  ASSERT_TRUE (result () ["data"].isNumber ());
  ASSERT_EQ (501, result () ["data"].cast <int> ());

  runLua ("result.data = 2");
  ASSERT_TRUE (result () ["data"].isNumber ());
  ASSERT_EQ (2, result () ["data"].cast <int> ());

  runLua ("result = Int (42).data");
  ASSERT_TRUE (result ().isNumber ());
  ASSERT_EQ (42, result ().cast <int> ());
}

TEST_F (ClassTests, Properties)
{
  using Int = Class <int>;

  luabridge::getGlobalNamespace (L)
    .beginClass <Int> ("Int")
    .addConstructor <void (*) (int)> ()
    .addProperty ("data", &Int::getData, &Int::setData)
    .endClass ();

  runLua ("result = Int (501)");
  ASSERT_TRUE (result () ["data"].isNumber ());
  ASSERT_EQ (501, result () ["data"].cast <int> ());

  runLua ("result.data = -2");
  ASSERT_TRUE (result () ["data"].isNumber ());
  ASSERT_EQ (-2, result () ["data"].cast <int> ());
}

TEST_F (ClassTests, ReadOnlyProperties)
{
  using Int = Class <int>;

  luabridge::getGlobalNamespace (L)
    .beginClass <Int> ("Int")
    .addConstructor <void (*) (int)> ()
    .addProperty ("data", &Int::getData)
    .endClass ();

  runLua ("result = Int (501)");
  ASSERT_TRUE (result () ["data"].isNumber ());
  ASSERT_EQ (501, result () ["data"].cast <int> ());

  ASSERT_THROW (runLua ("result.data = -2"), std::exception);
  ASSERT_EQ (501, result () ["data"].cast <int> ());
}

TEST_F (ClassTests, DerivedProperties)
{
  using Base = Class <std::string>;
  using Derived = Class <int, Base>;

  luabridge::getGlobalNamespace (L)
    .beginClass <Base> ("Base")
    .addProperty ("data", &Base::getData, &Base::setData)
    .endClass ()
    .deriveClass <Derived, Base> ("Derived")
    .endClass ();

  Derived derived (12);
  derived.Base::data = "abc";
  luabridge::setGlobal (L, &derived, "derived");

  runLua ("result = derived.data");
  ASSERT_TRUE (result ().isString ());
  ASSERT_EQ ("abc", result ().cast <std::string> ());

  runLua ("derived.data = 5"); // Lua just casts integer to string
  ASSERT_EQ ("5", derived.Base::data);
  ASSERT_EQ (12, derived.data);

  runLua ("derived.data = '123'");
  ASSERT_EQ ("123", derived.Base::data);
  ASSERT_EQ (12, derived.data);
}

TEST_F (ClassTests, OverriddenProperties)
{
  using Base = Class <float>;
  using Derived = Class <int, Base>;

  luabridge::getGlobalNamespace (L)
    .beginClass <Base> ("Base")
    .addProperty ("data", &Base::getData, &Base::setData)
    .endClass ()
    .deriveClass <Derived, Base> ("Derived")
    .addProperty ("data", &Derived::getData, &Derived::setData)
    .endClass ();

  Derived derived (50);
  derived.Base::data = 1.23f;
  luabridge::setGlobal (L, static_cast <Base*> (&derived), "base");
  luabridge::setGlobal (L, &derived, "derived");

  runLua ("result = base.data");
  ASSERT_TRUE (result ().isNumber ());
  ASSERT_EQ (1.23f, result ().cast <float> ());

  runLua ("result = derived.data");
  ASSERT_TRUE (result ().isNumber ());
  ASSERT_EQ (50, result ().cast <int> ());
  
  runLua ("base.data = -3.14");
  ASSERT_EQ (-3.14f, derived.Base::data);
  ASSERT_EQ (50, derived.data);

  runLua ("derived.data = 7");
  ASSERT_EQ (-3.14f, derived.Base::data);
  ASSERT_EQ (7, derived.data);
}

TEST_F (ClassTests, StaticFunctions)
{
  using Int = Class <int>;

  luabridge::getGlobalNamespace (L)
    .beginClass <Int> ("Int")
    .addConstructor <void (*) (int)> ()
    .addStaticFunction ("static", &Int::staticFunction)
    .endClass ();

  runLua ("result = Int.static (Int (35))");
  ASSERT_EQ (35, result ().cast <Int> ().data);
}

TEST_F (ClassTests, DerivedStaticFunctions)
{
  using Base = Class <std::string>;
  using Derived = Class <int, Base>;

  luabridge::getGlobalNamespace (L)
    .beginClass <Base> ("Base")
    .addConstructor <void (*) (std::string)> ()
    .addStaticFunction ("static", &Base::staticFunction)
    .endClass ()
    .deriveClass <Derived, Base> ("Derived")
    .endClass ();

  runLua ("result = Derived.static (Base ('abc'))");
  ASSERT_EQ ("abc", result ().cast <Base> ().data);
}

TEST_F (ClassTests, OverriddenStaticFunctions)
{
  using Base = Class <std::string>;
  using Derived = Class <int, Base>;

  luabridge::getGlobalNamespace (L)
    .beginClass <Base> ("Base")
    .addConstructor <void (*) (std::string)> ()
    .addStaticFunction ("staticFunction", &Base::staticFunction)
    .endClass ()
    .deriveClass <Derived, Base> ("Derived")
    .addConstructor <void (*) (int)> ()
    .addStaticFunction ("staticFunction", &Derived::staticFunction)
    .endClass ();

  runLua ("result = Base.staticFunction (Base ('abc'))");
  ASSERT_EQ ("abc", result ().cast <Base> ().data);

  runLua ("result = Derived.staticFunction (Derived (123))");
  ASSERT_EQ (123, result ().cast <Derived> ().data);
}

TEST_F (ClassTests, StaticData)
{
  using Int = Class <int>;

  luabridge::getGlobalNamespace (L)
    .beginClass <Int> ("Int")
    .addStaticData ("staticData", &Int::staticData, true)
    .endClass ();

  Int::staticData = 10;

  runLua ("result = Int.staticData");
  ASSERT_TRUE (result ().isNumber ());
  ASSERT_EQ (10, result ().cast <int> ());

  runLua ("Int.staticData = 20");
  ASSERT_EQ (20, Int::staticData);
}

TEST_F (ClassTests, ReadOnlyStaticData)
{
  using Int = Class <int>;

  luabridge::getGlobalNamespace (L)
    .beginClass <Int> ("Int")
    .addStaticData ("staticData", &Int::staticData, false)
    .endClass ();

  Int::staticData = 10;

  runLua ("result = Int.staticData");
  ASSERT_TRUE (result ().isNumber ());
  ASSERT_EQ (10, result ().cast <int> ());

  ASSERT_THROW (runLua ("Int.staticData = 20"), std::exception);
  ASSERT_EQ (10, Int::staticData);
}

TEST_F (ClassTests, DerivedStaticData)
{
  using Base = Class <float>;
  using Derived = Class <int, Base>;

  luabridge::getGlobalNamespace (L)
    .beginClass <Base> ("Base")
    .addStaticData ("staticData", &Base::staticData, true)
    .endClass ()
    .deriveClass <Derived, Base> ("Derived")
    .endClass ();

  Base::staticData = 1.23f;
  Derived::staticData = 50;

  runLua ("result = Derived.staticData");
  ASSERT_TRUE (result ().isNumber ());
  ASSERT_EQ (1.23f, result ().cast <float> ());

  runLua ("Derived.staticData = -3.14");
  ASSERT_EQ (-3.14f, Base::staticData);
  ASSERT_EQ (50, Derived::staticData);
}

TEST_F (ClassTests, OverriddenStaticData)
{
  using Base = Class <float>;
  using Derived = Class <int, Base>;

  luabridge::getGlobalNamespace (L)
    .beginClass <Base> ("Base")
    .addStaticData ("staticData", &Base::staticData, true)
    .endClass ()
    .deriveClass <Derived, Base> ("Derived")
    .addStaticData("staticData", &Derived::staticData, true)
    .endClass ();

  Base::staticData = 1.23f;
  Derived::staticData = 50;

  runLua ("result = Base.staticData");
  ASSERT_TRUE (result ().isNumber ());
  ASSERT_EQ (1.23f, result ().cast <float> ());

  runLua ("result = Derived.staticData");
  ASSERT_TRUE (result ().isNumber ());
  ASSERT_EQ (50, result ().cast <int> ());
  
  runLua ("Base.staticData = -3.14");
  ASSERT_EQ (-3.14f, Base::staticData);
  ASSERT_EQ (50, Derived::staticData);

  runLua ("Derived.staticData = 7");
  ASSERT_EQ (-3.14f, Base::staticData);
  ASSERT_EQ (7, Derived::staticData);
}

TEST_F (ClassTests, Metamethod__call)
{
  typedef Class <int> Int;

  luabridge::getGlobalNamespace (L)
    .beginClass <Int> ("Int")
    .addConstructor <void (*) (int)> ()
    .addFunction ("__call", &Int::operator())
    .endClass ();

  runLua ("result = Int (1) (-1)");
  ASSERT_TRUE (result ().isUserdata ());
  ASSERT_EQ (-1, result ().cast <Int> ().data);

  runLua ("result = Int (2) (5)");
  ASSERT_TRUE (result ().isUserdata ());
  ASSERT_EQ (5, result ().cast <Int> ().data);
}

TEST_F (ClassTests, Metamethod__tostring)
{
  typedef Class <int> Int;
  typedef Class <std::string> StringClass;

  luabridge::getGlobalNamespace (L)
    .beginClass <Int> ("Int")
    .addConstructor <void (*) (int)> ()
    .addFunction ("__tostring", &Int::toString)
    .endClass ()
    .beginClass <StringClass> ("String")
    .addConstructor <void (*) (std::string)> ()
    .addFunction ("__tostring", &StringClass::toString)
    .endClass ();

  Int intValue (-123);
  StringClass strValue ("abc");

  runLua ("result = tostring (Int (-123))");
  ASSERT_EQ ("-123", result ().cast <std::string> ());

#if LUA_VERSION_NUM >= 502
  // Lua 5.1 string.format doesn't use __tostring
  runLua ("result = string.format ('%s%s', String ('abc'), Int (-123))");
  ASSERT_EQ ("abc-123", result ().cast <std::string> ());
#endif
}

TEST_F (ClassTests, Metamethod__eq)
{
  typedef Class <int> Int;

  luabridge::getGlobalNamespace (L)
    .beginClass <Int> ("Int")
    .addConstructor <void (*) (int)> ()
    .addFunction ("__eq", &Int::operator==)
    .endClass ();

  runLua ("result = Int (1) == Int (1)");
  ASSERT_EQ (true, result ().cast <bool> ());

  runLua ("result = Int (1) ~= Int (1)");
  ASSERT_EQ (false, result ().cast <bool> ());

  runLua ("result = Int (1) == Int (2)");
  ASSERT_EQ (false, result ().cast <bool> ());

  runLua ("result = Int (1) ~= Int (2)");
  ASSERT_EQ (true, result ().cast <bool> ());
}

TEST_F (ClassTests, Metamethod__lt)
{
  typedef Class <int> Int;

  luabridge::getGlobalNamespace (L)
    .beginClass <Int> ("Int")
    .addConstructor <void (*) (int)> ()
    .addFunction ("__lt", &Int::operator<)
    .endClass ();

  runLua ("result = Int (1) < Int (1)");
  ASSERT_EQ (false, result ().cast <bool> ());

  runLua ("result = Int (1) < Int (2)");
  ASSERT_EQ (true, result ().cast <bool> ());

  runLua ("result = Int (2) < Int (1)");
  ASSERT_EQ (false, result ().cast <bool> ());
}

TEST_F (ClassTests, Metamethod__le)
{
  typedef Class <int> Int;

  luabridge::getGlobalNamespace (L)
    .beginClass <Int> ("Int")
    .addConstructor <void (*) (int)> ()
    .addFunction ("__le", &Int::operator<=)
    .endClass ();

  runLua ("result = Int (1) <= Int (1)");
  ASSERT_EQ (true, result ().cast <bool> ());

  runLua ("result = Int (1) <= Int (2)");
  ASSERT_EQ (true, result ().cast <bool> ());

  runLua ("result = Int (2) <= Int (1)");
  ASSERT_EQ (false, result ().cast <bool> ());
}

TEST_F (ClassTests, Metamethod__add)
{
  typedef Class <int> Int;

  luabridge::getGlobalNamespace (L)
    .beginClass <Int> ("Int")
    .addConstructor <void (*) (int)> ()
    .addFunction ("__add", &Int::operator+)
    .endClass ();

  runLua ("result = Int (1) + Int (2)");
  ASSERT_TRUE (result ().isUserdata ());
  ASSERT_EQ (3, result ().cast <Int> ().data);
}

TEST_F (ClassTests, Metamethod__sub)
{
  typedef Class <int> Int;

  luabridge::getGlobalNamespace (L)
    .beginClass <Int> ("Int")
    .addConstructor <void (*) (int)> ()
    .addFunction ("__sub", &Int::operator-)
    .endClass ();

  runLua ("result = Int (1) - Int (2)");
  ASSERT_TRUE (result ().isUserdata ());
  ASSERT_EQ (-1, result ().cast <Int> ().data);
}

TEST_F(ClassTests, Metamethod__mul)
{
  typedef Class <int> Int;

  luabridge::getGlobalNamespace (L)
    .beginClass <Int> ("Int")
    .addConstructor <void (*) (int)> ()
    .addFunction ("__mul", &Int::operator*)
    .endClass ();

  runLua ("result = Int (-2) * Int (-5)");
  ASSERT_TRUE (result ().isUserdata ());
  ASSERT_EQ (10, result ().cast <Int> ().data);
}

TEST_F (ClassTests, Metamethod__div)
{
  typedef Class <int> Int;

  luabridge::getGlobalNamespace (L)
    .beginClass <Int> ("Int")
    .addConstructor <void (*) (int)> ()
    .addFunction ("__div", &Int::operator/)
    .endClass ();

  runLua ("result = Int (10) / Int (2)");
  ASSERT_TRUE (result ().isUserdata ());
  ASSERT_EQ (5, result ().cast <Int> ().data);
}

TEST_F (ClassTests, Metamethod__mod)
{
  typedef Class <int> Int;

  luabridge::getGlobalNamespace(L)
    .beginClass <Int>("Int")
    .addConstructor <void(*) (int)>()
    .addFunction("__mod", &Int::operator%)
    .endClass();

  runLua("result = Int (7) % Int (2)");
  ASSERT_TRUE(result().isUserdata());
  ASSERT_EQ(1, result().cast <Int>().data);
}

TEST_F (ClassTests, Metamethod__pow)
{
  typedef Class <int> Int;

  luabridge::getGlobalNamespace (L)
    .beginClass <Int> ("Int")
    .addConstructor <void (*) (int)> ()
    .addFunction ("__pow", &Int::operator-)
    .endClass ();

  runLua ("result = Int (5) ^ Int (2)");
  ASSERT_TRUE (result ().isUserdata ());
  ASSERT_EQ (3, result ().cast <Int> ().data);
}

TEST_F (ClassTests, Metamethod__unm)
{
  typedef Class <int> Int;

  luabridge::getGlobalNamespace (L)
    .beginClass <Int> ("Int")
    .addConstructor <void (*) (int)> ()
    .addFunction ("__unm", &Int::negate)
    .endClass ();

  runLua ("result = -Int (-3)");
  ASSERT_TRUE (result ().isUserdata ());
  ASSERT_EQ (3, result ().cast <Int> ().data);
}

TEST_F (ClassTests, Metamethod__concat)
{
  typedef Class <std::string> String;

  luabridge::getGlobalNamespace (L)
    .beginClass <String> ("String")
    .addConstructor <void (*) (std::string)> ()
    .addFunction ("__concat", &String::operator+)
    .endClass ();

  ASSERT_THROW (runLua ("result = String ('a') + String ('b')"), std::exception);

  runLua ("result = String ('ab') .. String ('cd')");
  ASSERT_TRUE (result ().isUserdata ());
  ASSERT_EQ ("abcd", result ().cast <String> ().data);
}

TEST_F (ClassTests, Metamethod__len)
{
  typedef Class <int> Int;

  luabridge::getGlobalNamespace (L)
    .beginClass <Int> ("Int")
    .addConstructor <void (*) (int)> ()
    .addFunction ("__len", &Int::len)
    .endClass ();

  runLua ("result = #Int (1)");
  ASSERT_TRUE (result ().isNumber ());
  ASSERT_EQ (1, result ().cast <int> ());

  runLua("result = #Int (5)");
  ASSERT_TRUE (result ().isNumber ());
  ASSERT_EQ (5, result ().cast <int> ());
}

namespace {

struct Table
{
  int index (const std::string& key)
  {
    return map.at (key);
  }

  void newIndex(const std::string& key, int value)
  {
    map.emplace (key, value);
  }

  std::map <std::string, int> map;
};

} // namespace

TEST_F (ClassTests, Metamethod__index)
{
  luabridge::getGlobalNamespace (L)
    .beginClass <Table> ("Table")
    .addFunction ("__index", &Table::index)
    .endClass ();

  Table t {{{"a", 1}, {"b", 2}}};

  luabridge::setGlobal (L, &t, "t");

  runLua ("result = t.a");
  ASSERT_TRUE (result ().isNumber ());
  ASSERT_EQ (1, result ().cast <int> ());

  runLua ("result = t.b");
  ASSERT_TRUE (result ().isNumber ());
  ASSERT_EQ (2, result ().cast <int> ());

  ASSERT_THROW (runLua ("result = t.c"), std::exception); // at ("c") throws
}

TEST_F (ClassTests, Metamethod__newindex)
{
  typedef Class <int> Int;

  luabridge::getGlobalNamespace (L)
    .beginClass <Table> ("Table")
    .addFunction ("__newindex", &Table::newIndex)
    .endClass ();

  Table t;

  luabridge::setGlobal (L, &t, "t");

  runLua ("t.a = 1\n"
          "t ['b'] = 2");

  ASSERT_EQ ((std::map <std::string, int> {{"a", 1}, {"b", 2}}), t.map);
}

TEST_F (ClassTests, Metamethod__gcForbidden)
{
  typedef Class <int> Int;

  ASSERT_THROW (
    luabridge::getGlobalNamespace(L)
      .beginClass <Int> ("Int")
      .addFunction("__gc", &Int::method)
      .endClass(),
    std::exception);
}

TEST_F(ClassTests, EnclosedClassProperties)
{
  typedef Class <int> Inner;
  typedef Class <Inner> Outer;

  luabridge::getGlobalNamespace (L)
    .beginClass <Inner> ("Inner")
    .addData ("data", &Inner::data)
    .endClass ()
    .beginClass <Outer> ("Outer")
    .addData ("data", &Outer::data)
    .endClass ();

  Outer outer (Inner (0));
  luabridge::setGlobal (L, &outer, "outer");

  outer.data.data = 1;
  runLua ("outer.data.data = 10");
  ASSERT_EQ (10, outer.data.data);

  runLua ("result = outer.data.data");
  ASSERT_EQ (10, result ().cast <int> ());
}
