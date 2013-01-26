#include <netinet/in.h>

#include "socketHelpers.hpp"
#include "sqlTime.h"
#include "nameResolution.h"
#include "queryManagerConfiguration.h"

NameResolution::NameResolution(std::string sockPath, size_t sockTimeout)
{
	this -> sockPath = sockPath;
	this -> sockTimeout = sockTimeout;
}

bool NameResolution::getIPsFromName(ims::name::IPSet &ips, const std::string &name, ims::name::NameResolutionSourceType sourceType) {
	if (name.length() > 0xff) {
		// error...
		return false;
	}

	// prepare the command
	cmdLen = name.length() + 4;
	uint16_t tmpCmdLen = htons(cmdLen);
	memcpy(buf, &tmpCmdLen, sizeof(uint16_t));
	buf[2] = 0; // type, NameResolution
	buf[3] = 0; // function, getIPsFromName
	if (sourceType == ims::name::NameResolutionSourceType::ANY) {
		buf[4] = 0;
	} else if (sourceType == ims::name::NameResolutionSourceType::DNS) {
		buf[4] = 1;
	}
	buf[5] = (unsigned char) name.length();
	memcpy(buf + 6, name.c_str(), name.length());

	// issue the command
	if (socket_write(conn_fd, buf, cmdLen + 2, sockTimeout) != cmdLen + 2) {
		// something went wrong...reconnect, try one more time, then fail
		close(conn_fd);
		if (!establishConnection()) {
			return false;
		}
		if (socket_write(conn_fd, buf, cmdLen + 2, sockTimeout) != cmdLen + 2) {
			return false;
		}
	}
	// at this point, all is well, and command has been issued
	// read back the # of ips
	if (socket_read(conn_fd, (char *) &cmdLen, sizeof(uint16_t), sockTimeout) != sizeof(uint16_t)) {
		// error
		return false;
	}
	cmdLen = ntohs(cmdLen);
	// FIXME fill a buffer instead of reading one at a time
	for (uint16_t i = 0; i < cmdLen; ++i) {
		if (socket_read(conn_fd, (char *) &ip, sizeof(uint32_t), sockTimeout) != sizeof(uint32_t)) {
			return false;
		}
		ips.insert(ntohl(ip));
	}
	return true;
}

bool NameResolution::getNamesFromIP(ims::name::NameSet &names, const uint32_t &ip, ims::name::NameResolutionSourceType sourceType) {
	// prepare the command
	cmdLen = 7;
	uint16_t tmpCmdLen = htons(cmdLen);
	memcpy(buf, &tmpCmdLen, sizeof(uint16_t));
	buf[2] = 0; // type, NameResolution
	buf[3] = 1; // function, getNamesFromIP
	this -> ip = htonl(ip);
	if (sourceType == ims::name::NameResolutionSourceType::ANY) {
		buf[4] = 0;
	} else if (sourceType == ims::name::NameResolutionSourceType::DNS) {
		buf[4] = 1;
	}
	memcpy(buf + 5, &(this -> ip), sizeof(uint32_t)); // arg (ip)

	// issue the command
	if (socket_write(conn_fd, buf, cmdLen + 2, sockTimeout) != cmdLen + 2) {
		// something went wrong...reconnect, try one more time, then fail
		close(conn_fd);
		if (!establishConnection()) {
			return false;
		}
		if (socket_write(conn_fd, buf, cmdLen + 2, sockTimeout) != cmdLen + 2) {
			return false;
		}
	}
	// at this point, all is well, and command has been issued
	// read back the # of names
	if (socket_read(conn_fd, (char *) &cmdLen, sizeof(uint16_t), sockTimeout) != sizeof(uint16_t)) {
		// error
		return false;
	}
	cmdLen = ntohs(cmdLen);
	// FIXME fill a buffer instead of reading one at a time
	for (uint16_t i = 0; i < cmdLen; ++i) {
		if (socket_read(conn_fd, (char *) &nameLen, 1, sockTimeout) != 1) {
			return false;
		}
		if (socket_read(conn_fd, buf, nameLen, sockTimeout) != nameLen) {
			return false;
		}
		tmpName.assign(buf, nameLen);
		names.insert(tmpName);
	}
	return true;
}

