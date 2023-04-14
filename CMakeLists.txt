cmake_minimum_required(VERSION 3.25)

project(json C)

add_library(json OBJECT)

set_target_properties(
  json
  PROPERTIES
  C_STANDARD 99
  POSITION_INDEPENDENT_CODE ON
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
  WINDOWS_EXPORT_ALL_SYMBOLS ON
)

add_library(json_static STATIC $<TARGET_OBJECTS:json>)

set_target_properties(
  json_static
  PROPERTIES
  OUTPUT_NAME json
  PREFIX lib
)

install(TARGETS json_shared json_static)

install(FILES include/json.h DESTINATION include)

if(PROJECT_IS_TOP_LEVEL)
  enable_testing()
  add_subdirectory(test)
endif()
