#include <sstream>
#include <assert.h>

#include "domainSuffix.h"
#include "getHTTPData.h"

DomainSuffix::DomainSuffix() {
	exceptionMaxLevel = 0;
	suffixMaxLevel = 0;
}

bool DomainSuffix::cmpDots(std::string s1, std::string s2) {
	size_t d1(0), d2(0);

	for (size_t i = 0; i < s1.length(); ++i) {
		if (s1[i] == '.') {
			++d1;
		}
	}

	for (size_t i = 0; i < s2.length(); ++i) {
		if (s2[i] == '.') {
			++d2;
		}
	}

	return d1 > d2;
}

bool DomainSuffix::initialize(std::string uri) {
	// parse uri: [http://]host[:port][path]
	size_t pos;
	std::string suffixFileHost;
	std::string suffixFilePath;
	uint16_t suffixFilePort;

	if (uri.length() > 7 && uri.compare(0, 7, "http://", 7) == 0) {
		uri = uri.substr(7, uri.length() - 7);
	}

	pos = uri.find('/', 0);
	if (pos == std::string::npos) {
		suffixFileHost.assign(uri);
		suffixFilePath.assign("/");
	} else {
		suffixFileHost.assign(uri, 0, pos);
		suffixFilePath.assign(uri, pos, uri.length() - pos);
	}

	pos = suffixFileHost.find(':', 0);
	if (pos == std::string::npos) {
		suffixFilePort = 80;
	} else {
		suffixFilePort = (uint16_t) atoi(suffixFileHost.c_str() + pos + 1);
		suffixFileHost = suffixFileHost.substr(0, pos);
	}

	// fetch suffix list
	std::stringstream suffixFileStream;
	if (!getHTTPData(suffixFileHost.c_str(), suffixFilePort, suffixFilePath.c_str(), suffixFileStream)) {
		_error.assign("DomainSuffix: initialize(): Failed to fetch suffix list from \"");
		_error.append(uri + '"');
		//cerr << "DEBUG: host: " << suffixFileHost << endl
		//		 << "	   port: " << suffixFilePort << endl
		//		 << "	   path: " << suffixFilePath << endl;
		return false;
	}

	return initialize(suffixFileStream);

	return false;
}

bool DomainSuffix::initialize(std::istream &stream) {
	if (!stream) {
		_error.assign("DomainSuffix: initialize(): Bad I/O stream.");
		return false;
	}

	std::string s, t;
	size_t dotCount, pos;

	while (getline(stream, s)) {
		if (s.length() == 0 || isspace(s[0])) {
			// blank line, or whitespace starting the line
			continue;
		}

		if (s.length() >= 2 && s[0] == '/' && s[1] == '/') {
			// commented line
			continue;
		}

		if (s[0] == '!') {
			// exception domain
			if (s[1] == '.') {
				t = s.substr(2);
			} else {
				t = s.substr(1);
			}

			domainExceptions.insert(t);
			dotCount = count(t.begin(), t.end(), '.');
			if (dotCount > exceptionMaxLevel) {
				exceptionMaxLevel = dotCount;
			}

			continue;
		}

		if (s[0] == '.') {
			//t = s.substr(1);
			pos = 1;
		} else {
			//t = s;
			pos = 0;
		}

		if (s[pos] == '*') {
			assert(s[pos + 1] == '.');

			domainSuffixWildcards.insert(s.substr(pos + 2));
		} else if (pos != 0) {
			domainSuffixes.insert(s.substr(pos));
		} else {
			domainSuffixes.insert(s);
		}

		dotCount = count(s.begin() + pos, s.end(), '.');
		if (dotCount > exceptionMaxLevel) {
			suffixMaxLevel = dotCount;
		}
	}

	// level is #of dots + 1
	++exceptionMaxLevel;

	//sort(domainSuffixes.begin(), domainSuffixes.end(), cmpDots);

	return true;
}

size_t DomainSuffix::getDomainPos(std::string d) {
	// TODO case insensitive string implementation
	for (size_t i = 0; i < d.length(); d[i] = tolower(d[i]), ++i);

	// check exceptions
	size_t curPos, curLevel;

	curPos = d.rfind('.');
	for (curLevel = 1; curLevel < exceptionMaxLevel; ++curLevel) {
		curPos = d.rfind('.', curPos - 1);

		if (domainExceptions.find(d.substr(curPos + 1)) != domainExceptions.end()) {
			return curPos + 1;
		}

		if (curPos == std::string::npos) {
			break;
		}
	}

	// check suffixes
	std::vector <size_t> startPos;
	curPos = d.rfind('.');
	startPos.push_back(curPos + 1);
	for (curLevel = suffixMaxLevel;
		 curLevel > 0 && curPos != std::string::npos;
		 --curLevel)
	{
		curPos = d.rfind('.', curPos - 1);
		startPos.push_back(curPos + 1);
	}

	for (size_t i = startPos.size() - 1; i > 0; --i) {
		//std::cout << "Checking: " << d.substr(startPos.at(i)) << std::endl;
		if (domainSuffixes.find(d.substr(startPos.at(i))) != domainSuffixes.end()) {
			// startPos.at(i) is where the suffix starts. startPos.at(i - 1) is lower level, .at(i + 1) is higher level
			if (i < startPos.size() - 1) {
				return startPos.at(i + 1);
			} else {
				return d.rfind('.', startPos.at(i) - 2);
			}
		}

		//std::cout << "Checking wildcard: " << d.substr(startPos.at(i - 1)) << std::endl;
		if (domainSuffixWildcards.find(d.substr(startPos.at(i - 1))) != domainSuffixWildcards.end()) {
			//return d.rfind('.', d.rfind('.', startPos.at(i - 1) - 2) - 1) + 1;
			if (i < startPos.size() - 1) {
				return startPos.at(i + 1);
			} else {
				if (startPos.at(i) != 0) {
					return d.rfind('.', startPos.at(i) - 2);
				} else {
					return std::string::npos;
				}
			}
		}
	}
	//std::cout << "Checking final suffix." << std::endl;
	if (domainSuffixes.find(d.substr(startPos.at(0))) != domainSuffixes.end()) {
		if (startPos.at(0) != 0) {
			return d.rfind('.', startPos.at(0) - 2) + 1;
		} else {
			return std::string::npos;
		}
	}

	// invalid domain name...
	return std::string::npos;
}

const std::tr1::unordered_set <std::string> & DomainSuffix::getDomainExceptions() const {
	return domainExceptions;
}

const std::tr1::unordered_set <std::string> & DomainSuffix::getDomainSuffixes() const {
	return domainSuffixes;
}

const std::tr1::unordered_set <std::string> & DomainSuffix::getDomainSuffixWildcards() const {
	return domainSuffixWildcards;
}

std::string DomainSuffix::error() const {
	return _error;
}
