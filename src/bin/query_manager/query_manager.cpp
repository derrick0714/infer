#include <iostream>
#include <string>
#include <sstream>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>
#include <queue>
#include <sys/un.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <assert.h>

#include "configuration.hpp"
#include "socketHelpers.hpp"
#include "nameResolutionManager.h"
#include "dnsNameResolutionSource.h"
#include "queryManagerConfiguration.h"
#include "StrftimeReadEnumerator.hpp"

using namespace std;
using namespace tr1;

using namespace ims;
using namespace name;

const char RESOLUTION_TYPE = 0;

NameResolutionManager nameResMan;

// configuration parameters
QueryManagerConfiguration queryManConfig;

// signal handler thread
pthread_t sigThread;
// source updater thread
pthread_t dnsUpdateThread;
// listening thread
pthread_t accepterThread;
// query processor threads
unordered_set <pthread_t> queryProcessorThreads;

// used by sigHandler and dnsUpdater
queue <pair<TimeStamp, TimeStamp> > dnsUpdates;
pthread_mutex_t dnsUpdatesLock;
pthread_cond_t dnsUpdatesCondition;
pthread_cond_t dnsLoadedCondition;

bool running = true;
pthread_mutex_t runningLock;

// for the query processor threads...
struct NameResolutionResponseBuffs {
	IPSet ips;
	NameSet names;
	NameIPMap mappings;
};

pair <vector<string>, time_t> mkFilename(const string &type,
										 const time_t &t,
										 const time_t &interval)
{
	ostringstream filename;
	vector <string> filenames;

	time_t beginInterval = (t / interval) * interval;
	time_t beginDay;
	tm _tm;

	char date[12];
	
	localtime_r(&beginInterval, &_tm);
	_tm.tm_sec = 0;
	_tm.tm_min = 0;
	_tm.tm_hour = 0;
	beginDay = mktime(&_tm);

	strftime(date, 12, "%Y/%m/%d", &_tm);
	size_t curFile = (beginInterval - beginDay) / interval;
	size_t width = ((size_t) log10(86400 / interval)) + 1;
	size_t padding = (curFile != 0)?width - ((size_t) log10(curFile) + 1)
								   :width - 1;

	for (vector <string>::iterator it(queryManConfig.dbHomes.begin());
		 it != queryManConfig.dbHomes.end();
		 ++it)
	{
		filename << *it << '/' << date << '/' << type << '_';
		for (size_t i = 0; i < padding; ++i) {
			filename << '0';
		}
		filename << curFile;

		filenames.push_back(filename.str());

		filename.clear();
		filename.str("");
	}
	
	return make_pair(filenames, beginInterval);
}

void * sigHandler(void *) {
	sigset_t supportedSignals;
	int sig;

	while (1) {
		sigemptyset(&supportedSignals);
		sigaddset(&supportedSignals, SIGINT);
		sigaddset(&supportedSignals, SIGTERM);
		sigaddset(&supportedSignals, SIGALRM);

		sigwait(&supportedSignals, &sig);

		switch (sig) {
		  case SIGINT:
		  case SIGTERM:
			pthread_mutex_lock(&runningLock);
			running = false;
			pthread_mutex_unlock(&runningLock);
			pthread_mutex_lock(&dnsUpdatesLock);
			pthread_cond_signal(&dnsUpdatesCondition);
			pthread_mutex_unlock(&dnsUpdatesLock);
			pthread_kill(accepterThread, SIGUSR1);
			return NULL;
		  default:
			break;
		}
	}
}

