list(APPEND tests
  decode-utf8-array
  decode-utf8-array-empty
  decode-utf8-false
  decode-utf8-null
  decode-utf8-object
  decode-utf8-object-empty
  decode-utf8-string
  decode-utf8-string-empty
  decode-utf8-string-escape
  decode-utf8-true
)

foreach(test IN LISTS tests)
  add_executable(${test} ${test}.c)

  target_link_libraries(
    ${test}
    PRIVATE
      json_static
  )

  target_include_directories(
    ${test}
    PRIVATE
      $<TARGET_PROPERTY:json,INTERFACE_INCLUDE_DIRECTORIES>
  )

  add_test(
    NAME ${test}
    COMMAND ${test}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  )

  set_tests_properties(
    ${test}
    PROPERTIES
    TIMEOUT 30
  )
endforeach()
