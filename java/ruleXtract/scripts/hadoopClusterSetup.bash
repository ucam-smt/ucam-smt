#!/bin/bash

################################################################
## installs a single node pseudo-distributed hadoop cluster
## in current directory as per
## http://hadoop.apache.org/docs/r1.2.1/single_node_setup.html
################################################################

### download a recent java version (here: 1.8)
### workaround to automate download with wget from http://stackoverflow.com/questions/10268583/how-to-automate-download-and-installation-of-java-jdk-on-linux
### if this doesn't work, just use a browser
wget --no-check-certificate --no-cookies --header "Cookie: oraclelicense=accept-securebackup-cookie" -O jdk-8u5-linux-x64.tar.gz http://download.oracle.com/otn-pub/java/jdk/8u5-b13/jdk-8u5-linux-x64.tar.gz
tar -xzf jdk-8u5-linux-x64.tar.gz

### download hadoop
wget http://mirror.catn.com/pub/apache/hadoop/common/stable1/hadoop-1.2.1.tar.gz
tar -xzf hadoop-1.2.1.tar.gz

### modify config files for pseudo-distributed setup
cp hadoop-1.2.1/conf/core-site.xml hadoop-1.2.1/conf/core-site.xml.bak
cat hadoop-1.2.1/conf/core-site.xml.bak \
    | sed '\;<configuration>; a\\t<property>\n\t\t<name>fs.default.name</name>\n\t\t<value>hdfs://localhost:9000</value>\n\t</property>' \
    > hadoop-1.2.1/conf/core-site.xml

cp hadoop-1.2.1/conf/hdfs-site.xml hadoop-1.2.1/conf/hdfs-site.xml.bak
cat hadoop-1.2.1/conf/hdfs-site.xml.bak \
    | sed '\;<configuration>; a\\t<property>\n\t\t<name>dfs.replication</name>\n\t\t<value>1</value>\n\t</property>' \
    > hadoop-1.2.1/conf/hdfs-site.xml

cp hadoop-1.2.1/conf/mapred-site.xml hadoop-1.2.1/conf/mapred-site.xml.bak
cat hadoop-1.2.1/conf/mapred-site.xml.bak \
    | sed '\;<configuration>; a\\t<property>\n\t\t<name>mapred.job.tracker</name>\n\t\t<value>localhost:9001</value>\n\t</property>' \
    > hadoop-1.2.1/conf/mapred-site.xml

### modify config file for java version
cp hadoop-1.2.1/conf/hadoop-env.sh hadoop-1.2.1/conf/hadoop-env.sh.bak
cat hadoop-1.2.1/conf/hadoop-env.sh.bak \
    | sed  "\;export JAVA_HOME=; a\export JAVA_HOME=$PWD/jdk1.8.0_05" \
    > hadoop-1.2.1/conf/hadoop-env.sh

### set up passwordless and passphraseless ssh
ssh-keygen -t dsa -P '' -f ~/.ssh/id_dsa
cat ~/.ssh/id_dsa.pub >> ~/.ssh/authorized_keys

### format HDFS
hadoop-1.2.1/bin/hadoop namenode -format

### start Hadoop daemons
hadoop-1.2.1/bin/start-all.sh
