
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(
  brep_sweep
  VERSION 0.1.0
  LANGUAGES CXX
)

include(cmake/get_cpm.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options("-fcoroutines")

CPMAddPackage(
  NAME "Eigen3"
  GIT_REPOSITORY "https://gitlab.com/libeigen/eigen.git"
  GIT_TAG "3.4"
  OPTIONS
    "BUILD_TESTING OFF"
    "EIGEN_BUILD_DOC OFF"  
    "EIGEN_LEAVE_TEST_IN_ALL_TARGET OFF"
    "EIGEN_BUILD_PKGCONFIG OFF"
)

CPMAddPackage(
  NAME "tl-generator"
  GIT_REPOSITORY "https://github.com/TartanLlama/generator"
  GIT_TAG "a8e421f04159199f6896b09cad2bd3c6fbb2a5fb"
  OPTIONS
    "GENERATOR_BUILD_PACKAGE OFF"
    "GENERATOR_BUILD_TESTS OFF"
)

CPMAddPackage(
  NAME "tl-ranges"
  GIT_REPOSITORY "https://github.com/TartanLlama/ranges"
  GIT_TAG "79a31ef7842ce0356595d3594984bbf43195c860"
  OPTIONS
    "RANGES_BUILD_PACKAGE OFF"
    "RANGES_BUILD_TESTS OFF"
)


add_executable(brep_sweep src/main.cpp)

target_link_libraries(brep_sweep Eigen3::Eigen tl::generator tl::ranges)
target_include_directories(brep_sweep PUBLIC include)
target_precompile_headers(brep_sweep PRIVATE 
    <Eigen/Core>)

target_compile_features(brep_sweep PUBLIC cxx_std_20)
set_target_properties(brep_sweep PROPERTIES CXX_EXTENSIONS OFF)