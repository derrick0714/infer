#ifndef BERKELEYDB_HPP
#define BERKELEYDB_HPP

#include <vector>
#include <list>
#include <string>
#include <db44/db.h>

class BerkeleyDB {
  public:
    BerkeleyDB();
    ~BerkeleyDB();
    bool addFileName(const std::string);
    void addFileNames(std::vector <std::string>&);
    bool read(size_t&, void*&, size_t&);
  private:
    std::list <std::string> fileNames;
    DB *database;
    DBC *cursor;
    DBT key;
    DBT data;
    size_t hour;
    bool openNextDatabase();
    bool closeCurrentDatabase();
};

inline BerkeleyDB::BerkeleyDB() {
  database = NULL;
  bzero(&key, sizeof(key));
  bzero(&data, sizeof(data));
}

inline bool BerkeleyDB::addFileName(const std::string _fileName) {
  fileNames.push_back(_fileName);
  if (database == NULL) {
    return openNextDatabase();
  }
  return true;
}

inline void BerkeleyDB::addFileNames(std::vector <std::string> &_fileNames) {
  for (size_t fileName = 0; fileName < _fileNames.size(); ++fileName) {
    fileNames.push_back(_fileNames[fileName]);
  }
  if (database == NULL) {
    openNextDatabase();
  }
}

inline bool BerkeleyDB::closeCurrentDatabase() {
  if (database != NULL) {
    if (cursor -> c_close(cursor) != 0) {
      return false;
    }
    if (database -> close(database, 0) != 0) {
      return false;
    }
  }
  return true;
}

inline BerkeleyDB::~BerkeleyDB() {
  closeCurrentDatabase();
}

inline bool BerkeleyDB::openNextDatabase() {
  if (!closeCurrentDatabase()) {
    return false;
  }
  if (!fileNames.size()) {
    return false;
  }
  if (db_create(&database, NULL, 0) != 0) {
    return false;
  }
  if (database -> open(database, NULL, fileNames.begin() -> c_str(), NULL,
                       DB_RECNO, DB_RDONLY, 0) != 0) {
    database = NULL;
    return false;
  }
  if (database -> cursor(database, NULL, &cursor, 0) != 0) {
    return false;
  }
  hour = strtoul(fileNames.begin() -> substr(fileNames.begin() -> rfind('_') + 1).c_str(), NULL, 10);
  fileNames.pop_front();
  return true;
}

inline bool BerkeleyDB::read(size_t &size, void *&_data, size_t &_hour) {
  while (1) {
    if (cursor -> c_get(cursor, &key, &data, DB_NEXT) == 0) {
      size = data.size;
      _data = data.data;
      _hour = hour;
      return true;
    }
    if (!openNextDatabase()) {
      database = NULL;
      return false;
    }
  }
}

#endif
