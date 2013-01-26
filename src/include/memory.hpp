#ifndef MEMORY_H
#define MEMORY_H

#include <stack>

template <class T>
class Memory {
  public:
    Memory();
    Memory(const size_t);
    Memory(const size_t, const size_t);
    void initialize(const size_t);
    void initialize(const size_t, const size_t);
    const bool &operator!() const;
    T *allocate();
    bool free(T*);
    int lock();
    int unlock();
    const size_t &size() const;
    const size_t &capacity() const;
    ~Memory();
  private:
    bool error;
    size_t _size;
    size_t _capacity;
    T *beginning;
    T *end;
    T *tempPtr;
    std::stack <T*> blocks;
    pthread_mutex_t _lock;
    bool initialized;
    void _initialize(const size_t, const size_t);
};

template <class T>
Memory <T>::Memory() {
  initialized = false;
}

template <class T>
Memory <T>::Memory(const size_t numBlocks) {
  initialized = false;
  _initialize(numBlocks, 1);
}

template <class T>
Memory <T>::Memory(const size_t numBlocks, const size_t size) {
  initialized = false;
  _initialize(numBlocks, size);
}

template <class T>
void Memory <T>::initialize(const size_t numBlocks) {
  _initialize(numBlocks, 1);
}

template <class T>
void Memory <T>::initialize(const size_t numBlocks,
                            const size_t size) {
  _initialize(numBlocks, size);
}

template <class T>
const bool &Memory <T>::operator!() const {
  return error;
}

template <class T>
T *Memory <T>::allocate() {
  if (initialized) {
    if (blocks.size()) {
      tempPtr = blocks.top();
      blocks.pop();
      ++_size;
      return tempPtr;
    }
  }
  return NULL;
}

template <class T>
bool Memory <T>::free(T *pointer) {
  if (initialized && pointer >= beginning && pointer <= end) {
    blocks.push(pointer);
    --_size;
    return true;
  }
  return false;
}

template <class T>
void Memory <T>::_initialize(const size_t numBlocks, const size_t size) {
  if (!initialized) {
    pthread_mutex_init(&_lock, NULL);
    error = false;
    _size = 0;
    _capacity = numBlocks;
    beginning = new(std::nothrow) T[numBlocks * size];
    if (beginning == NULL) {
      error = true;
    }
    bzero(beginning, numBlocks * size);
    end = beginning + (numBlocks * size);
    for (size_t blockNumber = 0; blockNumber < numBlocks; ++blockNumber) {
      blocks.push(&(beginning[blockNumber * size]));
    }
    initialized = true;
  }
}

template <class T>
int Memory <T>::lock() {
  return pthread_mutex_lock(&_lock);
}

template <class T>
int Memory <T>::unlock() {
  return pthread_mutex_unlock(&_lock);
}

template <class T>
const size_t &Memory <T>::size() const {
  return _size;
}

template <class T>
const size_t &Memory <T>::capacity() const {
  return _capacity;
}

template <class T>
Memory <T>::~Memory() {
  if (initialized) {
    pthread_mutex_destroy(&_lock);
    delete[] beginning;
  }
}

#endif
