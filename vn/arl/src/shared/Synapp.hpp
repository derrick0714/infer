#ifndef SYNAPP_HPP
#define SYNAPP_HPP

#include <iostream>

#include "SynappArguments.h"

namespace vn {
namespace arl {
namespace shared {

class Synapp {
  public:
	explicit Synapp()
		:error(std::cerr),
		 debug(std::cout)
	{
	}

	virtual ~Synapp() {}

	virtual int run() = 0;

  protected:
	std::ostream &error;
	std::ostream &debug;
};

} // namespace shared
} // namespace arl
} // namespace vn

#endif
