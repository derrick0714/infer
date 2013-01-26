#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <vector>
#include <algorithm>
#include <dirent.h>
#include <sys/stat.h>

#include "timeStamp.h"
#include "sqlTime.h"
#include "StrftimeReadEnumerator.hpp"

void getDataFileNames(const std::string &dbHome,
                      std::string baseFileName, const TimeStamp &startTime,
                      const TimeStamp &endTime,
                      std::vector <std::string> &fileNames)
{
	StrftimeReadEnumerator readEnum(dbHome,
									"%Y/%m/%d/" + 
										baseFileName + "_%H",
									startTime,
									endTime);

	for (StrftimeReadEnumerator::const_iterator it = readEnum.begin();
		 it != readEnum.end();
		 ++it)
	{
		fileNames.push_back(it -> string());
	}
}

/*
 * Returns all files with data whose start time is between startTime and
 * endTime local time.
*/
void getDataFileNames(const std::vector <std::string> &dbHomes,
                      std::string baseFileName, const SQLTime localStartTime,
                      const SQLTime localEndTime,
                      std::vector <std::string> &fileNames) {
  /*
   * Converts the local times we were given to UTC, which is the time zone files
   * are named according to.
   */
  SQLTime startTime = getSQLTime(getUNIXTime(localStartTime) - getUTCOffset());
  SQLTime endTime = getSQLTime(getUNIXTime(localEndTime) - getUTCOffset());
  std::string fileName;
  baseFileName += '_';
  while (startTime < endTime) {
    for (size_t dbHome = 0; dbHome < dbHomes.size(); ++dbHome) {
      fileName = dbHomes[dbHome] + '/' + startTime.year + '/' +
                 startTime.month + '/' + startTime.day + '/' + baseFileName +
                 startTime.hour;
      if (access(fileName.c_str(), R_OK) == 0) {
        fileNames.push_back(fileName);
      }
    }
    startTime = getSQLTime(getUNIXTime(startTime) + 3600);
  }
}

template <typename Iterator>
uint64_t getDataSize(Iterator begin, Iterator end) {
  uint64_t size = 0;
  struct stat _stat;
  for (Iterator it(begin); it != end; ++it) {
    stat(it->string().c_str(), &_stat);
    size += _stat.st_size;
  }
  return size;
}

uint64_t getDataSize(const std::vector <std::string> &fileNames) {
  uint64_t size = 0;
  struct stat _stat;
  for (size_t fileName = 0; fileName < fileNames.size(); ++fileName) {
    stat(fileNames[fileName].c_str(), &_stat);
    size += _stat.st_size;
  }
  return size;
}

#endif
