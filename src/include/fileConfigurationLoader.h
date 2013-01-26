#ifndef FILE_CONFIGURATION_LOADER_H
#define FILE_CONFIGURATION_LOADER_H

#include "configurationLoader.hpp"

class FileConfigurationLoader : public ConfigurationLoader {
  public:
    FileConfigurationLoader(const std::string &fileName);
    FileConfigurationLoader(const std::string &directoryName, const std::string &programName);
    FileConfigurationLoader();
    void init(const std::string &fileName);
    void init(const std::string &directoryName, const std::string &programName);
	bool load(std::map <std::string, std::vector <std::string> > &options);
    bool operator!() const;
    std::string error() const;
    std::string fileName() const;
  private:
	std::string getCWD() const;
	std::string getProgramDirectory(const std::string&);

    std::string _fileName;
    bool _error;
    std::string errorMessage;
	std::string directoryName;
	std::string programName;
};

#endif
