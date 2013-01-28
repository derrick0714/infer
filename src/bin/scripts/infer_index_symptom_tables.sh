#!/bin/sh

if [ $# = 0 ]
then
	# yesterday by default
	date=$(date -j -v-1d "+%F")
else
	date=$(date -j -f "%F" "$1" "+%F" 2>/dev/null)
	if [ $? != 0 ]
	then
		echo "$0: invalid date \"$1\""
		usage
		exit 1
	fi
fi

pg_user=$(/usr/local/bin/infer_config postgresql.user)
pg_pass=$(/usr/local/bin/infer_config postgresql.password)
pg_db=$(/usr/local/bin/infer_config postgresql.dbname)

infected_contacts_schema=$(/usr/local/bin/infer_config infected_contacts_schema)
evasive_traffic_schema=$(/usr/local/bin/infer_config evasive_traffic_schema)
dark_space_sources_schema=$(/usr/local/bin/infer_config dark_space_sources_schema)
dark_space_targets_schema=$(/usr/local/bin/infer_config dark_space_targets_schema)
protocol_violations_schema=$(/usr/local/bin/infer_config protocol_violations_schema)


/usr/local/bin/psql -q "user = $pg_user password = $pg_pass dbname = $pg_db" << EOF
DROP INDEX IF EXISTS "${infected_contacts_schema}"."${date}_internalIP";
DROP INDEX IF EXISTS "${evasive_traffic_schema}"."${date}_internalIP";
DROP INDEX IF EXISTS "${dark_space_sources_schema}"."${date}_sourceIP";
DROP INDEX IF EXISTS "${dark_space_targets_schema}"."${date}_internalIP";
DROP INDEX IF EXISTS "${protocol_violations_schema}"."${date}_internalIP";
CREATE INDEX "${date}_internalIP" ON "${infected_contacts_schema}"."${date}" ("internalIP");
CREATE INDEX "${date}_internalIP" ON "${evasive_traffic_schema}"."${date}" ("internalIP");
CREATE INDEX "${date}_sourceIP" ON "${dark_space_sources_schema}"."${date}" ("sourceIP");
CREATE INDEX "${date}_internalIP" ON "${dark_space_targets_schema}"."${date}" ("internalIP");
CREATE INDEX "${date}_internalIP" ON "${protocol_violations_schema}"."${date}" ("internalIP");
EOF
