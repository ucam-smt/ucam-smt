#!/bin/bash
DIR=`dirname $0`
MEM=${RETRIEVE_MEM:-70G}
SCALA=${SCALA_BIN:-scala}
echo "Using ruleXtract jar: $DIR/../target/ruleXtract.jar"
exec $SCALA -nc -J-Xms"$MEM" -J-Xmx"$MEM" -classpath "$DIR/../target/ruleXtract.jar" $0 $@
!#

/*
 * Utility script to help remove the complexity of dealing with the lexical servers.
 * The script starts the lexical servers, and then starts the retrieval process.
 * 
 * Note that the resulting JVM will connect to itself via the lexical server ports,
 * so if you run this twice on the same machine the second process will not be able
 * to bind to the ports.
 * 
 * For language pairs with large amounts of lexical data it is recommended to start
 * the lexical servers up separately.
 *  
 */


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