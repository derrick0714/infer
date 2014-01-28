#!/bin/sh

################################################################################
# Options                                                                      #
################################################################################

#  The location of the src/ directory in the tree.
src_dir=$(pwd)

# A space-separated list of CIDR blocks to be monitored by INFER
monitored_networks='128.238.66.175/24 10.0.128.0/24 10.0.192.0/24'


# Sensor

# Where to store data written by the sensor
sensor_data_dir='/usr/home/infer/sensor/data'

# WHere to anyalsis data
analysis_dir = '/usr/home/infer/analysis'

# interface from which to capture packet data for analysis
capture_interface='bge0'

# IP address of the management interface
management_ip='128.238.66.175'

# The desired hostname of this machine
hostname='infer'

# Analysis
#pgsql user name: pgsql

# The desired PostgreSQL super user password
pgsql_pgsql_pass='infer1234'

# The desired PostgreSQL infer user password
pgsql_infer_pass='infer1234'

# The timezone of this machine, for PHP
# see http://www.php.net/manual/en/timezones.php
php_timezone='America/New_York'

# The desired username for the initial INFER frontend administrator
frontend_admin_user='admin'

# The desired password for the initial INFER frontend administrator
frontend_admin_pass='infer1234'

# OS info (you can probably just leave this alone)
# os_name=$(uname -s)
# os_major=$(uname -r | sed -e 's/\..*//')
# os_minor=$(uname -r | sed -e 's/-.*//' -e 's/.*\.//')
# os_arch=$(uname -m)

# do not modify
install_prefix='/usr/local'

################################################################################
# Installation                                                                 #
################################################################################

# compile and install from source
cd "$src_dir"
gmake dist
gmake install

mkdir -p $sensor_data_dir
chown sensor:sensor $sensor_data_dir

echo -e "0\t0\t*\t*\t*\tsensor\t/usr/local/bin/infer_delete_old_data" \
	>> /etc/crontab
echo -e "30\t2\t*\t*\t*\tanalysis\t/usr/local/bin/infer_daily.sh" \
	>> /etc/crontab
echo -e "15\t*\t*\t*\t*\tanalysis\t/usr/local/bin/infer_hourly.sh" \
	>> /etc/crontab
/etc/rc.d/cron restart


mkdir $analysis_dir/privacy_graphs
chown -Rh analysis:analysis $analysis_dir/privacy_graphs

mkdir $analysis_dir/pruned_network_graphs
chown -Rh analysis:analysis $analysis_dir/pruned_network_graphs

touch /var/log/infer_search.log
chown www:www /var/log/infer_search.log

