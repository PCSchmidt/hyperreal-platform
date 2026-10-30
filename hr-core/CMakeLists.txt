add_library(hr_core
  src/error.cpp
  src/sha256.cpp
  src/hash_chain.cpp)
add_library(hr::core ALIAS hr_core)
target_include_directories(hr_core PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_compile_features(hr_core PUBLIC cxx_std_20)
set_target_properties(hr_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Determinism-oriented flags: strict floating point, no fast-math, no FMA contraction.
if(MSVC)
  target_compile_options(hr_core PUBLIC /fp:strict /W4 /permissive-)
else()
  target_compile_options(hr_core PUBLIC -ffp-contract=off -fno-fast-math -Wall -Wextra -Wpedantic)
endif()

if(HR_BUILD_TESTS)
  include(FetchContent)
  FetchContent_Declare(doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.11)
  FetchContent_MakeAvailable(doctest)
  add_executable(hr_core_tests
    tests/test_main.cpp
    tests/test_rng.cpp
    tests/test_sha256.cpp
    tests/test_hash_chain.cpp
    tests/test_clock.cpp
    tests/test_error.cpp)
  target_link_libraries(hr_core_tests PRIVATE hr::core doctest::doctest)
  add_test(NAME hr_core_tests COMMAND hr_core_tests)
endif()
