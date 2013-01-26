/*
 * A getopt()-like command-line option parser with support for multi-character
 * options.
 */

#ifndef OPTIONS_H
#define OPTIONS_H

#include <string>
#include <tr1/unordered_map>

#include "stringHelpers.h"

class Options {
  public:
    Options();
    Options(int argc, char *const *argv, const char *optionString);
    void initialize(int argc, char *const *argv, const char *optionString);
    int getOption();
    const std::string &getArgument() const;
    const int &getIndex() const;
    bool operator!() const;
    const std::string &error() const;
  private:
    bool initialized;
    std::tr1::unordered_map <std::string, std::pair <int, bool> > options;
    std::tr1::unordered_map <std::string, std::pair <int, bool> >::const_iterator optionItr;
    int _argc;
    int __argc;
    char *const *_argv;
    std::string argument;
    bool _error;
    std::string errorMessage;
    void _initialize(int argc, char *const *argv, const char *optionString);
};

#endif
