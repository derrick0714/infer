#ifndef BRUTEHOSTPAIR_HPP
#define BRUTEHOSTPAIR_HPP

#include "hostPair.hpp"
#include "modules.hpp"

#define BRUTE_FORCERS_SCHEMA_NAME "BruteForcers"
#define BRUTE_FORCERS_TABLE_SCHEMA "\"sourceIP\" uint32 NOT NULL, \
                                    \"destinationIP\" uint32 NOT NULL, \
                                    \"destinationPort\" uint16 NOT NULL, \
                                    \"numAttempts\" uint32 NOT NULL, \
				    \"numBytes\" uint32 NOT NULL,\
                                    \"startTime\" uint32 NOT NULL, \
                                    \"endTime\" uint32 NOT NULL, \
                                    \"asNumber\" uint16 NOT NULL, \
                                    \"countryNumber\" SMALLINT NOT NULL, \
                                    PRIMARY KEY (\"sourceIP\", \"destinationIP\", \
                                                               \"destinationPort\",\"startTime\", \
                                                               \"endTime\")"

class BruteHostPair {
public:
    	uint8_t protocol;
    	uint32_t internalIP;
    	uint32_t externalIP;
    	uint16_t internalPort;
    	uint16_t externalPort;
	BruteHostPair();
	BruteHostPair(uint8_t &_protocol, uint32_t &_internalIP,uint32_t &_externalIP);
	inline bool operator< (const BruteHostPair &rhs) const {
      		if (protocol != rhs.protocol) {
        		return (protocol < rhs.protocol);
      		}
      		else {
        		if (internalIP != rhs.internalIP) {
          			return (internalIP < rhs.internalIP);
        		}
        		else {
          			if (externalIP != rhs.externalIP) {
            				return (externalIP < rhs.externalIP);
          			}
              			else {
                			return false;
              			}
        		}
      		}
    	}
	inline void operator= (const TwoWayHostPair &rhs){
		protocol=rhs.protocol;
		internalIP=rhs.internalIP;
		externalIP=rhs.externalIP;
		internalPort=rhs.internalPort;
		externalPort=rhs.externalPort;
	}
} __attribute__ ((packed));
 	
inline BruteHostPair::BruteHostPair(){};

inline BruteHostPair::BruteHostPair(uint8_t &_protocol, uint32_t &_internalIP,uint32_t &_externalIP) {
      		protocol = _protocol;
      		internalIP = _internalIP;
      		externalIP = _externalIP;
}


///BruteForcers Class stores details for brute IP's such as number of Initializations,
///start time, end time,first start time, last end time, duration and type of initiator.
class BruteForcers{
  public:
        uint32_t numInits;  ///<Stores number of TCP handshakes for each IP
        TimeStamp startTime; ///<Stores start Time of TCP handshakes
        TimeStamp endTime;   ///<Stores end Time of TCP handshakes
        TimeStamp duration;  ///<Calculated minimum duration for each Host Pair
        TimeStamp avgDuration; ///Average duaration for  each Host Pair
	uint32_t firstStartTime; ///<Stores first occurance of an SSH bruteforce attack
        uint32_t lastEndTime; ///<Stores last occurance of SSH bruteforce attack
        uint32_t numBytes;  ///<Stores the amount of data transferred between each host
        int8_t typeInit;   ///<Indicates whether the TCP initiator was internal/external
        double bytesInitsRatio; ///<Stores correlation of data transferred to number of initializations
	BruteForcers();
};

///Default Constructor for Member's of BruteForcers Class
inline BruteForcers::BruteForcers(){
                numInits=0;
                startTime.set(0, 0);
                endTime.set(0, 0);
                duration.set(0, 0);
		avgDuration.set(0, 0);
                firstStartTime=std::numeric_limits<uint32_t>::max();
                lastEndTime=0;
                numBytes=0;
                typeInit=TEMPORARILY_UNKNOWN_INITIATOR;
                bytesInitsRatio=0;
}


inline bool isFTPPort(uint16_t port){
        return (port == 21);
}

inline bool isSSHPort(uint16_t port){
        return (port == 22);
}

inline bool isTELNETPort(uint16_t port){
        return (port == 23);
}

//MS SQL - 1433, Oracle SQL - 1521, MySQL - 3306, PostGreSQL - 5432
inline bool isSQLPort(uint16_t port){
        return (port == 1433 || 
                port == 1521 || 
                port == 3306 || 
                port == 5432 );
}

#endif
