#ifndef PAYLOADSEARCHMANAGER_HPP
#define PAYLOADSEARCHMANAGER_HPP

#include <iostream>
#include <queue>
#include <set>
#include <boost/filesystem.hpp>

#include "StrftimeReadEnumerator.hpp"
#include "EnumeratedFileReader.hpp"
#include "DB44FileReader.hpp"
#include "HBFQueryProcessor.hpp"
#include "HBFHTTPCorrelator.hpp"
#include "SetWriter.hpp"

namespace vn {
namespace arl {
namespace shared {

template <typename WriterType>
class PayloadSearchManager {
  public:
	explicit PayloadSearchManager(WriterType *writer,
								  const boost::filesystem::path &inputDir,
								  const TimeStamp &startTime,
								  const TimeStamp &endTime,
								  const std::string &queryString,
								  size_t queryLength,
								  size_t matchLength,
								  const IPv4FlowMatcher &flowMatcher,
								  uint16_t maxMTU,
								  size_t maxFlows,
								  size_t hbfThreadCount)
		:_writer(writer),
		 _inputDir(inputDir),
		 _startTime(startTime),
		 _endTime(endTime),
		 _queryString(queryString),
		 _queryLength(queryLength),
		 _matchLength(matchLength),
		 _flowMatcher(flowMatcher),
		 _maxMTU(maxMTU),
		 _maxFlows(maxFlows),
		 _hbfThreadCount(hbfThreadCount),
		 _error(false),
		 _errorMsg(),
		 _results(),
		 _hbfQueryThread(),
		 _resultsLock(),
		 _resultsCondition(),
		 _hbfRunning(true)
	{
		if (pthread_mutex_init(&_resultsLock, NULL) != 0) {
			_error = true;
			_errorMsg.assign("Unable to initialize _resultsLock");
			return;
		}
		if (pthread_cond_init(&_resultsCondition, NULL) != 0) {
			_error = true;
			_errorMsg.assign("Unable to initialize _resultsCondition");
			return;
		}
	}

	int run();

	std::string error() const {
		return _errorMsg;
	}

	operator bool() const {
		return !_error;
	}

  private:
	PayloadSearchManager(const PayloadSearchManager &);
	const PayloadSearchManager & operator=(const PayloadSearchManager &);

	static void * hbfQueryThread(void *caller);
	static void * httpCorrelatorThread(void *caller);

	WriterType *_writer;
	boost::filesystem::path _inputDir;
	TimeStamp _startTime;
	TimeStamp _endTime;
	std::string _queryString;
	size_t _queryLength;
	size_t _matchLength;
	IPv4FlowMatcher _flowMatcher;
	uint16_t _maxMTU;
	size_t _maxFlows;
	size_t _hbfThreadCount;

	bool _error;
	std::string _errorMsg;

	std::queue <std::set <HBFResult> *> _results;

	pthread_t _hbfQueryThread;
	pthread_mutex_t _resultsLock;
	pthread_cond_t _resultsCondition;