void * dnsUpdater(void *) {
	TimeStamp oldBegin, oldEnd, begin, end;
	StrftimeReadEnumerator enumerator;

	pthread_mutex_lock(&runningLock);
	while (running) {
		pthread_mutex_unlock(&runningLock);
		pthread_mutex_lock(&dnsUpdatesLock);
		while (dnsUpdates.empty()) {
			pthread_cond_wait(&dnsUpdatesCondition, &dnsUpdatesLock);
			pthread_mutex_lock(&runningLock);
			if (!running) {
				pthread_mutex_unlock(&runningLock);
				while (!dnsUpdates.empty()) {
					dnsUpdates.pop();
				}
				pthread_mutex_unlock(&dnsUpdatesLock);
				return NULL;
			}
			pthread_mutex_unlock(&runningLock);
		}

		begin = dnsUpdates.front().first;
		end = dnsUpdates.front().second;
		if (begin != oldBegin || end != oldEnd) {
			nameResMan.clear();
			oldBegin = begin;
			oldEnd = end;
		}

		pthread_mutex_unlock(&dnsUpdatesLock);

		// load all the queryManConfig.dbInterval's for the previous queryManConfig.updateInterval
		
		enumerator.init(queryManConfig.dbHomes.front(),
						"%Y/%m/%d/dns_%H",
						begin,
						end);

		pthread_mutex_lock(&runningLock);
		for (StrftimeReadEnumerator::iterator it(enumerator.begin()); 
			 it != enumerator.end() && running;
			 ++it)
		{
			pthread_mutex_unlock(&runningLock);

			if (!nameResMan.updateMappings(*it, NameResolutionSourceType::DNS)) {
				cerr << "Warning: Unable to load mappings from " << *it << endl;
			} else {
				cerr << "Successfully loaded mappings from " << *it << endl;
			}

			pthread_mutex_lock(&runningLock);
		}
		pthread_mutex_unlock(&runningLock);
		pthread_mutex_lock(&dnsUpdatesLock);
		dnsUpdates.pop();
		if (dnsUpdates.empty()) {
			pthread_cond_broadcast(&dnsLoadedCondition);
		}
		pthread_mutex_unlock(&dnsUpdatesLock);
		pthread_mutex_lock(&runningLock);
	}
	pthread_mutex_unlock(&runningLock);

	// if execution reaches this point, we've received SIGINT or SIGTERM
	// don't bother loading any more mappings...

	return NULL;
}

