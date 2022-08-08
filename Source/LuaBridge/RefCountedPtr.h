// https://github.com/vinniefalco/LuaBridge
// Copyright 2021, Stefan Frings
// Copyright 2019, Dmitry Tarakanov
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2007, Nathan Reed
// SPDX-License-Identifier: MIT

#pragma once

#include <cassert>
#include <unordered_map>
#include <utility>

namespace luabridge {

namespace detail {

//==============================================================================
/**
  Support for our RefCountedPtr.
*/
struct RefCountedPtrBase
{
    // Declaration of container for the refcounts
    typedef std::unordered_map<const void*, int> RefCountsType;

protected:
    RefCountsType& getRefCounts() const
    {
        static RefCountsType refcounts;
        return refcounts;
    }
};

} // namespace detail

//==============================================================================
/**
  A reference counted smart pointer.

  The api is compatible with boost::RefCountedPtr and std::RefCountedPtr, in the
  sense that it implements a strict subset of the functionality.

  This implementation uses a hash table to look up the reference count
  associated with a particular pointer.

  @tparam T The class type.

  @todo Decompose RefCountedPtr using a policy. At a minimum, the underlying
        reference count should be policy based (to support atomic operations)
        and the delete behavior should be policy based (to support custom
        disposal methods).

  @todo Provide an intrusive version of RefCountedPtr.
*/
template<class T>
class RefCountedPtr : private detail::RefCountedPtrBase
{
public:
    template<typename Other>
    struct rebind
    {
        typedef RefCountedPtr<Other> other;
    };

    /** Construct as nullptr or from existing pointer to T.

        @param p The optional, existing pointer to assign from.
    */
    RefCountedPtr(T* const p = nullptr) : m_p(p)
    {
        if (m_p)
        {
            ++getRefCounts()[m_p];
        }
    }

    /** Construct from another RefCountedPtr.

        @param rhs The RefCountedPtr to assign from.
    */
    RefCountedPtr(RefCountedPtr<T> const& rhs) : RefCountedPtr(rhs.get()) {}

    /** Construct from a RefCountedPtr of a different type.

        @invariant A pointer to U must be convertible to a pointer to T.

        @tparam U   The other object type.
        @param  rhs The RefCountedPtr to assign from.
    */
    template<typename U>
    RefCountedPtr(RefCountedPtr<U> const& rhs) : RefCountedPtr(rhs.get())
    {
    }

    /** Release the object.

        If there are no more references then the object is deleted.
    */
    ~RefCountedPtr() { reset(); }

    /** Assign from another RefCountedPtr.

        @param  rhs The RefCountedPtr to assign from.
        @returns     A reference to the RefCountedPtr.
    */
    RefCountedPtr<T>& operator=(RefCountedPtr<T> const& rhs)
    {
        // NOTE Self assignment is handled gracefully
        *this = rhs.get();
        return *this;
    }

    /** Assign from another RefCountedPtr of a different type.

        @note A pointer to U must be convertible to a pointer to T.

        @tparam U   The other object type.
        @param  rhs The other RefCountedPtr to assign from.
        @returns     A reference to the RefCountedPtr.
    */
    template<typename U>
    RefCountedPtr<T>& operator=(RefCountedPtr<U> const& rhs)
    {
        // NOTE Self assignment is handled gracefully
        *this = rhs.get();
        return *this;
    }

    RefCountedPtr<T>& operator=(T* const p)
    {
        if (p != m_p)
        {
            RefCountedPtr<T> tmp(p);
            std::swap(m_p, tmp.m_p);
        }

        return *this;
    }

    /** Retrieve the raw pointer.

        @returns A pointer to the object.
    */
    T* get() const { return m_p; }

    /** Retrieve the raw pointer by conversion.

        @returns A pointer to the object.
    */
    operator T*() const { return m_p; }

    /** Retrieve the raw pointer.

        @returns A pointer to the object.
    */
    T* operator*() const { return m_p; }

    /** Retrieve the raw pointer.

        @returns A pointer to the object.
    */
    T* operator->() const { return m_p; }

    /** Determine the number of references.

        @note This is not thread-safe.

        @returns The number of active references.
    */
    RefCountsType::mapped_type use_count() const
    {
        if (!m_p)
        {
            return 0;
        }

        const auto itCounter = getRefCounts().find(m_p);
        assert(itCounter != getRefCounts().end());
        assert(itCounter->second > 0);

        return itCounter->second;
    }

    /** Release the pointer.

        The reference count is decremented. If the reference count reaches
        zero, the object is deleted.
    */
    void reset()
    {
        if (m_p)
        {
            const auto itCounter = getRefCounts().find(m_p);
            assert(itCounter != getRefCounts().end());
            assert(itCounter->second > 0);

            if (--itCounter->second == 0)
            {
                delete m_p;
                getRefCounts().erase(itCounter);
            }

            m_p = nullptr;
        }
    }

private:
    T* m_p;
};

//==============================================================================

// forward declaration
template<class T>
struct ContainerTraits;

template<class T>
struct ContainerTraits<RefCountedPtr<T>>
{
    typedef T Type;

    static T* get(RefCountedPtr<T> const& c) { return c.get(); }
};

} // namespace luabridge
