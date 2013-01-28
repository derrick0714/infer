#ifndef REGISTRY_HPP
#define REGISTRY_HPP

#include <sys/param.h>
#include <fetch.h>

class Registry {
  public:
    url *_url;
    pthread_t thread;
};

#endif
