#include "getHTTPData.h"

bool getHTTPData(const char *host, const uint16_t port,
                 const char *fileName, std::stringstream &data) {
  BIO *socket;
  std::stringstream authority, request;
  char buffer[2048];
  int bytesRead;
  authority << host << ':' << port;
  request << "GET " << fileName << " HTTP/1.1\r\n"
          << "Host: " << host << "\r\n"
          << "Connection: close\r\n\r\n";
  socket = BIO_new_connect((char*)authority.str().c_str());
  if (BIO_do_connect(socket) < 1) {
    BIO_free(socket);
    return false;
  }
  if (BIO_write(socket, request.str().c_str(), request.str().length()) < 1) {
    BIO_free(socket);
    return false;
  }
  while ((bytesRead = BIO_read(socket, buffer, sizeof(buffer)))) {
    buffer[bytesRead] = 0;
    data << buffer;
  }
  BIO_free(socket);
  return true;
}
