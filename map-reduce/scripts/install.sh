sudo apt-get install ssh wget -y
wget https://dlcdn.apache.org/hadoop/common/hadoop-3.3.1/hadoop-3.3.1-aarch64.tar.gz
tar -zxvf  hadoop-3.3.1-aarch64.tar.gz
sed -i 's/#.*\(export JAVA_HOME=\).*/\1\/usr/g' hadoop-3.3.1/etc/hadoop/hadoop-env.sh

cp config/*.xml hadoop-3.3.1/etc/hadoop
echo "log4j.logger.org.apache.hadoop.util.NativeCodeLoader=ERROR" >> hadoop-3.3.1/etc/hadoop/log4j.properties

echo "export HADOOP_HOME=$(pwd)/hadoop-3.3.1" >> ~/.bashrc
echo "export PDSH_RCMD_TYPE=ssh" >> ~/.bashrc
echo "alias hdfs=$HADOOP_HOME/bin/hdfs" >> ~/.bashrc
