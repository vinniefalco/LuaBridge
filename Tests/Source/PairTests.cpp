// https://github.com/vinniefalco/LuaBridge
// Copyright 2022, Stefan Frings
// SPDX-License-Identifier: MIT

#include "TestBase.h"
#include "TestTypes.h"

#include <cassert>

#include "LuaBridge/Pair.h"

template<class T>
struct PairTest : TestBase
{
};

TYPED_TEST_CASE_P(PairTest);

template<typename T>
std::string toLuaSrcString(T const& value)
{
    return std::to_string(value);
}

template<>
std::string toLuaSrcString(bool const& value)
{
    return value ? "true" : "false";
}

template<>
std::string toLuaSrcString(std::string const& value)
{
    return "'" + value + "'";
}

TYPED_TEST_P(PairTest, push)
{
    using Pair = TypeParam;
    using T1 = typename Pair::first_type;
    using T2 = typename Pair::second_type;
    using Traits1 = TypeTraits<T1>;
    using Traits2 = TypeTraits<T2>;

    assert(Traits1::values().size() == Traits2::values().size());

    for (std::size_t n = 0; n < Traits1::values().size(); ++n)
    {
        T1 const value1 = Traits1::values()[n];
        T2 const value2 = Traits2::values()[n];

        Pair const data(value1, value2);

        this->runLua("result = nil; function func(data) result = data end");

        luabridge::LuaRef func = luabridge::getGlobal(this->L, "func");
        func(data);

        Pair const actual = this->result();
        ASSERT_EQ(actual, data);
    }
}

TYPED_TEST_P(PairTest, get)
{
    using Pair = TypeParam;
    using T1 = typename Pair::first_type;
    using T2 = typename Pair::second_type;
    using Traits1 = TypeTraits<T1>;
    using Traits2 = TypeTraits<T2>;

    assert(Traits1::values().size() == Traits2::values().size());

    for (std::size_t n = 0; n < Traits1::values().size(); ++n)
    {
        T1 const value1 = Traits1::values()[n];
        T2 const value2 = Traits2::values()[n];

        std::string const luaSrc =
            "result = {" + toLuaSrcString(value1) + ", " + toLuaSrcString(value2) + "}";

        SCOPED_TRACE(luaSrc);
        this->runLua(luaSrc);

        Pair const actual = this->result();
        ASSERT_EQ(actual.first, value1);
        ASSERT_EQ(actual.second, value2);
    }
}

TYPED_TEST_P(PairTest, isInstance)
{
    using Pair = TypeParam;
    using T1 = typename Pair::first_type;
    using T2 = typename Pair::second_type;
    using Traits1 = TypeTraits<T1>;
    using Traits2 = TypeTraits<T2>;

    assert(Traits1::values().size() == Traits2::values().size());

    for (std::size_t n = 0; n < Traits1::values().size(); ++n)
    {
        T1 const value1 = Traits1::values()[n];
        T2 const value2 = Traits2::values()[n];

        std::string const luaSrc =
            "result = {" + toLuaSrcString(value1) + ", " + toLuaSrcString(value2) + "}";

        SCOPED_TRACE(luaSrc);
        this->runLua(luaSrc);

        luabridge::LuaRef ref = luabridge::getGlobal(this->L, "result");
        ASSERT_TRUE(ref.isInstance<Pair>());
        ASSERT_FALSE(ref.isInstance<T1>());
        ASSERT_FALSE(ref.isInstance<T2>());
    }
}

REGISTER_TYPED_TEST_CASE_P(PairTest, push, get, isInstance);

using TestTypesCartesianProduct = ::testing::Types<std::pair<bool, bool>,
                                                   std::pair<bool, int>,
                                                   std::pair<bool, long>,
                                                   std::pair<bool, float>,
                                                   std::pair<bool, double>,
                                                   std::pair<bool, std::string>,
                                                   std::pair<int, bool>,
                                                   std::pair<int, int>,
                                                   std::pair<int, long>,
                                                   std::pair<int, float>,
                                                   std::pair<int, double>,
                                                   std::pair<int, std::string>,
                                                   std::pair<long, bool>,
                                                   std::pair<long, int>,
                                                   std::pair<long, long>,
                                                   std::pair<long, float>,
                                                   std::pair<long, double>,
                                                   std::pair<long, std::string>,
                                                   std::pair<float, bool>,
                                                   std::pair<float, int>,
                                                   std::pair<float, long>,
                                                   std::pair<float, float>,
                                                   std::pair<float, double>,
                                                   std::pair<float, std::string>,
                                                   std::pair<double, bool>,
                                                   std::pair<double, int>,
                                                   std::pair<double, long>,
                                                   std::pair<double, float>,
                                                   std::pair<double, double>,
                                                   std::pair<double, std::string>,
                                                   std::pair<std::string, bool>,
                                                   std::pair<std::string, int>,
                                                   std::pair<std::string, long>,
                                                   std::pair<std::string, float>,
                                                   std::pair<std::string, double>,
                                                   std::pair<std::string, std::string>>;

INSTANTIATE_TYPED_TEST_CASE_P(PairTest, PairTest, TestTypesCartesianProduct);