int processNameResCmd(int fd,
					  char *buf,
					  size_t len,
					  NameResolutionResponseBuffs &nameResBuffs)
{
	NameResolutionSourceType stype;
	unsigned char ch;
	uint16_t count;
	uint32_t count32;
	ssize_t ret;
	uint32_t tmpIP;
	TimeStamp t;

	if (len < 2) {
		//wtF?
		cerr << "len error" << endl;
		return -1;
	}

	switch (buf[1]) {
	  case 0: // ANY
		stype = NameResolutionSourceType::ANY;
		break;
	  case 1: // DNS
		stype = NameResolutionSourceType::DNS;
		break;
	  default:
		// return error
		cerr << "type error" << endl;
		return -1;
	};

	TimeStamp begin, end;
	switch (buf[0]) {
	  case 0: //getIPsFromName
		//cerr << "Case 0" << endl;
		if (len != (3 + (unsigned char) buf[2])) {
			// return error
			return -1;
		}
		nameResBuffs.ips.clear();
		nameResMan.getIPsFromName(nameResBuffs.ips,
								  string(buf + 3,
								  		 (size_t) (unsigned char) buf[2]),
								  stype);
		// write the number of ips...
		count = nameResBuffs.ips.size();
		count = htons(count);
		if ((ret = socket_write(fd,
							    (char *) &count,
								2,
								queryManConfig.sockTimeout)) == -1)
		{
			// Error
			cerr << "socket_write() returned -1." << endl;
			return -1;
		} else if (ret != 2) {
			cerr << "socket_write() timeout." << endl;
			return -1;
		}
		// write the ips
		for (IPSet::iterator it = nameResBuffs.ips.begin();
			 it != nameResBuffs.ips.end();
			 ++it)
		{
			tmpIP = htonl(*it);
			if ((ret = socket_write(fd, (char *) &tmpIP, 4, queryManConfig.sockTimeout)) == -1) {
				// Error
				cerr << "socket_write() returned -1." << endl;
				return -1;
			} else if (ret != 4) {
				cerr << "socket_write() timeout." << endl;
				return -1;
			}
		}
		break;
	  case 1: //getNamesFromIP
		//cerr << "Case 1" << endl;
		if (len != 6) {
			// return error
			cerr << "len: " << len << endl;
			return -1;
		}
		nameResBuffs.names.clear();
		nameResMan.getNamesFromIP(nameResBuffs.names, ntohl(*(uint32_t *) (buf + 2)), stype);
		// write the number of names
		count = nameResBuffs.names.size();
		count = htons(count);
		// write the number of names
		if ((ret = socket_write(fd, (char *) &count, 2, queryManConfig.sockTimeout)) == -1) {
			// Error
			cerr << "socket_write() returned -1." << endl;
			return -1;
		} else if (ret != 2) {
			cerr << "socket_write() timeout." << endl;
			return -1;
		}
		// write the names
		for (NameSet::iterator it = nameResBuffs.names.begin();
			 it != nameResBuffs.names.end();
			 ++it)
		{
			// write the name length
			ch = it -> length();
			if ((ret = socket_write(fd, (char *) &ch, 1, queryManConfig.sockTimeout)) == -1) {
				// Error
				cerr << "socket_write() returned -1." << endl;
				return -1;
			} else if (ret != 1) {
				cerr << "socket_write() timeout." << endl;
				return -1;
			}
			// write the name
			if ((ret = socket_write(fd, it -> c_str(), it -> length(), queryManConfig.sockTimeout)) == -1) {
				// Error
				cerr << "socket_write() returned -1." << endl;
				return -1;
			} else if (ret != it -> length()) {
				cerr << "socket_write() timeout." << endl;
				return -1;
			}
		}
		//socket_write(fd, "Hello", 5, queryManConfig.sockTimeout);
		break;
	  case 2: //hasMappingName
		//cerr << "Case 2" << endl;
		if (len != (3 + (unsigned char) buf[2])) {
			// return error
			return -1;
		}
		ch = (nameResMan.hasMapping(string(buf + 3, (size_t) (unsigned char) buf[2]), stype))?1:0;
		if ((ret = socket_write(fd, (char *) &ch, 1, queryManConfig.sockTimeout)) == -1) {
			// Error
			cerr << "socket_write() returned -1." << endl;
			return -1;
		} else if (ret != 1) {
			cerr << "socket_write() timeout." << endl;
			return -1;
		}
		break;
	  case 3: //hasMappingIP
		//cerr << "Case 3" << endl;
		if (len != 6) {
			// return error
			return -1;
		}
		ch = (nameResMan.hasMapping(ntohl(*(uint32_t *) (buf + 2)), stype))?1:0;
		if ((ret = socket_write(fd, (char *) &ch, 1, queryManConfig.sockTimeout)) == -1) {
			// Error
			cerr << "socket_write() returned -1." << endl;
			return -1;
		} else if (ret != 1) {
			cerr << "socket_write() timeout." << endl;
			return -1;
		}
		break;
	  case 4: //getMappings
		//cerr << "Case 4" << endl;
		if (len != 2) {
			// return error
			return -1;
		}
		nameResMan.getMappings(nameResBuffs.mappings, stype);
		// write the number of names
		assert(nameResBuffs.mappings.size() <= 0xffffffff);
		count32 = nameResBuffs.mappings.size();
		count32 = htonl(count32);
		if ((ret = socket_write(fd, (char *) &count32, 4, queryManConfig.sockTimeout)) == -1) {
			// Error
			cerr << "socket_write() returned -1." << endl;
			return -1;
		} else if (ret != 4) {
			cerr << "socket_write() timeout." << endl;
			return -1;
		}
		// write the mappings
		for (NameIPMap::iterator it = nameResBuffs.mappings.begin();
			 it != nameResBuffs.mappings.end();
			 ++it)
		{
			// write the name length
			ch = it -> first.length();
			if ((ret = socket_write(fd, (char *) &ch, 1, queryManConfig.sockTimeout)) == -1) {
				// Error
				cerr << "socket_write() returned -1." << endl;
				return -1;
			} else if (ret != 1) {
				cerr << "socket_write() timeout." << endl;
				return -1;
			}
			// write the name
			if ((ret = socket_write(fd, it -> first.c_str(), it -> first.length(), queryManConfig.sockTimeout)) == -1) {
				// Error
				cerr << "socket_write() returned -1." << endl;
				return -1;
			} else if (ret != it -> first.length()) {
				cerr << "socket_write() timeout." << endl;
				return -1;
			}
			// write the ip
			tmpIP = htonl(it -> second);
			if ((ret = socket_write(fd, (char *) &tmpIP, 4, queryManConfig.sockTimeout)) == -1) {
				// Error
				cerr << "socket_write() returned -1." << endl;
				return -1;
			} else if (ret != 4) {
				cerr << "socket_write() timeout." << endl;
				return -1;
			}
		}
		break;
	  case 5: // getResolvedIPs
		if (len != 2) {
			return -1;
		}
		nameResBuffs.ips.clear();
		nameResMan.getResolvedIPs(nameResBuffs.ips, stype);
		// write the number of ips...
		assert(nameResBuffs.ips.size() <= 0xffffffff);
		count32 = nameResBuffs.ips.size();
		count32 = htonl(count32);
		if ((ret = socket_write(fd, (char *) &count32, sizeof(count32), queryManConfig.sockTimeout)) == -1) {
			// Error
			cerr << "socket_write() returned -1." << endl;
			return -1;
		} else if (ret != sizeof(count32)) {
			cerr << "socket_write() timeout." << endl;
			return -1;
		}
		// write the ips
		for (IPSet::iterator it = nameResBuffs.ips.begin();
			 it != nameResBuffs.ips.end();
			 ++it)
		{
			tmpIP = htonl(*it);
			if ((ret = socket_write(fd, (char *) &tmpIP, 4, queryManConfig.sockTimeout)) == -1) {
				// Error
				cerr << "socket_write() returned -1." << endl;
				return -1;
			} else if (ret != 4) {
				cerr << "socket_write() timeout." << endl;
				return -1;
			}
		}
		break;
	  case 6: // hasHostResolvedIP
		if (len != 2 + 4 + 4 + 8) {
			return -1;
		}
		ch = (nameResMan.hasHostResolvedIP(ntohl(*(uint32_t *) (buf + 2)), ntohl(*(uint32_t *) (buf + 6)), TimeStamp(ntohl(*(uint32_t *) (buf + 10)), ntohl(*(uint32_t *) (buf + 14))), stype))?1:0;
		if ((ret = socket_write(fd, (char *) &ch, 1, queryManConfig.sockTimeout)) == -1) {
			// Error
			cerr << "socket_write() returned -1." << endl;
			return -1;
		} else if (ret != 1) {
			cerr << "socket_write() timeout." << endl;
			return -1;
		}
		break;
	  case 7: // getLastResolutionTime
		if (len != 2 + 4 + 4 + 8) {
			return -1;
		}
		t = nameResMan.getLastResolutionTime(ntohl(*(uint32_t *) (buf + 2)), ntohl(*(uint32_t *) (buf + 6)), TimeStamp(ntohl(*(uint32_t *) (buf + 10)), ntohl(*(uint32_t *) (buf + 14))), stype);
		t.set(ntohl(t.seconds()), ntohl(t.microseconds()));
		if ((ret = socket_write(fd, (char *) &t, sizeof(t), queryManConfig.sockTimeout)) == -1) {
			// Error
			cerr << "socket_write() returned -1." << endl;
			return -1;
		} else if (ret != sizeof(t)) {
			cerr << "socket_write() timeout." << endl;
			return -1;
		}
		break;
	  case -1: // clear
		if (len != 2) {
			// return error
			return -1;
		}
		nameResMan.clear(stype);
		ch = 0;
		if ((ret = socket_write(fd, (char *) &ch, 1, queryManConfig.sockTimeout)) == -1) {
			// Error
			cerr << "socket_write() returned -1." << endl;
			return -1;
		} else if (ret != 1) {
			cerr << "socket_write() timeout." << endl;
			return -1;
		}
		break;
	  case -2: // load
		// load specific name resolution data
		if (len != 18) {
			// return error
			return -1;
		}
		begin.set(ntohl(*reinterpret_cast<uint32_t*>(buf + 2)), ntohl(*reinterpret_cast<uint32_t*>(buf + 6)));
		end.set(ntohl(*reinterpret_cast<uint32_t*>(buf + 10)), ntohl(*reinterpret_cast<uint32_t*>(buf + 14)));

		pthread_mutex_lock(&dnsUpdatesLock);
		dnsUpdates.push(make_pair(begin, end));
		pthread_cond_signal(&dnsUpdatesCondition);
		pthread_mutex_unlock(&dnsUpdatesLock);
		
		ch = 0;
		if ((ret = socket_write(fd, (char *) &ch, 1, queryManConfig.sockTimeout)) == -1) {
			// Error
			cerr << "socket_write() returned -1." << endl;
			return -1;
		} else if (ret != 1) {
			cerr << "socket_write() timeout." << endl;
			return -1;
		}
		break;
	  case -3: // waitForData
		// wait for all data to be loaded.
		if (len != 2) {
			// return error
			return -1;
		}

		timespec ts;
		ts.tv_sec = time(NULL) + static_cast <time_t>(queryManConfig.sockTimeout / 2); // heartbeat every queryManConfig.sockTimeout / 2
		ts.tv_nsec = 0;

		pthread_mutex_lock(&dnsUpdatesLock);
		while (!dnsUpdates.empty()) {
			if (pthread_cond_timedwait(&dnsLoadedCondition, &dnsUpdatesLock, &ts) == ETIMEDOUT) {
				ch = 0;
				if ((ret = socket_write(fd, (char *) &ch, 1, queryManConfig.sockTimeout)) == -1) {
					// Error
					cerr << "socket_write() returned -1." << endl;
					return -1;
				} else if (ret != 1) {
					cerr << "socket_write() timeout." << endl;
					return -1;
				}
				ts.tv_sec = time(NULL) + static_cast <time_t>(queryManConfig.sockTimeout / 2); // heartbeat every queryManConfig.sockTimeout / 2
			}
		}
		pthread_mutex_unlock(&dnsUpdatesLock);

		ch = 1;
		if ((ret = socket_write(fd, (char *) &ch, 1, queryManConfig.sockTimeout)) == -1) {
			// Error
			cerr << "socket_write() returned -1." << endl;
			return -1;
		} else if (ret != 1) {
			cerr << "socket_write() timeout." << endl;
			return -1;
		}
		break;
	  case -4: // getConfig
		if (len != 2) {
			// return error
			return -1;
		}

		// write number of dbHomes...
		assert(queryManConfig.dbHomes.size() <= 0xffffffff);
		count32 = queryManConfig.dbHomes.size();
		count32 = htonl(count32);
		if ((ret = socket_write(fd, (char *) &count32, sizeof(count32), queryManConfig.sockTimeout)) == -1) {
			// Error
			cerr << "socket_write() returned -1." << endl;
			return -1;
		} else if (ret != sizeof(count32)) {
			cerr << "socket_write() timeout." << endl;
			return -1;
		}
		// for each dbHome...
		for (vector <string>::iterator it = queryManConfig.dbHomes.begin(); it != queryManConfig.dbHomes.end(); ++it) {
			// write dbHome.size()
			assert(it -> size() <= 0xffffffff);
			count32 = it -> size();
			count32 = htonl(count32);
			if ((ret = socket_write(fd, (char *) &count32, sizeof(count32), queryManConfig.sockTimeout)) == -1) {
				// Error
				cerr << "socket_write() returned -1." << endl;
				return -1;
			} else if (ret != sizeof(count32)) {
				cerr << "socket_write() timeout." << endl;
				return -1;
			}
			// write dbHome
			if ((ret = socket_write(fd, it -> c_str(), it -> size(), queryManConfig.sockTimeout)) == -1) {
				// Error
				cerr << "socket_write() returned -1." << endl;
				return -1;
			} else if (ret != it -> size()) {
				cerr << "socket_write() timeout." << endl;
				return -1;
			}
		}

		// write queryManConfig.sockPath.size()
		assert(queryManConfig.sockPath.size() <= 0xffffffff);
		count32 = queryManConfig.sockPath.size();
		count32 = htonl(count32);
		if ((ret = socket_write(fd, (char *) &count32, sizeof(count32), queryManConfig.sockTimeout)) == -1) {
			// Error
			cerr << "socket_write() returned -1." << endl;
			return -1;
		} else if (ret != sizeof(count32)) {
			cerr << "socket_write() timeout." << endl;
			return -1;
		}
		// write queryManConfig.sockPath
		if ((ret = socket_write(fd, queryManConfig.sockPath.c_str(), queryManConfig.sockPath.size(), queryManConfig.sockTimeout)) == -1) {
			// Error
			cerr << "socket_write() returned -1." << endl;
			return -1;
		} else if (ret != queryManConfig.sockPath.size()) {
			cerr << "socket_write() timeout." << endl;
			return -1;
		}

		// write queryManConfig.sockTimeout
		tmpIP = static_cast <uint32_t> (queryManConfig.sockTimeout);
		tmpIP = htonl(tmpIP);
		if ((ret = socket_write(fd, (char *) &tmpIP, sizeof(tmpIP), queryManConfig.sockTimeout)) == -1) {
			// Error
			cerr << "socket_write() returned -1." << endl;
			return -1;
		} else if (ret != sizeof(tmpIP)) {
			cerr << "socket_write() timeout." << endl;
			return -1;
		}

		break;
	  default:
		// unknown command...return error
		cerr << "processNameResCmd(): Unknown command!" << endl;
		return -1;
	};

	return 0;
}

