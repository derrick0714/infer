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
# System setup                                                                 #
################################################################################

if ! grep "^hostname=" /etc/rc.conf
then
	echo "hostname=\"$hostname\"" >> /etc/rc.conf
else
	sed -e "s/^hostname.*/hostname=\"$hostname\"/" /etc/rc.conf
fi
hostname $hostname
echo -e "$management_ip\t$hostname\t$hostname" >> /etc/hosts

# users and groups
pw groupadd -n bpf
pw groupadd -n sensor
pw useradd -n sensor -g sensor -G bpf -m
pw groupadd -n analysis
pw useradd -n analysis -g analysis -m

# bpf
echo -e "[bpf=5]\nadd path 'bpf*' mode 0640 group bpf" >> /etc/devfs.rules
echo 'devfs_system_ruleset="bpf"' >> /etc/rc.conf
/etc/rc.d/devfs restart

if ! grep "ifconfig_$capture_interface" /etc/rc.conf
then
	echo "ifconfig_$capture_interface=\"up\"" >> /etc/rc.conf
fi


################################################################################
# Prerequisites                                                                #
################################################################################

# Sensor and Analysis

# Screen
cd /usr/ports/sysutils/screen/
make -D BATCH install clean || \
	{ echo $0: Error: unable to build screen; exit 1; }

# Boost
cd /usr/ports/devel/boost-libs
make -D BATCH install clean || \
	{ echo $0: Error: unable to build boost-libs; exit 1; }

# Sensor
# Fftw3
cd /usr/ports/math/fftw3/
make -D BATCH install clean || \
	{ echo $0: Error: unable to build fftw2; exit 1; }


# Analysis

# PostgreSQL
cd /usr/ports/databases/postgresql83-server
make -D BATCH -D WITHOUT_NLS install clean || \
	{ echo $0: Error: unable to build postgresql83-server; exit 1; }
echo 'postgresql_enable="YES"' >> /etc/rc.conf
/usr/local/etc/rc.d/postgresql initdb
/usr/local/etc/rc.d/postgresql start
psql -c "ALTER USER pgsql WITH ENCRYPTED PASSWORD '$pgsql_pgsql_pass';" \
	-U pgsql -d postgres
sed -e 's/^host/#&/' -e 's/^\(local.*\)trust$/\1md5/' -i '.bak' \
	/usr/local/pgsql/data/pg_hba.conf
/usr/local/etc/rc.d/postgresql restart

cd /usr/ports/databases/db44
make -D BATCH install clean || \
	{ echo $0: Error: unable to build db44; exit 1; }

# Apache
cd /usr/ports/www/apache22/
make -D BATCH install clean || \
#WITHOUT_APACHE_OPTIONS=yes WITH_MPM=worker WITH_SSL_MODULES=yes \
	{ echo $0: Error: unable to build apache22; exit 1; }
# configure apache
sed -e "s/^#ServerName.*/ServerName $management_ip:80/" \
	-e "s/DirectoryIndex index\.html/& index.php/" \
	-i '.bak' /usr/local/etc/apache22/httpd.conf
awk '{ sub(/AddType application\/x-gzip \.gz \.tgz/, \
		   "&\n    AddType application\/x-httpd-php .php"); print $0; }' \
	/usr/local/etc/apache22/httpd.conf > /usr/local/etc/apache22/httpd.conf.new
mv /usr/local/etc/apache22/httpd.conf.new /usr/local/etc/apache22/httpd.conf
echo 'apache22_enable="YES"' >> /etc/rc.conf

cat > /usr/local/etc/apache22/Includes/httpd-vhosts.conf << EOF
NameVirtualHost *:80

<VirtualHost *:80>
  DocumentRoot "$analysis_dir/www/htdocs"
  ServerName $management_ip
  RewriteEngine On
  RewriteRule / https://%{HTTP_HOST}%{REQUEST_URI}
  <Location />
    SSLRequireSSL
  </Location>
</VirtualHost>
EOF

cat > /tmp/$management_ip.cfg << EOF
[ req ]
default_bits = 4096
encrypt_key = yes
distinguished_name = req_dn
x509_extensions = cert_type
prompt = no

[ req_dn ]
# country (2 letter code)
C=US

# State or Province Name (full name)
ST=New York

# Locality Name (eg. city)
L=New York

# Organization (eg. company)
O=Vivic Networks

# Organizational Unit Name (eg. section)
OU=INFER

# Common Name (*.example.com is also possible)
CN=$management_ip

# E-mail contact
emailAddress=justin@isis.poly.edu

[ cert_type ]
nsCertType = server
EOF

openssl req -new -x509 -nodes -config /tmp/$management_ip.cfg \
	-out /usr/local/etc/apache22/$management_ip.crt \
	-keyout /usr/local/etc/apache22/$management_ip.key -days 730
chmod 640 /usr/local/etc/apache22/$management_ip.key

