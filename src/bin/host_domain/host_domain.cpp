#include <iostream>
#include <string>

#include <boost/program_options.hpp>

#include "configuration.hpp"
#include "domainSuffix.h"

using namespace std;
using namespace boost::program_options;

int main(int argc, char **argv) {
	options_description desc_gen("Arguments");
	desc_gen.add_options()
		("help", "display help message")
		("config-file",
			value<string>()->default_value
				("/usr/local/etc/infer.conf"),
			"specify configuration file")
	;

	variables_map vm;
	try {
		store(command_line_parser(argc, argv).
			options(desc_gen).run(), vm);
	}
	catch (const boost::program_options::error &e) {
		cerr << e.what() << endl;
		return 1;
	}
	notify(vm);

	if (vm.count("help")) {
		cout << desc_gen << endl;
		return 0;
	}

	configuration conf;
	if (!conf.load(vm["config-file"].as<string>())) {
		cerr << argv[0] << ": unable to load configuration" << endl;
		return 1;
	}

	string domain_suffix_file;
	if (conf.get(domain_suffix_file, "domain-suffix-file", "host_domain")
			!= configuration::OK)
	{
		cerr << argv[0] << ": domain-suffix-file required" << endl;
		return 1;
	}

	DomainSuffix domain_suffix_handle;
	{
		ifstream dsf(domain_suffix_file.c_str());
		if (!dsf) {
			cerr << argv[0] << ": unable to open domain suffix file '"
				 << domain_suffix_file << "'." << endl;
			return 1;
		}

		if (!domain_suffix_handle.initialize(dsf)) {
			cerr << argv[0] << ": unable to initialize domain_suffix_handle: "
				 << domain_suffix_handle.error() << endl;
			return 1;
		}
	}

	string line;
	size_t pos;
	while (getline(cin, line)) {
		// process line
		pos = domain_suffix_handle.getDomainPos(line);
		cout << (pos != string::npos ? line.substr(pos) : "") << endl;
	}
}
