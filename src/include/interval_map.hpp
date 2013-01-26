#ifndef INCLUDE_INTERVAL_MAP_HPP
#define INCLUDE_INTERVAL_MAP_HPP

#include <map>

template <typename Interval, typename T>
class interval_map {
  public:
	typedef Interval key_type;
	typedef T mapped_type;
	typedef std::map<key_type, mapped_type> map_type;
	typedef typename map_type::value_type value_type;
	typedef typename map_type::size_type size_type;
	typedef typename map_type::iterator iterator;
	typedef typename map_type::const_iterator const_iterator;
	typedef typename map_type::reverse_iterator reverse_iterator;
	typedef typename map_type::const_reverse_iterator const_reverse_iterator;
	typedef typename map_type::key_compare key_compare;
	typedef typename map_type::value_compare value_compare;
	typedef typename map_type::allocator_type allocator_type;

	iterator begin() {
		return _map.begin();
	}

	const_iterator begin() const {
		return _map.begin();
	}

	iterator end() {
		return _map.end();
	}

	const_iterator end() const {
		return _map.end();
	}

	reverse_iterator rbegin() {
		return _map.rbegin();
	}

	const_reverse_iterator rbegin() const {
		return _map.rbegin();
	}

	reverse_iterator rend() {
		return _map.rend();
	}

	const_reverse_iterator rend() const {
		return _map.rend();
	}

	bool empty() const {
		return _map.empty();
	}

	size_type size() const {
		return _map.size();
	}

	size_type max_size() const {
		return _map.max_size();
	}

	mapped_type & operator[] (const key_type &k) {
		return _map[k];
	}

	std::pair<iterator, bool> insert(const value_type &x) {
		if (_map.empty()) {
			return _map.insert(x);
		}

		iterator i(_map.lower_bound(x.first));
		if (i == _map.end()) {
			return _map.insert(x);
		}

		while (i != _map.end() && !(i->first > x.first)) {
			if (x.first.begin() <= i->first.begin()) {
				if (x.first.end() < i->first.end()) {
					std::pair<key_type, mapped_type> revised_value(*i);
					_map.erase(i++);
					typename key_type::value_type k(x.first.end());
					++k;
					revised_value.first.begin(k);
					_map.insert(revised_value);
				}
				else {
					_map.erase(i++);
				}
			}
			else {
				if (x.first.end() < i->first.end()) {
					std::pair<key_type, mapped_type> new_value_before(*i);
					std::pair<key_type, mapped_type> new_value_after(*i);
					_map.erase(i++);

					typename key_type::value_type k(x.first.begin());
					--k;
					new_value_before.first.end(k);
					_map.insert(new_value_before);

					k = x.first.end();
					++k;
					new_value_after.first.begin(k);
					_map.insert(new_value_after);
				}
				else {
					std::pair<key_type, mapped_type> revised_value(*i);
					_map.erase(i++);
					typename key_type::value_type k(x.first.begin());
					--k;
					revised_value.first.end(k);
					_map.insert(revised_value);
				}
			}
		}

		return _map.insert(x);
	}

	void erase(iterator position) {
		_map.erase(position);
	}

	size_type erase(const key_type &x) {
		return _map.erase(x);
	}

	void erase(iterator first, iterator last) {
		_map.erase(first, last);
	}

	void swap(interval_map<Interval, T> &mp) {
		_map.swap(mp._map);
	}

	void clear() {
		_map.clear();
	}

	key_compare key_comp() const {
		return _map.key_comp();
	}

	value_compare value_comp() const {
		return _map.value_comp();
	}

	iterator find(const key_type &x) {
		return _map.find(x);
	}

	const_iterator find(const key_type &x) const {
		return _map.find(x);
	}

	const_iterator find(const typename key_type::value_type &x) const {
		key_type i;
		i.set(x, x);
		const_iterator ret(_map.lower_bound(i));
		if (ret->first.contains(x)) {
			return ret;
		}

		return _map.end();
	}

	size_type count(const key_type &x) const {
		return _map.count(x);
	}

	iterator lower_bound(const key_type &x) {
		return _map.lower_bound(x);
	}

	const_iterator lower_bound(const key_type &x) const {
		return _map.lower_bound(x);
	}

	iterator upper_bound(const key_type &x) {
		return _map.upper_bound(x);
	}

	const_iterator upper_bound(const key_type &x) const {
		return _map.upper_bound(x);
	}

	std::pair<iterator, iterator> equal_range(const key_type &x) {
		return _map.equal_range(x);
	}

	std::pair<const_iterator, const_iterator> equal_range(const key_type &x) const {
		return _map.equal_range(x);
	}

	allocator_type get_allocator() const {
		return _map.get_allocator();
	}

  private:
	map_type _map;
};

#endif
