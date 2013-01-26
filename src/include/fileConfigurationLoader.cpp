#include <fstream>
#include <errno.h>

#include "fileConfigurationLoader.h"

FileConfigurationLoader::FileConfigurationLoader(const std::string &fileName) {
	init(fileName);
}

FileConfigurationLoader::FileConfigurationLoader(const std::string &directoryName,
												 const std::string &programName)
{
	init(directoryName, programName);
}

FileConfigurationLoader::FileConfigurationLoader() 
	:_error(true), errorMessage("FileConfigurationLoader: not initialized.")
{
}

void FileConfigurationLoader::init(const std::string &fileName) {
	//directoryName = getProgramDirectory(programName);
	//this -> programName = programName.substr(programName.rfind('/') + 1);
	_fileName = fileName;
	_error = false;
	errorMessage.clear();
}

void FileConfigurationLoader::init(const std::string &directoryName,
								   const std::string &programName)
{
	this -> directoryName = directoryName;
	this -> programName = programName;
	_fileName = directoryName + '/' + programName + ".conf";
	_error = false;
	errorMessage.clear();
}

bool FileConfigurationLoader::load
		(std::map <std::string, std::vector <std::string> > &options) 
{
	std::string line, option, value;
	size_t delimiter, openingQuote, closingQuote;
	std::ifstream file;
	_error = false;
	file.open(_fileName.c_str());
	if (!file) {
		_error = true;
		errorMessage = _fileName + ": " + strerror(errno);
		return false;
	}
	while (getline(file, line)) {
		delimiter = line.find('=');
		option = line.substr(0, delimiter);
		value = line.substr(delimiter + 1);
		openingQuote = value.find('"');
		if (openingQuote != std::string::npos) {
			closingQuote = value.rfind('"');
			if (closingQuote != std::string::npos) {
				value = value.substr(openingQuote + 1,
									 closingQuote - openingQuote - 1);
			}
		}    
		options[option].push_back(value);
	}
	file.close();
	return true;
}

bool FileConfigurationLoader::operator!() const {
	return _error;
}

std::string FileConfigurationLoader::error() const {
	return errorMessage;
}

std::string FileConfigurationLoader::fileName() const {
	return _fileName;
}

std::string FileConfigurationLoader::getCWD() const {
	char cwd[PATH_MAX];
	getcwd(cwd, PATH_MAX);
	return cwd;
}
std::string FileConfigurationLoader::getProgramDirectory
		(const std::string &programName)
{
	if (programName[0] == '/') {
		return programName.substr(0, programName.rfind('/'));
	}
	return (getCWD() + '/' + programName.substr(0, programName.rfind('/')));
}
