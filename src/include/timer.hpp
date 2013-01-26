#ifndef INFER_INCLUDE_TIMER_HPP_
#define INFER_INCLUDE_TIMER_HPP_

#include <sys/time.h>

#include "timeStamp.h"

class timer {
  public:
	timer()
		:_elapsed(0, 0),
		 _begin(),
		 _now(),
		 _running(false)
	{
	}

	void start() {
		if (_running) {
			return;
		}

		_running = true;
		gettimeofday(&_begin, NULL);
	}

	void stop() {
		if (!_running) {
			return;
		}

		_running = false;
		gettimeofday(&_now, NULL);

		_elapsed += TimeStamp(_now) - _begin;
	}

	void reset() {
		_elapsed.set(0, 0);
		
		if (_running) {
			gettimeofday(&_begin, NULL);
		}
	}

	TimeStamp value() const {
		if (!_running) {
			return _elapsed;
		}

		gettimeofday(&_now, NULL);

		return _elapsed + (TimeStamp(_now) - _begin);
	}

  private:
	TimeStamp _elapsed;
	struct timeval _begin;
	mutable struct timeval _now;
	bool _running;
};

#endif
