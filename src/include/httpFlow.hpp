#ifndef HTTPFLOW_HPP
#define HTTPFLOW_HPP

#include <string>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

#include "configuration.h"
#include "timeStamp.h"
#include "sqlTime.h"
#include "stringHelpers.h"

const std::string delimiter = "\n";
size_t spaceMarker=0;

struct HTTPStats {
  uint32_t sourceIP;
  uint32_t destinationIP;
  uint16_t sourcePort;
  uint16_t destinationPort;
  TimeStamp time;
  char type;
};

struct HTTPRequest {
  std::string type;         ///<GET,HEAD,POST,PUT,DELETE,TRACE,CONNECT[7]
  std::string uri;          ///<HTTP protocol places NO prior limit on the length of URI[255].
  std::string host;         ///<FQDN cannot exceed[255]
  std::string version;      ///<HTTP/1.X[8]
  std::string browser;      ///<Stores client browser type[60] 
  std::string referer;      ///Stores HTTP RFC's misspelled referRer[255]
};

struct HTTPResponse {
  uint16_t type;         //<1XX,2XX,3XX,4XX,5XX
  std::string version;   ///<HTTP/1.X [8]
  std::string server;    ///<Stores HTTP server type [60]
  std::string content;   ///Stores content type [30]
};

struct HTTP {
  HTTPStats stats;
  HTTPRequest request;
  HTTPResponse response;
};

///Function that stores tokenized HTTP requests/responses in appropriate structures
inline void tokenizeHTTP(HTTP &http, const std::string httpString) {
  static std::vector <std::string> httpTokens;
  explodeString(httpTokens, httpString, delimiter);
  switch (httpString[0]) {
    case 'q':
      http.request.type.assign(httpTokens[0], 1, 7);
      http.request.uri.assign(httpTokens[1], 0, 255);
      http.request.version.assign(httpTokens[2], 0, 8);
      http.request.host.assign(httpTokens[3], 0, 255);
      http.request.browser.assign(httpTokens[4], 0, 60);
      http.request.referer.assign(httpTokens[5], 0, 255);
      http.response.type = 0;
      http.response.version.clear();
      http.response.server.clear();
      http.response.content.clear();
      break;
    case 's':
      http.response.type = boost::lexical_cast <uint16_t> (httpTokens[1]);
      http.response.version.assign(httpTokens[0], 1, 8);
      http.response.server.assign(httpTokens[3], 0, 60);
      spaceMarker=httpTokens[4].find(' ',0);
      http.response.content.assign(httpTokens[4].substr(0,spaceMarker-1), 0, 30);
      http.request.type.clear();
      http.request.uri.clear();
      http.request.host.clear();
      http.request.version.clear();
      http.request.browser.clear();
      httpTokens.clear();
      break;
  }
  httpTokens.clear();
}

#endif
