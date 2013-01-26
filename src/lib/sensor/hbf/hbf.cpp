#include <iostream>
#include <tr1/unordered_map>
#include <openssl/md5.h>

#include "HBF.h"
#include "modules.h"
#include "network.h"
#include "timeStamp.h"
#include "ObjectPool.hpp"
#include "FlatFileWriter.hpp"
#include "InferFileWriter.hpp"
#include "AsynchronousHBFWriter.hpp"

using namespace std;
using namespace tr1;

static char flowID[13];
static const char *payload;
static MD5_CTX md5Context;
static bool maxFlowsWarning = true;
static unsigned char md[MD5_DIGEST_LENGTH];
static ObjectPool <HBF> *hbfPool;
static HBF *flowPtr;
/* The flow table--a hash table of pointers to HBF structures. */
static unordered_map <string, HBF*> flowTable;
static pthread_mutex_t *flowTableLocks, flushLock;
static size_t maxFlows;
/* Why not use a static allocation for this? */
static uint32_t idleTimeout;
static AsynchronousHBFWriter
		<
			InferFileWriter<FlatFileWriter<ZlibCompressedHBF> >,
			HBF*
		> *writer;

static void makeFlowID(char *flowID,
					   const uint8_t &protocol,
					   const uint32_t &sourceIP,
					   const uint32_t &destinationIP,
					   const uint16_t &sourcePort,
					   const uint16_t &destinationPort)
{
	memcpy(flowID, &protocol, 1);
	memcpy(flowID + 1, &sourceIP, 4);
	memcpy(flowID + 5, &destinationIP, 4);
	memcpy(flowID + 9, &sourcePort, 2);
	memcpy(flowID + 11, &destinationPort, 2);
}

static void initializeFlow(HBF *flow, Packet &packet) {
	flow->protocol(static_cast<uint8_t>(flowID[0]));
	flow->rawSourceIP(*reinterpret_cast<uint32_t*>(flowID+1));
	flow->rawDestinationIP(*reinterpret_cast<uint32_t*>(flowID+5));
	flow->rawSourcePort(*reinterpret_cast<uint16_t*>(flowID+9));
	flow->rawDestinationPort(*reinterpret_cast<uint16_t*>(flowID+11));
	flow -> startTime(packet.time());
}

