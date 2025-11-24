#include <assert.h>
#include <utf.h>

#include "../include/json.h"

int
main() {
  int e;

  json_t *actual;
  e = json_decode_utf8((utf8_t *) "{ \"a\": 1, \"b\": 2, \"c\": 3 }", -1, &actual);
  assert(e == 0);

  json_t *expected;
  e = json_create_object(3, &expected);
  assert(e == 0);

  json_t *k, *v;

#define V(key, value) \
  e = json_create_string_utf8((utf8_t *) key, -1, &k); \
  assert(e == 0); \
  e = json_create_number(value, &v); \
  assert(e == 0); \
  e = json_object_set(expected, k, v); \
  assert(e == 0);

  V("a", 1)
  V("b", 2)
  V("c", 3)
#undef V

  e = json_compare(actual, expected);
  assert(e == 0);
}
