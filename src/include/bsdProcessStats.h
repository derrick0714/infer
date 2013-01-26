#include <fcntl.h>
#include <kvm.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/user.h>

#define PROCESS_STATS_SCHEMA_NAME "ProcessStats"
#define PROCESS_STATS_TABLE_SCHEMA "\"processName\" TEXT NOT NULL, \
                                    \"percentComplete\" SMALLINT NOT NULL, \
                                    \"startTime\" uint32 NOT NULL, \
                                    \"endTime\" uint32 NOT NULL, \
                                    \"cpuTime\" uint16 NOT NULL, \
                                    \"residentSize\" uint64 NOT NULL, \
                                    \"totalSize\" uint64 NOT NULL, \
                                    \"insertedRows\" uint32, \
                                    \"crashed\" BOOLEAN NOT NULL"

bool updateProcessStats(PGconn* postgreSQL, std::string tableName,
                        std::string processName, uint32_t percentComplete,
                        uint32_t insertedRows) {
  int pageSize = getpagesize(), numProcesses;
  kvm_t* kvmDescriptor = kvm_open(NULL, "/dev/null", NULL, O_RDONLY,
                                  "kvm_open");
  kinfo_proc* kvmProcessInfo = kvm_getprocs(kvmDescriptor, KERN_PROC_PID,
                                            getpid(), &numProcesses);
  uint32_t startTime = kvmProcessInfo -> ki_start.tv_sec,
           endTime = time(NULL),
           cpuTime = kvmProcessInfo -> ki_runtime / 1000000;
  unsigned int residentSize = (kvmProcessInfo -> ki_rssize * pageSize) / 1024,
               totalSize = (kvmProcessInfo -> ki_size) / 1024;
  std::stringstream query;
  if (percentComplete == 0) {
    return(insertPGRow(postgreSQL, PROCESS_STATS_SCHEMA_NAME, tableName,
                       "%s, %ud, %ud, %ud, %ud, %ud, %ud, %ud, %s",
                       processName.c_str(), percentComplete, startTime, endTime,
                       cpuTime, residentSize, totalSize, insertedRows, "f"));
  }
  else {
    query << "UPDATE \"" << PROCESS_STATS_SCHEMA_NAME << "\".\"" << tableName
          << "\" SET \"percentComplete\" = '" << percentComplete
          << "', \"endTime\" = '" << endTime
          << "', \"cpuTime\" = '" << cpuTime
          << "', \"residentSize\" = '" << residentSize
          << "', \"totalSize\" = '" << totalSize
          << "', \"insertedRows\" = '" << insertedRows
          << "' WHERE \"processName\" = '" << processName << '\'';
  }
  return (PQresultStatus(PQexec(postgreSQL,
                                query.str().c_str())) == PGRES_COMMAND_OK);
}
