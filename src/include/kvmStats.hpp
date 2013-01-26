#ifndef KVMSTATS_HPP
#define KVMSTATS_HPP

#include <ostream>
#include <string>
#include <kvm.h>
#include <sys/user.h>
#include <sys/sysctl.h>
#include <fcntl.h>

class KVMStats {
  public:
    KVMStats();
    bool operator!() const;

    bool update();

    void printStats(std::ostream &out, std::string prefix);

    size_t residentSize() const; // in bytes
    size_t virtualSize() const; // in bytes
    
    //friend ostream& operator<<(ostream &out, KVMStats &kvmStats);

  private:
    kvm_t *kvmDescriptor;
    kinfo_proc *kvmProcessInfo;
    int numProcesses;
    int pageSize;
};

inline KVMStats::KVMStats() {
    kvmDescriptor = kvm_open(NULL, "/dev/null", NULL, O_RDONLY, "KVMStats:");
    pageSize = getpagesize();
}

inline bool KVMStats::operator!() const {
    return kvmDescriptor == NULL;
}

inline bool KVMStats::update() {
    kvmProcessInfo = kvm_getprocs(kvmDescriptor, KERN_PROC_PID,
				  getpid(), &numProcesses);

    return kvmProcessInfo != NULL;
}

inline void KVMStats::printStats(std::ostream &out, std::string prefix) {
    out << prefix << " Memory Stats" << std::endl;
    for (size_t i = 0; i < prefix.length() + 1; ++i) {
	out << ' ';
    }
    out << "Resident Size (KiB): " << (double) residentSize() / 1024.0 << std::endl;
    for (size_t i = 0; i < prefix.length() + 1; ++i) {
	out << ' ';
    }
    out << "Virtual Size  (KiB): " << (double) virtualSize() / 1024.0 << std::endl;
}

inline size_t KVMStats::residentSize() const {
    return kvmProcessInfo -> ki_rssize * pageSize;
}

inline size_t KVMStats::virtualSize() const {
    return kvmProcessInfo -> ki_size;
}
    
#endif
