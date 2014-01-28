#!/bin/sh

if [ $# -ne 3 ]; then
    echo "usage: $0 pgsql_pgsql_pass frontend_admin_user frontend_admin_pass"
    exit 1
fi

pgsql_pgsql_pass=$1
frontend_admin_user=$2
frontend_admin_pass=$3

echo "pgsql_pgsql_pass:    $pgsql_pgsql_pass"
echo "frontend_admin_user: $frontend_admin_user"
echo "frontend_admin_pass: $frontend_admin_pass"

################################################################################
# Options                                                                      #
################################################################################

capture_interface=$(/usr/local/bin/infer_config sensor.interface)
sensor_data_dir=$(/usr/local/bin/infer_config data-directory)
monitored_networks=$(/usr/local/bin/infer_config local-networks)

pgsql_infer_user=$(/usr/local/bin/infer_config postgresql.user)
pgsql_infer_pass=$(/usr/local/bin/infer_config postgresql.password)
pgsql_dbname=$(/usr/local/bin/infer_config postgresql.dbname)

# do not modify
install_prefix='/usr/local'

# database setup
psql -q "user = pgsql password = $pgsql_pgsql_pass dbname = postgres" << EOF
CREATE ROLE ims WITH LOGIN NOINHERIT ENCRYPTED PASSWORD '$pgsql_infer_pass';
CREATE DATABASE "IMS" ENCODING 'SQL_ASCII' OWNER ims;
EOF

psql -q "user = pgsql password = $pgsql_pgsql_pass dbname = IMS" << EOF
CREATE FUNCTION "uint16In"(cstring) RETURNS uint16 AS '$install_prefix/lib/libinfer_postgresql_uint16.so' LANGUAGE 'C';
CREATE FUNCTION "uint16Out"(uint16) RETURNS cstring AS '$install_prefix/lib/libinfer_postgresql_uint16.so' LANGUAGE 'C';
CREATE FUNCTION "uint16Receive"(internal) RETURNS uint16 AS '$install_prefix/lib/libinfer_postgresql_uint16.so' LANGUAGE 'C';
CREATE FUNCTION "uint16Send"(uint16) RETURNS bytea AS '$install_prefix/lib/libinfer_postgresql_uint16.so' LANGUAGE 'C';
CREATE TYPE "uint16" (INPUT = "uint16In", OUTPUT = "uint16Out", RECEIVE = "uint16Receive", SEND = "uint16Send", INTERNALLENGTH = 2, EXTERNALLENGTH = VARIABLE, ALIGNMENT = INT2);
CREATE FUNCTION "uint16LessThan"(uint16, uint16) RETURNS bool AS '$install_prefix/lib/libinfer_postgresql_uint16.so' LANGUAGE 'C';
CREATE FUNCTION "uint16LessThanOrEqualTo"(uint16, uint16) RETURNS bool AS '$install_prefix/lib/libinfer_postgresql_uint16.so' LANGUAGE 'C';
CREATE FUNCTION "uint16EqualTo"(uint16, uint16) RETURNS bool AS '$install_prefix/lib/libinfer_postgresql_uint16.so' LANGUAGE 'C';
CREATE FUNCTION "uint16NotEqualTo"(uint16, uint16) RETURNS bool AS '$install_prefix/lib/libinfer_postgresql_uint16.so' LANGUAGE 'C';
CREATE FUNCTION "uint16GreaterThanOrEqualTo"(uint16, uint16) RETURNS bool AS '$install_prefix/lib/libinfer_postgresql_uint16.so' LANGUAGE 'C';
CREATE FUNCTION "uint16GreaterThan"(uint16, uint16) RETURNS bool AS '$install_prefix/lib/libinfer_postgresql_uint16.so' LANGUAGE 'C';
CREATE FUNCTION "uint16Comparator"(uint16, uint16) RETURNS integer AS '$install_prefix/lib/libinfer_postgresql_uint16.so' LANGUAGE 'C';
CREATE FUNCTION "uint16Add"(uint16, uint16) RETURNS uint16 AS '$install_prefix/lib/libinfer_postgresql_uint16.so' LANGUAGE 'C';
CREATE FUNCTION "uint16Min"(uint16, uint16) RETURNS uint16 AS '$install_prefix/lib/libinfer_postgresql_uint16.so' LANGUAGE 'C';
CREATE FUNCTION "uint16Max"(uint16, uint16) RETURNS uint16 AS '$install_prefix/lib/libinfer_postgresql_uint16.so' LANGUAGE 'C';
CREATE OPERATOR < (LEFTARG = uint16, RIGHTARG = uint16, PROCEDURE = "uint16LessThan", COMMUTATOR = >, NEGATOR = >=);
CREATE OPERATOR <= (LEFTARG = uint16, RIGHTARG = uint16, PROCEDURE = "uint16LessThanOrEqualTo", COMMUTATOR = >=, NEGATOR = >);
CREATE OPERATOR = (LEFTARG = uint16, RIGHTARG = uint16, PROCEDURE = "uint16EqualTo", COMMUTATOR = =, NEGATOR = !=);
CREATE OPERATOR != (LEFTARG = uint16, RIGHTARG = uint16, PROCEDURE = "uint16NotEqualTo", COMMUTATOR = !=, NEGATOR = =);
CREATE OPERATOR >= (LEFTARG = uint16, RIGHTARG = uint16, PROCEDURE = "uint16GreaterThanOrEqualTo", COMMUTATOR = <=, NEGATOR = <);
CREATE OPERATOR > (LEFTARG = uint16, RIGHTARG = uint16, PROCEDURE = "uint16GreaterThan", COMMUTATOR = <, NEGATOR = <=);
CREATE OPERATOR + (LEFTARG = uint16, RIGHTARG = uint16, PROCEDURE = "uint16Add", COMMUTATOR = +);
CREATE OPERATOR CLASS "uint16Operators" DEFAULT FOR TYPE uint16 USING btree AS OPERATOR 1 <, OPERATOR 2 <=, OPERATOR 3 =, OPERATOR 4 >=, OPERATOR 5 >, FUNCTION 1 "uint16Comparator"(uint16, uint16);
CREATE AGGREGATE SUM(uint16) (SFUNC = "uint16Add", STYPE = uint16, INITCOND = '(0, 0)');
CREATE AGGREGATE MIN(uint16) (SFUNC = "uint16Min", STYPE = uint16, INITCOND = '65535');
CREATE AGGREGATE MAX(uint16) (SFUNC = "uint16Max", STYPE = uint16, INITCOND = '0');
CREATE FUNCTION "uint32In"(cstring) RETURNS uint32 AS '$install_prefix/lib/libinfer_postgresql_uint32.so' LANGUAGE 'C';
CREATE FUNCTION "uint32Out"(uint32) RETURNS cstring AS '$install_prefix/lib/libinfer_postgresql_uint32.so' LANGUAGE 'C';
CREATE FUNCTION "uint32Receive"(internal) RETURNS uint32 AS '$install_prefix/lib/libinfer_postgresql_uint32.so' LANGUAGE 'C';
CREATE FUNCTION "uint32Send"(uint32) RETURNS bytea AS '$install_prefix/lib/libinfer_postgresql_uint32.so' LANGUAGE 'C';
CREATE TYPE "uint32" (INPUT = "uint32In", OUTPUT = "uint32Out", RECEIVE = "uint32Receive", SEND = "uint32Send", INTERNALLENGTH = 4, EXTERNALLENGTH = VARIABLE, ALIGNMENT = INT4);
CREATE FUNCTION "uint32LessThan"(uint32, uint32) RETURNS bool AS '$install_prefix/lib/libinfer_postgresql_uint32.so' LANGUAGE 'C';
CREATE FUNCTION "uint32LessThanOrEqualTo"(uint32, uint32) RETURNS bool AS '$install_prefix/lib/libinfer_postgresql_uint32.so' LANGUAGE 'C';
CREATE FUNCTION "uint32EqualTo"(uint32, uint32) RETURNS bool AS '$install_prefix/lib/libinfer_postgresql_uint32.so' LANGUAGE 'C';
CREATE FUNCTION "uint32NotEqualTo"(uint32, uint32) RETURNS bool AS '$install_prefix/lib/libinfer_postgresql_uint32.so' LANGUAGE 'C';
CREATE FUNCTION "uint32GreaterThanOrEqualTo"(uint32, uint32) RETURNS bool AS '$install_prefix/lib/libinfer_postgresql_uint32.so' LANGUAGE 'C';
CREATE FUNCTION "uint32GreaterThan"(uint32, uint32) RETURNS bool AS '$install_prefix/lib/libinfer_postgresql_uint32.so' LANGUAGE 'C';
CREATE FUNCTION "uint32Comparator"(uint32, uint32) RETURNS integer AS '$install_prefix/lib/libinfer_postgresql_uint32.so' LANGUAGE 'C';
CREATE FUNCTION "uint32Add"(uint32, uint32) RETURNS uint32 AS '$install_prefix/lib/libinfer_postgresql_uint32.so' LANGUAGE 'C';
CREATE FUNCTION "uint32Subtract"(uint32, uint32) RETURNS uint32 AS '$install_prefix/lib/libinfer_postgresql_uint32.so' LANGUAGE 'C';
CREATE FUNCTION "uint32Min"(uint32, uint32) RETURNS uint32 AS '$install_prefix/lib/libinfer_postgresql_uint32.so' LANGUAGE 'C';
CREATE FUNCTION "uint32Max"(uint32, uint32) RETURNS uint32 AS '$install_prefix/lib/libinfer_postgresql_uint32.so' LANGUAGE 'C';
CREATE FUNCTION "uint32AddArray"(uint32[], uint32[]) RETURNS uint32[] as '$install_prefix/lib/libinfer_postgresql_uint32.so' LANGUAGE 'C';
CREATE OPERATOR < (LEFTARG = uint32, RIGHTARG = uint32, PROCEDURE = "uint32LessThan", COMMUTATOR = >, NEGATOR = >=);
CREATE OPERATOR <= (LEFTARG = uint32, RIGHTARG = uint32, PROCEDURE = "uint32LessThanOrEqualTo", COMMUTATOR = >=, NEGATOR = >);
CREATE OPERATOR = (LEFTARG = uint32, RIGHTARG = uint32, PROCEDURE = "uint32EqualTo", COMMUTATOR = =, NEGATOR = !=);
CREATE OPERATOR != (LEFTARG = uint32, RIGHTARG = uint32, PROCEDURE = "uint32NotEqualTo", COMMUTATOR = !=, NEGATOR = =);
CREATE OPERATOR >= (LEFTARG = uint32, RIGHTARG = uint32, PROCEDURE = "uint32GreaterThanOrEqualTo", COMMUTATOR = <=, NEGATOR = <);
CREATE OPERATOR > (LEFTARG = uint32, RIGHTARG = uint32, PROCEDURE = "uint32GreaterThan", COMMUTATOR = <, NEGATOR = <=);
CREATE OPERATOR + (LEFTARG = uint32, RIGHTARG = uint32, PROCEDURE = "uint32Add", COMMUTATOR = +);
CREATE OPERATOR - (LEFTARG = uint32, RIGHTARG = uint32, PROCEDURE = "uint32Subtract", COMMUTATOR = -);
CREATE OPERATOR CLASS "uint32Operators" DEFAULT FOR TYPE uint32 USING btree AS OPERATOR 1 <, OPERATOR 2 <=, OPERATOR 3 =, OPERATOR 4 >=, OPERATOR 5 >, FUNCTION 1 "uint32Comparator"(uint32, uint32);
CREATE AGGREGATE SUM(uint32) (SFUNC = "uint32Add", STYPE = uint32, INITCOND = '(0, 0)');
CREATE AGGREGATE MIN(uint32) (SFUNC = "uint32Min", STYPE = uint32, INITCOND = '4294967295');
CREATE AGGREGATE MAX(uint32) (SFUNC = "uint32Max", STYPE = uint32, INITCOND = '0');
CREATE AGGREGATE SUM(uint32[]) (SFUNC = "uint32AddArray", STYPE = uint32[], INITCOND = '{0}');
CREATE FUNCTION "uint64In"(cstring) RETURNS uint64 AS '$install_prefix/lib/libinfer_postgresql_uint64.so' LANGUAGE 'C';
CREATE FUNCTION "uint64Out"(uint64) RETURNS cstring AS '$install_prefix/lib/libinfer_postgresql_uint64.so' LANGUAGE 'C';
CREATE FUNCTION "uint64Receive"(internal) RETURNS uint64 AS '$install_prefix/lib/libinfer_postgresql_uint64.so' LANGUAGE 'C';
CREATE FUNCTION "uint64Send"(uint64) RETURNS bytea AS '$install_prefix/lib/libinfer_postgresql_uint64.so' LANGUAGE 'C';
CREATE TYPE "uint64" (INPUT = "uint64In", OUTPUT = "uint64Out", RECEIVE = "uint64Receive", SEND = "uint64Send", INTERNALLENGTH = 8, EXTERNALLENGTH = VARIABLE, ALIGNMENT = DOUBLE);
CREATE FUNCTION "uint64LessThan"(uint64, uint64) RETURNS bool AS '$install_prefix/lib/libinfer_postgresql_uint64.so' LANGUAGE 'C';
CREATE FUNCTION "uint64LessThanOrEqualTo"(uint64, uint64) RETURNS bool AS '$install_prefix/lib/libinfer_postgresql_uint64.so' LANGUAGE 'C';
CREATE FUNCTION "uint64EqualTo"(uint64, uint64) RETURNS bool AS '$install_prefix/lib/libinfer_postgresql_uint64.so' LANGUAGE 'C';
CREATE FUNCTION "uint64NotEqualTo"(uint64, uint64) RETURNS bool AS '$install_prefix/lib/libinfer_postgresql_uint64.so' LANGUAGE 'C';
CREATE FUNCTION "uint64GreaterThanOrEqualTo"(uint64, uint64) RETURNS bool AS '$install_prefix/lib/libinfer_postgresql_uint64.so' LANGUAGE 'C';
CREATE FUNCTION "uint64GreaterThan"(uint64, uint64) RETURNS bool AS '$install_prefix/lib/libinfer_postgresql_uint64.so' LANGUAGE 'C';
CREATE FUNCTION "uint64Comparator"(uint64, uint64) RETURNS integer AS '$install_prefix/lib/libinfer_postgresql_uint64.so' LANGUAGE 'C';
CREATE FUNCTION "uint64Add"(uint64, uint64) RETURNS uint64 AS '$install_prefix/lib/libinfer_postgresql_uint64.so' LANGUAGE 'C';
CREATE FUNCTION "uint64Min"(uint64, uint64) RETURNS uint64 AS '$install_prefix/lib/libinfer_postgresql_uint64.so' LANGUAGE 'C';
CREATE FUNCTION "uint64Max"(uint64, uint64) RETURNS uint64 AS '$install_prefix/lib/libinfer_postgresql_uint64.so' LANGUAGE 'C';
CREATE OPERATOR < (LEFTARG = uint64, RIGHTARG = uint64, PROCEDURE = "uint64LessThan", COMMUTATOR = >, NEGATOR = >=);
CREATE OPERATOR <= (LEFTARG = uint64, RIGHTARG = uint64, PROCEDURE = "uint64LessThanOrEqualTo", COMMUTATOR = >=, NEGATOR = >);
CREATE OPERATOR = (LEFTARG = uint64, RIGHTARG = uint64, PROCEDURE = "uint64EqualTo", COMMUTATOR = =, NEGATOR = !=);
CREATE OPERATOR != (LEFTARG = uint64, RIGHTARG = uint64, PROCEDURE = "uint64NotEqualTo", COMMUTATOR = !=, NEGATOR = =);
CREATE OPERATOR >= (LEFTARG = uint64, RIGHTARG = uint64, PROCEDURE = "uint64GreaterThanOrEqualTo", COMMUTATOR = <=, NEGATOR = <);
CREATE OPERATOR > (LEFTARG = uint64, RIGHTARG = uint64, PROCEDURE = "uint64GreaterThan", COMMUTATOR = <, NEGATOR = <=);
CREATE OPERATOR + (LEFTARG = uint64, RIGHTARG = uint64, PROCEDURE = "uint64Add", COMMUTATOR = +);
CREATE OPERATOR CLASS "uint64Operators" DEFAULT FOR TYPE uint64 USING btree AS OPERATOR 1 <, OPERATOR 2 <=, OPERATOR 3 =, OPERATOR 4 >=, OPERATOR 5 >, FUNCTION 1 "uint64Comparator"(uint64, uint64);
CREATE AGGREGATE SUM(uint64) (SFUNC = "uint64Add", STYPE = uint64, INITCOND = '(0, 0)');
CREATE AGGREGATE MIN(uint64) (SFUNC = "uint64Min", STYPE = uint64, INITCOND = '18446744073709551615');
CREATE AGGREGATE MAX(uint64) (SFUNC = "uint64Max", STYPE = uint64, INITCOND = '0');
CREATE SCHEMA "AccessControl" AUTHORIZATION ims;
CREATE TABLE "AccessControl".users ( uid serial NOT NULL, user_name text UNIQUE NOT NULL, sha256_password text NOT NULL, name text, privileges uint16, active boolean NOT NULL );
ALTER TABLE "AccessControl"."users" OWNER TO ims;
CREATE SCHEMA "ASExposure" AUTHORIZATION ims;
CREATE SCHEMA "ASReputations" AUTHORIZATION ims;
CREATE SCHEMA "BrowserStats" AUTHORIZATION ims;
CREATE SCHEMA "BrowserVersionStats" AUTHORIZATION ims;
CREATE SCHEMA "BruteForcers" AUTHORIZATION ims;
CREATE SCHEMA "CommChannels" AUTHORIZATION ims;
CREATE SCHEMA "Configuration" AUTHORIZATION ims;
CREATE TABLE "Configuration"."analysis" ("name" TEXT NOT NULL, "value" TEXT, "default" TEXT NOT NULL, "description" TEXT NOT NULL);
ALTER TABLE "Configuration"."analysis" OWNER TO ims;
CREATE SCHEMA "ConnectionSearchQueries" AUTHORIZATION ims;
CREATE SCHEMA "ContactsQueries" AUTHORIZATION ims;
CREATE SCHEMA "CountryReputations" AUTHORIZATION ims;
CREATE SCHEMA "DarkSpaceSources" AUTHORIZATION ims;
CREATE SCHEMA "DarkSpaceTargets" AUTHORIZATION ims;
CREATE SCHEMA "DataSize" AUTHORIZATION ims;
CREATE SCHEMA "DomainReputations" AUTHORIZATION ims;
CREATE SCHEMA "EvasiveTraffic" AUTHORIZATION ims;
CREATE SCHEMA "ExternalContacts" AUTHORIZATION ims;
CREATE SCHEMA "HBFQueries" AUTHORIZATION ims;
CREATE SCHEMA "HostASExposure" AUTHORIZATION ims;
CREATE SCHEMA "HostIndexPairs" AUTHORIZATION ims;
CREATE SCHEMA "HostIndexes" AUTHORIZATION ims;
CREATE SCHEMA "HostPairs" AUTHORIZATION ims;
CREATE SCHEMA "HostTraffic" AUTHORIZATION ims;
CREATE SCHEMA "HTTPDomainRelevance" AUTHORIZATION ims;
CREATE SCHEMA "HTTPHostRelevance" AUTHORIZATION ims;
CREATE SCHEMA "HTTPRequests" AUTHORIZATION ims;
CREATE SCHEMA "HTTPResponses" AUTHORIZATION ims;
CREATE SCHEMA "IPReputations" AUTHORIZATION ims;
CREATE SCHEMA "Indexes" AUTHORIZATION ims;
CREATE TABLE "Indexes"."connectionSearchQueries" (id text NOT NULL, name text, filter text, "dataStartTime" text, "dataEndTime" text, pid uint32, username text, "startTime" uint32, "pauseTime" uint32, "resumeTime" uint32, duration uint32, "timeLeft" uint32, "numResults" uint32, "percentComplete" uint16, status uint16, details text, PRIMARY KEY (id));
ALTER TABLE "Indexes"."connectionSearchQueries" OWNER TO ims;
CREATE TABLE "Indexes"."contactsQueries" ("id" text NOT NULL, "name" text, "queryIP" uint32, "queryStartTime" uint32, "queryEndTime" uint32, "pid" uint32, "username" text, "startTime" uint32, "pauseTime" uint32, "resumeTime" uint32, "duration" uint32, "timeLeft" uint32, "numResults" uint32, "percentComplete" uint16, "status" uint16, "details" text);
ALTER TABLE "Indexes"."contactsQueries" OWNER TO ims;
CREATE TABLE "Indexes"."hbfQueries" ("id" TEXT, "name" TEXT, "queryString" TEXT, "dataStartTime" uint32, "dataEndTime" uint32, "queryStringOffset" uint32, "queryStringLength" uint32, "matchLength" uint32, "protocol" uint16, "minSourceIP" uint32, "maxSourceIP" uint32, "minDestinationIP" uint32, "maxDestinationIP" uint32, "minSourcePort" uint16, "maxSourcePort" uint16, "minDestinationPort" uint16, "maxDestinationPort" uint16, "pid" uint32, "username" TEXT, "startTime" uint32, "pauseTime" uint32, "resumeTime" uint32, "duration" uint32, "timeLeft" uint32, "numResults" uint32, "percentComplete" uint16, "status" uint16, "details" TEXT, PRIMARY KEY ("id"));
ALTER TABLE "Indexes"."hbfQueries" OWNER TO ims;
CREATE TABLE "Indexes"."payloadQueries" (id text NOT NULL, name text, "queryString" text, "dataStartTime" text, "dataEndTime" text, "queryStringLength" uint32, "matchLength" uint32, filter text, pid uint32, username text, "startTime" uint32, "pauseTime" uint32, "resumeTime" uint32, duration uint32, status uint16, details text);
ALTER TABLE "Indexes"."payloadQueries" OWNER TO ims;
CREATE TABLE "Indexes"."searchQueries" (id text NOT NULL, name text, filter text, "dataStartTime" text, "dataEndTime" text, pid uint32, username text, "startTime" uint32, "pauseTime" uint32, "resumeTime" uint32, duration uint32, "timeLeft" uint32, "numResults" uint32, "percentComplete" uint16, status uint16, details text, PRIMARY KEY (id));
ALTER TABLE "Indexes"."searchQueries" OWNER TO ims;
CREATE TABLE "Indexes"."interestingIPDates" ("ip" uint32 NOT NULL, "dates" DATE[], PRIMARY KEY ("ip"));
ALTER TABLE "Indexes"."interestingIPDates" OWNER TO ims;
CREATE TABLE "Indexes"."interestingPorts" ("date" DATE, "interestingPorts" uint16[] NOT NULL);
ALTER TABLE "Indexes"."interestingPorts" OWNER TO ims;
CREATE SCHEMA "InfectedContacts" AUTHORIZATION ims;
CREATE SCHEMA "InfectedIPs" AUTHORIZATION ims;
CREATE SCHEMA "InterestingIPs" AUTHORIZATION ims;
CREATE SCHEMA "InternalContacts" AUTHORIZATION ims;
CREATE SCHEMA "LiveIPs" AUTHORIZATION ims;
CREATE SCHEMA "Maps" AUTHORIZATION ims;
CREATE TABLE "Maps"."asIPBlocks" ("asn" uint16 NOT NULL, "firstIP" uint32 NOT NULL, "lastIP" uint32 NOT NULL, PRIMARY KEY ("firstIP", "lastIP"));
ALTER TABLE "Maps"."asIPBlocks" OWNER TO ims;
CREATE TABLE "Maps"."asNames" ("asNumber" uint16, "asName" TEXT, "asDescription" TEXT, PRIMARY KEY ("asNumber"));
ALTER TABLE "Maps"."asNames" OWNER TO ims;
CREATE TABLE "Maps"."containedHosts" ("ip" uint32 NOT NULL, "time" uint32 NOT NULL, PRIMARY KEY ("ip"));
ALTER TABLE "Maps"."containedHosts" OWNER TO ims;
CREATE TABLE "Maps"."countryIPBlocks" ("countryNumber" SMALLINT NOT NULL, "firstIP" uint32 NOT NULL, "lastIP" uint32 NOT NULL);
ALTER TABLE "Maps"."countryIPBlocks" OWNER TO ims;
CREATE TABLE "Maps"."countryNames" ("countryNumber" SMALLINT, "countryCode" TEXT, "countryName" TEXT, PRIMARY KEY ("countryNumber"));
ALTER TABLE "Maps"."countryNames" OWNER TO ims;
CREATE TABLE "Maps"."infectedIPs" ("ip" uint32, "time" uint32, "name" TEXT, "reason" TEXT, "lastSeenTime" uint32, "asNumber" uint16, "countryNumber" SMALLINT);
ALTER TABLE "Maps"."infectedIPs" OWNER TO ims;
CREATE TABLE "Maps"."ldapCache" ("ip" TEXT, "hostname" TEXT, "mac" TEXT, "email" TEXT, "firstName" TEXT, "lastName" TEXT, "title" TEXT, "location" TEXT, "phoneNumber" TEXT, "department" TEXT, "time" uint32);
ALTER TABLE "Maps"."ldapCache" OWNER TO ims;
CREATE TABLE "Maps"."ldapConfiguration" ("serverURL" TEXT, "port" uint16, "relativeDistinguishedName" TEXT, "password" TEXT, "distinguishedName" TEXT, "ipField" TEXT, "hostnameField" TEXT, "macField" TEXT, "emailField" TEXT, "departmentField" TEXT, "firstNameField" TEXT, "lastNameField" TEXT, "titleField" TEXT, "locationField" TEXT, "phoneNumberField" TEXT, "cacheTTL" uint16);
ALTER TABLE "Maps"."ldapConfiguration" OWNER TO ims;
CREATE TABLE "Maps"."monitoredServices" ("name" TEXT, "ports" uint16[], "initiator" SMALLINT, PRIMARY KEY ("name"));
ALTER TABLE "Maps"."monitoredServices" OWNER TO ims;
CREATE TABLE "Maps"."nonDNSTrafficWhiteList" ("ipBlock" TEXT, "time" uint32, "name" TEXT, "comments" TEXT);
ALTER TABLE "Maps"."nonDNSTrafficWhiteList" OWNER TO ims;
CREATE TABLE "Maps"."protocolNames" ("protocolNumber" uint16, "protocolName" TEXT, PRIMARY KEY ("protocolNumber"));
ALTER TABLE "Maps"."protocolNames" OWNER TO ims;
CREATE TABLE "Maps"."serviceNames" ("port" uint16 NOT NULL, "protocol" SMALLINT NOT NULL, "name" TEXT, PRIMARY KEY ("port", "protocol", "name"));
ALTER TABLE "Maps"."serviceNames" OWNER TO ims;
CREATE TABLE "Maps"."whiteList" ("ip" uint32 NOT NULL, "time" uint32 NOT NULL, "name" TEXT, "comments" TEXT);
ALTER TABLE "Maps"."whiteList" OWNER TO ims;
CREATE SCHEMA "MutualContacts" AUTHORIZATION ims;
CREATE SCHEMA "NeighborReputations" AUTHORIZATION ims;
CREATE SCHEMA "NetworkTraffic" AUTHORIZATION ims;
CREATE SCHEMA "NonDNSTraffic" AUTHORIZATION ims;
CREATE SCHEMA "PayloadQueries" AUTHORIZATION ims;
CREATE SCHEMA "PortIPs" AUTHORIZATION ims;
CREATE SCHEMA "ProcessStats" AUTHORIZATION ims;
CREATE SCHEMA "RankWhiteList" AUTHORIZATION ims;
CREATE SCHEMA "Reboots" AUTHORIZATION ims;
CREATE SCHEMA "Roles" AUTHORIZATION ims;
CREATE SCHEMA "Scanners" AUTHORIZATION ims;
CREATE SCHEMA "SearchQueries" AUTHORIZATION ims;
CREATE SCHEMA "SeenIPs" AUTHORIZATION ims;
CREATE SCHEMA "SharedPrivateContacts" AUTHORIZATION ims;
CREATE SCHEMA "Slowdown" AUTHORIZATION ims;
CREATE SCHEMA "Stats" AUTHORIZATION ims;
CREATE TABLE "Stats".network (date TEXT, internal_ip_count uint32, external_ip_count uint32);
ALTER TABLE "Stats".network OWNER TO ims;
CREATE TABLE "Stats".bandwidth_utilization (interval_start uint32, ingress_bytes_per_second DOUBLE PRECISION, egress_bytes_per_second DOUBLE PRECISION);
ALTER TABLE "Stats".bandwidth_utilization OWNER TO ims;
CREATE TABLE "Stats".host_bandwidth_utilization (host uint32 NOT NULL, interval_start uint32 NOT NULL, ingress_bytes_per_second DOUBLE PRECISION, egress_bytes_per_second DOUBLE PRECISION, PRIMARY KEY (host, interval_start) );
ALTER TABLE "Stats".host_bandwidth_utilization OWNER TO ims;
CREATE SCHEMA "TopPorts" AUTHORIZATION ims;
CREATE SCHEMA "TypoSquatters" AUTHORIZATION ims;
CREATE SCHEMA "TypoSquatterContacts" AUTHORIZATION ims;
CREATE LANGUAGE plpgsql;
CREATE OR REPLACE FUNCTION expire_tables(the_schema TEXT, the_table TEXT) RETURNS VOID AS \$\$
  DECLARE
    r RECORD;
    sql TEXT;
  
  BEGIN
    FOR r IN SELECT table_name FROM information_schema.tables WHERE table_schema = the_schema AND table_name <= the_table LOOP
    
       -- Insert the title of a matching book into the text_output
       -- variable, followed by a newline.
      sql := 'DROP TABLE ' || quote_ident(the_schema) || '.' || quote_ident(r.table_name);
      EXECUTE sql;
    END LOOP;
  END;
\$\$ LANGUAGE 'plpgsql';
EOF

psql -q "user = ims password = $pgsql_infer_pass dbname = IMS" << EOF 
INSERT INTO "AccessControl"."users" VALUES (default, '$frontend_admin_user', '$(sha256 -q -s "$frontend_admin_pass")', NULL, '65535', 't');
INSERT INTO "Configuration"."analysis" VALUES ('udpTimeout', NULL, 30, 'UDP timeout for connection initiator API');
INSERT INTO "Configuration"."analysis" VALUES ('localNetworks', NULL, '$monitored_networks', 'monitored networks');
INSERT INTO "Configuration"."analysis" VALUES ('dbHome', NULL, '$sensor_data_dir', 'sensor data directory');
INSERT INTO "Configuration"."analysis" VALUES ('startHour', NULL, 0, 'start of hour range for analysis');
INSERT INTO "Configuration"."analysis" VALUES ('endHour', NULL, 24, 'end of hour range for analysis (non-inclusive)');
INSERT INTO "Configuration"."analysis" VALUES ('querySockTimeout', NULL, 600, 'socket timeout for connection to query manager');
INSERT INTO "Configuration"."analysis" VALUES ('querySockPath', NULL, '/tmp/imsQueryManager.sock', 'Path to query manager UNIX socket');
EOF

psql -q "user = ims password = $pgsql_infer_pass dbname = IMS" \
	< /usr/local/share/www/scripts/data/asIPBlocks.sql

psql -q "user = ims password = $pgsql_infer_pass dbname = IMS" \
	< /usr/local/share/www/scripts/data/asIPNames.sql

psql -q "user = ims password = $pgsql_infer_pass dbname = IMS" \
	< /usr/local/share/www/scripts/data/countryNames.sql

psql -q "user = ims password = $pgsql_infer_pass dbname = IMS" \
	< /usr/local/share/www/scripts/data/monitoredServices.sql

psql -q "user = ims password = $pgsql_infer_pass dbname = IMS" \
	< /usr/local/share/www/scripts/data/protocolNames.sql

psql -q "user = ims password = $pgsql_infer_pass dbname = IMS" \
	< /usr/local/share/www/scripts/data/serviceNames.sql

psql -q "user = ims password = $pgsql_infer_pass dbname = IMS" \
	< /usr/local/share/www/scripts/data/role_names.sql
