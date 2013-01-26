#include <iostream>
#include <tr1/unordered_map>
#include <vector>

#include "FlatFileWriter.hpp"
#include "InferFileWriter.hpp"
#include "AsynchronousWriter.hpp"
#include "ObjectPool.hpp"
#include "FlowStats.hpp"
#include "featureset.h"

#include "neoflow.h"

using namespace std;
using namespace tr1;

string dataTypes[9] = { "Plaintext", "BMP image", "WAV audio", "Compressed",
                        "JPEG image", "MP3 audio", "MPEG video", "Encrypted" };

template <typename T>
void debug_dereference(const T *data) {
	char ch;
	for (size_t i(0); i < sizeof(T); ++i) {
		ch = *(reinterpret_cast<const char *>(data) + i);
	}
}

// Config file parameters
static size_t maxFlows;
static uint32_t idleTimeout;
static uint32_t activeTimeout;

/*
 * Byte 1: Layer-4 protocol.
 * Bytes 2-5: source IP.
 * Bytes 6-9: destination IP.
 * Bytes 10-11: source port.
 * Bytes 12-13: destination port.
 */
static char flowID[13];
static bool maxFlowsWarning;
static Flow *flowPtr;
static FlowStats *statsPtr;
//static boost::shared_ptr<FlowPayload> payloadPtr;
static size_t bucket;

static ObjectPool <Flow> *flowPool;
static ObjectPool <FlowStats> *statsPool;
static ObjectPool <FlowPayload> *payloadPool;

typedef PayloadClassifier
			<
				AsynchronousWriter
					<
						InferFileWriter<FlatFileWriter<FlowStats> >,
						FlowStats*,
						ObjectPool<FlowStats>
					>
			> _PayloadClassifier;
static _PayloadClassifier *payloadClassifier;

static unordered_map <string, Flow*> flowTable;
static unordered_map <string, Flow*>::iterator flowIter;
static pthread_mutex_t flushLock;
static pthread_mutex_t *flowTableLocks;

static svm_model *svmModel;

static AsynchronousWriter
		<
			InferFileWriter<FlatFileWriter<FlowStats> >,
			FlowStats*,
			ObjectPool<FlowStats>
		> *writer;
//static InferFileWriter<FlatFileWriter<FlowStats> > *writer;

static void makeFlowID(char *flowID,
					   uint8_t protocol,
					   uint32_t sourceIP,
					   uint32_t destinationIP,
					   uint16_t sourcePort,
					   uint16_t destinationPort)
{
	memcpy(flowID, &protocol, sizeof(protocol));
	memcpy(flowID + 1, &sourceIP, sizeof(sourceIP));
	memcpy(flowID + 5, &destinationIP, sizeof(destinationIP));
	memcpy(flowID + 9, &sourcePort, sizeof(sourcePort));
	memcpy(flowID + 11, &destinationPort, sizeof(destinationPort));
}

static void initializeFlowStats(FlowStats *stats,
								const Packet &packet)
{
	stats->protocol(packet.protocol());
	stats->rawSourceIP(packet.sourceIP());
	stats->rawDestinationIP(packet.destinationIP());
	stats->rawSourcePort(packet.sourcePort());
	stats->rawDestinationPort(packet.destinationPort());
	stats->version(NeoflowVersion);
	stats->typeOfService(packet.typeOfService());
	stats->startTime(packet.time());
	stats->endTime(packet.time());
	/*
	 * The minimum inter-arrival time is initially set to the largest values its
	 * data types can hold, so that only less-than comparisons need to be made
	 * when checking for a minimum.
	 */
	stats->minInterArrivalTime(TimeStamp(numeric_limits <uint32_t>::max(),
										 numeric_limits <uint32_t>::max()));
	stats->minPacketSize(packet.size());
	stats->maxPacketSize(packet.size());
	stats->minTTL(packet.ttl());
	stats->maxTTL(packet.ttl());
};

