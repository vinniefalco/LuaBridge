// https://github.com/vinniefalco/LuaBridge
// Copyright 2019, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"
#include "TestTypes.h"

#include "LuaBridge/Array.h"

#include <algorithm>
#include <array>

template<class T>
struct ArrayTest : TestBase
{
};

TYPED_TEST_CASE_P(ArrayTest);

TYPED_TEST_P(ArrayTest, LuaRef)
{
    using Traits = TypeTraits<TypeParam>;

    this->runLua("result = {" + Traits::list() + "}");

    std::array<TypeParam, 3> expected;
    std::copy_n(Traits::values().begin(), 3, expected.begin());

    std::array<TypeParam, 3> actual = this->result();
    ASSERT_EQ(expected, actual);
}

REGISTER_TYPED_TEST_CASE_P(ArrayTest, LuaRef);

INSTANTIATE_TYPED_TEST_CASE_P(ArrayTest, ArrayTest, TestTypes);

namespace {

struct Data
{
    /* explicit */ Data(int i = -1000) : i(i) {}

    int i;
};

bool operator==(const Data& lhs, const Data& rhs)
{
    return lhs.i == rhs.i;
}

std::ostream& operator<<(std::ostream& lhs, const Data& rhs)
{
    lhs << "{" << rhs.i << "}";
    return lhs;
}

template<std::size_t S>
std::array<Data, S> processValues(const std::array<Data, S>& data)
{
    return data;
}

template<std::size_t S>
std::array<Data, S> processPointers(const std::array<const Data*, S>& data)
{
    std::array<Data, S> result;
    std::size_t arrayIndex = 0;
    for (const auto* item : data)
    {
        result[arrayIndex] = *item;
        ++arrayIndex;
    }
    return result;
}

} // namespace

struct ArrayTests : TestBase
{
};

TEST_F(ArrayTests, PassFromLua)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<Data>("Data")
        .addConstructor<void (*)(int)>()
        .endClass()
        .addFunction("processValues", &processValues<3>)
        .addFunction("processPointers", &processPointers<3>);

    resetResult();
    runLua("result = processValues ({Data (-1), Data (2), Data (5)})");

    ASSERT_EQ((std::array<Data, 3>({-1, 2, 5})), (result<std::array<Data, 3>>()));

    resetResult();
    runLua("result = processPointers ({Data (-3), Data (4), Data (9)})");

    ASSERT_EQ((std::array<Data, 3>({-3, 4, 9})), (result<std::array<Data, 3>>()));
}