// conn is a connected socket. this thread processes queries from this socket...
// NOTE: what happens when this thread exits? It's not joined till SIGINT or SIGTERM
// is received, and it could lead to reaching MAX_THREADS...do I need a separate thread
// whose job it is to join finished queryProcessor threads? -probably
void * queryProcessor(void *conn) {
	char cmdBuf[0xffff];
	uint16_t cmdLen;
	int conn_fd = *((int *) conn);
	delete (int *) conn;
	//char cmdType;

	int flags = fcntl(conn_fd, F_GETFL, 0);
	fcntl(conn_fd, F_SETFL, flags | O_NONBLOCK);

	NameResolutionResponseBuffs nameResBuffs;
	
	ssize_t ret;

	pthread_mutex_lock(&runningLock);
	while (running) {
		pthread_mutex_unlock(&runningLock);
		if ((ret = socket_read(conn_fd, (char *) &cmdLen, 2, queryManConfig.sockTimeout)) == -1) {
			// CONNRESET
			cerr << "Connection was reset...thread exiting." << endl;
			close(conn_fd);
			return NULL;
		} else if (ret != 2) {
			// Error reading type from socket
			cerr << "Socket read timeout getting length..." << endl;
			close(conn_fd);
			return NULL;
		}
		cmdLen = ntohs(cmdLen);

		//cerr << "Successfully read command length: " << cmdLen << endl;

		// read rest of command...
		if ((ret = socket_read(conn_fd, cmdBuf, cmdLen, queryManConfig.sockTimeout)) != cmdLen) {
			// Error reading type from socket
			cerr << "Socket read timeout getting command..." << endl;
			close(conn_fd);
			return NULL;
		}
		//cerr << "Successfully read command." << endl;

		// based on type, send to appropriate command processor
		switch (cmdBuf[0]) {
		  case RESOLUTION_TYPE:
			if (processNameResCmd(conn_fd, cmdBuf + 1, cmdLen - 1, nameResBuffs) < 0) {
				cerr << "processNameResCmd() error..." << endl;
				close(conn_fd);
				return NULL;
			}
			break;
		  default:
			// unknown command type...close the connection
			close(conn_fd);
			return NULL;
		};

		pthread_mutex_lock(&runningLock);
	}
	pthread_mutex_unlock(&runningLock);
	
	close(conn_fd);

	return NULL;
}

