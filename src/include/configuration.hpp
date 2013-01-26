#ifndef INFER_INCLUDE_CONFIGURATION_HPP_
#define INFER_INCLUDE_CONFIGURATION_HPP_

#include <vector>
#include <string>

#include <boost/lexical_cast.hpp>

#include "infer_conf_parser.hpp"

class configuration {
  public:
	enum error { OK, BAD_PATH, BAD_DATA };

	configuration()
		:_pt()
	{
	}

	bool load(const std::string &file) {
		using namespace boost::property_tree::infer_conf_parser;

		try {
			read_infer_conf(file, _pt);
		}
		catch (infer_conf_parser_error &e) {
			return false;
		}

		return true;
	}

/* This doesn't work because of a boost bug: http://lists.boost.org/boost-users/2010/01/55578.php
	template <typename T>
	error get(std::vector<T> &vals,
			  const std::string &option,
			  const std::string &section = "",
			  bool try_global = false)
	{
		using namespace boost::property_tree;

		ptree sec;
		std::string path;
		if (section.empty()) {
			sec = _pt;
		}
		else {
			try {
				sec = _pt.get_child(section);
			}
			catch (ptree_bad_path &e) {
				if (!try_global) {
					return BAD_PATH;
				}
				sec = _pt;
			}
		}
		
		std::pair<ptree::assoc_iterator, ptree::assoc_iterator> range(
			sec.equal_range(option));
		if (range.first == range.second) {
			range = _pt.equal_range(option);
		}
		for (ptree::assoc_iterator it(range.first); it != range.second; ++it) {
			vals.push_back(boost::lexical_cast<T>(*it));
		}

		return OK;
	}
*/

	template <typename T>
	error get(std::vector<T> &vals,
			  const std::string &option,
			  const std::string &section = "",
			  bool try_global = false) const
	{
		using namespace boost::property_tree;

		ptree sec;
		std::string path;
		if (section.empty()) {
			sec = _pt;
		}
		else {
			try {
				sec = _pt.get_child(section);
			}
			catch (ptree_bad_path &e) {
				if (!try_global) {
					return BAD_PATH;
				}
				sec = _pt;
			}
		}
		
		bool found(false);
		for (ptree::const_iterator it(sec.begin());
			 it != sec.end();
			 ++it)
		{
			if (it->first == option) {
				found = true;
				vals.push_back(boost::lexical_cast<T>(it->second.data()));
			}
		}

		if (found) {
			return OK;
		}

		if (sec == _pt) {
			return BAD_PATH;
		}

		for (ptree::const_iterator it(_pt.begin());
			 it != _pt.end();
			 ++it)
		{
			if (it->first == option) {
				found = true;
				vals.push_back(boost::lexical_cast<T>(it->second.data()));
			}
		}

		if (found) {
			return OK;
		}

		return BAD_PATH;
	}

	// gets option section.option. if try_global, return global value if not
	// found in section
	template <typename T>
	error get(T &val,
			  const std::string &option,
			  const std::string &section = "",
			  bool try_global = false) const
	{
		using namespace boost::property_tree;

		std::string path;
		if (section.empty()) {
			path = option;
		}
		else {
			path = section + "." + option;
		}

		try {
			val = _pt.get<T>(path);
			return OK;
		}
		catch (ptree_bad_data &e) {
			return BAD_DATA;
		}
		catch (ptree_bad_path &e) {
			if (!try_global) {
				return BAD_PATH;
			}
		}

		try {
			val = _pt.get<T>(option);
		}
		catch (ptree_bad_data &e) {
			return BAD_DATA;
		}
		catch (ptree_bad_path &e) {
			return BAD_PATH;
		}

		return OK;
	}

  private:
	boost::property_tree::ptree _pt;
};

#endif