static void updateInterArrivalTimes(FlowStats *flowStats,
									const TimeStamp &timeStamp)
{
	TimeStamp interArrivalTime = timeStamp - flowStats->endTime();
	if (interArrivalTime < flowStats->minInterArrivalTime()) {
		flowStats->minInterArrivalTime(interArrivalTime);
	}
	if (interArrivalTime > flowStats->maxInterArrivalTime()) {
		flowStats->maxInterArrivalTime(interArrivalTime);
	}
}

extern "C" {

	int initialize(const configuration &conf,
				   const std::string &outputDirectory,
				   const std::string &)
	{
		if (conf.get(maxFlows, "max-flows", "sensor_neoflow")
				!= configuration::OK)
		{
			cerr << "sensor_neoflow: missing or invalid max-flows" << endl;
			return 1;
		}

		if (conf.get(idleTimeout, "idle-timeout", "sensor_neoflow")
				!= configuration::OK)
		{
			cerr << "sensor_neoflow: missing or invalid idle-timeout" << endl;
			return 1;
		}

		if (conf.get(activeTimeout, "active-timeout", "sensor_neoflow")
				!= configuration::OK)
		{
			cerr << "sensor_neoflow: missing or invalid active-timeout" << endl;
			return 1;
		}

		string model_file;
		if (conf.get(model_file, "model-file", "sensor_neoflow")
				!= configuration::OK)
		{
			cerr << "sensor_dns: missing or invalid model-file" << endl;
			return 1;
		}

		svmModel = svm_load_model(model_file.c_str());
		if (svmModel == NULL) {
			cerr << "sensor_neoflow: Error loading svm model." << endl;
			return 1;
		}
		debug_dereference(svmModel);
		

		flowPool = new ObjectPool<Flow>(maxFlows);
		statsPool = new ObjectPool<FlowStats>(maxFlows);
		payloadPool = new ObjectPool<FlowPayload>(maxFlows);

		flowTable.rehash(maxFlows);

		boost::shared_ptr<StrftimeWriteEnumerator<FlowStats> >
			enumerator(new StrftimeWriteEnumerator<FlowStats>(
				outputDirectory, "%Y/%m/%d/neoflow_%H"));

		//writer = new InferFileWriter<FlatFileWriter<FlowStats> >(enumerator);
		boost::shared_ptr<InferFileWriter<FlatFileWriter<FlowStats> > >
			inferWriter(
				new InferFileWriter<FlatFileWriter<FlowStats> >(
					enumerator));
		writer = new AsynchronousWriter
					<
						InferFileWriter<FlatFileWriter<FlowStats> >,
						FlowStats*,
						ObjectPool<FlowStats>
					>(inferWriter, *statsPool);

		payloadClassifier = new _PayloadClassifier(*writer,
												   svmModel,
												   *flowPool,
												   *payloadPool);
		payloadClassifier->start();

		pthread_mutex_init(&flushLock, NULL);
		//
		// As the lock granularity of the flow table is per-bucket, allocates a
		// lock for every bucket.
		//
		flowTableLocks = new pthread_mutex_t[flowTable.bucket_count()];
		for (size_t bucket = 0; bucket < flowTable.bucket_count(); ++bucket) {
			pthread_mutex_init(&(flowTableLocks[bucket]), NULL);
		}

		return 0;
	}

	int processPacket(Packet &packet) {
		makeFlowID(flowID, packet.protocol(),
						   packet.sourceIP(),
						   packet.destinationIP(),
						   packet.sourcePort(),
						   packet.destinationPort());

		bucket = flowTable.bucket(flowID);
		// Locks the flow ID's bucket to prevent interference with flush(). 
		pthread_mutex_lock(&(flowTableLocks[bucket]));
		// Checks whether there is already a flow with this flowID in memory. 
		unordered_map<string, Flow*>::iterator flowItr(flowTable.find(flowID));
		// If not, attempts to allocate memory for a new flow. 
		if (flowItr == flowTable.end()) {
			//
			// If there is no free memory, prints that the flow table is full,
			// disables that warning until after flush() is called next time,
			// and returns.
			//
			if ((statsPtr = statsPool->allocate()) == NULL)
			{
				if (maxFlowsWarning) {
					cout << "neoflow: flow table is full" << endl;
					maxFlowsWarning = false;
				}
				pthread_mutex_unlock(&(flowTableLocks[bucket]));
				return 0;
			}
			if ((flowPtr = flowPool->allocate()) == NULL)
			{
				cerr << "neoflow: ERROR: allocated FlowStats but unable to "
						"allocate Flow!" << endl;
				abort();
			}
			if ((flowPtr->payloadPtr = payloadPool->allocate()) == NULL)
			{
				// this (beoing able to allocate a FlowStats but not a Flow)
				// should NEVER happen
				cerr << "neoflow: ERROR: allocated Flow but unable to "
						"allocate FlowPayload!" << endl;
				abort();
			}

			//
			// If there was free memory, initializes the newly-allocated flow
			// and inserts it into the flow table.
			//
			initializeFlowStats(statsPtr, packet);
			flowPtr->statsPtr = statsPtr;
			flowItr = flowTable.insert(make_pair(flowID, flowPtr)).first;
		}

		// Updates total number of bytes transferred. 
		flowItr->second->statsPtr->numBytes(
			flowItr->second->statsPtr->numBytes() + packet.size());
		if (packet.size() < flowItr->second->statsPtr->minPacketSize()) {
			// Updates minimum packet size. 
			flowItr->second->statsPtr->minPacketSize(packet.size());
		}
		else if (packet.size() > flowItr->second->statsPtr->maxPacketSize()) {
			// Updates maximumpacket size. 
			flowItr->second->statsPtr->maxPacketSize(packet.size());
		}

		// Updates total number of packets. 
		flowItr->second->statsPtr->numPackets(
			flowItr->second->statsPtr->numPackets() + 1);
		// Updates minimum TTL. 
		if (packet.ttl() < flowItr->second->statsPtr->minTTL()) {
			flowItr->second->statsPtr->minTTL(packet.ttl());
		}
		else if (packet.ttl() > flowItr->second->statsPtr->maxTTL()) {
			// Updates maximum TTL. 
			flowItr->second->statsPtr->maxTTL(packet.ttl());
		}
		// Updates packet inter-arrival times. 
		if (flowItr->second->statsPtr->numPackets() > 1) {
			updateInterArrivalTimes(flowItr->second->statsPtr, packet.time());
		}
		// Updates end time. 
		flowItr->second->statsPtr->endTime(packet.time());
		// Updates size distribution. 
		static size_t sizeDistCol;
		sizeDistCol = packet.size() / FlowStats::SizeDistRangeWidth;
		flowItr->second->statsPtr->sizeDistribution(
			sizeDistCol,
			flowItr->second->statsPtr->sizeDistribution(sizeDistCol) + 1);
		// Updates number of fragmented packets. 
		if (packet.fragmented() == true) {
			flowItr->second->statsPtr->numFrags(
				flowItr->second->statsPtr->numFrags() + 1);
		}
		//
		// If the packet is TCP, updates the number of times each TCP flag has
		// been seen.
		//
		if (packet.protocol() == IPPROTO_TCP) {
			flowItr->second->statsPtr->tcpFlags(
				flowItr->second->statsPtr->tcpFlags() | packet.tcpFlags());
			if (packet.tcpFlags() & TH_URG) {
				flowItr->second->statsPtr->tcpURGs(
					flowItr->second->statsPtr->tcpURGs() + 1);
			}
			if (packet.tcpFlags() & TH_ACK) {
				flowItr->second->statsPtr->tcpACKs(
					flowItr->second->statsPtr->tcpACKs() + 1);
			}
			if (packet.tcpFlags() & TH_PUSH) {
				flowItr->second->statsPtr->tcpPUSHs(
					flowItr->second->statsPtr->tcpPUSHs() + 1);
			}
			if (packet.tcpFlags() & TH_RST) {
				flowItr->second->statsPtr->tcpRSTs(
					flowItr->second->statsPtr->tcpRSTs() + 1);
			}
			if (packet.tcpFlags() & TH_SYN) {
				flowItr->second->statsPtr->tcpSYNs(
					flowItr->second->statsPtr->tcpSYNs() + 1);
			}
			if (packet.tcpFlags() & TH_FIN) {
				flowItr->second->statsPtr->tcpFINs(
					flowItr->second->statsPtr->tcpFINs() + 1);
			}
			if (packet.tcpFlags() == TH_SYN && 
				!(flowItr->second->statsPtr->firstSYNTime().seconds()))
			{
				// Records the time of the first part of the TCP handshake. 
				flowItr->second->statsPtr->firstSYNTime(packet.time());
			}
			else if (packet.tcpFlags() == (TH_SYN | TH_ACK) &&
					 !(flowItr->second->statsPtr->firstSYNACKTime().seconds()))
			{
				// Records the time of the second part of the TCP handshake. 
				flowItr->second->statsPtr->firstSYNACKTime(packet.time());
			}
			else if (packet.tcpFlags() == TH_ACK &&
					 !(flowItr->second->statsPtr->firstACKTime().seconds()))
			{
				// Records the time of the third part of the TCP handshake. 
				flowItr->second->statsPtr->firstACKTime(packet.time());
			}
		}
		//
		// If the flow has not accumulated 16 KiB of payload data for
		// classification, appends payload data until exactly 16 KiB have been
		// accumulated.
		//
		if (flowItr->second->payloadPtr->size < FlowPayload::MaxPayload) {
			// Sanity check against corrupted or malicious packets. 
			if (packet.payloadSize() > 0 &&
				packet.payloadSize() < packet.capturedSize())
			{
				flowItr->second->flagsLock.lock();
				if (flowItr->second->classifyingCount == 0) {
					flowItr->second->flagsLock.unlock();
					if (flowItr->second->payloadPtr->size + packet.payloadSize()
							<= FlowPayload::MaxPayload) {
						memcpy(flowItr->second->payloadPtr->data + 
									flowItr->second->payloadPtr->size,
							   packet.payload(),
							   packet.payloadSize());
						flowItr->second->payloadPtr->size +=
							packet.payloadSize();
					}
					else {
						size_t amount(FlowPayload::MaxPayload
										- flowItr->second->payloadPtr->size);
						memcpy(flowItr->second->payloadPtr->data +
									flowItr->second->payloadPtr->size,
							   packet.payload(),
							   amount);
						flowItr->second->payloadPtr->size =
							FlowPayload::MaxPayload;

						// classification happen here
						payloadClassifier->classify(flowItr->second);
					}
				}
				else {
					flowItr->second->flagsLock.unlock();
				}
			}
		}
		//
		// Writes the flow to disk and removes it from the flow table if an
		// RST of FIN TCP flag is seen in a TCP flow--signaling the end of the
		// TCP session -- or if the flow has been active for as long as or
		// longer than the configured active timeout.
		//
		if ((packet.protocol() == IPPROTO_TCP &&
				(packet.tcpFlags() & TH_RST || packet.tcpFlags() & TH_FIN))
			||
			flowItr->second->statsPtr->endTime().seconds() -
					flowItr->second->statsPtr->startTime().seconds() 
				>= activeTimeout)
		{
			flowItr->second->flagsLock.lock();
			if (flowItr->second->classifyingCount > 0) {
				flowItr->second->writable = true;
				flowItr->second->flagsLock.unlock();
			}
			else
			{
				flowItr->second->flagsLock.unlock();
				statsPtr = flowItr->second->statsPtr;
				payloadPool->free(flowItr->second->payloadPtr);
				flowPool->free(flowItr->second);
				if (writer->write(statsPtr) != E_SUCCESS) {
					cerr << "neoflow: Writer error!" << endl;
					// TODO do something more elegant than calling abort()
					abort();
				}
			}
			flowTable.erase(flowItr);
		}

		pthread_mutex_unlock(&(flowTableLocks[bucket]));

		return 0;
	}

	/*
	* Iterates through the flow table, writes any flows that have exceeded the
	* configured idle timeout to disk, and removes them from the flow
	* table.
	*/
	int flush() {
		static time_t _time;
		static unordered_map <string, Flow*>::local_iterator flowItr;
		static vector <string> eraseList;
		_time = time(NULL);
		static FlowStats *flowStatsPtr;
		// Debug output. 
		cout << "neoflow: flush() called (flowTable: " << flowTable.size()
			 << ", classifier: " << payloadClassifier->queueSize()
			 << ", classified payloads: "
			 	<< payloadClassifier->classifiedPayloads()
			 << ", classifier wakeups: " << payloadClassifier->wakeups() << ")"
			 << endl;

		//
		// We'll only warn about the flow table being full a maximum of once in
		// between flush() calls, in hopes of not cluttering the log.
		//
		maxFlowsWarning = true;
		// Ensures that flush() and finish() do not run simultaneously. 
		pthread_mutex_lock(&flushLock);
		if (!flowTable.size()) {
			pthread_mutex_unlock(&flushLock);
			return 0;
		}
		for (size_t bucket(0); bucket < flowTable.bucket_count(); ++bucket) {
			//
			// Locks the bucket whose flows we will be examining, to prevent
			// interference with processPacket().
			//
			pthread_mutex_lock(&(flowTableLocks[bucket]));
			for (flowItr = flowTable.begin(bucket);
				 flowItr != flowTable.end(bucket);
				 ++flowItr)
			{
				//
				// Writes a flow to disk if its idle time is equal to or
				// greater than the configured idle timeout.
				//
				if (_time - flowItr->second->statsPtr->endTime().seconds()
						>= idleTimeout)
				{
					flowItr->second->flagsLock.lock();
					if (flowItr->second->classifyingCount > 0) {
						flowItr->second->writable = true;
						flowItr->second->flagsLock.unlock();
					}
					else
					{
						flowStatsPtr = flowItr->second->statsPtr;
						flowItr->second->flagsLock.unlock();
						payloadPool->free(flowItr->second->payloadPtr);
						flowPool->free(flowItr->second);
						if (writer->write(flowStatsPtr) != E_SUCCESS) {
							cerr << "neoflow: flush(): Writer error!" << endl;
							// TODO don't call abort()
							abort();
						}
					}
					eraseList.push_back(flowItr->first);
				}
			}
			//
			// Removes any flows we have written to disk from the flow
			// table.
			// As far as I know, this cannot be done in the above loop due to
			// iterator invalidation.
			//
			for (size_t index(0); index < eraseList.size(); ++index) {
				flowTable.erase(flowTable.find(eraseList[index]));
			}
			pthread_mutex_unlock(&(flowTableLocks[bucket]));

			eraseList.clear();
		}
		pthread_mutex_unlock(&flushLock);
		return 0;
	}

	//
	// Waits for flush() to finish, stops the classification thread, and writes
	// all remaining flows to disk.
	//
	int finish() {
		// Waits for flush() to finish. 
		pthread_mutex_lock(&flushLock);
		cout << "neoflow: finish()ing..." << endl;

		cout << "neoflow: stopping classifier." << endl;
		payloadClassifier->stop();

		cout << "neoflow: writing remaining flows." << endl;
		// Writes all flows to disk.
		for (unordered_map <string, Flow*>::iterator flowItr(flowTable.begin());
			 flowItr != flowTable.end();
			 ++flowItr)
		{
			if (writer->write(flowItr->second->statsPtr) != E_SUCCESS) {
				cerr << "neoflow: finish(): Writer error!" << endl;
				// TODO don't call abort()
				abort();
			}
		}
		pthread_mutex_unlock(&flushLock);

		cout << "neoflow: closing writer." << endl;
		// Waits for all flows to be written to disk. 
		writer->close();

		flowTable.clear();

		delete writer;
		delete[] flowTableLocks;
		delete payloadClassifier;
		delete payloadPool;
		delete statsPool;
		delete flowPool;

		cout << "neoflow: finish()ed..." << endl;

		return 0;
	}
}
