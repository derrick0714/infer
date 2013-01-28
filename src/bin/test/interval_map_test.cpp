#include <iostream>

#include "interval.hpp"
#include "interval_map.hpp"

using namespace std;

template <typename Key, typename T>
void print_interval_map(const interval_map<Key, T> &im) {
	for (typename interval_map<Key, T>::const_iterator i(im.begin());
		 i != im.end();
		 ++i)
	{
		cout << '[' << i->first.begin() << ',' << i->first.end() << "]: " << i->second << endl;
	}
	cout << endl;
}

int main() {
	interval_map<interval<uint32_t>, uint32_t> im;

	interval<uint32_t> ival;

	ival.set(3, 5);
	im.insert(make_pair(ival, 35));
	print_interval_map(im);

	ival.set(6, 7);
	im.insert(make_pair(ival, 67));
	print_interval_map(im);

	ival.set(4, 6);
	im.insert(make_pair(ival, 46));
	print_interval_map(im);

	ival.set(2, 8);
	im.insert(make_pair(ival, 28));
	print_interval_map(im);

	ival.set(3, 7);
	im.insert(make_pair(ival, 37));
	print_interval_map(im);

	ival.set(4, 9);
	im.insert(make_pair(ival, 49));
	print_interval_map(im);

	ival.set(1, 5);
	im.insert(make_pair(ival, 15));
	print_interval_map(im);

	ival.set(3, 5);
	im.insert(make_pair(ival, 35));
	print_interval_map(im);

	for (uint32_t i(0); i < 100; ++i) {
		interval_map<interval<uint32_t>, uint32_t>::const_iterator iter(im.find(i));
		if (iter != im.end()) {
			cout << "Found " << i << ": [" << iter->first.begin() << ',' << iter->first.end() << "]: " << iter->second << endl;
		}
	}

	return 0;
}
