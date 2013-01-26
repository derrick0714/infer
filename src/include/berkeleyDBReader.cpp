#include "berkeleyDBReader.h"

BerkeleyDBReader::BerkeleyDBReader() {
  newDatabase = false;
  _finished = true;
}

void BerkeleyDBReader::addFiles(const std::vector <std::string> &_files) {
  for (size_t file = 0; file < _files.size(); ++file) {
    files.push_back(_files[file]);
  }
  if (files.size() > 0 && _finished == true) {
    openNextDatabase();
    _finished = false;
  }
}

bool BerkeleyDBReader::closeCurrentDatabase() {
  if (_finished == false) {
    if (cursor -> c_close(cursor) != 0) {
      return false;
    }
    if (database -> close(database, 0) != 0) {
      return false;
    }
    if (_file.length() > 0) {
      _file.clear();
    }
  }
  return true;
}

BerkeleyDBReader::~BerkeleyDBReader() {
  closeCurrentDatabase();
}

bool BerkeleyDBReader::openNextDatabase() {
  if (files.size() == 0) {
    return false;
  }
  if (!closeCurrentDatabase()) {
    return false;
  }
  if (db_create(&database, NULL, 0) != 0) {
    return false;
  }
  if (database -> open(database, NULL, files.begin() -> c_str(), NULL,
                       DB_RECNO, DB_RDONLY, 0) != 0) {
    return false;
  }
  if (database -> cursor(database, NULL, &cursor, 0) != 0) {
    return false;
  }
  _file = files.front();
  files.pop_front();
  return true;
}

/*
 * Returns BDB_OK if a record was read successfully, BDB_NEW_DB if a record
 * was read successfully and a new database has been opened, or BDB_DONE if
 * there are no more records to read.
 */
unsigned int BerkeleyDBReader::read(DBT &key, DBT &data) {
  while (1) {
    if (cursor -> c_get(cursor, &key, &data, DB_NEXT) == 0) {
      if (newDatabase == false) {
        return BDB_OK;
      }
      newDatabase = false;
      return BDB_NEW_DB;
    }
    if (!openNextDatabase()) {
      _finished = true;
      return BDB_DONE;
    }
    newDatabase = true;
  }
}

const std::string &BerkeleyDBReader::file() {
  return _file;
}

bool BerkeleyDBReader::finished() const {
  return _finished;
}
