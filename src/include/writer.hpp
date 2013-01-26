#ifndef WRITER_HPP
#define WRITER_HPP

#include <string>
#include <queue>

#include "berkeleyDB.h"
#include "memory.hpp"

enum { POINTER_WRITER, APPEND_WRITER };

void append(std::string &flow, const char &data) {
  flow += data;
}
    
void append(std::string &flow, const uint8_t &data) {
  flow += data;
}
    
void append(std::string &flow, uint16_t &data, bool swap = false) {
  if (swap) {
    data = htons(data);
  }
  flow.append((const char*)&data, sizeof(data));
}

void append(std::string &flow, const uint16_t &data, bool swap = false) {
  static uint16_t _data;
  if (swap) {
    _data = htons(data);
    flow.append((const char*)&_data, sizeof(data));
  }
  else {
    flow.append((const char*)&data, sizeof(data));
  }
}

void append(std::string &flow, const uint32_t &data, bool swap = false) {
  static uint32_t _data;
  if (swap) {
    _data = htonl(data);
    flow.append((const char*)&_data, sizeof(_data));
  }
  else {
    flow.append((const char*)&data, sizeof(_data));
  }
}

void append(std::string &flow, uint32_t &data, bool swap = false) {
  if (swap) {
    data = htonl(data);
  }
  flow.append((const char*)&data, sizeof(data));
}

void append(std::string &flow, const std::string &data) {
  flow += data;
}

void append(std::string &flow, const int32_t &data, bool swap = false) {
  static int32_t _data;
  if (swap) {
    _data = htonl(data);
    flow.append((const char*)&_data, sizeof(_data));
  }
  else {
    flow.append((const char*)&data, sizeof(_data));
  }
}

void append(std::string &flow, int32_t &data, bool swap = false) {
  if (swap) {
    data = htonl(data);
  }
  flow.append((const char*)&data, sizeof(data));
}

template <class Flow>
class Writer {
  public:
    typedef void *(*pointerFunction)(const Flow&, size_t&);
    typedef void (*appendFunction)(std::string&, const Flow&);
    Writer();
    template <class Function>
    Writer(const std::string, const std::string, const uint32_t,
           const uint8_t, Function, Memory <Flow>*); 
    template <class Function>
    bool initialize(const std::string, const std::string, const uint32_t,
                    const uint8_t, Function, Memory <Flow>*);
    const bool &operator!() const;
    std::string error() const;
    template <class _Flow>
    friend void *writeFlows(void*);
    void write(Flow*, const uint32_t&);
    void flush();
    void finish();
    ~Writer();
  private:
    bool _error;
    bool initialized;
    uint8_t _type;
    Configuration *_sensorConf;
    BerkeleyDB database;
    std::queue <std::pair <Flow*, uint32_t> > writeQueue;
    Memory <Flow> *_memory;
    pthread_t writerThread;
    bool status;
    bool _flush;
    pthread_mutex_t writeLock;
    pthread_mutex_t statusLock;
    pthread_mutex_t queueLock;
    pthread_mutex_t flushLock;
    pthread_cond_t writeCondition;
    bool _write;
    void *_function;
    template <class Function>
    bool _initialize(const std::string, const std::string, const uint32_t,
                     const uint8_t, Function, Memory <Flow>*);
    void _writeFlows();
};

template <class Flow>
void *writeFlows(void *writer) {
  ((Writer <Flow>*)writer) -> _writeFlows();
  return NULL;
}

