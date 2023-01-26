set(LUABRIDGE_TEST_SOURCE_FILES
    Source/ArrayTests.cpp
    Source/ClassTests.cpp
    Source/IssueTests.cpp
    Source/IteratorTests.cpp
    Source/LegacyTests.cpp
    Source/ListTests.cpp
    Source/LuaRefTests.cpp
    Source/MapTests.cpp
    Source/NamespaceTests.cpp
    Source/PairTests.cpp
    Source/PerformanceTests.cpp
    Source/RefCountedObjectTests.cpp
    Source/RefCountedPtrTests.cpp
    Source/StackTests.cpp
    Source/Tests.cpp
    Source/TestBase.h
    Source/TestTypes.h
    Source/TestsMain.cpp
    Source/UnorderedMapTests.cpp
    Source/VectorTests.cpp
)

if(LUABRIDGE_CXX17)
    set(LUABRIDGE_TEST_SOURCE_FILES
        ${LUABRIDGE_TEST_SOURCE_FILES}
        Source/OptionalTests.cpp
    )
endif()

source_group("Source" FILES ${LUABRIDGE_TEST_SOURCE_FILES})

# Lua 5.1

file(GLOB LUABRIDGE_TEST_LUA51_FILES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    Lua/lua-5.1.5/src/*.c
    Lua/lua-5.1.5/src/*.h
    Lua/LuaLibrary.h
)

add_library(Lua51 ${LUABRIDGE_TEST_LUA51_FILES})
target_compile_definitions(Lua51 PUBLIC LUABRIDGE_TEST_LUA_VERSION=501)
target_include_directories(Lua51 PUBLIC Lua/lua-5.1.5/src)
source_group("Source" FILES ${LUABRIDGE_TEST_LUA51_FILES})

# Lua 5.2

file(GLOB LUABRIDGE_TEST_LUA52_FILES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    Lua/lua-5.2.4/src/*.c
    Lua/lua-5.2.4/src/*.h
    Lua/lua-5.2.4/src/*.hpp
    Lua/LuaLibrary.h
)

add_library(Lua52 ${LUABRIDGE_TEST_LUA52_FILES})
target_compile_definitions(Lua52 PUBLIC LUABRIDGE_TEST_LUA_VERSION=502)
target_include_directories(Lua52 PUBLIC Lua/lua-5.2.4/src)
source_group("Source" FILES ${LUABRIDGE_TEST_LUA52_FILES})

# Lua 5.3

file(GLOB LUABRIDGE_TEST_LUA53_FILES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    Lua/lua-5.3.6/src/*.c
    Lua/lua-5.3.6/src/*.h
    Lua/lua-5.3.6/src/*.hpp
    Lua/LuaLibrary.h
)

add_library(Lua53 ${LUABRIDGE_TEST_LUA53_FILES})
target_compile_definitions(Lua53 PUBLIC LUABRIDGE_TEST_LUA_VERSION=503)
target_include_directories(Lua53 PUBLIC Lua/lua-5.3.6/src)
source_group("Source" FILES ${LUABRIDGE_TEST_LUA53_FILES})

# Lua 5.4

file(GLOB LUABRIDGE_TEST_LUA54_FILES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    Lua/lua-5.4.4/src/*.c
    Lua/lua-5.4.4/src/*.h
    Lua/lua-5.4.4/src/*.hpp
    Lua/LuaLibrary.h
)

add_library(Lua54 ${LUABRIDGE_TEST_LUA54_FILES})
target_compile_definitions(Lua54 PUBLIC LUABRIDGE_TEST_LUA_VERSION=504)
target_include_directories(Lua54 PUBLIC Lua/lua-5.4.4/src)
source_group("Source" FILES ${LUABRIDGE_TEST_LUA54_FILES})

# Common test app definitions

macro(add_test_app APP_NAME LUA_LIBRARY CXX_VERSION)
    add_executable(${APP_NAME}
        ${LUABRIDGE_TEST_SOURCE_FILES}
    )
    
    set_target_properties(${APP_NAME} PROPERTIES
        CXX_STANDARD ${CXX_VERSION}
    )

    target_include_directories(${APP_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

    target_link_libraries(${APP_NAME}
        LuaBridge
        ${LUA_LIBRARY}
        GTest::gtest
    )

    add_test(${APP_NAME} ${APP_NAME})
endmacro(add_test_app)

# Concrete test apps

add_test_app(Tests51 Lua51 14)

add_test_app(Tests51Cxx17 Lua51 17)
target_compile_definitions(Tests51Cxx17 PRIVATE LUABRIDGE_CXX17)

add_test_app(Tests52 Lua52 14)

add_test_app(Tests52Cxx17 Lua52 17)
target_compile_definitions(Tests52Cxx17 PRIVATE LUABRIDGE_CXX17)

add_test_app(Tests53 Lua53 14)

add_test_app(Tests53Cxx17 Lua53 17)
target_compile_definitions(Tests53Cxx17 PRIVATE LUABRIDGE_CXX17)

add_test_app(Tests54 Lua54 14)

add_test_app(Tests54Cxx17 Lua54 17)
target_compile_definitions(Tests54Cxx17 PRIVATE LUABRIDGE_CXX17)
