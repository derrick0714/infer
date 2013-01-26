#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <errno.h>

#include "configurationLoader.hpp"

class Configuration {
  public:
    Configuration(ConfigurationLoader &loader);
    Configuration();
    bool open(ConfigurationLoader &loader);
    bool operator!() const;
    std::string error() const;
    std::string getString(const std::string);
    std::vector <std::string> &getStrings(const std::string);
    size_t getNumber(const std::string);
  private:
    std::map <std::string, std::vector <std::string> > options;
    bool _error;
    std::string errorMessage;
};

// BEGIN shit that shouldn't be in this file
void printError(const std::string, const std::string);
void printError(const std::string, const std::string, const std::string);
void printError(const std::string, const std::string, const std::string,
                const std::string);

std::string makePGConnectionString(Configuration&);
// END shit that shouldn't be in this file

#endif
