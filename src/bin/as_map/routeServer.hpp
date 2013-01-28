#ifndef ROUTE_SERVER_HPP
#define ROUTE_SERVER_HPP

#include <string>
#include <openssl/bio.h>
#include <openssl/err.h>

class RouteServer {
  public:
    pthread_t thread;
    bool status;
    BIO *socket;
    std::string address;
    time_t lastReadTime;
    RouteServer();
    int lock();
    int unlock();
    ~RouteServer();
  private:
    pthread_mutex_t _lock;
};

RouteServer::RouteServer() {
  pthread_mutex_init(&_lock, NULL);
  status = true;
  lastReadTime = 0;
}

int RouteServer::lock() {
  return pthread_mutex_lock(&_lock);
}

int RouteServer::unlock() {
  return pthread_mutex_unlock(&_lock);
}

RouteServer::~RouteServer() {
  pthread_mutex_destroy(&_lock);
}

#endif
