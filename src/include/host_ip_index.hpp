#ifndef INFER_INCLUDE_HOSTIPINDEX_HPP_
#define INFER_INCLUDE_HOSTIPINDEX_HPP_

class host_ip_index {
  public:
	host_ip_index()
		:_ip(0),
		 _index(0)
	{
	}

	host_ip_index(uint32_t ip, uint32_t index)
		:_ip(ip),
		 _index(index)
	{
	}

	uint32_t ip() const {
		return _ip;
	}

	void ip(uint32_t ip) {
		_ip = ip;
	}

	uint32_t index() const {
		return _index;
	}

	void index(uint32_t index) {
		_index = index;
	}

  private:
	uint32_t _ip;
	uint32_t _index;
};

#endif
