#ifndef HBFSYNAPP_HPP
#define HBFSYNAPP_HPP

#include <iostream>
#include <string>
#include <tr1/unordered_map>
#include <openssl/md5.h>
#include <netinet/in.h>

#include "../shared/HBFSynappArguments.h"
#include "../shared/TimeStamp.h"
#include "../shared/HBF.h"
#include "../shared/ZlibCompressedHBF.h"
#include "../shared/TempMemory.hpp"
#include "../shared/HBFSynappConfiguration.h"
#include "../shared/EthernetFrame.h"
#include "../shared/IPv4Datagram.hpp"
#include "../shared/TCPSegment.hpp"
#include "../shared/UDPSegment.hpp"

namespace vn {
namespace arl {
namespace synapps {

using namespace shared;

template <typename ReaderType, typename WriterType>
class HBFSynapp {
  public:
	explicit HBFSynapp(const HBFSynappArguments &args,
					   ReaderType &reader,
					   WriterType &writer);

	int run();

  private:
	const HBFSynappArguments args;
	char flowID[13];
	uint16_t currentBlockSize;
	uint16_t blockNumber;
	uint16_t maxInsertions;
	const char *payload;
	MD5_CTX md5Context;
	unsigned char md[MD5_DIGEST_LENGTH];
	Memory <HBF> memory;
	std::tr1::unordered_map <std::string, HBF*> flowTable;
	std::tr1::unordered_map <std::string, HBF*>::iterator flowItr;
	HBF *flowPtr;
	size_t maxFlows;
	TimeStamp idleTimeout;
	ReaderType &_reader;
	WriterType &_writer;
	std::ostream &error;
	std::ostream &debug;

	HBFSynapp(const HBFSynapp &);
	void operator=(const HBFSynapp&);

	void makeFlowID(uint8_t protocol,
					uint32_t sourceIP,
					uint32_t destinationIP,
					uint16_t sourcePort,
					uint16_t destinationPort);

	void flush();
	void flush(const TimeStamp &t);
};

template <typename ReaderType, typename WriterType>
HBFSynapp<ReaderType, WriterType>::
HBFSynapp(const HBFSynappArguments &args,
		  ReaderType &reader,
		  WriterType &writer)
	:args(args),
	 currentBlockSize(0),
	 blockNumber(0),
	 maxInsertions(HBF::HBFSize / 5),
	 payload(NULL),
	 md5Context(),
	 memory(),
	 flowTable(),
	 flowItr(),
	 flowPtr(NULL),
	 maxFlows(0),
	 idleTimeout(),
	 _reader(reader),
	 _writer(writer),
	 error(std::cerr),
	 debug(std::cout)
{
}

template <typename ReaderType, typename WriterType>
int HBFSynapp<ReaderType, WriterType>::run() {
	if (!args) {
		error << "Error: " << args.error() << std::endl << std::endl
			  << args << std::endl;

		return 1;
	}

	HBFSynappConfiguration conf(args.configFile());
	if (!conf) {
		error << "Error: Configuration: " << conf.error() << std::endl
			  << conf << std::endl;
		return 1;
	}

	idleTimeout = conf.idleTimeout();
	//debug << "idleTimeout: " << idleTimeout.seconds() << '.' << idleTimeout.microseconds() << std::endl;

	/*
	debug << "Command line arguments:" << std::endl;
	debug << "\tInput Dir:   " << args.inputDir() << std::endl;
	debug << "\tOutput Dir:  " << args.outputDir() << std::endl;
	debug << "\tConfig File: " << args.configFile() << std::endl;
	debug << std::endl;
	debug << "Configuration file options:" << std::endl;
	debug << "\tDB Timeout:  " << conf.dbTimeout() << std::endl;
	debug << "\tMax Flows:   " << conf.maxFlows() << std::endl;
	*/

/*
	// only support IPv4, TCP and UDP for now.
	std::string pcapFilter("ip and (tcp or udp)");
	if (!pcapReader.setFilter(pcapFilter)) {
		error << "Error setting pcap filter: \"" << pcapFilter << "\"" 
			  << std::endl;
		return 1;
	}
	//debug << "DEBUG: PcapFileReader initialized." << std::endl;
*/

	memory.initialize(conf.maxFlows());
	if (!memory) {
		error << "Error initializing Memory." << std::endl;
		return 1;
	}
	//debug << "DEBUG: Memory initialized." << std::endl;

	// Supported packet types
	EthernetFrame ef;
	IPv4Datagram <EthernetFrame> d;
	TCPSegment <IPv4Datagram <EthernetFrame> > t;
	UDPSegment <IPv4Datagram <EthernetFrame> > u;

	void *tmpFlowPtr;
	//size_t framesRead(0);
	//size_t framesSkipped(0);
	uint16_t payloadSize;
	TimeStamp time;
	//size_t flushCount(0);
	while (_reader.read(ef)) {
		if (!d.frame(ef)) {
			// unsupported datagram type
			continue;
		}

		if (ef.capturedSize() < conf.minPayload()) {
			continue;
		}

		if (t.datagram(d)) {
			// processing of tcp
			makeFlowID(d.protocol(),
					   d.rawSourceIP(),
					   d.rawDestinationIP(),
					   t.rawSourcePort(),
					   t.rawDestinationPort());
			payloadSize = t.payloadSize();
			time = ef.time();
			payload = (const char *) t.payload();
		} else if (u.datagram(d)) {
			// processing of udp
			makeFlowID(d.protocol(),
					   d.rawSourceIP(),
					   d.rawDestinationIP(),
					   u.rawSourcePort(),
					   u.rawDestinationPort());
			payloadSize = u.payloadSize();
			time = ef.time();
			payload = (const char *) u.payload();
		} else {
			// unsupported segment type. do nothing
			continue;
		}

		flowItr = flowTable.find(flowID);
		if (flowItr == flowTable.end()) {
			memory.lock();
			while ((tmpFlowPtr = reinterpret_cast<void*>(memory.allocate()))
																	== NULL)
			{
				memory.unlock();
				flush(time);
				memory.lock();
				//++flushCount;
			}
			memory.unlock();
			// yeah, baby!
			flowPtr = new(tmpFlowPtr) HBF
									(flowID, payloadSize, time);
			flowItr = flowTable.insert(std::make_pair(flowID, flowPtr)).first;
		} else {
			flowItr -> second -> endTime(time);
			if (payloadSize > flowItr -> second -> maxPayload()) {
				flowItr -> second -> maxPayload(payloadSize);
			}
		}
		currentBlockSize = HBF::BlockSize;
		while (currentBlockSize <= payloadSize) {
			blockNumber = 0;
			while (currentBlockSize * (blockNumber + 1) <= payloadSize)
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

				for (size_t hashNumber(0);
					 hashNumber < HBF::NumHashes;
					 ++hashNumber)
				{
					flowItr -> second -> setBit
							(*(uint32_t*)(md + (hashNumber * sizeof(uint32_t)))
								% HBF::HBFSize);
					//flowItr -> second -> incrementNumInsertions();
				}
				++blockNumber;
			}
			currentBlockSize *= 2;
		}
		/*
		* We delete a flow if we see an RST or FIN TCP flag, or if we have inserted
		* maxInsertions elements into its hierarchical bloom filter.
		*/
		if (flowItr -> second -> numInsertions() >= maxInsertions) {
			if (!_writer.write(ZlibCompressedHBF(*(flowItr -> second)))) {
				error << "Write error: " << _writer.error() << std::endl;
				abort();
			}
			memory.lock();
			memory.free(flowItr -> second);
			memory.unlock();
			flowTable.erase(flowItr);
		} else if (d.segmentType() == SEGMENT_TCP) {
			if (t.rst() || t.fin()) {
				if (!_writer.write(ZlibCompressedHBF(*(flowItr -> second)))) {
					error << "Write error: " << _writer.error() << std::endl;
					abort();
				}
				memory.lock();
				memory.free(flowItr -> second);
				memory.unlock();
				flowTable.erase(flowItr);
			}
		}
	}

