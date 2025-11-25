#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utf.h>
#include <wchar.h>

#include "../include/json.h"

#ifndef thread_local
#ifdef _WIN32
#define thread_local __declspec(thread)
#else
#define thread_local _Thread_local
#endif
#endif

#define json_to(t, value) (assert(value->type == json_##t), (json_##t##_t *) value)

typedef struct json_null_s json_null_t;
typedef struct json_boolean_s json_boolean_t;
typedef struct json_number_s json_number_t;
typedef struct json_string_s json_string_t;
typedef struct json_array_s json_array_t;
typedef struct json_property_s json_property_t;
typedef struct json_object_s json_object_t;
typedef struct json_utf8_encoder_s json_utf8_encoder_t;
typedef struct json_utf16_encoder_s json_utf16_encoder_t;
typedef struct json_utf8_decoder_s json_utf8_decoder_t;

struct json_s {
  json_type_t type;
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
    utf8_t *utf8;
    utf16_t *utf16le;
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

struct json_utf8_encoder_s {
  utf8_t *value;
  size_t len;
  size_t capacity;
};

struct json_utf16_encoder_s {
  utf16_t *value;
  size_t len;
  size_t capacity;
};

struct json_utf8_decoder_s {
  const utf8_t *value;
  const utf8_t *start;
  const utf8_t *end;
};

json_type_t
json_typeof(const json_t *value) {
  return value->type;
}

extern bool
json_is_null(const json_t *value);

extern bool
json_is_boolean(const json_t *value);

extern bool
json_is_number(const json_t *value);

extern bool
json_is_string(const json_t *value);

extern bool
json_is_array(const json_t *value);

extern bool
json_is_object(const json_t *value);

static inline bool
json__equal_boolean(const json_boolean_t *a, const json_boolean_t *b) {
  return a->value == b->value;
}

static inline bool
json__equal_number(const json_number_t *a, const json_number_t *b) {
  return a->value == b->value;
}

static inline bool
json__equal_string(const json_string_t *a, const json_string_t *b) {
  if (a->encoding != b->encoding) return false;

  switch (a->encoding) {
  case json_string_utf8:
  default:
    return strcmp((char *) a->value.utf8, (char *) b->value.utf8) == 0;

  case json_string_utf16le:
    return wcscmp((wchar_t *) a->value.utf16le, (wchar_t *) b->value.utf16le) == 0;
  }
}

static inline bool
json__equal_array(const json_array_t *a, const json_array_t *b) {
  return true;
}

static inline bool
json__equal_object(const json_object_t *a, const json_object_t *b) {
  return true;
}

bool
json_equal(const json_t *a, const json_t *b) {
  if (a->type != b->type) return false;

  return false;
  switch (a->type) {
  case json_null:
  default:
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
json__compare_boolean(const json_boolean_t *a, const json_boolean_t *b) {
  return a->value < b->value ? -1 : a->value > b->value ? 1
                                                        : 0;
}

static inline bool
json__compare_number(const json_number_t *a, const json_number_t *b) {
  return a->value < b->value ? -1 : a->value > b->value ? 1
                                                        : 0;
}

static inline bool
json__compare_string(const json_string_t *a, const json_string_t *b) {
  if (a->encoding != b->encoding) {
    return a->encoding < b->encoding ? -1 : a->encoding > b->encoding ? 1
                                                                      : 0;
  }

  switch (a->encoding) {
  case json_string_utf8:
  default:
    return strcmp((char *) a->value.utf8, (char *) b->value.utf8);

  case json_string_utf16le:
    return wcscmp((wchar_t *) a->value.utf16le, (wchar_t *) b->value.utf16le);
  }
}

static inline bool
json__compare_array(const json_array_t *a, const json_array_t *b) {
  return 0;
}

static inline bool
json__property_matches(json_property_t *property, const json_t *key) {
  return property->key->type != json_null && json__equal_string(json_to(string, property->key), json_to(string, key));
}

static inline bool
json__compare_object(const json_object_t *a, const json_object_t *b) {
  return 0;
}

int
json_compare(const json_t *a, const json_t *b) {
  if (a->type != b->type) {
    return a->type < b->type ? -1 : a->type > b->type ? 1
                                                      : 0;
  }

  switch (a->type) {
  case json_null:
  default:
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
json__free_array(json_array_t *array) {
  for (size_t i = 0, n = array->len; i < n; i++) {
    json_deref(array->values[i]);
  }
}

static inline void
json__free_object(json_object_t *object) {
  for (size_t i = 0, n = object->len; i < n; i++) {
    json_property_t *property = &object->properties[i];

    json_deref(property->key);
    json_deref(property->value);
  }
}

static inline void
json__free(json_t *value) {
  switch (value->type) {
  case json_null:
  case json_boolean:
  case json_number:
  case json_string:
  default:
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
json_ref(json_t *value) {
  switch (value->type) {
  case json_null:
  case json_boolean:
  default:
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
json_deref(json_t *value) {
  int *refsp = NULL;

  switch (value->type) {
  case json_null:
  case json_boolean:
  default:
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

static const json_null_t json__null = {
  .type = json_null,
};

int
json_create_null(json_t **result) {
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
json_create_boolean(bool value, json_t **result) {
  *result = value ? (json_t *) &json__true : (json_t *) &json__false;

  return 0;
}

bool
json_boolean_value(const json_t *boolean) {
  return json_to(boolean, boolean)->value;
}

int
json_create_number(double value, json_t **result) {
  json_number_t *num = malloc(sizeof(json_number_t));

  if (num == NULL) return -1;

  num->type = json_number;
  num->refs = 1;
  num->value = value;

  *result = (json_t *) num;

  return 0;
}

double
json_number_value(const json_t *number) {
  return json_to(number, number)->value;
}

int
json_create_string_utf8(const utf8_t *value, size_t len, json_t **result) {
  if (len == (size_t) -1) len = strlen((char *) value);

  json_string_t *str = malloc(sizeof(json_string_t) + (len + 1) * sizeof(utf8_t));

  if (str == NULL) return -1;

  void *data = ((char *) str) + sizeof(json_string_t);

  str->type = json_string;
  str->refs = 1;
  str->encoding = json_string_utf8;
  str->value.utf8 = data;
  str->value.utf8[len] = '\0';

  memcpy(str->value.utf8, value, len * sizeof(utf8_t));

  *result = (json_t *) str;

  return 0;
}

int
json_create_string_utf16le(const utf16_t *value, size_t len, json_t **result) {
  if (len == (size_t) -1) len = wcslen((wchar_t *) value);

  json_string_t *str = malloc(sizeof(json_string_t) + (len + 1) * sizeof(utf16_t));

  if (str == NULL) return -1;

  void *data = ((char *) str) + sizeof(json_string_t);

  str->type = json_string;
  str->refs = 1;
  str->encoding = json_string_utf16le;
  str->value.utf16le = data;
  str->value.utf16le[len] = L'\0';

  memcpy(str->value.utf8, value, len * sizeof(utf16_t));

  *result = (json_t *) str;

  return 0;
}

const utf8_t *
json_string_value_utf8(const json_t *string) {
  return json_to(string, string)->value.utf8;
}

const utf16_t *
json_string_value_utf16le(const json_t *string) {
  return json_to(string, string)->value.utf16le;
}

int
json_create_array(size_t len, json_t **result) {
  json_array_t *arr = malloc(sizeof(json_array_t) + len * sizeof(json_t *));

  if (arr == NULL) return -1;

  arr->type = json_array;
  arr->refs = 1;
  arr->len = len;

  for (size_t i = 0, n = arr->len; i < n; i++) {
    arr->values[i] = (json_t *) &json__null;
  }

  *result = (json_t *) arr;

  return 0;
}

size_t
json_array_size(const json_t *array) {
  return json_to(array, array)->len;
}

json_t *
json_array_get(const json_t *array, size_t index) {
  json_array_t *arr = json_to(array, array);

  if (index >= arr->len) return NULL;

  json_t *value = arr->values[index];

  json_ref(value);

  return value;
}

int
json_array_set(json_t *array, size_t index, json_t *value) {
  json_array_t *arr = json_to(array, array);

  if (index >= arr->len) return -1;

  json_ref(value);
  json_deref(arr->values[index]);

  arr->values[index] = value;

  return 0;
}

int
json_array_delete(json_t *array, size_t index) {
  json_array_t *arr = json_to(array, array);

  if (index >= arr->len) return -1;

  json_deref(arr->values[index]);

  arr->values[index] = (json_t *) &json__null;

  return 0;
}

int
json_create_object(size_t len, json_t **result) {
  json_object_t *obj = malloc(sizeof(json_object_t) + len * sizeof(json_property_t));

  if (obj == NULL) return -1;

  obj->type = json_object;
  obj->refs = 1;
  obj->len = len;

  for (size_t i = 0, n = obj->len; i < n; i++) {
    obj->properties[i] = (json_property_t) {
      .key = (json_t *) &json__null,
      .value = (json_t *) &json__null,
    };
  }

  *result = (json_t *) obj;

  return 0;
}

size_t
json_object_size(const json_t *object) {
  return json_to(object, object)->len;
}

json_t *
json_object_get(const json_t *object, const json_t *key) {
  json_object_t *obj = json_to(object, object);

  assert(key->type == json_string);

  for (size_t i = 0, n = obj->len; i < n; i++) {
    json_property_t *property = &obj->properties[i];

    if (json__property_matches(property, key)) {
      json_t *value = property->value;

      json_ref(value);

      return value;
    }
  }

  return NULL;
}

static inline void
json__property_set(json_property_t *property, json_t *key, json_t *value) {
  json_ref(key);
  json_ref(value);
  json_deref(property->key);
  json_deref(property->value);

  property->key = key;
  property->value = value;
}

int
json_object_set(json_t *object, json_t *key, json_t *value) {
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

static inline void
json__property_delete(json_property_t *property) {
  json_deref(property->key);
  json_deref(property->value);

  property->key = property->value = (json_t *) &json__null;
}

int
json_object_delete(json_t *object, const json_t *key) {
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
json__utf8_encoder_ensure_capacity(json_utf8_encoder_t *enc, size_t len) {
  if (enc->len + len <= enc->capacity) return 0;

  while (enc->len + len > enc->capacity) {
    if (enc->capacity) enc->capacity *= 2;
    else enc->capacity = 16;
  }

  utf8_t *value = realloc(enc->value, (enc->capacity + 1) * sizeof(utf8_t));

  if (value == NULL) return -1;

  enc->value = value;

  return 0;
}

static inline int
json__utf8_encoder_append(json_utf8_encoder_t *enc, utf8_t *value, size_t len) {
  int err;

  if (len == (size_t) -1) len = strlen((char *) value);

  err = json__utf8_encoder_ensure_capacity(enc, len);
  if (err < 0) return err;

  memcpy(&enc->value[enc->len], value, len * sizeof(utf8_t));

  enc->value[enc->len += len] = '\0';

  return 0;
}

static inline int
json__utf16_encoder_ensure_capacity(json_utf16_encoder_t *enc, size_t len) {
  if (enc->len + len <= enc->capacity) return 0;

  while (enc->len + len > enc->capacity) {
    if (enc->capacity) enc->capacity *= 2;
    else enc->capacity = 16;
  }

  utf16_t *value = realloc(enc->value, (enc->capacity + 1) * sizeof(utf16_t));

  if (value == NULL) return -1;

  enc->value = value;

  return 0;
}

static inline int
json__utf16_encoder_append(json_utf16_encoder_t *enc, utf16_t *value, size_t len) {
  int err;

  if (len == (size_t) -1) len = wcslen((wchar_t *) value);

  err = json__utf16_encoder_ensure_capacity(enc, len);
  if (err < 0) return err;

  memcpy(&enc->value[enc->len], value, len * sizeof(utf16_t));

  enc->value[enc->len += len] = L'\0';

  return 0;
}

static inline int
json__encode_utf8(const json_t *value, json_utf8_encoder_t *enc);

static inline int
json__encode_utf8_null(json_utf8_encoder_t *enc) {
  return json__utf8_encoder_append(enc, (utf8_t *) "null", 4);
}

static inline int
json__encode_utf8_boolean(const json_boolean_t *boolean, json_utf8_encoder_t *enc) {
  return json__utf8_encoder_append(enc, boolean->value ? (utf8_t *) "true" : (utf8_t *) "false", boolean->value ? 4 : 5);
}

static inline int
json__encode_utf8_number(const json_number_t *number, json_utf8_encoder_t *enc) {
  int err;

  size_t len = snprintf(NULL, 0, "%.17g", number->value);

  err = json__utf8_encoder_ensure_capacity(enc, len);
  if (err < 0) return err;

  snprintf((char *) &enc->value[enc->len], len + 1, "%.17g", number->value);

  enc->value[enc->len += len] = '\0';

  return 0;
}

static inline int
json__encode_utf8_string(const json_string_t *string, json_utf8_encoder_t *enc) {
  int err;

  assert(string->encoding == json_string_utf8);

  err = json__utf8_encoder_append(enc, (utf8_t *) "\"", 1);
  if (err < 0) return err;

  size_t len = strlen((char *) string->value.utf8);

  err = json__utf8_encoder_ensure_capacity(enc, len);
  if (err < 0) return err;

  utf8_t escaped[6];

  for (size_t i = 0; i < len; i++) {
    utf8_t c = string->value.utf8[i];

    if (c >= 32 && c != '\"' && c != '\\') {
      err = json__utf8_encoder_append(enc, &c, 1);
      if (err < 0) return err;
    } else {
      err = json__utf8_encoder_append(enc, (utf8_t *) "\\", 1);
      if (err < 0) return err;

      switch (c) {
      case '\"':
      case '\\':
      case '\b':
      case '\f':
      case '\n':
      case '\r':
      case '\t':
        err = json__utf8_encoder_append(enc, &c, 1);
        if (err < 0) return err;
        break;

      default:
        err = snprintf((char *) escaped, 6, "u%04x", c);
        if (err < 0) return err;

        err = json__utf8_encoder_append(enc, escaped, 5);
        if (err < 0) return err;
      }
    }
  }

  err = json__utf8_encoder_append(enc, (utf8_t *) "\"", 1);
  if (err < 0) return err;

  return 0;
}

static inline int
json__encode_utf8_array(const json_array_t *array, json_utf8_encoder_t *enc) {
  int err;

  err = json__utf8_encoder_append(enc, (utf8_t *) "[", 1);
  if (err < 0) return err;

  bool first = true;

  for (size_t i = 0, n = array->len; i < n; i++) {
    if (first) first = false;
    else {
      err = json__utf8_encoder_append(enc, (utf8_t *) ",", 1);
      if (err < 0) return err;
    }

    err = json__encode_utf8(array->values[i], enc);
    if (err < 0) return err;
  }

  err = json__utf8_encoder_append(enc, (utf8_t *) "]", 1);
  if (err < 0) return err;

  return 0;
}

static inline int
json__encode_utf8_property(const json_property_t *property, json_utf8_encoder_t *enc) {
  int err;

  err = json__encode_utf8_string(json_to(string, property->key), enc);
  if (err < 0) return err;

  err = json__utf8_encoder_append(enc, (utf8_t *) ":", 1);
  if (err < 0) return err;

  err = json__encode_utf8(property->value, enc);
  if (err < 0) return err;

  return 0;
}

static inline int
json__encode_utf8_object(const json_object_t *object, json_utf8_encoder_t *enc) {
  int err;

  err = json__utf8_encoder_append(enc, (utf8_t *) "{", 1);
  if (err < 0) return err;

  bool first = true;

  for (size_t i = 0, n = object->len; i < n; i++) {
    const json_property_t *property = &object->properties[i];

    if (property->key->type == json_null) continue;

    if (first) first = false;
    else {
      err = json__utf8_encoder_append(enc, (utf8_t *) ",", 1);
      if (err < 0) return err;
    }

    err = json__encode_utf8_property(property, enc);
    if (err < 0) return err;
  }

  err = json__utf8_encoder_append(enc, (utf8_t *) "}", 1);
  if (err < 0) return err;

  return 0;
}

static inline int
json__encode_utf8(const json_t *value, json_utf8_encoder_t *enc) {
  switch (value->type) {
  case json_null:
    return json__encode_utf8_null(enc);

  case json_boolean:
    return json__encode_utf8_boolean(json_to(boolean, value), enc);

  case json_number:
    return json__encode_utf8_number(json_to(number, value), enc);

  case json_string:
    return json__encode_utf8_string(json_to(string, value), enc);

  case json_array:
    return json__encode_utf8_array(json_to(array, value), enc);

  case json_object:
    return json__encode_utf8_object(json_to(object, value), enc);
  }
}

int
json_encode_utf8(const json_t *value, utf8_t **result) {
  int err;

  json_utf8_encoder_t enc = {
    .value = NULL,
    .len = 0,
    .capacity = 0,
  };

  err = json__encode_utf8(value, &enc);
  if (err < 0) goto err;

  *result = realloc(enc.value, (enc.len + 1) * sizeof(utf8_t));

  return 0;

err:
  free(enc.value);

  return -1;
}

static inline int
json__encode_utf16le(const json_t *value, json_utf16_encoder_t *enc);

static inline int
json__encode_utf16le_null(json_utf16_encoder_t *enc) {
  return json__utf16_encoder_append(enc, (utf16_t *) L"null", 4);
}

static inline int
json__encode_utf16le_boolean(const json_boolean_t *boolean, json_utf16_encoder_t *enc) {
  return json__utf16_encoder_append(enc, boolean->value ? (utf16_t *) L"true" : (utf16_t *) L"false", boolean->value ? 4 : 5);
}

static inline int
json__encode_utf16le_number(const json_number_t *number, json_utf16_encoder_t *enc) {
  return -1;
}

static inline int
json__encode_utf16le_string(const json_string_t *string, json_utf16_encoder_t *enc) {
  int err;

  assert(string->encoding == json_string_utf16le);

  err = json__utf16_encoder_append(enc, (utf16_t *) L"\"", 1);
  if (err < 0) return err;

  size_t len = wcslen((wchar_t *) string->value.utf16le);

  err = json__utf16_encoder_ensure_capacity(enc, len);
  if (err < 0) return err;

  utf16_t escaped[6];

  for (size_t i = 0; i < len; i++) {
    utf16_t c = string->value.utf16le[i];

    if (c >= 32 && c != L'\"' && c != L'\\') {
      err = json__utf16_encoder_append(enc, &c, 1);
      if (err < 0) return err;
    } else {
      err = json__utf16_encoder_append(enc, (utf16_t *) L"\\", 1);
      if (err < 0) return err;

      switch (c) {
      case L'\"':
      case L'\\':
      case L'\b':
      case L'\f':
      case L'\n':
      case L'\r':
      case L'\t':
        err = json__utf16_encoder_append(enc, &c, 1);
        if (err < 0) return err;
        break;

      default:
        err = swprintf((wchar_t *) escaped, 6, L"u%04x", c);
        if (err < 0) return err;

        err = json__utf16_encoder_append(enc, escaped, 5);
        if (err < 0) return err;
      }
    }
  }

  err = json__utf16_encoder_append(enc, (utf16_t *) L"\"", 1);
  if (err < 0) return err;

  return 0;
}

static inline int
json__encode_utf16le_array(const json_array_t *array, json_utf16_encoder_t *enc) {
  int err;

  err = json__utf16_encoder_append(enc, (utf16_t *) L"[", 1);
  if (err < 0) return err;

  bool first = true;

  for (size_t i = 0, n = array->len; i < n; i++) {
    if (first) first = false;
    else {
      err = json__utf16_encoder_append(enc, (utf16_t *) L",", 1);
      if (err < 0) return err;
    }

    err = json__encode_utf16le(array->values[i], enc);
    if (err < 0) return err;
  }

  err = json__utf16_encoder_append(enc, (utf16_t *) L"]", 1);
  if (err < 0) return err;

  return 0;
}

static inline int
json__encode_utf16le_property(const json_property_t *property, json_utf16_encoder_t *enc) {
  int err;

  err = json__encode_utf16le_string(json_to(string, property->key), enc);
  if (err < 0) return err;

  err = json__utf16_encoder_append(enc, (utf16_t *) L":", 1);
  if (err < 0) return err;

  err = json__encode_utf16le(property->value, enc);
  if (err < 0) return err;

  return 0;
}

static inline int
json__encode_utf16le_object(const json_object_t *object, json_utf16_encoder_t *enc) {
  int err;

  err = json__utf16_encoder_append(enc, (utf16_t *) L"{", 1);
  if (err < 0) return err;

  bool first = true;

  for (size_t i = 0, n = object->len; i < n; i++) {
    const json_property_t *property = &object->properties[i];

    if (property->key->type == json_null) continue;

    if (first) first = false;
    else {
      err = json__utf16_encoder_append(enc, (utf16_t *) L",", 1);
      if (err < 0) return err;
    }

    err = json__encode_utf16le_property(property, enc);
    if (err < 0) return err;
  }

  err = json__utf16_encoder_append(enc, (utf16_t *) L"}", 1);
  if (err < 0) return err;

  return 0;
}

static inline int
json__encode_utf16le(const json_t *value, json_utf16_encoder_t *enc) {
  switch (value->type) {
  case json_null:
    return json__encode_utf16le_null(enc);

  case json_boolean:
    return json__encode_utf16le_boolean(json_to(boolean, value), enc);

  case json_number:
    return json__encode_utf16le_number(json_to(number, value), enc);

  case json_string:
    return json__encode_utf16le_string(json_to(string, value), enc);

  case json_array:
    return json__encode_utf16le_array(json_to(array, value), enc);

  case json_object:
    return json__encode_utf16le_object(json_to(object, value), enc);
  }
}

int
json_encode_utf16le(const json_t *value, utf16_t **result) {
  int err;

  json_utf16_encoder_t enc = {
    .value = NULL,
    .len = 0,
    .capacity = 0,
  };

  err = json__encode_utf16le(value, &enc);
  if (err < 0) goto err;

  *result = realloc(enc.value, (enc.len + 1) * sizeof(utf16_t));

  return 0;

err:
  free(enc.value);

  return -1;
}

static inline void
json__utf8_decoder_skip_whitespace(json_utf8_decoder_t *dec) {
  while (dec->value < dec->end && isspace(*dec->value)) {
    dec->value++;
  }
}

static inline int
json__decode_utf8(json_utf8_decoder_t *dec, json_t **result);

static inline int
json__decode_utf8_number(json_utf8_decoder_t *dec, json_t **result) {
  const utf8_t *start = dec->value;

  if (dec->value < dec->end && *dec->value == '-') dec->value++;

  if (dec->value < dec->end && *dec->value == '0') {
    dec->value++;
  } else {
    while (dec->value < dec->end && isdigit(*dec->value)) {
      dec->value++;
    }
  }

  if (dec->value < dec->end && *dec->value == '.') {
    dec->value++;

    while (dec->value < dec->end && isdigit(*dec->value)) {
      dec->value++;
    }
  }

  if (dec->value < dec->end && (*dec->value == 'e' || *dec->value == 'E')) {
    dec->value++;

    if (dec->value < dec->end && (*dec->value == '+' || *dec->value == '-')) {
      dec->value++;
    }

    while (dec->value < dec->end && isdigit(*dec->value)) {
      dec->value++;
    }
  }

  size_t len = dec->value - start;

  if (len == 0 || len > 64) return -1;

  if (result == NULL) return 0;

  char value[65];
  value[len] = '\0';
  memcpy(value, start, len);

  json_number_t *num = malloc(sizeof(json_number_t));

  if (num == NULL) return -1;

  num->type = json_number;
  num->refs = 1;
  num->value = strtod(value, NULL);

  *result = (json_t *) num;

  return 0;
}

static inline int
json__decode_utf8_string(json_utf8_decoder_t *dec, json_t **result) {
  const utf8_t *start = ++dec->value;

  size_t len = 0;

  while (true) {
    if (dec->value >= dec->end) return -1;

    utf8_t c = *dec->value++;

    if (c == '"') break;

    if (c == '\\') {
      if (dec->value >= dec->end) return -1;

      utf8_t e = *dec->value++;

      if (e == '\"' || e == '\\' || e == '/' || e == 'b' || e == 'f' || e == 'n' || e == 'r' || e == 't') {
        ;
      } else if (e == 'u') {
        if (dec->end - dec->value < 4) return -1;

        dec->value += 4;

        return -1; // TODO Handle conversion
      } else {
        return -1;
      }
    }

    len++;
  }

  if (result == NULL) return 0;

  json_string_t *str = malloc(sizeof(json_string_t) + (len + 1) * sizeof(utf8_t));

  if (str == NULL) return -1;

  void *data = ((char *) str) + sizeof(json_string_t);

  str->type = json_string;
  str->refs = 1;
  str->encoding = json_string_utf8;
  str->value.utf8 = data;
  str->value.utf8[len] = '\0';

  dec->value = start;

  size_t i = 0;

  while (true) {
    utf8_t c = *dec->value++;

    if (c == '"') break;

    if (c == '\\') {
      utf8_t e = *dec->value++;

      if (e == '\"' || e == '\\' || e == '/') {
        c = e;
      } else if (e == 'b') {
        c = '\b';
      } else if (e == 'f') {
        c = '\f';
      } else if (e == 'n') {
        c = '\n';
      } else if (e == 'r') {
        c = '\r';
      } else if (e == 't') {
        c = '\t';
      } else if (e == 'u') {
        dec->value += 4;

        // TODO Handle conversion
      }
    }

    str->value.utf8[i++] = c;
  }

  *result = (json_t *) str;

  return 0;
}

static inline int
json__decode_utf8_array(json_utf8_decoder_t *dec, json_t **result) {
  int err;

  const utf8_t *start = ++dec->value;

  size_t len = 0;

  while (true) {
    json__utf8_decoder_skip_whitespace(dec);

    if (dec->value >= dec->end) return -1;

    utf8_t c = *dec->value;

    if (c == ']') {
      dec->value++;
      break;
    }

    err = json__decode_utf8(dec, NULL);
    if (err < 0) return err;

    len++;

    json__utf8_decoder_skip_whitespace(dec);

    if (dec->value >= dec->end) return -1;

    c = *dec->value++;

    if (c == ']') break;
    else if (c != ',') return -1;
  }

  if (result == NULL) return 0;

  json_array_t *arr = malloc(sizeof(json_array_t) + len * sizeof(json_t *));

  if (arr == NULL) return -1;

  arr->type = json_array;
  arr->refs = 1;
  arr->len = len;

  dec->value = start;

  size_t i = 0;

  while (true) {
    json__utf8_decoder_skip_whitespace(dec);

    utf8_t c = *dec->value;

    if (c == ']') {
      dec->value++;
      break;
    }

    err = json__decode_utf8(dec, &arr->values[i++]);
    (void) err;

    json__utf8_decoder_skip_whitespace(dec);

    c = *dec->value++;

    if (c == ']') break;
  }

  *result = (json_t *) arr;

  return 0;
}

static inline int
json__decode_utf8_object(json_utf8_decoder_t *dec, json_t **result) {
  int err;

  const utf8_t *start = ++dec->value;

  size_t len = 0;

  while (true) {
    json__utf8_decoder_skip_whitespace(dec);

    if (dec->value >= dec->end) return -1;

    utf8_t c = *dec->value;

    if (c == '}') {
      dec->value++;
      break;
    }

    err = json__decode_utf8_string(dec, NULL);
    if (err < 0) return err;

    json__utf8_decoder_skip_whitespace(dec);

    if (dec->value >= dec->end) return -1;

    c = *dec->value++;

    if (c != ':') return -1;

    json__utf8_decoder_skip_whitespace(dec);

    err = json__decode_utf8(dec, NULL);
    if (err < 0) return err;

    len++;

    json__utf8_decoder_skip_whitespace(dec);

    if (dec->value >= dec->end) return -1;

    c = *dec->value++;

    if (c == '}') break;
    else if (c != ',') return -1;
  }

  if (result == NULL) return 0;

  json_object_t *obj = malloc(sizeof(json_object_t) + len * sizeof(json_property_t));

  if (obj == NULL) return -1;

  obj->type = json_object;
  obj->refs = 1;
  obj->len = len;

  dec->value = start;

  size_t i = 0;

  while (true) {
    json__utf8_decoder_skip_whitespace(dec);

    utf8_t c = *dec->value;

    if (c == '}') {
      dec->value++;
      break;
    }

    json_property_t *property = &obj->properties[i++];

    err = json__decode_utf8_string(dec, &property->key);
    (void) err;

    json__utf8_decoder_skip_whitespace(dec);

    (void) *dec->value++;

    json__utf8_decoder_skip_whitespace(dec);

    err = json__decode_utf8(dec, &property->value);
    (void) err;

    json__utf8_decoder_skip_whitespace(dec);

    c = *dec->value++;

    if (c == '}') break;
  }

  *result = (json_t *) obj;

  return 0;
}

static inline int
json__decode_utf8(json_utf8_decoder_t *dec, json_t **result) {
  json__utf8_decoder_skip_whitespace(dec);

  if (
    dec->end - dec->value >= 4 &&
    dec->value[0] == 't' &&
    dec->value[1] == 'r' &&
    dec->value[2] == 'u' &&
    dec->value[3] == 'e'
  ) {
    dec->value += 4;

    if (result) *result = (json_t *) &json__true;

    return 0;
  }

  if (
    dec->end - dec->value >= 5 &&
    dec->value[0] == 'f' &&
    dec->value[1] == 'a' &&
    dec->value[2] == 'l' &&
    dec->value[3] == 's' &&
    dec->value[4] == 'e'
  ) {
    dec->value += 5;

    if (result) *result = (json_t *) &json__false;

    return 0;
  }

  if (
    dec->end - dec->value >= 4 &&
    dec->value[0] == 'n' &&
    dec->value[1] == 'u' &&
    dec->value[2] == 'l' &&
    dec->value[3] == 'l'
  ) {
    dec->value += 4;

    if (result) *result = (json_t *) &json__null;

    return 0;
  }

  utf8_t c = *dec->value;

  if (c == '"') {
    return json__decode_utf8_string(dec, result);
  }

  if (c == '[') {
    return json__decode_utf8_array(dec, result);
  }

  if (c == '{') {
    return json__decode_utf8_object(dec, result);
  }

  return json__decode_utf8_number(dec, result);
}

int
json_decode_utf8(const utf8_t *buffer, size_t len, json_t **result) {
  int err;

  if (len == (size_t) -1) len = strlen((char *) buffer);

  json_utf8_decoder_t dec = {
    .value = buffer,
    .start = buffer,
    .end = buffer + len,
  };

  json_t *value;
  err = json__decode_utf8(&dec, &value);
  if (err < 0) return err;

  if (dec.value != dec.end) {
    json_deref(value);

    return -1;
  }

  *result = value;

  return err;
}

int
json_decode_utf16le(const utf16_t *buffer, size_t len, json_t **result) {
  return -1;
}
