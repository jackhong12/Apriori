#!/usr/bin/bash
export PDSH_RCMD_TYPE=ssh
export HADOOP_HOME=$(pwd)/hadoop-3.3.1
alias hdfs=$HADOOP_HOME/bin/hdfs

# 1. format the filesystem
$HADOOP_HOME/bin/hdfs namenode -format

# 2. start NameNode daemon and DataNode daemon
$HADOOP_HOME/sbin/start-dfs.sh

# 3. start all nodes
$HADOOP_HOME/sbin/start-all.sh

# show active nodes
echo "\033[32mActive Nodes:\033[39m"
jps
