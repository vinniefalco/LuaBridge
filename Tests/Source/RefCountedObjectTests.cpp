// https://github.com/vinniefalco/LuaBridge
// Copyright 2019, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include "LuaBridge/RefCountedObject.h"

struct RefCountedObjectTests : TestBase
{
    template<class T>
    T variable(const std::string& name)
    {
        runLua("result = " + name);
        return result().cast<T>();
    }
};

namespace {

class RefCounted : public luabridge::RefCountedObject
{
public:
    explicit RefCounted(bool& deleted) : m_deleted(deleted) { m_deleted = false; }

    ~RefCounted() { m_deleted = true; }

    bool isDeleted() const { return m_deleted; }

private:
    bool& m_deleted;
};

} // namespace

TEST_F(RefCountedObjectTests, ConstructorDefault)
{
    const luabridge::RefCountedObjectPtr<RefCounted> ptr;

    ASSERT_EQ(ptr, nullptr);
}

TEST_F(RefCountedObjectTests, ConstructorObject)
{
    bool deleted = false;
    RefCounted* const rawPtr = new RefCounted(deleted);
    const luabridge::RefCountedObjectPtr<RefCounted> ptr(rawPtr);

    ASSERT_EQ(ptr, rawPtr);
    ASSERT_EQ(rawPtr->getReferenceCount(), 1);
    ASSERT_FALSE(deleted);
}

TEST_F(RefCountedObjectTests, ConstructorCopy)
{
    bool deleted = false;
    RefCounted* const rawPtr = new RefCounted(deleted);
    const luabridge::RefCountedObjectPtr<RefCounted> ptr(rawPtr);
    const luabridge::RefCountedObjectPtr<RefCounted> ptrCopy(ptr);

    ASSERT_EQ(ptr, rawPtr);
    ASSERT_EQ(ptrCopy, rawPtr);
    ASSERT_EQ(rawPtr->getReferenceCount(), 2);
    ASSERT_FALSE(deleted);
}

TEST_F(RefCountedObjectTests, ConstructorCopyPolymorph)
{
    struct RefCountedSpecialized : public RefCounted
    {
        explicit RefCountedSpecialized(bool& deleted) : RefCounted(deleted) {}
    };

    bool deleted = false;
    RefCountedSpecialized* const rawPtr = new RefCountedSpecialized(deleted);

    const luabridge::RefCountedObjectPtr<RefCountedSpecialized> ptr(rawPtr);
    const luabridge::RefCountedObjectPtr<RefCounted> ptrCopy(ptr);

    ASSERT_EQ(ptr, rawPtr);
    ASSERT_EQ(ptrCopy, rawPtr);
    ASSERT_EQ(rawPtr->getReferenceCount(), 2);
    ASSERT_FALSE(deleted);
}

TEST_F(RefCountedObjectTests, Destructor)
{
    bool deleted = false;
    {
        luabridge::RefCountedObjectPtr<RefCounted> ptr(new RefCounted(deleted));
        ASSERT_FALSE(deleted);
    }

    ASSERT_TRUE(deleted);
}

TEST_F(RefCountedObjectTests, AssignOperator)
{
    bool deletedPrevious = false;
    luabridge::RefCountedObjectPtr<RefCounted> ptr(new RefCounted(deletedPrevious));

    bool deletedNew = false;
    RefCounted* const rawPtr = new RefCounted(deletedNew);

    const luabridge::RefCountedObjectPtr<RefCounted>& returnValue = (ptr = rawPtr);

    ASSERT_EQ(&returnValue, &ptr);
    ASSERT_EQ(ptr, rawPtr);
    ASSERT_EQ(rawPtr->getReferenceCount(), 1);
    ASSERT_TRUE(deletedPrevious);
    ASSERT_FALSE(deletedNew);
}

TEST_F(RefCountedObjectTests, AssignOperatorSameObject)
{
    bool deleted = false;
    RefCounted* const rawPtr = new RefCounted(deleted);

    luabridge::RefCountedObjectPtr<RefCounted> ptr(rawPtr);

    const luabridge::RefCountedObjectPtr<RefCounted>& returnValue = (ptr = rawPtr);

    ASSERT_EQ(&returnValue, &ptr);
    ASSERT_EQ(ptr, rawPtr);
    ASSERT_EQ(rawPtr->getReferenceCount(), 1);
    ASSERT_FALSE(deleted);
}

TEST_F(RefCountedObjectTests, AssignOperatorRef)
{
    bool deletedPrevious = false;
    luabridge::RefCountedObjectPtr<RefCounted> ptr(new RefCounted(deletedPrevious));

    bool deletedNew = false;
    RefCounted* const rawPtrNew = new RefCounted(deletedNew);
    const luabridge::RefCountedObjectPtr<RefCounted> ptrNew(rawPtrNew);

    const luabridge::RefCountedObjectPtr<RefCounted>& returnValue = (ptr = ptrNew);

    ASSERT_EQ(&returnValue, &ptr);
    ASSERT_EQ(ptr, rawPtrNew);
    ASSERT_EQ(rawPtrNew->getReferenceCount(), 2);
    ASSERT_TRUE(deletedPrevious);
    ASSERT_FALSE(deletedNew);
}

