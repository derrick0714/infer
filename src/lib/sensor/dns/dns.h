#include "modules.h"
#include "network.h"
#include "timeStamp.h"

#define DNS_QUERY               0
#define DNS_RESPONSE            1

#define NUM_SUPPORTED_TYPES     4

#define A_RECORD                1
#define CNAME_RECORD            5
#define PTR_RECORD              12
#define MX_RECORD               15

/* We're only interested in A, CNAME, MX, and PTR records. */
uint16_t supportedResourceRecordTypes[NUM_SUPPORTED_TYPES] = { 1, 5, 12, 15 };
