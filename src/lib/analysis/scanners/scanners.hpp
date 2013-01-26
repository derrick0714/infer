#ifndef SCANNERS_HPP
#define SCANNERS_HPP

const double theta0=0.20;  ///<Pr[Y=successful/H0] where H0 is the hypothesis that remote source is benign.
const double theta1=0.80;  ///<Pr[Y=successful/H1] where H1 is the hypothesis that remote source is malicious

/// \brief Container to store aggregate connection stats for each sourceIP initiating TCP connections
class ConnectionStats{
public:
        uint32_t successful;  ///<Stores number of successful TCP connections
        uint32_t failed;      ///<Stores number of failed TCP connections
        uint32_t startTime;
        uint32_t endTime;
        uint32_t numPackets;
        uint32_t numBytes;
        uint32_t failedSYNs;
	uint32_t failedACKs;
	uint32_t failedFINs;
        uint32_t failedXMASs;
        std::string scanType;
        ConnectionStats();
};

inline ConnectionStats::ConnectionStats(){
        successful=0;
        failed=0;
        startTime=std::numeric_limits<uint32_t>::max();
        endTime=0;
        numPackets=0;
        numBytes=0; 
        failedSYNs=0;
        failedACKs=0;
        failedFINs=0;
        failedXMASs=0;
        scanType.clear();
}


/// \brief Container used to store current state of tcp connection streams
class States{
public:
        bool handshake;     ///<Inidcates if three way handshake was seen 
        int8_t initiator;   ///<Stores the initiator of connection stream
        uint32_t firstSYNTime;  ///<Stores the time first SYN was seen for the connection
        uint32_t startTime;
        uint32_t endTime;
        uint32_t tcpSYNs;
        uint32_t tcpACKs;
        uint32_t tcpFINs;
        uint32_t tcpRSTs;
        uint32_t tcpURGs;
        uint32_t tcpPUSHs;
        uint32_t numFlows;
        uint32_t numBytes;
        uint32_t numPackets;
        States();
};

inline States::States(){
        handshake = false;
        initiator=TEMPORARILY_UNKNOWN_INITIATOR;
        firstSYNTime=std::numeric_limits<uint32_t>::max();
        startTime=std::numeric_limits<uint32_t>::max();
        endTime=0;
        tcpSYNs=0;
        tcpACKs=0;
        tcpFINs=0;
        tcpRSTs=0;
        tcpURGs=0;
        tcpPUSHs=0;
        numFlows=0;
        numBytes=0;
        numPackets=0;
}


/// \brief Container used to store destination Ports and IPs.
class Destination{
public:
        uint32_t destinationIP;
        uint16_t destinationPort;
        Destination();
        Destination(const uint32_t &_destinationIP, const uint16_t &_destinationPort);
	inline bool operator<(const Destination &rhs) const {
          if (destinationIP != rhs.destinationIP) {
            return (destinationIP < rhs.destinationIP);
          }
          else {
            if (destinationPort != rhs.destinationPort) {
              return (destinationPort < rhs.destinationPort);
            }
            else{
              return false;
            }
          }
        }
};

inline Destination::Destination(){
        destinationIP=0;
        destinationPort=0;
}

inline Destination::Destination(const uint32_t &_destinationIP, const uint16_t &_destinationPort){
        destinationIP = _destinationIP;
        destinationPort = _destinationPort;
}

/// \ brief Function to return Destination Pair
inline Destination makeDestinationPair(const uint32_t &destinationIP,const uint16_t &destinationPort){
	return Destination(destinationIP, destinationPort);
}


/*
 * /// \brief Function to compute sequential Hypothesis for each granular IP 
 * /// \param succesful represents the number of succesful connections made
 * /// \param failed represents the number of failed connections made
 * /// \returns the likelihood ratio which determines the model as benign or malicious,
 *      value between 0 and 1 and if value exceeds 0.99 it is deemed malicious
 */  
inline double seqHypothesis (uint32_t &successful, uint32_t &failed){
  double likelihoodRatio=0.01;  ///<Initial hypothesis that all hosts are benign
  likelihoodRatio*=pow((theta0/theta1),static_cast<double>(successful));
  likelihoodRatio*=pow((theta1/theta0),static_cast<double>(failed));
  return likelihoodRatio;
}

#endif
