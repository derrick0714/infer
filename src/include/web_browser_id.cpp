#include <boost/regex.hpp>

#include "web_browser_id.h"

using namespace std;

bool get_browser_from_user_agent(pair<string, string> &browser,
								 const string &ua)
{
	using boost::regex;
	using boost::smatch;

	static const regex
		firefox("Firefox\\/([[:digit:]\\.]+)"),
		msie("MSIE ([[:digit:]\\.]+)"),
		chrome("Chrome\\/([[:digit:]\\.]+)"),
		opera("Opera\\/([[:digit:]\\.]+)"),
		safari_mobile("Version\\/([[:digit:]\\.]+ Mobile).*Safari"),
		safari("Version\\/([[:digit:]\\.]+).*Safari");
	smatch matches;

	if (regex_search(ua, matches, firefox)) {
		browser.first.assign("Firefox");
		browser.second.assign(matches[1].first, matches[1].second);

		return true;
	}

	if (regex_search(ua, matches, msie)) {
		browser.first.assign("Internet Explorer");
		browser.second.assign(matches[1].first, matches[1].second);

		return true;
	}

	if (regex_search(ua, matches, chrome)) {
		browser.first.assign("Chrome");
		browser.second.assign(matches[1].first, matches[1].second);

		return true;
	}

	if (regex_search(ua, matches, opera)) {
		browser.first.assign("Opera");
		browser.second.assign(matches[1].first, matches[1].second);

		return true;
	}

	if (regex_search(ua, matches, safari_mobile)) {
		browser.first.assign("Safari");
		browser.second.assign(matches[1].first, matches[1].second);

		return true;
	}

	if (regex_search(ua, matches, safari)) {
		browser.first.assign("Safari");
		browser.second.assign(matches[1].first, matches[1].second);

		return true;
	}

	return false;
}
