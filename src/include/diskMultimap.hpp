#ifndef DISKMULTIMAP_HPP
#define DISKMULTIMAP_HPP

#include <list>
#include <string>
#include <map>
#include <db44/db.h>
#include <errno.h>

template <class Key, class Value>
class LRU {
  public:
    LRU();
    LRU(const Value &value);
    Value _value;
    typename std::list <typename std::multimap <Key, LRU <Key, Value> >::iterator>::iterator iterator;
};

template <class Key, class Value>
LRU <Key, Value>::LRU() {}

template <class Key, class Value>
LRU <Key, Value>::LRU(const Value &value) {
  _value = value;
}

template <class Key, class Value>
class DiskMultimap {
  public:
    DiskMultimap();
    DiskMultimap(const std::string fileName, const size_t size);
    bool initialize(const std::string fileName, const size_t size);
    const bool &operator!() const;
    const std::string &error();
    typename std::multimap <Key, LRU <Key, Value> >::iterator find(const Key &key);
    typename std::multimap <Key, LRU <Key, Value> >::iterator end();
    typename std::multimap <Key, LRU <Key, Value> >::const_iterator end() const;
    typename std::multimap <Key, LRU <Key, Value> >::iterator insert(const std::pair <Key, Value> &pair);
    template <class _Key, class _Value>
    friend void *write(void *diskMultimap);
    bool close();
    bool openForReading();
    bool read(const Key *&key, Value *&data);
  private:
    std::multimap <Key, LRU <Key, Value> > container;
    std::list <typename std::multimap <Key, LRU <Key, Value> >::iterator> leastRecentlyUsedList;
    bool initialized;
    bool _error;
    std::string errorMessage;
    std::string _fileName;
    DB *database;
    DBC *cursor;
    DBT _key;
    DBT _data;
    size_t _size;
    bool __write;
    pthread_mutex_t initLock;
    pthread_cond_t initCondition;
    pthread_mutex_t writeLock;
    pthread_mutex_t writeStatusLock;
    bool writeStatus;
    /* Used to ensure consistency between the container and LRU list. */
    pthread_mutex_t bigLock;
    pthread_cond_t writeCondition;
    pthread_cond_t fullCondition;
    pthread_mutex_t hardLimitLock;
    bool _openForReading;
    pthread_t writeThread;
    bool _initialize(const std::string fileName, const size_t size);
    void _write();
};

template <class Key, class Value>
DiskMultimap <Key, Value>::DiskMultimap() {
  initialized = false;
}

template <class Key, class Value>
DiskMultimap <Key, Value>::DiskMultimap(const std::string fileName,
                                        const size_t size = 0) {
  initialized = false;
  _initialize(fileName, size);
}

template <class Key, class Value>
bool DiskMultimap <Key, Value>::initialize(const std::string fileName,
                                           const size_t size = 0) {
  return _initialize(fileName, size);
}

template <class Key, class Value>
const bool &DiskMultimap <Key, Value>::operator!() const {
  return _error;
}
      
template <class Key, class Value>
const std::string &DiskMultimap <Key, Value>::error() {
  return errorMessage;
}

template <class Key, class Value>
typename std::multimap <Key, LRU <Key, Value> >::iterator DiskMultimap <Key, Value>::find(const Key &key) {
  return container.find(key);
}

template <class Key, class Value>
typename std::multimap <Key, LRU <Key, Value> >::iterator DiskMultimap <Key, Value>::end() {
  return container.end();
}

template <class Key, class Value>
typename std::multimap <Key, LRU <Key, Value> >::const_iterator DiskMultimap <Key, Value>::end() const {
  return container.end();
}

template <class Key, class Value>
typename std::multimap <Key, LRU <Key, Value> >::iterator DiskMultimap <Key, Value>::insert(const std::pair <Key, Value> &pair) {
  static typename std::multimap <Key, LRU <Key, Value> >::iterator containerIterator;
  pthread_mutex_lock(&bigLock);
  /* Inserts the element into the container. */
  containerIterator = container.insert(std::make_pair(pair.first, LRU <Key, Value> (pair.second)));
  /* Inserts a reference to the element at the front of the LRU list. */
  leastRecentlyUsedList.push_front(containerIterator);
  containerIterator -> second.iterator = leastRecentlyUsedList.begin();
  if (_size && container.size() == _size) {
    pthread_mutex_unlock(&bigLock);
    pthread_mutex_lock(&hardLimitLock);
    pthread_cond_wait(&fullCondition, &hardLimitLock);
    pthread_mutex_unlock(&hardLimitLock);
   }
  else {
    pthread_mutex_unlock(&bigLock);
  }
  /* Wakes up the write thread if it is sleeping. */
  pthread_mutex_lock(&writeStatusLock);
  if (!writeStatus) {
    pthread_cond_broadcast(&writeCondition);
  }
  pthread_mutex_unlock(&writeStatusLock);
  return containerIterator;
}

