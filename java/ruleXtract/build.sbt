lazy val root = (project in file(".")).
  configs(IntegrationTest).
  settings(Defaults.itSettings: _*)

name := "ruleXtract"

version := "1.0"

organization := "University of Cambridge"

EclipseKeys.withSource := true

// uncomment this for debugging
// javacOptions += "-g"

libraryDependencies ++= Seq(
		    "com.beust" % "jcommander" % "1.35",
		    "org.apache.hadoop" % "hadoop-common" % "2.6.0" intransitive(),
		    "org.apache.hadoop" % "hadoop-mapreduce-client-core" % "2.6.0"  intransitive(),
		    "org.apache.hadoop" % "hadoop-mapreduce-client-common" % "2.6.0"  intransitive(),
		    "org.apache.hadoop" % "hadoop-annotations" % "2.6.0" intransitive(), 
		    "org.apache.hadoop" % "hadoop-auth" % "2.6.0" intransitive(),
		    "org.apache.hadoop" % "hadoop-yarn-common" % "2.6.0" intransitive(),
		    "org.apache.hadoop" % "hadoop-yarn-api" % "2.6.0",
		    "org.apache.hbase" % "hbase-server" % "0.96.2-hadoop2" intransitive(), 
		    "org.apache.hbase" % "hbase-common" % "0.96.2-hadoop2"  intransitive(), 
		    "org.apache.hbase" % "hbase-client" % "0.96.2-hadoop2" intransitive(),
                    "org.apache.hbase" % "hbase-protocol" % "0.96.2-hadoop2",
		    "org.cloudera.htrace" % "htrace-core" % "2.04" intransitive(),
		    "commons-configuration" % "commons-configuration" % "1.6" exclude("commons-beanutils", "commons-beanutils-core"),
		    "commons-httpclient" % "commons-httpclient" % "3.1",
		    "commons-io" % "commons-io" % "2.4",
		    "com.google.guava" % "guava" % "11.0.2",
		    "com.google.protobuf" % "protobuf-java" % "2.5.0",
		    "log4j" % "log4j" % "1.2.16",
		    "org.slf4j" % "slf4j-log4j12" % "1.7.5",
		    "org.codehaus.jackson" % "jackson-jaxrs" % "1.9.13",
		    "org.apache.avro" % "avro" % "1.7.4",
		    "junit" % "junit" % "4.11" % "it,test",
		    "com.novocode" % "junit-interface" % "0.10" % "it,test",
		    "com.jsuereth" % "scala-arm_2.11" % "1.4" % "it,test",
		    "org.scalatest" % "scalatest_2.11" % "2.2.1" % "it,test"
		    )

assemblyMergeStrategy in assembly := {
	case PathList("org", "apache", "hadoop", "yarn", xs @ _*) => MergeStrategy.first 
	case x => val oldStrategy = (assemblyMergeStrategy in assembly).value
    oldStrategy(x)
}

assemblyExcludedJars in assembly := { 
  val cp = (fullClasspath in assembly).value
  cp filter {_.data.getName == "commons-beanutils-1.7.0.jar"}
}

scalaVersion := "2.11.4"

// output jar is here: target/ruleXtract.jar
assemblyOutputPath in assembly := file("target/ruleXtract.jar") 

// we want a jar without a main class so we can run it as "hadoop jar class args"
mainClass in (Compile, packageBin) := None
