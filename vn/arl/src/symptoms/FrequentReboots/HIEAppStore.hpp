/*
 * File:   HIEAppStore.hpp
 * Author: Mikhail Sosonkin
 *
 * Created on February 6, 2010
 */

#ifndef _HIE_APP_STORE_HPP
#define _HIE_APP_STORE_HPP

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <boost/filesystem.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>

#include "HIEApplication.h"
#include "../../shared/pmap.hpp"

namespace vn {
    namespace arl {
        namespace symptom {
            
            using namespace std;
            using namespace vn::arl::shared;
            using namespace boost::asio::ip;

            typedef pair<HIEApplication, string> entry_type;
            typedef vector<entry_type> store_type;
            typedef pmap<int, address> dns_map_type;

            class HIEAppStore : public SynappConfiguration {
            public:

                explicit HIEAppStore(const boost::filesystem::path &filename,
                                     const boost::filesystem::path &dnsState) : SynappConfiguration(filename) {

                    const char* fname = dnsState.string().c_str();

                    int seg_size = 25 * 1024 * 1024; //25MB
                    if(!dns_map.create(fname, seg_size)) {
                        if(!dns_map.extend(fname, seg_size)) {
                            _error = true;
                            _errorMsg.assign("Unable to open dns map file: ");
                            _errorMsg.append(fname);

                            return;
                        }
                    }

                    setOptionsDescriptions();
                    parseOptions();
                }

                ~HIEAppStore() {
                    dns_map.close();
                }

                bool errored() {
                    return _error;
                }

                int size() {
                    return store.size();
                }

                bool contains(const HIEApplication& app) const {
                    return getComment(app) != end();
                }

                void storeDNSMapping(const string& name, const address& ip) {
                    store_type::iterator it = store.begin();
                    while(it != store.end()) {
                        HIEApplication& check = it->first;

                        if(dnsCompare(check.getName(), name) == 0) {
                            check.setAddress(ip);

                            int hash = stringHashCode(check.getName());
                            dns_map.insert(dns_map_type::value_type(hash, ip));
                        }

                        it++;
                    }
                }

                int dnsCompare(const string& str1, const string& str2) {
                    string comp1 = (str1[str1.size() - 1] == '.'?str1.substr(0, str1.size() - 1):str1);
                    string comp2 = (str2[str2.size() - 1] == '.'?str2.substr(0, str2.size() - 1):str2);

                    return comp1.compare(comp2);
                }

                unsigned int stringHashCode(const string& str) {
                    unsigned int hash = 0;
                    const char* data = str.c_str();
                    int count = str.size();

                    for(int i = 0; i < count; i++) {
                        hash = 31*hash + data[i];
                    }

                    return hash;
                }

                const store_type::const_iterator getComment(const HIEApplication& app) const {
                    store_type::const_iterator it = store.begin();
                    while(it != store.end()) {
                        const HIEApplication& check = it->first;

                        if(check == app) {
                            return it;
                        }

                        it++;
                    }

                    return store.end();
                }

                const store_type::const_iterator end() const {
                    return store.end();
                }

            private:
                store_type store;
                dns_map_type dns_map;

                virtual void setOptionsDescriptions() {
                    options.add_options()
                        (
                        "Application",
                        boost::program_options::value<vector<string> > (),
                        "Application definition"
                        )
                        (
                        "Application_dns",
                        boost::program_options::value<vector<string> > (),
                        "Application definition using a DNS String"
                        );
                }

                virtual void parseOptions() {
                    this->allowUnregistered = true;
                    boost::program_options::variables_map vals;
                    if (!_parseOptions(vals)) {
                        return;
                    }

                    string ipField = "Application";
                    string dnsField = "Application_dns";

                    if(vals.count(ipField)) {
                        vector<string> arr = vals[ipField].as<vector<string> >();
                        vector<string>::iterator it = arr.begin();

                        while(it != arr.end()) {
                            string ip, comment;
                            int port;
                            address addr;
                            istringstream ss(*it);

                            try {
                                ss >> ip >> port >> comment;
                                addr = boost::asio::ip::address::from_string(ip);
                            } catch ( ... ) {
                                _error = true;
                                _errorMsg.assign("Error parsing application definition file at: ");
                                _errorMsg.append(*it);
                                _errorMsg.append(". Format is: [ip] [port] [comment]");
                                return;
                            }

                            HIEApplication app(addr, port);
                            entry_type par(app, comment);

                            store.push_back(par);

                            it++;
                        }
                    }

                    if(vals.count(dnsField)) {
                        vector<string> arr = vals[dnsField].as<vector<string> >();
                        vector<string>::iterator it = arr.begin();
                        
                        while(it != arr.end()) {
                            string name, comment;
                            int port;
                            istringstream ss(*it);

                            try {
                                ss >> name >> port >> comment;
                            } catch ( ... ) {
                                _error = true;
                                _errorMsg.assign("Error parsing application definition file at: ");
                                _errorMsg.append(*it);
                                _errorMsg.append(". Format is: [ip] [port] [comment]");
                                return;
                            }

                            HIEApplication app(name, port);

                            // here we need to get stuff from the map.
                            int nameHash = stringHashCode(name);
                            dns_map_type::const_iterator addrIt = dns_map.find(nameHash);
                            if(addrIt != dns_map.end()) {
                                const address& addr = addrIt->second;

                                app.setAddress(addr);
                            }

                            entry_type par(app, comment);
                            store.push_back(par);

                            it++;
                        }
                    }

                }
            };
        }
    }
}

#endif