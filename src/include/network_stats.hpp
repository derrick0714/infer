#ifndef INFER_INCLUDE_NETWORKSTATS_HPP_
#define INFER_INCLUDE_NETWORKSTATS_HPP_

#include <string>

class network_stats {
  public:
	network_stats()
		:_date(),
		 _internal_ip_count(0),
		 _external_ip_count(0)
	{
	}

	network_stats(const std::string &date,
				  uint32_t internal_ip_count,
				  uint32_t external_ip_count)
		:_date(date),
		 _internal_ip_count(internal_ip_count),
		 _external_ip_count(external_ip_count)
	{
	}

	std::string date() const {
		return _date;
	}

	void date(const std::string &date) {
		_date = date;
	}

	uint32_t internal_ip_count() const {
		return _internal_ip_count;
	}

	void internal_ip_count(uint32_t internal_ip_count) {
		_internal_ip_count = internal_ip_count;
	}

	uint32_t external_ip_count() const {
		return _external_ip_count;
	}

	void external_ip_count(uint32_t external_ip_count) {
		_external_ip_count = external_ip_count;
	}

  private:
	std::string _date;
	uint32_t _internal_ip_count;
	uint32_t _external_ip_count;
};

#endif
