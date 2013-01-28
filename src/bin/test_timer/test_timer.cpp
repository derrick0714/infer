#include <iostream>

#include "timer.hpp"

using namespace std;

int main() {
	timer t;
	t.start();
	cout << "Hello, world!" << endl;
	sleep(1);
	t.stop();
	cout << "cout took: " << t.value().seconds() << ':' << t.value().microseconds() << endl;
	
	t.start();
	sleep(2);
	t.stop();
	cout << "cout took: " << t.value().seconds() << ':' << t.value().microseconds() << endl;

	t.reset();
	t.start();
	sleep(3);
	cout << "cout took: " << t.value().seconds() << ':' << t.value().microseconds() << endl;
	t.reset();
	sleep(4);
	cout << "cout took: " << t.value().seconds() << ':' << t.value().microseconds() << endl;

	return 0;
}
