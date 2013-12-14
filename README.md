# INFER [![Build Status](https://secure.travis-ci.org/twitter/bower.png)](http://travis-ci.org/twitter/bower)


INFER system  is a project applying highly efficient data mining techniques to monitor the  network traffic so that the network resource is secure and properly used without abuse.

## Getting Started

Note: To install, make sure you have installed FreeBSD as documented in freebsd.txt and that you have followed the instructions in time.txt for setting up NTP and adjusting the timezone accordingly.

First you'll need to fork and clone this repo
```bash
git@github.com:derrick0714/infer.git 
```


Go into the src directory and copy the install script(s) into a directory in your PATH. 
```bash
cp bin/scripts/infer_{install,database}* /usr/local/bin/
```

Next, modify all of the configuration options in the install script. 
```bash
vim /usr/local/bin/infer_install.sh
```

Finally, execute the script. 
```bash
sh /usr/local/bin/infer_install.sh
```

## First Launch

When you finish intallation, you can login the Infer through any browser using user name and pwd configured in 

```bash
/usr/local/bin/infer_install.sh
```

<img src="https://raw.github.com/derrick0714/infer/master/Screenshot/Dashboard.png?token=1255657__eyJzY29wZSI6IlJhd0Jsb2I6ZGVycmljazA3MTQvaW5mZXIvbWFzdGVyL1NjcmVlbnNob3QvRGFzaGJvYXJkLnBuZyIsImV4cGlyZXMiOjEzODc2MDE4NjN9--3a6539de7b9fc8822f4ecc34ae328fb1b91c17de" />

## How does Infer work?

INFER is primarily composed of three main parts: The backend( capture packets ), middlewar (analyse network incidents), and frontend ( user interface ). 


## What traffics can be captured? 

The sensor component of INFER is responsible for packet capture. The
sensor program captures packets using libpcap, and feeds captured
packets to various modules. The modules are located in src/lib/sensor/.
Each module is responsible for writing its own synopses of the captured
traffic. Currently, the following modules have been developed:

  - HTTP (src/lib/sensor/http)

	Parses and stores information from http headers.

  - HBF (src/lib/sensor/hbf)

	Stores HBFs for all packet payloads above a configurable size.

  - NEOFLOW (src/lib/sensor/neoflow)

	Stores neoflow, a flow synopses which includes all layer 2 and 3
	info, as well as flow statistics and traffic-content type.

  - LIVE IPS (src/lib/sensor/live_ips)

	Keeps track of which hosts on the network were "alive". A live
	host is defined as a host that was seen as a source of network
	traffic.

  - DNS (src/lib/sensor/dns)

	Parses and stores DNS information. ie. who queried which names,
	who resolved which IPs, and so on.

## What network incidents can be analysed?

The middleware component of infer is responsible for processing the data
that was written by the sensor. It is not necessary for the middleware
to run on the same machine as the sensor; it just needs access to the
data written by the sensor modules.

### The middleware is run via the system cron daemon.

The daily script is responsible for launching the rest of the middleware processes: (src/bin/analysis))
```
src/bin/scripts/infer_daily.sh
```

The hourly script currently has one responsibility: insert the http data that was output from the sensor into PostgreSQL.
```
src/bin/scripts/infer_hourly.sh
```

Much like the sensor, the analysis does not do any processing itself; it
it merely responsible for loading modules, reading data, and passing the
data to the modules. Each module is responsible for keeping its own
state and storing the results of its analysis in a PostgreSQL database.
Modules are run in stages. That is, all data is fed to each module in a
stage. When that stage is complete, the next stage of modules is fed the
same data. This allows for later-stage modules to make use of state
possibly accumulated by earlier-stage modules.

### The following modules are desgined for analysing different incidents: 

