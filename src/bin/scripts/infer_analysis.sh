#!/bin/sh

browser_stats_enable=$(/usr/local/bin/infer_config browser_stats_enable)

pg_user=$(/usr/local/bin/infer_config postgresql.user)
pg_pass=$(/usr/local/bin/infer_config postgresql.password)
pg_db=$(/usr/local/bin/infer_config postgresql.dbname)


usage()
{
	echo "Usage: $0 [YYYY-mm-dd]"
}

fail()
{
	if [ "$1" != false ]
	then
		/usr/local/bin/psql -q "user = $pg_user password = $pg_pass dbname = $pg_db" << EOF
UPDATE "ProcessStats"."$date" SET "crashed" = 't' WHERE "processName" = '$1';
DROP TABLE IF EXISTS "InterestingIPs"."$date";
EOF
	fi

	echo "$0: $1 failed"
	exit 1
}

if [ $# != 0 -a $# != 1 ]
then
	usage
	exit 1
fi

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
start_time=$(date -j -u -f "%s" $(date -j -f "%F %H:%M:%S" "$date 00:00:00" "+%s") "+%F %H:%M:%S")
end_time=$(date -j -u -f "%s" $(date -j -v +1d -f "%F %H:%M:%S" "$date 00:00:00" "+%s") "+%F %H:%M:%S")

echo Date: $date

/usr/local/bin/infer_analysis $date || fail infer_analysis
/usr/local/bin/infer_interesting_ips $date || fail infer_interesting_ips
/usr/local/bin/infer_index_symptom_tables.sh $date
/usr/local/bin/infer_http_domain_relevance.pl $date
/usr/local/bin/infer_http_host_relevance.pl $date

if [ "$browser_stats_enable" = "YES" ]
then
	/usr/local/bin/infer_browser_stats --start-time "$start_time" --end-time "$end_time"
fi

/usr/local/bin/infer_web_server_browsers --start-time "$start_time" --end-time "$end_time"
/usr/local/bin/infer_web_server_crawlers --start-time "$start_time" --end-time "$end_time"
/usr/local/bin/infer_web_server_servers --start-time "$start_time" --end-time "$end_time"
/usr/local/bin/infer_web_server_top_hosts --start-time "$start_time" --end-time "$end_time"
/usr/local/bin/infer_web_server_top_urls --start-time "$start_time" --end-time "$end_time"


/usr/local/bin/psql -q "user = $pg_user password = $pg_pass dbname = $pg_db" << EOF
DROP INDEX IF EXISTS "HTTPRequests"."${date}_source_ip_destination_ip";
DROP INDEX IF EXISTS "HTTPResponses"."${date}_destination_ip_source_ip";
CREATE INDEX "${date}_source_ip_destination_ip" ON "HTTPRequests"."$date" (source_ip, destination_ip);
CREATE INDEX "${date}_destination_ip_source_ip" ON "HTTPResponses"."$date" (destination_ip, source_ip);
EOF
