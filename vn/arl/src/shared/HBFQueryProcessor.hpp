#ifndef HBFQUERYPROCESSOR_HPP
#define HBFQUERYPROCESSOR_HPP

#include <fstream>
#include <string>
#include <tr1/unordered_map>
#include <openssl/md5.h>
#include <netinet/in.h>
#include <queue>

#include <boost/asio/ip/address_v4.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "TimeStamp.h"
#include "HBF.h"
#include "HBFResult.hpp"
#include "ZlibCompressedHBF.h"
#include "Base64.h"
#include "TempMemory.hpp"
#include "IPv4FlowMatcher.hpp"

#include "OstreamHelpers.h"

namespace vn {
namespace arl {
namespace shared {

template <typename ReaderType, typename WriterType>
class HBFQueryProcessor {
  public:
  	explicit HBFQueryProcessor();
	explicit HBFQueryProcessor(ReaderType *reader,
							   WriterType *writer,
							   const std::string &queryString,
							   size_t queryLength,
							   size_t matchLength,
							   const IPv4FlowMatcher &flowMatcher,
							   uint16_t maxMTU,
							   size_t maxFlows,
							   size_t threadCount);

	void init(ReaderType *reader,
			  WriterType *writer,
			  const std::string &queryString,
			  size_t queryLength,
			  size_t matchLength,
			  const IPv4FlowMatcher &flowMatcher,
			  uint16_t maxMTU,
			  size_t maxFlows,
			  size_t threadCount);

	int run();

	std::string error() const {
		return _errorMsg;
	}

  private:
	typedef void * (*thread_function_t)(void *);

	ReaderType *_reader;
	WriterType *_writer;

	bool _error;
	std::string _errorMsg;

	std::vector <std::vector <std::vector <uint32_t> > > _resultMatrix;

	Memory <HBF> _hbfMemory;
	pthread_mutex_t _memoryLock;
	pthread_cond_t _memoryCondition;

	std::queue <HBF*> _hbfQueue;
	pthread_mutex_t _queueLock;
	pthread_cond_t _queueCondition;

	bool _readerStatus;
	pthread_mutex_t _readerStatusLock;

	// for printing results
	pthread_mutex_t _resultLock;

	size_t _hbfsMatched;
	std::string _queryString;
	size_t _queryLength;
	size_t _matchLength;
	IPv4FlowMatcher _flowMatcher;
	uint16_t _maxMTU;
	size_t _maxFlows;
	size_t _threadCount;

	HBFQueryProcessor(const HBFQueryProcessor &);
	void operator=(const HBFQueryProcessor&);

	static void * _queryThread(void *caller);

