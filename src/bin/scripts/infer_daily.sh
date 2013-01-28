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
start_unix=$(date -jf "%Y-%m-%d %H:%M:%S" "$date 00:00:00" "+%s")
end_unix=$(date -j -v+1d -f "%Y-%m-%d %H:%M:%S" "$date 00:00:00" "+%s")


http2psql_enable=$(/usr/local/bin/infer_config http2psql_enable)
http2psql_expire=$(/usr/local/bin/infer_config http2psql.expire)
http2psql_request_schema=$(/usr/local/bin/infer_config http2psql.request-schema)
http2psql_response_schema=$(/usr/local/bin/infer_config http2psql.response-schema)

mutual_contacts_enable=$(/usr/local/bin/infer_config mutual_contacts_enable)
mutual_contacts_schema=$(/usr/local/bin/infer_config mutual_contacts.mutual-contacts-schema)

privacy_graph_enable=$(/usr/local/bin/infer_config privacy_graph_enable)
privacy_graph_dir=$(/usr/local/bin/infer_config privacy-graph-dir)

pruned_network_graph_enable=$(/usr/local/bin/infer_config pruned_network_graph_enable)
pruned_network_graph_dir=$(/usr/local/bin/infer_config pruned-network-graph-dir)

external_contacts_enable=$(/usr/local/bin/infer_config external_contacts_enable)
external_contacts_schema=$(/usr/local/bin/infer_config external_contacts.external-contacts-schema)

internal_contacts_enable=$(/usr/local/bin/infer_config internal_contacts_enable)
internal_contacts_schema=$(/usr/local/bin/infer_config internal_contacts.internal-contacts-schema)

copy_data_enable=$(/usr/local/bin/infer_config copy_data_enable)

smtp_notify_enable=$(/usr/local/bin/infer_config smtp_notify_enable)

pg_user=$(/usr/local/bin/infer_config postgresql.user)
pg_pass=$(/usr/local/bin/infer_config postgresql.password)
pg_db=$(/usr/local/bin/infer_config postgresql.dbname)


http2psql_expire_date=$(date -u -v -$http2psql_expire "+%F")
if [ $? != 0 ]
then
	echo "$0: invalid expire period \"$http2psql_expire\""
	exit 1
fi

/usr/local/bin/psql -q "user = $pg_user password = $pg_pass dbname = $pg_db" << EOF
SELECT expire_tables('$http2psql_request_schema', '$http2psql_expire_date');
SELECT expire_tables('$http2psql_response_schema', '$http2psql_expire_date');
EOF

if [ "$copy_data_enable" = "YES" ]; then
	/usr/local/bin/psql -q "user = $pg_user password = $pg_pass dbname = $pg_db" << EOF
DROP TABLE IF EXISTS "$http2psql_request_schema"."$date";
DROP TABLE IF EXISTS "$http2psql_response_schema"."$date";
EOF

	/usr/local/bin/infer_copy_data.sh $date || exit 1

	if [ "$http2psql_enable" = "YES" ]
	then
		start=$(date -ujf "%s" "$start_unix" "+%Y-%m-%d %H:00:00")
		end=$(date -ujf "%s" "$end_unix" "+%Y-%m-%d %H:00:00")
		/usr/local/bin/infer_http2psql --start-time "$start" --end-time "$end"
	fi
fi

/usr/local/bin/infer_analysis.sh $date || exit 1

if [ "$mutual_contacts_enable" = "YES" ]
then
	/usr/local/bin/psql -q "user = $pg_user password = $pg_pass dbname = $pg_db" << EOF
DROP TABLE IF EXISTS "$mutual_contacts_schema"."$date";
CREATE TABLE "$mutual_contacts_schema"."$date" (ip uint32, related_ip uint32, score DOUBLE PRECISION, shared_contacts uint32);
EOF

	/usr/local/bin/infer_mutual_contacts --decimal $date 0.0.0.0 | /usr/local/bin/psql -q "user = $pg_user password = $pg_pass dbname = $pg_db" -c "\\copy \"$mutual_contacts_schema\".\"$date\" from STDIN"
fi

if [ "$privacy_graph_enable" = "YES" ]
then
	/usr/local/bin/infer_privacy_matrix -o "$privacy_graph_dir/$date" $date
fi

if [ "$pruned_network_graph_enable" = "YES" ]
then
	/usr/local/bin/infer_pruned_network_graph -o "$pruned_network_graph_dir/$date" $date
fi

if [ "$external_contacts_enable" = "YES" ]
then
	/usr/local/bin/psql -q "user = $pg_user password = $pg_pass dbname = $pg_db" << EOF
DROP TABLE IF EXISTS "$external_contacts_schema"."$date";
CREATE TABLE "$external_contacts_schema"."$date" (external_ip uint32, internal_host_count uint32);
EOF

	/usr/local/bin/infer_external_contacts --decimal $date | /usr/local/bin/psql -q "user = $pg_user password = $pg_pass dbname = $pg_db" -c "\\copy \"$external_contacts_schema\".\"$date\" from STDIN"
	/usr/local/bin/infer_comm_channel_fanin $date

	/usr/local/bin/psql -q "user = $pg_user password = $pg_pass dbname = $pg_db" << EOF
ALTER TABLE "$external_contacts_schema"."$date" ADD PRIMARY KEY (external_ip);
EOF

	/usr/local/bin/infer_update_portip_relevance.pl $date
fi

if [ "$internal_contacts_enable" = "YES" ]
then
	/usr/local/bin/psql -q "user = $pg_user password = $pg_pass dbname = $pg_db" << EOF
DROP TABLE IF EXISTS "$internal_contacts_schema"."$date";
CREATE TABLE "$internal_contacts_schema"."$date" (internal_ip uint32, external_host_count uint32);
EOF

	/usr/local/bin/infer_internal_contacts --decimal $date | /usr/local/bin/psql -q "user = $pg_user password = $pg_pass dbname = $pg_db" -c "\\copy \"$internal_contacts_schema\".\"$date\" from STDIN"

	/usr/local/bin/psql -q "user = $pg_user password = $pg_pass dbname = $pg_db" << EOF
ALTER TABLE "$internal_contacts_schema"."$date" ADD PRIMARY KEY (internal_ip);
EOF

fi

if [ "$smtp_notify_enable" = "YES" ]
then
	/usr/local/bin/infer_smtp_notify.pl $date
fi
