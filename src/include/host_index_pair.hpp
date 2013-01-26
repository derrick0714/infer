#ifndef INFER_INCLUDE_HOSTINDEXPAIR_HPP_
#define INFER_INCLUDE_HOSTINDEXPAIR_HPP_

class host_index_pair {
  public:
	host_index_pair()
		:_internal_id(0),
		 _external_id(0)
	{
	}

	host_index_pair(uint32_t internal_id, uint32_t external_id)
		:_internal_id(internal_id),
		 _external_id(external_id)
	{
	}

	uint32_t internal_id() const {
		return _internal_id;
	}

	void internal_id(uint32_t internal_id) {
		_internal_id = internal_id;
	}

	uint32_t external_id() const {
		return _external_id;
	}

	void external_id(uint32_t external_id) {
		_external_id = external_id;
	}

  private:
	uint32_t _internal_id;
	uint32_t _external_id;
};

#endif
