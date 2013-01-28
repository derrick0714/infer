#include <iostream>
#include <sstream>
#include <vector>
#include <cmath>
#include <signal.h>
#include <errno.h>
#ifdef __FreeBSD__
	#include <sys/ioctl.h>
#endif

#include <boost/program_options.hpp>

#include "configuration.hpp"
#include "locks.h"
#include "modules.h"
#include "network.h"
#include "packet.h"
#include "queue.hpp"
#include "stringHelpers.h"
#include "print_error.hpp"

using namespace std;
using namespace boost::program_options;

pcap_t *pcapDescriptor;
bool capture = true;
Memory <Packet> packetMemory;
Memory <char> pcapPacketMemory;
Locks locks;
size_t flush_interval;
uint64_t processedPackets, droppedPackets;
pcap_stat oldPcapStats, pcapStats;
pthread_mutex_t pcapStatsLock, droppedPacketsLock;

void getLocalNetworks(const string &localNetworks,
					  vector <pair <uint32_t, uint32_t> > &_localNetworks) {
	stringstream __localNetworks(localNetworks);
	string localNetwork;
	size_t slash;
	uint32_t ip;
	while (__localNetworks >> localNetwork) {
		slash = localNetwork.rfind('/');
		ip = textToIP(localNetwork.substr(0, slash));
		_localNetworks.push_back(
			make_pair(ip,
					  ip +
						pow((double)2,
							(double)(32 -
								strtoul(localNetwork.substr(slash + 1).c_str(),
										NULL, 10))) - 1));
	}
}

void *getPcapStats(void *stats_interval) {
	size_t *_stats_interval = (size_t*)stats_interval;
	uint32_t processedDifference, droppedDifference;
	uint64_t totalDroppedBPFPackets = 0, oldDroppedPackets = 0;
	while (capture) {
		sleep(*_stats_interval);
		oldPcapStats = pcapStats;
		if (pcap_stats(pcapDescriptor, &pcapStats) == -1) {
			print_error("getPcapStats(): ", pcap_geterr(pcapDescriptor));
		}
		if (pcapStats.ps_recv >= oldPcapStats.ps_recv) {
			processedDifference = pcapStats.ps_recv - oldPcapStats.ps_recv;
		}
		else {
			processedDifference = numeric_limits <uint32_t>::max() -
									oldPcapStats.ps_recv + pcapStats.ps_recv;
		}
		processedPackets += processedDifference;
		if (pcapStats.ps_drop >= oldPcapStats.ps_drop) {
			droppedDifference = pcapStats.ps_drop - oldPcapStats.ps_drop;
		}
		else {
			droppedDifference = numeric_limits <uint32_t>::max() -
									oldPcapStats.ps_drop + pcapStats.ps_drop;
		}
		totalDroppedBPFPackets += droppedDifference;
		cout << "Packets processed: " << processedPackets << endl
			 << "Current packet loss: BPF: " << droppedDifference << " ("
			 << (double)droppedDifference / processedDifference * 100
			 << "%), sensor: " << droppedPackets - oldDroppedPackets << " ("
			 << (double)(droppedPackets - oldDroppedPackets) /
					processedDifference * 100
			 << "%), total: " << droppedDifference +
					(droppedPackets - oldDroppedPackets)
			 << " (" << (double)(droppedDifference +
					(droppedPackets - oldDroppedPackets)) /
						processedDifference * 100
			 << "%)" << endl << "Total packet loss: BPF: "
			 << totalDroppedBPFPackets << " ("
			 << (double)totalDroppedBPFPackets / processedPackets * 100
			 << "%), sensor: " << droppedPackets << " ("
			 << (double)droppedPackets / processedPackets * 100
			 << "%), total: " << totalDroppedBPFPackets + droppedPackets
			 << " ("
			 << (double)(totalDroppedBPFPackets + droppedPackets) /
					processedPackets * 100
			 << "%)" << endl << endl;
		pthread_mutex_lock(&droppedPacketsLock);
		oldDroppedPackets = droppedPackets;
		pthread_mutex_unlock(&droppedPacketsLock);
	}
	return NULL;
}

void *flushThread(void *moduleGroup) {
	ModuleGroup *_moduleGroup = (ModuleGroup*)moduleGroup;
	while (capture) {
		sleep(flush_interval);
		for (size_t module = 0;
			 module < _moduleGroup -> modules.size();
			 ++module)
		{
			_moduleGroup -> modules[module].flush();
		}
	}
	return NULL;
}

