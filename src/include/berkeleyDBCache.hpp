#include <db44/db.h>
#include <string>
#include <errno.h>

#include<iostream>
using namespace std;

template <typename Key, typename Data>
class BerkeleyDBCache {
  public:
    typedef void (*aggregateFunction)(Data*, const Data&);
    BerkeleyDBCache(const std::string&, const std::string&, aggregateFunction, bool);
    bool operator!() const;
    bool write(const Key&, const Data&);
    std::string error();
    bool close();
    bool openForReading();
    bool read(Key*&, Data*&);
    bool readNextNonDuplicate(Key*&, Data*&);
    bool readNextDuplicate(Key*&, Data*&);
    ~BerkeleyDBCache();
  private:
    DB_ENV *environment;
    DB *database;
    DBC *cursor;
    DBT _key;
    DBT _data;
    int status;
    aggregateFunction _aggregate;
    char *_directory;
    std::string _fileName;
    bool _error;
    bool _allowDuplicates;
    std::string errorMessage;
};

template <typename Key, typename Data>
BerkeleyDBCache <Key, Data>::BerkeleyDBCache(const std::string &directory,
                                             const std::string &fileName,
                                             aggregateFunction aggregate,
				             bool allowDuplicates) {
  bzero(&_key, sizeof(_key));
  bzero(&_data, sizeof(_data));
  _directory = strdup(directory.c_str());
  _fileName = fileName;
  _allowDuplicates = allowDuplicates;
  status = db_env_create(&environment, 0);
  if (status != 0) {
    _error = true;
    errorMessage = db_strerror(status);
  }
  else {
    status = environment -> open(environment, _directory,
                                 DB_CREATE | DB_INIT_MPOOL, 0);
    if (status != 0) {
      _error = true;
      errorMessage = db_strerror(status);
    }
    else {
      status = db_create(&database, environment, 0);
      if (status != 0) {
        _error = true;
        errorMessage = db_strerror(status);
      }
      else{
        if(allowDuplicates){
          status = database -> set_flags(database, DB_DUPSORT);
        }
        if (status != 0) {
        _error = true;
        errorMessage = db_strerror(status);
        }
        else {
          status = database -> open(database, NULL, fileName.c_str(), NULL,
                                    DB_BTREE, DB_CREATE | DB_TRUNCATE, 0);
          if (status != 0) {
            _error = true;
            errorMessage = db_strerror(status);
          }
          else {
            _aggregate = aggregate;
            _error = false;
          }
        }
      }
    }
  }
}

template <typename Key, typename Data>
bool BerkeleyDBCache <Key, Data>::operator!() const {
  return _error;
}

template <typename Key, typename Data>
bool BerkeleyDBCache <Key, Data>::write(const Key &key, const Data &data) {
  _key.data = (void*)&key;
  _key.size = sizeof(key);
  if (_allowDuplicates) {
    _data.data = (void*)&data;
    _data.size = sizeof(data);
    status = database -> put(database, NULL, &_key, &_data, 0);
    if (status == DB_KEYEXIST ){
        return true;
    }
    else{
      if (status != 0) {
        errorMessage = db_strerror(status);
        return false;
      }
    }
  }
  else{   
    status = database -> get(database, NULL, &_key, &_data, 0);
    if (status > 0) {
      errorMessage = db_strerror(status);
      return false;
    }
    if (status == DB_NOTFOUND ) {
      _data.data = (void*)&data;
      _data.size = sizeof(data);
      status = database -> put(database, NULL, &_key, &_data, 0);
      if (status != 0) {
        errorMessage = db_strerror(status);
        return false;
      }
    }
    else{
      _aggregate((Data*)_data.data, data);
      status = database -> put(database, NULL, &_key, &_data, 0);
      if (status != 0) {
        errorMessage = db_strerror(status);
        return false;
      }
    }
  }
  return true;
}


template <typename Key, typename Data>
std::string BerkeleyDBCache <Key, Data>::error() {
  return errorMessage;
}

template <typename Key, typename Data>
bool BerkeleyDBCache <Key, Data>::close() {
  status = database -> close(database, 0);      
  if (status != 0) {
    errorMessage = db_strerror(status);
    return false;
  }
  status = environment -> close(environment, 0);
  if (status != 0) {
    errorMessage = db_strerror(status);
    return false;
  }
  return true;
}

template <typename Key, typename Data>
bool BerkeleyDBCache <Key, Data>::openForReading() {
  status = db_env_create(&environment, 0);
  if (status != 0) {
    errorMessage = db_strerror(status);
    return false;
  }
  status = environment -> open(environment, _directory, DB_INIT_MPOOL, 0);
  if (status != 0) {
    errorMessage = db_strerror(status);
    return false;
  }
  status = db_create(&database, environment, 0);
  if (status != 0) {
    errorMessage = db_strerror(status);
    return false;
  }
  status = database -> open(database, NULL, _fileName.c_str(), NULL, DB_BTREE,
                            DB_RDONLY, 0);
  if (status != 0) {
    errorMessage = db_strerror(status);
    return false;
  }
  status = database -> cursor(database, NULL, &cursor, 0);  
  if (status != 0) {
    errorMessage = db_strerror(status);
    return false;
  }
  return true;
}

template <typename Key, typename Data>
bool BerkeleyDBCache <Key, Data>::read(Key *&key, Data *&data) {
  status = cursor -> c_get(cursor, &_key, &_data, DB_NEXT);
  if (status == 0) {
    key = (Key*)_key.data;
    data = (Data*)_data.data;
    return true;
  }
  errorMessage = db_strerror(status);
  return false;
}

template <typename Key, typename Data>
bool BerkeleyDBCache <Key, Data>::readNextNonDuplicate(Key *&key, Data *&data) {
  status = cursor -> c_get(cursor, &_key, &_data, DB_NEXT_NODUP);
  if (status == 0) {
    key = (Key*)_key.data;
    data = (Data*)_data.data;
    return true;
  }
  errorMessage = db_strerror(status);
  return false;
}

template <typename Key, typename Data>
bool BerkeleyDBCache <Key, Data>::readNextDuplicate(Key *&key, Data *&data) {
  status = cursor -> c_get(cursor, &_key, &_data, DB_NEXT_DUP);
  if (status == 0) {
    key = (Key*)_key.data;
    data = (Data*)_data.data;
    return true;
  }
  errorMessage = db_strerror(status);
  return false;
}

template <typename Key, typename Data>
BerkeleyDBCache <Key, Data>::~BerkeleyDBCache() {
  delete _directory;
}