bool NameResolution::hasMapping(bool &ret, const std::string &name, ims::name::NameResolutionSourceType sourceType) {
	if (name.length() > 0xff) {
		// error...
		return false;
	}

	// prepare the command
	cmdLen = name.length() + 4;
	uint16_t tmpCmdLen = htons(cmdLen);
	memcpy(buf, &tmpCmdLen, sizeof(uint16_t));
	buf[2] = 0; // type, NameResolution
	buf[3] = 2; // function, hasMappingName
	if (sourceType == ims::name::NameResolutionSourceType::ANY) {
		buf[4] = 0;
	} else if (sourceType == ims::name::NameResolutionSourceType::DNS) {
		buf[4] = 1;
	}
	buf[5] = (unsigned char) name.length();
	memcpy(buf + 6, name.c_str(), name.length());

	//std::cerr << "// issue the command\n";
	if (socket_write(conn_fd, buf, cmdLen + 2, sockTimeout) != cmdLen + 2) {
		//std::cerr << "// something went wrong...reconnect, try one more time, then fail\n";
		close(conn_fd);
		if (!establishConnection()) {
			return false;
		}
		if (socket_write(conn_fd, buf, cmdLen + 2, sockTimeout) != cmdLen + 2) {
			return false;
		}
	}
	// at this point, all is well, and command has been issued
	// read back the answer
	if (socket_read(conn_fd, buf, 1, sockTimeout) != 1) {
		// error
		return false;
	}
	ret = (buf[0] == 1)?true:false;

	return true;
}

bool NameResolution::hasMapping(bool &ret, const uint32_t &ip, ims::name::NameResolutionSourceType sourceType) {
	// prepare the command
	cmdLen = 7; //
	uint16_t tmpCmdLen = htons(cmdLen);
	memcpy(buf, &tmpCmdLen, sizeof(uint16_t));
	buf[2] = 0; // type, NameResolution
	buf[3] = 3; // function, hasMappingIP
	if (sourceType == ims::name::NameResolutionSourceType::ANY) {
		buf[4] = 0;
	} else if (sourceType == ims::name::NameResolutionSourceType::DNS) {
		buf[4] = 1;
	}
	this -> ip = htonl(ip);
	memcpy(buf + 5, &(this -> ip), sizeof(uint32_t)); // arg (ip)

	// issue the command
	if (socket_write(conn_fd, buf, cmdLen + 2, sockTimeout) != cmdLen + 2) {
		// something went wrong...reconnect, try one more time, then fail
		close(conn_fd);
		if (!establishConnection()) {
			return false;
		}
		if (socket_write(conn_fd, buf, cmdLen + 2, sockTimeout) != cmdLen + 2) {
			return false;
		}
	}
	// at this point, all is well, and command has been issued
	// read back the answer
	if (socket_read(conn_fd, buf, 1, sockTimeout) != 1) {
		// error
		return false;
	}
	ret = (buf[0] == 1)?true:false;

	return true;
}

bool NameResolution::getMappings(ims::name::NameIPMap &mappings, ims::name::NameResolutionSourceType sourceType) {
	// prepare the command
	cmdLen = 3;
	uint16_t tmpCmdLen = htons(cmdLen);
	memcpy(buf, &tmpCmdLen, sizeof(uint16_t));
	buf[2] = 0; // type, NameResolution
	buf[3] = 4; // function, getMappings
	if (sourceType == ims::name::NameResolutionSourceType::ANY) {
		buf[4] = 0;
	} else if (sourceType == ims::name::NameResolutionSourceType::DNS) {
		buf[4] = 1;
	}
	
	// issue the command
	if (socket_write(conn_fd, buf, cmdLen + 2, sockTimeout) != cmdLen + 2) {
		// something went wrong...reconnect, try one more time, then fail
		close(conn_fd);
		if (!establishConnection()) {
			return false;
		}
		if (socket_write(conn_fd, buf, cmdLen + 2, sockTimeout) != cmdLen + 2) {
			return false;
		}
	}
	// at this point, all is well, and command has been issued
	// read back the # of mappings
	uint32_t count32;
	if (socket_read(conn_fd, (char *) &count32, sizeof(uint32_t), sockTimeout) != sizeof(uint32_t)) {
		// error
		return false;
	}
	count32 = ntohl(count32);
	// FIXME fill a buffer instead of reading one at a time
	for (uint32_t i = 0; i < count32; ++i) {
		if (socket_read(conn_fd, (char *) &nameLen, 1, sockTimeout) != 1) {
			return false;
		}
		if (socket_read(conn_fd, buf, nameLen, sockTimeout) != nameLen) {
			return false;
		}
		tmpName.assign(buf, nameLen);
		if (socket_read(conn_fd, (char *) &ip, sizeof(uint32_t), sockTimeout) != sizeof(uint32_t)) {
			return false;
		}
		mappings.insert(std::make_pair(tmpName, ntohl(ip)));
	}
	return true;
}

