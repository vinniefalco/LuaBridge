set (LUABRIDGE_TEST_SOURCE_FILES
  Source/ClassTests.cpp
  Source/IssueTests.cpp
  Source/IteratorTests.cpp
  Source/LegacyTests.cpp
  Source/ListTests.cpp
  Source/LuaRefTests.cpp
  Source/MapTests.cpp
  Source/NamespaceTests.cpp
  Source/PerformanceTests.cpp
  Source/Tests.cpp
  Source/TestBase.h
  Source/TestsMain.cpp
  Source/VectorTests.cpp
)

source_group ("Source" FILES ${LUABRIDGE_TEST_SOURCE_FILES})

set (LUABRIDGE_TEST_JUICE_FILES
  JuceLibraryCode/AppConfig.h
  JuceLibraryCode/BinaryData.h
  JuceLibraryCode/BinaryData.cpp
  JuceLibraryCode/JuceHeader.h
)

add_library (JuceLibraryCode ${LUABRIDGE_TEST_JUICE_FILES})

# Lua 5.1

file (GLOB_RECURSE LUABRIDGE_TEST_LUA51_FILES
  LuaLibrary.h
  Lua/LuaLibrary5.1.5.cpp
)

add_library (Lua51 ${LUABRIDGE_TEST_LUA51_FILES})

target_compile_options (Lua51 PUBLIC -DLUABRIDGEDEMO_LUA_VERSION=501)

target_include_directories (Lua51 PRIVATE .)

add_executable (LuaBridgeTests51
  ${LUABRIDGE_TEST_SOURCE_FILES}
)

target_include_directories (LuaBridgeTests51 PRIVATE . Source)

target_link_libraries (LuaBridgeTests51
  LuaBridge
  Lua51
  gtest
  JuceLibraryCode
)

# Lua 5.2

file (GLOB_RECURSE LUABRIDGE_TEST_LUA52_FILES
  Lua/LuaLibrary.h
  Lua/LuaLibrary5.2.0.cpp
)

add_library (Lua52 ${LUABRIDGE_TEST_LUA52_FILES})

target_compile_options (Lua52 PUBLIC -DLUABRIDGEDEMO_LUA_VERSION=502)

target_include_directories (Lua52 PRIVATE .)

add_executable (LuaBridgeTests52
  ${LUABRIDGE_TEST_SOURCE_FILES}
)

target_include_directories (LuaBridgeTests52 PRIVATE . Source)

target_link_libraries (LuaBridgeTests52
  LuaBridge
  Lua52
  gtest
  JuceLibraryCode
)

# Fix runtime library link options by gtest macro
fix_default_compiler_settings_()
