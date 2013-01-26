#ifndef INCLUDED_PMAP_HPP
#define INCLUDED_PMAP_HPP

#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/containers/flat_map.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/creation_tags.hpp>

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"

#include <utility>
#include <iostream>
#include <iomanip>
#include <string>

namespace vn{
namespace arl{
namespace shared{
	/// \brief This class implements a persistent STL-like map.
	///
	/// This class implements a persistent STL-like map. The map is file system
	/// persistent and can be used by two or more processes for reading. pmap
	/// does not have concurrency gurantees. It is designed for a use-case
	/// where all elements are inserted initially and subsequently they are
	/// searched.
	///
	/// \remark pmap currently doesn't support iteration over the entire map. The
	/// reason for lack of support is current implementation uses multiple maps
	/// behind the scene and there is no straight forward way to support iterating
	/// over multiple maps in different segments.
	///
	/// \remark Though it is named pmap it is actually a multimap. It is a
	/// plural map in the sense each pmap is represented by many segment_size
	/// byte STL maps. The only advantage of this design is that only
	/// segment_size data is memory resident and rest of it is stored on disk.
	/// Disadvantage is that it is not a proper STL map in that each segment_size
	/// map may have duplicate entries in them and user is responsible for
	/// cleaning up those entries.
	///
	template <typename _Key, typename _Data, typename _Compare=std::less<_Key> >
	class pmap{

		public:
		typedef _Key key_type;
		typedef _Data data_type;
		typedef std::pair<key_type, data_type> value_type;

		typedef boost::interprocess::allocator<value_type, boost::interprocess::managed_mapped_file::segment_manager> allocator_type;
		typedef boost::interprocess::flat_map<_Key, _Data, _Compare, allocator_type> container_type;

		typedef typename container_type::iterator iterator;
		typedef typename container_type::const_iterator const_iterator;
		typedef size_t size_type;

		/// Default constructor
		pmap():segment(NULL), p_allocator(NULL), p_map_container(NULL), segment_size(0), current_file_number(0), read_prev_end(false){
		}

		/// Destructor
		~pmap(){
		}

		/// \brief Creates a persistent map with corresponding segment size.
		/// \param (file_name) File path to a file to persist the map's contents.
		/// \param (segment_size) Size of the memory resident portion of pmap.
		/// \return Returns true on success; False otherwise.
		///
		/// The function creates a file backed filesystem-persistent map. Only
		/// segment_size bytes of the map is stored in memory and rest of it
		/// is flushed to disk with file prefix file_name.XXX This function
		/// will fail if file_name already exists.
		///
		bool create(const char* file_name, size_type segment_size){
			try{
				prefix_file_name.assign(file_name);
				this->segment_size = segment_size;
				segment = new boost::interprocess::managed_mapped_file(boost::interprocess::create_only, prefix_file_name.c_str(), segment_size);
				if(segment == NULL)
					return false;

				p_allocator = new allocator_type(segment->get_segment_manager());
				if(p_allocator == NULL)
					return false;

				p_map_container = segment->construct<container_type>("MyMap")(_Compare(), *p_allocator);
				if(p_map_container == NULL)
					return false;
			}catch(...){
				return false;
			}
			return true;
		}

		/// \brief Extends an existing pmap instance by appending new maps.
		/// \param (file_name) Path of the file which has the pmap to extend.
		/// \param (segment_size) Size of memory resident portion of pmap.
		/// \return Returns true if the pmap is sucessfully loaded for
		/// extending; False otherwise.
		///
		/// Given an existing pmap the function loads the pmap from disk to
		/// append/change the pmap. The function never creates a new pmap if
		/// file_name doesn't exist and returns false.
		///
		bool extend(const char* file_name, size_t segment_size){
			try{
				prefix_file_name.assign(file_name);
				this->segment_size = segment_size;
				current_file_name.assign(_find_latest_file_name());

				segment = new boost::interprocess::managed_mapped_file(boost::interprocess::open_only, current_file_name.c_str());
				if(segment == NULL){
					return false;
				}

				/// Handle the case where the first file may have been shrunk
				/// by clear().
				if(segment->get_size() < segment_size){	
					boost::interprocess::managed_mapped_file::grow(current_file_name.c_str(), (segment_size - segment->get_size()));
				}

				p_allocator = new allocator_type(segment->get_segment_manager());
				if(p_allocator == NULL){
					return false;
				}

				p_map_container = (segment->find<container_type>("MyMap")).first;
				if(p_map_container == NULL){
					return false;
				}
			}catch(...){
				return false;
			}
			return true;
		}

