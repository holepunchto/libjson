#include <assert.h>
#include <utf.h>

#include "../include/json.h"

int
main() {
  int e;

  json_t *value;

  // RFC 8259 section 2: JSON-text = ws value ws
  e = json_decode_utf8((utf8_t *) "{\n  \"a\": 1\n}\n", -1, &value);
  assert(e == 0);
  assert(json_is_object(value));

  e = json_decode_utf8((utf8_t *) "  [1, 2]\t\n", -1, &value);
  assert(e == 0);
  assert(json_is_array(value));

  e = json_decode_utf8((utf8_t *) "1 ", -1, &value);
  assert(e == 0);
  assert(json_is_number(value));

  e = json_decode_utf8((utf8_t *) "{} x", -1, &value);
  assert(e == -1);
}
