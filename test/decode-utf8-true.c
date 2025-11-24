#include <assert.h>
#include <utf.h>

#include "../include/json.h"

int
main() {
  int e;

  json_t *actual;
  e = json_decode_utf8((utf8_t *) "true", -1, &actual);
  assert(e == 0);

  json_t *expected;
  e = json_create_boolean(true, &expected);
  assert(e == 0);

  e = json_compare(actual, expected);
  assert(e == 0);
}