	flush();

	/*
	debug << "DEBUG: Frames Read:    " << framesRead << std::endl
		  << "       Frames Skipped: " << framesSkipped << std::endl
		  << "       Total Frames:   " << framesRead + framesSkipped << std::endl;
	*/

	//debug << "Flush count: " << flushCount << std::endl;

	if (!_reader) {
		error << "Error: Reader: " << _reader.error() << std::endl;
		return 1;
	}

	return 0;
}

template <typename ReaderType, typename WriterType>
inline void HBFSynapp<ReaderType, WriterType>::makeFlowID(uint8_t protocol,
											  uint32_t sourceIP,
											  uint32_t destinationIP,
											  uint16_t sourcePort,
											  uint16_t destinationPort)
{
	memcpy(flowID, &protocol, 1);
	memcpy(flowID + 1, &sourceIP, 4);
	memcpy(flowID + 5, &destinationIP, 4);
	memcpy(flowID + 9, &sourcePort, 2);
	memcpy(flowID + 11, &destinationPort, 2);
}

template <typename ReaderType, typename WriterType>
void HBFSynapp<ReaderType, WriterType>::flush() {
	std::tr1::unordered_map <std::string, HBF*>::iterator flowItr
														(flowTable.begin());

	while (flowItr != flowTable.end())
	{
		if (!_writer.write(ZlibCompressedHBF(*(flowItr -> second)))) {
			error << "Write error: " << _writer.error() << std::endl;
			abort();
		}
		memory.lock();
		memory.free(flowItr -> second);
		memory.unlock();
		flowItr = flowTable.erase(flowItr);
	}
}

template <typename ReaderType, typename WriterType>
void HBFSynapp<ReaderType, WriterType>::flush(const TimeStamp &t) {
	size_t flushedFlows(0);

	{
		std::tr1::unordered_map <std::string, HBF*>::iterator flowItr
														(flowTable.begin());

		//debug << "flowTable.size(): " << flowTable.size() << std::endl;

		while (flowItr != flowTable.end())
		{
			if (t - flowItr->second->endTime() < idleTimeout) {
				++flowItr;
				continue;
			}

			if (!_writer.write(ZlibCompressedHBF(*(flowItr -> second)))) {
				error << "Write error: " << _writer.error() << std::endl;
				abort();
			}
			memory.lock();
			memory.free(flowItr -> second);
			memory.unlock();
			flowItr = flowTable.erase(flowItr);
			++flushedFlows;
		}
	}

	//debug << "flushedFlows: " << flushedFlows << std::endl;
	if (flushedFlows == 0) {
		//debug << "HARD flush()" << std::endl;
		std::tr1::unordered_map <std::string, HBF*>::iterator flowItr
														(flowTable.begin());

		size_t flushedFlows(0);
		while (flowItr != flowTable.end())
		{
			if (!_writer.write(ZlibCompressedHBF(*(flowItr -> second)))) {
				error << "Write error: " << _writer.error() << std::endl;
				abort();
			}
			memory.lock();
			memory.free(flowItr -> second);
			memory.unlock();
			flowItr = flowTable.erase(flowItr);
			++flushedFlows;
		}
	}
}

} // namespace synapps
} // namespace arl
} // namespace synapps

#endif
