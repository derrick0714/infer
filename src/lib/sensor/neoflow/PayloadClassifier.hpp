#ifndef INFER_INCLUDE_PAYLOADCLASSIFIER_HPP_
#define INFER_INCLUDE_PAYLOADCLASSIFIER_HPP_

#include <queue>

#include <boost/thread/mutex.hpp>

#include "Flow.hpp"
#include "ObjectPool.hpp"

template <typename WriterType>
class PayloadClassifier {
  public:
	PayloadClassifier(WriterType &writer, svm_model *svmModel, ObjectPool<Flow> flowPool, ObjectPool<FlowPayload> payloadPool)
		:_writer(writer),
		 _svmModel(svmModel),
		 _flowPool(flowPool),
		 _payloadPool(payloadPool),
		 _featureSet(),
		 //_svmNode(),
		 _q(),
		 _classifierThread(),
		 _qLock(),
		 _qCond(),
		 _run(false),
		 _active(false),
		 _activeLock(),
		 _lock(),
		 _wakeups(0),
		 _classifiedPayloads(0)
	{
		init_featureset(&_featureSet, FlowPayload::MaxPayload);
	}

	void classify(Flow *flow) {
		flow->flagsLock.lock();
		++(flow->classifyingCount);
		flow->flagsLock.unlock();

		boost::unique_lock<boost::mutex> lock(_qLock);

		_q.push(flow);

		lock.unlock();

		_activeLock.lock();
		if (!_active) {
			_qCond.notify_one();
			++_wakeups;
		}
		++_classifiedPayloads;
		_activeLock.unlock();
	}

	void start() {
		_classifierThread = boost::thread(ClassifierThread(_writer,
														   _svmModel,
														   _flowPool,
														   _payloadPool,
														   _featureSet,
														   _svmNode,
														   _q,
														   _qLock,
														   _qCond,
														   _run,
														   _active,
														   _activeLock,
														   _lock));
		boost::unique_lock<boost::mutex> lock(_qLock);
		while (!_run) {
			_qCond.wait(lock);
		}
		lock.unlock();
	}

	void stop() {
		boost::unique_lock<boost::mutex> lock(_qLock);
		_run = false;
		lock.unlock();
		_qCond.notify_one();

		_classifierThread.join();
	}

	size_t queueSize() {
		_qLock.lock();

		size_t ret(_q.size());

		_qLock.unlock();

		return ret;
	}

	size_t wakeups() {
		_activeLock.lock();
		size_t ret(_wakeups);
		_activeLock.unlock();

		return ret;
	}

	size_t classifiedPayloads() {
		_activeLock.lock();
		size_t ret(_classifiedPayloads);
		_activeLock.unlock();

		return ret;
	}
	
  private:
	class ClassifierThread {
	  public:
		ClassifierThread(WriterType &writer,
						 svm_model *svmModel,
						 ObjectPool<Flow> &flowPool,
						 ObjectPool<FlowPayload> &payloadPool,
						 featureset &featureSet,
						 svm_node *svmNode,
						 std::queue<Flow*> &q,
						 boost::mutex &qLock,
						 boost::condition_variable &qCond,
						 bool &run,
						 bool &active,
						 boost::mutex &activeLock,
						 boost::mutex &lock)
			:_writer(writer),
			 _svmModel(svmModel),
			 _flowPool(flowPool),
			 _payloadPool(payloadPool),
			 _featureSet(featureSet),
			 _svmNode(svmNode),
			 _q(q),
			 _qLock(qLock),
			 _qCond(qCond),
			 _run(run),
			 _active(active),
			 _activeLock(activeLock),
			 _lock(lock)
		{
		}

		void operator()() {
			FlowStats *statsPtr;
			boost::unique_lock<boost::mutex> lock(_lock);
			_run = true;
			lock.unlock();
			_qCond.notify_all();
			lock.lock();
			while(_run) {
				_qCond.wait(lock);

				_activeLock.lock();
				_active = true;
				_activeLock.unlock();

				while (!_q.empty()) {
					// classification.
					FlowStats::ContentType contentType(
						_classifyPayload(_q.front()->payloadPtr));
					_q.front()->statsPtr->content(contentType,
										 _q.front()->statsPtr->content(contentType) + 1);

					_q.front()->flagsLock.lock();
					--(_q.front()->classifyingCount);
					if (_q.front()->writable && _q.front()->classifyingCount == 0) {
						_q.front()->flagsLock.unlock();
						statsPtr = _q.front()->statsPtr;
						_payloadPool.free(_q.front()->payloadPtr);
						_flowPool.free(_q.front());
						_writer.write(statsPtr);
					}
					else {
						_q.front()->payloadPtr->size = 0;
						_q.front()->flagsLock.unlock();
					}

					_qLock.lock();
					_q.pop();
					_qLock.unlock();
				}
				_activeLock.lock();
				_active = false;
				_activeLock.unlock();
			}

			lock.unlock();
		}

	  private:
		FlowStats::ContentType _classifyPayload(const FlowPayload *payload) {
			compute_features(payload->data, payload->size, &_featureSet);
			normalize_features(&_featureSet, payload->size);
			init_svmnode(&_featureSet, _svmNode);

			switch (static_cast<uint8_t>(svm_predict(_svmModel, _svmNode))) {
			  case 1:
				return FlowStats::PLAINTEXT_TYPE;
				break;
			  case 2:
				return FlowStats::BMP_IMAGE_TYPE;
				break;
			  case 3:
				return FlowStats::WAV_AUDIO_TYPE;
				break;
			  case 4:
				return FlowStats::COMPRESSED_TYPE;
				break;
			  case 5:
				return FlowStats::JPEG_IMAGE_TYPE;
				break;
			  case 6:
				return FlowStats::MP3_AUDIO_TYPE;
				break;
			  case 7:
				return FlowStats::MPEG_VIDEO_TYPE;
				break;
			  case 8:
				return FlowStats::ENCRYPTED_TYPE;
				break;
			  default:
				abort();
				break;
			}

			// this should NEVER execute
			return FlowStats::CONTENT_TYPES;
		}

		WriterType &_writer;
		svm_model *_svmModel;
		ObjectPool<Flow> &_flowPool;
		ObjectPool<FlowPayload> &_payloadPool;
		featureset &_featureSet;
		svm_node *_svmNode;
		std::queue<Flow*> &_q;
		boost::mutex &_qLock;
		boost::condition_variable &_qCond;
		bool &_run;
		bool &_active;
		boost::mutex &_activeLock;
		boost::mutex &_lock;
	};

	WriterType &_writer;
	svm_model *_svmModel;
	ObjectPool<Flow> _flowPool;
	ObjectPool<FlowPayload> _payloadPool;
	featureset _featureSet;
	svm_node _svmNode[7];
	std::queue<Flow*> _q;
	boost::thread _classifierThread;
	boost::mutex _qLock;
	boost::condition_variable _qCond;
	bool _run;
	bool _active;
	boost::mutex _activeLock;
	boost::mutex _lock;
	size_t _wakeups;
	size_t _classifiedPayloads;
};

#endif
