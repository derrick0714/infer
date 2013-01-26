#include "dnsRecords.hpp"

DNSRecords::DNSRecords(bidirectional_unordered_multimap <uint32_t, std::string>::type *records)
	: records(records)
{
}

void DNSRecords::nameLookup(std::string name, std::vector <uint32_t> &ips) const {
    ips.clear();
    boost::multi_index::nth_index<bidirectional_unordered_multimap <uint32_t, std::string>::type, 1>::type::iterator iter1, iter2;

    boost::tie(iter1, iter2) = boost::multi_index::get<1>(*records).equal_range(name);
    while (iter1 != iter2) {
	ips.push_back(iter1 -> first);
	++iter1;
    }
}

void DNSRecords::ipLookup(uint32_t ip, std::vector <std::string> &names) const {
    names.clear();
    boost::multi_index::nth_index<bidirectional_unordered_multimap <uint32_t, std::string>::type, 0>::type::iterator iter1, iter2;

    boost::tie(iter1, iter2) = boost::multi_index::get<0>(*records).equal_range(ip);
    while (iter1 != iter2) {
	names.push_back(iter1 -> second);
	++iter1;
    }
}

bidirectional_unordered_multimap <uint32_t, std::string>::type * DNSRecords::getMultimap() {
    return records;
}
