#include "shared/FilteredFileEnumerator.hpp"
#include "shared/arl_parsing/PDXRecord.h"
#include "shared/arl_parsing/PDXFileReader.hpp"
#include "shared/arl_parsing/NetFlowRecords.h"
#include "shared/arl_parsing/NetFlowARLReader.hpp"
#include "shared/arl_parsing/ISTRReader.hpp"
#include "shared/arl_parsing/ISTRRecord.h"

#include "boost/date_time/posix_time/time_formatters.hpp"

namespace vs = vn::arl::shared;

using namespace std;
using namespace vs;

int main(int argc, char **argv) {
    if (argc != 2) {
	std::cerr << "usage: " << argv[0] << " path" << std::endl;
	return 1;
    }

    NetFlowARLRecord* vCheck1;
    NetFlowARLRecord* vCheck2;
    NetFlowARLRecord* vCheck3;

    vCheck1 = new NetFlowARL_v1_Record();
    vCheck2 = new NetFlowARL_v2_Record();
    vCheck3 = new NetFlowARL_v3_Record();

    cout << "Available version: " << vCheck1->getVersion()
         << " Size: " << vCheck1->size() << endl;

    cout << "Available version: " << vCheck2->getVersion()
         << " Size: " << vCheck2->size() << endl;

    cout << "Available version: " << vCheck3->getVersion()
         << " Size: " << vCheck3->size() << endl;

    delete vCheck1;
    delete vCheck2;
    delete vCheck3;

    // Testing PIDX
    {
	vs::FilteredFileEnumerator rfe;

	if (!rfe.init(argv[1], ".*pidx$")) {
	    std::cerr << "RecursiveFileEnumerator not initialized. exiting..." << std::endl;
	    return 1;
	}

	for (vs::FilteredFileEnumerator::const_iterator i = rfe.begin();
		i != rfe.end();
		++i) {
	    std::cout << *i << std::endl;
	}

	vs::PDXFileReader<vs::FilteredFileEnumerator> fr(rfe);
	boost::shared_ptr<vs::Serializable <vs::PDXRecord> > rec;

	int records = 0;
	while (rec = fr.read()) {
	    using namespace boost::posix_time;
	    
	    vs::PDXRecord* pdx;
	    vs::TimeStamp time;

	    pdx = static_cast<vs::PDXRecord*>(&(*rec));
	    time = pdx->getFlowTime();

	    //cout << pdx->toString() << endl;

	    records++;
	}

	cout << "Pidx Records: " << records << endl;
    }

    // Testing NetFlow
    {
	vs::FilteredFileEnumerator rfe;
	boost::shared_ptr<vs::Serializable <vs::NetFlowARLRecord> > rec;

	if (!rfe.init(argv[1], ".*flow$")) {
	    std::cerr << "RecursiveFileEnumerator not initialized. exiting..." << std::endl;
	    return 1;
	}

	for (vs::FilteredFileEnumerator::const_iterator i = rfe.begin();
		i != rfe.end();
		++i) {
	    std::cout << *i << std::endl;
	}

	vs::NetFlowARLReader<vs::FilteredFileEnumerator> fr(rfe);

	int records = 0;
	while (rec = fr.read()) {
	    using namespace boost::posix_time;

	    vs::NetFlowARLRecord* netFlow;
	    vs::TimeStamp time;

	    netFlow = static_cast<vs::NetFlowARLRecord*>(&(*rec));
	    time = netFlow->startTime();

	    cout << to_simple_string( (ptime)time ) << " " << netFlow->getClientIP().to_string() << endl;

	    records++;
	}

	cout << "NetFlow Records: " << records << endl;
    }

    // Testing Istr
    {
	vs::FilteredFileEnumerator rfe;
	boost::shared_ptr<vs::Serializable <vs::ISTRRecord> > rec;

	if (!rfe.init(argv[1], ".*istr$")) {
	    std::cerr << "RecursiveFileEnumerator not initialized. exiting..." << std::endl;
	    return 1;
	}

	for (vs::FilteredFileEnumerator::const_iterator i = rfe.begin();
		i != rfe.end();
		++i) {
	    std::cout << *i << std::endl;
	}

	vs::ISTRReader<vs::FilteredFileEnumerator> fr(rfe);

	int records = 0;
	while (rec = fr.read()) {
	    vs::ISTRRecord* istr;

	    istr = static_cast<vs::ISTRRecord*>(&(*rec));

	    records++;
	}

	cout << "ISTR Records: " << records << endl;
    }

    return 0;
}
