#ifndef BERKELEY_DB_READER_H
#define BERKELEY_DB_READER_H

#include <db44/db.h>
#include <vector>
#include <list>
#include <string>

enum { BDB_OK, BDB_NEW_DB, BDB_DONE };

class BerkeleyDBReader {
  public:
    BerkeleyDBReader();
    ~BerkeleyDBReader();
    void addFiles(const std::vector <std::string>&);
    unsigned int read(DBT&, DBT&);
    const std::string &file();
    bool finished() const;
  private:
    std::list <std::string> files;
    std::string _file;
    DB *database;
    DBC *cursor;
    bool newDatabase;
    bool _finished;
    bool openNextDatabase();
    bool closeCurrentDatabase();
};

#endif
