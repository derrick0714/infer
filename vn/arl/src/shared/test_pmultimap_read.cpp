#include "pmultimap.hpp"
#include <iostream>

using namespace std;

const size_t segment_size = (1024*1024); //1MB

int main(int argc, char** argv){
	using namespace vn::arl::shared;
	pmultimap<int, int> p;

	if(argc < 2){
		cerr << "usage: " << argv[0] << " <file_name>" << endl;
		return 1;
	}

	/// The line below loads a pmultimap (or segment) into memory.
	/// All subsequent functions are on this map/segment and only on this
	/// map/segment.
	///
	cerr << "Going to load the file" << endl;
	if(!p.load(argv[1], segment_size)){ // Load an existing pmultimap.
		cerr << "Couldn't load pmultimap from file " << argv[1] << endl;
		return 1;
	}

	/// Want to load or browse through other pmultimaps? Sure. The following loop
	/// steps through them (loading the corresponding map from disk into memory).
	///
	cerr << "Going to read forward... " << endl;
	for(int i = 0; p.load_next_segment(); ++i){
		if(i == 0)
			cerr << "Reading first segment (pmultimap::size() = " << p.size() << endl;
		else
			cerr << "Reading segment " << (i - 1)  << " (pmultimap::size() = " << p.size() << endl;
	}

	cerr << "Going to read backward... " << endl;
	/// Now read backwards
	for(int i = 0; p.load_prev_segment(); ++i){
		if(i == 0)
			cerr << "Reading first segment (pmultimap::size() = " << p.size() << endl;
		else
			cerr << "Reading segment " << (i - 1)  << " (pmultimap::size() = " << p.size() << endl;
	}


	/// Here is an example of search & delete
	/// Find some value, say 17.
	/// There is always a map in memory (your cache). So, search that first.
	///
	pmultimap<int, int>::iterator itr(p.find(17));
	if(itr == p.end()){
		cerr << "Didn't find 17 in memory!" << endl;
		/// Didn't find what you want in memory?
		/// So, it must have been flushed to disk.
		/// Since this map is already in the beginning (see above for-loop
		/// where we traversed back-to-front) we can step through this
		/// front-to-back in search of what you're looking for.
		/// In general, this forward-backward movement is helpful in
		/// implementing much clever search algorithms since a user probably
		/// know more about where an element might be (or inserted). Otherwise,
		/// worst case is a simple search of all available maps in some sequence.
		///
		while(p.load_next_segment()){
			itr = p.find(17);
			if(itr == p.end()){
				cerr << "Didn't find 17 in memory!" << endl;
			}else{
				cerr << "Found: " << itr->first << "-->" << itr->second << endl;
				/// Cool! Now delete it!
				if(!p.erase(17)){
					cerr << "Couldn't delete 17!" << endl;
				}else{
					cerr << "17 is no more. Deleted!" << endl;
				}
				break;
			}
		}
	}else{
		/// Cache hit!
		cerr << "Found: " << itr->first << "-->" << itr->second << endl;

		/// Cool! Now delete it!
		if(!p.erase(17)){
			cerr << "Couldn't delete 17!" << endl;
		}else{
			cerr << "17 is no more. Deleted!" << endl;
		}
	}


	/// This is similar to extend() but on an already opened pmultimap
	///
	cerr << "Going to load the latest file" << endl;
	if(!p.load_latest_segment()){
		cerr << "Failed to load the latest segment" << endl;
	}else{
		cerr << "Loaded latest segment for reading" << endl;
	}

	/// Let's iterate 10 elements from the lastest segment
	cerr << "Iterating through 10 elements of the lastest map" << endl;
	itr = p.begin();
	pmultimap<int, int>::iterator end_itr(p.end());
	for(unsigned int i=0; (i < 10) && (itr != end_itr); ++i, ++itr)
		cerr << "\t[" << i << "] = " << itr->first << " --> " << itr->second << endl;   
	cerr << endl;


	/// I want to get rid of this segment! Sure, just call clear()
	///
	p.clear();
	cerr << "Successfully deleted the map" << endl;

	if(p.begin() == p.end()){
		cerr << "Got rid of the map from this segment successfully" << endl;
	}else{
		cerr << "Failed to get rid of the map from this segment successfully" << endl;
	}

	/// What happens when you insert into a shrunk_to_fit() segment?
	for(int i= 4097; i < 5121; ++i){
		if(p.insert(std::pair<int, int>(i, (5121-i))) == p.end()){
			cerr << "Couldn't insert into map!" << endl;
			break;
		}
	}
	cerr << "Inserted " << p.size() << " elements into map" << endl << endl;


	/// Let's traverse back and forth one more time to assert shrink-to-fit
	/// doesn't affect anything.
	///
	cerr << "Going to read backward... " << endl;
	/// Now read backwards
	for(int i = 0; p.load_prev_segment(); ++i){
		if(i == 0)
			cerr << "Reading first segment (pmultimap::size() = " << p.size() << endl;
		else
			cerr << "Reading segment " << (i - 1)  << " (pmultimap::size() = " << p.size() << endl;
	}
	cerr << endl;

	cerr << "Going to read forward... " << endl;
	for(int i = 0; p.load_next_segment(); ++i){
		if(i == 0)
			cerr << "Reading first segment (pmultimap::size() = " << p.size() << endl;
		else
			cerr << "Reading segment " << (i - 1)  << " (pmultimap::size() = " << p.size() << endl;
	}
	cerr << endl;


	/// What happens when you drop a map in the middle and insert a lot of
	/// elements in it right after that?
	///
	cerr << "Rewinding maps by 2.." << endl;
	/// Now read backwards
	for(int i = 0; (i < 2) && p.load_prev_segment(); ++i){
		if(i == 0)
			cerr << "Reading first segment (pmultimap::size() = " << p.size() << endl;
		else
			cerr << "Reading segment " << (i - 1)  << " (pmultimap::size() = " << p.size() << endl;
		p.clear();
	}
	cerr << endl;

	char c;
	cerr << "Please check the size of shrunk files now: (and press enter)" << endl;
	cin >> c;

	cerr << "Going to insert " << (2048*100) << " elements..." << endl;

	for(int i=0; (i < (2048*100)); ++i){
		if(p.insert(pair<int, int>(i, (i-2048*100))) == p.end()){
			cerr << "Inserted only " << i << " of intended " << (2048*100) << endl;
			break;
		}
	}	

	/// Get a pointer to the encapsulated container
	pmultimap<int, int>::container_type* p_container(p.get_container());
	pmultimap<int, int>::const_range p_range;
	p_range = p_container->equal_range(10);

	p.close();
	
	return 0;
}

