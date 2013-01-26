/* 
 * File:   NetflowViewerMainTemplate.hpp
 * Author: Mike
 *
 * Created on January 20, 2010, 9:12 PM
 */

#ifndef _NETFLOWVIEWERMAINTEMPLATE_HPP
#define	_NETFLOWVIEWERMAINTEMPLATE_HPP

// This field is a static string that is written out when a flow
//  marked by the symptom. ie. "Evasive traffic symptom"
#ifndef FLAG_COMMENT
 #error "FLAG_COMMENT is not defined."
#endif

// name of the symptom to be printed when flows are identified.
//  ie. "Evasive Traffic"
#ifndef SYMPTOM_NAME
 #error "SYMPTOM_NAME is not defined."
#endif


#include "NetflowEntryViewer.hpp"
#include "../NetflowEntry.h"
#include "../../shared/DB44FileReader.hpp"
#include "../../shared/SingleFileEnumerator.hpp"

#include <iostream>
#include <boost/shared_ptr.hpp>

using namespace std;
using namespace vn::arl::shared;
using namespace vn::arl::symptom;

int main(int argc, char** argv) {
    NetflowViewerArguments args(argc, argv);

    if (!args) {
	    cerr << "Error: " << args.error() << endl << endl
		     << args << endl;

	    return 1;
    }

    if (args.isHelp()) {
	    cout << args << std::endl;
	    return 0;
    }


    SingleFileEnumerator fenum(args.inputFile());
    DB44FileReader<NetflowEntry> reader;

    reader.open(args.inputFile());

    NetflowEntryViewer< DB44FileReader<NetflowEntry> > nf_viewer (args, reader, FLAG_COMMENT, SYMPTOM_NAME);

    int ret = nf_viewer.run();

    reader.close();

    return ret;
}

#endif	/* _NETFLOWVIEWERMAINTEMPLATE_HPP */

