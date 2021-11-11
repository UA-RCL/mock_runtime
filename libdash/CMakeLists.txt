cmake_minimum_required(VERSION 3.10)
project(lib_dash VERSION 1.0
  DESCRIPTION "A library of functions for use with DASH applications"
  LANGUAGES CXX)

message(STATUS "Building libdash")

set(LIBDASH_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/dash.cpp)

find_library(GSL libgsl.a)
find_library(GSLCBLAS libgslcblas.a)
find_library(MATH libm.a)
find_library(MATH_SO libm.so)

add_library(dash_base OBJECT ${LIBDASH_SRCS})

if (${RUNTIME_BUILD})
  message(STATUS "Building to rely on availability of a runtime-provided enqueue_kernel function")
  set(SUFFIX "-rt")
else()
  message(STATUS "Building to operate as a standalone, CPU-only library")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCPU_ONLY")
  string(STRIP "${CMAKE_CXX_FLAGS}" CMAKE_CXX_FLAGS)
endif()

add_custom_target(dash
  ALL
  COMMAND ${CMAKE_AR} rcsT libdash${SUFFIX}.a $<TARGET_OBJECTS:dash_base> ${GSL} ${GSLCBLAS} ${MATH}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS dash_base
)

add_custom_target(dash_so
  ALL
  COMMAND ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_FLAGS} -shared -fPIC ${LIBDASH_SRCS} ${GSL} ${GSLCBLAS} ${MATH_SO} -o libdash${SUFFIX}.so
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
