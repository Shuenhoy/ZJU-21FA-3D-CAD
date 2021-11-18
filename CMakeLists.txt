
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(
  brep_sweep
  VERSION 0.1.0
  LANGUAGES CXX
)

include(cmake/get_cpm.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)



CPMAddPackage(
  NAME "clip2tri"
  GIT_REPOSITORY "https://github.com/raptor/clip2tri.git"
  GIT_TAG "f62a734d22733814b8a970ed8a68a4d94c24fa5f"
  PATCH_COMMAND git restore . && git apply ${CMAKE_CURRENT_SOURCE_DIR}/clip2tri.patch && echo "patch applied"
  OPTIONS
    "BUILD_SHARED_LIBS OFF"
)
CPMAddPackage(
  NAME "Eigen3"
  GIT_REPOSITORY "https://github.com/hexagon-geo-surv/eigen.git"
  GIT_TAG "3.4"
  OPTIONS
    "BUILD_TESTING OFF"
    "EIGEN_BUILD_DOC OFF"  
    "EIGEN_LEAVE_TEST_IN_ALL_TARGET OFF"
    "EIGEN_BUILD_PKGCONFIG OFF"
)

CPMAddPackage(
  NAME "libigl"
  GIT_REPOSITORY "https://github.com/libigl/libigl.git"
  GIT_TAG "v2.3.0"
  OPTIONS
    "LIBIGL_USE_STATIC_LIBRARY OFF"
    "LIBIGL_WITH_COMISO OFF"
    "LIBIGL_WITH_EMBREE OFF"
    "LIBIGL_WITH_PNG OFF"
    "LIBIGL_WITH_PNG OFF"
    "LIBIGL_WITH_TRIANGLE OFF"
    "LIBIGL_WITH_PREDICATES OFF"
    "LIBIGL_WITH_TETGEN OFF"
    "LIBIGL_WITH_XML OFF"
    "LIBIGL_WITH_PYTHON OFF"
)

CPMAddPackage(
  NAME "exprtk"
  GIT_REPOSITORY "https://github.com/ArashPartow/exprtk.git"
  GIT_TAG "93a9f44f99b910bfe07cd1e933371e83cea3841c"
  PATCH_COMMAND git clean -f && git restore . && git apply ${CMAKE_CURRENT_SOURCE_DIR}/exprtk.patch && echo "patch applied"
)

add_executable(brep_sweep src/main.cpp src/sweep_expr.cpp src/triangulate.cpp src/sweeping.cpp)

target_link_libraries(brep_sweep 
  PUBLIC
    Eigen3::Eigen
    igl::core igl::opengl_glfw
    clip2tri-static
    exprtk
)
target_include_directories(brep_sweep PUBLIC include)
target_precompile_headers(brep_sweep PRIVATE 
    <Eigen/Core>)

target_compile_features(brep_sweep PUBLIC cxx_std_20)
set_target_properties(brep_sweep PROPERTIES CXX_EXTENSIONS OFF)