		/// \brief Simply read existing persistent map for querying.
		///
		bool load(const char* file_name, size_t segment_size){
			try{
				prefix_file_name.assign(file_name);
				this->segment_size = segment_size;

				segment = new boost::interprocess::managed_mapped_file(boost::interprocess::open_only, prefix_file_name.c_str());
				if(segment == NULL)
					return false;

				/// Handle the case where the first file may have been shrunk
				/// by clear().
				if(segment->get_size() < segment_size){	
					boost::interprocess::managed_mapped_file::grow(current_file_name.c_str(), (segment_size - segment->get_size()));
				}


				p_allocator = new allocator_type(segment->get_segment_manager());
				if(p_allocator == NULL)
					return false;

				p_map_container = (segment->find<container_type>("MyMap")).first;
				if(p_map_container == NULL)
					return false;
			}catch(...){
				return false;
			}
			return true;
		}

		bool load_next_segment(){
			return _load_next_segment();
		}

		bool load_prev_segment(){
			if(read_prev_end)	// reached the end of the segments
				return false;

			return _load_prev_segment();
		}

		bool load_latest_segment(){
			return _load_latest_segment();
		}

		bool load_first_segment(){
			return _load_first_segment();
		}

		bool close(){
			if(p_allocator != NULL){
				delete p_allocator;
				p_allocator = NULL;
			}

			if(segment != NULL){
				delete segment;
				segment = NULL;
			}
			return true;
		}

		bool insert(const value_type& v){
			bool status;
			try{
				status = p_map_container->insert(v).second;
			}catch(...){

				/// Before you create a new file check to see if this is a
				/// shrunk segment and attent to that. Just like load() and
				/// extend()
				/// Handle the case where the first file may have been shrunk
				/// by clear().
				if(segment->get_size() < segment_size){	
					boost::interprocess::managed_mapped_file::grow(current_file_name.c_str(), (segment_size - segment->get_size()));
				}else{
					//std::cerr << __LINE__ << ": Exception thrown during insert" << std::endl;
					// Probably ran out of memory. Let's make some room.
					if(!_create_new_segment()){
						//std::cerr << __LINE__ << ": Probably ran out of memory. Trying to create a new segment failed." << std::endl;
						return false; // can't do much if this fails.
					}
				}

				// Try insertion again into the new segment.
				try{
					status = p_map_container->insert(v).second;
				}catch(...){
					// If it throws now then we are out of luck!
					//std::cerr << __LINE__ << ": Exception thrown during second insert" << std::endl;
					return status;
				}
				return status;
			}
			return status;
		}

		size_type size() const{ 
			return p_map_container->size();
		}

		iterator begin(){
			return p_map_container->begin();
		}

		const_iterator begin() const{
			return p_map_container->begin();
		}

		iterator end(){
			return p_map_container->end();
		}

		const_iterator end() const{
			return p_map_container->end();
		}

		iterator find(const key_type& key){
			return p_map_container->find(key);
		}

		const_iterator find(const key_type& key) const{
			return p_map_container->find(key);
		}

		size_type erase(const key_type& key){
			return p_map_container->erase(key);
		}

		iterator erase(iterator pos){
			return p_map_container->erase(pos);
		}

		iterator erase(iterator first, iterator last){
			return p_map_container->erase(first, last);
		}

		void clear(){
			/// \remark Wouldn't it be quicker to just destroy the map via
			/// segment->destroy_ptr(p_map_container); and allocate an empty map?
			/// I assume stl::map does the right thing and call the destructor
			/// in clear() which would be same as the suggestion using segment.
			///
			p_map_container->clear();
			if(current_file_number != 0)
				boost::interprocess::managed_mapped_file::shrink_to_fit(current_file_name.c_str());

			/// \todo Instead of shrinking the current file this function
			/// should just remove the file from disk and move any subsequent
			/// files forward to fill this file's name.
		}

