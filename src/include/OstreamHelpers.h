#ifndef INFER_INCLUDE_OSTREAMHELPERS_H_
#define INFER_INCLUDE_OSTREAMHELPERS_H_

#include <iosfwd>

// use forward declarations instead of #include

class HBFResult;
class OldHTTP;
//class HTTPResult;
//class HBFHTTPResult;

std::ostream & operator<<(std::ostream &os,
						  const HBFResult &result);

std::ostream & operator<<(std::ostream &os,
						  const OldHTTP &http);

/*
std::ostream & operator<<(std::ostream &os,
						  const HTTPResult &result);

std::ostream & operator<<(std::ostream &os,
						  const HBFHTTPResult &result);
*/

#endif
