#!/bin/bash

cd $1

echo "PWD: `pwd`"

FILES_MAIN=`ls sym_main_* |  egrep -v '\.'`
FILES_VIEW=`ls sym_view_* |  egrep -v '\.'`

for f in $FILES_MAIN; do
  strip $f
  NNAME=`echo $f | sed 's/_/ /g' | awk '{print $3}'`
  mv $f $NNAME
done

for f in $FILES_VIEW; do
  strip $f
  NNAME=`echo $f | sed 's/_/ /g' | awk '{print $3}'`
  mv $f ${NNAME}V
done

