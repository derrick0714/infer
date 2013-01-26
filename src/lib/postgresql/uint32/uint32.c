#include <sys/endian.h>
#include <postgres.h>
#include <fmgr.h>
#include <libpq/pqformat.h>
#include <lib/stringinfo.h>

#ifdef PG_MODULE_MAGIC
  PG_MODULE_MAGIC;
#endif

uint32_t *uint32In(char *input) {
  uint32_t *result;
  if (input == NULL) {
    return 0;
  }
  result = (uint32_t*)palloc(sizeof(uint32_t));
  *result = (uint32_t)strtoul(input, NULL, 10);
  return result;
}

char *uint32Out(uint32_t *input) {
  char *result = (char*)palloc(10);
  sprintf(result, "%u", *input);
  return result;
}

/* 
 * placeholder for actual function
 */

PG_FUNCTION_INFO_V1(uint32Receive);
Datum uint32Receive(PG_FUNCTION_ARGS) {
  StringInfo  buf = (StringInfo) PG_GETARG_POINTER(0);

  PG_RETURN_UINT32((uint32_t) pq_getmsgint(buf, sizeof(uint32_t)));
}

Datum uint32Send(uint32_t *input) {
  uint32_t bigEndianInput = htobe32(*input);
  StringInfoData result;
  pq_begintypsend(&result);
  pq_sendbytes(&result, (char*)&bigEndianInput, sizeof(uint32_t));
  PG_RETURN_BYTEA_P(pq_endtypsend(&result));
}

bool uint32LessThan(uint32_t *left, uint32_t *right) {
  if (left == NULL || right == NULL) {
    return false;
  }
  return (*left < *right);
}

bool uint32LessThanOrEqualTo(uint32_t *left, uint32_t *right) {
  if (left == NULL || right == NULL) {
    return false;
  }
  return (*left <= *right);
}

bool uint32EqualTo(uint32_t *left, uint32_t *right) {
  if (left == NULL || right == NULL) {
    return false;
  }
  return (*left == *right);
}

bool uint32NotEqualTo(uint32_t *left, uint32_t *right) {
  if (left == NULL || right == NULL) {
    return false;
  }
  return (*left != *right);
}

bool uint32GreaterThanOrEqualTo(uint32_t *left, uint32_t *right) {
  if (left == NULL || right == NULL) {
    return false;
  }
  return (*left >= *right);
}

bool uint32GreaterThan(uint32_t *left, uint32_t *right) {
  if (left == NULL || right == NULL) {
    return false;
  }
  return (*left > *right);
}

int uint32Comparator(uint32_t *left, uint32_t *right) {
  if (*left < *right) {
    return -1;
  }
  if (*left > *right) {
    return 1;
  }
  return 0;
}

uint32_t *uint32Min(uint32_t *left, uint32_t *right) {
  uint32_t *result = (uint32_t*)palloc(sizeof(uint32_t));
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

uint32_t *uint32Max(uint32_t *left, uint32_t *right) {
  uint32_t *result = (uint32_t*)palloc(sizeof(uint32_t));
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

uint32_t *uint32Add(uint32_t *left, uint32_t *right) {
  uint32_t *result = (uint32_t*)palloc(sizeof(uint32_t));
  *result = *left + *right;
  return result;
}

uint32_t *uint32Subtract(uint32_t *left, uint32_t *right) {
  uint32_t *result = (uint32_t*)palloc(sizeof(uint32_t));
  *result = *left - *right;
  return result;
}

uint32_t *uint32AddArray(uint32_t *left, uint32_t *right) {
  uint32_t *result;
  uint8_t size, index;
  if (*(uint8_t*)(left + VARHDRSZ) < *(uint8_t*)(right + VARHDRSZ)) {
    result = (uint32_t*)palloc(VARSIZE(right));
    size = *(uint8_t*)(right + VARHDRSZ);
    SET_VARSIZE(result, VARSIZE(right));
    memcpy((void*)VARDATA(result), (void*)VARDATA(right), 20);
    for (index = 0; index < *(uint8_t*)(left + VARHDRSZ); ++index) {
      *(uint32_t*)(VARDATA(result) + 20 + (index * 4)) = *(uint32_t*)(VARDATA(right) + 20 + (index * 4)) + *(uint32_t*)(VARDATA(left) + 20 + (index * 4));
    }
    for (index = *(uint8_t*)(left + VARHDRSZ); index < size; ++index) {
      *(uint32_t*)(VARDATA(result) + 20 + (index * 4)) = *(uint32_t*)(VARDATA(right) + 20 + (index * 4));
    }
  }
  else {
    result = (uint32_t*)palloc(VARSIZE(left));
    size = *(uint8_t*)(left + VARHDRSZ);
    SET_VARSIZE(result, VARSIZE(left));
    memcpy((void*)VARDATA(result), (void*)VARDATA(left), 20);
    for (index = 0; index < size; ++index) {
      *(uint32_t*)(VARDATA(result) + 20 + (index * 4)) = *(uint32_t*)(VARDATA(right) + 20 + (index * 4)) + *(uint32_t*)(VARDATA(left) + 20 + (index * 4));
    }
  }
  return result;
}
