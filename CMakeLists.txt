cmake_minimum_required(VERSION 3.0...3.22)
project(
  Minigrid2PRISM
  VERSION 0.1
  LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

include(util/CMakeLists.txt)
include(FetchContent)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG master
  OVERRIDE_FIND_PACKAGE
)
FetchContent_GetProperties(yaml-cpp)
if(NOT yaml-cpp_POPULATED)
  message(STATUS "Fetching yaml-cpp...")
  FetchContent_Populate(yaml-cpp)
  add_subdirectory(${yaml-cpp_SOURCE_DIR} ${yaml-cpp_BINARY_DIR})
endif()
FetchContent_MakeAvailable(yaml-cpp)

add_executable(main ${SRCS} main.cpp)
target_link_libraries(main pthread yaml-cpp::yaml-cpp)
