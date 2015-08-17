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
 *******************************************************************************/

package uk.ac.cam.eng.extraction.hadoop.util;

import java.io.IOException;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.SequenceFile;
import org.apache.hadoop.io.SequenceFile.CompressionType;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.util.ReflectionUtils;

/**
 * Copy records from a sequence file for testing
 * 
 * @author aaw35
 *
 */

public class CopyRecordsForTesting {

	public static void main(String[] args) throws IOException {
		if (args.length != 3) {
			System.err
					.println("Args: <sequence file in> <sequence file out> <modulo #>");
			System.exit(1);
		}
		int modulo = Integer.parseInt(args[2]);
		Configuration conf = new Configuration();
		FileSystem fs = FileSystem.get(conf);
		Path pathIn = new Path(args[0]);
		SequenceFile.Reader reader = new SequenceFile.Reader(fs, pathIn, conf);
		Path pathOut = new Path(args[1]);
		SequenceFile.Writer writer = SequenceFile.createWriter(fs, conf,
				pathOut, reader.getKeyClass(), reader.getValueClass(),
				CompressionType.BLOCK);
		Writable key = (Writable) ReflectionUtils.newInstance(
				reader.getKeyClass(), conf);
		Writable value = (Writable) ReflectionUtils.newInstance(
				reader.getValueClass(), conf);
		int count=0;
		while (reader.next(key, value)) {
			if(count % modulo ==0){
				writer.append(key, value);
			}
			++count;
		}
		reader.close();
		writer.close();
	}

}