void dummyHandler(int) {
	//cerr << "dummyHandler() called..." << endl;
}

// this is the thread that waits for connections, then spawns queryProcessor threads
void * connectionAccepter(void *) {
	int listen_fd;
	int *conn_fd;
	sockaddr_un listen_addr;
	pthread_t nextThread;

	listen_fd = socket(PF_LOCAL, SOCK_STREAM, 0);
	if (listen_fd == -1) {
		// Error creating socket
		perror("socket() error");
		exit(1);
	}

	listen_addr.sun_family = AF_LOCAL;
	strcpy(listen_addr.sun_path, queryManConfig.sockPath.c_str());
	unlink(listen_addr.sun_path);

	int len = sizeof(listen_addr.sun_len) + sizeof(listen_addr.sun_family) + strlen(listen_addr.sun_path); 

	if (bind(listen_fd, (sockaddr *) &listen_addr, len) != 0) {
		// Error binding socket
		perror("bind() error");
		exit(1);
	}

	if (listen(listen_fd, 1) != 0) {
		// Error listening on socket
		perror("listen() error");
		exit(1);
	}

	// set up signal handler for SIGUSR1...this thread will recieve this signal
	// from the signal handler thread
	sigset_t usr1sig;
	sigemptyset(&usr1sig);
	sigaddset(&usr1sig, SIGUSR1);
	pthread_sigmask(SIG_UNBLOCK, &usr1sig, NULL);

	struct sigaction sigact;
	sigact.sa_handler = dummyHandler;
	sigact.sa_flags = 0;
	sigemptyset(&sigact.sa_mask);
	sigaction(SIGUSR1, &sigact, NULL);

	conn_fd = new int;
	//cerr << "Accepter thread: entering main loop" << endl;
	pthread_mutex_lock(&runningLock);
	while (running) {
		pthread_mutex_unlock(&runningLock);
		//cerr << "Accepter thread: waiting for a connection" << endl;
		*conn_fd = accept(listen_fd, (struct sockaddr *) NULL, (socklen_t *) NULL);
		//cerr << "Accepter thread: accept() returned..." << endl;
		if (*conn_fd == -1) {
			if (errno == EINTR) {
				// accept returned early due to signal.
				pthread_mutex_lock(&runningLock);
				continue;
			}
			// Error
			perror("accept() error");
			exit(1);
		}
	
		// spawn queryProcessor on this connection...
		//pthread_sigmask(SIG_BLOCK, &usr1sig, NULL);
		// processor threads shouldn't block SIGUSR1, either...
		pthread_create(&nextThread, NULL, queryProcessor, (void *) conn_fd);
		//pthread_sigmask(SIG_UNBLOCK, &usr1sig, NULL);
		queryProcessorThreads.insert(nextThread);

		conn_fd = new int;

		pthread_mutex_lock(&runningLock);
	}
	pthread_mutex_unlock(&runningLock);
	
	// terminating...clean up.
	// kill all query processor threads
	// join all query processor threads
	for (unordered_set <pthread_t>::iterator it = queryProcessorThreads.begin();
		 it != queryProcessorThreads.end();
		 ++it)
	{
		pthread_kill(*it, SIGUSR1);
		pthread_join(*it, NULL);
	}
	close(listen_fd);
	unlink(listen_addr.sun_path);

	return NULL;
}

