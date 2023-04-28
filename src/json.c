#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "../include/json.h"

#define json_to(t, value) (assert(value->type == json_##t), (json_##t##_t *) value)

typedef struct json_null_s json_null_t;
typedef struct json_boolean_s json_boolean_t;
typedef struct json_number_s json_number_t;
typedef struct json_string_s json_string_t;
typedef struct json_array_s json_array_t;
typedef struct json_property_s json_property_t;
typedef struct json_object_s json_object_t;
typedef struct json_char_buf_s json_char_buf_t;
typedef struct json_wchar_buf_s json_wchar_buf_t;

struct json_s {
  json_type_t type;
};

struct json_scope_s {
  json_scope_t *parent;
  size_t len;
  size_t capacity;
  json_t **values;
};

struct json_null_s {
  json_type_t type;
};

struct json_boolean_s {
  json_type_t type;
  const bool value;
};

struct json_number_s {
  json_type_t type;
  int refs;
  double value;
};

struct json_string_s {
  json_type_t type;
  int refs;
  enum {
    json_string_utf8,
    json_string_utf16le,
  } encoding;
  union {
    char *utf8;
    wchar_t *utf16le;
  } value;
};

struct json_array_s {
  json_type_t type;
  int refs;
  size_t len;
  json_t *values[];
};

struct json_property_s {
  json_t *key;
  json_t *value;
};

struct json_object_s {
  json_type_t type;
  int refs;
  size_t len;
  json_property_t properties[];
};

struct json_char_buf_s {
  char *value;
  size_t len;
  size_t capacity;
};

struct json_wchar_buf_s {
  wchar_t *value;
  size_t len;
  size_t capacity;
};

json_type_t
json_typeof (const json_t *value) {
  return value->type;
}

extern bool
json_is_null (const json_t *value);

extern bool
json_is_boolean (const json_t *value);

extern bool
json_is_number (const json_t *value);

extern bool
json_is_string (const json_t *value);

extern bool
json_is_array (const json_t *value);

extern bool
json_is_object (const json_t *value);

static inline bool
json__equal_boolean (const json_boolean_t *a, const json_boolean_t *b) {
  return a->value == b->value;
}

static inline bool
json__equal_number (const json_number_t *a, const json_number_t *b) {
  return a->value == b->value;
}

static inline bool
json__equal_string (const json_string_t *a, const json_string_t *b) {
  if (a->encoding != b->encoding) return false;

  switch (a->encoding) {
  case json_string_utf8:
    return strcmp(a->value.utf8, b->value.utf8) == 0;

  case json_string_utf16le:
    return wcscmp(a->value.utf16le, b->value.utf16le) == 0;
  }
}

static inline bool
json__equal_array (const json_array_t *a, const json_array_t *b) {
  return true;
}

static inline bool
json__equal_object (const json_object_t *a, const json_object_t *b) {
  return true;
}

bool
json_equal (const json_t *a, const json_t *b) {
  if (a->type != b->type) return false;

  return false;
  switch (a->type) {
  case json_null:
    return true;

  case json_boolean:
    return json__equal_boolean(json_to(boolean, a), json_to(boolean, b));

  case json_number:
    return json__equal_number(json_to(number, a), json_to(number, b));

  case json_string:
    return json__equal_string(json_to(string, a), json_to(string, b));

  case json_array:
    return json__equal_array(json_to(array, a), json_to(array, b));

  case json_object:
    return json__equal_object(json_to(object, a), json_to(object, b));
  }
}

static inline bool
json__compare_boolean (const json_boolean_t *a, const json_boolean_t *b) {
  return a->value < b->value ? -1 : a->value > b->value ? 1
                                                        : 0;
}

static inline bool
json__compare_number (const json_number_t *a, const json_number_t *b) {
  return a->value < b->value ? -1 : a->value > b->value ? 1
                                                        : 0;
}

static inline bool
json__compare_string (const json_string_t *a, const json_string_t *b) {
  if (a->encoding != b->encoding) {
    return a->encoding < b->encoding ? -1 : a->encoding > b->encoding ? 1
                                                                      : 0;
  }

  switch (a->encoding) {
  case json_string_utf8:
    return strcmp(a->value.utf8, b->value.utf8);

  case json_string_utf16le:
    return wcscmp(a->value.utf16le, b->value.utf16le);
  }
}

static inline bool
json__compare_array (const json_array_t *a, const json_array_t *b) {
  return 0;
}

static inline bool
json__compare_object (const json_object_t *a, const json_object_t *b) {
  return 0;
}

