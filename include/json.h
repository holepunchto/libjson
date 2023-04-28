#ifndef JSON_H
#define JSON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

typedef enum {
  json_null,
  json_true,
  json_false,
  json_number,
  json_string,
  json_array,
  json_object,
} json_type_t;

typedef struct json_s json_t;
typedef struct json_scope_s json_scope_t;

json_type_t
json_typeof (const json_t *value);

inline bool
json_is_null (const json_t *value) {
  return json_typeof(value) == json_null;
}

inline bool
json_is_true (const json_t *value) {
  return json_typeof(value) == json_true;
}

inline bool
json_is_false (const json_t *value) {
  return json_typeof(value) == json_false;
}

inline bool
json_is_boolean (const json_t *value) {
  return json_is_true(value) || json_is_false(value);
}

inline bool
json_is_number (const json_t *value) {
  return json_typeof(value) == json_number;
}

inline bool
json_is_string (const json_t *value) {
  return json_typeof(value) == json_string;
}

inline bool
json_is_array (const json_t *value) {
  return json_typeof(value) == json_array;
}

inline bool
json_is_object (const json_t *value) {
  return json_typeof(value) == json_object;
}

bool
json_equal (const json_t *a, const json_t *b);

int
json_compare (const json_t *a, const json_t *b);

int
json_ref (json_t *value);

int
json_deref (json_t *value);

int
json_open_scope (json_scope_t **result);

int
json_close_scope (json_scope_t *scope);

int
json_create_null (json_t **result);

int
json_create_boolean (bool value, json_t **result);

bool
json_boolean_value (const json_t *boolean);

int
json_create_number (double value, json_t **result);

double
json_number_value (const json_t *number);

int
json_create_string (const char *value, json_t **result);

const char *
json_string_value (const json_t *string);

int
json_create_array (size_t len, json_t **result);

size_t
json_array_size (const json_t *array);

json_t *
json_array_get (const json_t *array, size_t index);

int
json_array_set (json_t *array, size_t index, json_t *value);

int
json_array_delete (json_t *array, size_t index);

int
json_create_object (size_t len, json_t **result);

size_t
json_object_size (const json_t *object);

json_t *
json_object_get (const json_t *object, const json_t *key);

int
json_object_set (json_t *object, json_t *key, json_t *value);

int
json_object_delete (json_t *object, const json_t *key);

int
json_encode (const json_t *value, char **result);

int
json_decode (const char *buffer, size_t len, json_t **result);

#ifdef __cplusplus
}
#endif

#endif // JSON_H
