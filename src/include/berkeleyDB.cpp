#include <iostream>
#include <vector>
#include <sys/stat.h>
#include <errno.h>

#include "berkeleyDB.h"

_BerkeleyDB::_BerkeleyDB() {
  bzero(&key, sizeof(key));
  bzero(&data, sizeof(data));
}

BerkeleyDB::BerkeleyDB() {
}

BerkeleyDB::BerkeleyDB(const std::string baseDirectory,
                       const std::string baseFileName, const uint32_t timeout) {
  _initialize(baseDirectory, baseFileName, timeout);
}

void BerkeleyDB::initialize(const std::string baseDirectory,
                            const std::string baseFileName,
                            const uint32_t timeout) {
  _initialize(baseDirectory, baseFileName, timeout);
}

const bool &BerkeleyDB::operator!() const {
  return error;
}

/* Given a time, returns the hour of the day in the format of 00 through 23. */
std::string BerkeleyDB::getHour(const uint32_t &hourStartTime) {
  time_t time = hourStartTime;
  tm _tm;
  gmtime_r(&time, &_tm);
  std::ostringstream hour;
  if (_tm.tm_hour < 10) {
    hour << '0';
  }
  hour << _tm.tm_hour;
  return hour.str();
}

/* Given a time, returns an absolute data directory. */
std::string BerkeleyDB::getDataDirectory(const time_t &time) {
  tm _tm;
  gmtime_r(&time, &_tm);
  std::ostringstream dataDirectory;
  dataDirectory << _baseDirectory << '/' << _tm.tm_year + 1900 << '/';
  if (_tm.tm_mon < 9) {
    dataDirectory << '0';
  }
  dataDirectory << (_tm.tm_mon + 1) << '/';
  if (_tm.tm_mday < 10) {
    dataDirectory << '0';
  }
  dataDirectory << _tm.tm_mday << '/';
  return dataDirectory.str();
}

/* Recursively creates an absolute directory. */
bool BerkeleyDB::makeDirectory(const std::string &directory,
                               const mode_t mode) {
  size_t lastPosition = 0, position;
  std::string currentDirectory;
  do {
    position = directory.find('/', lastPosition);
    currentDirectory += directory.substr(lastPosition,
                                         position - lastPosition) + '/';
    lastPosition = position + 1;
    if (currentDirectory != "/" &&
        mkdir(currentDirectory.c_str(), mode) == -1 && errno != EEXIST) {
      return false;
    }
  } while (position != std::string::npos);
  return true;
}

/*
 * Given a time, opens the appropriate Berkeley DB database--creating it if it
 * doesn't exist, along with any directories in its path--and sets its record
 * number to the appropriate value (1 for new databases, last record number + 1
 * for existing databases).
 */
std::tr1::unordered_map <uint32_t, _BerkeleyDB*>::iterator BerkeleyDB::createDatabase(const uint32_t &hourStartTime) {
  std::string dataFileName = getDataDirectory(hourStartTime);
  std::tr1::unordered_map <uint32_t, _BerkeleyDB*>::iterator databaseItr;
  if (!makeDirectory(dataFileName, 0755)) {
    return databases.end();
  }
  databaseItr = databases.insert(std::make_pair(hourStartTime,
                                                new _BerkeleyDB)).first;
  dataFileName += _baseFileName + '_' + getHour(hourStartTime);
  if (db_create(&(databaseItr -> second -> database), NULL, 0) != 0) {
    return databases.end();
  }
  if (databaseItr -> second -> database -> open(databaseItr -> second -> database,
                                                NULL, dataFileName.c_str(),
                                                NULL, DB_RECNO, DB_CREATE,
                                                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) != 0) {
    return databases.end();
  }
  if (databaseItr -> second -> database -> cursor(databaseItr -> second -> database,
                                                  NULL, &(databaseItr -> second -> cursor),
                                                  0) != 0) {
    return databases.end();
  }
  if (databaseItr -> second -> cursor -> c_get(databaseItr -> second -> cursor,
                                               &(databaseItr -> second -> key),
                                               &(databaseItr -> second -> data),
                                               DB_LAST) == DB_NOTFOUND) {
    databaseItr -> second -> recordNumber = 1;
  }
  else {
    databaseItr -> second -> recordNumber = *(uint32_t*)databaseItr -> second -> key.data + 1;
  }
  databaseItr -> second -> cursor -> c_close(databaseItr -> second -> cursor);
  return databaseItr;
}

/*
 * Given a time, returns an iterator to the appropriate database, creating it if
 * it doesn't exist.
 */
std::tr1::unordered_map <uint32_t, _BerkeleyDB*>::iterator BerkeleyDB::findDatabase(const uint32_t hourStartTime) {
  std::tr1::unordered_map <uint32_t, _BerkeleyDB*>::iterator databaseItr = databases.find(hourStartTime);
  if (databaseItr != databases.end()) {
    return databaseItr;
  }
  return createDatabase(hourStartTime);
}

/*
 * Given a record, its size, and its start time, writes it to the appropriate
 * database, creating it if it doesn't exist.
 */
bool BerkeleyDB::write(const void* data, const size_t dataSize, const uint32_t startTime) {
  std::tr1::unordered_map <uint32_t, _BerkeleyDB*>::iterator databaseItr = findDatabase(startTime - (startTime % 3600));
  if (databaseItr != databases.end()) {
    databaseItr -> second -> key.size = sizeof(databaseItr -> second -> recordNumber);
    databaseItr -> second -> key.data = &(databaseItr -> second -> recordNumber);
    databaseItr -> second -> data.size = dataSize;
    databaseItr -> second -> data.data = (void*)data;
    if (databaseItr -> second -> database -> put(databaseItr -> second -> database,
                                                 NULL, &(databaseItr -> second -> key),
                                                 &(databaseItr -> second -> data), 0) == 0) {
      ++databaseItr -> second -> recordNumber;
      return true;
    }
  }
  return false;
}

/*
 * Writes any records in Berkeley DB's cache to disk and closes any databases
 * that have been open for as long as or longer than the timeout.
 */
bool BerkeleyDB::flush() {
  uint32_t currentTime = time(NULL);
  std::vector <std::tr1::unordered_map <uint32_t, _BerkeleyDB*>::iterator> deleteList;
  for (std::tr1::unordered_map <uint32_t, _BerkeleyDB*>::iterator databaseItr = databases.begin();
       databaseItr != databases.end(); ++databaseItr) {
    if (databaseItr -> second -> database -> sync(databaseItr -> second -> database, 0) != 0) {
      return false;
    }
    if (currentTime >= databaseItr -> first + 3600 + _timeout) {
      if (databaseItr -> second -> database -> close(databaseItr -> second -> database, 0) != 0) {
        return false;
      }
      else {
        deleteList.push_back(databaseItr);
      }
    }
  }
  for (size_t index = 0; index < deleteList.size(); ++index) {
    databases.erase(deleteList[index]);
  }
  return true;
}

BerkeleyDB::~BerkeleyDB() {
  for (std::tr1::unordered_map <uint32_t, _BerkeleyDB*>::iterator databaseItr = databases.begin();
       databaseItr != databases.end(); ++databaseItr) {
    databaseItr -> second -> database -> close(databaseItr -> second -> database, 0);
  }
}

void BerkeleyDB::_initialize(const std::string baseDirectory,
                             const std::string baseFileName,
                             const uint32_t timeout) {
  _baseDirectory = baseDirectory;
  _baseFileName = baseFileName;
  _timeout = timeout;
}
