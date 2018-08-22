set (LUABRIDGE_TEST_SOURCE_FILES
    Source/Performance.h
    Source/Performance.cpp
    Source/Tests.h
    Source/Tests.cpp
    Source/TestsMain.cpp
)

source_group ("Tests\\Source" FILES ${LUABRIDGE_TEST_SOURCE_FILES})

set (LUABRIDGE_TEST_JUICE_FILES
	JuceLibraryCode/AppConfig.h
	JuceLibraryCode/BinaryData.h
	JuceLibraryCode/BinaryData.cpp
	JuceLibraryCode/JuceHeader.h
)

source_group ("Tests\\JuceLibraryCode" FILES ${LUABRIDGE_TEST_JUICE_FILES})

add_compile_options(-DLUABRIDGEDEMO_LUA_VERSION=502)

file (GLOB_RECURSE LUABRIDGE_TEST_LUA52_FILES
    LuaLibrary.h
	Lua/LuaLibrary5.2.0.cpp
)

source_group ("Tests\\Lua5.2.0" FILES ${LUABRIDGE_TEST_LUA52_FILES})

add_executable (LuaBridgeTests
    ${LUABRIDGE_TEST_SOURCE_FILES}
	${LUABRIDGE_TEST_JUICE_FILES}
    ${LUABRIDGE_TEST_LUA52_FILES}
)

target_include_directories (LuaBridgeTests PRIVATE . Source)

target_link_libraries(LuaBridgeTests LuaBridge)