bool NameResolution::getResolvedIPs(ims::name::IPSet &resolvedIPs, ims::name::NameResolutionSourceType sourceType) {
	// prepare the command
	cmdLen = 3;
	uint16_t tmpCmdLen = htons(cmdLen);
	uint32_t count32;
	memcpy(buf, &tmpCmdLen, sizeof(uint16_t));
	buf[2] = 0; // type, NameResolution
	buf[3] = 5; // function, getResolvedIPs
	if (sourceType == ims::name::NameResolutionSourceType::ANY) {
		buf[4] = 0;
	} else if (sourceType == ims::name::NameResolutionSourceType::DNS) {
		buf[4] = 1;
	}

	// issue the command
	if (socket_write(conn_fd, buf, cmdLen + 2, sockTimeout) != cmdLen + 2) {
		// something went wrong...reconnect, try one more time, then fail
		close(conn_fd);
		if (!establishConnection()) {
			return false;
		}
		if (socket_write(conn_fd, buf, cmdLen + 2, sockTimeout) != cmdLen + 2) {
			return false;
		}
	}
	// at this point, all is well, and command has been issued
	// read back the # of ips
	if (socket_read(conn_fd, (char *) &count32, sizeof(count32), sockTimeout) != sizeof(count32)) {
		// error
		return false;
	}
	count32 = ntohl(count32);
	// FIXME fill a buffer instead of reading one at a time
	for (uint32_t i = 0; i < count32; ++i) {
		if (socket_read(conn_fd, (char *) &ip, sizeof(uint32_t), sockTimeout) != sizeof(uint32_t)) {
			return false;
		}
		resolvedIPs.insert(ntohl(ip));
	}
	return true;
}

bool NameResolution::hasHostResolvedIP(bool &ret, const uint32_t &host, const uint32_t &resolvedIP, const TimeStamp &t, ims::name::NameResolutionSourceType sourceType) {
	// prepare the command
	cmdLen = 19; //
	uint16_t tmpCmdLen = htons(cmdLen);
	memcpy(buf, &tmpCmdLen, sizeof(uint16_t));
	buf[2] = 0; // type, NameResolution
	buf[3] = 6; // function, hasHostResolvedIP
	if (sourceType == ims::name::NameResolutionSourceType::ANY) {
		buf[4] = 0;
	} else if (sourceType == ims::name::NameResolutionSourceType::DNS) {
		buf[4] = 1;
	}
	this -> ip = htonl(host);
	memcpy(buf + 5, &(this -> ip), sizeof(uint32_t)); // arg (host)
	this -> ip = htonl(resolvedIP);
	memcpy(buf + 9, &(this -> ip), sizeof(uint32_t)); // arg (resolvedIP)
	this -> ip = htonl(t.seconds());
	memcpy(buf + 13, &(this -> ip), sizeof(uint32_t)); // arg (seconds)
	this -> ip = htonl(t.microseconds());
	memcpy(buf + 17, &(this -> ip), sizeof(uint32_t)); // arg (microseconds)

	// issue the command
	if (socket_write(conn_fd, buf, cmdLen + 2, sockTimeout) != cmdLen + 2) {
		// something went wrong...reconnect, try one more time, then fail
		close(conn_fd);
		if (!establishConnection()) {
			return false;
		}
		if (socket_write(conn_fd, buf, cmdLen + 2, sockTimeout) != cmdLen + 2) {
			return false;
		}
	}
	// at this point, all is well, and command has been issued
	// read back the answer
	if (socket_read(conn_fd, buf, 1, sockTimeout) != 1) {
		// error
		return false;
	}
	ret = (buf[0] == 1)?true:false;

	return true;
}

