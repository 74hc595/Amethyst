#!/bin/sh

SECTION=$1
shift
echo "OVERLAY ADDR(.${SECTION}) + SIZEOF(.${SECTION}) : {"
for app in $@
do
  echo ".app_${app}_${SECTION} { KEEP(build/apps/${app}/*(.${SECTION}*)) }"
done
echo "} > data"