TEST_F(RefCountedObjectTests, AssignOperatorRefPolymorph)
{
    struct RefCountedSpecialized : public RefCounted
    {
        explicit RefCountedSpecialized(bool& deleted) : RefCounted(deleted) {}
    };

    bool deletedPrevious = false;
    luabridge::RefCountedObjectPtr<RefCounted> ptr(new RefCounted(deletedPrevious));

    bool deletedNew = false;
    RefCountedSpecialized* const rawPtrNew = new RefCountedSpecialized(deletedNew);
    const luabridge::RefCountedObjectPtr<RefCountedSpecialized> ptrNew(rawPtrNew);

    const luabridge::RefCountedObjectPtr<RefCounted>& returnValue = (ptr = ptrNew);

    ASSERT_EQ(&returnValue, &ptr);
    ASSERT_EQ(ptr, rawPtrNew);
    ASSERT_EQ(rawPtrNew->getReferenceCount(), 2);
    ASSERT_TRUE(deletedPrevious);
    ASSERT_FALSE(deletedNew);
}

TEST_F(RefCountedObjectTests, AssignOperatorRefSelfAssignment)
{
    bool deleted = false;
    RefCounted* const rawPtr = new RefCounted(deleted);

    luabridge::RefCountedObjectPtr<RefCounted> ptr(rawPtr);

    const luabridge::RefCountedObjectPtr<RefCounted>& returnValue = (ptr = ptr);

    ASSERT_EQ(&returnValue, &ptr);
    ASSERT_EQ(ptr, rawPtr);
    ASSERT_EQ(rawPtr->getReferenceCount(), 1);
    ASSERT_FALSE(deleted);
}

TEST_F(RefCountedObjectTests, AssignOperatorRefSameObject)
{
    bool deleted = false;
    RefCounted* const rawPtr = new RefCounted(deleted);

    const luabridge::RefCountedObjectPtr<RefCounted> ptr1(rawPtr);
    luabridge::RefCountedObjectPtr<RefCounted> ptr2(rawPtr);

    const luabridge::RefCountedObjectPtr<RefCounted>& returnValue = (ptr2 = ptr1);

    ASSERT_EQ(&returnValue, &ptr2);
    ASSERT_EQ(ptr1, rawPtr);
    ASSERT_EQ(ptr2, rawPtr);
    ASSERT_EQ(rawPtr->getReferenceCount(), 2);
    ASSERT_FALSE(deleted);
}

namespace {

class TestObjectNested final : public luabridge::RefCountedObject
{
public:
    explicit TestObjectNested(const uint64_t value) : m_value(value) {}

    uint64_t getValue() const { return m_value; }

    luabridge::RefCountedObjectPtr<TestObjectNested>& getChild() { return m_child; }

private:
    const uint64_t m_value;

    luabridge::RefCountedObjectPtr<TestObjectNested> m_child;
};

} // namespace

TEST_F(RefCountedObjectTests, AssignOperatorRefNestedObjects)
{
    // Test assignment operator in the case that the previous referenced object is
    // part of the new referenced object. This nested situation can only be handled
    // if the reference count of the new object is FIRST increased and after that
    // the count of the old object is decreased. If this happens vice versa the
    // stored pointer is invalid after the assignment because it points to an already
    // deleted object.
    const uint64_t parentValue = 123, childValue = 456;

    luabridge::RefCountedObjectPtr<TestObjectNested> ref = new TestObjectNested(parentValue);
    ref->getChild() = new TestObjectNested(childValue);

    ASSERT_EQ(ref->getValue(), parentValue);
    ASSERT_EQ(ref->getChild()->getValue(), childValue);

    const luabridge::RefCountedObjectPtr<TestObjectNested>& returnValue = (ref = ref->getChild());
    ASSERT_EQ(&returnValue, &ref);
    ASSERT_EQ(ref->getValue(), childValue);
}

TEST_F(RefCountedObjectTests, CompareOperators)
{
    bool deleted = false;

    RefCounted* const rawPtr1 = new RefCounted(deleted);
    luabridge::RefCountedObjectPtr<RefCounted> ptr1(rawPtr1);

    RefCounted* const rawPtr2 = new RefCounted(deleted);
    luabridge::RefCountedObjectPtr<RefCounted> ptr2(rawPtr2);

    ASSERT_TRUE(ptr1 == ptr1);
    ASSERT_TRUE(ptr1 != ptr2);

    ASSERT_TRUE(rawPtr1 == ptr1);
    ASSERT_TRUE(ptr1 == rawPtr1);

    ASSERT_TRUE(rawPtr2 != ptr1);
    ASSERT_TRUE(ptr1 != rawPtr2);
}

TEST_F(RefCountedObjectTests, LastReferenceInLua)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<RefCounted>("Class")
        .addProperty("deleted", &RefCounted::isDeleted)
        .endClass();

    bool deleted = false;
    luabridge::RefCountedObjectPtr<RefCounted> object(new RefCounted(deleted));

    luabridge::setGlobal(L, object, "object");
    runLua("result = object.deleted");
    ASSERT_EQ(true, result().isBool());
    ASSERT_EQ(false, result<bool>());

    object = nullptr;
    runLua("result = object.deleted");
    ASSERT_EQ(true, result().isBool());
    ASSERT_EQ(false, result<bool>());
    ASSERT_EQ(false, deleted);

    runLua("object = nil");
    lua_gc(L, LUA_GCCOLLECT, 1);

    ASSERT_EQ(true, deleted);
}

TEST_F(RefCountedObjectTests, LastReferenceInCpp)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<RefCounted>("Class")
        .addProperty("deleted", &RefCounted::isDeleted)
        .endClass();

    bool deleted = false;
    luabridge::RefCountedObjectPtr<RefCounted> object(new RefCounted(deleted));

    luabridge::setGlobal(L, object, "object");
    runLua("result = object.deleted");
    ASSERT_EQ(true, result().isBool());
    ASSERT_EQ(false, result<bool>());

    runLua("object = nil");
    lua_gc(L, LUA_GCCOLLECT, 1);
    ASSERT_EQ(false, deleted);

    object = nullptr;
    ASSERT_EQ(true, deleted);
}