	void initializeResultMatrix(const uint16_t maxMTU);
	static bool isAtAlignment(const std::bitset <HBF::HBFSize> &hbf,
							  const std::vector <uint32_t> &positions,
							  size_t index);
	static bool findAlignment(const std::bitset <HBF::HBFSize> &hbf,
					   uint16_t maxPayload,
					   uint16_t &queryStringOffset,
					   uint16_t &blockNumber,
					   size_t &index,
					   const std::vector <
							std::vector <
								std::vector <uint32_t>
							>
						> &resultMatrix);
	static bool isAtAlignment(uint16_t queryStringOffset,
					   const std::bitset <HBF::HBFSize> &hbf,
					   uint16_t blockNumber,
					   unsigned char md[MD5_DIGEST_LENGTH],
					   unsigned char block[2048],
					   MD5_CTX &md5Context,
					   uint16_t blockSize,
					   const std::string &queryString);
};

template <typename ReaderType, typename WriterType>
HBFQueryProcessor<ReaderType, WriterType>::HBFQueryProcessor()
	:_reader(NULL),
	 _writer(NULL),
	 _error(false),
	 _errorMsg(),
	 _resultMatrix(),
	 _hbfMemory(),
	 _memoryLock(),
	 _memoryCondition(),
	 _hbfQueue(),
	 _queueLock(),
	 _queueCondition(),
	 _readerStatus(true),
	 _readerStatusLock(),
	 _resultLock(),
	 _hbfsMatched(0),
	 _queryString(),
	 _queryLength(0),
	 _matchLength(0),
	 _flowMatcher(),
	 _maxMTU(0),
	 _maxFlows(0),
	 _threadCount(0)
{
	pthread_mutex_init(&_readerStatusLock, NULL);

	pthread_mutex_init(&_queueLock, NULL);
	pthread_cond_init(&_queueCondition, NULL);

	pthread_mutex_init(&_memoryLock, NULL);
	pthread_cond_init(&_memoryCondition, NULL);

	pthread_mutex_init(&_resultLock, NULL);

}

template <typename ReaderType, typename WriterType>
void
HBFQueryProcessor<ReaderType, WriterType>
	::init(ReaderType *reader,
		   WriterType *writer,
		   const std::string &queryString,
		   size_t queryLength,
		   size_t matchLength,
		   const IPv4FlowMatcher &flowMatcher,
		   uint16_t maxMTU,
		   size_t maxFlows,
		   size_t threadCount)
{
	_reader = reader;
	_writer = writer;
	_queryString = queryString;
	_queryLength = queryLength;
	_matchLength = matchLength;
	_flowMatcher = flowMatcher;
	_maxMTU = maxMTU;
	_maxFlows = maxFlows;
	_threadCount = threadCount;
}

template <typename ReaderType, typename WriterType>
HBFQueryProcessor<ReaderType, WriterType>
	::HBFQueryProcessor(ReaderType *reader,
						WriterType *writer,
						const std::string &queryString,
						size_t queryLength,
						size_t matchLength,
						const IPv4FlowMatcher &flowMatcher,
						uint16_t maxMTU,
						size_t maxFlows,
						size_t threadCount)
	:_reader(reader),
	 _writer(writer),
	 _error(false),
	 _errorMsg(),
	 _resultMatrix(),
	 _hbfMemory(),
	 _memoryLock(),
	 _memoryCondition(),
	 _hbfQueue(),
	 _queueLock(),
	 _queueCondition(),
	 _readerStatus(true),
	 _readerStatusLock(),
	 _resultLock(),
	 _hbfsMatched(0),
	 _queryString(queryString),
	 _queryLength(queryLength),
	 _matchLength(matchLength),
	 _flowMatcher(flowMatcher),
	 _maxMTU(maxMTU),
	 _maxFlows(maxFlows),
	 _threadCount(threadCount)
{
	// do some initialization
	initializeResultMatrix(_maxMTU);

	pthread_mutex_init(&_readerStatusLock, NULL);

	pthread_mutex_init(&_queueLock, NULL);
	pthread_cond_init(&_queueCondition, NULL);

	pthread_mutex_init(&_memoryLock, NULL);
	pthread_cond_init(&_memoryCondition, NULL);

	pthread_mutex_init(&_resultLock, NULL);
}

template <typename ReaderType, typename WriterType>
int HBFQueryProcessor<ReaderType, WriterType>::run() {
	/*
	debug << "DEBUG: base64 encoded query string: "
		  << Base64::encode(_queryString.data(), _queryString.size())
		  << std::endl << std::endl;
	*/

	// do some initialization
	initializeResultMatrix(_maxMTU);

	// preallocate some memory for the HBFs
	_hbfMemory.initialize(_maxFlows);

	_readerStatus = true;

	// launch query threads
	pthread_t *queryThreads = new pthread_t[_threadCount];
	for (size_t thread = 0; thread < _threadCount; ++thread) {
		if (pthread_create(&(queryThreads[thread]),
					   NULL,
					   (thread_function_t) _queryThread,
					   reinterpret_cast <void *>(this)) != 0) {
			return 1;
		}
	}

	using namespace boost::posix_time;

	// start reading HBFs into the queue
	HBF *hbf;
	ZlibCompressedHBF compressedHBF;
	size_t hbfsRead(0);
	int ret;
	while (_reader->read(compressedHBF)) {
		if (compressedHBF.maxPayload() > _maxMTU) {
			continue;
		}
		if (!_flowMatcher.isMatch(compressedHBF)) {
			continue;
		}

		// if no memory, wait for a query thread to free some up
		pthread_mutex_lock(&_memoryLock);
		while ((hbf = _hbfMemory.allocate()) == NULL) {
			pthread_cond_wait(&_memoryCondition, &_memoryLock);
		}
		// at this point, an hbf has been successfully allocated.
		pthread_mutex_unlock(&_memoryLock);

		// uncompress the bitset portion and put the HBF in the processing
		// queue
		new(hbf) HBF();
		ret = hbf->init(compressedHBF);
		if (ret != Z_OK) {
			_error = true;
			_errorMsg.assign("Error decompressing HBF: ");
			switch (ret) {
			  case Z_MEM_ERROR:
				_errorMsg.append("Not enough memory.");
				break;
			  case Z_BUF_ERROR:
				_errorMsg.append("Not enough room in output buffer.");
				break;
			  case Z_DATA_ERROR:
				_errorMsg.append("Input data corrupted or incomplete.");
				break;
			  default:
				_errorMsg.append("Unknown error.");
				break;
			}
			HBFResult foo;
			foo.rawSourceIP(hbf->rawSourceIP());
			foo.rawDestinationIP(hbf->rawDestinationIP());
			foo.rawSourcePort(hbf->rawSourcePort());
			foo.rawDestinationPort(hbf->rawDestinationPort());
			foo.startTime(hbf->startTime());
			foo.endTime(hbf->endTime());

			return 1;
		}

		HBFResult foo;
		foo.rawSourceIP(hbf->rawSourceIP());
		foo.rawDestinationIP(hbf->rawDestinationIP());
		foo.rawSourcePort(hbf->rawSourcePort());
		foo.rawDestinationPort(hbf->rawDestinationPort());
		foo.startTime(hbf->startTime());
		foo.endTime(hbf->endTime());

		// put hbf in queue, and let a query thread know
		pthread_mutex_lock(&_queueLock);
		_hbfQueue.push(hbf);
		pthread_cond_signal(&_queueCondition);
		pthread_mutex_unlock(&_queueLock);

		++hbfsRead;
	}
	if (!(*_reader)) {
		_error = true;
		_errorMsg.assign("Reader: ");
		_errorMsg.append(_reader->error());
		return 1;
	}

	// reader is done.
	pthread_mutex_lock(&_readerStatusLock);
	_readerStatus = false;
	pthread_mutex_unlock(&_readerStatusLock);

	// wake up any threads waiting for more in the queue, so they can come
	// alive and see there's noting in it and the reader is done
	pthread_cond_broadcast(&_queueCondition);

	// wait for all query threads to finish
	for (size_t thread = 0; thread < _threadCount; ++thread) {
		pthread_join(queryThreads[thread], NULL);
	}
	delete[] queryThreads;

	// print some debug info
	//std::cerr << "DEBUG: HBFs Read:    " << hbfsRead << std::endl
	//	  << "DEBUG: HBFs Matched: " << _hbfsMatched << std::endl;

	return 0;
}

template <typename ReaderType, typename WriterType>
void * HBFQueryProcessor<ReaderType, WriterType>::_queryThread(void *caller) {
	HBFQueryProcessor *p
					(reinterpret_cast<HBFQueryProcessor*>(caller));
	HBF *hbf;
	HBFResult result;
	uint16_t currentBlockSize, startingBlockNumber, currentBlockNumber, level,
	startingQueryStringOffset, matchingOffset, currentQueryStringOffset;
	MD5_CTX md5Context;
	unsigned char md[MD5_DIGEST_LENGTH], block[2048];
	size_t index, level0Results;
	std::vector <std::vector <bool> > results;

	while (true) {
		// as long as the queue is empty, wait for a condition signal
		// UNLESS the reader is finished, in which case that signal
		// would never come
		pthread_mutex_lock(&p->_queueLock);
		while (p->_hbfQueue.empty()) {
			pthread_mutex_lock(&p->_readerStatusLock);

			if (p->_readerStatus == false) {
				pthread_mutex_unlock(&p->_queueLock);
				pthread_mutex_unlock(&p->_readerStatusLock);
				return NULL;
			}
			pthread_mutex_unlock(&p->_readerStatusLock);
			pthread_cond_wait(&p->_queueCondition, &p->_queueLock);
		}

		// now there's something in the queue. get it.
		hbf = p->_hbfQueue.front();
		p->_hbfQueue.pop();

		pthread_mutex_unlock(&p->_queueLock);

		// we now have an hbf from the queue. do something with it.

		/* Level 0. */
		startingQueryStringOffset = 0;
		startingBlockNumber = 0;
		level0Results = 0;
		level0:
		if (!findAlignment(hbf -> hbf(), hbf -> maxPayload(),
			startingQueryStringOffset, startingBlockNumber,
			index, p->_resultMatrix))
		{
			pthread_mutex_lock(&p->_memoryLock);
			p->_hbfMemory.free(hbf);
			pthread_cond_signal(&p->_memoryCondition);
			pthread_mutex_unlock(&p->_memoryLock);
			continue;
		}
		matchingOffset = startingQueryStringOffset;
		currentBlockSize = HBF::BlockSize;
		results.resize(1);
		results[0].clear();
		results[0].resize(hbf -> maxPayload() / currentBlockSize);
		results[0][startingBlockNumber] = true;
		level0Results = 1;
		currentBlockNumber = startingBlockNumber;
		currentQueryStringOffset = startingQueryStringOffset + currentBlockSize;
		while (++currentBlockNumber < results[0].size() &&
			   static_cast<size_t>((currentBlockNumber - startingBlockNumber) *
															currentBlockSize)
									< p->_matchLength &&
			   currentQueryStringOffset + currentBlockSize
									<= p->_queryString.length() &&
			   isAtAlignment(currentQueryStringOffset, hbf -> hbf(),
							 currentBlockNumber, md, block, md5Context,
							 currentBlockSize, p->_queryString))
		{
			results[0][currentBlockNumber] = true;
			++level0Results;
			currentQueryStringOffset += currentBlockSize;
		}

		if (level0Results * HBF::BlockSize < p->_matchLength) {
			if (startingQueryStringOffset <
					(p->_queryString.length() - p->_matchLength) ||
				startingBlockNumber < hbf -> maxPayload() / HBF::BlockSize)
			{
				++startingBlockNumber;
				goto level0;
			}
			pthread_mutex_lock(&p->_memoryLock);
			p->_hbfMemory.free(hbf);
			pthread_cond_signal(&p->_memoryCondition);
			pthread_mutex_unlock(&p->_memoryLock);
			continue;
		}

		/* Levels 1 and up. */
		level = 1;
		while ((currentBlockSize *= 2) <= hbf -> maxPayload()) {
			if (startingBlockNumber % 2 == 0) {
				startingBlockNumber /= 2;
			}
			else {
				startingBlockNumber = (startingBlockNumber / 2) + 1;
				startingQueryStringOffset += currentBlockSize / 2;
			}

			if (startingBlockNumber >
								(hbf -> maxPayload() / currentBlockSize) - 1
						||
				!isAtAlignment(startingQueryStringOffset, hbf -> hbf(),
							   startingBlockNumber, md, block, md5Context,
							   currentBlockSize, p->_queryString))
			{
				break;
			}

			results.resize(results.size() + 1);
			results[level].clear();
			results[level].resize(hbf -> maxPayload() / currentBlockSize);
			results[level][startingBlockNumber] = true;
			currentBlockNumber = startingBlockNumber;
			currentQueryStringOffset = startingQueryStringOffset;
			while (++currentBlockNumber < results[level].size() &&
				   isAtAlignment((currentQueryStringOffset += currentBlockSize),
				   hbf -> hbf(), currentBlockNumber, md, block, md5Context,
				   currentBlockSize, p->_queryString))
			{
				results[level][currentBlockNumber] = true;
			}
			++level;
		}
		result.protocol(hbf->protocol());
		result.rawSourceIP(hbf->rawSourceIP());
		result.rawDestinationIP(hbf->rawDestinationIP());
		result.rawSourcePort(hbf->rawSourcePort());
		result.rawDestinationPort(hbf->rawDestinationPort());
		result.startTime(hbf->startTime());
		result.endTime(hbf->endTime());
		result.match(p->_queryString.substr(matchingOffset,
											level0Results * HBF::BlockSize));

		pthread_mutex_lock(&p->_resultLock);
		p->_writer->write(result);
		++(p->_hbfsMatched);
		pthread_mutex_unlock(&p->_resultLock);

		pthread_mutex_lock(&p->_memoryLock);
		p->_hbfMemory.free(hbf);
		pthread_cond_signal(&p->_memoryCondition);
		pthread_mutex_unlock(&p->_memoryLock);
    }

	return NULL;
}

template <typename ReaderType, typename WriterType>
void HBFQueryProcessor<ReaderType, WriterType>::initializeResultMatrix(const uint16_t maxMTU) {
	unsigned char md[MD5_DIGEST_LENGTH], block[HBF::BlockSize + 2];
	uint16_t numBlocks = maxMTU / HBF::BlockSize;
	_resultMatrix.resize(_queryString.length() - _matchLength);
	for (size_t queryStringOffset = 0;
		 queryStringOffset < _queryString.length() - _matchLength;
		++queryStringOffset)
	{
		_resultMatrix[queryStringOffset].resize(numBlocks);
		memcpy(block,
			   _queryString.c_str() + queryStringOffset,
			   HBF::BlockSize);

		for (uint16_t blockNumber = 0; blockNumber < numBlocks; ++blockNumber) {
			*(uint16_t*)(block + HBF::BlockSize) = blockNumber;
			MD5(block, HBF::BlockSize + 2, md);
			for (size_t hashNumber = 0;
				 hashNumber < HBF::NumHashes;
				 ++hashNumber)
			{
				_resultMatrix[queryStringOffset][blockNumber].push_back
						(*(uint32_t*)(md + (hashNumber * 4)) % HBF::HBFSize);
			}
		}
	}
}

template <typename ReaderType, typename WriterType>
bool HBFQueryProcessor<ReaderType, WriterType>::isAtAlignment(const std::bitset <HBF::HBFSize>
													&hbf,
											   const std::vector <uint32_t>
													&positions,
											   size_t index)
{
	for (index = 0; index < positions.size(); ++index) {
		if (!hbf.test(positions[index])) {
			return false;
		}
	}
	return true;
}

template <typename ReaderType, typename WriterType>
bool HBFQueryProcessor<ReaderType, WriterType>::findAlignment(const std::bitset <HBF::HBFSize>
													&hbf,
											   uint16_t maxPayload,
											   uint16_t &queryStringOffset,
											   uint16_t &blockNumber,
											   size_t &index,
											   const std::vector <
													std::vector <
														std::vector <uint32_t>
													>
												> &resultMatrix)
{
	while (queryStringOffset < resultMatrix.size()) {
		while (blockNumber < maxPayload / HBF::BlockSize) {
			if (isAtAlignment(hbf,
							  resultMatrix[queryStringOffset][blockNumber],
							  index))
			{
				index = resultMatrix[queryStringOffset][blockNumber].size();
				return true;
			}
			++blockNumber;
		}
		blockNumber = 0;
		++queryStringOffset;
	}
	return false;
}

template <typename ReaderType, typename WriterType>
bool HBFQueryProcessor<ReaderType, WriterType>::isAtAlignment(uint16_t queryStringOffset,
											   const std::bitset <HBF::HBFSize>
														&hbf,
											   uint16_t blockNumber,
											   unsigned char
														md[MD5_DIGEST_LENGTH],
											   unsigned char * /*block[2048]*/,
											   MD5_CTX &md5Context,
											   uint16_t blockSize,
											   const std::string &queryString)
{
	// Initialize MD5 context.
	MD5_Init(&md5Context);

	// Hash block.
	MD5_Update(&md5Context,
			   queryString.c_str() + queryStringOffset,
			   blockSize);

	// Hash block number.
	MD5_Update(&md5Context, &blockNumber, sizeof(uint16_t));

	// Finalize hash for block and block number.
	MD5_Final(&(md[0]), &md5Context);

	for (size_t hashNumber = 0; hashNumber < HBF::NumHashes; ++hashNumber) {
		if (!hbf.test(*(uint32_t*)(md + (hashNumber * 4)) % HBF::HBFSize)) {
			return false;
		}
	}

	return true;
}

} // namespace synapps
} // namespace arl
} // namespace vn

#endif
