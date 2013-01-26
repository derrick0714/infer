#ifndef MODULES_H
#define MODULES_H

#include <vector>
#include <queue>
#include <dlfcn.h>

#include "packet.h"
#include "queue.hpp"
#include "configuration.hpp"

typedef int (*initializeFunction)(const configuration &, const std::string&,
                                  const std::string&);
typedef int (*processPacketFunction)(const Packet&);
typedef int (*flushFunction)();
typedef int (*finishFunction)();

class Module {
  public:
    initializeFunction initialize;
    processPacketFunction processPacket;
    flushFunction flush;
    finishFunction finish;
    Queue <Packet*> packetQueue;
    Module(const std::string&, const std::string&, const std::string&,
           const size_t&, const int&, const configuration &conf,
		   const std::string &filter_string);
    const bool &operator!() const;
    const std::string &error() const;
    const bpf_program &bpfProgram() const;
    void incrementProcessedPackets();
    const uint64_t &processedPackets() const;
  private:
    bool _error;
    std::string errorMessage;
    bpf_program _bpfProgram;
    uint64_t _processedPackets;
};

class ModuleGroup {
  public:
    ModuleGroup(const std::string, const std::string,
                const std::vector <std::string>&, const size_t&, const int&,
				const configuration &conf,
				const std::vector<std::string> &filter_strings);
    const bool &operator!() const;
    const std::string &error() const;
    int waitForWork();
    int tryLock();
    int lock();
    int unlock();
    void setActive();
    void setInactive();
    void checkForWork();
    int lockQueueSize();
    int unlockQueueSize();
    void incrementQueueSize();
    void decrementQueueSize();
    bool empty();
    uint64_t processedPackets() const;
    const uint64_t &wakeUps() const;
    std::vector <Module> modules;
    void finish();
    pthread_t processingThread;
    pthread_t flushThread;
  private:
	std::string getCWD() const;
	std::string getProgramDirectory(const std::string &);

    bool _error;
    std::string errorMessage;
    bool status;
    size_t size;
    uint64_t _wakeUps;
    pthread_mutex_t _lock;
    pthread_mutex_t queueSizeLock;
    pthread_mutex_t statusLock;
    pthread_cond_t activationCondition;
};

#endif
