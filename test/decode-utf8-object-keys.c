#include <assert.h>
#include <string.h>
#include <utf.h>

#include "../include/json.h"

int
main() {
  int e;

  json_t *object;
  e = json_decode_utf8((utf8_t *) "{ \"a\": 1, \"b\": 2, \"c\": 3 }", -1, &object);
  assert(e == 0);

  assert(json_object_size(object) == 3);

  const char *keys[] = {"a", "b", "c"};

  for (size_t i = 0; i < 3; i++) {
    json_t *key = json_object_key(object, i);
    assert(key != NULL);
    assert(strcmp((char *) json_string_value_utf8(key), keys[i]) == 0);

    json_t *value = json_object_value(object, i);
    assert(value != NULL);
    assert(json_number_value(value) == (double) (i + 1));
  }

  assert(json_object_key(object, 3) == NULL);
  assert(json_object_value(object, 3) == NULL);
}
