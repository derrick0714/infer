#ifndef _NAME_RESOLUTION_SOURCE_TYPE_HPP_
#define _NAME_RESOLUTION_SOURCE_TYPE_HPP_

#include <string>

namespace ims {
  namespace name {
    class NameResolutionSourceType {
      public:
	enum SourceType { ANY, DNS, NETBIOS };

	NameResolutionSourceType();
	NameResolutionSourceType(const NameResolutionSourceType &nameResolutionSourceType);
	NameResolutionSourceType(SourceType sourceType);

	bool operator==(const NameResolutionSourceType &rhs) const;
	//bool operator==(SourceType rhs) const;
	bool operator!=(const NameResolutionSourceType &rhs) const;
	//bool operator!=(SourceType rhs) const;
	NameResolutionSourceType &operator=(const NameResolutionSourceType &rhs);
	//NameResolutionSourceType &operator=(SourceType rhs);
	bool operator<(const NameResolutionSourceType &rhs) const;
	//bool operator<(SourceType rhs) const;

      private:
	SourceType type;
    };

    inline NameResolutionSourceType::NameResolutionSourceType() 
	:type(ANY)
    {}

    inline NameResolutionSourceType::NameResolutionSourceType(const NameResolutionSourceType &nameResolutionSourceType)
	:type(nameResolutionSourceType.type)
    {}

    inline NameResolutionSourceType::NameResolutionSourceType(SourceType sourceType)
	:type(sourceType)
    {}

    inline bool NameResolutionSourceType::operator==(const NameResolutionSourceType &rhs) const {
	return type == rhs.type;
    }
/*
    inline bool NameResolutionSourceType::operator==(const SourceType rhs) const {
	return type == rhs;
    }
*/
    inline bool NameResolutionSourceType::operator!=(const NameResolutionSourceType &rhs) const {
	return !(operator==(rhs));
    }
/*
    inline bool NameResolutionSourceType::operator!=(SourceType rhs) const {
	return !(operator==(rhs));
    }
*/
    inline NameResolutionSourceType &NameResolutionSourceType::operator=(const NameResolutionSourceType &rhs) {
	if (this != &rhs) {
	    type = rhs.type;
	}

	return *this;
    }
/*
    inline NameResolutionSourceType &NameResolutionSourceType::operator=(SourceType rhs) {
	type = rhs;

	return *this;
    }
*/
    inline bool NameResolutionSourceType::operator<(const NameResolutionSourceType &rhs) const {
	return type < rhs.type;
    }
/*
    inline bool NameResolutionSourceType::operator<(SourceType rhs) const {
	return type < rhs;
    }
*/
  }
}

#endif