/*
 * As long as the program hasn't received SIGINT, waits to be woken up by
 * the module group's checkForWork() function in main(), then, as long as any
 * modules in the group have packets in their packet queues, sequentially
 * calls the modules' processPacket() functions for the packets.
 */
void *processingThread(void *moduleGroup) {
	ModuleGroup *_moduleGroup = (ModuleGroup*)moduleGroup;
	Module *modulePtr;
	Packet *packet;
	while (capture && _moduleGroup -> tryLock() == 0) {
		_moduleGroup -> waitForWork();
		_moduleGroup -> setActive();
		while (_moduleGroup -> lockQueueSize() == 0 &&
				!(_moduleGroup -> empty()))
		{
			_moduleGroup -> unlockQueueSize();
			for (size_t module = 0;
				 module < _moduleGroup -> modules.size();
				 ++module)
			{
				modulePtr = &(_moduleGroup -> modules[module]);
				if (!(modulePtr -> packetQueue.empty())) {
					modulePtr -> packetQueue.lock();
					packet = modulePtr -> packetQueue.front();
					modulePtr -> packetQueue.pop();
					modulePtr -> packetQueue.unlock();
					_moduleGroup -> lockQueueSize();
					_moduleGroup -> decrementQueueSize();
					_moduleGroup -> unlockQueueSize();
					modulePtr -> processPacket(*packet);
					packet -> lock();
					packet -> dereference();
					if (packet -> references() == 0 && packet -> freeable()) {
						packet -> unlock();
						packet -> free();
						packetMemory.lock();
						packetMemory.free(packet);
						packetMemory.unlock();
					}
					else {
						packet -> unlock();
					}
				}
			}
		}
		_moduleGroup -> unlockQueueSize();
		_moduleGroup -> setInactive();
		_moduleGroup -> unlock();
	}
	pthread_join(_moduleGroup -> flushThread, NULL);
	for (size_t module(0);
		 module < _moduleGroup -> modules.size();
		++module)
	{
		_moduleGroup -> modules[module].finish();
	}
	return NULL;
}

void sigint(int) {
	if (capture) {
		cout << "Caught SIGINT; exiting." << endl;
		capture = false;
	}
}

