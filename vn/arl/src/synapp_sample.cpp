#include "synapps/SampleSynapp.h"

using namespace std;
using namespace vn::arl::shared;
using namespace vn::arl::synapps;

int main(int argc, char **argv) {
	DefaultSynappArguments synArgs(argc, argv);
	SampleSynapp sampleSynapp(synArgs);
	return sampleSynapp.run();
}
