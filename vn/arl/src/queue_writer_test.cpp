#include <iostream>
#include <queue>

#include "shared/HBFResult.hpp"
#include "shared/QueueWriter.hpp"

using namespace std;
using namespace vn::arl::shared;

int main() {
	queue <HBFResult> q;
	QueueWriter <HBFResult> qw(q);
	HBFResult result;

	for (int i = 0; i < 100; ++i) {
		qw.write(result);
	}

	size_t count(0);
	while (!q.empty()) {
		q.pop();
		++count;
	}

	cout << "Count: " << count << endl;

	return 0;
}
