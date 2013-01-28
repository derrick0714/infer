#include <iostream>
#include <string>
#include <set>
#include <map>
#include <sys/param.h>
#include <sys/mount.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#include "configuration.hpp"
#include "timeStamp.h"
#include "sqlTime.h"
#include "print_error.hpp"

using namespace std;

char textMonths[12][3] = { "01", "02", "03", "04", "05", "06", "07",
													 "08", "09", "10", "11", "12" };

bool getFileMode(mode_t &mode, struct stat &status, const string &name) {
	if (stat(name.c_str(), &status) != 0) {
		return false;
	}
	mode = status.st_mode;
	return true;
}

int main(int, char *argv[]) {
	SQLTime today = getSQLTime(time(NULL)),
					yesterday = getSQLTime(time(NULL) - 86400);
	struct statfs fsStats;
	int64_t currentFreeSpace, minimumFreeSpace;
	DIR *directory;
	dirent *directoryEntry;
	struct stat fileStatus;
	string fileName;
	mode_t fileMode;
	set <string> years, days;
	size_t numMonths = 0, numDays = 0;
	map <string, size_t> monthMap;
	map <size_t, string> months;

	configuration conf;
	if (!conf.load("/usr/local/etc/infer.conf")) {
		cerr << argv[0] << ": unable to load configuration" << endl;
		return 1;
	}

	string data_directory;
	if (conf.get(data_directory, "data-directory", "delete_old_data", true)
			!= configuration::OK)
	{
		cerr << argv[0] << ": data-directory required" << endl;
		return 1;
	}

	double min_free;
	if (conf.get(min_free, "min-free", "delete_old_data")
			!= configuration::OK)
	{
		cerr << argv[0] << ": min-free required" << endl;
		return 1;
	}

	if (statfs(data_directory.c_str(), &fsStats) != 0) {
		print_error(argv[0], "statfs", data_directory,
							 strerror(errno));
		return 1;
	}
	currentFreeSpace = fsStats.f_bavail * fsStats.f_bsize;
	minimumFreeSpace = (double)fsStats.f_blocks * fsStats.f_bsize * min_free / 100;
	if (currentFreeSpace >= minimumFreeSpace) {
		cout << fsStats.f_mntonname	<< " has at least "
			 << min_free << "% free space; exiting."
			 << endl;
		return 0;
	}
	else {
		cout << fsStats.f_mntonname << " has less than "
			 << min_free
			 << "% free space; proceeding to delete old data." << endl;
		directory = opendir(data_directory.c_str());
		if (directory == NULL) {
			print_error(argv[0], "opendir", data_directory,
								 strerror(errno));
			return 1;
		}
		while ((directoryEntry = readdir(directory)) != NULL) {
			if (strcmp(directoryEntry -> d_name, ".") != 0 &&
					strcmp(directoryEntry -> d_name, "..") != 0)
			{
				fileName = data_directory + '/' + directoryEntry -> d_name;
				if (!getFileMode(fileMode, fileStatus, fileName)) {
					print_error(argv[0], "stat", fileName, strerror(errno));
					return 1;
				}
				if (S_ISDIR(fileMode)) {
					years.insert(fileName);
				}
			}
		}
		closedir(directory);
		for (size_t month = 0; month < 12; ++month) {
			monthMap.insert(make_pair(textMonths[month], month));
		}
		for (set <string>::const_iterator yearItr = years.begin(); yearItr != years.end(); ++yearItr) {
			directory = opendir((*yearItr).c_str());
			if (directory == NULL) {
				print_error(argv[0], "opendir", *yearItr, strerror(errno));
				return 1;
			}
			while ((directoryEntry = readdir(directory)) != NULL) {
				if (strcmp(directoryEntry -> d_name, ".") != 0 &&
						strcmp(directoryEntry -> d_name, "..") != 0)
				{
					fileName = *yearItr + '/' + directoryEntry -> d_name;
					if (!getFileMode(fileMode, fileStatus, fileName)) {
						print_error(argv[0], "stat", fileName, strerror(errno));
						return 1;
					}
					if (S_ISDIR(fileMode) &&
							monthMap.find(directoryEntry -> d_name) != monthMap.end()) {
						months.insert(make_pair(monthMap[directoryEntry -> d_name],
																		fileName));
						++numMonths;
					}
				}
			}
			closedir(directory);
			for (map <size_t, string>::const_iterator monthItr = months.begin(); monthItr != months.end(); ++monthItr) {
				directory = opendir(monthItr -> second.c_str());
				if (directory == NULL) {
					print_error(argv[0], "opendir", monthItr -> second.c_str(),
							   strerror(errno));
					return 1;
				}
				while ((directoryEntry = readdir(directory)) != NULL) {
					if (strcmp(directoryEntry -> d_name, ".") != 0 &&
							strcmp(directoryEntry -> d_name, "..") != 0)
					{
						fileName = monthItr -> second + '/' + directoryEntry -> d_name;
						if (!getFileMode(fileMode, fileStatus, fileName)) {
							print_error(argv[0], "stat", fileName, strerror(errno));
							return 1;
						}
						if (S_ISDIR(fileMode)) {
							if (!(yesterday.year == (*yearItr).substr((*yearItr).rfind('/') + 1) &&
										yesterday.month == intToString(monthItr -> first + 1, "%02d") &&
										yesterday.day == directoryEntry -> d_name) &&
									!(today.year == (*yearItr).substr((*yearItr).rfind('/') + 1) &&
										today.month == intToString(monthItr -> first + 1, "%02d") &&
										today.day == directoryEntry -> d_name))
							{
								days.insert(fileName);
							}
							else {
								cout << "Data in " << fileName << " is too recent; skipping it."
										 << endl;
							}
							++numDays;
						}
					}
				}
				closedir(directory);
				for (set <string>::const_iterator dayItr = days.begin(); dayItr != days.end(); ++dayItr) {
					directory = opendir((*dayItr).c_str());
					if (directory == NULL) {
						print_error(argv[0], "opendir", *dayItr, strerror(errno));
						return 1;
					}
					while ((directoryEntry = readdir(directory)) != NULL) {
						if (strcmp(directoryEntry -> d_name, ".") != 0 &&
								strcmp(directoryEntry -> d_name, "..") != 0)
						{
							fileName = *dayItr + '/' + directoryEntry -> d_name;
							if (!getFileMode(fileMode, fileStatus, fileName)) {
								print_error(argv[0], "stat", fileName, strerror(errno));
								return 1;
							}
							if (S_ISREG(fileMode)) {
								if (unlink(fileName.c_str()) != 0) {
									print_error(argv[0], "unlink", fileName, strerror(errno));
									return 1;
								}
								else {
									currentFreeSpace += (fileStatus.st_blocks * 512);
									cout << "Deleted " << fileName << endl; 
								}
							}
						}
					}
					closedir(directory);
					if (rmdir((*dayItr).c_str()) != 0) {
						print_error(argv[0], "rmdir", *dayItr, strerror(errno));
						return 1;
					}
					else {
						--numDays;
						cout << "Removed " << *dayItr << endl;
						if (!numDays) {
							if (rmdir(monthItr -> second.c_str()) != 0) {
								print_error(argv[0], "rmdir", monthItr -> second,
													 strerror(errno));
								return 1;
							}
							else {
								--numMonths;
								cout << "Removed " << monthItr -> second << endl;
								if (!numMonths) {
									if (rmdir((*yearItr).c_str()) != 0) {
										print_error(argv[0], "rmdir", *yearItr, strerror(errno));
										return 1;
									}
									else {
										cout << "Removed " << *yearItr << endl;
									}
								}
							}
						}
						if (currentFreeSpace >= minimumFreeSpace) {
							cout << fsStats.f_mntonname	<< " has at least "
									 << min_free
									 << "% free space; exiting." << endl;
							return 0;
						}
					}
				}
				days.clear();
			}
			months.clear();
		}
	}
	return 0;
}
