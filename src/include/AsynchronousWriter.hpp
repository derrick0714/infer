#ifndef INFER_INCLUDE_ASYNCHRONOUSWRITER_HPP_
#define INFER_INCLUDE_ASYNCHRONOUSWRITER_HPP_

#include <queue>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

#include "DataTypeTraits.hpp"
#include "ErrorStatus.hpp"

template <typename T>
class DeleteDeallocator {
  public:
	void free(T *t) {
		delete t;
	}
};

/// \brief An AsynchronousWriter
///
/// This class provides a buffered 
template <typename WriterType,
		  typename TPointer = typename WriterType::value_type *,
		  typename Deallocator = DeleteDeallocator<typename WriterType::value_type> >
class AsynchronousWriter {
  public:
	/// \brief typedef for the value type
	typedef typename WriterType::value_type value_type;

	/// \brief Constructor
	explicit AsynchronousWriter(boost::shared_ptr<WriterType> writer,
								Deallocator deallocator = Deallocator())
		:_writer(writer),
		 _q(),
		 _qLock(),
		 _qCond(),
		 _lock(),
		 _run(false),
		 _active(false),
		 _activeLock(),
		 _deallocator(deallocator),
		 _writerThread(WriterThread(*_writer,
		 							_q,
									_qLock,
									_qCond,
									_lock,
									_run,
									_active,
									_activeLock,
									_deallocator))
	{
		boost::unique_lock<boost::mutex> lock(_qLock);
		while (!_run) {
			_qCond.wait(lock);
		}
		lock.unlock();
	}

	/// \brief Destructor
	~AsynchronousWriter() {
		_qLock.lock();
		if (_run) {
			_qLock.unlock();
			close();
		}
		else {
			_qLock.unlock();
		}
	}

	/// \brief Write an object
	/// \param obj the object to write
	/// \returns E_SUCCESS if the write was successful
	ErrorStatus write(TPointer ptr) {
		if (!_run) {
			return E_NOTOPEN;
		}
		
		_qLock.lock();
		_q.push(ptr);
		_qLock.unlock();

		_activeLock.lock();
		if (!_active) {
			_qCond.notify_one();
		}
		_activeLock.unlock();

		return E_SUCCESS;
	}

	ErrorStatus close() {
		_qLock.lock();
		if (!_run) {
			_qLock.unlock();
			return E_NOTOPEN;
		}
		_run = false;
		_qCond.notify_one();
		_qLock.unlock();
		_writerThread.join();
		return _writer->close();
	}

  private:
	class WriterThread {
	  public:
		WriterThread(WriterType &writer,
					 std::queue <TPointer> &q,
					 boost::mutex &qLock,
					 boost::condition_variable &qCond,
					 boost::mutex &lock,
					 bool &run,
					 bool &active,
					 boost::mutex &activeLock,
					 Deallocator &deallocator)
			:_writer(writer),
			 _q(q),
			 _qLock(qLock),
			 _qCond(qCond),
			 _lock(lock),
			 _run(run),
			 _active(active),
			 _activeLock(activeLock),
			 _deallocator(deallocator)
		{
		}

		void operator()() {
			boost::unique_lock<boost::mutex> lock(_lock);
			_run = true;
			lock.unlock();
			_qCond.notify_all();
			lock.lock();
			while (_run) {
				_qCond.wait(lock);

				_activeLock.lock();
				_active = true;
				_activeLock.unlock();

				while (!_q.empty()) {
					if (_writer.write(_q.front()) != E_SUCCESS) {
						abort();
					}

					_deallocator.free(_q.front());
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
		WriterType &_writer;
		std::queue <TPointer> &_q;
		boost::mutex &_qLock;
		boost::condition_variable &_qCond;
		boost::mutex &_lock;
		bool &_run;
		bool &_active;
		boost::mutex &_activeLock;
		Deallocator &_deallocator;
	};

	/// \brief Disable copying
	AsynchronousWriter(const AsynchronousWriter &);

	/// \brief Disable copying
	AsynchronousWriter & operator=(const AsynchronousWriter &);

	boost::shared_ptr <WriterType> _writer;
	std::queue <TPointer> _q;
	boost::mutex _qLock;
	boost::condition_variable _qCond;
	boost::mutex _lock;
	bool _run;
	bool _active;
	boost::mutex _activeLock;
	Deallocator _deallocator;

	boost::thread _writerThread;
};

#endif
