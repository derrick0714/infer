#ifndef INFER_INCLUDE_MISCHELPERS_HPP_
#define INFER_INCLUDE_MISCHELPERS_HPP_

#include "FlowStats.hpp"

uint8_t flowProtocol(const char *flowID) {
  return *(uint8_t*)flowID;
}

uint32_t flowSourceIP(const char *flowID) {
  return *(uint32_t*)(flowID + 1);
}

uint32_t flowDestinationIP(const char *flowID) {
  return *(uint32_t*)(flowID + 5);
}

uint16_t flowSourcePort(const char *flowID) {
  return *(uint16_t*)(flowID + 9);
}

uint16_t flowDestinationPort(const char *flowID) {
  return *(uint16_t*)(flowID + 11);
}

template <class T, class U>
inline void compareLessThan(T &startTime, const U &flowStartTime) {
  if (flowStartTime < startTime) {
    startTime = flowStartTime;
  }
}

template <class T, class U>
inline void compareGreaterThan(T &endTime, const U &flowEndTime) {
  if (flowEndTime > endTime) {
    endTime = flowEndTime;
  }
}

/*
 * Adds bytes per content type from the source data structure to the destination
 * data structure.
 */
template <class Destination, class Source>
inline void accumulateContent(Destination &destination, const Source *source) {
  for (size_t contentType = 0; contentType < FlowStats::CONTENT_TYPES; ++contentType) {
    destination[contentType] += source->content(static_cast<FlowStats::ContentType>(contentType));
  }
}

/*
 * Adds bytes per content type from the source data structure to the destination
 * data structure.
 */
template <class Destination>
inline void accumulateContent(Destination &destination, const uint32_t (&source)[FlowStats::CONTENT_TYPES]) {
  for (size_t contentType = 0; contentType < FlowStats::CONTENT_TYPES; ++contentType) {
    destination[contentType] += source[contentType];
  }
}

/*
 * Adds bytes per content type from the source data structure to the destination
 * data structure.
 */
template <class Destination>
inline void accumulateContent(Destination &destination, const std::vector<uint32_t> &source) {
  for (size_t contentType = 0; contentType < FlowStats::CONTENT_TYPES; ++contentType) {
    destination[contentType] += source[contentType];
  }
}

/*
 * Adds bytes per content type from the source data structure to the destination
 * data structure.
 */
template <class Destination, class Source>
inline void accumulateContent(Destination &destination, const Source &source) {
  for (size_t contentType = 0; contentType < FlowStats::CONTENT_TYPES; ++contentType) {
    destination[contentType] += source.content(contentType);
  }
}

/*
 * Returns true if there is any WAV audio, JPEG image, MP3 audio, or MPEG video
 * data in the content data structure, or false if there isn't.
 */
template <class Content>
inline bool hasMultimediaContent(const Content &content) {
  return (content[2] || content[4] || content[5] || content[6]);
}

/*
 * Returns true is there is any plaintext, BMP image, compressed, or encrypted
 * data in the content data structure, or false if there isn't.
 */
template <class Content>
inline bool hasNonMultimediaContent(const Content &content) {
  return (content[0] || content[1] || content[3] || content[7]);
}

/* Returns the percentage of multimedia traffic as a double value between 0 
 * and 1
 */
template <class Content>
inline double getMultimediaPercent(const Content &content) {
  double totalBytes = 0.0;
  for (size_t index = 0; index < FlowStats::CONTENT_TYPES; ++index) 
  totalBytes += content[index];
  if(totalBytes == 0){ //Avoiding Nan comparisons
    return 0.0;
  }
  else{ 
    return((16 * static_cast<double>(content[2] + content[4] + content[5] + content[6])) /
           (16 * totalBytes ));
  }  
}

#endif
