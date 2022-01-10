#!/usr/bin/bash
export PDSH_RCMD_TYPE=ssh
export HADOOP_HOME=$(pwd)/hadoop-3.3.1
hdfs=$HADOOP_HOME/bin/hdfs


# stop daemons
$HADOOP_HOME/sbin/stop-all.sh

# remove all files
rm -rf /tmp/hadoop-*
