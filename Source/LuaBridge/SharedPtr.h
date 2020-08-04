// https://github.com/vinniefalco/LuaBridge
//
// Copyright 2020, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include <LuaBridge/detail/TypeTraits.h>

#include <memory>

namespace luabridge {

template <class T>
struct ContainerTraits;

template <class T>
struct ContainerTraits <std::shared_ptr <T>>
{
  typedef T Type;

  static T* get (const std::shared_ptr <T>& c)
  {
    return c.get ();
  }
};

} // namespace luabridge

