#ifndef INFER_INCLUDE_WEB_BROWSER_ID_H_
#define INFER_INCLUDE_WEB_BROWSER_ID_H_

#include <string>

bool get_browser_from_user_agent(std::pair<std::string, std::string> &browser,
								 const std::string &ua);

#endif
