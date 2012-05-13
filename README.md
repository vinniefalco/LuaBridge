# LuaBridge

[LuaBridge][3] is a lightweight, dependency-free library for binding to C++.
It works with Lua revisions starting from 5.1.2. [Lua][4] is a powerful, fast,
lightweight, embeddable scripting language.

## Compiling

LuaBridge is distributed as a single header file "LuaBridge.h" that you simply
include where you want to register your functions, classes, and variables.
There are no additional source files, no compilation settings, and no
Makefiles or IDE-specific project files. LuaBridge is easy to integrate.

## Usage

LuaBridge is based on C++ template metaprogramming.  It contains template code
to automatically generate at compile-time the various Lua API calls necessary
to export your program's classes and functions to the Lua environment.

### Registering functions

If `L` is a pointer to an instance of `lua_State`, the following code creates
a LuaBridge scope for registering C++ functions and classes to `L`:

    luabridge::scope s (L);

Functions can then be registered as follows:

    s .function ("foo", &foo)
      .function ("bar", &bar);

The `function` function returns a reference to `s`, so you can chain many
definitions together.  The first argument is the name by which the function
will be available in Lua, and the second is the function's address. LuaBridge
will automatically detect the number (up to 8, by default) and type of the
parameters.  Functions registered this way will be available at the global
scope to Lua scripts executed by `L`.  Overloading of function names is not
supported, nor is it likely to be supported in the future.

### Registering data

Variables can also be registered.  You can expose a 'bare' variable to Lua, or
wrap it in getter and setter functions:

    s .variable_rw ("var1", &var1)
      .variable_rw ("var2", &getter2, &setter2)
      .variable_ro ("var3", &var3)
      .variable_ro ("var4", &getter4)

The first registration above gives Lua direct access to the `var1` variable.
The second creates a variable which appears like any other variable to Lua
code, but is retrieved and set through the `getter2` and `setter2` function.
The getter must take no parameters and return a value, and the setter must
take a value of the same type and return nothing.  The `variable_rw` function
creates a readable and writeable variable, while `variable_ro` creates a
read-only one. Obviously, there is no setter for the read-only variable.

Basic types for supported variables, and function arguments and returns, are:
  
- `bool`
- `char`, converted to a string of length one.
- Integers, `float`, and `double`, converted to Lua_number.
- Strings: `char const*` and `std::string`

Of course, LuaBridge supports passing objects of class type, in a variety of
ways including dynamically allocated objects created with `new`. The behavior
of the object with respect to lifetime management depends on the manner in
which the object is passed. Given `class T`, these argument types are
supported:

- `T`, `T const` : Pass `T` by value. The lifetime is managed by Lua.
- `T*`, `T&`, `T const*`, `T const&` : Pass `T` by reference. The lifetime
    is managed by C++.
- `SharedPtr <T>`, `SharedPtr <T const>` : Pass `T` by container. The lifetime
    is managed by the container.

When Lua manages the lifetime of the object, it is subjected to all of the
normal garbage collection rules. C++ functions and member functions can
receive pointers and references to these objects, but care must be taken
to make sure that no attempt is made to access the object after it is
garbage collected. Usually this is done by simply not storing a pointer to
the object somewhere inside your C++.

When C++ manages the lifetime of the object, the Lua garbage collector has
no effect on it. Care must be taken to make sure that the C++ code does not
destroy the object while Lua still has a reference to it.

### Shared Pointers

A `SharedPtr` container template allows for object lifetime management that
behaves like `std::shared_ptr`. That is, objects are dynamically allocated
and reference counted. The object is not destroyed until the reference count
drops to zero. Such objects are safe to store in C++ and Lua code. A
garbage collection will only decrement the reference count.

LuaBridge registraton templates will automatically detect arguments which
behave like containers. For example, this registration is valid:

    extern void func (shared_ptr <T> p);

    s.function ("func", &func);

Any container may be used. LuaBridge expects that the container in question
is a class template with one template argument, and a member function called
get() which returns a pointer to the underlying object. If you need to use
a container with a different interface, you can specialize the `Container`
class for your container type and provide an extraction function. Your
specialization needs to be in the `luabridge` namespace. Here's an example
specialization for a container called `ReferenceCountedObject` which provides
a member function `getObject` that retrieves the pointer.

    namespace luabridge
    {
        template <>
        struct Container <ReferenceCountedObjectPtr>
        {
          template <class T>
          static T* get (ReferenceCountedObjectPtr <T> const& p)
          {
            return p.getObject ();
          }
        };
    }

