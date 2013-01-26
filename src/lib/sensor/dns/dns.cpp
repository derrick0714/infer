#include <iostream>
#include <tr1/unordered_set>
#include <tr1/unordered_map>
#include <string.h>

#include "dns.h"
#include "DNS.hpp"
#include "FlatFileWriter.hpp"
#include "InferFileWriter.hpp"
#include "AsynchronousWriter.hpp"

using namespace std;
using namespace tr1;

static char flowID[12];
static bool qr, maxFlowsWarning = true;
static unordered_map <string, DNS*> flowTable;
static unordered_map <string, DNS*>::iterator flowItr;
static unordered_set <const char*> visitedPositions;
static pthread_mutex_t *flowTableLocks, flushLock;
static uint32_t queryTimeout;
static size_t numBadPackets, bucket, maxFlows;
static AsynchronousWriter<InferFileWriter<FlatFileWriter<DNS> > > *writer;

static void makeFlowID(char *flowID, const uint32_t &ip, const uint32_t &_ip,
                       const uint16_t &port, const uint16_t &_port)
{
	memcpy(flowID, &ip, 4);
	memcpy(flowID + 4, &_ip, 4);
	memcpy(flowID + 8, &port, 2);
	memcpy(flowID + 10, &_port, 2);
}

// Given the address of a DNS payload, returns the offset of the answer
// section, or 0 if the packet doesn't comply with RFC 1035.
static uint16_t getAnswerSectionOffset(const char *payload) {
	// The first label in the question section starts after 12 bytes. */
	uint16_t answerOffset = 12;
	// RFC 1035 states that labels cannot be larger than 255 bytes. */
	while (*(payload + answerOffset) && answerOffset <= 255) {
		++answerOffset;
	}
	if (answerOffset < 255) {
		//
		// The QTYPE field (two bytes) and QCLASS field (two bytes) follow the
		// label in a question section.
		//
		return answerOffset + 5;
	}
	return 0;
}

// Appends a label to a name.
static bool getLabel(const char *_name, string &name) {
	for (uint8_t index = 0; index < *_name; ++index) {
		if (*(_name + index + 1) < 33 || *(_name + index + 1) > 126) {
			return false;
		}
		name += *(_name + index + 1);
	}
	if (*(_name + *_name + 1)) {
		name += '.';
	}
	return true;
}

// Returns whether a byte is a pointer (has its two most significant bits set).
static bool isPointer(const char *byte) {
	return (*byte & 0xc0);
}

// Copies the QNAME field from an answer section and returns its size, taking
// into account pointers.
static uint16_t getName(string &name, const char *payload, const char *_name) {
	uint16_t nameLength = 0;
	// Copies non-pointer parts.
	while (*_name && !isPointer(_name)) {
		if (name.length() + *_name > 255) {
			return 0;
		}      
		nameLength += *_name + 1;
		if (!getLabel(_name, name)) {
			return 0;
		}
		_name += *_name + 1;
	}
  // Follows any pointers that may exist and copies their contents.
	if (isPointer(_name)) {
		visitedPositions.clear();
		nameLength += 2;
		while (isPointer(_name)) {
			visitedPositions.insert(_name);
			_name = payload + (ntohs(*(uint16_t*)_name) & 0x3fff);
			// Detects more than one visit to the same location in the packet,
			// which would result in an infinite loop.
			if (visitedPositions.find(_name) != visitedPositions.end()) {
				return 0;
			}
			while (*_name && !isPointer(_name)) {
				if (name.length() + *_name > 255) {
					return 0;
				}
				if (!getLabel(_name, name)) {
					return 0;
				}
				_name += *_name + 1;
			}
		}
	}
	else {
		if (nameLength > 2) {
			++nameLength;
		}
	}
	return nameLength;
}

// Returns whether we support the given resource record type.
static bool isSupportedType(uint16_t resourceRecordType) {
	for (uint16_t index = 0; index < NUM_SUPPORTED_TYPES; ++index) {
		if (supportedResourceRecordTypes[index] == resourceRecordType) {
			return true;
		}
	}
	return false;
}