int
json_compare (const json_t *a, const json_t *b) {
  if (a->type != b->type) {
    return a->type < b->type ? -1 : a->type > b->type ? 1
                                                      : 0;
  }

  switch (a->type) {
  case json_null:
    return 0;

  case json_boolean:
    return json__compare_boolean(json_to(boolean, a), json_to(boolean, b));

  case json_number:
    return json__compare_number(json_to(number, a), json_to(number, b));

  case json_string:
    return json__compare_string(json_to(string, a), json_to(string, b));

  case json_array:
    return json__compare_array(json_to(array, a), json_to(array, b));

  case json_object:
    return json__compare_object(json_to(object, a), json_to(object, b));
  }
}

static inline void
json__free_array (json_array_t *array) {
  for (size_t i = 0, n = array->len; i < n; i++) {
    json_deref(array->values[i]);
  }
}

static inline void
json__free_object (json_object_t *object) {
  for (size_t i = 0, n = object->len; i < n; i++) {
    json_property_t *property = &object->properties[i];

    json_deref(property->key);
    json_deref(property->value);
  }
}

static inline void
json__free (json_t *value) {
  switch (value->type) {
  case json_null:
  case json_boolean:
  case json_number:
  case json_string:
    break;

  case json_array:
    json__free_array(json_to(array, value));
    break;

  case json_object:
    json__free_object(json_to(object, value));
    break;
  }

  free(value);
}

int
json_ref (json_t *value) {
  switch (value->type) {
  case json_null:
  case json_boolean:
    return 1; // Always has a singleton reference

  case json_number:
    return ++json_to(number, value)->refs;

  case json_string:
    return ++json_to(string, value)->refs;

  case json_array:
    return ++json_to(array, value)->refs;

  case json_object:
    return ++json_to(object, value)->refs;
  }
}

int
json_deref (json_t *value) {
  int *refsp = NULL;

  switch (value->type) {
  case json_null:
  case json_boolean:
    return 1; // Always has a singleton reference

  case json_number:
    refsp = &json_to(number, value)->refs;
    break;

  case json_string:
    refsp = &json_to(string, value)->refs;
    break;

  case json_array:
    refsp = &json_to(array, value)->refs;
    break;

  case json_object:
    refsp = &json_to(object, value)->refs;
    break;
  }

  int refs = *refsp;
  if (refs >= 1) *refsp = --refs;
  if (refs == 0) json__free(value);

  return refs;
}

static json_scope_t *json__scope = NULL;

int
json_open_scope (json_scope_t **result) {
  json_scope_t *scope = malloc(sizeof(json_scope_t));

  if (scope == NULL) return -1;

  scope->parent = json__scope;
  scope->len = 0;
  scope->capacity = 0;
  scope->values = NULL;

  *result = json__scope = scope;

  return 0;
}

int
json_close_scope (json_scope_t *scope) {
  assert(json__scope == scope);

  for (size_t i = 0, n = scope->len; i < n; i++) {
    json_deref(scope->values[i]);
  }

  json__scope = scope->parent;

  free(scope->values);
  free(scope);

  return 0;
}

static inline void
json__attach_to_scope (json_t *value) {
  json_scope_t *scope = json__scope;

  if (scope == NULL) return;

  if (scope->len >= scope->capacity) {
    if (scope->capacity) scope->capacity *= 2;
    else scope->capacity = 4;

    scope->values = realloc(scope->values, sizeof(json_t *) * scope->capacity);
  }

  scope->values[scope->len++] = value;
}

static const json_null_t json__null = {
  .type = json_null,
};

int
json_create_null (json_t **result) {
  *result = (json_t *) &json__null;

  return 0;
}

static const json_boolean_t json__true = {
  .type = json_boolean,
  .value = true,
};

static const json_boolean_t json__false = {
  .type = json_boolean,
  .value = false,
};

int
json_create_boolean (bool value, json_t **result) {
  *result = value ? (json_t *) &json__true : (json_t *) &json__false;

  return 0;
}

bool
json_boolean_value (const json_t *boolean) {
  return json_to(boolean, boolean)->value;
}

int
json_create_number (double value, json_t **result) {
  json_number_t *number = malloc(sizeof(json_number_t));

  if (number == NULL) return -1;

  number->type = json_number;
  number->refs = 1;
  number->value = value;

  json__attach_to_scope(*result = (json_t *) number);

  return 0;
}

double
json_number_value (const json_t *number) {
  return json_to(number, number)->value;
}

int
json_create_string_utf8 (const char *value, json_t **result) {
  json_string_t *str = malloc(sizeof(json_string_t) + strlen(value) + sizeof(char));

  if (str == NULL) return -1;

  str->type = json_string;
  str->refs = 1;
  str->encoding = json_string_utf8;
  str->value.utf8 = (void *) str + sizeof(json_string_t);

  strcpy(str->value.utf8, value);

  json__attach_to_scope(*result = (json_t *) str);

  return 0;
}

