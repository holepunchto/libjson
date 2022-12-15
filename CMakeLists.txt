cmake_minimum_required(VERSION 3.22)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(WIN32)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

project(json C)

add_library(json OBJECT)

set_target_properties(
  json
  PROPERTIES
  C_STANDARD 99
  POSITION_INDEPENDENT_CODE 1
)

target_sources(
  json
  INTERFACE
    include/json.h
  PRIVATE
    src/json.c
)

target_include_directories(
  json
  PUBLIC
    include
)

add_library(json_shared SHARED $<TARGET_OBJECTS:json>)

set_target_properties(
  json_shared
  PROPERTIES
  OUTPUT_NAME json
)

add_library(json_static STATIC $<TARGET_OBJECTS:json>)

set_target_properties(
  json_static
  PROPERTIES
  OUTPUT_NAME json
)

install(TARGETS json_shared json_static)

install(FILES include/json.h DESTINATION include)

if(PROJECT_IS_TOP_LEVEL)
  enable_testing()
  add_subdirectory(test)
endif()
