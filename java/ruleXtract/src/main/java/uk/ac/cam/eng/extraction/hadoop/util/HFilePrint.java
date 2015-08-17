/*******************************************************************************
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use these files except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Copyright 2014 - Juan Pino, Aurelien Waite, William Byrne
 *******************************************************************************/

package uk.ac.cam.eng.extraction.hadoop.util;

import java.io.IOException;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.hbase.io.hfile.CacheConfig;
import org.apache.hadoop.hbase.io.hfile.HFile;

import uk.ac.cam.eng.extraction.Rule;
import uk.ac.cam.eng.extraction.hadoop.datatypes.RuleData;
import uk.ac.cam.eng.rule.retrieval.HFileRuleReader;
import uk.ac.cam.eng.util.Pair;

/**
 * @author Juan Pino
 * @date 17 July 2014
 */
public class HFilePrint {

	public static void main(String[] args) throws IOException {
		if (args.length != 1) {
			System.err.println("Args: <HFile to be printed>");
			System.exit(1);
		}
		Configuration conf = new Configuration();
		CacheConfig cacheConf = new CacheConfig(conf);
		HFile.Reader hfReader = HFile.createReader(FileSystem.get(conf),
				new Path(args[0]), cacheConf);
		HFileRuleReader reader = new HFileRuleReader(hfReader);
		for (Pair<Rule, RuleData> entry : reader) {
			System.out.println(entry);
		}
	}

}
