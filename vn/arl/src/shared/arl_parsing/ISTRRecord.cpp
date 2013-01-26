/* 
 * File:   ISTRRecord.cpp
 * Author: Mike
 * 
 * Created on August 12, 2009, 12:29 AM
 */

#include "ISTRRecord.h"

namespace vn {
    namespace arl {
	namespace shared {
	    typedef std::string::size_type size_type;

	    ISTRRecord::ISTRRecord():hasHeader(false) {
	    }

	    ISTRRecord::~ISTRRecord() {
	    }

	    /// \brief Get the start time of the data
	    /// \returns the start time of the data
	    TimeStamp ISTRRecord::startTime() const {
		return TimeStamp();
	    }

	    /// \brief Get the end time of the data
	    /// \returns the end time of the data
	    TimeStamp ISTRRecord::endTime() const {
		return TimeStamp();
	    }

	    /// \brief Get the size of the the serialized data
	    /// \returns the size of the serialized data
	    size_type ISTRRecord::size() const {
		return 0;
	    }

	    /// \brief Serialize data
	    /// \param ostr the string in which to store the serialized data
	    /// \returns true if the data was successfully serialized into ostr
	    bool ISTRRecord::serialize(std::string &ostr) const {
		return false;
	    }

	    /// \brief Unserialize data
	    /// \param istr the string from which unserialize data
	    /// \returns true if the data was successfully unserialized from ostr
	    bool ISTRRecord::unserialize(const std::string &istr) {
		if(!hasHeader) {
		    hasHeader = true;

		    using namespace std;
		    using namespace boost::asio::ip;
		    using namespace boost::posix_time;

		    string marker;
		    string conType;
		    string dateStr;
		    string ipVer;
		    string srcip;
		    string dstip;
		    long hourStr;
		    long minuteStr;
		    long secondStr;
		    long microStr;

		    istringstream head(istr);

		    head >> marker >> srcip >> src_port >> dstip >> dst_port >> conType
			 >> dateStr >> hourStr >> minuteStr >> secondStr >> microStr >> ipVer;

		    src_ip = address::from_string(srcip);
		    dst_ip = address::from_string(dstip);

		    ptime dateStamp = ptime(boost::gregorian::from_undelimited_string(dateStr));
		    dateStamp += hours(hourStr);
		    dateStamp += minutes(minuteStr);
		    dateStamp += seconds(secondStr);
		    dateStamp += microseconds(microStr);

		    timeStamp = dateStamp;
		} else {
		    lines.push_back(istr);
		}

		return true;
	    }

	    const std::vector<std::string>& ISTRRecord::getLines() const {
		return lines;
	    }

	    const boost::asio::ip::address& ISTRRecord::getSourceIP() const {
		return src_ip;
	    }

	    const boost::asio::ip::address& ISTRRecord::getDestinationIP() const {
		return dst_ip;
	    }

	    const int ISTRRecord::getSourcePort() const {
		return src_port;
	    }

	    const int ISTRRecord::getDestinationPort() const {
		return dst_port;
	    }

	    const TimeStamp& ISTRRecord::getTimeStamp() const {
		return timeStamp;
	    }

	} // namespace shared
    } // namespace arl
} // namespace vn
