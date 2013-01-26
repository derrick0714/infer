/* Common string-processing functions. */

#ifndef STRING_H
#define STRING_H

#include <string>
#include <vector>
#include <netinet/in.h>

void implodeString(std::string &_string,
                   const std::vector  <std::string> &array,
                   const std::string delimiter);
std::string implodeString(const std::vector <std::string> &array,
                          const std::string delimiter);
void explodeString(std::vector <std::string> &array, const std::string &_string,
                   const std::string delimiter);
std::vector <std::string> explodeString(const std::string &_string,
                                        const std::string delimiter);
void append(std::string &_string, const char &data);
void append(std::string &_string, const uint8_t &data);
void append(std::string &_string, uint16_t &data, bool swap = false);
void append(std::string &_string, const uint16_t &data, bool swap = false);
void append(std::string &_string, const uint32_t &data, bool swap = false);
void append(std::string &_string, uint32_t &data, bool swap = false);
void append(std::string &_string, const std::string &data);
void append(std::string &_string, const int32_t &data, bool swap = false);
void append(std::string &_string, int32_t &data, bool swap = false);

#endif
