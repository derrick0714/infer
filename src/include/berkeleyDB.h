#ifndef BERKELEYDB_H
#define BERKELEYDB_H

#include <tr1/unordered_map>
#include <sstream>
#include <string>
#include <db44/db.h>

class _BerkeleyDB {
  public:
    _BerkeleyDB();
    DB *database;
    DBC *cursor;
    DBT key;
    DBT data;
    uint32_t recordNumber;
};

class BerkeleyDB {
  public:
    BerkeleyDB();
    BerkeleyDB(const std::string, const std::string, const uint32_t);
    void initialize(const std::string, const std::string, const uint32_t);
    const bool &operator!() const;
    bool write(const void *data, const size_t size, const uint32_t startTime);
    bool flush();
    ~BerkeleyDB();
  private:
    bool error;
    std::string errorMessage;
    std::string _baseDirectory;
    std::string _baseFileName;
    uint32_t _timeout;
    /*
     * A hash table of _BerkeleyDB classes allows us to find the one that we
     * will write to quickly given the start time of the record to be written.
     */
    std::tr1::unordered_map <uint32_t, _BerkeleyDB*> databases;
    void _initialize(const std::string, const std::string, const uint32_t);
    std::string getDataDirectory(const time_t&);
    std::string getHour(const uint32_t&);
    std::tr1::unordered_map <uint32_t, _BerkeleyDB*>::iterator findDatabase(const uint32_t);
    std::tr1::unordered_map <uint32_t, _BerkeleyDB*>::iterator createDatabase(const uint32_t&);
    bool makeDirectory(const std::string &directory, const mode_t mode);
};

#endif
