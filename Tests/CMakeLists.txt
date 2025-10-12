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
    Source/UserdataTests.cpp
    Source/VectorTests.cpp
)

if(LUABRIDGE_CXX17)
    set(LUABRIDGE_TEST_SOURCE_FILES
        ${LUABRIDGE_TEST_SOURCE_FILES}
        Source/OptionalTests.cpp
    )
endif()

source_group("Source" FILES ${LUABRIDGE_TEST_SOURCE_FILES})

# Common lua library definition

macro(add_lua_lib LUA_LIB_NAME LUA_VERSION LUA_SOURCE_DIR)
    file(GLOB LUA_SOURCES
        RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        ${LUA_SOURCE_DIR}/*.c
        ${LUA_SOURCE_DIR}/*.h
        ${LUA_SOURCE_DIR}/*.hpp
        Lua/LuaLibrary.h
    )
    list(REMOVE_ITEM LUA_SOURCES
        ${LUA_SOURCE_DIR}/lua.c
        ${LUA_SOURCE_DIR}/luac.c
    )

    add_library(${LUA_LIB_NAME} ${LUA_SOURCES})
    target_compile_definitions(${LUA_LIB_NAME} PUBLIC LUABRIDGE_TEST_LUA_VERSION=${LUA_VERSION})
    target_include_directories(${LUA_LIB_NAME} PUBLIC ${LUA_SOURCE_DIR})
    target_compile_options(${LUA_LIB_NAME} PRIVATE $<IF:$<BOOL:${MSVC}>,/W0,-w>)
    target_compile_definitions(${LUA_LIB_NAME} PRIVATE $<IF:$<C_COMPILER_ID:GNU>,-D_GNU_SOURCE,>)
    source_group("Source" FILES ${LUA_SOURCES})
endmacro()

# Lua 5.1

add_lua_lib(Lua51 501 Lua/lua-5.1.5/src)

# Lua 5.2

add_lua_lib(Lua52 502 Lua/lua-5.2.4/src)

# Lua 5.3

add_lua_lib(Lua53 503 Lua/lua-5.3.6/src)

# Lua 5.4

add_lua_lib(Lua54 504 Lua/lua-5.4.4/src)

# Common test app definitions

macro(add_test_app APP_NAME LUA_LIBRARY CXX_VERSION)
    add_executable(${APP_NAME}
        ${LUABRIDGE_TEST_SOURCE_FILES}
    )
    
    set_target_properties(${APP_NAME} PROPERTIES
        CXX_STANDARD ${CXX_VERSION}
    )

    target_include_directories(${APP_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_options(${APP_NAME} PRIVATE $<IF:$<BOOL:${MSVC}>,/W4 /WX,-Wall -Werror>)

    target_link_libraries(${APP_NAME}
        LuaBridge
        ${LUA_LIBRARY}
        gtest
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
