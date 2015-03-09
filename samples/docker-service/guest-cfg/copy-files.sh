#!/bin/bash

CONFIG=$1

if [ -d "$CONFIG" ]; then
   echo "Using config: $CONFIG"  
else
   echo "Please choose configuration"
   echo "usage: ./copy-config.sh [memex|xdata]"
   exit 0
fi

cp -f $CONFIG/hdfs-site.xml /usr/lib/hadoop/etc/hadoop/.
cp -f $CONFIG/core-site.xml /usr/lib/hadoop/etc/hadoop/.
cp -f $CONFIG/mapred-site.xml /usr/lib/hadoop/etc/hadoop/.
cp -f $CONFIG/yarn-site.xml /usr/lib/hadoop/etc/hadoop/.
cp -f $CONFIG/hive-site.xml /usr/lib/hive/conf/.

if [ -a "$CONFIG/resolv.conf" ]; then
  cp /etc/resolv.conf /etc/resolv.conf.old
  cp -f $CONFIG/resolv.conf /etc/resolv.conf
fi

if [ -a "$CONFIG/hosts" ]; then
  cp /etc/hosts /etc/hosts.old
  cp -f $CONFIG/hosts /etc/hosts
fi
