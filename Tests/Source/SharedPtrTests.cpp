// https://github.com/vinniefalco/LuaBridge
//
// Copyright 2020, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include "LuaBridge/SharedPtr.h"

struct SharedPtrTests : TestBase
{
};


struct Object
{
  explicit Object(bool& deleted)
    : deleted (deleted)
  {
    deleted = false;
  }  

  ~Object()
  {
    deleted = true;
  }

  Object(Object&&) = delete;
  Object(const Object&) = delete;

  Object& operator=(Object&&) = delete;
  Object& operator=(const Object&) = delete;

  bool isDeleted () const
  {
    return deleted;
  }

  bool& deleted;
};

TEST_F (SharedPtrTests, LastReferenceInLua)
{
  luabridge::getGlobalNamespace (L)
    .beginClass <Object> ("Class")
    .addProperty ("deleted", &Object::isDeleted)
    .endClass ();

  bool deleted = false;

  auto object = std::make_shared <Object> (deleted);

  luabridge::setGlobal (L, object, "object");
  runLua("result = object.deleted");
  ASSERT_EQ (true, result ().isBool ());
  ASSERT_EQ (false, result <bool> ());

  object = nullptr;
  runLua("result = object.deleted");
  ASSERT_EQ(true, result ().isBool ());
  ASSERT_EQ(false, result <bool>());
  ASSERT_EQ(false, deleted);

  runLua ("object = nil");
  lua_gc (L, LUA_GCCOLLECT, 1);

  ASSERT_EQ (true, deleted);
}

TEST_F (SharedPtrTests, LastReferenceInCpp)
{
  luabridge::getGlobalNamespace (L)
    .beginClass <Object> ("Class")
    .addProperty ("deleted", &Object::isDeleted)
    .endClass ();

  bool deleted = false;

  auto object = std::make_shared <Object> (deleted);

  luabridge::setGlobal (L, object, "object");
  runLua("result = object.deleted");
  ASSERT_EQ (true, result ().isBool ());
  ASSERT_EQ (false, result <bool> ());

  runLua ("object = nil");
  lua_gc (L, LUA_GCCOLLECT, 1);
  ASSERT_EQ(false, deleted);

  object = nullptr;
  ASSERT_EQ (true, deleted);
}
