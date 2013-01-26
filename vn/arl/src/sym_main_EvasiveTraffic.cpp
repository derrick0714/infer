#include "symptoms/EvasiveTraffic/EvasiveTrafficSymptom.hpp"
#include "symptoms/EvasiveTraffic/EvasiveTrafficArguments.h"
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
    EvasiveTrafficArguments args(argc, argv);

    if (!args) {
	    cerr << "Error: " << args.error() << endl << endl
		     << args << endl;

	    return 1;
    }

    if (args.isHelp()) {
	    cout << args << std::endl;
	    return 0;
    }


    SingleFileEnumerator fenum(args.inputNetFlowFile());

    DB44FileWriter<NetflowEntry> writer;
    NetFlowARLReader<SingleFileEnumerator> reader(fenum);

    writer.open(args.outputFile());

    EvasiveTrafficSymptom< NetFlowARLReader<SingleFileEnumerator>, DB44FileWriter<NetflowEntry> > et_sym (args, reader, writer);

    et_sym.run();

    writer.close();

    return 0;
}
