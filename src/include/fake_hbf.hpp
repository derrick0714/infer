#ifndef INFER_INCLUDE_FAKE_HBF_HPP_
#define INFER_INCLUDE_FAKE_HBF_HPP_

#include "vhbf_header.hpp"

template <typename V>
class fake_hbf {
  public:
	static const size_t hbf_size = V::hbf_size;

	TimeStamp start_time() const {
		return _vhbf_header.start_time();
	}

	TimeStamp end_time() const {
		return _vhbf_header.end_time();
	}

	uint8_t protocol() const {
		return _vhbf_header.protocol();
	}

	uint32_t source_ip() const {
		return _vhbf_header.source_ip();
	}

	uint32_t raw_source_ip() const {
		return _vhbf_header.raw_source_ip();
	}

	uint32_t destination_ip() const {
		return _vhbf_header.destination_ip();
	}

	uint32_t raw_destination_ip() const {
		return _vhbf_header.raw_destination_ip();
	}

	uint16_t source_port() const {
		return _vhbf_header.source_port();
	}

	uint16_t raw_source_port() const {
		return _vhbf_header.raw_source_port();
	}

	uint16_t destination_port() const {
		return _vhbf_header.destination_port();
	}

	uint16_t raw_destination_port() const {
		return _vhbf_header.raw_destination_port();
	}

	uint8_t version() const {
		return _vhbf_header.version();
	}

	uint16_t max_payload() const {
		return _vhbf_header.max_payload();
	}

	uint16_t num_insertions() const {
		return _vhbf_header.num_insertions();
	}

	bool test(size_t n) const {
		return _v->test_bit(_row, n);
	}

  private:
	vhbf_header _vhbf_header;
	V *_v;
	size_t _row;

  public:
	template <typename T>
	struct ID {
		typedef T me;
	};

	friend class ID<V>::me;
};

#endif