bool NameResolution::getLastResolutionTime(TimeStamp &ret, const uint32_t &host, const uint32_t &resolvedIP, const TimeStamp &t, ims::name::NameResolutionSourceType sourceType) {
	// prepare the command
	cmdLen = 19; //
	uint16_t tmpCmdLen = htons(cmdLen);
	memcpy(buf, &tmpCmdLen, sizeof(uint16_t));
	buf[2] = 0; // type, NameResolution
	buf[3] = 7; // function, getLastResolutionTime
	if (sourceType == ims::name::NameResolutionSourceType::ANY) {
		buf[4] = 0;
	} else if (sourceType == ims::name::NameResolutionSourceType::DNS) {
		buf[4] = 1;
	}
	this -> ip = htonl(host);
	memcpy(buf + 5, &(this -> ip), sizeof(uint32_t)); // arg (host)
	this -> ip = htonl(resolvedIP);
	memcpy(buf + 9, &(this -> ip), sizeof(uint32_t)); // arg (resolvedIP)
	this -> ip = htonl(t.seconds());
	memcpy(buf + 13, &(this -> ip), sizeof(uint32_t)); // arg (seconds)
	this -> ip = htonl(t.microseconds());
	memcpy(buf + 17, &(this -> ip), sizeof(uint32_t)); // arg (microseconds)

	// issue the command
	if (socket_write(conn_fd, buf, cmdLen + 2, sockTimeout) != cmdLen + 2) {
		// something went wrong...reconnect, try one more time, then fail
		close(conn_fd);
		if (!establishConnection()) {
			return false;
		}
		if (socket_write(conn_fd, buf, cmdLen + 2, sockTimeout) != cmdLen + 2) {
			return false;
		}
	}
	// at this point, all is well, and command has been issued
	// read back the answer
	if (socket_read(conn_fd, (char *) &ret, sizeof(ret), sockTimeout) != sizeof(ret)) {
		// error
		return false;
	}
	ret.set(ntohl(ret.seconds()), ntohl(ret.microseconds()));

	return true;
}

bool NameResolution::clear(ims::name::NameResolutionSourceType sourceType) {
	// prepare the command
	cmdLen = 3;
	uint16_t tmpCmdLen = htons(cmdLen);
	memcpy(buf, &tmpCmdLen, sizeof(uint16_t));
	buf[2] = 0; // type, NameResolution
	buf[3] = -1; // function, clear
	if (sourceType == ims::name::NameResolutionSourceType::ANY) {
		buf[4] = 0;
	} else if (sourceType == ims::name::NameResolutionSourceType::DNS) {
		buf[4] = 1;
	}

	// issue the command
	if (socket_write(conn_fd, buf, cmdLen + 2, sockTimeout) != cmdLen + 2) {
		// something went wrong...reconnect, try one more time, then fail
		close(conn_fd);
		if (!establishConnection()) {
			return false;
		}
		if (socket_write(conn_fd, buf, cmdLen + 2, sockTimeout) != cmdLen + 2) {
			return false;
		}
	}
	
	if (socket_read(conn_fd, buf, 1, sockTimeout) != 1) {
		// error
		return false;
	}

	return true;
}

bool NameResolution::load(const TimeStamp &begin, const TimeStamp &end, ims::name::NameResolutionSourceType sourceType) {
	// prepare the command
	cmdLen = 19; //
	uint16_t tmpCmdLen = htons(cmdLen);
	memcpy(buf, &tmpCmdLen, sizeof(uint16_t));
	buf[2] = 0; // type, NameResolution
	buf[3] = -2; // function, load
	if (sourceType == ims::name::NameResolutionSourceType::ANY) {
		buf[4] = 0;
	} else if (sourceType == ims::name::NameResolutionSourceType::DNS) {
		buf[4] = 1;
	}
	this -> ip = htonl(begin.seconds());
	memcpy(buf + 5, &(this -> ip), sizeof(uint32_t)); // arg (t)
	this -> ip = htonl(begin.microseconds());
	memcpy(buf + 9, &(this -> ip), sizeof(uint32_t)); // arg (t)
	this -> ip = htonl(end.seconds());
	memcpy(buf + 13, &(this -> ip), sizeof(uint32_t)); // arg (t)
	this -> ip = htonl(end.microseconds());
	memcpy(buf + 17, &(this -> ip), sizeof(uint32_t)); // arg (t)

	// issue the command
	if (socket_write(conn_fd, buf, cmdLen + 2, sockTimeout) != cmdLen + 2) {
		// something went wrong...reconnect, try one more time, then fail
		close(conn_fd);
		if (!establishConnection()) {
			return false;
		}
		if (socket_write(conn_fd, buf, cmdLen + 2, sockTimeout) != cmdLen + 2) {
			return false;
		}
	}

	if (socket_read(conn_fd, buf, 1, sockTimeout) != 1) {
		// error
		return false;
	}

	return true;
}