int main(int argc, char *argv[]) {
	options_description desc_gen("Arguments");
	desc_gen.add_options()
		("help,h", "display help message")
		("config-file,c",
			value<string>()->default_value
				("/usr/local/etc/infer.conf"),
			"specify configuration file")
		(
			"input-file,f",
			value<vector<string> >(),
			"specify input file"
		)
	;

	positional_options_description pd;
	pd.add("input-file", 1);

	variables_map vm;
	try {
		store(command_line_parser(argc, argv).
			options(desc_gen).positional(pd).run(), vm);
	}
	catch (error e) {
		cerr << e.what() << endl;
		cerr << "usage: "
			 << argv[0] << " [ -c configuration_file ] [ pcap_file ]"
			 << endl;
		return 1;
	}
	notify(vm);

	if (vm.count("help")) {
		cerr << "usage: "
			 << argv[0] << " [ -c configuration_file ] [ pcap_file ]"
			 << endl;
		return 0;
	}

	vector<string> input_files;
	if (vm.count("input-file")) {
		input_files = vm["input-file"].as<vector<string> >();
	}

	cout << "config file: " << vm["config-file"].as<string>() << endl;
	for (vector<string>::const_iterator i(input_files.begin());
		 i != input_files.end();
		 ++i)
	{
		cout << "pcap file:   " << *i << endl;
	}

	configuration conf;
	if (!conf.load(vm["config-file"].as<string>())) {
		cerr << argv[0] << ": unable to load configuration" << endl;
		return 1;
	}

	string local_networks;
	if (conf.get(local_networks, "local-networks", "sensor", true) !=
			configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid local-networks" << endl;
		return 1;
	}

	string data_directory;
	if (conf.get(data_directory, "data-directory", "sensor", true)
			!= configuration::OK)
	{
		cerr << argv[0] << ": data-directory required" << endl;
		return 1;
	}

	string interface;
	if (conf.get(interface, "interface", "sensor") !=
			configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid interface" << endl;
		return 1;
	}

	size_t snap_length;
	if (conf.get(snap_length, "snap-length", "sensor") !=
			configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid snap-length" << endl;
		return 1;
	}

	string module_directory;
	if (conf.get(module_directory, "module-directory", "sensor") !=
			configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid module-directory" << endl;
		return 1;
	}

	string module_prefix;
	if (conf.get(module_prefix, "module-prefix", "sensor") !=
			configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid module-prefix" << endl;
		return 1;
	}

	vector<string> module_groups;
	if (conf.get(module_groups, "module-group", "sensor") !=
			configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid module-group" << endl;
		return 1;
	}

	size_t packet_queue_size;
	if (conf.get(packet_queue_size, "packet-queue-size", "sensor") !=
			configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid packet-queue-size" << endl;
		return 1;
	}

	if (conf.get(flush_interval, "flush-interval", "sensor") !=
			configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid flush-interval" << endl;
		return 1;
	}

	size_t stats_interval;
	if (conf.get(stats_interval, "stats-interval", "sensor") !=
			configuration::OK)
	{
		cerr << argv[0] << ": missing or invalid stats-interval" << endl;
		return 1;
	}

	vector <ModuleGroup> moduleGroups;
	Module *modulePtr;
	char errorBuffer[PCAP_ERRBUF_SIZE];
	string filter;
	bpf_program bpfProgram;
	#ifdef __FreeBSD__
		u_int immediate = 1;
	#endif
	const u_char *pcapPacket;
	pcap_pkthdr pcapHeader;
	Packet *packet;
	vector <pair <uint32_t, uint32_t> > localNetworks;
	pthread_t pcapStatsThread;
	signal(SIGINT, sigint);
	getLocalNetworks(local_networks, localNetworks);
	packetMemory.initialize(packet_queue_size);
	pcapPacketMemory.initialize(packet_queue_size, snap_length);
	locks.initialize(packet_queue_size);
	pthread_mutex_init(&pcapStatsLock, NULL);
	pthread_mutex_init(&droppedPacketsLock, NULL);
	/* Loads modules into module groups. */

	vector<vector<string> > module_filters;
	for (size_t index = 0; index < module_groups.size(); ++index) {
		module_filters.push_back(vector<string>());
		vector<string> groupModules(explodeString(module_groups[index], " "));
		for (size_t j = 0; j < groupModules.size(); ++j) {
			string str;
			if (conf.get(str, "filter", "sensor_" + groupModules[j]) !=
					configuration::OK)
			{
				cerr << argv[0] << ": missing or invalid sensor_"
					 << groupModules[j] << ".filter" << endl;
				return 1;
			}
			module_filters.rbegin()->push_back(str);

			groupModules[j] = module_prefix + groupModules[j];
		}
		moduleGroups.push_back(ModuleGroup(data_directory,
										   module_directory,
										   groupModules,
										   packet_queue_size,
										   snap_length,
										   conf,
										   module_filters[index]));
		if (!moduleGroups[index]) {
			print_error(argv[0], "moduleGrops", moduleGroups[index].error());
			return 1;
		}
	}

	if (input_files.empty()) {
		pcapDescriptor = pcap_open_live(interface.c_str(),
										snap_length, 1, 0, errorBuffer);
		if (pcapDescriptor == NULL) {
			print_error(argv[0], "pcap_open_live", errorBuffer);
			return 1;
		}
	}
	else {
		// FIXME currently, we only read from the first file specified
		pcapDescriptor = pcap_open_offline(input_files[0].c_str(), errorBuffer);
		if (pcapDescriptor == NULL) {
			print_error(argv[0], "pcap_open_offline", errorBuffer);
			return 1;
		}
	}
	/*
	 * Combines all modules' filter strings to form the main filter string, and
	 * starts each module group's processing thread and flush thread.
	 */
	for (size_t moduleGroup = 0; moduleGroup < moduleGroups.size(); ++moduleGroup) {
		for (size_t module = 0;
			 module < moduleGroups[moduleGroup].modules.size();
			 ++module)
		{
			filter += '(' + module_filters[moduleGroup][module] + ')';
			if (moduleGroup != moduleGroups.size() - 1 ||
					module != moduleGroups[moduleGroup].modules.size() - 1)
			{
				filter += " or ";
			}
		}
		pthread_create(&(moduleGroups[moduleGroup].processingThread), NULL,
									 &processingThread, &(moduleGroups[moduleGroup]));
		pthread_create(&(moduleGroups[moduleGroup].flushThread), NULL, &flushThread,
									 &(moduleGroups[moduleGroup]));
	}
	if (pcap_compile(pcapDescriptor, &bpfProgram, filter.c_str(), 1, 0) == -1) {
		print_error(argv[0], pcap_geterr(pcapDescriptor));
		return 1;
	}
	if (pcap_setfilter(pcapDescriptor, &bpfProgram) == -1) {
		print_error(argv[0], pcap_geterr(pcapDescriptor));
		return 1;
	}
	/* If running on FreeBSD, puts the BPF device into immediate mode. */
	#ifdef __FreeBSD__
		if (input_files.empty()) {
			if (ioctl(pcap_fileno(pcapDescriptor), BIOCIMMEDIATE, &immediate) == -1) {
				print_error(argv[0], strerror(errno));
				return 1;
			}
		}
	#endif
	if (input_files.empty()) {
		cout << "Capturing on " << interface << '.' << endl;
	}
	else {
		cout << "Reading from " << input_files[0] << '.' << endl;
	}

	/* Starts the stats thread. */
	if (input_files.empty()) {
		pthread_create(&pcapStatsThread, NULL, &getPcapStats, &stats_interval);
	}
	/*
	 * Fills the packet queue with captured packets until the capture variable is
	 * set to false by the signal handler.
	 */
	while (capture) {
		if ((pcapPacket = pcap_next(pcapDescriptor, &pcapHeader)) != NULL) {
			/* Allocates memory for a Packet class using the memory allocator. */
			packetMemory.lock();
			if ((packet = packetMemory.allocate()) == NULL) {
				packetMemory.unlock();
				pthread_mutex_lock(&droppedPacketsLock);
				++droppedPackets;
				pthread_mutex_unlock(&droppedPacketsLock);
			}
			else {
				packetMemory.unlock();
				/*
				 * Initializes the Packet class with information from the captured
				 * packet.
				 */
				packet -> initialize(pcapHeader, pcapPacket, pcapPacketMemory, locks,
														 localNetworks);
				for (size_t moduleGroup = 0; moduleGroup < moduleGroups.size(); ++moduleGroup) {
					for (size_t module = 0; module < moduleGroups[moduleGroup].modules.size(); ++module) {
						modulePtr = &(moduleGroups[moduleGroup].modules[module]);
						/* Checks if a module is interested in the packet. */
						if (bpf_filter(modulePtr -> bpfProgram().bf_insns,
													 (u_char*)pcapPacket, pcapHeader.len,
													 pcapHeader.caplen) != 0) {
							/* Increments the packet's reference count. */
							packet -> lock();
							packet -> reference();
							packet -> unlock();
							/* Adds a pointer to the packet to the module's packet queue. */
							modulePtr -> packetQueue.lock();
							modulePtr -> packetQueue.push(packet);
							modulePtr -> packetQueue.unlock();
							moduleGroups[moduleGroup].lockQueueSize();
							moduleGroups[moduleGroup].incrementQueueSize();
							moduleGroups[moduleGroup].unlockQueueSize();
						}
					}
				}
				/*
				 * Frees the packet if its reference count is 0, or sets it freeable by
				 * a processing thread otherwise.
				 */
				packet -> lock();
				if (!(packet -> references())) {
					packet -> unlock();
					packet -> free();
					packetMemory.lock();
					packetMemory.free(packet);
					packetMemory.unlock();
				}
				else {
					packet -> setFreeable();
					packet -> unlock();
				}
			}
		}
		else if (!input_files.empty()) {
			capture = false;
		}
		/*
		 * Checks if any processing threads are sleeping and, if so, wakes them up
		 * up if there's any work for them to do.
		 */
		for (size_t moduleGroup = 0; moduleGroup < moduleGroups.size(); ++moduleGroup) {
			moduleGroups[moduleGroup].checkForWork();
		}
	}
	/* Waits for all threads to return. */
	for (size_t moduleGroup = 0; moduleGroup < moduleGroups.size(); ++moduleGroup) {
		moduleGroups[moduleGroup].finish();
	}
	pthread_join(pcapStatsThread, NULL);
}
