project('LuaBridge', 'cpp',
  license: 'MIT',
  default_options: ['cpp_std=c++17'],
)

install_headers([
  'Source/LuaBridge/List.h',
  'Source/LuaBridge/LuaBridge.h',
  'Source/LuaBridge/Map.h',
  'Source/LuaBridge/RefCountedObject.h',
  'Source/LuaBridge/RefCountedPtr.h',
  'Source/LuaBridge/UnorderedMap.h',
  'Source/LuaBridge/Vector.h',
  'Source/LuaBridge/Array.h',
  ], subdir: 'LuaBridge')
install_headers([
  'Source/LuaBridge/detail/CFunctions.h',
  'Source/LuaBridge/detail/ClassInfo.h',
  'Source/LuaBridge/detail/Config.h',
  'Source/LuaBridge/detail/Constructor.h',
  'Source/LuaBridge/detail/dump.h',
  'Source/LuaBridge/detail/FuncTraits.h',
  'Source/LuaBridge/detail/Iterator.h',
  'Source/LuaBridge/detail/LuaException.h',
  'Source/LuaBridge/detail/LuaHelpers.h',
  'Source/LuaBridge/detail/LuaRef.h',
  'Source/LuaBridge/detail/Namespace.h',
  'Source/LuaBridge/detail/Stack.h',
  'Source/LuaBridge/detail/TypeList.h',
  'Source/LuaBridge/detail/TypeTraits.h',
  'Source/LuaBridge/detail/Userdata.h',
  ], subdir: 'LuaBridge/detail')

luabridge_dep = declare_dependency(
  include_directories: include_directories('Source'),
  compile_args: '-DLUABRIDGE_CXX17',
)
