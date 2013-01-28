#include <iostream>
#include <fstream>
#include <string>

#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include "infer_conf_parser.hpp"

using namespace std;
using namespace boost::program_options;
using boost::property_tree::ptree;
using namespace boost::property_tree::infer_conf_parser;

int main(int argc, char **argv) {
	options_description desc_gen("Infer Configuration Interface");
	desc_gen.add_options()
		(
			"help",
			"display help message"
		)
		(
			"config-file",
			value<string>()->default_value("/usr/local/etc/infer.conf"),
			"specify configuration file"
		)
		(
			"option-name",
			value<string>(),
			"the name of the configuration option"
		)
		(
			"option-value",
			value<string>(),
			"the value to apply to the specified option"
		)
	;
	
	positional_options_description pd;
	pd.add("option-name", 1).add("option-value", 2);

	variables_map vm;
	try {
		store(command_line_parser(argc, argv).
			options(desc_gen).positional(pd).run(), vm);
	}
	catch (error e) {
		cerr << e.what() << endl;
		cout << "usage: " << argv[0] << " option-name [option-value]" << endl;
		return 1;
	}
	notify(vm);

	if (vm.count("help")) {
		cout << "usage: " << argv[0] << " option-name [option-value]" << endl;
		cout << desc_gen << endl;
		return 0;
	}

	if (!vm.count("option-name")) {
		cerr << "error: missing option-name" << endl;
		cout << "usage: " << argv[0] << " option-name [option-value]" << endl;
		cout << desc_gen << endl;
		return 1;
	}

	string var;
	ptree pt;
	read_infer_conf(vm["config-file"].as<string>(), pt);
	if (vm.count("option-value")) {
		pt.put(vm["option-name"].as<string>(), vm["option-value"].as<string>());
		ofstream fout(vm["config-file"].as<string>().c_str(),
					  ios_base::out | ios_base::trunc);
		if (fout.fail()) {
			cerr << "error: unable to open configuration file for writing."
				 << endl;
			return 1;
		}
		write_infer_conf(fout, pt);
		fout.close();
	}
	else {
		try {
			var = pt.get<string>(vm["option-name"].as<string>());
		}
		catch (boost::property_tree::ptree_bad_path &) {
			cerr << "error: option not set" << endl;
			return 1;
		}
		cout << var << endl;
	}
	
	return 0;
}
