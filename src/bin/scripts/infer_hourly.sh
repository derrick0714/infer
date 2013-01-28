#!/bin/sh

http2psql_enable=$(/usr/local/bin/infer_config http2psql_enable)

if [ "$http2psql_enable" = "YES" ]
then
	start=$(date -u -v -2H "+%Y-%m-%d %H:00:00")
	end=$(date -u -v -1H "+%Y-%m-%d %H:00:00")
	/usr/local/bin/infer_http2psql --start-time "$start" --end-time "$end"
fi
