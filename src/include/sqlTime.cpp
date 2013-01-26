#include "sqlTime.h"

std::string SQLTime::getDate() {
  return year + '-' + month + '-' + day;
}

std::string SQLTime::getTime() {
  return hour + ':' + minute + ':' + second;
}

std::string SQLTime::getDateTime() {
  return getDate() + ' ' + getTime();
}

bool SQLTime::operator==(const SQLTime &rhs) const {
  return (year == rhs.year && month == rhs.month && day == rhs.day &&
          hour == rhs.hour && minute == rhs.minute && second == rhs.second);
}

bool SQLTime::operator<(const SQLTime &rhs) const {
  if (year != rhs.year) {
    return (year < rhs.year);
  }
  else {
    if (month != rhs.month) {
      return (month < rhs.month);
    }
    else {
      if (day != rhs.day) {
        return (day < rhs.day);
      }
      else {
        if (hour != rhs.hour) {
          return (hour < rhs.hour);
        }
        else {
          if (minute != rhs.minute) {
            return (minute < rhs.minute);
          }
          else {
            return (second < rhs.second);
          }
        }
      }
    }
  }
}

bool SQLTime::operator <=(const SQLTime &rhs) const {
  return (*this < rhs || *this == rhs);
}

std::string intToString(uint32_t number, const char* format) {
  char* cString = new char[11];
  std::string cxxString;
  sprintf(cString, format, number);
  cxxString = cString;
  delete cString;
  return cxxString;
}

SQLTime getSQLTime(uint32_t time) {
  time_t tempTime = time;
  tm timeStruct;
  localtime_r(&tempTime, &timeStruct);
  SQLTime sqlTime;
  sqlTime.year = intToString(timeStruct.tm_year + 1900, "%d");
  sqlTime.month = intToString(timeStruct.tm_mon + 1, "%02d");
  sqlTime.day = intToString(timeStruct.tm_mday, "%02d");
  sqlTime.hour = intToString(timeStruct.tm_hour, "%02d");
  sqlTime.minute = intToString(timeStruct.tm_min, "%02d");
  sqlTime.second = intToString(timeStruct.tm_sec, "%02d");
  return sqlTime;
};

std::string getDisplayTime(const TimeStamp timeStamp) {
  std::ostringstream timeString;
  if (timeStamp.seconds()) {
    timeString << getSQLTime(timeStamp.seconds()).getDateTime() << '.'
               << std::setw(6) << std::setfill('0') << timeStamp.microseconds();
    return timeString.str();
  }
  return "Not seen";
}

bool isSQLDate(const char* sqlDate) {
  return (strlen(sqlDate) == 10);
  return true;
}

bool isSQLTime(char* sqlTime) {
  return (strlen(sqlTime) == 8);
}

uint32_t getUNIXTime(SQLTime sqlTime) {
  tm time;
  time.tm_year = atoi(sqlTime.year.c_str()) - 1900;
  time.tm_mon = atoi(sqlTime.month.c_str()) - 1;
  time.tm_mday = atoi(sqlTime.day.c_str());
  time.tm_hour = atoi(sqlTime.hour.c_str());
  time.tm_min = atoi(sqlTime.minute.c_str());
  time.tm_sec = atoi(sqlTime.second.c_str());
  time.tm_isdst = -1;
  return mktime(&time);
}

SQLTime explodeDate(std::string sqlDate, std::string _sqlTime) {
  SQLTime sqlTime;
  sqlTime.year = sqlDate.substr(0, 4);
  sqlTime.month = sqlDate.substr(5, 2);
  sqlTime.day = sqlDate.substr(8, 2);
  if (_sqlTime.length()) {
    sqlTime.hour = _sqlTime.substr(0, 2);
    sqlTime.minute = _sqlTime.substr(3, 2);
    sqlTime.second = _sqlTime.substr(6, 2);
  }
  return sqlTime;
}

bool isRelevant(uint32_t &dayStartTime, uint32_t &dayEndTime,
                uint32_t &flowTime) {
  return (flowTime >= dayStartTime && flowTime < dayEndTime);
}

uint32_t getDayStartTime(const char* date){
  tm time;
  strptime(date, "%Y-%m-%d", &time);
  time.tm_hour = 0;
  time.tm_min = 0;
  time.tm_sec = 0;
  time.tm_isdst = -1;
  return mktime(&time);
}

uint32_t getDayStartTime(uint32_t t) {
  tm time;
  time_t _t = static_cast <time_t>(t);
  localtime_r(&_t, &time);
  time.tm_hour = 0;
  time.tm_min = 0;
  time.tm_sec = 0;
  time.tm_isdst = -1;
  return mktime(&time);
}

uint32_t getNextDayStartTime(uint32_t t) {
  tm time;
  time_t _t = static_cast <time_t>(t);
  localtime_r(&_t, &time);
  ++time.tm_mday;
  time.tm_hour = 0;
  time.tm_min = 0;
  time.tm_sec = 0;
  time.tm_isdst = -1;
  return mktime(&time);
}

double diffTime(TimeStamp &previousTime, TimeStamp &currentTime){
    double diffSeconds = currentTime.seconds() - previousTime.seconds();
    int32_t diffMicroseconds = currentTime.microseconds() - previousTime.microseconds();
    if (diffMicroseconds < 0){
      --diffSeconds;
      diffMicroseconds += 1000000;
    }
    return (diffSeconds + (static_cast<double>(diffMicroseconds)/1000000));
}

long getUTCOffset() {
  tm _tm;
  time_t _time = time(NULL);
  localtime_r(&_time, &_tm);
  return _tm.tm_gmtoff;
}