template <class Key, class Value>
void *write(void *diskMultimap) {
  ((DiskMultimap <Key, Value>*)diskMultimap) -> _write();
  return NULL;
}

template <class Key, class Value>
void DiskMultimap <Key, Value>::_write() {
  typename std::list <typename std::multimap <Key, LRU <Key, Value> >::iterator>::iterator lruIterator;
  int status;
  while (__write && pthread_mutex_trylock(&writeLock) == 0) {
    if (!initialized) {
      pthread_cond_broadcast(&initCondition);
    }
    pthread_cond_wait(&writeCondition, &writeLock);
    pthread_mutex_lock(&writeStatusLock);
    writeStatus = true;
    pthread_mutex_unlock(&writeStatusLock);
    while (pthread_mutex_lock(&bigLock) == 0 && container.size() > 1) {
      lruIterator = --(leastRecentlyUsedList.end());
      pthread_mutex_unlock(&bigLock);
      _key.data = (void*)&((*lruIterator) -> first);
      _key.size = sizeof((*lruIterator) -> first);
      _data.data = (void*)&((*lruIterator) -> second._value);
      _data.size = sizeof((*lruIterator) -> second._value);
      status = database -> put(database, NULL, &_key, &_data, 0);
      if (status > 0) {
        _error = true;
        errorMessage = "database -> put(): ";
        errorMessage += db_strerror(status);
      }
      pthread_mutex_lock(&bigLock);
      /* Removes the element from the container. */
      container.erase(*lruIterator);
      /* Removes the reference to the element from the LRU list. */
      leastRecentlyUsedList.erase(lruIterator);
      /*
       * If container size enforcement was requested and the container was full
       * prior to the last write, wakes up insert() in the event that it is
       * sleeping.
       */
      if (_size && container.size() == _size - 1) {
        pthread_mutex_unlock(&bigLock);
        pthread_cond_broadcast(&fullCondition);
      }
      else {
        pthread_mutex_unlock(&bigLock);
      }
    }
    pthread_mutex_unlock(&bigLock);
    pthread_mutex_lock(&writeStatusLock);
    writeStatus = false;
    pthread_mutex_unlock(&writeStatusLock);
    pthread_mutex_unlock(&writeLock);
  }
}

template <class Key, class Value>
bool DiskMultimap <Key, Value>::close() {
  int status;
  __write = false;
  pthread_mutex_lock(&writeStatusLock);
  if (!writeStatus) {
    pthread_cond_broadcast(&writeCondition);
  }
  pthread_mutex_unlock(&writeStatusLock);
  pthread_join(writeThread, NULL);
  status = database -> close(database, 0);
  if (status != 0) {
    _error = true;
    errorMessage = "database -> close(): ";
    errorMessage +=  db_strerror(status);
    return false;
  }
  initialized = false;
  return true;
}

template <class Key, class Value>
bool DiskMultimap <Key, Value>::openForReading() {
  int status;
  if (initialized) {
    _error = true;
    errorMessage = "database not closed";
    return false;
  }
  status = db_create(&database, NULL, 0);
  if (status != 0) {
    _error = true;
    errorMessage = "db_create(): " + _fileName + ": " + db_strerror(status);
    return false;
  }
  status = database -> open(database, NULL, _fileName.c_str(), NULL,
                            DB_BTREE, DB_RDONLY, 0);
  if (status != 0) {
    _error = true;
    errorMessage = "database -> open(): " + _fileName + ": " + db_strerror(status);
    return false;
  }
  status = database -> cursor(database, NULL, &cursor, 0);
  if (status != 0) {
    _error = true;
    errorMessage = "database -> cursor(): " + _fileName + ": " + db_strerror(status);
    return false;
  }
  _openForReading = true;
  return true;
}

