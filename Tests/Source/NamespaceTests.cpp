// https://github.com/vinniefalco/LuaBridge
//
// Copyright 2018, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"

struct NamespaceTests : TestBase
{
  template <class T>
  T variable (const std::string& name)
  {
    runLua ("result = " + name);
    return result ().cast <T>();
  }
};

TEST_F (NamespaceTests, Variables)
{
  int int_ = -10;
  auto any = luabridge::newTable (L);
  any ["a"] = 1;

  ASSERT_THROW (
    luabridge::getGlobalNamespace (L).addVariable ("int", &int_),
    std::logic_error);

  runLua ("result = int");
  ASSERT_TRUE (result ().isNil ());

  luabridge::getGlobalNamespace (L)
    .beginNamespace ("ns")
    .addVariable ("int", &int_)
    .addVariable ("any", &any)
    .endNamespace ();

  ASSERT_EQ (-10, variable <int> ("ns.int"));
  ASSERT_EQ (any, variable <luabridge::LuaRef> ("ns.any"));

  runLua ("ns.int = -20");
  ASSERT_EQ (-20, int_);

  runLua ("ns.any = {b = 2}");
  ASSERT_TRUE (any.isTable ());
  ASSERT_TRUE (any ["b"].isNumber ());
  ASSERT_EQ (2, any ["b"].cast <int> ());
}

TEST_F (NamespaceTests, ReadOnlyVariables)
{
  int int_ = -10;
  auto any = luabridge::newTable (L);
  any["a"] = 1;

  ASSERT_THROW (
    luabridge::getGlobalNamespace (L).addVariable ("int", &int_),
    std::logic_error);

  runLua ("result = int");
  ASSERT_TRUE (result ().isNil ());

  luabridge::getGlobalNamespace (L)
    .beginNamespace ("ns")
    .addVariable ("int", &int_, false)
    .addVariable ("any", &any, false)
    .endNamespace ();

  ASSERT_EQ (-10, variable <int> ("ns.int"));
  ASSERT_EQ (any, variable <luabridge::LuaRef> ("ns.any"));

  ASSERT_THROW (runLua ("ns.int = -20"), std::runtime_error);
  ASSERT_EQ (-10, variable <int> ("ns.int"));

  ASSERT_THROW (runLua ("ns.any = {b = 2}"), std::runtime_error);
  ASSERT_EQ (any, variable <luabridge::LuaRef> ("ns.any"));
}

namespace {

template <class T>
struct Property
{
  static T value;
};

template <class T>
T Property <T>::value;

template <class T>
void setProperty (const T& value)
{
  Property <T>::value = value;
}

template <class T>
const T& getProperty ()
{
  return Property <T>::value;
}

} // namespace

TEST_F (NamespaceTests, Properties)
{
  setProperty <int> (-10);

  ASSERT_THROW (
    luabridge::getGlobalNamespace (L)
      .addProperty ("int", &getProperty <int>, &setProperty <int>),
    std::logic_error);

  runLua ("result = int");
  ASSERT_TRUE (result ().isNil ());

  luabridge::getGlobalNamespace (L)
    .beginNamespace ("ns")
      .addProperty ("int", &getProperty <int>, &setProperty <int>)
    .endNamespace ();

  ASSERT_EQ (-10, variable <int> ("ns.int"));

  runLua ("ns.int = -20");
  ASSERT_EQ (-20, getProperty <int> ());
}

TEST_F (NamespaceTests, ReadOnlyProperties)
{
  setProperty <int> (-10);

  ASSERT_THROW (
    luabridge::getGlobalNamespace (L)
      .addProperty ("int", &getProperty <int>),
    std::logic_error);

  runLua ("result = int");
  ASSERT_TRUE (result ().isNil ());

  luabridge::getGlobalNamespace (L)
    .beginNamespace ("ns")
      .addProperty ("int", &getProperty <int>)
    .endNamespace ();

  ASSERT_EQ (-10, variable <int> ("ns.int"));

  ASSERT_THROW (
    runLua ("ns.int = -20"),
    std::runtime_error);
  ASSERT_EQ (-10, getProperty <int> ());
}

#if defined(_WINDOWS) || defined(WIN32)

namespace {

int __stdcall StdCall (int i)
{
  return i + 10;
}

} // namespace

TEST_F (NamespaceTests, StdCallFunctions)
{
  luabridge::getGlobalNamespace (L)
    .addFunction ("StdCall", &StdCall);

  runLua ("result = StdCall (2)");
  ASSERT_TRUE (result ().isNumber ());
  ASSERT_EQ (12, result ().cast <int> ());
}

#endif // _WINDOWS || WIN32
