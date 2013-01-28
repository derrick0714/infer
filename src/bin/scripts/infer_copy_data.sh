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

copy_data_source=$(/usr/local/bin/infer_config copy-data-source)
copy_data_prefixes=$(/usr/local/bin/infer_config copy-data-prefixes)
data_directory=$(/usr/local/bin/infer_config data-directory)

echo "Date: $date"

start_unix=$(date -jf "%Y-%m-%d %H:%M:%S" "$date 00:00:00" "+%s")
end_unix=$(date -j -v+1d -f "%Y-%m-%d %H:%M:%S" "$date 00:00:00" "+%s")

cur_unix=$start_unix
while [ "$cur_unix" != "$end_unix" ]; do
	date -ujf "%s" "$cur_unix" "+%Y-%m-%d %H:%M:%S..."

	year=$(date -ujf "%s" "$cur_unix" "+%Y")
	month=$(date -ujf "%s" "$cur_unix" "+%m")
	day=$(date -ujf "%s" "$cur_unix" "+%d")
	hour=$(date -ujf "%s" "$cur_unix" "+%H")

	dst="$data_directory/$year/$month/$day/"

	if [ ! -d "$dst" ]; then
		mkdir -p "$dst"
	fi
	for i in $copy_data_prefixes; do
		if [ ! -e "$dst/${i}_$hour" ]; then
			echo -e "\t${i}_$hour"
			src="$copy_data_source/$year/$month/$day/${i}_$hour"
			cp "$src" "$dst"
		fi
	done

	cur_unix=$(date -j -v+1H -r $cur_unix "+%s")
done
