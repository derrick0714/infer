#ifndef INFER_INCLUDE_MEMORYPOOL_HPP_
#define INFER_INCLUDE_MEMORYPOOL_HPP_

#include <stack>
#include <set>

class MemoryPool {
  public:
	MemoryPool(size_t requested_size, size_t next_size = 32)
		:_requested_size(requested_size),
		 _next_size(next_size),
		 _free_chunks(),
		 _chunks()
	{
	}

	~MemoryPool() {
		purge_memory();
	}

	void * malloc() {
		if (_free_chunks.empty()) {
			malloc_next();
		}

		void *ret(_free_chunks.top());
		_free_chunks.pop();
		return ret;
	}

	void free(void * chunk) {
		_free_chunks.push(chunk);
	}

	bool release_memory() {
		if (_free_chunks.empty()) {
			return false;
		}

		while (!_free_chunks.empty()) {
			delete [] reinterpret_cast<char*>(_free_chunks.top());
			_free_chunks.pop();
		}

		return true;
	}

	bool purge_memory() {
		if (_chunks.empty()) {
			return false;
		}

		while (!_free_chunks.empty()) {
			_free_chunks.pop();
		}

		for (std::set<void*>::iterator it(_chunks.begin());
			 it != _chunks.end();
			 ++it)
		{
			delete [] reinterpret_cast<char*>(*it);
		}

		return true;
	}

	void next_size(size_t next_size) {
		_next_size = next_size;
	}

	size_t next_size() const {
		return _next_size;
	}

  private:
	MemoryPool(const MemoryPool &);
	void operator=(const MemoryPool &);

	void malloc_next() {
		void *next;
		for (size_t i(0); i < _next_size; ++i) {
			next = reinterpret_cast<void*>(
				new (std::nothrow) char[_requested_size]);
			_chunks.insert(next);
			_free_chunks.push(next);
		}
		
		_next_size = _chunks.size();
	}

	const size_t _requested_size;
	size_t _next_size;
	std::stack<void*> _free_chunks;
	std::set<void*> _chunks;
};

#endif
