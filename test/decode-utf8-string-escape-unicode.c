#include <assert.h>
#include <utf.h>

#include "../include/json.h"

static void
compare(const char *source, const char *expected) {
  int e;

  json_t *actual;
  e = json_decode_utf8((utf8_t *) source, -1, &actual);
  assert(e == 0);

  json_t *string;
  e = json_create_string_utf8((utf8_t *) expected, -1, &string);
  assert(e == 0);

  e = json_compare(actual, string);
  assert(e == 0);
}

int
main() {
  compare("\"\\u0061\"", "a");
  compare("\"\\u00e9\"", "\xc3\xa9");
  compare("\"\\u20ac\"", "\xe2\x82\xac");
  compare("\"\\ud83d\\ude00\"", "\xf0\x9f\x98\x80");
  compare("\"a\\u00e9\\ud83d\\ude00z\"", "a\xc3\xa9\xf0\x9f\x98\x80"
                                         "z");
  compare("\"\\ud800\"", "\xef\xbf\xbd");
  compare("\"\\udc00a\"", "\xef\xbf\xbd"
                          "a");
}
