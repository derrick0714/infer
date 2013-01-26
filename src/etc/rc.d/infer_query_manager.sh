#!/bin/sh

infer_loop="/usr/local/etc/rc.d/infer_loop.sh"
user="analysis"
binary="/usr/local/bin/infer_query_manager"

export TERM=cons25

if [ $# = 0 ]; then
  echo "Usage: $0 start"
  return 1
fi

if [ $1 = "start" ]; then
  su - $user -c "/usr/local/bin/screen -d -m /bin/sh $infer_loop $binary"
fi