		private:
			/// Container related allocators and managed memory components.
			boost::interprocess::managed_mapped_file *segment;
			allocator_type *p_allocator;
			container_type *p_map_container;

			std::string prefix_file_name;
			std::string current_file_name;
			size_t segment_size;
			unsigned int current_file_number;
			bool read_prev_end;


			/// \brief Takes the current prefix and generates the next file in
			/// sequence.
			std::string _get_next_file_name(){
				std::ostringstream tmp;
				tmp << prefix_file_name << "." << std::setfill('0') << std::setw(4) << current_file_number++;
				read_prev_end = false;
				return tmp.str();
			}

			/// \brief Takes the current prefix and generates the previous file in
			/// sequence.
			std::string _get_prev_file_name(){
				if(current_file_number == 0){
					read_prev_end = true;
					return prefix_file_name;
				}

				std::ostringstream tmp;
				tmp << prefix_file_name << "." << std::setfill('0') << std::setw(4) << --current_file_number;
				return tmp.str();
			}

			std::string _find_latest_file_name(){
				//Simple, stupid routine. Bruteforce search the files on disk to
				//find the file that represents the last segment.

				if(!boost::filesystem::exists(prefix_file_name))// No such files exist!
					return prefix_file_name;

				for(unsigned int i=0; i < 2048; ++i){
					std::ostringstream tmp;
					tmp << prefix_file_name << "." << std::setfill('0') << std::setw(4) << i;

					if(!boost::filesystem::exists(tmp.str())){// current try doesn't exist
						if(i != 0){
							current_file_number = i;
							std::ostringstream result;
							result << prefix_file_name << "." << std::setfill('0') << std::setw(4) << (i - 1);
							return result.str();
						}
						return prefix_file_name;
					}
				}

				// this is a problem. Too many files!
				return prefix_file_name;
			}

			/// \brief Flushes current segment and creates a new one.
			///
			bool _create_new_segment(){
				current_file_name.assign(_get_next_file_name()); /// Get the next file name.
				boost::interprocess::managed_mapped_file *_segment;
				allocator_type *_p_allocator;
				container_type *_p_map_container;


				//std::cerr << __LINE__ << ": Going to create segment backing file " << current_file_name << " with segment_size= " << segment_size << std::endl;
				/// Re-create a new segment backed by current_file_name.
				try{
					_segment = new boost::interprocess::managed_mapped_file(boost::interprocess::create_only, current_file_name.c_str(), segment_size);
					if(_segment == NULL){
						//std::cerr << __LINE__ << ": Failed to create segment." << std::endl;
						return false;
					}
	
					_p_allocator = new allocator_type(_segment->get_segment_manager());
					if(_p_allocator == NULL){
						delete _segment;
						//std::cerr << __LINE__ << ": Failed to create allocator." << std::endl;
						return false;
					}
	
					_p_map_container = _segment->construct<container_type>("MyMap")(_Compare(), *_p_allocator);
					if(_p_map_container == NULL){
						delete _p_allocator;
						delete _segment;
						//std::cerr << __LINE__ << ": Failed to create map container." << std::endl;
						return false;
					}
				}catch(...){
					//std::cerr << __LINE__ << ": Exception was thrown in " << __FUNCTION__ << std::endl;
					return false;
				}

				close();	// Flush the old segment to disk

				/// Assign the new segment
				segment = _segment;
				p_allocator = _p_allocator;
				p_map_container = _p_map_container;

				return true;
			}

