#include "shared/SingleFileEnumerator.hpp"
#include "shared/DB44FileWriter.hpp"
#include "shared/DNS/DnsPcapReader.hpp"
#include "shared/DNS/DNSPacket.hpp"
#include "shared/DNS/DnsPcapReader.hpp"
#include "shared/arl_parsing/NetFlowARLReader.hpp"
#include "shared/arl_parsing/NetFlowARLRecord.h"
#include "symptoms/DNSFreeFlows/DNSFreeFlowsSymptom.hpp"
#include "symptoms/DNSFreeFlows/DNSFreeFlowsArguments.h"
#include "symptoms/NetflowEntry.h"

#include <boost/asio/ip/address.hpp>

using namespace std;
using namespace vn::arl::shared;
using namespace vn::arl::symptom;

int main(int argc, char** argv) {
    DNSFreeFlowsArguments args(argc, argv);

    if (argc < 2 || args.help()) {
	    cout << args << std::endl;
	    return 0;
    }

    if (!args) {
	    cerr << "Error: " << args.error() << endl << endl
		     << args << endl;

	    return 1;
    }

    SingleFileEnumerator fenum_flow(args.inputNetFlowFile());
    SingleFileEnumerator fenum_pcap(args.inputPcapFile());

    DB44FileWriter<NetflowEntry> writer;
    NetFlowARLReader<SingleFileEnumerator> net_reader(fenum_flow);
    DnsPcapReader<SingleFileEnumerator> dns_reader(fenum_pcap);

    writer.open(args.outputFile());

    DNSFreeFlowsSymptom<
        NetFlowARLReader<SingleFileEnumerator>,
        DnsPcapReader<SingleFileEnumerator>,
        DB44FileWriter<NetflowEntry> > et_sym (args, net_reader, dns_reader, writer);

    int ret = et_sym.run();

    writer.close();

    if(ret) {
	cout << "DNS Free Flows errored out: " << et_sym.getError() << endl;
    }

    return ret;
}