#include "pmultimap.hpp"
#include "test_pmultimap_constants.h"
#include <iostream>

int main(int argc, char** argv){

	using namespace vn::arl::shared;

	/// OK, pmultimap works by keeping only a small segment in memory and flushing
	/// the rest of the stuff to disk. The interface is developed with the
	/// assumption that most of the insertion will be done once and then the map
	/// will be searched repeatedly.
	///
	/// Since, the allocator uses disk for storage its members and any members
	/// of the member should be owned by the allocator. So, for now, no member
	/// inserted into pmultimap can have containers in them or allocate memory
	/// within the them (especially if you expect those allocated members to be
	/// file system persistent and be able to read back). But, pmultimap is not 
	/// restricted to POD types. You can have classes and structures in them but
	/// they can't have members allocated on heap (or elsewhere).
	///	
	pmultimap<int, int> p;

	if(argc < 2){
		std::cerr << "usage: " << argv[0] << " <file_name>" << std::endl;
		return 1;
	}

	/// Creates file backed pmultimap on disk. File names are derived from the first
	/// argument. Second agrument is the segment size. This is essentially the
	/// amount of head/main memory used for this instance at any given time. Rest
	/// of it is on disk.
	if(!p.create(argv[1], pmultimap_segment_size)){ // A 1MB pmultimap.
		std::cerr << "Couldn't create a new pmultimap in file " << argv[1] << std::endl;
		return 1;
	}

	std::cout << "container::value_type = " << sizeof(pmultimap<int, int>::value_type) << std::endl;
	std::cout << "container::pointer = " << sizeof(pmultimap<int, int>::container_type::pointer) << std::endl;


	int max_value= pmultimap_max_elements;	/// number of elements essentially
	for(int i= pmultimap_create_start_value; i < max_value; ++i){
		/// Inserts a bunch of elements. When it runs out of segment size a map
		/// will be flushed to disk and a new one will be created. This will go
		/// one until 1000 files are used by this instance. There is no limit on
		/// the segment size per file, could be 1MB or 10GB. For file system
		/// portability reasons better stick to below 2GB, however.
		///
		if(p.insert(std::pair<int, int>(i, (max_value-i))) == p.end()){
			/// You can probably skip the above check
			std::cerr << "Insertion failed for i = " << i << std::endl;
			break;
		}
	}

	/// Mike's test case for segfault with one file on file system.
	std::cerr << "Going to use an empty map" << std::endl;
	std::cerr << "How many elements do we have " << p.size() << std::endl;
	unsigned int processed_records = 0;
	while(p.size() == 0 && p.load_next_segment());

	pmultimap<int, int>::iterator itr = p.begin();
	while(itr != p.end()){
		std::cerr << "pmultimap[" << processed_records << "] = " << itr->first << " ---> " << itr->second << std::endl;
		++itr;
		++processed_records;
	}


	p.close();

	return 0;
}

