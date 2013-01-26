#include "symptoms/FrequentReboots/FrequentRebootsSymptom.hpp"
#include "symptoms/FrequentReboots/FrequentRebootsArguments.h"
#include "symptoms/NetflowEntry.h"
#include "shared/arl_parsing/NetFlowARLReader.hpp"
#include "shared/DB44FileWriter.hpp"
#include "shared/SingleFileEnumerator.hpp"

#include <iostream>
#include <boost/shared_ptr.hpp>

using namespace std;

using namespace vn::arl::shared;
using namespace vn::arl::symptom;

int main(int argc, char** argv) {
    FrequentRebootsArguments args(argc, argv);

    if (args.isHelp()) {
	    cout << args << std::endl;
	    return 0;
    }

    if (!args) {
	    cerr << "Error: " << args.error() << endl << endl
		     << args << endl;

	    return 1;
    }

    SingleFileEnumerator fenum(args.inputNetFlowFile());

    DB44FileWriter<NetflowEntry> writer;
    NetFlowARLReader<SingleFileEnumerator> reader(fenum);

    writer.open(args.outputFile());

    FrequentRebootsSymptom< NetFlowARLReader<SingleFileEnumerator>, DB44FileWriter<NetflowEntry> > et_sym (args, reader, writer);

    int ret = et_sym.run();

    writer.close();

    if(ret) {
	cout << "Frequent Reboots errored out: " << et_sym.getError() << endl;
    }

    return ret;
}
