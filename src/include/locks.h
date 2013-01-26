#ifndef LOCKS_H
#define LOCKS_H

#include <string>
#include <stack>
#include <pthread.h>

class Locks {
  public:
    Locks();
    Locks(const size_t);
    bool initialize(const size_t);
    const bool &operator!() const;
    pthread_mutex_t *allocate();
    bool free(pthread_mutex_t*);
    int lock();
    int unlock();
    const size_t &size() const;
    const size_t &capacity() const;
    bool full();
    ~Locks();
  private:
    bool error;
    size_t _size;
    size_t _capacity;
    pthread_mutex_t *beginning;
    pthread_mutex_t *end;
    pthread_mutex_t *tempPtr;
    std::stack <pthread_mutex_t*> locks;
    pthread_mutex_t _lock;
    bool initialized;
    bool _initialize(const size_t&);
};

#endif
