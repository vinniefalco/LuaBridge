<a href="http://lua.org">
<img src="http://vinniefalco.github.io/LuaBridgeDemo/powered-by-lua.png">
</a><br>

# LuaBridge 2.8

[LuaBridge][1] is a lightweight and dependency-free library for mapping data,
functions, and classes back and forth between C++ and [Lua][2] (a powerful,
fast, lightweight, embeddable scripting language). LuaBridge has been tested
and works with Lua revisions starting from 5.1.5, although it should work in
any version of Lua from 5.1.0 as well as [LuaJit][3].

LuaBridge offers the following features:

- [MIT Licensed][4]
- A printable [Reference Manual][5].
- Headers-only: No Makefile, no .cpp files, just one #include!
- Simple, light, and nothing else needed (like Boost).
- No macros, settings, or configuration scripts needed.
- Supports different object lifetime management models.
- Convenient, type-safe access to the Lua stack.
- Automatic function parameter type binding.
- Easy access to Lua objects like tables and functions.
- Written in a clear and easy to debug style.

Please read the [LuaBridge Reference Manual][5] for more details on the API.

## Unit Tests

Unit test build requires a CMake and C++11 compliant compiler.

To enable C++17 features (`std::optional` and `std::string_view`) specify an extra option: `-DLUABRIDGE_CXX17=1`.

There are the following unit test flavors:
* `Tests51` - uses Lua 5.1.5
* `Tests51Cxx17` - uses Lua 5.1.5 and C++17 features
* `Tests52` - uses Lua 5.2.4,
* `Tests52Cxx17` - uses Lua 5.2.4 and C++17 features
* `Tests53` - uses Lua 5.3.6
* `Tests53Cxx17` - uses Lua 5.3.6 and C++17 features
* `Tests54` - uses Lua 5.4.4
* `Tests54Cxx17` - uses Lua 5.4.4 and C++17 features

Build using Make on Linux/MacOS:
```bash
clone --recurse-submodules git@github.com:vinniefalco/LuaBridge.git
cd LuaBridge
cmake -DCMAKE_BUILD_TYPE=Debug -B build
# or cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -B build
# or cmake -DCMAKE_BUILD_TYPE=Release -B build
cd build
make -j
```

Generate XCode project on MacOS:
```bash
clone --recurse-submodules git@github.com:vinniefalco/LuaBridge.git
cd LuaBridge
cmake -G Xcode -B build
# Generates XCode project build/LuaBridge.xcodeproj
```

Generate MSVS solution on Windows:
```cmd
clone --recurse-submodules git@github.com:vinniefalco/LuaBridge.git
cd LuaBridge
mkdir build
cd build
cmake -G "Visual Studio 17 2022 Win64" -B build
# or cmake -G "Visual Studio 15 2017 Win64" -B build
# or cmake -G "Visual Studio 14 2015" -B build
# or cmake -G "Visual Studio 15 2017 Win64" -B build
# or cmake -G "Visual Studio 15 2017" -B build
# or cmake -G "Visual Studio 15 2019" -A Win64 -B build
# or cmake -G "Visual Studio 15 2019" -B build
# Generates MSVS solution build/LuaBridge.sln
```

## Installing LuaBridge (vcpkg)

You can download and install LuaBridge using the [vcpkg](https://github.com/Microsoft/vcpkg) dependency manager:
```Powershell or bash
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh # The name of the script should be "./bootstrap-vcpkg.bat" for Powershell
./vcpkg integrate install
./vcpkg install luabridge
```

The LuaBridge port in vcpkg is kept up to date by Microsoft team members and community contributors. If the version is out of date, please [create an issue or pull request](https://github.com/Microsoft/vcpkg) on the vcpkg repository.


## Official Repository

LuaBridge is published under the terms of the [MIT License][4].

The original version of LuaBridge was written by Nathan Reed. The project has
been taken over by [Vinnie Falco][7], who added new functionality, wrote the new
documentation, and incorporated contributions from Nigel Atkinson.

For questions, comments, or bug reports feel free to open a Github issue
or contact Vinnie Falco directly at the email address indicated below.

Copyright 2019, Dmitry Tarakanov<br>
Copyright 2012, [Vinnie Falco][7] (<[vinnie.falco@gmail.com][8]>)<br>
Copyright 2008, Nigel Atkinson<br>
Copyright 2007, Nathan Reed<br>

Portions from The Loki Library:<br>
Copyright (C) 2001 by Andrei Alexandrescu

Older versions of LuaBridge up to and including 0.2 are distributed under the
BSD 3-Clause License. See the corresponding license file in those versions
(distributed separately) for more details.

[1]:  https://github.com/vinniefalco/LuaBridge "LuaBridge"
[2]:  http://lua.org "The Lua Programming Language"
[3]:  http://luajit.org/ "The LuaJIT Probject"
[4]:  http://www.opensource.org/licenses/mit-license.html "The MIT License"
[5]:  http://vinniefalco.github.io/LuaBridge "LuaBridge Reference Manual"
[6]:  https://github.com/vinniefalco/LuaBridgeDemo "LuaBridge Demo"
[7]:  https://github.com/vinniefalco "Vinnie Falco's Github"
[8]:  mailto:vinnie.falco@gmail.com "Vinnie Falco (Email)"