See [all models](https://raw.github.com/derrick0714/infer/master/doc/architecture/all_models.txt?login=derrick0714&token=b2beeabd5d053b185fb1aa7b918a6f37) for more information.

  - DarkSpaceSources (src/lib/analysis/darkSpaceSources)

	Determines which external hosts attempted to contact internal hosts
	that were not "alive".

  - InfectedContacts (src/lib/analysis/infectedContacts)

	Determines which internal hosts have contacted hosts which were on
	one of several publically-accessible black lists.

  - NonDNSTraffic (src/lib/analysis/nonDNSTraffic)

	Determines which internal hosts have contacted external hosts
	without having first resolved their IP addresses. This is often
	indicative of P2P traffic.

  - FtpBruteForcers (src/lib/analysis/ftpBruteForcers)
  
  - SqlBruteForcers (src/lib/analysis/sqlBruteForcers)
 
  - SshBruteForcers (src/lib/analysis/sshBruteForcers)
 
  - TelnetBruteForcers (src/lib/analysis/telnetBruteForcers)

	Determines which external hosts have attempted to "brute force"
	unauthorized access to various services running on internal hosts.

  - HostTraffic (src/lib/analysis/hostTraffic)

	Accumulates various traffic statistics about each host on the
	network, including the number of flows, number of packets, bytes
	transferred, etc.

  - EvasiveTraffic (src/lib/analysis/evasiveTraffic)

	Isolates traffic that exibits behavior in line with methods used
	to evade detection by older IDSs, including traffic with a low ttl
	and highly-fragmented traffic.

  - DarkSpaceTargets (src/lib/analysis/darkSpaceTargets)

  	Determines which "live" internal hosts were in contact with known
	external dark-space sources.

  - HostPairs (src/lib/analysis/hostPairs)

	Records pairs of internal/external IP address that have been in
	communication with each other.

  - NetworkTraffic (src/lib/analysis/networkTraffic)

	Accumulates basic network-wide statistics, namely: the number of
	"live" IPs, the total number of flows, the total number of packets,
	and the number of bytes transferred.

  - TopPorts (src/lib/analysis/topPorts)

	Determines the most-active ports for both ingress and egress traffic
	for each internal host.

  - CommChannels (src/lib/analysis/commChannels)

	Isolates flows exibiting command-and-control-type behavior. If a host
	is determined to be infected, these could possibly be the channels
	used to control the host remotely.

  - Bandwidth_utilization (src/lib/analysis/bandwidth_utilization)

	Records the overall ingress and egress bandwidth usilization of the
	network.

  - Host_bandwidth_utilization (src/lib/analysis/host_bandwidth_utilization)

	Records the overall ingress and egress bandwidth usilization of each
	host on the network.

  - Network_exposure (src/lib/analysis/network_exposure)

	Records the exposure of the network to each AS and Country.

  - Host_exposure (src/lib/analysis/host_exposure)

	Records the exposure of each internal host to each AS and Country.



## How does the fornt end work?


The frontend component of infer is written in a combination of php,
html, and javascript. The frontend consists of the following file
hierarchy:

- The old site. This is useful to have for reference when porting oldfeatures to the new site.
```
src/www/infer/
```
	
- The new site.
```
src/www/infer-devel/
```
	

### Under the new site, the hierarchy is as follows:
```
scripts/[payload_]search.php
```
This script is executed by the frontend when a [payload] search
is initiated by the user. It is responsible for launching the
c++ program that actually runs the search, as well as updating
the DB with various stats and/or error codes after completion.

```
htdocs/
```
The www document root.

### Under the document root, the hierarchy is as follows:

```
css/
```
Style sheets are kept here.

```
data/
```
Data api location. Any time data is to be displayed to the user,
the javascript on the client side initiates an ajax call to an
accessor under this directory.

```
data_tables/
```
Format information. The client obtains the model by which to display
data from here.

```
images/
```
Various images. Arrows, country flags, etc.

```
include/
```
Stores the site layout header and footer, along with 404 page.

```
js/
```
Javascript code lives here. Vivic Networks' js lives in js/vn. All
other javascript in this directory is third-party.

```
site/
```
This directory contains the content for individual subsections of the
site. This is where the vast majority of site content (aside from the
actual data) comes from.

### The site is architected as follows:

Each major section of the site has its own php page directly under the
document root. ie. /dashboard.php, /incidents.php, etc.

Each major section of the site has one or more subsections. The content
for each subsection is stored under the /site directory and is loaded
via ajax calls on the client site. This means that navigating from one
subsection to another subsection of the same major section does not
cause the browser to reload the entire page, but just the content.

Each subsection is responsible for updating the content on uri change
events, which means that each subsection can have an arbitrary level
of url depth. This is done by implementing a tab_uri_notify javascript
function that gets called when the hash section of the uri changes.