template <class Flow>
void Writer <Flow>::_writeFlows() {
  void *pointerFlow;
  size_t pointerFlowSize;
  std::string appendFlow;
  while (_write && pthread_mutex_trylock(&writeLock) == 0) {
    pthread_cond_wait(&writeCondition, &writeLock);
    pthread_mutex_lock(&statusLock);
    status = true;
    pthread_mutex_unlock(&statusLock);
    while (!writeQueue.empty()) {
      switch (_type) {
        case POINTER_WRITER:
          pointerFlow = ((pointerFunction)_function)(*(writeQueue.front().first),
                                                     pointerFlowSize);
          database.write(pointerFlow, pointerFlowSize,
                         writeQueue.front().second);
          break;
        case APPEND_WRITER:
          ((appendFunction)_function)(appendFlow, *(writeQueue.front().first));
          database.write(appendFlow.c_str(), appendFlow.length(),
                         writeQueue.front().second);
          appendFlow.clear();
          break;
      }
      if (_memory == NULL) {
        delete writeQueue.front().first;
      }
      else {
        _memory -> lock();
        _memory -> free(writeQueue.front().first);
        _memory -> unlock();
      }
      pthread_mutex_lock(&queueLock);
      writeQueue.pop();
      pthread_mutex_unlock(&queueLock);
    }
    pthread_mutex_lock(&statusLock);
    status = false;
    pthread_mutex_unlock(&statusLock);
    pthread_mutex_lock(&flushLock);
    if (_flush) {
      _flush = false;
      pthread_mutex_unlock(&flushLock);
      database.flush();
    }
    else {
      pthread_mutex_unlock(&flushLock);
    }
    pthread_mutex_unlock(&writeLock);
  }
}

template <class Flow>
Writer <Flow>::Writer() {
  initialized = false;
}

template <class Flow>
template <class Function>
Writer <Flow>::Writer(const std::string baseDirectory,
                          const std::string baseFileName,
                          const uint32_t timeout, const uint8_t type,
                          Function function,
                          Memory <Flow> *memory = NULL) {
  initialized = false;
  if (!_initialize(baseDirectory, baseFileName, timeout, type, function,
                   memory)) {
    _error = true;
  }
  _error = false;
}

template <class Flow>
template <class Function>
bool Writer <Flow>::initialize(const std::string baseDirectory,
                                   const std::string baseFileName,
                                   const uint32_t timeout, const uint8_t type,
                                   Function function,
                                   Memory <Flow> *memory = NULL) {
  initialized = false;
  if (!_initialize(baseDirectory, baseFileName, timeout, type, function,
                   memory)) {
    _error = true;
    return false;
  }
  _error = false;
  return true;
}

template <class Flow>
const bool &Writer <Flow>::operator!() const {
  return _error;
}

template <class Flow>
void Writer <Flow>::write(Flow *flow, const uint32_t &startTime) {
  pthread_mutex_lock(&queueLock);
  writeQueue.push(std::make_pair(flow, startTime));
  pthread_mutex_unlock(&queueLock);
  pthread_mutex_lock(&statusLock);
  if (!status) {
    pthread_cond_broadcast(&writeCondition);
  }
  pthread_mutex_unlock(&statusLock);
}

template <class Flow>
void Writer <Flow>::flush() {
  pthread_mutex_lock(&flushLock);
  _flush = true;
  pthread_mutex_unlock(&flushLock);
}

template <class Flow>
void Writer <Flow>::finish() {
  _write = false;
  pthread_mutex_lock(&statusLock);
  if (!status) {
    pthread_cond_broadcast(&writeCondition);
  }
  pthread_mutex_unlock(&statusLock);
  pthread_join(writerThread, NULL);
}

template <class Flow>
Writer <Flow>::~Writer() {
  if (initialized) {
    pthread_mutex_destroy(&writeLock);
    pthread_mutex_destroy(&statusLock);
    pthread_mutex_destroy(&queueLock);
    pthread_mutex_destroy(&flushLock);
    pthread_cond_destroy(&writeCondition);
  }
}

template <class Flow>
template <class Function>
bool Writer <Flow>::_initialize(const std::string baseDirectory,
                                const std::string baseFileName,
                                const uint32_t timeout, const uint8_t type,
                                Function function,
                                Memory <Flow> *memory) {
  if (!initialized) {
    initialized = true;
    pthread_mutex_init(&writeLock, NULL);
    pthread_mutex_init(&statusLock, NULL);
    pthread_mutex_init(&queueLock, NULL);
    pthread_mutex_init(&flushLock, NULL);
    pthread_cond_init(&writeCondition, NULL);
    _type = type;
    _write = true;
    status = false;
    _flush = false;
    _memory = memory;
    _function = (void*)function;
    database.initialize(baseDirectory, baseFileName, timeout);
    pthread_create(&writerThread, NULL, &writeFlows <Flow>, this);
    return true;
  }
  return false;
}

#endif
