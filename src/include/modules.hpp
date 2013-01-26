#include <string>
#include <map>
#include <vector>
#include <tr1/unordered_set>
#include <tr1/unordered_map>
#include <dlfcn.h>

#include "configuration.hpp"
#include "timeStamp.h"
#include "sqlTime.h"
#include "connectionInitiator.hpp"
#include "ipInformation.h"
#include "reputationInformation.hpp"
#include "dnsRecords.hpp"
#include "roles.h"
#include "seenIPsInformation.hpp"
#include "nameResolution.h"
#include "stringHelpers.h"
#include "PostgreSQLConnection.hpp"

class SharedState {
  public:
	PGconn *postgreSQL;
	size_t flushSize;
	const char *date;
	std::vector <std::pair <uint32_t, uint32_t> > *localNetworks;
	std::tr1::unordered_map <uint32_t, std::string> *liveIPs;
	ConnectionInitiators *connectionInitiators;
	IPInformation *ipInformation;
	ReputationInformation *reputationInformation;
	std::tr1::unordered_multimap <uint32_t, Role> *roles;
	SeenIPsInformation *seenIPsInformation;
	NameResolution *nameResolution;
};

class ModuleState {
  public:
	std::string directory;
	configuration conf;
};

typedef bool (*initializeFunction)(SharedState&, ModuleState&);
typedef void (*aggregateFunction)(const void*, size_t);
typedef int (*commitFunction)(PostgreSQLConnection &, size_t&, const char*);

class Module {
  public:
	Module(const configuration &conf,
		   const std::string &moduleDirectory,
		   const std::string &moduleName);
	bool virulence();
	std::string columnName();
	ModuleState moduleState;
	bool operator!() const;
	std::string error();
	std::string name();
	initializeFunction initialize;
	aggregateFunction aggregate;
	commitFunction commit;
  private:
	std::string _name;
	bool _error;
	std::string errorMessage;
};

inline
Module::Module(const configuration &conf,
			   const std::string &moduleDirectory,
			   const std::string &moduleName) {
	std::string moduleFileName(moduleDirectory + "/lib" + moduleName + ".so");
	_error = false;
	_name = moduleName;
	moduleState.conf = conf;
	moduleState.directory = moduleDirectory;
	void *handle = dlopen(moduleFileName.c_str(), RTLD_NOW);
	if (!handle) {
		_error = true;
		errorMessage = dlerror();
	}
	else {
		initialize = (initializeFunction)dlsym(handle, "initialize");
		aggregate = (aggregateFunction)dlsym(handle, "aggregate");
		commit = (commitFunction)dlsym(handle, "commit");
		if (!initialize || !aggregate || !commit) {
			_error = true;
			errorMessage = moduleFileName + ": " + dlerror();
		}
	}
}

inline
bool Module::operator!() const {
	return _error;
}

inline
std::string Module::error() {
	return errorMessage;
}

inline
std::string Module::name() {
	return _name;
}

std::string getCWD() {
		char cwd[PATH_MAX];
		getcwd(cwd, PATH_MAX);
		return cwd;
}
std::string getProgramDirectory
				(const std::string programName)
{
		if (programName[0] == '/') {
				return programName.substr(0, programName.rfind('/'));
		}
		return (getCWD() + '/' + programName.substr(0, programName.rfind('/')));
}

bool loadModules(std::vector<std::vector<Module> > &modules,
				 const std::vector<std::string> &stages,
				 const std::string &module_path,
				 const std::string &module_prefix,
				 const configuration &conf)
{
	std::vector<std::string> module_names;

	for (std::vector<std::string>::const_iterator stage(stages.begin());
		 stage != stages.end();
		 ++stage)
	{
		module_names = explodeString(*stage, " ");
		modules.push_back(std::vector<Module>());
		for (std::vector<std::string>::iterator name(module_names.begin());
			 name != module_names.end();
			 ++name)
		{
			modules.rbegin()->push_back(Module(conf,
											   module_path,
											   module_prefix + *name));
			if (!(*(modules.rbegin()->rbegin()))) {
				return false;
			}
		}
	}

	return true;
}