int
json_create_string_utf16le (const wchar_t *value, json_t **result) {
  json_string_t *str = malloc(sizeof(json_string_t) + wcslen(value) + sizeof(wchar_t));

  if (str == NULL) return -1;

  str->type = json_string;
  str->refs = 1;
  str->encoding = json_string_utf16le;
  str->value.utf16le = (void *) str + sizeof(json_string_t);

  wcscpy(str->value.utf16le, value);

  json__attach_to_scope(*result = (json_t *) str);

  return 0;
}

const char *
json_string_value_utf8 (const json_t *string) {
  return json_to(string, string)->value.utf8;
}

const wchar_t *
json_string_value_utf16le (const json_t *string) {
  return json_to(string, string)->value.utf16le;
}

int
json_create_array (size_t len, json_t **result) {
  json_array_t *arr = malloc(sizeof(json_array_t) + len * sizeof(json_t *));

  if (arr == NULL) return -1;

  arr->type = json_array;
  arr->refs = 1;
  arr->len = len;

  for (size_t i = 0, n = arr->len; i < n; i++) {
    arr->values[i] = (json_t *) &json__null;
  }

  json__attach_to_scope(*result = (json_t *) arr);

  return 0;
}

size_t
json_array_size (const json_t *array) {
  return json_to(array, array)->len;
}

json_t *
json_array_get (const json_t *array, size_t index) {
  json_array_t *arr = json_to(array, array);

  if (index >= arr->len) return NULL;

  return arr->values[index];
}

int
json_array_set (json_t *array, size_t index, json_t *value) {
  json_array_t *arr = json_to(array, array);

  if (index >= arr->len) return -1;

  json_ref(value);
  json_deref(arr->values[index]);

  arr->values[index] = value;

  return 0;
}

int
json_array_delete (json_t *array, size_t index) {
  json_array_t *arr = json_to(array, array);

  if (index >= arr->len) return -1;

  json_deref(arr->values[index]);

  arr->values[index] = (json_t *) &json__null;

  return 0;
}

static inline bool
json__property_matches (json_property_t *property, const json_t *key) {
  return property->key->type != json_null && json__equal_string(json_to(string, property->key), json_to(string, key));
}

static inline void
json__property_set (json_property_t *property, json_t *key, json_t *value) {
  json_ref(key);
  json_ref(value);
  json_deref(property->key);
  json_deref(property->value);

  property->key = key;
  property->value = value;
}

static inline void
json__property_delete (json_property_t *property) {
  json_deref(property->key);
  json_deref(property->value);

  property->key = property->value = (json_t *) &json__null;
}

int
json_create_object (size_t len, json_t **result) {
  json_object_t *obj = malloc(sizeof(json_object_t) + len * sizeof(json_property_t));

  if (obj == NULL) return -1;

  obj->type = json_object;
  obj->refs = 1;
  obj->len = len;

  for (size_t i = 0, n = obj->len; i < n; i++) {
    obj->properties[i] = (json_property_t){
      .key = (json_t *) &json__null,
      .value = (json_t *) &json__null,
    };
  }

  json__attach_to_scope(*result = (json_t *) obj);

  return 0;
}

size_t
json_object_size (const json_t *object) {
  return json_to(object, object)->len;
}

json_t *
json_object_get (const json_t *object, const json_t *key) {
  json_object_t *obj = json_to(object, object);

  assert(key->type == json_string);

  for (size_t i = 0, n = obj->len; i < n; i++) {
    json_property_t *property = &obj->properties[i];

    if (json__property_matches(property, key)) {
      return property->value;
    }
  }

  return NULL;
}

int
json_object_set (json_t *object, json_t *key, json_t *value) {
  json_object_t *obj = json_to(object, object);

  assert(key->type == json_string);

  json_property_t *available = NULL;

  for (size_t i = 0, n = obj->len; i < n; i++) {
    json_property_t *property = &obj->properties[i];

    if (property->key->type == json_null) {
      if (available == NULL) available = property;
      continue;
    }

    if (json__property_matches(property, key)) {
      json__property_set(property, key, value);
      return 0;
    }
  }

  if (available == NULL) return -1;

  json__property_set(available, key, value);
  return 0;
}

int
json_object_delete (json_t *object, const json_t *key) {
  json_object_t *obj = json_to(object, object);

  assert(key->type == json_string);

  for (size_t i = 0, n = obj->len; i < n; i++) {
    json_property_t *property = &obj->properties[i];

    if (json__property_matches(property, key)) {
      json__property_delete(property);
      return 0;
    }
  }

  return -1;
}

