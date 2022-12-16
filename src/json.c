#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "../include/json.h"

#define json_to(t, value) (assert(value->type == t), (t##_t *) value)

typedef struct json_null_s json_null_t;
typedef struct json_true_s json_true_t;
typedef struct json_false_s json_false_t;
typedef struct json_number_s json_number_t;
typedef struct json_string_s json_string_t;
typedef struct json_array_s json_array_t;
typedef struct json_property_s json_property_t;
typedef struct json_object_s json_object_t;

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

struct json_true_s {
  json_type_t type;
};

struct json_false_s {
  json_type_t type;
};

struct json_number_s {
  json_type_t type;
  int refs;
  double value;
};

struct json_string_s {
  json_type_t type;
  int refs;
  char value[];
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

json_type_t
json_typeof (const json_t *value) {
  return value->type;
}

extern bool
json_is_null (const json_t *value);

extern bool
json_is_true (const json_t *value);

extern bool
json_is_false (const json_t *value);

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

void
json_free_array (json_array_t *array) {
  for (size_t i = 0, n = array->len; i < n; i++) {
    json_deref(array->values[i]);
  }
}

void
json_free_object (json_object_t *object) {
  for (size_t i = 0, n = object->len; i < n; i++) {
    json_property_t *property = &object->properties[i];

    json_deref(property->key);
    json_deref(property->value);
  }
}

void
json_free (json_t *value) {
  switch (value->type) {
  case json_null:
  case json_true:
  case json_false:
  case json_number:
  case json_string:
    break;

  case json_array:
    json_free_array(json_to(json_array, value));
    break;

  case json_object:
    json_free_object(json_to(json_object, value));
    break;
  }

  free(value);
}

int
json_ref (json_t *value) {
  switch (value->type) {
  case json_null:
  case json_true:
  case json_false:
    return 1; // Always has a singleton reference

  case json_number: {
    json_number_t *number = json_to(json_number, value);

    return ++number->refs;
  }

  case json_string: {
    json_string_t *string = json_to(json_string, value);

    return ++string->refs;
  }

  case json_array: {
    json_array_t *array = json_to(json_array, value);

    return ++array->refs;
  }

  case json_object: {
    json_object_t *object = json_to(json_object, value);

    return ++object->refs;
  }
  }
}

int
json_deref (json_t *value) {
  int *refsp = NULL;

  switch (value->type) {
  case json_null:
  case json_true:
  case json_false:
    return 1; // Always has a singleton reference

  case json_number:
    refsp = &json_to(json_number, value)->refs;
    break;

  case json_string:
    refsp = &json_to(json_string, value)->refs;
    break;

  case json_array:
    refsp = &json_to(json_array, value)->refs;
    break;

  case json_object:
    refsp = &json_to(json_object, value)->refs;
    break;
  }

  int refs = *refsp;
  if (refs >= 1) *refsp = --refs;
  if (refs == 0) json_free(value);

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
json_attach_to_scope (json_t *value) {
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

static const json_true_t json__true = {
  .type = json_true,
};

static const json_false_t json__false = {
  .type = json_false,
};

int
json_create_boolean (bool value, json_t **result) {
  *result = value ? (json_t *) &json__true : (json_t *) &json__false;

  return 0;
}

bool
json_boolean_value (const json_t *boolean) {
  assert(boolean->type == json_true || boolean->type == json_false);
  return boolean->type == json_true;
}

int
json_create_number (double value, json_t **result) {
  json_number_t *number = malloc(sizeof(json_number_t));

  if (number == NULL) return -1;

  number->type = json_number;
  number->refs = 1;
  number->value = value;

  json_attach_to_scope(*result = (json_t *) number);

  return 0;
}

double
json_number_value (const json_t *number) {
  return json_to(json_number, number)->value;
}

int
json_create_string (const char *value, json_t **result) {
  json_string_t *str = malloc(sizeof(json_string_t) + strlen(value) + 1);

  if (str == NULL) return -1;

  str->type = json_string;
  str->refs = 1;

  strcpy(str->value, value);

  json_attach_to_scope(*result = (json_t *) str);

  return 0;
}

const char *
json_string_value (const json_t *string) {
  return json_to(json_string, string)->value;
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

  json_attach_to_scope(*result = (json_t *) arr);

  return 0;
}

size_t
json_array_size (const json_t *array) {
  return json_to(json_array, array)->len;
}

json_t *
json_array_get (const json_t *array, size_t index) {
  json_array_t *arr = json_to(json_array, array);

  if (index >= arr->len) return NULL;

  return arr->values[index];
}

int
json_array_set (json_t *array, size_t index, json_t *value) {
  json_array_t *arr = json_to(json_array, array);

  if (index >= arr->len) return -1;

  arr->values[index] = value;

  json_ref(value);

  return 0;
}

int
json_array_delete (json_t *array, size_t index) {
  json_array_t *arr = json_to(json_array, array);

  if (index >= arr->len) return -1;

  json_t *value = arr->values[index];

  arr->values[index] = (json_t *) &json__null;

  json_deref(value);

  return 0;
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

  json_attach_to_scope(*result = (json_t *) obj);

  return 0;
}

size_t
json_object_size (const json_t *object) {
  return json_to(json_object, object)->len;
}

json_t *
json_object_get (const json_t *object, const json_t *key) {
  return NULL;
}

int
json_object_set (json_t *object, json_t *key, json_t *value) {
  return -1;
}

int
json_object_delete (json_t *object, json_t *key) {
  return -1;
}