// Fills in the RFC 1035 ANCOUNT, TYPE, TTL, and RDATA fields.
static bool getAnswers(DNS *dnsQuery, const char *payload,
                       const uint16_t &payloadSize)
{
	uint16_t answerOffset = getAnswerSectionOffset(payload), nameLength,
	resourceDataLength, preference;
	if (!answerOffset) {
		return false;
	}
	// We only process the answer section if there are any answers.
	if (!ntohs(*(uint16_t*)(payload + 6))) {
		return true;
	}

	do {
		vector <DNS::DNSResponse*>::iterator it(dnsQuery->responses().insert(dnsQuery -> responses().end(), new DNS::DNSResponse));
		nameLength = getName((*it) -> name(),
							 payload,
							 payload + answerOffset);
		if (!nameLength) {
			return false;
		}
		(*it)->rawType(*(uint16_t*)(payload + answerOffset + nameLength));
		(*it)->rawTTL(*(int32_t*)(payload + answerOffset + nameLength + 4));
		resourceDataLength = ntohs(*(uint16_t*)(payload + answerOffset + nameLength + 8));
		switch ((*it) -> type()) {
		  case A_RECORD:
			(*it) -> resourceData().append((const char*)(payload + answerOffset + nameLength + 10),
			resourceDataLength);
			answerOffset += nameLength + 10 + resourceDataLength;
			break;
		  case MX_RECORD:
			preference = *(uint16_t*)(payload + answerOffset + nameLength + 10);
			(*it) -> resourceData().append((const char*)&preference,
			sizeof(preference));
			if (!getName((*it) -> resourceData(), payload,
				payload + answerOffset + nameLength + 12))
			{
				return false;
			}
			answerOffset += nameLength + 10 + resourceDataLength;
			break;
		  case CNAME_RECORD:
		  case PTR_RECORD:
			if (!getName((*it) -> resourceData(), payload,
				payload + answerOffset + nameLength + 10))
			{
				return false;
			}
			answerOffset += nameLength + 10 + resourceDataLength;
			break;
		}
	} while (answerOffset <= payloadSize &&
		     dnsQuery -> responses().size() < ntohs(*(uint16_t*)(payload + 6)));

	return true;
}

// Fills in the RFC 1035 QNAME and QTYPE fields.
static bool getQuestion(DNS *dnsQuery, const char *payload) {
	uint16_t nameLength = getName(dnsQuery->queryName(), payload, payload + 12);
	if (!nameLength) {
		return false;
	}
	dnsQuery -> rawQueryType(*(uint16_t*)(payload + 12 + nameLength));
	return true;
}

