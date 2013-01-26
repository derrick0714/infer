#include "shared/HBFQueryProcessor.hpp"
#include "shared/HBF.h"
#include "shared/DB44FileReader.hpp"
#include "shared/ZlibCompressedFileReader.hpp"
#include "shared/StrftimeReadEnumerator.hpp"
#include "shared/EnumeratedFileReader.hpp"
#include "shared/OstreamWriter.hpp"
#include "shared/HBFQueryProcessorConfiguration.h"
#include "shared/HBFQueryProcessorArguments.h"

#include <iostream>

using namespace std;

using namespace vn::arl::shared;

int main(int argc, char **argv) {
	HBFQueryProcessorArguments args(argc, argv);
	
	if (!args) {
		cerr << "Error: " << args.error() << endl << endl
			 << args << endl;

		return 1;
	}

	if (args.help()) {
		cout << args << std::endl;
		return 0;
	}

	HBFQueryProcessorConfiguration conf(args.configFile());
	if (!conf) {
		cerr << "Error: HBFQueryProcessorConfiguration: " << conf.error() 
			 << endl;
		return 1;
	}

	// open the file used for the query
	std::ifstream fin;
	fin.open(args.inputFile().file_string().c_str());
	if (!fin.good()) {
		cerr << "Unable to open input file" << endl;
		return 1;
	}

	// fill the query string
	string queryString(args.queryLength(), '\0');
	fin.read(const_cast<char *>(queryString.data()), args.queryLength());
	if (!fin.good()) {
		cerr << "Unable to read from input file" << endl;
		return 1;
	}

	// close the input file
	fin.close();
	if (!fin.good()) {
		cerr << "Unable to close input file" << endl;
		return 1;
	}

	boost::shared_ptr <StrftimeReadEnumerator> rEnum
				(new StrftimeReadEnumerator(args.inputDir(),
											"%Y/%m/%d/hbf_%H",
											args.startTime(),
											args.endTime()));
	
	if (!(*rEnum)) {
		cerr << "Error: StrftimeReadEnumerator: " << rEnum -> error() << endl;
		return 1;
	}

	/*
	for (StrftimeReadEnumerator::const_iterator it(rEnum -> begin());
		 it != rEnum -> end();
		 ++it)
	{
		cerr << "DEBUG: File: " << *it << endl;
	}
	cerr << endl;
	*/

	EnumeratedFileReader
		<DB44FileReader
			<ZlibCompressedHBF>,
		 StrftimeReadEnumerator
		> enumReader(rEnum);

	OstreamWriter <HBFResult> osWriter(std::cout);

	HBFQueryProcessor
		<EnumeratedFileReader
			<DB44FileReader
				<ZlibCompressedHBF>,
			 StrftimeReadEnumerator
			>,
		 OstreamWriter <HBFResult>
		> hbfQueryProcessor(&enumReader,
							&osWriter,
							queryString,
							args.queryLength(),
							args.matchLength(),
							args.ipv4FlowMatcher(),
							conf.maxMTU(),
							conf.maxFlows(),
							conf.threadCount());

	if (hbfQueryProcessor.run() != 0) {
		cerr << "Error: " << hbfQueryProcessor.error() << endl;
		return 1;
	}

	return 0;
}
