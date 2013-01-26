/* 
 * File:   HIEApplication.cpp
 * Author: Mikhail Sosonkin
 * 
 * Created on February 4, 2010, 5:08 PM
 */

#include "HIEApplication.h"
#include <iostream>

namespace vn {
    namespace arl {
        namespace symptom {

            using namespace std;

            HIEApplication::HIEApplication(const HIEApplication& orig):
                addr(orig.addr),
                port(orig.port),
                name(orig.name),
                timestamp(orig.timestamp) {
            }

            HIEApplication::HIEApplication(const address& _addr, int _port):
                addr(_addr), port(_port) {
            }

            HIEApplication::HIEApplication(const string& _name, int _port):
                name(_name), port(_port) {
            }

            HIEApplication::HIEApplication(const TimeStamp& _timestamp, const address& _addr, int _port):
                addr(_addr), port(_port), timestamp(_timestamp) {
            }

            HIEApplication::HIEApplication(const TimeStamp& _timestamp, const string& _name, int _port):
                name(_name), port(_port), timestamp(_timestamp) {
            }

            const string& HIEApplication::getName() const {
                return name;
            }

            const TimeStamp& HIEApplication::getTimeStamp() const {
                return timestamp;
            }

            const address& HIEApplication::getAddress() const {
                return addr;
            }

            void HIEApplication::setAddress(const address& _addr) {
                addr = _addr;
            }

            int HIEApplication::getPort() const {
                return port;
            }

            bool HIEApplication::operator==(const HIEApplication &rhs) const {
                return (this->port == rhs.port) && (this->addr == rhs.addr);
            }

            bool HIEApplication::operator!=(const HIEApplication &rhs) const {
                return !(*this == rhs);
            }
        }
    }
}
