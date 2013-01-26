#ifndef INFER_INCLUDE_PRINT_ERROR_HPP_
#define INFER_INCLUDE_PRINT_ERROR_HPP_

#include <iostream>
#include <string>

inline
void print_error(const std::string &program, const std::string &reason) {
	std::cerr << program << ": " << reason << std::endl;
}

inline
void print_error(const std::string &program, const std::string &object,
				 const std::string &reason)
{
	std::cerr << program << ": " << object << ": " << reason << std::endl;
}

inline
void print_error(const std::string &program, const std::string &function,
				 const std::string &object, const std::string &reason)
{
	std::cerr << program << ": " << function << ": " << object << ": "
			  << reason << std::endl;
}

#endif
