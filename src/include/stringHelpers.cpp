#include "stringHelpers.h"

void implodeString(std::string &_string,
                   const std::vector  <std::string> &array,
                   const std::string delimiter) {
  for (size_t index = 0; index < array.size(); ++index) {
    _string += array[index];
    if (index < array.size() - 1) {
      _string += delimiter;
    }
  }
}

std::string implodeString(const std::vector <std::string> &array,
                          const std::string delimiter) {
  std::string _string;
  implodeString(_string, array, delimiter);
  return _string;
}

void explodeString(std::vector <std::string> &array, const std::string &_string,
                   const std::string delimiter) {
  size_t startPosition = 0, endPosition;
  do {
    endPosition = _string.find(delimiter, startPosition);
    array.push_back(_string.substr(startPosition, endPosition - startPosition));
    startPosition = endPosition + 1;
  } while (endPosition != std::string::npos);
}

std::vector <std::string> explodeString(const std::string &_string,
                                        const std::string delimiter) {
  std::vector <std::string> array;
  explodeString(array, _string, delimiter);
  return array;
}

void append(std::string &_string, const char &data) {
  _string += data;
}
           
void append(std::string &_string, const uint8_t &data) {
  _string += data;
}
    
void append(std::string &_string, uint16_t &data, bool swap) {
  if (swap) {
    data = htons(data);
  }
  _string.append((const char*)&data, sizeof(data));
}
    
void append(std::string &_string, const uint16_t &data, bool swap) {
  static uint16_t _data;
  if (swap) {
    _data = htons(data);
    _string.append((const char*)&_data, sizeof(data));
  }
  else {
    _string.append((const char*)&data, sizeof(data));
  }
}
    
void append(std::string &_string, const uint32_t &data, bool swap) {
  static uint32_t _data;
  if (swap) {
    _data = htonl(data);
    _string.append((const char*)&_data, sizeof(_data));
  }
  else {
    _string.append((const char*)&data, sizeof(_data));
  }
}
    
void append(std::string &_string, uint32_t &data, bool swap) {
  if (swap) {
    data = htonl(data);
  }
  _string.append((const char*)&data, sizeof(data));
}
 
void append(std::string &_string, const std::string &data) {
  _string += data;
}
  
void append(std::string &_string, const int32_t &data, bool swap) {
  static int32_t _data;
  if (swap) {
    _data = htonl(data);
    _string.append((const char*)&_data, sizeof(_data));
  }
  else {
    _string.append((const char*)&data, sizeof(_data));
  }
}
          
void append(std::string &_string, int32_t &data, bool swap) {
  if (swap) {
    data = htonl(data);
  }
  _string.append((const char*)&data, sizeof(data));
}
