cmake_minimum_required(VERSION 3.10)
project(mock_runtime VERSION 1.0
  DESCRIPTION "A \"Pretty Good\" (tm) mock runtime for DASH applications"
  LANGUAGES CXX)

# First, build libdash for use with mock runtime
set(RUNTIME_BUILD 1)
add_subdirectory(libdash)

# Then, build mock runtime
set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/mock_runtime.cpp)
set(INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/libdash)
set(LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libdash/libdash-rt.a dl pthread)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/exported.txt ${CMAKE_CURRENT_BINARY_DIR}/exported.txt COPYONLY)

add_executable(mock_runtime ${SOURCES})
add_dependencies(mock_runtime dash)
target_include_directories(mock_runtime PRIVATE ${INCLUDES})
target_link_libraries(mock_runtime PRIVATE ${LIBRARIES})

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--dynamic-list=./exported.txt")
set_target_properties(mock_runtime PROPERTIES LINK_FLAGS "-Wl,--dynamic-list=./exported.txt")