template <class Key, class Value>
bool DiskMultimap <Key, Value>::read(const Key *&key, Value *&data) {
  static int status = 0;
  static bool readContainer = false;
  static typename std::multimap <Key, LRU <Key, Value> >::iterator containerIterator;
  if (!_openForReading) {
    _error = true;
    errorMessage = "database not open for reading";
    return false;
  }
  status = cursor -> c_get(cursor, &_key, &_data, DB_NEXT);
  if (status != DB_NOTFOUND) {
    if (status != 0 && status != DB_NOTFOUND) {
      _error = true;
      errorMessage = "database -> c_get(): ";
      errorMessage += db_strerror(status);
      return false;
    }
    key = (Key*)_key.data;
    data = (Value*)_data.data;
    containerIterator = container.find(*key);
  }
  else {
    //Needs FIX ::On sequential read when end of DB is reached looking in to container returns a junk value
    //Temporarily returning false here to allow modules from violating PostgreSQL constraints.
    return false;
    if (!readContainer) {
      readContainer = true;
      containerIterator = container.begin();
    }
    else {
      ++containerIterator;
    }
    if (containerIterator == container.end()) {
      _error = true;
      errorMessage = "no more data to read";
      return false;
    }
    key = &(containerIterator -> first);
    data = &(containerIterator -> second._value);
  }
  return true;
}

template <class Key, class Value>
bool DiskMultimap <Key, Value>::_initialize(const std::string fileName,
                                            const size_t size) {
  int status;
  if (!initialized) {
    _error = true;
    _fileName = fileName;
    status = db_create(&database, NULL, 0);
    if (status != 0) {
      errorMessage = "db_create(): " + fileName + ": " + db_strerror(status);
      return false;
    }
    status = database -> set_flags(database, DB_DUPSORT);
    if (status != 0) {
      errorMessage = "database -> open(): " + fileName + ": " + db_strerror(status);
      return false;
    }
    status = database -> open(database, NULL, fileName.c_str(), NULL,
                              DB_BTREE, DB_CREATE | DB_TRUNCATE, 0);
    if (status != 0) {
      errorMessage = "database -> open(): " + fileName + ": " + db_strerror(status);
      return false;
    }
    status = database -> cursor(database, NULL, &cursor, 0);
    if (status != 0) {
      errorMessage = "database -> cursor(): " + fileName + ": " + db_strerror(status);
      return false;
    }
    bzero(&_key, sizeof(_key));
    bzero(&_data, sizeof(_data));
    _size = size;
    status = pthread_mutex_init(&writeStatusLock, NULL);
    if (status != 0) {
      errorMessage = "pthread_mutex_init(): ";
      errorMessage += strerror(errno);
      return false;
    }
    writeStatus = false;
    status = pthread_mutex_init(&bigLock, NULL);
    if (status != 0) {
      errorMessage = "pthread_mutex_init(): ";
      errorMessage += strerror(errno);
      return false;
    }
    status = pthread_cond_init(&writeCondition, NULL);
    if (status != 0) {
      errorMessage = "pthread_cond_init(): ";
      errorMessage += strerror(errno);
      return false;
    }
    status = pthread_cond_init(&fullCondition, NULL);
    if (status != 0) {
      errorMessage = "pthread_cond_init(): ";
      errorMessage += strerror(errno);
      return false;
    }
    status = pthread_mutex_init(&hardLimitLock, NULL);
    if (status != 0) {
      errorMessage = "pthread_mutex_init(): ";
      errorMessage += strerror(errno);
      return false;
    }
    __write = true;
    status = pthread_mutex_init(&initLock, NULL);
    if (status != 0) {
      errorMessage = "pthread_mutex_init(): ";
      errorMessage += strerror(errno);
      return false;
    }
    status = pthread_cond_init(&initCondition, NULL);
    if (status != 0) {
      errorMessage = "pthread_cond_init(): ";
      errorMessage += strerror(errno);
      return false;
    }
    status = pthread_mutex_init(&writeLock, NULL);
    if (status != 0) {
      errorMessage = "pthread_mutex_init(): ";
      errorMessage += strerror(errno);
      return false;
    }
    _openForReading = false;
    status = pthread_create(&writeThread, NULL, &(write <Key, Value>), this);
    if (status != 0) {
      errorMessage = "pthread_create(): ";
      errorMessage += strerror(errno);
      return false;
    }
    pthread_mutex_lock(&initLock);
    pthread_cond_wait(&initCondition, &initLock);
    pthread_mutex_unlock(&initLock);
    initialized = true;
    _error = false;
    return true;
  }                          
  return false;
}

#endif
