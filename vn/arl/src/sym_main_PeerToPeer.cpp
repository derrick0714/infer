#include "symptoms/PeerToPeer/PeerToPeerSymptom.hpp"
#include "symptoms/PeerToPeer/PeerToPeerArguments.h"
#include "symptoms/NetflowEntry.h"
#include "shared/DB44FileReader.hpp"
#include "shared/DB44FileWriter.hpp"
#include "shared/SingleFileEnumerator.hpp"

#include <iostream>
#include <boost/shared_ptr.hpp>

using namespace std;

using namespace vn::arl::shared;
using namespace vn::arl::symptom;

int main(int argc, char** argv) {
    PeerToPeerArguments args(argc, argv);

    if (args.isHelp()) {
	    cout << args << std::endl;
	    return 0;
    }

    if (!args) {
	    cerr << "Error: " << args.error() << endl << endl
		     << args << endl;

	    return 1;
    }

    DB44FileWriter<NetflowEntry> writer;
    DB44FileReader<NetflowEntry> reader;

    writer.open(args.outputFile());
    reader.open(args.inputDNSFreeFile());

    PeerToPeerSymptom< DB44FileReader<NetflowEntry>, DB44FileWriter<NetflowEntry> > et_sym (args, reader, writer);

    int ret = et_sym.run();

    writer.close();

    if(ret) {
	cout << "Peer To Peer Symptom errored out: " << et_sym.getError() << endl;
    }

    return ret;
}
