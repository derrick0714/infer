#include "shared/StrftimeWriteEnumerator.hpp"
#include "shared/DB44FileWriter.hpp"
#include "shared/EnumeratedFileWriter.hpp"
#include "shared/RecursiveReadEnumerator.hpp"
#include "shared/PcapFileReader.hpp"
#include "shared/EnumeratedFileReader.hpp"
#include "synapps/HBFSynapp.hpp"

#include <iostream>
#include <boost/shared_ptr.hpp>

using namespace std;

using namespace vn::arl::shared;
using namespace vn::arl::synapps;

int main(int argc, char **argv) {
	HBFSynappArguments synArgs(argc, argv);
	
	if (!synArgs) {
		cerr << "Error: " << synArgs.error() << endl << endl
			 << synArgs << endl;

		return 1;
	}

	if (synArgs.help()) {
		cout << synArgs << std::endl;
		return 0;
	}

	boost::shared_ptr <RecursiveReadEnumerator> fEnum
		(new RecursiveReadEnumerator(synArgs.inputDir()));

	EnumeratedFileReader
		<PcapFileReader
			<EthernetFrame>,
		 RecursiveReadEnumerator
		> enumReader(fEnum);

	boost::shared_ptr <StrftimeWriteEnumerator <ZlibCompressedHBF> > wEnum
				(new StrftimeWriteEnumerator <ZlibCompressedHBF>
							(synArgs.outputDir(), "%Y/%m/%d/hbf_%H"));

	EnumeratedFileWriter
		<DB44FileWriter
			<ZlibCompressedHBF>,
		 StrftimeWriteEnumerator
			<ZlibCompressedHBF>
		> enumWriter(wEnum);

	HBFSynapp
		<EnumeratedFileReader
			<PcapFileReader
				<EthernetFrame>,
			 RecursiveReadEnumerator
			>,
		 EnumeratedFileWriter
			<DB44FileWriter
				<ZlibCompressedHBF>,
			 StrftimeWriteEnumerator
				<ZlibCompressedHBF>
			>
		> hbfSynapp(synArgs, enumReader, enumWriter);

	int ret(hbfSynapp.run());

	if (ret == 0) {
		if (!enumWriter.close()) {
			cerr << "Error closing Writer: " << enumWriter.error() << endl;
			return 1;
		}
	}

	return ret;
}
