// https://github.com/vinniefalco/LuaBridge
//
// Copyright 2018, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include <exception>


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

  T method (T value)
  {
    return value;
  }

  T constMethod (T value)
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
  using IntClass = Class <int>;
  using AnyClass = Class <luabridge::LuaRef>;

  luabridge::getGlobalNamespace (L)
    .beginClass <IntClass> ("IntClass")
    .addConstructor <void (*) (int)> ()
    .addData ("data", &IntClass::data, true)
    .endClass ();

  runLua ("result = IntClass (501)");
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
  using IntClass = Class <int>;
  using AnyClass = Class <luabridge::LuaRef>;

  luabridge::getGlobalNamespace (L)
    .beginClass <IntClass> ("IntClass")
    .addConstructor <void (*) (int)> ()
    .addProperty ("data", &IntClass::getData, &IntClass::setData)
    .endClass ();

  runLua ("result = IntClass (501)");
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
  using IntClass = Class <int>;
  using AnyClass = Class <luabridge::LuaRef>;

  luabridge::getGlobalNamespace (L)
    .beginClass <IntClass> ("IntClass")
    .addConstructor <void (*) (int)> ()
    .addProperty ("data", &IntClass::getData)
    .endClass ();

  runLua ("result = IntClass (501)");
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

TEST_F(ClassTests, DISABLED_ClassProperties2)
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
