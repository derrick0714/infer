#ifndef CONFIGURATION_LOADER_HPP
#define CONFIGURATION_LOADER_HPP

#include <map>
#include <vector>
#include <string>

class ConfigurationLoader {
  public:
	virtual ~ConfigurationLoader() {};
	virtual bool operator!() const = 0;
    virtual std::string error() const = 0;
	virtual bool load(std::map <std::string, std::vector <std::string> > &options) = 0;
};

#endif