bool NameResolution::waitForData(ims::name::NameResolutionSourceType sourceType) {
	// prepare the command
	cmdLen = 3;
	uint16_t tmpCmdLen = htons(cmdLen);
	memcpy(buf, &tmpCmdLen, sizeof(uint16_t));
	buf[2] = 0; // type, NameResolution
	buf[3] = -3; // function, waitForData
	if (sourceType == ims::name::NameResolutionSourceType::ANY) {
		buf[4] = 0;
	} else if (sourceType == ims::name::NameResolutionSourceType::DNS) {
		buf[4] = 1;
	}

	// issue the command
	if (socket_write(conn_fd, buf, cmdLen + 2, sockTimeout) != cmdLen + 2) {
		// something went wrong...reconnect, try one more time, then fail
		close(conn_fd);
		if (!establishConnection()) {
			return false;
		}
		if (socket_write(conn_fd, buf, cmdLen + 2, sockTimeout) != cmdLen + 2) {
			return false;
		}
	}
	
	do {
		if (socket_read(conn_fd, buf, 1, sockTimeout) != 1) {
			// error
			return false;
		}
	} while (buf[0] == 0);

	return true;
}

bool NameResolution::getConfiguration(ims::QueryManagerConfiguration &queryManConfig) {
	// prepare the command
	cmdLen = 3;
	uint16_t tmpCmdLen = htons(cmdLen);
	memcpy(buf, &tmpCmdLen, sizeof(uint16_t));
	buf[2] = 0; // type, NameResolution
	buf[3] = -4; // function, getConfiguration
	buf[4] = 0;

	// issue the command
	if (socket_write(conn_fd, buf, cmdLen + 2, sockTimeout) != cmdLen + 2) {
		// something went wrong...reconnect, try one more time, then fail
		close(conn_fd);
		if (!establishConnection()) {
			return false;
		}
		if (socket_write(conn_fd, buf, cmdLen + 2, sockTimeout) != cmdLen + 2) {
			return false;
		}
	}
	
	uint32_t count32, tmp32;
	std::string tmpStr;
	// read all dbHomes...
	// number of dbHomes...
	if (socket_read(conn_fd, (char *) &count32, sizeof(count32), sockTimeout) != sizeof(count32)) {
		// error
		return false;
	}
	tmp32 = ntohl(count32);

	for (uint32_t i = 0; i < tmp32; ++i) {
		// dbHome.size()
		if (socket_read(conn_fd, (char *) &count32, sizeof(count32), sockTimeout) != sizeof(count32)) {
			// error
			return false;
		}
		count32 = ntohl(count32);

		// dbHome
		if (count32 <= 0xffff) {
			if (socket_read(conn_fd, buf, count32, sockTimeout) != count32) {
				return false;
			}
			tmpStr.assign(buf, count32);
		} else {
			if (socket_read(conn_fd, buf, 0xffff, sockTimeout) != 0xffff) {
				return false;
			}
			tmpStr.assign(buf, 0xffff);
			
			count32 -= 0xffff;
			if (socket_read(conn_fd, buf, count32, sockTimeout) != count32) {
				return false;
			}
			tmpStr.append(buf, count32);
		}
		queryManConfig.dbHomes.push_back(tmpStr);
	}

	// sockPath.size()
	if (socket_read(conn_fd, (char *) &count32, sizeof(count32), sockTimeout) != sizeof(count32)) {
		// error
		return false;
	}
	count32 = ntohl(count32);

	// sockPath
	if (count32 <= 0xffff) {
		if (socket_read(conn_fd, buf, count32, sockTimeout) != count32) {
			return false;
		}
		queryManConfig.sockPath.assign(buf, count32);
	} else {
		if (socket_read(conn_fd, buf, 0xffff, sockTimeout) != 0xffff) {
			return false;
		}
		queryManConfig.sockPath.assign(buf, 0xffff);
		
		count32 -= 0xffff;
		if (socket_read(conn_fd, buf, count32, sockTimeout) != count32) {
			return false;
		}
		queryManConfig.sockPath.append(buf, count32);
	}

	// sockTimeout
	if (socket_read(conn_fd, (char *) &count32, sizeof(count32), sockTimeout) != sizeof(count32)) {
		// error
		return false;
	}
	queryManConfig.sockTimeout = ntohl(count32);

	return true;
}
