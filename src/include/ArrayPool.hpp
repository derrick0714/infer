#ifndef INFER_INCLUDE_ARRAYPOOL_HPP_
#define INFER_INCLUDE_ARRAYPOOL_HPP_

#include <boost/shared_array.hpp>
#include <boost/thread/mutex.hpp>

#include "MemoryPool.hpp"

template <typename T>
class ArrayPool {
  public:
	ArrayPool(size_t arrayLen, size_t maxArrays)
		:_pool(sizeof(T) * arrayLen, maxArrays),
		 _availableArrays(maxArrays),
		 _mutex()
	{
	}

	boost::shared_array<T> allocate() {
		_mutex.lock();
		if (_availableArrays == 0) {
			_mutex.unlock();
			return boost::shared_array<T>();
		}

		T *t(reinterpret_cast<T*>(_pool.malloc()));
		if (t == NULL) {
			_mutex.unlock();
			return boost::shared_array<T>();
		}

		--_availableArrays;
		_mutex.unlock();

		return boost::shared_array<T>(t, Deallocator(*this));
	}

  private:
	class Deallocator {
	  public:
		Deallocator(ArrayPool &oPool)
			:_oPool(oPool)
		{
		}

		void operator()(T *p) {
			_oPool.free(p);
		}
	  private:
		ArrayPool<T> &_oPool;
	};

	void free(T *t) {
		_mutex.lock();
		_pool.free(t);
		++_availableArrays;
		_mutex.unlock();
	}

	MemoryPool _pool;
	size_t _availableArrays;
	boost::mutex _mutex;
};

#endif
