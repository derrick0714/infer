#ifndef INFER_LIB_SENSOR_NEOFLOW_FLOW_HPP_
#define INFER_LIB_SENSOR_NEOFLOW_FLOW_HPP_

#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

#include "FlowStats.hpp"
#include "FlowPayload.hpp"

struct Flow {
	Flow()
		:statsPtr(NULL),
		 payloadPtr(NULL),
		 flagsLock(),
		 classifyingCount(0),
		 writable(false)
	{
	}

	FlowStats *statsPtr;
	FlowPayload *payloadPtr;

	boost::mutex flagsLock;
	size_t classifyingCount;
	bool writable;
};

#endif
