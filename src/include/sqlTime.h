#ifndef SQLTIME_H
#define SQLTIME_H

#include <sys/time.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

#include "timeStamp.h"

struct SQLTime {
  public:
    std::string year;
    std::string month;
    std::string day;
    std::string hour;
    std::string minute;
    std::string second;
    std::string getDate();
    std::string getTime();
    std::string getDateTime();
    bool operator==(const SQLTime &rhs) const;
    bool operator<(const SQLTime &rhs) const;
    bool operator <=(const SQLTime &rhs) const;
};

std::string intToString(uint32_t number, const char* format);
SQLTime getSQLTime(uint32_t time);
std::string getDisplayTime(const TimeStamp timeStamp);
bool isSQLDate(const char* sqlDate);
bool isSQLTime(char* sqlTime);
uint32_t getUNIXTime(SQLTime sqlTime);
SQLTime explodeDate(std::string sqlDate, std::string _sqlTime = "");
bool isRelevant(uint32_t &dayStartTime, uint32_t &dayEndTime,
                uint32_t &flowTime);
uint32_t getDayStartTime(const char* date);
uint32_t getDayStartTime(uint32_t t);
uint32_t getNextDayStartTime(uint32_t t);
double diffTime(TimeStamp &previousTime, TimeStamp &currentTime);
long getUTCOffset();

#endif
