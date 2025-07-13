// https://github.com/vinniefalco/LuaBridge
// Copyright 2022, Stefan Frings
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include "LuaBridge/LuaBridge.h"

class TestClass
{
public:
    explicit inline TestClass(int i) : m_i(i) {}

    inline int get() const { return m_i; }

private:
    int m_i;
};

int testFunctionObject(TestClass object)
{
    return object.get();
}

int testFunctionObjectConst(const TestClass object)
{
    return object.get();
}

int testFunctionRef(TestClass& object)
{
    return object.get();
}

int testFunctionRefConst(const TestClass& object)
{
    return object.get();
}

struct UserDataTest : TestBase
{
    void SetUp() override
    {
        TestBase::SetUp();

        luabridge::getGlobalNamespace(L)
            .beginClass<TestClass>("TestClass")
            .addConstructor<void (*)(int)>()
            .endClass()
            .

            addFunction("testFunctionObject", testFunctionObject)
            .addFunction("testFunctionObjectConst", testFunctionObjectConst)
            .addFunction("testFunctionRef", testFunctionRef)
            .addFunction("testFunctionRefConst", testFunctionRefConst);
    }
};

TEST_F(UserDataTest, Object)
{
    runLua("object = TestClass(123)\n"
           "result = testFunctionObject(object)");

    ASSERT_EQ(result(), 123);
}

TEST_F(UserDataTest, ObjectConst)
{
    runLua("object = TestClass(123)\n"
           "result = testFunctionObjectConst(object)");

    ASSERT_EQ(result(), 123);
}

TEST_F(UserDataTest, Ref)
{
    runLua("object = TestClass(123)\n"
           "result = testFunctionRef(object)");

    ASSERT_EQ(result(), 123);
}

TEST_F(UserDataTest, RefConst)
{
    runLua("object = TestClass(123)\n"
           "result = testFunctionRefConst(object)");

    ASSERT_EQ(result(), 123);
}

TEST_F(UserDataTest, FailNumberObject)
{
    ASSERT_THROW(runLua("testFunctionObject(132)");, std::runtime_error);
}

TEST_F(UserDataTest, FailNumberObjectConst)
{
    ASSERT_THROW(runLua("testFunctionObjectConst(132)");, std::runtime_error);
}

TEST_F(UserDataTest, FailNumberRef)
{
    ASSERT_THROW(runLua("testFunctionRef(132)");, std::runtime_error);
}

TEST_F(UserDataTest, FailNumberRefConst)
{
    ASSERT_THROW(runLua("testFunctionRefConst(132)");, std::runtime_error);
}

TEST_F(UserDataTest, FailNilObject)
{
    ASSERT_THROW(runLua("testFunctionObject(nil)");, std::runtime_error);
}

TEST_F(UserDataTest, FailNilObjectConst)
{
    ASSERT_THROW(runLua("testFunctionObjectConst(nil)");, std::runtime_error);
}

TEST_F(UserDataTest, FailNilRef)
{
    ASSERT_THROW(runLua("testFunctionRef(nil)");, std::runtime_error);
}

TEST_F(UserDataTest, FailNilRefConst)
{
    ASSERT_THROW(runLua("testFunctionRefConst(nil)");, std::runtime_error);
}
