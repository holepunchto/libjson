#include <assert.h>
#include <utf.h>

#include "../include/json.h"

int
main() {
  int e;

  json_t *actual;
  e = json_decode_utf8((utf8_t *) "[1, 2, 3]", -1, &actual);
  assert(e == 0);

  json_t *expected;
  e = json_create_array(3, &expected);
  assert(e == 0);

  json_t *v;

#define V(i, value) \
  e = json_create_number(value, &v); \
  assert(e == 0); \
  e = json_array_set(expected, i, v); \
  assert(e == 0);

  V(0, 1)
  V(1, 2)
  V(2, 3)
#undef V

  e = json_compare(actual, expected);
  assert(e == 0);
}