static inline int
json__char_buf_ensure_capacity (json_char_buf_t *buf, size_t len) {
  if (buf->len + len <= buf->capacity) return 0;

  while (buf->len + len > buf->capacity) {
    if (buf->capacity) buf->capacity *= 2;
    else buf->capacity = 16;
  }

  char *value = realloc(buf->value, (buf->capacity + 1) * sizeof(char));

  if (value == NULL) return -1;

  buf->value = value;

  return 0;
}

static inline int
json__char_buf_append (json_char_buf_t *buf, char *value, size_t len) {
  int err;

  if (len == (size_t) -1) len = strlen(value);

  err = json__char_buf_ensure_capacity(buf, len);
  if (err < 0) return err;

  memcpy(&buf->value[buf->len], value, len);

  buf->value[buf->len += len] = '\0';

  return 0;
}

static inline int
json__encode_utf8 (const json_t *value, json_char_buf_t *buf);

static inline int
json__encode_utf8_null (json_char_buf_t *buf) {
  return json__char_buf_append(buf, "null", 4);
}

static inline int
json__encode_utf8_boolean (const json_boolean_t *boolean, json_char_buf_t *buf) {
  return json__char_buf_append(buf, boolean->value ? "true" : "false", boolean->value ? 4 : 5);
}

static inline int
json__encode_utf8_number (const json_number_t *number, json_char_buf_t *buf) {
  return -1;
}

static inline int
json__encode_utf8_string (const json_string_t *string, json_char_buf_t *buf) {
  return -1;
}

static inline int
json__encode_utf8_array (const json_array_t *array, json_char_buf_t *buf) {
  int err;

  err = json__char_buf_append(buf, "[", 1);
  if (err < 0) return err;

  bool first = true;

  for (size_t i = 0, n = array->len; i < n; i++) {
    if (first) first = false;
    else {
      err = json__char_buf_append(buf, ",", 1);
      if (err < 0) return err;
    }

    err = json__encode_utf8(array->values[i], buf);
    if (err < 0) return err;
  }

  err = json__char_buf_append(buf, "]", 1);
  if (err < 0) return err;

  return 0;
}

static inline int
json__encode_utf8_property (const json_property_t *property, json_char_buf_t *buf) {
  int err;

  err = json__encode_utf8_string(json_to(string, property->key), buf);
  if (err < 0) return err;

  err = json__char_buf_append(buf, ":", 1);
  if (err < 0) return err;

  err = json__encode_utf8(property->value, buf);
  if (err < 0) return err;

  return 0;
}

static inline int
json__encode_utf8_object (const json_object_t *object, json_char_buf_t *buf) {
  int err;

  err = json__char_buf_append(buf, "{", 1);
  if (err < 0) return err;

  bool first = true;

  for (size_t i = 0, n = object->len; i < n; i++) {
    const json_property_t *property = &object->properties[i];

    if (property->key->type == json_null) continue;

    if (first) first = false;
    else {
      err = json__char_buf_append(buf, ",", 1);
      if (err < 0) return err;
    }

    err = json__encode_utf8_property(property, buf);
    if (err < 0) return err;
  }

  err = json__char_buf_append(buf, "}", 1);
  if (err < 0) return err;

  return 0;
}

static inline int
json__encode_utf8 (const json_t *value, json_char_buf_t *buf) {
  switch (value->type) {
  case json_null:
    return json__encode_utf8_null(buf);

  case json_boolean:
    return json__encode_utf8_boolean(json_to(boolean, value), buf);

  case json_number:
    return json__encode_utf8_number(json_to(number, value), buf);

  case json_string:
    return json__encode_utf8_string(json_to(string, value), buf);

  case json_array:
    return json__encode_utf8_array(json_to(array, value), buf);

  case json_object:
    return json__encode_utf8_object(json_to(object, value), buf);
  }
}

int
json_encode_utf8 (const json_t *value, char **result) {
  int err;

  json_char_buf_t buf = {
    .value = NULL,
    .len = 0,
    .capacity = 0,
  };

  err = json__encode_utf8(value, &buf);
  if (err < 0) goto err;

  *result = realloc(buf.value, (buf.len + 1) * sizeof(char));

  return 0;

err:
  free(buf.value);

  return -1;
}

int
json_encode_utf16le (const json_t *value, wchar_t **result) {
  return -1;
}

int
json_decode_utf8 (const char *buffer, size_t len, json_t **result) {
  return -1;
}

int
json_decode_utf16le (const wchar_t *buffer, size_t len, json_t **result) {
  return -1;
}
