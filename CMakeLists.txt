cmake_minimum_required (VERSION 3.0)

project(jsandplus
  LANGUAGES CXX
  HOMEPAGE_URL https://github.com/jeffersonfr/jsandplus
  DESCRIPTION "Sand emulator"
  VERSION 1.0.0
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

enable_testing()

add_subdirectory(src)
