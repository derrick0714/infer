#ifndef DOMAIN_SUFFIX_H
#define DOMAIN_SUFFIX_H

#include <iostream>
#include <string>
#include <tr1/unordered_set>
#include <vector>

class DomainSuffix {
  public:
	DomainSuffix();

	bool initialize(std::string uri);
	bool initialize(std::istream &stream);
	size_t getDomainPos(std::string d);

	const std::tr1::unordered_set <std::string> & getDomainExceptions() const;
	const std::tr1::unordered_set <std::string> & getDomainSuffixes() const;
	const std::tr1::unordered_set <std::string> & getDomainSuffixWildcards() const;

	std::string error() const;

  private:
	static bool cmpDots(std::string s1, std::string s2);

	std::tr1::unordered_set <std::string> domainExceptions;
	std::tr1::unordered_set <std::string> domainSuffixes;
	std::tr1::unordered_set <std::string> domainSuffixWildcards;

	std::string _error;
	size_t exceptionMaxLevel;
	size_t suffixMaxLevel;
};

#endif
