// https://github.com/vinniefalco/LuaBridge
// Copyright 2021, Stefan Frings
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include "LuaBridge/RefCountedPtr.h"

class RefCountedPtrTests : public ::testing::Test, private luabridge::detail::RefCountedPtrBase
{
public:
    void SetUp() override { getRefCounts().clear(); }

    void TearDown() override
    {
        for (const RefCountsType::value_type& refCountEntry : getRefCounts())
        {
            std::cout << "REMAINING ptr=" << refCountEntry.first
                      << ", count=" << refCountEntry.second << std::endl;
        }

        ASSERT_TRUE(getRefCounts().empty());
    }

    int getNumRefCounts() const { return getRefCounts().size(); }
};

TEST_F(RefCountedPtrTests, ConstructorDefault)
{
    const luabridge::RefCountedPtr<int> ptr;

    ASSERT_EQ(ptr.get(), nullptr);
    ASSERT_EQ(getNumRefCounts(), 0);
    ASSERT_EQ(ptr.use_count(), 0);
}

TEST_F(RefCountedPtrTests, ConstructorObject)
{
    int* const object = new int(123);
    const luabridge::RefCountedPtr<int> ptr(object);

    ASSERT_EQ(ptr.get(), object);
    ASSERT_EQ(getNumRefCounts(), 1);
    ASSERT_EQ(ptr.use_count(), 1);
}

TEST_F(RefCountedPtrTests, ConstructorCopy)
{
    int* const object = new int(123);
    const luabridge::RefCountedPtr<int> ptr(object);
    const luabridge::RefCountedPtr<int> ptrCopy(ptr);

    ASSERT_EQ(ptr.get(), object);
    ASSERT_EQ(ptrCopy.get(), object);
    ASSERT_EQ(getNumRefCounts(), 1);
    ASSERT_EQ(ptr.use_count(), 2);
    ASSERT_EQ(ptrCopy.use_count(), 2);
}

TEST_F(RefCountedPtrTests, ConstructorCopyPolymorph)
{
    struct Base
    {
    };
    struct Specialized : public Base
    {
    };
    Specialized* const object = new Specialized;

    const luabridge::RefCountedPtr<Specialized> ptr(object);
    const luabridge::RefCountedPtr<Base> ptrCopy(ptr);

    ASSERT_EQ(ptr.get(), object);
    ASSERT_EQ(ptrCopy.get(), object);
    ASSERT_EQ(getNumRefCounts(), 1);
    ASSERT_EQ(ptr.use_count(), 2);
    ASSERT_EQ(ptrCopy.use_count(), 2);
}

class TestObject
{
public:
    explicit TestObject(bool& deleted) : m_deleted(deleted) { m_deleted = false; }

    ~TestObject() { m_deleted = true; }

private:
    bool& m_deleted;
};

TEST_F(RefCountedPtrTests, Destructor)
{
    bool deleted = false;
    {
        const luabridge::RefCountedPtr<TestObject> ptr(new TestObject(deleted));
        ASSERT_FALSE(deleted);
    }

    ASSERT_TRUE(deleted);
}

TEST_F(RefCountedPtrTests, AssignOperator)
{
    bool deletedPrevious = false;
    luabridge::RefCountedPtr<TestObject> ptr(new TestObject(deletedPrevious));

    bool deletedNew = false;
    TestObject* const rawPtr = new TestObject(deletedNew);

    luabridge::RefCountedPtr<TestObject>& returnValue = (ptr = rawPtr);

    ASSERT_EQ(&returnValue, &ptr);
    ASSERT_EQ(ptr, rawPtr);
    ASSERT_EQ(ptr.use_count(), 1);
    ASSERT_TRUE(deletedPrevious);
    ASSERT_FALSE(deletedNew);
}

TEST_F(RefCountedPtrTests, AssignOperatorSelfAssignment)
{
    bool deleted = false;
    TestObject* const rawPtr = new TestObject(deleted);

    luabridge::RefCountedPtr<TestObject> ptr(rawPtr);

    luabridge::RefCountedPtr<TestObject>& returnValue = (ptr = ptr);

    ASSERT_EQ(&returnValue, &ptr);
    ASSERT_EQ(ptr, rawPtr);
    ASSERT_EQ(ptr.use_count(), 1);
    ASSERT_FALSE(deleted);
}

TEST_F(RefCountedPtrTests, AssignOperatorSameObject)
{
    bool deleted = false;
    TestObject* const rawPtr = new TestObject(deleted);

    luabridge::RefCountedPtr<TestObject> ptr(rawPtr);

    luabridge::RefCountedPtr<TestObject>& returnValue = (ptr = rawPtr);

    ASSERT_EQ(&returnValue, &ptr);
    ASSERT_EQ(ptr, rawPtr);
    ASSERT_EQ(ptr.use_count(), 1);
    ASSERT_FALSE(deleted);
}

class TestObjectNested
{
public:
    explicit TestObjectNested(const uint64_t value) : m_value(value) {}

    uint64_t getValue() const { return m_value; }

    luabridge::RefCountedPtr<TestObjectNested>& getChild() { return m_child; }

private:
    const uint64_t m_value;

    luabridge::RefCountedPtr<TestObjectNested> m_child;
};

TEST_F(RefCountedPtrTests, AssignOperatorNestedObjects)
{
    // Test assignment operator in the case that the previous referenced object is
    // part of the new referenced object. This nested situation can only be handled
    // if the reference count of the new object is FIRST increased and after that
    // the count of the old object is decreased. If this happens vice versa the
    // stored pointer is invalid after the assignment because it points to an already
    // deleted object.
    const uint64_t parentValue = 123, childValue = 456;

    luabridge::RefCountedPtr<TestObjectNested> ref = new TestObjectNested(parentValue);
    ref->getChild() = new TestObjectNested(childValue);

    ASSERT_EQ(ref->getValue(), parentValue);
    ASSERT_EQ(ref->getChild()->getValue(), childValue);

    const luabridge::RefCountedPtr<TestObjectNested>& returnValue = (ref = ref->getChild());
    ASSERT_EQ(&returnValue, &ref);
    ASSERT_EQ(ref->getValue(), childValue);
}

TEST_F(RefCountedPtrTests, CompareOperators)
{
    int* const rawPtr1 = new int;
    luabridge::RefCountedPtr<int> ptr1(rawPtr1);

    int* const rawPtr2 = new int;
    luabridge::RefCountedPtr<int> ptr2(rawPtr2);

    ASSERT_TRUE(ptr1 == ptr1);
    ASSERT_TRUE(ptr1 != ptr2);

    ASSERT_TRUE(rawPtr1 == ptr1);
    ASSERT_TRUE(ptr1 == rawPtr1);

    ASSERT_TRUE(rawPtr2 != ptr1);
    ASSERT_TRUE(ptr1 != rawPtr2);
}
