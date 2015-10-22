#!/bin/bash
DIR=`dirname $0`
MEM=${RETRIEVE_MEM:-70G}
SCALA=${SCALA_BIN:-scala}
echo "Using ruleXtract jar: $DIR/../target/ruleXtract.jar"
exec $SCALA -nc -J-Xms"$MEM" -J-Xmx"$MEM" -classpath "$DIR/../target/ruleXtract.jar" $0 $@
!#

import java.io.File
import com.beust.jcommander.{JCommander, Parameter}

import uk.ac.cam.eng.extraction.hadoop.util.Util
import uk.ac.cam.eng.util.CLI
import uk.ac.cam.eng.extraction.hadoop.features.lexical.TTableServer
import uk.ac.cam.eng.rule.retrieval.RuleRetriever

def startTTableServer(params : CLI.TTableServerParameters) = {
	val server = new TTableServer
	server.setup(params)
	server.startServer
}

object Args {
    @Parameter( names = Array("--s2t_language_pair"), description = "Language pair for s2t", required = true)
    var s2tlp: String = null

    @Parameter( names = Array("--t2s_language_pair"), description = "Language pair for t2s", required = true)
    var t2slp: String = null
}

Util.parseCommandLine(args, Args)

val params = new CLI.TTableServerParameters()
Util.parseCommandLine("--ttable_language_pair none --ttable_direction none".split(" ") ++ args, params)
params.ttableDirection = "s2t" 
params.ttableLanguagePair = Args.s2tlp
startTTableServer(params)
params.ttableDirection = "t2s" 
params.ttableLanguagePair = Args.t2slp
startTTableServer(params)

RuleRetriever.main(args)
// We need an explicit exit because of the threadpools in the ttable servers
System.exit(0)