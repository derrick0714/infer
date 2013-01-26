#ifndef INFER_INCLUDE_FLOWPAYLOAD_HPP_
#define INFER_INCLUDE_FLOWPAYLOAD_HPP_

class FlowPayload {
  public:
	static const size_t MaxPayload = 16384;

	FlowPayload()
		:size(0)
	{}

    uint8_t data[MaxPayload];
    uint16_t size;
};

#endif