extern "C" {

	int initialize(const configuration &conf,
				   const std::string &outputDirectory,
				   const std::string &)
	{
		if (conf.get(maxFlows, "max-flows", "sensor_dns")
				!= configuration::OK)
		{
			cerr << "sensor_dns: missing or invalid max-flows" << endl;
			return 1;
		}

		if (conf.get(queryTimeout, "query-timeout", "sensor_dns")
				!= configuration::OK)
		{
			cerr << "sensor_dns: missing or invalid query-timeout" << endl;
			return 1;
		}

		flowTable.rehash(maxFlows);
		boost::shared_ptr<StrftimeWriteEnumerator<DNS> >
		enumerator(new StrftimeWriteEnumerator<DNS>(
		outputDirectory, "%Y/%m/%d/dns_%H"));

		boost::shared_ptr<InferFileWriter<FlatFileWriter<DNS> > > inferWriter(new InferFileWriter<FlatFileWriter<DNS> >(enumerator));
		writer = new AsynchronousWriter<InferFileWriter<FlatFileWriter<DNS> > >(inferWriter);
		flowTableLocks = new pthread_mutex_t[flowTable.bucket_count()];
		for (size_t bucket = 0; bucket < flowTable.bucket_count(); ++bucket) {
			if (pthread_mutex_init(&(flowTableLocks[bucket]), NULL) != 0) {
				abort();
			}
		}
		pthread_mutex_init(&flushLock, NULL);
		return 0;
	}

	int processPacket(const Packet &packet) {
		// DNS query.
		if (packet.sourcePort() != ntohs(53)) {
			qr = DNS_QUERY;
			makeFlowID(flowID, packet.sourceIP(), packet.destinationIP(),
			packet.sourcePort(), packet.destinationPort());
		}
		// DNS response.
		else {
			qr = DNS_RESPONSE;
			makeFlowID(flowID, packet.destinationIP(), packet.sourceIP(),
			packet.destinationPort(), packet.sourcePort());
		}
		bucket = flowTable.bucket(flowID);
		// Prevents interference with flush().
		pthread_mutex_lock(&(flowTableLocks[bucket]));
		flowItr = flowTable.find(flowID);
		if (flowItr == flowTable.end()) {
			if (flowTable.size() == maxFlows) {
				if (maxFlowsWarning) {
					cout << "dns: flow table is full" << endl;
					maxFlowsWarning = false;
				}
				pthread_mutex_unlock(&(flowTableLocks[bucket]));
				return 0;
			}
			flowItr = flowTable.insert(make_pair(flowID, new DNS)).first;
			flowItr -> second -> rawClientIP(*(uint32_t*)(flowID));
			flowItr -> second -> rawServerIP(*(uint32_t*)(flowID + 4));
			if (!getQuestion(flowItr -> second, packet.payload())) {
				++numBadPackets;
				delete flowItr -> second;
				flowTable.erase(flowItr);
				pthread_mutex_unlock(&flowTableLocks[bucket]);
				return 0;
			}
		}
		// We only process supported resource record types.
		if (!isSupportedType(flowItr -> second -> queryType())) {
			delete flowItr -> second;
			flowTable.erase(flowItr);
			pthread_mutex_unlock(&flowTableLocks[bucket]);
			return 0;
		}
		if (qr == DNS_QUERY) {
			flowItr->second->queryTime(packet.time());
			flowItr->second->rawQueryFlags(*(uint16_t*)(packet.payload() + 2));
		}
		else {
			flowItr->second->responseTime(packet.time());
			flowItr->second->rawResponseFlags(*(uint16_t*)(packet.payload() + 2));
			if (getAnswers(flowItr->second, packet.payload(),
				packet.payloadSize()))
			{
				writer->write(flowItr -> second);
			}
			else {
				delete flowItr -> second;
				++numBadPackets;
			}
			flowTable.erase(flowItr);
		}
		pthread_mutex_unlock(&flowTableLocks[bucket]);
		return 0;
	}

	int flush() {
		static time_t _time;
		static size_t bucket, index;
		static unordered_map <string, DNS*>::local_iterator flowItr;
		static vector <string> eraseList;
		_time = time(NULL);
		/* Debug output. */
		cout << "dns: flush() called (flowTable: " << flowTable.size()
			 << ", numBadPackets: " << numBadPackets << ')' << endl;
		/* Prevents interference with finish(). */
		pthread_mutex_lock(&flushLock);
		if (flowTable.size()) {
			for (bucket = 0; bucket < flowTable.bucket_count(); ++bucket) {
				if (eraseList.size() > 0) {
					eraseList.clear();
				}
				/* Prevents interference with processPacket(). */
				pthread_mutex_lock(&flowTableLocks[bucket]);
				for (flowItr = flowTable.begin(bucket);
					 flowItr != flowTable.end(bucket);
					 ++flowItr)
				{
					if (_time - flowItr -> second -> queryTime().seconds() >= queryTimeout) {
						writer->write(flowItr -> second);
						eraseList.push_back(flowItr -> first);
					}
				}
				for (index = 0; index < eraseList.size(); ++index) {
					flowTable.erase(flowTable.find(eraseList[index]));
				}
				pthread_mutex_unlock(&flowTableLocks[bucket]);
			}
		}
		pthread_mutex_unlock(&flushLock);
		return 0;
	}

	// Waits for flush() to finish and writes any remaining flows out.
	int finish() {
		// Prevents interference with flush().
		pthread_mutex_lock(&flushLock);
		for (unordered_map <string, DNS*>::iterator flowItr(flowTable.begin());
			 flowItr != flowTable.end();
			 ++flowItr)
		{
			writer->write(flowItr -> second);
		}
		pthread_mutex_unlock(&flushLock);
		writer->close();
		delete writer;
		return 0;
	}
}
