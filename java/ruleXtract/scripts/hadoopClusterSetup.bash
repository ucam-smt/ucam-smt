#!/bin/bash

################################################################
## installs a single node pseudo-distributed hadoop cluster
## in current directory as per
## http://hadoop.apache.org/docs/r1.2.1/single_node_setup.html
################################################################

### requirements
# linux, preferably 64-bit; this was tried on ubuntu 12.04 x86_64
# java 1.7+ (1.6 is enough to install the cluster but not to run ruleXtract code),
# but this script installs a java version in current directory
# if java 1.7+ is not already installed

### java checking (modified from http://stackoverflow.com/questions/7334754/correct-way-to-check-java-version-from-bash-script)
downloadJava=0
if type -p java; then
    echo found java executable in PATH > /dev/stderr
    _java=$(type -p java)
elif [[ -n "$JAVA_HOME" ]] && [[ -x "$JAVA_HOME/bin/java" ]];  then
    echo found java executable in JAVA_HOME > /dev/stderr
    _java="$JAVA_HOME/bin/java"
else
    echo "no java" > /dev/stderr
    downloadJava=1
fi

if [[ "$_java" ]]; then
    version=$("$_java" -version 2>&1 | awk -F '"' '/version/ {print $2}')
    echo version "$version" > /dev/stderr
    if [[ "$version" > "1.7" ]]; then
        echo version is more than 1.7 > /dev/stderr
    else
        echo version is less than 1.7 > /dev/stderr
	downloadJava=1
    fi
fi

if [ $downloadJava == 0 ]; then
    JAVA_HOME=$(dirname $(dirname $(readlink -f $_java)))
else
    ### download a recent java version (here: 1.8)
    ### workaround to automate download with wget from http://stackoverflow.com/questions/10268583/how-to-automate-download-and-installation-of-java-jdk-on-linux
    ### if this doesn't work, just use a browser
    echo "Downloading java..." > /dev/stderr
    wget --no-check-certificate --no-cookies --header "Cookie: oraclelicense=accept-securebackup-cookie" -O jdk-8u5-linux-x64.tar.gz http://download.oracle.com/otn-pub/java/jdk/8u5-b13/jdk-8u5-linux-x64.tar.gz
    tar -xzf jdk-8u5-linux-x64.tar.gz
    JAVA_HOME=$PWD/jdk1.8.0_05
fi

### download hadoop
echo "Downloading hadoop..." > /dev/stderr
wget http://mirror.catn.com/pub/apache/hadoop/common/stable1/hadoop-1.2.1.tar.gz
tar -xzf hadoop-1.2.1.tar.gz

### download libraries
wget http://search.maven.org/remotecontent?filepath=com/beust/jcommander/1.35/jcommander-1.35.jar -O jcommander-1.35.jar
JCOMMANDER_JAR=$PWD/jcommander-1.35.jar

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
### and add libraries needed by rulextract to the class path
cp hadoop-1.2.1/conf/hadoop-env.sh hadoop-1.2.1/conf/hadoop-env.sh.bak
cat hadoop-1.2.1/conf/hadoop-env.sh.bak \
    | sed  "\;export JAVA_HOME=; a\export JAVA_HOME=$JAVA_HOME" \
    | sed  "\;export HADOOP_CLASSPATH=; a\export HADOOP_CLASSPATH=$JCOMMANDER_JAR" \
    > hadoop-1.2.1/conf/hadoop-env.sh

### set up passwordless and passphraseless ssh
ssh-keygen -t dsa -P '' -f ~/.ssh/id_dsa
cat ~/.ssh/id_dsa.pub >> ~/.ssh/authorized_keys
ssh-add

### format HDFS
hadoop-1.2.1/bin/hadoop namenode -format

### start Hadoop daemons
hadoop-1.2.1/bin/start-all.sh

### test a simple Hadoop command
echo "Sleeping a bit to wait for the cluster to be ready..." > /dev/stderr
sleep 5
echo "Testing the 'ls' hadoop command..." > /dev/stderr
hadoop-1.2.1/bin/hadoop fs -ls /
echo "You should see a '/tmp' directory listed."

### making a user directory
echo "Making a user directory"
hadoop-1.2.1/bin/hadoop fs -mkdir /user/$(whoami)

### shut down the cluster
echo "Shutting down the cluster. To restart, run hadoop-1.2.1/bin/start-all.sh"
hadoop-1.2.1/bin/stop-all.sh
