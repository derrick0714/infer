#include "options.h"

Options::Options() {
  initialized = false;
}

Options::Options(int argc, char *const *argv, const char *optionString) {
  _initialize(argc, argv, optionString);
  initialized = true;
}

void Options::initialize(int argc, char *const *argv,
                         const char *optionString) {
  _initialize(argc, argv, optionString);
  initialized = true;
}

/*
 * Returns a non-negative value corresponding to an option, -1 if there are no
 * more options to process, or -2 if there was an error.
 */
int Options::getOption() {
  /* Checks whether the class has been initialized. */
  if (initialized) {
    /* Checks whether there are any more options to process. */
    if (__argc < _argc && _argv[__argc][0] == '-') {
      /* If so, checks whether the option is valid. */
      optionItr = options.find(_argv[__argc] + 1);
      /* Returns an error if the option is invalid. */
      if (optionItr == options.end()) {
        _error = true;
        errorMessage = "illegal option \"";
        errorMessage += _argv[__argc];
        errorMessage += '"';
        ++__argc;
        return -2;
      }
      /* Otherwise, determines if the option requires an argument. */
      else {
        /* If so, checks whether the option has an argument. */
        if (optionItr -> second.second) {
          /*
           * If so, sets the argument to the word following the option and
           * returns the option's value.
           */
          if (__argc < _argc - 1) {
            argument = _argv[++__argc];
            ++__argc;
            return (optionItr -> second.first);
          }
          /* Otherwise, returns an error. */
          else {
            _error = true;
            errorMessage = "option \"";
            errorMessage += _argv[__argc];
            errorMessage += "\" requires argument";
            ++__argc;
            return -2;
          }
        }
        /*
         * Returns the option's value if it does not require an argument.
         */
        ++__argc;
        return (optionItr -> second.first);
      }
    }
    return -1;
  }
  /* Returns an error if the class has not been initialized. */
  _error = true;
  errorMessage = "object not initialized";
  return -2;
}

const std::string &Options::getArgument() const {
  return argument;
}

const int &Options::getIndex() const {
  return __argc;
}

bool Options::operator!() const {
  return _error;
}
  
const std::string &Options::error() const {
  return errorMessage;
}

void Options::_initialize(int argc, char *const *argv,
                          const char *optionString) {
  /* The supported options are delimited by spaces. */
  std::vector <std::string> optionStrings = explodeString(optionString, " ");
  _argc = argc;
  __argc = 1;
  _argv = argv;
  _error = false;
  /* Assigns values to options in the order they appear, starting with 0. */
  for (size_t option = 0; option < optionStrings.size(); ++option) {
    /* If an option ends with a colon, it requires an argument. */
    if (optionStrings[option][optionStrings[option].length() - 1] == ':') {
      options.insert(std::make_pair(optionStrings[option].substr(0, optionStrings[option].length() - 1),
                                    std::make_pair(option, true)));
    }
    /* Otherwise, it doesn't. */
    else {
      options.insert(std::make_pair(optionStrings[option],
                                    std::make_pair(option, false)));
    }
  }
}
