#include <sys/endian.h>
#include <postgres.h>
#include <fmgr.h>
#include <libpq/pqformat.h>
#include <lib/stringinfo.h>

#ifdef PG_MODULE_MAGIC
  PG_MODULE_MAGIC;
#endif

uint16_t *uint16In(char *input) {
  uint16_t *result;
  if (input == NULL) {
    return 0;
  }
  result = (uint16_t*)palloc(sizeof(uint16_t));
  *result = (uint16_t)strtoul(input, NULL, 10);
  return result;
}

char *uint16Out(uint16_t *input) {
  char *result = (char*)palloc(5);
  sprintf(result, "%hu", *input);
  return result;
}

/* 
 * placeholder for actual function
 */
PG_FUNCTION_INFO_V1(uint16Receive);
Datum uint16Receive(PG_FUNCTION_ARGS) {
  StringInfo  buf = (StringInfo) PG_GETARG_POINTER(0);

  PG_RETURN_INT16((uint16_t) pq_getmsgint(buf, sizeof(uint16_t)));
}


Datum uint16Send(uint16_t *input) {
  uint16_t bigEndianInput = htobe16(*input);
  StringInfoData result;
  pq_begintypsend(&result);
  pq_sendbytes(&result, (char*)&bigEndianInput, sizeof(uint16_t));
  PG_RETURN_BYTEA_P(pq_endtypsend(&result));
}

bool uint16LessThan(uint16_t *left, uint16_t *right) {
  if (left == NULL || right == NULL) {
    return false;
  }
  return (*left < *right);
}

bool uint16LessThanOrEqualTo(uint16_t *left, uint16_t *right) {
  if (left == NULL || right == NULL) {
    return false;
  }
  return (*left <= *right);
}

bool uint16EqualTo(uint16_t *left, uint16_t *right) {
  if (left == NULL || right == NULL) {
    return false;
  }
  return (*left == *right);
}

bool uint16NotEqualTo(uint16_t *left, uint16_t *right) {
  if (left == NULL || right == NULL) {
    return false;
  }
  return (*left != *right);
}

bool uint16GreaterThanOrEqualTo(uint16_t *left, uint16_t *right) {
  if (left == NULL || right == NULL) {
    return false;
  }
  return (*left >= *right);
}

bool uint16GreaterThan(uint16_t *left, uint16_t *right) {
  if (left == NULL || right == NULL) {
    return false;
  }
  return (*left > *right);
}

int uint16Comparator(uint16_t *left, uint16_t *right) {
  if (*left < *right) {
    return -1;
  }
  if (*left > *right) {
    return 1;
  }
  return 0;
}

uint16_t *uint16Min(uint16_t *left, uint16_t *right) {
  uint16_t *result = (uint16_t*)palloc(sizeof(uint16_t));
  if (left == NULL) {
    *result = *right;
    return result;
  }
  if (right == NULL) {
    *result = *left;
    return result;
  }
  if (*left < *right) {
    *result = *left;
    return result;
  }
  *result = *right;
  return result;
}

uint16_t *uint16Max(uint16_t *left, uint16_t *right) {
  uint16_t *result = (uint16_t*)palloc(sizeof(uint16_t));
  if (left == NULL) {
    *result = *right;
    return result;
  }
  if (right == NULL) {
    *result = *left;
    return result;
  }
  if (*left > *right) {
    *result = *left;
    return result;
  }
  *result = *right;
  return result;
}

uint16_t *uint16Add(uint16_t *left, uint16_t *right) {
  uint16_t* result = (uint16_t*)palloc(sizeof(uint16_t));
  *result = *left + *right;
  return result;
}
