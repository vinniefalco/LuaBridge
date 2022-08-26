// https://github.com/vinniefalco/LuaBridge
// Copyright 2020, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"

struct StackTests : TestBase
{
};

#ifdef LUABRIDGE_CXX17
#define CXX17(EXPR) EXPR
#else
#define CXX17(EXPR)
#endif

#if LUA_VERSION_NUM <= 502
#define LUA_501_502(EXPR) EXPR
#else
#define LUA_501_502(EXPR)
#endif

#if LUA_VERSION_NUM >= 503
#define LUA_503_504(EXPR) EXPR
#else
#define LUA_503_504(EXPR)
#endif

TEST_F(StackTests, Nil)
{
    luabridge::push(L, luabridge::Nil());

    ASSERT_TRUE(luabridge::isInstance<luabridge::Nil>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<bool>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<int>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<unsigned>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<float>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<double>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<std::string>(L, -1));
    CXX17(ASSERT_FALSE(luabridge::isInstance<std::string_view>(L, -1)));
}

TEST_F(StackTests, Bool)
{
    luabridge::push(L, true);

    ASSERT_FALSE(luabridge::isInstance<luabridge::Nil>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<luabridge::Nil>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<bool>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<int>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<unsigned>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<float>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<double>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<const char*>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<std::string>(L, -1));
    CXX17(ASSERT_FALSE(luabridge::isInstance<std::string_view>(L, -1)));

    ASSERT_EQ(true, luabridge::get<bool>(L, -1));
}

TEST_F(StackTests, Int)
{
    luabridge::push(L, 5);

    ASSERT_FALSE(luabridge::isInstance<luabridge::Nil>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<bool>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<int>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<unsigned>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<float>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<double>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<const char*>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<std::string>(L, -1));
    CXX17(ASSERT_FALSE(luabridge::isInstance<std::string_view>(L, -1)));

    ASSERT_EQ(5, luabridge::get<int>(L, -1));
    ASSERT_NEAR(5.f, luabridge::get<float>(L, -1), 1e-5);
    ASSERT_NEAR(5.0, luabridge::get<double>(L, -1), 1e-6);
}

TEST_F(StackTests, Float)
{
    luabridge::push(L, 3.14f);

    ASSERT_FALSE(luabridge::isInstance<luabridge::Nil>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<bool>(L, -1));
    LUA_501_502(ASSERT_TRUE(luabridge::isInstance<int>(L, -1)));
    LUA_503_504(ASSERT_FALSE(luabridge::isInstance<int>(L, -1)));
    LUA_501_502(ASSERT_TRUE(luabridge::isInstance<unsigned>(L, -1)));
    LUA_503_504(ASSERT_FALSE(luabridge::isInstance<unsigned>(L, -1)));
    ASSERT_TRUE(luabridge::isInstance<float>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<double>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<const char*>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<std::string>(L, -1));
    CXX17(ASSERT_FALSE(luabridge::isInstance<std::string_view>(L, -1)));

    LUA_501_502(ASSERT_EQ(3, luabridge::get<int>(L, -1)));
    LUA_501_502(ASSERT_EQ(3, luabridge::get<unsigned>(L, -1)));
    ASSERT_NEAR(3.14f, luabridge::get<float>(L, -1), 1e-5);
    ASSERT_NEAR(3.14, luabridge::get<double>(L, -1), 1e-6);
}

TEST_F(StackTests, CString)
{
    luabridge::push(L, "abc");

    ASSERT_FALSE(luabridge::isInstance<luabridge::Nil>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<bool>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<int>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<unsigned>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<float>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<double>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<const char*>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<std::string>(L, -1));
    CXX17(ASSERT_TRUE(luabridge::isInstance<std::string_view>(L, -1)));

    ASSERT_STREQ("abc", luabridge::get<const char*>(L, -1));
    ASSERT_EQ("abc", luabridge::get<std::string>(L, -1));
    CXX17(ASSERT_EQ("abc", luabridge::get<std::string_view>(L, -1)));
}

TEST_F(StackTests, StdString)
{
    luabridge::push(L, std::string("abc"));

    ASSERT_FALSE(luabridge::isInstance<luabridge::Nil>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<bool>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<int>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<unsigned>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<float>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<double>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<const char*>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<std::string>(L, -1));
    CXX17(ASSERT_TRUE(luabridge::isInstance<std::string_view>(L, -1)));

    ASSERT_STREQ("abc", luabridge::get<const char*>(L, -1));
    ASSERT_EQ("abc", luabridge::get<std::string>(L, -1));
    CXX17(ASSERT_EQ("abc", luabridge::get<std::string_view>(L, -1)));
}

#ifdef LUABRIDGE_CXX17

TEST_F(StackTests, StdStringView)
{
    luabridge::push(L, std::string_view("abc"));

    ASSERT_FALSE(luabridge::isInstance<luabridge::Nil>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<bool>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<int>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<unsigned>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<float>(L, -1));
    ASSERT_FALSE(luabridge::isInstance<double>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<const char*>(L, -1));
    ASSERT_TRUE(luabridge::isInstance<std::string>(L, -1));
    CXX17(ASSERT_TRUE(luabridge::isInstance<std::string_view>(L, -1)));

    ASSERT_STREQ("abc", luabridge::get<const char*>(L, -1));
    ASSERT_EQ("abc", luabridge::get<std::string>(L, -1));
    CXX17(ASSERT_EQ("abc", luabridge::get<std::string_view>(L, -1)));
}

#endif // LUABRIDGE_CXX17
