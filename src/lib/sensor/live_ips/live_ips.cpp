#include <iostream>
#include <string>
#include <tr1/unordered_map>

#include "modules.h"
#include "network.h"
#include "LiveIP.hpp"
#include "FlatFileWriter.hpp"
#include "StrftimeWriteEnumerator.hpp"

using namespace std;
using namespace tr1;

static uint32_t startTime, interval;
static unordered_map <uint32_t, string> *liveIPs(
	new unordered_map <uint32_t, string>);
static unordered_map <uint32_t, string>::iterator liveIPItr;
static pthread_mutex_t liveIPsLock, flushLock;
static string dataDirectory;

// hack to use StrftimeWriteEnumerator for filename generation
class TimeStampTime {
  public:
	TimeStampTime(const TimeStamp &time)
		:_time(time)
	{}

	TimeStamp time() const {
		return _time;
	}

	void time(const TimeStamp &time) {
		_time = time;
	}

  private:
	TimeStampTime() {};

	TimeStamp _time;
};

static StrftimeWriteEnumerator<TimeStampTime> *writeEnum;
FlatFileWriter<LiveIP> writer;

extern "C" {

	int initialize(const configuration &conf,
				   const std::string &outputDirectory,
				   const std::string &)
	{
		if (conf.get(interval, "interval", "sensor_live_ips")
				!= configuration::OK)
		{
			cerr << "sensor_live_ips: missing or invalid interval" << endl;
			return 1;
		}

		if (pthread_mutex_init(&liveIPsLock, NULL) != 0 ||
				pthread_mutex_init(&flushLock, NULL) != 0) {
			abort();
		}
		startTime = time(NULL);
		dataDirectory = outputDirectory;

		writeEnum = new StrftimeWriteEnumerator<TimeStampTime>(outputDirectory,
													"%Y/%m/%d/live_ips_%H");

		return 0;
	}

	int processPacket(const Packet &packet) {
		if (packet.internalSource()) {
			pthread_mutex_lock(&liveIPsLock);
			liveIPItr = liveIPs -> find(packet.sourceIP());
			if (liveIPItr == liveIPs -> end()) {
				liveIPs->insert(make_pair(packet.sourceIP(),
										  string(packet.sourceEthernetAddress(),
										  		 6)));
			}
			pthread_mutex_unlock(&liveIPsLock);
		}
		return 0;
	}

	int writeLiveIPs(unordered_map <uint32_t, string> *writeList,
					 uint32_t &startTime,
					 uint32_t &endTime)
	{
		string record;
		uint32_t numLiveIPs = writeList -> size();
		record.append((const char*)&startTime, sizeof(startTime));
		record.append((const char*)&endTime, sizeof(endTime));
		record.append((const char*)&numLiveIPs, sizeof(numLiveIPs));

		if (writer.open(writeEnum->getFileName(TimeStamp(startTime, 0)))
				!= E_SUCCESS)
		{
				return 1;
		}
		
		LiveIP liveIP;
		boost::array<uint8_t, 6> mac;
		for (unordered_map<uint32_t, string>::iterator
				liveIPItr(writeList->begin());
			 liveIPItr != writeList->end();
			 ++liveIPItr)
		{
			liveIP.rawIP(liveIPItr->first);
			memcpy(&mac, liveIPItr->second.data(), 6);
			liveIP.mac(mac);
			if (writer.write(&liveIP) != E_SUCCESS) {
				writer.close();
				return 1;
			}
		}

		if (writer.close() != E_SUCCESS) {
			return 1;
		}

		return 0;
	}

	int flush() {
		uint32_t currentTime = time(NULL), oldStartTime;
		unordered_map <uint32_t, string> *writeList;
		/* Debug output. */
		cout << "live_ips: flush() called (liveIPs: "
			 << liveIPs -> size()
			 << ')'
			 << endl;
		/* Prevents interference with finish(). */
		pthread_mutex_lock(&flushLock);
		if (currentTime - (currentTime % interval) >= startTime) {
			/* Prevents interference with processPacket(). */
			pthread_mutex_lock(&liveIPsLock);
			oldStartTime = startTime;
			startTime = currentTime;
			writeList = liveIPs;
			liveIPs = new unordered_map <uint32_t, string>;
			pthread_mutex_unlock(&liveIPsLock);
			if (writeLiveIPs(writeList, oldStartTime, currentTime) != 0) {
				delete writeList;
				return 1;
			}
			delete writeList;
		}
		pthread_mutex_unlock(&flushLock); 
		return 0;
	}

	int finish() {
		uint32_t currentTime = time(NULL);
		/* Prevents interference with flush(). */
		pthread_mutex_lock(&flushLock);
		writeLiveIPs(liveIPs, startTime, currentTime);
		pthread_mutex_unlock(&flushLock);

		delete writeEnum;
		return 0;
	}

}
