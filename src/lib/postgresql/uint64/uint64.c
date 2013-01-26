#include <sys/endian.h>
#include <machine/_limits.h>
#include <postgres.h>
#include <fmgr.h>
#include <libpq/pqformat.h>
#include <lib/stringinfo.h>

#ifdef PG_MODULE_MAGIC
  PG_MODULE_MAGIC;
#endif

#if __LONG_BIT == 32
  #define UINT64_FORMAT_STRING "%llu"
#else
  #define UINT64_FORMAT_STRING "%lu"
#endif

uint64_t *uint64In(char *input) {
  uint64_t *result;
  if (input == NULL) {
    return 0;
  }
  result = (uint64_t*)palloc(sizeof(uint64_t));
  *result = (uint64_t)strtoull(input, NULL, 10);
  return result;
}

char *uint64Out(uint64_t *input) {
  char *result = (char*)palloc(20);
  sprintf(result, UINT64_FORMAT_STRING, *input);
  return result;
}

/* 
 * placeholder for actual function
 */

Datum uint64Receive(uint64_t *input) {
  return (Datum)NULL;
}

Datum uint64Send(uint64_t *input) {
  uint64_t bigEndianInput = htobe64(*input);
  StringInfoData result;
  pq_begintypsend(&result);
  pq_sendbytes(&result, (char*)&bigEndianInput, sizeof(uint64_t));
  PG_RETURN_BYTEA_P(pq_endtypsend(&result));
}

bool uint64LessThan(uint64_t *left, uint64_t *right) {
  if (left == NULL || right == NULL) {
    return false;
  }
  return (*left < *right);
}

bool uint64LessThanOrEqualTo(uint64_t *left, uint64_t *right) {
  if (left == NULL || right == NULL) {
    return false;
  }
  return (*left <= *right);
}

bool uint64EqualTo(uint64_t *left, uint64_t *right) {
  if (left == NULL || right == NULL) {
    return false;
  }

  return (*left == *right);
}

bool uint64NotEqualTo(uint64_t *left, uint64_t *right) {
  if (left == NULL || right == NULL) {
    return false;
  }

  return (*left != *right);
}

bool uint64GreaterThanOrEqualTo(uint64_t *left, uint64_t *right) {
  if (left == NULL || right == NULL) {
    return false;
  }
  return (*left >= *right);
}

bool uint64GreaterThan(uint64_t *left, uint64_t *right) {
  if (left == NULL || right == NULL) {
    return false;
  }
  return (*left > *right);
}

int uint64Comparator(uint64_t *left, uint64_t *right) {
  if (*left < *right) {
    return -1;
  }
  if (*left > *right) {
    return 1;
  }
  return 0;
}

uint64_t *uint64Min(uint64_t *left, uint64_t *right) {
  uint64_t *result = (uint64_t*)palloc(sizeof(uint64_t));
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

uint64_t *uint64Max(uint64_t *left, uint64_t *right) {
  uint64_t *result = (uint64_t*)palloc(sizeof(uint64_t));
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

uint64_t *uint64Add(uint64_t *left, uint64_t *right) {
  uint64_t *result = (uint64_t*)palloc(sizeof(uint64_t));
  *result = *left + *right;
  return result;
}

/* Converts uint32 to uint64. */
uint64_t *uint32To64(uint32_t *input) {
  uint64_t *result = (uint64_t*)palloc(sizeof(uint64_t));
  *result = *input;
  return result;
}
