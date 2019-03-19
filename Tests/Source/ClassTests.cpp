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

template <class T>
struct Class
{
  Class (T data)
    : data (data)
  {
  }

  std::string toString () const
  {
    std::ostringstream stream;
    stream << data;
    return stream.str ();
  }

  bool operator== (const Class <T>& rhs) const
  {
    return data == rhs.data;
  }

  bool operator< (const Class <T>& rhs) const
  {
    return data < rhs.data;
  }

  bool operator<= (const Class <T>& rhs) const
  {
    return data <= rhs.data;
  }

  Class <T> operator+ (const Class <T>& rhs) const
  {
    return Class <T> (data + rhs.data);
  }

  Class <T> operator- (const Class <T>& rhs) const
  {
    return Class <T> (data - rhs.data);
  }

  Class <T> operator* (const Class <T>& rhs) const
  {
    return Class <T> (data * rhs.data);
  }

  Class <T> operator/ (const Class <T>& rhs) const
  {
    return Class <T> (data / rhs.data);
  }

  Class <T> operator% (const Class <T>& rhs) const
  {
    return Class <T> (data % rhs.data);
  }

  Class <T> operator() (T param)
  {
    return Class <T> (param);
  }

  int len () const
  {
    return data;
  }

  Class <T> negate () const
  {
    return Class <T> (-data);
  }

  T method (T value)
  {
    return value;
  }

  T constMethod (T value) const
  {
    return value;
  }

  static T getData (const Class* object)
  {
    return object->data;
  }

  static void setData (Class* object, T data)
  {
    object->data = data;
  }

  mutable T data;
};

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
  return &returnRef ();
}

Class <int> returnValue ()
{
  return returnRef ();
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

TEST_F (ClassTests, Data)
{
  using Int = Class <int>;
  using AnyClass = Class <luabridge::LuaRef>;

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

  luabridge::getGlobalNamespace (L)
    .beginNamespace ("ns")
    .beginClass <AnyClass> ("AnyClass")
    .addConstructor <void (*) (luabridge::LuaRef)> ()
    .addData ("data", &AnyClass::data)
    .endClass ()
    .endNamespace ();

  runLua ("result = ns.AnyClass ({a = 31})");
  ASSERT_TRUE (result () ["data"].isTable ());
  ASSERT_TRUE (result () ["data"] ["a"].isNumber ());
  ASSERT_EQ (31, result () ["data"] ["a"].cast <int> ());

  runLua ("result.data = true");
  ASSERT_TRUE (result () ["data"].isBool ());
  ASSERT_EQ (true, result () ["data"].cast <bool> ());
}

TEST_F (ClassTests, Properties)
{
  using Int = Class <int>;
  using AnyClass = Class <luabridge::LuaRef>;

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

  luabridge::getGlobalNamespace (L)
    .beginNamespace ("ns")
    .beginClass <AnyClass> ("AnyClass")
    .addConstructor <void (*) (luabridge::LuaRef)> ()
    .addProperty ("data", &AnyClass::getData, &AnyClass::setData)
    .endClass ()
    .endNamespace ();

  runLua ("result = ns.AnyClass ({a = 31})");
  ASSERT_TRUE (result () ["data"].isTable ());
  ASSERT_TRUE (result () ["data"] ["a"].isNumber ());
  ASSERT_EQ (31, result () ["data"] ["a"].cast <int> ());

  runLua ("result.data = 'abc'");
  ASSERT_TRUE (result () ["data"].isString ());
  ASSERT_EQ ("abc", result () ["data"].cast <std::string> ());
}

TEST_F (ClassTests, ReadOnlyProperties)
{
  using Int = Class <int>;
  using AnyClass = Class <luabridge::LuaRef>;

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

  luabridge::getGlobalNamespace (L)
    .beginNamespace ("ns")
    .beginClass <AnyClass> ("AnyClass")
    .addConstructor <void (*) (luabridge::LuaRef)> ()
    .addProperty ("data", &AnyClass::getData)
    .endClass ()
    .endNamespace ();

  runLua ("result = ns.AnyClass ({a = 31})");
  ASSERT_TRUE (result () ["data"].isTable ());
  ASSERT_TRUE (result () ["data"] ["a"].isNumber ());
  ASSERT_EQ (31, result () ["data"] ["a"].cast <int> ());

  ASSERT_THROW (runLua ("result.data = 'abc'"), std::exception);
  ASSERT_TRUE (result () ["data"].isTable ());
  ASSERT_TRUE (result () ["data"] ["a"].isNumber ());
  ASSERT_EQ (31, result () ["data"] ["a"].cast <int> ());
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

#if LUABRIDGEDEMO_LUA_VERSION != 501 // tostring () only
  runLua ("result = string.format ('%s', String ('abc'))");
  ASSERT_EQ ("abc", result ().cast <std::string> ());
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
      .beginClass <Int>("Int")
      .addFunction("__gc", &Int::method)
      .endClass(),
    std::exception);
}

TEST_F (ClassTests, DISABLED_ClassProperties2)
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
