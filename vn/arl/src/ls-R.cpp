#include <iostream>

#include "shared/RecursiveReadEnumerator.hpp"

using namespace std;
using namespace vn::arl::shared;

int main(int argc, char **argv) {
	if (argc != 2) {
		cerr << "usage: " << argv[0] << " path" << endl;
		return 1;
	}

	RecursiveReadEnumerator rfe(argv[1]);

	if (!rfe) {
		cerr << "RecursiveFileEnumerator not initialized. exiting..." << endl;
		return 1;
	}

	for (RecursiveReadEnumerator::const_iterator i = rfe.begin();
		 i != rfe.end();
		 ++i)
	{
		cout << *i << endl;
	}

	return 0;
}
