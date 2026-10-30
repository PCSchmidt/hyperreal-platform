cmake_minimum_required(VERSION 3.21)
project(hyperreal_platform VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build tests only when this is the top-level project (consumers opt in explicitly).
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(_HR_TOP ON)
else()
  set(_HR_TOP OFF)
endif()
option(HR_BUILD_TESTS "Build hr-core tests" ${_HR_TOP})

if(HR_BUILD_TESTS)
  enable_testing()
endif()

add_subdirectory(hr-core)
