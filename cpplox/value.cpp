#include "value.h"

#include <cstdio>
#include <cstring>

#include "memory.h"
#include "object.h"

void ValueArray::Write(Value value) {
  if (capacity < count + 1) {
    int old_capacity = capacity;
    capacity = GROW_CAPACITY(capacity);
    values = GROW_ARRAY(Value, values, old_capacity, capacity);
  }

  values[count++] = value;
}

ValueArray::~ValueArray() {
  FREE_ARRAY(Value, values, capacity);

  capacity = count = 0;
  values = nullptr;
}

bool ValuesEqual(Value a, Value b) {
  if (a.type != b.type) return false;

  switch (a.type) {
    case VAL_BOOL:
      return AS_BOOL(a) == AS_BOOL(b);

    case VAL_NIL:
      return true;

    case VAL_NUMBER:
      return AS_NUMBER(a) == AS_NUMBER(b);

    case VAL_OBJ: {
      ObjString* a_string = AS_STRING(a);
      ObjString* b_string = AS_STRING(b);

      return a_string->length == b_string->length &&
             memcmp(a_string->chars, b_string->chars, a_string->length) == 0;
    }

    default:
      return false;
  }
}

void PrintValue(Value value) {
  switch (value.type) {
    case VAL_BOOL:
      printf(AS_BOOL(value) ? "true" : "false");
      break;
    case VAL_NIL:
      printf("nil");
      break;

    case VAL_NUMBER:
      printf("%g", AS_NUMBER(value));
      break;
    case VAL_OBJ:
      PrintObject(value);
      break;
  }
}
