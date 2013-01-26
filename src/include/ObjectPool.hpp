#ifndef INFER_INCLUDE_OBJECTPOOL_HPP_
#define INFER_INCLUDE_OBJECTPOOL_HPP_

#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

#include "MemoryPool.hpp"

template <typename T>
class ObjectPool {
  public:
	ObjectPool(size_t maxObjects)
		:_pool(new MemoryPool(sizeof(T), maxObjects)),
		 _availableObjects(new size_t(maxObjects)),
		 _mutex(new boost::mutex)
	{
	}

	T * allocate() {
		_mutex->lock();
		if (*_availableObjects == 0) {
			_mutex->unlock();
			return NULL;
		}

		T *t(reinterpret_cast<T*>(_pool->malloc()));
		if (t == NULL) {
			_mutex->unlock();
			return NULL;
		}

		--(*_availableObjects);
		_mutex->unlock();

		return new(t) T;
	}

	void free(T *t) {
		t->~T();
		_mutex->lock();
		_pool->free(t);
		++(*_availableObjects);
		_mutex->unlock();
	}

  private:
	boost::shared_ptr<MemoryPool> _pool;
	boost::shared_ptr<size_t> _availableObjects;
	boost::shared_ptr<boost::mutex> _mutex;
};

#endif
