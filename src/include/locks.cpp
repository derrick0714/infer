#include "locks.h"

Locks::Locks() {
  initialized = false;
}

Locks::Locks(const size_t size) {
  initialized = false;
  if (!_initialize(size)) {
    error = true;
  }
  else {
    error = false;
  }
}

bool Locks::initialize(const size_t size) {
  initialized = false;
  if (!_initialize(size)) {
    error = true;
    return false;
  }
  error = false;
  return true;
}

const bool &Locks::operator!() const {
  return error;
}

pthread_mutex_t *Locks::allocate() {
  if (initialized) {
    if (locks.size()) {
      tempPtr = locks.top();
      locks.pop();
      ++_size;
      return tempPtr;
    }
  }
  return NULL;
}

bool Locks::free(pthread_mutex_t *tempPtr) {
  if (initialized && tempPtr >= beginning && tempPtr <= end) {
    locks.push(tempPtr);
    --_size;
    return true;
  }
  return false;
}

int Locks::lock() {
  return pthread_mutex_lock(&_lock);
}

int Locks::unlock() {
  return pthread_mutex_unlock(&_lock);
}

const size_t &Locks::size() const {
  return _size;
}

const size_t &Locks::capacity() const {
  return _capacity;
}

bool Locks::full() {
  return (_size == _capacity);
}

Locks::~Locks() {
  pthread_mutex_destroy(&_lock);
  for (size_t index = 0; index < _capacity; ++index) {
    pthread_mutex_destroy(&(beginning[index]));
  }
  delete[] beginning;
}

bool Locks::_initialize(const size_t &size) {
  if (!initialized) {
    pthread_mutex_init(&_lock, NULL);
    _size = 0;
    _capacity = size;
    beginning = new(std::nothrow) pthread_mutex_t[size];
    if (beginning == NULL) {
      return false;
    }
    end = beginning + size;
    for (size_t lock = 0; lock < _capacity; ++lock) {
      if (pthread_mutex_init(&(beginning[lock]), NULL) != 0) {
        return false;
      }
      locks.push(&(beginning[lock]));
    }
    initialized = true;
    return true;
  }
  return false;
}
