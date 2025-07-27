project(yomm2_example)
cmake_minimum_required(VERSION 3.0)

# set policy CMP0063
cmake_policy(SET CMP0063 NEW)


set(CMAKE_CXX_STANDARD 20)
set(BOOST_ROOT /home/yoco/libs/boost_1_87_0)
find_package(Boost REQUIRED)
find_package(GTest REQUIRED)
message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")


find_package(Python 3.8 COMPONENTS Interpreter Development.Module REQUIRED)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Detect the installed nanobind package and import it into CMake
execute_process(
  COMMAND "${Python_EXECUTABLE}" -m nanobind --cmake_dir
  OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE NB_DIR)
message("NB_DIR: ${NB_DIR}")
list(APPEND CMAKE_PREFIX_PATH "${NB_DIR}")

add_executable(a.out main.cpp)
target_include_directories(
    a.out PRIVATE
)

target_link_libraries(a.out PRIVATE pthread)
target_include_directories(a.out PRIVATE ${Boost_INCLUDE_DIRS})

add_subdirectory(test)