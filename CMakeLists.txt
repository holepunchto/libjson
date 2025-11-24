cmake_minimum_required(VERSION 4.0)

find_package(cmake-fetch REQUIRED PATHS node_modules/cmake-fetch)

project(json C)

fetch_package("github:holepunchto/libutf")

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
    $<TARGET_PROPERTY:utf,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(json_shared SHARED)

set_target_properties(
  json_shared
  PROPERTIES
  OUTPUT_NAME json
  WINDOWS_EXPORT_ALL_SYMBOLS ON
)

target_link_libraries(
  json_shared
  PUBLIC
    json
    utf_shared
)

add_library(json_static STATIC)

set_target_properties(
  json_static
  PROPERTIES
  OUTPUT_NAME json
  PREFIX lib
)

target_link_libraries(
  json_static
  PUBLIC
    json
    utf_static
)

install(TARGETS json_shared json_static)

install(FILES include/json.h DESTINATION include)

if(PROJECT_IS_TOP_LEVEL)
  enable_testing()

  add_subdirectory(test)
endif()
