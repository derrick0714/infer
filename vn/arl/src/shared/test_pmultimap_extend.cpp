#include "pmultimap.hpp"
#include "test_pmultimap_constants.h"
#include <iostream>

int main(int argc, char** argv){

	using namespace vn::arl::shared;
	pmultimap<int, int> p;

	/// Use this only if you want to extend an extending set of file-backed
	/// pmultimaps. Probably not needed for us at the momement.
	///
	if(argc < 2){
		std::cerr << "usage: " << argv[0] << " <file_name>" << std::endl;
		return 1;
	}

	if(!p.extend(argv[1], pmultimap_segment_size)){ // Load an existing pmultimap.
		std::cerr << "Couldn't load pmultimap from file " << argv[1] << std::endl;
		return 1;
	}

	std::cerr << "(before insertion) pmultimap::size() = " << p.size() << std::endl;
	for(int i= pmultimap_extend_start_value; i < (pmultimap_extend_start_value + pmultimap_max_elements); ++i){
		if(p.insert(std::pair<int, int>(i, ((pmultimap_extend_start_value + pmultimap_max_elements) - i))) == p.end()){
			/// The check probably can be omitted
			std::cerr << "Insertion failed for i = " << i << std::endl;
			break;
		}
	}
	std::cerr << "(after insertion) pmultimap::size() = " << p.size() << std::endl;

	std::cerr << "Clearing the map of everything" << std::endl;
	p.clear();

	std::cerr << "Going to loop through the maps..." << std::endl;
	/// Mike's test case for segfault with one file on file system.
	unsigned int processed_records = 0;
	while(p.size() == 0 && p.load_next_segment());
	std::cerr << "Done looping through the maps..." << std::endl;

	if(p.size()){
		pmultimap<int, int>::iterator itr = p.begin();
		while(itr != p.end()){
			std::cerr << "pmultimap[" << processed_records << "] = " << itr->first << " ---> " << itr->second << std::endl;
			++itr;
			++processed_records;
		}
	}

	/// Mike's reverse case
	std::cerr << "Going to loop through the maps backwards..." << std::endl;
	while(p.size() == 0 && p.load_prev_segment());
	std::cerr << "Done looping through the maps backwards..." << std::endl;
	if(p.size()){
		std::cerr << "pmultimap::size() = " << p.size() << std::endl;
		pmultimap<int, int>::iterator itr = p.begin();
		while(itr != p.end()){
			std::cerr << "pmultimap[" << processed_records << "] = " << itr->first << " ---> " << itr->second << std::endl;
			++itr;
			++processed_records;
		}
	}

	p.close();

	return 0;
}

