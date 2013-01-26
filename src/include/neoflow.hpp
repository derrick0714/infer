#ifndef __NEOFLOW_HPP
#define __NEOFLOW_HPP

#include <vector>
#include <sys/types.h>

#include "timeStamp.h"

#define CONTENT_TYPES 8

#define PLAINTEXT_TYPE	0
#define BMP_IMAGE_TYPE	1
#define WAV_AUDIO_TYPE	2
#define COMPRESSED_TYPE	3
#define JPEG_IMAGE_TYPE	4
#define MP3_AUDIO_TYPE	5
#define MPEG_VIDEO_TYPE	6
#define ENCRYPTED_TYPE	7

struct FlowStats {
  uint8_t version;
  uint8_t typeOfService;
  uint8_t padding;
  uint8_t protocol;
  uint32_t sourceIP;
  uint32_t destinationIP;
  uint16_t sourcePort;
  uint16_t destinationPort;
  TimeStamp startTime;
  TimeStamp firstSYNTime;
  TimeStamp firstSYNACKTime;
  TimeStamp firstACKTime;
  TimeStamp endTime;
  TimeStamp minInterArrivalTime;
  TimeStamp maxInterArrivalTime;
  uint8_t tcpFlags;
  uint8_t minTTL;
  uint8_t maxTTL;
  uint8_t _padding;
  uint32_t numFrags;
  uint32_t numBytes;
  uint32_t numPackets;
  uint32_t minPacketSize;
  uint32_t maxPacketSize;
  uint32_t tcpURGs;
  uint32_t tcpACKs;
  uint32_t tcpPUSHs;
  uint32_t tcpRSTs;
  uint32_t tcpSYNs;
  uint32_t tcpFINs;
  uint32_t packets_0_256;
  uint32_t packets_257_512;
  uint32_t packets_513_768;
  uint32_t packets_769_1024;
  uint32_t packets_1025_1280;
  uint32_t packets_1281_1536;
  uint32_t packets_1537_1792;
  uint32_t packets_1793_2048;
  uint32_t content[CONTENT_TYPES];
};

#endif
