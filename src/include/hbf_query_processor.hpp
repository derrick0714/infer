#ifndef INFER_INCLUDE_HBF_QUERY_PROCESSOR_HPP_
#define INFER_INCLUDE_HBF_QUERY_PROCESSOR_HPP_

#include <fstream>
#include <string>
#include <tr1/unordered_map>
#include <openssl/md5.h>
#include <netinet/in.h>
#include <queue>

#include "timeStamp.h"
#include "HBF.h"
#include "HBFResult.hpp"
#include "IPv4FlowMatcher.hpp"
#include "ErrorStatus.hpp"

#include "OstreamHelpers.h"

// FIXME? maybe? maybe not? -Justin
// NOTE: although there is a threadCount parameter, DO NOT set it to a value
//       greater than 1 UNLESS the ReaderType is thread-safe. No locking on
//       the reader is handled by this class   -Justin
//       Perhaps this shall be solved with traits:
//              if (traits<reader>::thread_safe()) then it's ok for multithread
template <typename ReaderType, typename WriterType>
class hbf_query_processor {
  public:
  	explicit hbf_query_processor();
	explicit hbf_query_processor(ReaderType *reader,
							   WriterType *writer,
							   const std::string &queryString,
							   size_t queryLength,
							   size_t matchLength,
							   const IPv4FlowMatcher &flowMatcher,
							   uint16_t maxMTU,
							   size_t threadCount);

	void init(ReaderType *reader,
			  WriterType *writer,
			  const std::string &queryString,
			  size_t queryLength,
			  size_t matchLength,
			  const IPv4FlowMatcher &flowMatcher,
			  uint16_t maxMTU,
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

	hbf_query_processor(const hbf_query_processor &);
	void operator=(const hbf_query_processor&);

	static void * _queryThread(void *caller);

	void initializeResultMatrix(const uint16_t maxMTU);
	template <typename T>
	static bool isAtAlignment(const T &hbf,
							  const std::vector <uint32_t> &positions,
							  size_t index);
	template <typename T>
	static bool findAlignment(const T &hbf,
					   uint16_t maxPayload,
					   uint16_t &queryStringOffset,
					   uint16_t &blockNumber,
					   size_t &index,
					   const std::vector <
							std::vector <
								std::vector <uint32_t>
							>
						> &resultMatrix);
	template <typename T>
	static bool isAtAlignment(uint16_t queryStringOffset,
					   const T &hbf,
					   uint16_t blockNumber,
					   unsigned char md[MD5_DIGEST_LENGTH],
					   unsigned char block[2048],
					   MD5_CTX &md5Context,
					   uint16_t blockSize,
					   const std::string &queryString);
};

template <typename ReaderType, typename WriterType>
hbf_query_processor<ReaderType, WriterType>::hbf_query_processor()
	:_reader(NULL),
	 _writer(NULL),
	 _error(false),
	 _errorMsg(),
	 _resultMatrix(),
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
	pthread_mutex_init(&_resultLock, NULL);
}

template <typename ReaderType, typename WriterType>
void
hbf_query_processor<ReaderType, WriterType>
	::init(ReaderType *reader,
		   WriterType *writer,
		   const std::string &queryString,
		   size_t queryLength,
		   size_t matchLength,
		   const IPv4FlowMatcher &flowMatcher,
		   uint16_t maxMTU,
		   size_t threadCount)
{
	_reader = reader;
	_writer = writer;
	_queryString = queryString;
	_queryLength = queryLength;
	_matchLength = matchLength;
	_flowMatcher = flowMatcher;
	_maxMTU = maxMTU;
	_threadCount = threadCount;
}

template <typename ReaderType, typename WriterType>
hbf_query_processor<ReaderType, WriterType>
	::hbf_query_processor(ReaderType *reader,
						WriterType *writer,
						const std::string &queryString,
						size_t queryLength,
						size_t matchLength,
						const IPv4FlowMatcher &flowMatcher,
						uint16_t maxMTU,
						size_t threadCount)
	:_reader(reader),
	 _writer(writer),
	 _error(false),
	 _errorMsg(),
	 _resultMatrix(),
	 _resultLock(),
	 _hbfsMatched(0),
	 _queryString(queryString),
	 _queryLength(queryLength),
	 _matchLength(matchLength),
	 _flowMatcher(flowMatcher),
	 _maxMTU(maxMTU),
	 _threadCount(threadCount)
{
	pthread_mutex_init(&_resultLock, NULL);
}

