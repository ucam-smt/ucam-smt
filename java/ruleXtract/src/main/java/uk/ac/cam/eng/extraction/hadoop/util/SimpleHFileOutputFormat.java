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

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.hbase.KeyValue;
import org.apache.hadoop.hbase.io.hfile.CacheConfig;
import org.apache.hadoop.hbase.io.hfile.HFile;
import org.apache.hadoop.hbase.regionserver.BloomType;
import org.apache.hadoop.hbase.util.BloomFilterFactory;
import org.apache.hadoop.hbase.util.BloomFilterWriter;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.mapreduce.RecordWriter;
import org.apache.hadoop.mapreduce.TaskAttemptContext;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;

import scala.Array;
import uk.ac.cam.eng.extraction.RuleString;
import uk.ac.cam.eng.extraction.hadoop.datatypes.TargetFeatureList;

/**
 * Create an HFile with a block size of 64k and a Bloom Filter
 * 
 * @author Aurelien Waite
 * @date 28 May 2014
 */
public class SimpleHFileOutputFormat extends
		FileOutputFormat<RuleString, TargetFeatureList> {

	@Override
	public RecordWriter<RuleString, TargetFeatureList> getRecordWriter(
			TaskAttemptContext job) throws IOException {

		final Configuration conf = job.getConfiguration();
		Path file = getDefaultWorkFile(job, ".hfile");
		FileSystem fs = file.getFileSystem(conf);
		final CacheConfig cacheConfig = new CacheConfig(conf);
		HFile.WriterFactory writerFactory = HFile.getWriterFactory(conf, cacheConfig).withPath(fs, file)
				.withBlockSize(64 * 1024).withCompression("gz");
		final HFile.Writer writer = writerFactory.create();
		return new RecordWriter<RuleString, TargetFeatureList>() {

			private ByteArrayOutputStream bytesOut = new ByteArrayOutputStream();

			private DataOutputStream out = new DataOutputStream(bytesOut);

			BloomFilterWriter bloomFilterWriter = BloomFilterFactory
					.createGeneralBloomAtWrite(conf, cacheConfig, BloomType.ROW, -1,
							writer);

			private byte[] createBytes(Writable obj) throws IOException {
				bytesOut.reset();
				obj.write(out);
				return bytesOut.toByteArray();
			}

			@Override
			public void write(RuleString key, TargetFeatureList value)
					throws IOException {
				byte[] keyBytes = createBytes(key);
				byte[] valueBytes = createBytes(value);
				KeyValue toWrite = new KeyValue(keyBytes, Array.emptyByteArray(), 
						Array.emptyByteArray(), 0, valueBytes);
				writer.append(toWrite);
				bloomFilterWriter.add(keyBytes, 0, keyBytes.length);
			}

			@Override
			public void close(TaskAttemptContext context) throws IOException {
				writer.addGeneralBloomFilter(bloomFilterWriter);
				writer.close();
			}
		};
	}

}
