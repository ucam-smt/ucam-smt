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
import org.apache.hadoop.io.SequenceFile;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.util.ReflectionUtils;

/**
 * @author Juan Pino
 * @date 17 July 2014
 */
public class SequenceFilePrint {

	public static void main(String[] args) throws IOException {
		if (args.length != 1) {
			System.err.println("Args: <sequence file to print>");
			System.exit(1);
		}
		Configuration conf = new Configuration();
		FileSystem fs = FileSystem.get(conf);
		Path path = new Path(args[0]);
		SequenceFile.Reader reader = new SequenceFile.Reader(fs, path, conf);
		Writable key = (Writable) ReflectionUtils.newInstance(
				reader.getKeyClass(), conf);
        Writable value = (Writable) ReflectionUtils.newInstance(
        		reader.getValueClass(), conf);
		while (reader.next(key, value)) {
			System.out.println(key + "\t" + value);
		}
		reader.close();
	}

}