	bool _hbfRunning;
};

template <typename WriterType>
int PayloadSearchManager<WriterType>::run() {
	// create HBFQueryProcessor thread
	_hbfRunning = true;
	if (pthread_create(&_hbfQueryThread,
					   NULL,
					   hbfQueryThread,
					   this) != 0)
	{
		_error = true;
		_errorMsg.assign("Unable ot create HBF query thread");
		return 1;
	}

	// prepare the correlator
	HBFHTTPCorrelator
		<EnumeratedFileReader
			<DB44FileReader
				<HTTP>,
			 StrftimeReadEnumerator
			>,
		 WriterType
		> correlator;

	// prepare a StrftimeReadEnumerator
	boost::shared_ptr<StrftimeReadEnumerator> readEnum
												(new StrftimeReadEnumerator());

	std::set <HBFResult> *curResults;

	while (true) {
		// lock results lock
		if (pthread_mutex_lock(&_resultsLock) != 0) {
			// error
			_error = true;
			_errorMsg.assign("Unable to lock _resultsLock [1st in loop]");
			if (pthread_join(_hbfQueryThread, NULL) != 0) {
				// something is terribly wrong
				_errorMsg.append(", Unable to join _hbfQueryThread");
			}
			return 1;
		}
		while (_results.empty()) {
			if (!_hbfRunning) {
				break;
			}
			// wait for condition signal
			if (pthread_cond_wait(&_resultsCondition, &_resultsLock) != 0) {
				_error = true;
				_errorMsg.assign("Error waiting for _resultsCondition");
				if (pthread_join(_hbfQueryThread, NULL) != 0) {
					// something is terribly wrong
					_errorMsg.append(", Unable to join _hbfQueryThread");
				}
				return 1;
			}
		}
		if (!_hbfRunning) {
			// unlock results lock
			if (pthread_mutex_unlock(&_resultsLock) != 0) {
				// error
				_error = true;
				_errorMsg.assign("Unable to unlock _resultsLock");
				if (pthread_join(_hbfQueryThread, NULL) != 0) {
					// something is terribly wrong
					_errorMsg.append(", Unable to join _hbfQueryThread");
				}
				return 1;
			}

			break;
		}
		// pop result
		curResults = _results.front();
		_results.pop();

		// unlock results lock
		if (pthread_mutex_unlock(&_resultsLock) != 0) {
			// error
			_error = true;
			_errorMsg.assign("Unable to unlock _resultsLock");
			if (pthread_join(_hbfQueryThread, NULL) != 0) {
				// something is terribly wrong
				_errorMsg.append(", Unable to join _hbfQueryThread");
			}
			return 1;
		}

		// only run the correlator if there are results in the set
		if (curResults->empty()) {
			// free the empty set
			delete curResults;
			continue;
		}

		// initialize read enumerator using a result from the set
		readEnum->init(_inputDir,
					   "%Y/%m/%d/http_%H",
					   curResults->begin()->startTime(),
					   curResults->begin()->startTime());
		if (!(*readEnum)) {
			// error
			_error = true;
			_errorMsg.assign("Unable to initialize read enumerator");
			if (pthread_join(_hbfQueryThread, NULL) != 0) {
				// something is terribly wrong
				_errorMsg.append(", Unable to join _hbfQueryThread");
			}
			return 1;
		}

		// initialize EnumeratedFileReader of DB44FileReaders
		EnumeratedFileReader
			<DB44FileReader
				<HTTP>,
			 StrftimeReadEnumerator
			> reader(readEnum);

		// reinit correlator with EnumeratedFileReader and Writer
		correlator.init(&reader, _writer, curResults);

		// correlator.run()
		if (correlator.run() != 0) {
			_error = true;
			_errorMsg.assign(correlator.error());
			if (pthread_join(_hbfQueryThread, NULL) != 0) {
				// something is terribly wrong
				_errorMsg.append(", Unable to join _hbfQueryThread");
			}
			return 1;
		}

		// free the processed set
		delete curResults;
	}

	// join query processor thread
	if (pthread_join(_hbfQueryThread, NULL) != 0) {
		_error = true;
		_errorMsg.assign("Unable to join _hbfQueryThread");
		return 1;
	}

	return _error?1:0;
}

template <typename WriterType>
void *
PayloadSearchManager<WriterType>::hbfQueryThread(void *caller) {
	PayloadSearchManager<WriterType> *_this
		(reinterpret_cast<PayloadSearchManager<WriterType> *>(caller));

	// initialize read enumerator
	StrftimeReadEnumerator readEnum(_this->_inputDir,
									"%Y/%m/%d/hbf_%H",
									_this->_startTime,
									_this->_endTime);
	if (!readEnum) {
		_this->_error = true;
		_this->_errorMsg.assign("StrftimeReadEnumerator: ");
		_this->_errorMsg.append(readEnum.error());
		// lock results lock
		if (pthread_mutex_lock(&(_this->_resultsLock)) != 0) {
			// error
			_this->_error = true;
			_this->_errorMsg.assign("Unable to lock _resultsLock");
			return NULL;
		}
		_this->_hbfRunning = false;

		// signal condition for correlator thread
		if (pthread_cond_signal(&(_this->_resultsCondition)) != 0) {
			_this->_error = true;
			_this->_errorMsg.assign("Unable to signal _resultsCondition");
			return NULL;
		}

		// unlock results lock
		if (pthread_mutex_unlock(&(_this->_resultsLock)) != 0) {
			// error
			_this->_error = true;
			_this->_errorMsg.assign("Unable to unlock _resultsLock");
			return NULL;
		}
		return NULL;
	}

	HBFQueryProcessor
		<DB44FileReader
			<ZlibCompressedHBF>,
		 SetWriter
		 	<HBFResult>
		> processor;

	DB44FileReader <ZlibCompressedHBF> reader;

	std::set <HBFResult> *curResults;

	// for each file
	for (StrftimeReadEnumerator::const_iterator it(readEnum.begin());
		 it != readEnum.end();
		 ++it)
	{
		// initialize DB44FileReader
		if (!reader.open(*it)) {
			_this->_error = true;
			_this->_errorMsg.assign("DB44FileReader: ");
			_this->_errorMsg.append(reader.error());
			// lock results lock
			if (pthread_mutex_lock(&(_this->_resultsLock)) != 0) {
				// error
				_this->_error = true;
				_this->_errorMsg.assign("Unable to lock _resultsLock");
				return NULL;
			}
			_this->_hbfRunning = false;

			// signal condition for correlator thread
			if (pthread_cond_signal(&(_this->_resultsCondition)) != 0) {
				_this->_error = true;
				_this->_errorMsg.assign("Unable to signal _resultsCondition");
				return NULL;
			}

			// unlock results lock
			if (pthread_mutex_unlock(&(_this->_resultsLock)) != 0) {
				// error
				_this->_error = true;
				_this->_errorMsg.assign("Unable to unlock _resultsLock");
				return NULL;
			}
			return NULL;
		}
		if (!reader) {
			_this->_error = true;
			_this->_errorMsg.assign("DB44FileReader: ");
			_this->_errorMsg.append(reader.error());
			// lock results lock
			if (pthread_mutex_lock(&(_this->_resultsLock)) != 0) {
				// error
				_this->_error = true;
				_this->_errorMsg.assign("Unable to lock _resultsLock");
				return NULL;
			}
			_this->_hbfRunning = false;

			// signal condition for correlator thread
			if (pthread_cond_signal(&(_this->_resultsCondition)) != 0) {
				_this->_error = true;
				_this->_errorMsg.assign("Unable to signal _resultsCondition");
				return NULL;
			}

			// unlock results lock
			if (pthread_mutex_unlock(&(_this->_resultsLock)) != 0) {
				// error
				_this->_error = true;
				_this->_errorMsg.assign("Unable to unlock _resultsLock");
				return NULL;
			}
			return NULL;
		}

		// initialize SetWriter
		curResults = new std::set <HBFResult>;
		SetWriter <HBFResult> writer(*curResults);

		// initialize HBFQuery <DB44FileReader, SetWriter>
		processor.init(&reader,
					   &writer,
					   _this->_queryString,
					   _this->_queryLength,
					   _this->_matchLength,
					   _this->_flowMatcher,
					   _this->_maxMTU,
					   _this->_maxFlows,
					   _this->_hbfThreadCount);

		// hbfQuery.run()
		if (processor.run() != 0) {
			_this->_error = true;
			_this->_errorMsg.assign("HBFQueryProcessor: ");
			_this->_errorMsg.append(processor.error());
			// lock results lock
			if (pthread_mutex_lock(&(_this->_resultsLock)) != 0) {
				// error
				_this->_error = true;
				_this->_errorMsg.assign("Unable to lock _resultsLock");
				return NULL;
			}
			_this->_hbfRunning = false;

			// signal condition for correlator thread
			if (pthread_cond_signal(&(_this->_resultsCondition)) != 0) {
				_this->_error = true;
				_this->_errorMsg.assign("Unable to signal _resultsCondition");
				return NULL;
			}

			// unlock results lock
			if (pthread_mutex_unlock(&(_this->_resultsLock)) != 0) {
				// error
				_this->_error = true;
				_this->_errorMsg.assign("Unable to unlock _resultsLock");
				return NULL;
			}
			return NULL;
		}

		// lock results lock
		if (pthread_mutex_lock(&(_this->_resultsLock)) != 0) {
			// error
			_this->_error = true;
			_this->_errorMsg.assign("Unable to lock _resultsLock");
			return NULL;
		}
		// push_back set into _results
		_this->_results.push(curResults);

		// signal condition for correlator thread
		if (pthread_cond_signal(&(_this->_resultsCondition)) != 0) {
			_this->_error = true;
			_this->_errorMsg.assign("Unable to signal _resultsCondition");
			return NULL;
		}

		// unlock results lock
		if (pthread_mutex_unlock(&(_this->_resultsLock)) != 0) {
			// error
			_this->_error = true;
			_this->_errorMsg.assign("Unable to unlock _resultsLock");
			return NULL;
		}

		reader.close();
	}

	// lock results lock
	if (pthread_mutex_lock(&(_this->_resultsLock)) != 0) {
		// error
		_this->_error = true;
		_this->_errorMsg.assign("Unable to lock _resultsLock");
		return NULL;
	}
	_this->_hbfRunning = false;

	// signal condition for correlator thread
	if (pthread_cond_signal(&(_this->_resultsCondition)) != 0) {
		_this->_error = true;
		_this->_errorMsg.assign("Unable to signal _resultsCondition");
		return NULL;
	}

	// unlock results lock
	if (pthread_mutex_unlock(&(_this->_resultsLock)) != 0) {
		// error
		_this->_error = true;
		_this->_errorMsg.assign("Unable to unlock _resultsLock");
		return NULL;
	}

	return NULL;
}

} // namespace shared
} // namespace arl
} // namespace vn

#endif