			bool _load_next_segment(){
				current_file_name.assign(_get_next_file_name()); /// Get the next file name.
				boost::interprocess::managed_mapped_file *_segment;
				allocator_type *_p_allocator;
				container_type *_p_map_container;

				//std::cerr << __LINE__ << ": Going to open a new file " << current_file_name << " current_file_number= " << current_file_number << std::endl; 
				/// Re-create a new segment backed by current_file_name.
				try{
					_segment = new boost::interprocess::managed_mapped_file(boost::interprocess::open_only, current_file_name.c_str());
					if(_segment == NULL){
						--current_file_number;
						//std::cerr << __LINE__ << ": Failed to open file. current_file_number " << current_file_number << std::endl;
						return false;
					}
	
					_p_allocator = new allocator_type(_segment->get_segment_manager());
					if(_p_allocator == NULL){
						--current_file_number;
						delete _segment;
						//std::cerr << __LINE__ << ": Allocate memory for allocator. current_file_number " << current_file_number << std::endl;
						return false;
					}
	
					_p_map_container = (_segment->find<container_type>("MyMap")).first;
					if(_p_map_container == NULL){
						--current_file_number; 
						delete _p_allocator;
						delete _segment;
						//std::cerr << __LINE__ << ": Failed to create map. current_file_number " << current_file_number << std::endl;
						return false;
					}
				}catch(...){
					--current_file_number;
					//std::cerr << __LINE__ << ": Exception was thrown. current_file_number = " << current_file_number << std::endl;
					return false;
				}

				close();	// Flush the old segment to disk

				/// Assign the new segment
				segment = _segment;
				p_allocator = _p_allocator;
				p_map_container = _p_map_container;

				return true;
			}

			bool _load_prev_segment(){
				current_file_name.assign(_get_prev_file_name()); /// Get the next file name.
				boost::interprocess::managed_mapped_file *_segment;
				allocator_type *_p_allocator;
				container_type *_p_map_container;

				/// Re-create a new segment backed by current_file_name.
				try{
					_segment = new boost::interprocess::managed_mapped_file(boost::interprocess::open_only, current_file_name.c_str());
					if(_segment == NULL){
						return false;
					}
	
					_p_allocator = new allocator_type(_segment->get_segment_manager());
					if(_p_allocator == NULL){
						delete _segment;
						return false;
					}
	
					_p_map_container = (_segment->find<container_type>("MyMap")).first;
					if(_p_map_container == NULL){
						delete _p_allocator;
						delete _segment;
						return false;
					}
				}catch(...){
					++current_file_number;
					return false;
				}

				close();	// Flush the old segment to disk

				/// Assign the new segment
				segment = _segment;
				p_allocator = _p_allocator;
				p_map_container = _p_map_container;

				return true;
			}

			bool _load_latest_segment(){
				current_file_name.assign(_find_latest_file_name()); /// Get the next file name.
				boost::interprocess::managed_mapped_file *_segment;
				allocator_type *_p_allocator;
				container_type *_p_map_container;

				/// Re-create a new segment backed by current_file_name.
				try{
					_segment = new boost::interprocess::managed_mapped_file(boost::interprocess::open_only, current_file_name.c_str());
					if(_segment == NULL){
						return false;
					}
	
					_p_allocator = new allocator_type(_segment->get_segment_manager());
					if(_p_allocator == NULL){
						delete _segment;
						return false;
					}
	
					_p_map_container = (_segment->find<container_type>("MyMap")).first;
					if(_p_map_container == NULL){
						delete _p_allocator;
						delete _segment;
						return false;
					}
				}catch(...){
					++current_file_number;
					return false;
				}

				close();	/// Flush the old segment to disk

				/// Assign the new segment
				segment = _segment;
				p_allocator = _p_allocator;
				p_map_container = _p_map_container;

				return true;
			}

			bool _load_first_segment(){
				boost::interprocess::managed_mapped_file *_segment;
				allocator_type *_p_allocator;
				container_type *_p_map_container;

				/// Re-create a new segment backed by current_file_name.
				try{
					_segment = new boost::interprocess::managed_mapped_file(boost::interprocess::open_only, prefix_file_name.c_str());
					if(_segment == NULL){
						return false;
					}
	
					_p_allocator = new allocator_type(_segment->get_segment_manager());
					if(_p_allocator == NULL){
						delete _segment;
						return false;
					}
	
					_p_map_container = (_segment->find<container_type>("MyMap")).first;
					if(_p_map_container == NULL){
						delete _p_allocator;
						delete _segment;
						return false;
					}
				}catch(...){
					return false;
				}

				close();	/// Flush the old segment to disk

				/// Assign the new segment
				current_file_number = 0;
				segment = _segment;
				p_allocator = _p_allocator;
				p_map_container = _p_map_container;

				return true;
			}

	};// class PMap
}//namespace shared
}//namespace arl
}//namespace vn

#endif
