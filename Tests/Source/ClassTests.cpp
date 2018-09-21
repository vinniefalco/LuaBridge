// https://github.com/vinniefalco/LuaBridge
//
// Copyright 2018, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"

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
struct DataClass
{
  DataClass (T data)
    : data (data)
  {
  }

  static T getData (const DataClass* object)
  {
    return object->data;
  }

  static void setData (DataClass* object, T data)
  {
    object->data = data;
  }

  mutable T data;
};

} // namespace

TEST_F (ClassTests, Data)
{
  using IntClass = DataClass <int>;
  using AnyClass = DataClass <luabridge::LuaRef>;

  luabridge::getGlobalNamespace (L)
    .beginClass <IntClass> ("IntClass")
    .addConstructor <void (*) (int)> ()
    .addData ("data", &IntClass::data)
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
  using IntClass = DataClass <int>;
  using AnyClass = DataClass <luabridge::LuaRef>;

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
  using IntClass = DataClass <int>;
  using AnyClass = DataClass <luabridge::LuaRef>;

  luabridge::getGlobalNamespace (L)
    .beginClass <IntClass> ("IntClass")
    .addConstructor <void (*) (int)> ()
    .addProperty ("data", &IntClass::getData)
    .endClass ();

  runLua ("result = IntClass (501)");
  ASSERT_TRUE (result () ["data"].isNumber ());
  ASSERT_EQ (501, result () ["data"].cast <int> ());

  ASSERT_THROW (runLua ("result.data = -2"), std::runtime_error);
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

  ASSERT_THROW (runLua ("result.data = 'abc'"), std::runtime_error);
  ASSERT_TRUE (result () ["data"].isTable ());
  ASSERT_TRUE (result () ["data"] ["a"].isNumber ());
  ASSERT_EQ (31, result () ["data"] ["a"].cast <int> ());
}
