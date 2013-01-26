#ifndef PCAPFILEREADER_HPP
#define PCAPFILEREADER_HPP

#include <pcap.h>
#include <boost/filesystem/path.hpp>

#include "DataTypeTraits.hpp"
#include "FrameTypeTraits.hpp"

namespace vn {
namespace arl {
namespace shared {

/// \brief A FileReader that reads pcap files
///
/// This class provides a mechanism with which to read data from pcap files.
template <typename T>
class PcapFileReader {
  public:
	typedef file_reader_type_tag category;
	typedef T value_type;

	/// \brief Constructor
	PcapFileReader();
	
	/// \brief Destructor
	~PcapFileReader();

	/// \brief Open a pcap file
	/// \param fileName the file to open
	/// \returns true if the file was successfully opened
	bool open(const boost::filesystem::path &fileName);

	/// \brief Find out if the file is open
	/// \returns true if the file is open
	bool isOpen() const {
		return _isOpen;
	}

	/// \brief Close the currently open pcap file
	/// \returns true if the file was perviously open, and was closed
	/// successfully
	bool close();

	/// \brief Read an object
	/// \param obj the object to read
	bool read(T& obj);

	/// \brief Check for eof
	/// \returns true if there's nothing left to read
	bool eof() const {
		return _eof;
	}

	/// \brief Get the error string
	/// \returns the error string.
	std::string error() const {
		return _errorMsg;
	}

	/// \brief Boolean cast operator
	/// \returns true if there is no error
	///
	/// This is the means by which clients will test for an error.
	operator bool() const {
		return !_error;
	}

	/// \brief Get the name of the currently opened file
	/// \returns the name of the currently opened file
	boost::filesystem::path fileName() const {
		return _fileName;
	}

	/// \brief Set pcap filter
	/// \param filter The pcap filter string
	/// \returns true if the filter has successfully been set
	///
	/// Sets the pcap filter to be used so that only packets matching the
	/// filter string will be read.
	bool setFilter(const std::string &filter);

	/// \brief Get the current pcap filter string
	/// \returns the current pcap filter string
	const std::string & getFilter() const;

  private:
	/// \brief Private copy onstructor
	///
	/// Prevents copying
	PcapFileReader(const PcapFileReader &);

	/// \brief Private copy assignment operator
	///
	/// Prevents copying
	PcapFileReader & operator=(const PcapFileReader &);

	/// \brief Read an ethernet_frame
	/// \param obj the ethernet_frame to read
	/// \returns true if the read was successful
	bool read(T &obj, ethernet_frame_type_tag);

	bool checkFrameType(frame_type_tag);

	bool checkFrameType(ethernet_frame_type_tag);

	/// \brief The error status
	bool _error;

	/// \brief The error message
	std::string _errorMsg;

	/// \brief The eof status
	bool _eof;

	/// \brief The name of the currently open file
	boost::filesystem::path _fileName;

	/// Whether or not the file is currently open
	bool _isOpen;
	
	/// The pcap descriptor
  	pcap_t *_pcapDescriptor;

	/// The current pcap filter string
	std::string _pcapFilter;
};

template <typename T>
PcapFileReader<T>::PcapFileReader()
	:_error(false),
	 _errorMsg(),
	 _eof(false),
	 _fileName(),
	 _isOpen(false),
	 _pcapDescriptor(NULL),
	 _pcapFilter()
{
}

template <typename T>
PcapFileReader<T>::~PcapFileReader() {
	if (_isOpen) {
		close();
	}
}

template <typename T>
bool PcapFileReader<T>::open(const boost::filesystem::path &fileName) {
	if (_isOpen) {
		_error = true;
		_errorMsg.assign("File is already open.");
		return false;
	}

	static char errorBuffer[PCAP_ERRBUF_SIZE];
	_pcapDescriptor = pcap_open_offline(fileName.file_string().c_str(),
										errorBuffer);

	if (_pcapDescriptor == NULL) {
		_errorMsg.assign("unable to open pcap file: ");
		_errorMsg.append(errorBuffer);
		_error = true;
		return false;
	}

	if (!_pcapFilter.empty()) {
		bpf_program bpfProgram;
                /*
                 * SuSe 10, pcap 0.9.7:
                 *   int pcap_compile(pcap_t *, struct bpf_program *, const char *, int, bpf_u_int32);
                 *
                 * CentOS 5.4, pcap 0.9.4:
                 *   int pcap_compile(pcap_t *, struct bpf_program *, char *, int, bpf_u_int32);
                 */
                char* str_filter = (char*)_pcapFilter.c_str();
                
		if (pcap_compile(_pcapDescriptor,
						 &bpfProgram,
						 str_filter,
						 1,
						 0)                   == -1)
		{
			_errorMsg.assign("unable to create pcap filter: ");
			_errorMsg.append(pcap_geterr(_pcapDescriptor));
			_error = true;
			return false;
		}
		if (pcap_setfilter(_pcapDescriptor, &bpfProgram) == -1) {
			_errorMsg.assign("unable to set pcap filter: ");
			_errorMsg.append(pcap_geterr(_pcapDescriptor));
			_error = true;
			return false;
		}
	}
	_isOpen = true;

	if (!checkFrameType(typename T::frame_type())) {
		close();
		_error = true;
		_errorMsg.assign("Frame type mismatch.");
		return false;
	}

	return true;
}

template <typename T>
bool PcapFileReader<T>::checkFrameType(frame_type_tag) {
	return true;;
}

template <typename T>
bool PcapFileReader<T>::checkFrameType(ethernet_frame_type_tag) {
	return pcap_datalink(_pcapDescriptor) == DLT_EN10MB;
}

template <typename T>
bool PcapFileReader<T>::close() {
	if (!_isOpen) {
		_error = true;
		_errorMsg.assign("Pcap file not open.");
		return false;
	}

	pcap_close(_pcapDescriptor);
	_isOpen = false;

	return true;
}

template <typename T>
bool PcapFileReader<T>::read(T &obj)
{
	if (!_isOpen) {
		_error = true;
		_errorMsg.assign("Pcap file not open.");
		return false;
	}

	static int pcapReadStatus;
	static pcap_pkthdr *pcapHeader;
	static const u_char *pcapData;
	pcapReadStatus = pcap_next_ex(_pcapDescriptor, &pcapHeader, &pcapData);
	if (pcapReadStatus == -1) {
		_error = true;
		_errorMsg.assign("Pcap read error: ");
		_errorMsg.append(pcap_geterr(_pcapDescriptor));
		return false;
	}
	if (pcapReadStatus == -2) {
		// no more packets in this file
		_eof = true;
		return false;
	}
	
	// now pcapHeader and pcapData point to the next header and packet.
	// assign them to obj, and we're done here. If this assignment fails, ie.
	// due to a memory allocation error, it's the client's responsibilty to
	// check obj's error status.
	obj.assign(pcapHeader, pcapData);
	return true;
}

template <typename T>
bool PcapFileReader<T>::setFilter(const std::string &filter) {
	if (isOpen()) {
		return false;
	}	

	_pcapFilter = filter;
	return true;
}

template <typename T>
const std::string & PcapFileReader<T>::getFilter() const {
	return _pcapFilter;
}

} // namespace shared
} // namespace arl
} // namespace vn

/// \brief Read an object
/// \param obj the object to read
template <typename T>
vn::arl::shared::PcapFileReader <T> &
	operator>>(vn::arl::shared::PcapFileReader <T> &reader,
			   T &obj)
{
	reader.read(obj);
	return reader;
}

#endif