awk '{ sub(/<VirtualHost _default_:443>/, \
		   "&\n<Location />\n  allow from all\n  " \
		   "options multiviews\n  AcceptPathInfo ON\n</Location>"); print $0; }' \
	/usr/local/etc/apache22/extra/httpd-ssl.conf \
		> /usr/local/etc/apache22/Includes/httpd-ssl.conf
sed -e "s/^\(DocumentRoot\).*/\1 \"\/usr\/home\/infer\/analysis\/www\/htdocs\"/" \
	-e "s/^ServerName.*/ServerName $management_ip:443/" \
	-e "s/^\(SSLCertificateFile.*\)server/\1$management_ip/" \
	-e "s/^\(SSLCertificateKeyFile.*\)server/\1$management_ip/" \
	-i '' /usr/local/etc/apache22/Includes/httpd-ssl.conf




# PHP
cd /usr/ports/lang/php5/
make -D BATCH -D WITHOUT_CGI -D WITHOUT_IPV6 -D WITH_APACHE install clean || \
	{ echo $0: Error: unable to build php5; exit 1; }
echo "date.timezone = $php_timezone" > /usr/local/etc/php.ini
echo "magic_quotes_gpc = Off" >> /usr/local/etc/php.ini
echo "magic_quotes_runtime = Off" >> /usr/local/etc/php.ini
echo "magic_quotes_sybase = Off" >> /usr/local/etc/php.ini

cd /usr/ports/graphics/php5-gd/
make -D BATCH -D WITHOUT_X11 install clean || \
	{ echo $0: Error: unable to build php5-gd; exit 1; }

cd /usr/ports/net/php5-ldap/
make -D BATCH install clean || \
	{ echo $0: Error: unable to build php5-ldap; exit 1; }

cd /usr/ports/security/php5-hash/
make -D BATCH install clean || \
	{ echo $0: Error: unable to build php5-hash; exit 1; }

cd /usr/ports/sysutils/php5-posix/
make -D BATCH install clean || \
	{ echo $0: Error: unable to build php5-posix; exit 1; }

cd /usr/ports/devel/php5-pcntl/
make -D BATCH install clean || \
	{ echo $0: Error: unable to build php5-pcntl; exit 1; }

cd /usr/ports/databases/php5-pgsql/
make -D BATCH install clean || \
	{ echo $0: Error: unable to build php5-pgsql; exit 1; }

cd /usr/ports/devel/php5-json/
make -D BATCH install clean || \
	{ echo $0: Error: unable to build php5-json; exit 1; }

cd /usr/ports/www/php5-session/
make -D BATCH install clean || \
	{ echo $0: Error: unable to build php5-session; exit 1; }

# JpGraph2
cd /usr/ports/graphics/jpgraph2/
make -D BATCH install clean || \
	{ echo $0: Error: unable to build jpgraph2; exit 1; }

cd /usr/ports/databases/p5-DBD-Pg/
make -D BATCH install clean || \
	{ echo $0: Error: unable to build p5-DBD-Pg; exit 1; }

cd /usr/ports/mail/p5-Net-SMTP_auth/
make -D BATCH install clean || \
	{ echo $0: Error: unable to build p5-Net-SMTP_auth; exit 1; }


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

################################################################################
# Configuration                                                                #
################################################################################

# Sensor
cp /usr/local/share/examples/infer/infer_sensor_neoflow_fcc.model /usr/local/etc
cp /usr/local/share/examples/infer/infer.conf /usr/local/etc/
chown root:analysis /usr/local/etc/infer.conf
chmod 660 /usr/local/etc/infer.conf
pw groupmod analysis -m www
/usr/local/bin/infer_config sensor.interface "$capture_interface"
/usr/local/bin/infer_config data-directory "$sensor_data_dir"
/usr/local/bin/infer_config local-networks "$monitored_networks"
/usr/local/bin/infer_config analysis_dir "$analysis_dir"

# Analysis
cp /usr/local/share/examples/infer/infer_analysis_scanners_monitoredServices.conf /usr/local/etc/
if [ ! -e /usr/local/etc/infer.conf ]
then
	cp /usr/local/share/examples/infer/infer.conf /usr/local/etc/
fi

/usr/local/bin/infer_config postgresql.user "ims"
/usr/local/bin/infer_config postgresql.password "$pgsql_infer_pass"
/usr/local/bin/infer_config postgresql.dbname "IMS"

/usr/local/bin/infer_config data-directory "$sensor_data_dir"
/usr/local/bin/infer_config local-networks "$monitored_networks"
/usr/local/bin/infer_config management-host "$management_ip"

################################################################################
# Database Setup                                                               #
################################################################################

/usr/local/bin/infer_database_setup.sh "$pgsql_pgsql_pass" "$frontend_admin_user" "$frontend_admin_pass"


################################################################################
# Country Map                                                                  #
################################################################################

/usr/local/bin/infer_country_map


################################################################################
# Initialization                                                               #
################################################################################

/usr/local/etc/rc.d/infer_sensor.sh start

/usr/local/etc/rc.d/apache22 start
/usr/local/etc/rc.d/infer_query_manager.sh start
