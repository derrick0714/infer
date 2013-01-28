/// A simple, stupid test data generator for executive dashboard.
/// Author: Kulesh Shanmugasundaram
/// Date: 05/21/2010
///
/// Compiling: g++ -Wall data-generator.cpp -o generate-data
/// 
/// Usage: ./generate-data
/// The program takes no arguments. Every time it is called it will generate
/// the nesscessary data send to standard output. This output can (and perhaps
/// should be) piped to a file. Everytime the frontend requests an update from
/// the backend (via Ajax) data in this file should be read, converted to JSON
/// representation, and sent to the frontend for appropriate display.
///
/// Possible Use-Case: One way to use this file to generate fresh data is to
/// call this program from a shell-script (or shell command) periodically. For
/// example the following command in bash will refresh data every 10 seconds.
///
/// % while(true) do ./generate-data > data.tmp && mv data.tmp data.log && sleep 10; done;
///
/// Notes: 
/// * Data in data.log (and data.tmp) is always overwritten with new data
/// because that's what the frontend is interested in.
///
/// * The backend PHP script will read data.log (NOT data.tmp). The reason
/// the output is saved to data.tmp and then renamed to data.log is because
/// writes from this program are not atomic. But, mv operation in Unix is atomic.
/// This prevents the PHP script from reading mangled data.
///


#include <iostream>
#include <iomanip>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

using namespace std;

int main(int argc, char** argv){
	/// First lets output random data for network throughput quadrant
	/// (top-left). There are three different tables for this:
	/// 1. Ingress traffic with types
	/// 2. Egress traffic with types
	/// 3. Throughput at five-minute interval of the day
	/// 
	/// This data is for quadrant 1 shown in Slide-04 on top-left corner.
	/// Corresponding JSON file is named traffic.json (from json-files.zip)
	///
	
	string content_type[]={	"MP3",
							"MPEG",
							"JPEG",
							"Encrypted",
							"Compressed",
							"PlainText",
							"Binary",
							"Wave Audio"
							};
	
	srand(time(NULL));

	/// 1. Ingress Traffic
	cout << "Content Type\t: Ingress Traffic in Bytes" << endl;
	for(unsigned int i=0; i < 8; ++i){
		cout << content_type[i] << "\t: " << (rand() % 10000) << endl;
	}

	cout << endl;

	/// 2. Egress Traffic
	cout << "Content Type\t: Egress Traffic in Bytes" << endl;
	for(unsigned int i=0; i < 8; ++i){
		cout << content_type[i] << "\t: " << (rand() % 10000) << endl;
	}

	cout << endl;
	/// 3. Throughput at five-minute intervals
	cout << "Hour:Minutes\t:Throughput in Mbps" << endl;
	for(unsigned int i=0; i < 24; ++i){
		for(unsigned int j=0; j < 60; j += 5){
			cout << setw(2) << setfill('0') << i << ":" << setw(2) << setfill('0') << j << "\t: " << (rand() % 1000) << endl;
		}
	}

	cout << endl;



	/// The next data to generate is about host inventory (second quadrant, top
	/// right on Slide-4). Here, basically we are talking about the number of 
	/// different servers we found on a network and how many connections were
	/// established to the servers over the past two weeks.
	///
	/// This data is for quadrant 2 shown in Slide-04 on top-right corner.
	/// Corresponding JSON file is named host-inventory.json (from json-files.zip)
	///
	
	string host_roles[]={"Web Servers",
						 "Web Clients",
						 "Mail Servers",
						 "Mail Clients",
						 "P2P Nodes"
						};

	for(unsigned int i=0; i < 5; ++i){
		cout << "Host Role: " << host_roles[i] << ": " << (rand() % 10) << endl;
		cout << "Week\tConnections" << endl;
		for(unsigned int j=1; j < 15; ++j){
			cout << j << ": " << (rand() % 10000) << endl;
		}
		cout << endl;
	}


	/// The third data we generate is application inventory. This is the third
	/// quadrant (bottom left) in the screen on Slide-4.
	///
	/// This data is for quadrant 3 shown in Slide-04 on bottom-left corner.
	/// Corresponding JSON file is named app-inventory.json (from json-files.zip)
	///

	string app_invetory[]={	"Browsing\tInternet Explorer\t6.1,7.0\t7.0-90%",
							"Emails\tThunderbird\t3.2\t3.2-100%",
							"IM\tJabber, AOL, Yahoo!\t6.0, 3.2, 4.5\t6.0-90%"
						};
	cout << "Activity\tApplication\tVersions\tPopular Version\tTime Spent (HH:MM)"<<endl;
	for(unsigned int i=0; i < 3; ++i){
		cout << app_invetory[i] << "\t" << (rand() % 10) << ":" << (rand() % 59) << endl;
	}

	cout << endl;

	/// Finally, we generate data for the fourth quadrant (bottom right on
	/// Slide 4). This data is for network exposure.
	///
	/// This data is for quadrant 4 shown in Slide-04 on bottom-right corner.
	/// Corresponding JSON file is named network-exposure.json (from json-files.zip)
	///

	string as_names[]={	"Google",
						"Facebook",
						"Twitter",
						"Microsoft",
						"The Planet",
						"CNG Group"
						};

	cout << "Autonomous System\tAS Number\tDistinct Contacts\tIngress Traffic in MB\tEgress Traffic in MB" << endl;

	for(unsigned int i=0; i < 6; ++i){
		cout << as_names[i] << "\t" << (rand() % 2048) << "\t" << (rand() % 256) << "\t" << (rand() % 1024) << "\t" << (rand() % 1024) << endl;
	}

	return 0;
}