int main(int, char **argv) {
	configuration conf;
	if (!conf.load("/usr/local/etc/infer.conf")) {
		cerr << argv[0] << ": unable to load configuration" << endl;
		return 1;
	}
	
	if (conf.get(queryManConfig.dbHomes,
				 "data-directory",
				 "query_manager",
				 true)
			!= configuration::OK)
	{
		cerr << argv[0] << ": data-directory required" << endl;
		return 1;
	}

	if (conf.get(queryManConfig.sockPath,
				 "socket-path",
				 "query_manager")
			!= configuration::OK)
	{
		cerr << argv[0] << ": socket-path required" << endl;
		return 1;
	}

	if (conf.get(queryManConfig.sockTimeout,
				 "socket-timeout",
				 "query_manager")
			!= configuration::OK)
	{
		cerr << argv[0] << ": socket-timeout required" << endl;
		return 1;
	}
	
	// signal set
	sigset_t sigSet;

	// create the dns source
	nameResMan.addSource(new DNSNameResolutionSource());

	// block all signals in preparation for the signal handler thread
	sigfillset(&sigSet);
	pthread_sigmask(SIG_BLOCK, &sigSet, NULL);

	// initialize mutexes
	pthread_mutex_init(&dnsUpdatesLock, NULL);
	pthread_mutex_init(&runningLock, NULL);
	pthread_cond_init(&dnsUpdatesCondition, NULL);
	pthread_cond_init(&dnsLoadedCondition, NULL);

	// start the signal handler and dns source updater threads
	pthread_create(&sigThread, NULL, sigHandler, NULL);
	pthread_create(&dnsUpdateThread, NULL, dnsUpdater, NULL);
	pthread_create(&accepterThread, NULL, connectionAccepter, NULL);

	// join all remaining threads
	pthread_join(sigThread, NULL);
	pthread_join(dnsUpdateThread, NULL);
	pthread_join(accepterThread, NULL);

	return 0;
}