extern "C" {

	int initialize(const configuration &conf,
				   const std::string &outputDirectory,
				   const std::string &)
	{
		if (conf.get(maxFlows, "max-flows", "sensor_hbf")
				!= configuration::OK)
		{
			cerr << "sensor_hbf: missing or invalid max-flows" << endl;
			return 1;
		}

		if (conf.get(idleTimeout, "idle-timeout", "sensor_hbf")
				!= configuration::OK)
		{
			cerr << "sensor_hbf: missing or invalid idle-timeout" << endl;
			return 1;
		}

		hbfPool = new ObjectPool <HBF>(maxFlows);
		flowTable.rehash(maxFlows);
		boost::shared_ptr<StrftimeWriteEnumerator<ZlibCompressedHBF> >
				enumerator(new StrftimeWriteEnumerator<ZlibCompressedHBF>(
						outputDirectory, "%Y/%m/%d/hbf_%H"));
		
		boost::shared_ptr<InferFileWriter<FlatFileWriter<ZlibCompressedHBF> > >
			inferWriter(new InferFileWriter
							<FlatFileWriter<ZlibCompressedHBF> >(enumerator));
		writer = new AsynchronousHBFWriter
						<
							InferFileWriter<FlatFileWriter<ZlibCompressedHBF> >,
						 	HBF*
						> (inferWriter, *hbfPool);

		flowTableLocks = new pthread_mutex_t[flowTable.bucket_count()];
		for (size_t bucket = 0; bucket < flowTable.bucket_count(); ++bucket) {
			pthread_mutex_init(&(flowTableLocks[bucket]), NULL);
		}
		return 0;
	}

	int processPacket(Packet &packet) {
		static size_t bucket;
		static uint16_t currentBlockSize;
		static uint16_t blockNumber;
		static uint16_t maxInsertions(HBF::HBFSize / 5);
		static unordered_map <string, HBF*>::iterator flowItr;
		makeFlowID(flowID, packet.protocol(), packet.sourceIP(),
							 packet.destinationIP(), packet.sourcePort(),
							 packet.destinationPort());
		bucket = flowTable.bucket(flowID);
		/* Prevents interference with flush(). */
		pthread_mutex_lock(&(flowTableLocks[bucket]));
		flowItr = flowTable.find(flowID);
		if (flowItr == flowTable.end()) {
			if ((flowPtr = hbfPool->allocate()) == NULL) {
				if (maxFlowsWarning) {
					cout << "hbf: flow table is full" << endl;
					maxFlowsWarning = false;
				}
				pthread_mutex_unlock(&(flowTableLocks[bucket]));
				return 0;
			}
			initializeFlow(flowPtr, packet);
			flowItr = flowTable.insert(
							pair<string, HBF*>(flowID, flowPtr)).first;
		}

		flowItr -> second -> endTime(packet.time());
		if (packet.payloadSize() > flowItr -> second -> maxPayload()) {
			flowItr -> second -> maxPayload(packet.payloadSize());
		}
		currentBlockSize = HBF::BlockSize;
		payload = packet.payload();
		while (currentBlockSize <= packet.payloadSize()) {
			blockNumber = 0;
			while (currentBlockSize * (blockNumber + 1)
						<= packet.payloadSize())
			{
				/* Initialize MD5 context. */
				MD5_Init(&md5Context);
				/* Hash block. */
				MD5_Update(&md5Context,
						   payload + (blockNumber * currentBlockSize),
						   currentBlockSize);
				/* Hash block number. */
				MD5_Update(&md5Context, &blockNumber, sizeof(uint16_t));
				/* Finalize hash for block and block number. */
				MD5_Final(&(md[0]), &md5Context);
				for (size_t hashNumber = 0;
					 hashNumber < HBF::NumHashes;
					 ++hashNumber)
				{
					flowItr->second->setBit(*(uint32_t*)
						(md + (hashNumber * sizeof(uint32_t))) % HBF::HBFSize);
				}
				++blockNumber;
			}
			currentBlockSize *= 2;
		}
		/*
		 * We delete a flow if we see an RST or FIN TCP flag, or if we have 
		 * inserted maxInsertions elements into its hierarchical bloom filter.
		 */
		if ((packet.protocol() == IPPROTO_TCP && (packet.tcpFlags() & TH_RST ||
				 packet.tcpFlags() & TH_FIN)) ||
				flowItr->second->numInsertions() >= maxInsertions) {
			writer->write(flowItr->second);
			flowTable.erase(flowItr);
		}
		pthread_mutex_unlock(&(flowTableLocks[bucket]));
		return 0;
	}

	int flush() {
		static time_t _time;
		static size_t bucket, index;
		static unordered_map <string, HBF*>::local_iterator localItr;
		static vector <string> eraseList;
		_time = time(NULL);
		/* Debug output. */
		cout << "hbf: flush() called (flowTable: " << flowTable.size() << ')'
				 << endl;
		/*
		 * We'll only warn about the flow table being full a maximum of once in
		 * between flush() calls, in hopes of not cluttering the log.
		 */
		maxFlowsWarning = true;
		/* Prevents interference with finish(). */
		pthread_mutex_lock(&flushLock);
		if (flowTable.size()) {
			for (bucket = 0; bucket < flowTable.bucket_count(); ++bucket) {
				if (eraseList.size() > 0) {
					eraseList.clear();
				}
				/* Prevents interference with processPacket(). */
				pthread_mutex_lock(&(flowTableLocks[bucket]));
				for (localItr = flowTable.begin(bucket);
					 localItr != flowTable.end(bucket);
					 ++localItr)
				{
					if (_time - localItr->second->endTime().seconds() 
							>= idleTimeout)
					{
						writer->write(localItr->second);
						eraseList.push_back(localItr->first);
					}
				}
				for (index = 0; index < eraseList.size(); ++index) {
					flowTable.erase(flowTable.find(eraseList[index]));
				}
				pthread_mutex_unlock(&(flowTableLocks[bucket]));
			}
		}
		//writer.flush();
		pthread_mutex_unlock(&flushLock);
		cout << "hbf: flush() returning..." << endl;
		return 0;
	}

	/* Waits for flush() to finish and writes any remaining flows out. */
	int finish() {
		/* Prevents interference with flush(). */
		pthread_mutex_lock(&flushLock);
		for (unordered_map<string, HBF*>::iterator it(flowTable.begin());
				 it != flowTable.end(); it = flowTable.erase(it)) {
			writer->write(it -> second);
		}
		pthread_mutex_unlock(&flushLock);
		writer->close();

		delete[] flowTableLocks;
		delete writer;
		delete hbfPool;
		return 0;
	}

}