### Registering classes

C++ classes can be registered with Lua as follows:

    s .class_ <MyClass> ("MyClass")
      .constructor <void (*) (void)> ()
      .method ("method1", &MyClass::method1)
      .method ("method2", &MyClass::method2);

    s .subclass <MySubclass, MyBaseClass> ("MySubclass")
      .constructor <...>
      ...

The `class_` function registers a class; its constructor will be available as
a global function with name given as argument to `class_`.  The object
returned can then be used to register the constructor (no overloading is
supported, so there can only be one constructor) and methods.

LuaBridge cannot automatically determine the number and types of constructor
parameters like it can for functions and methods, so you must provide them.
This is done by letting the `constructor` function take a template parameter,
which must be a function pointer type.  The parameter types will be extracted
from this (the return type is ignored).  For example, to register a
constructor taking two parameters, one `int` and one `char const*`, you would
write:

    s .class_ <MyClass> ()
      .constructor <void (*) (int, const char *)> ()

Note that in the example above, the name of the class was ommitted from the
argument to the `class` function. This allows you to add additional
registrations to a class that has already been registered.

Method registration works just like function registration.  Virtual methods
work normally; no special syntax is needed. const methods are detected and
const-correctness is enforced, so if a function returns a const object (or
rather, a `shared_ptr` to a const object) to Lua, that reference to the object
will be considered const and only const methods will be called on it.
Destructors are registered automatically for each class.

Static methods may be registered using the `static_method` function, which is
simply an alias for the `function` function:

    s .class_ <MyClass> ()
      .static_method ("method3", &MyClass::method3)

LuaBridge also supports properties, which allow class data members to be read
and written from Lua as if they were variables.  Properties work much like
variables do, and the syntax for registering them is as follows:

    s .class_ <MyClass> ()
      .property_rw ("property1", &MyClass::property1)
      .property_rw ("property2", &MyClass::getter2, &MyClass::setter2)
      .property_ro ("property3", &MyClass::property3)
      .property_ro ("property4", &MyClass::getter4)

Static properties on classes are also supported, using `static_property_rw`
and `static_property_ro`, which are simply aliases for `variable_rw` and
`variable_ro`.

To register a subclass of another class that has been registered with Lua, use
the `subclass` function.  This takes two template arguments: the class to be
registered, and its base class.  Inherited methods do not have to be
re-declared and will function normally in Lua.  If a class has a base class
that is *not* registered with Lua, there is no need to declare it as a
subclass.

## Limitations 

LuaBridge does not support:

- More than 8 parameters on a function or method (although this can be
  increased by adding more `typelist` specializations).
- Overloaded functions, methods, or constructors.
- Global variables (variables must be wrapped in a named scope).
- Automatic conversion between STL container types and Lua tables.
- Inheriting Lua classes from C++ classes.

## Development

[Github][3] is the new official home for LuaBridge. The old SVN repository is
deprecated since it is no longer used, or maintained. The original author has
graciously passed the reins to Vinnie Falco for maintaining and improving the
project. To obtain the older official releases, checkout the tags from 2.1
and earlier.

We welcome contributions to LuaBridge. Feel free to fork the repository and
issue a Pull Request. All questions, comments, suggestions, and/or proposed
changes will be handled by the new maintainer.

## License

Copyright (C) 2012, [Vinnie Falco][1] ([e-mail][0]) <br>
Copyright (C) 2007, Nathan Reed <br>
  
Portions from The Loki Library: <br>
Copyright (C) 2001 by Andrei Alexandrescu

License: The [MIT License][2]

Older versions of LuaBridge up to and including 0.2 are distributed under the
BSD 3-Clause License. See the corresponding license file in those versions
for more details.

[0]: mailto:vinnie.falco@gmail.com "Vinnie Falco (Email)"
[1]: http://www.vinniefalco.com "Vinnie Falco"
[2]: http://www.opensource.org/licenses/mit-license.html "The MIT License"
[3]: https://github.com/vinniefalco/LuaBridge "LuaBridge"
[4]: http://lua.org "The Lua Programming Language"