template <typename ReaderType, typename WriterType>
int hbf_query_processor<ReaderType, WriterType>::run() {
	/*
	debug << "DEBUG: base64 encoded query string: "
		  << Base64::encode(_queryString.data(), _queryString.size())
		  << std::endl << std::endl;
	*/

	// do some initialization
	initializeResultMatrix(_maxMTU);

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
void * hbf_query_processor<ReaderType, WriterType>::_queryThread(void *caller) {
	hbf_query_processor *p
					(reinterpret_cast<hbf_query_processor*>(caller));
	typename ReaderType::value_type hbf;
	HBFResult result;
	uint16_t currentBlockSize, startingBlockNumber, currentBlockNumber, level,
	startingQueryStringOffset, matchingOffset, currentQueryStringOffset;
	MD5_CTX md5Context;
	unsigned char md[MD5_DIGEST_LENGTH], block[2048];
	size_t index, level0Results;
	std::vector <std::vector <bool> > results;

	ErrorStatus error_status;
	while ((error_status = p->_reader->read(hbf)) == E_SUCCESS) {
		/* Level 0. */
		startingQueryStringOffset = 0;
		startingBlockNumber = 0;
		level0Results = 0;
		level0:
		if (!findAlignment(hbf, hbf.max_payload(),
			startingQueryStringOffset, startingBlockNumber,
			index, p->_resultMatrix))
		{
			continue;
		}
		matchingOffset = startingQueryStringOffset;
		currentBlockSize = HBF::BlockSize;
		results.resize(1);
		results[0].clear();
		results[0].resize(hbf.max_payload() / currentBlockSize);
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
			   isAtAlignment(currentQueryStringOffset, hbf,
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
				startingBlockNumber < hbf.max_payload() / HBF::BlockSize)
			{
				++startingBlockNumber;
				goto level0;
			}
			continue;
		}

		/* Levels 1 and up. */
		level = 1;
		while ((currentBlockSize *= 2) <= hbf.max_payload()) {
			if (startingBlockNumber % 2 == 0) {
				startingBlockNumber /= 2;
			}
			else {
				startingBlockNumber = (startingBlockNumber / 2) + 1;
				startingQueryStringOffset += currentBlockSize / 2;
			}

			if (startingBlockNumber >
								(hbf.max_payload() / currentBlockSize) - 1
						||
				!isAtAlignment(startingQueryStringOffset, hbf,
							   startingBlockNumber, md, block, md5Context,
							   currentBlockSize, p->_queryString))
			{
				break;
			}

			results.resize(results.size() + 1);
			results[level].clear();
			results[level].resize(hbf.max_payload() / currentBlockSize);
			results[level][startingBlockNumber] = true;
			currentBlockNumber = startingBlockNumber;
			currentQueryStringOffset = startingQueryStringOffset;
			while (++currentBlockNumber < results[level].size() &&
				   isAtAlignment((currentQueryStringOffset += currentBlockSize),
				   hbf, currentBlockNumber, md, block, md5Context,
				   currentBlockSize, p->_queryString))
			{
				results[level][currentBlockNumber] = true;
			}
			++level;
		}
		result.protocol(hbf.protocol());
		result.rawSourceIP(hbf.raw_source_ip());
		result.rawDestinationIP(hbf.raw_destination_ip());
		result.rawSourcePort(hbf.raw_source_port());
		result.rawDestinationPort(hbf.raw_destination_port());
		result.startTime(hbf.start_time());
		result.endTime(hbf.end_time());
		result.match(p->_queryString.substr(matchingOffset,
											level0Results * HBF::BlockSize));

		pthread_mutex_lock(&p->_resultLock);
		p->_writer->write(result);
		++(p->_hbfsMatched);
		pthread_mutex_unlock(&p->_resultLock);
    }

	if (error_status != E_EOF) {
		// FIXME
		abort();
	}

	return NULL;
}

template <typename ReaderType, typename WriterType>
void hbf_query_processor<ReaderType, WriterType>::initializeResultMatrix(const uint16_t maxMTU) {
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
template <typename T>
bool hbf_query_processor<ReaderType, WriterType>::isAtAlignment(const T &hbf,
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
template <typename T>
bool hbf_query_processor<ReaderType, WriterType>::findAlignment(const T &hbf,
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
template <typename T>
bool hbf_query_processor<ReaderType, WriterType>::isAtAlignment(uint16_t queryStringOffset,
											   const T &hbf,
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

#endif
