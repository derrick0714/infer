#include "modules.h"

#include <iostream>

Module::Module(const std::string &outputDirectory,
               const std::string &moduleDirectory,
               const std::string &moduleName, const size_t &maxPacketQueueSize,
               const int &snapLength, const configuration &conf,
			   const std::string &filter_string) {
  std::string moduleFileName = moduleDirectory + "/lib" + moduleName + ".so";
  pcap_t *pcapDescriptor;
  _error = false;
  _processedPackets = 0;
  void *handle = dlopen(moduleFileName.c_str(), RTLD_NOW);
  if (!handle) {
    _error = true;
    errorMessage = dlerror();
  }
  else {
    initialize = (initializeFunction)dlsym(handle, "initialize");
    processPacket = (processPacketFunction)dlsym(handle, "processPacket");
    flush = (flushFunction)dlsym(handle, "flush");
    finish = (finishFunction)dlsym(handle, "finish");
    if (!initialize || !processPacket || !flush || !finish) {
      _error = true;
      errorMessage = moduleFileName + ": " + dlerror();
    }
    else {
      initialize(conf, outputDirectory, moduleDirectory);
      pcapDescriptor = pcap_open_dead(DLT_EN10MB, snapLength);
      if (pcapDescriptor == NULL) {
        _error = true;
        errorMessage = pcap_geterr(pcapDescriptor);
      }
      else {
        if (pcap_compile(pcapDescriptor, &_bpfProgram,
                         filter_string.c_str(), 1, 0) == -1) {
          _error = true;
          errorMessage = moduleName + ": " + pcap_geterr(pcapDescriptor);
        }
      }
      pcap_close(pcapDescriptor);
      packetQueue.initialize(maxPacketQueueSize);
    }
  }
}

const bool &Module::operator!() const {
  return _error;
}

const std::string &Module::error() const {
  return errorMessage;
}

const bpf_program &Module::bpfProgram() const {
  return _bpfProgram;
}

void Module::incrementProcessedPackets() {
  ++_processedPackets;
}

const uint64_t &Module::processedPackets() const {
  return _processedPackets;
}

ModuleGroup::ModuleGroup(const std::string outputDirectory,
                         const std::string moduleDirectory,
                         const std::vector <std::string> &moduleNames,
                         const size_t &maxPacketQueueSize,
                         const int &snapLength, const configuration &conf,
						 const std::vector<std::string> &filter_strings)
{
  pthread_mutex_init(&_lock, NULL);
  pthread_mutex_init(&queueSizeLock, NULL);
  pthread_mutex_init(&statusLock, NULL);
  pthread_cond_init(&activationCondition, NULL);
  _error = false;
  status = false;
  size = 0;
  _wakeUps = 0;
  for (size_t index = 0; index < moduleNames.size(); ++index) {
    modules.push_back(Module(outputDirectory, moduleDirectory,
                             moduleNames[index], maxPacketQueueSize,
                             snapLength, conf, filter_strings[index]));
    if (!modules[index]) {
      _error = true;
      errorMessage = modules[index].error();
      break;
    }
  }
}

const bool &ModuleGroup::operator!() const {
  return _error;
}
   
const std::string &ModuleGroup::error() const {
  return errorMessage;
}

int ModuleGroup::waitForWork() {
  return pthread_cond_wait(&activationCondition, &_lock);
}

int ModuleGroup::tryLock() {
  return pthread_mutex_trylock(&_lock);
}

int ModuleGroup::lock() {
  return pthread_mutex_lock(&_lock);
}

int ModuleGroup::unlock() {
  return pthread_mutex_unlock(&_lock);
}

void ModuleGroup::setActive() {
  pthread_mutex_lock(&statusLock);
  status = true;
  pthread_mutex_unlock(&statusLock);
}

void ModuleGroup::setInactive() {
  pthread_mutex_lock(&statusLock);
  status = false;
  pthread_mutex_unlock(&statusLock);
}

void ModuleGroup::checkForWork() {
  pthread_mutex_lock(&statusLock);
  if (!status && !empty()) {
    pthread_cond_broadcast(&activationCondition);
    ++_wakeUps;
  }
  pthread_mutex_unlock(&statusLock);
}

int ModuleGroup::lockQueueSize() {
  return pthread_mutex_lock(&queueSizeLock);
}

int ModuleGroup::unlockQueueSize() {
  return pthread_mutex_unlock(&queueSizeLock);
}

void ModuleGroup::incrementQueueSize() {
  ++size;
}

void ModuleGroup::decrementQueueSize() {
  --size;
}

bool ModuleGroup::empty() {
  return (size == 0);
}

uint64_t ModuleGroup::processedPackets() const {
  size_t _processedPackets = 0;
  for (size_t module = 0; module < modules.size(); ++module) {
    _processedPackets += modules[module].processedPackets();
  }
  return _processedPackets;
}

const uint64_t &ModuleGroup::wakeUps() const {
  return _wakeUps;
}

void ModuleGroup::finish() {
  if (!status) {
    pthread_cond_broadcast(&activationCondition);
  }
  pthread_join(processingThread, NULL);
}

std::string ModuleGroup::getCWD() const {
	char cwd[PATH_MAX];
	getcwd(cwd, PATH_MAX);
	return cwd;
}
std::string ModuleGroup::getProgramDirectory
		(const std::string &programName)
{
	if (programName[0] == '/') {
		return programName.substr(0, programName.rfind('/'));
	}
	return (getCWD() + '/' + programName.substr(0, programName.rfind('/')));
}
