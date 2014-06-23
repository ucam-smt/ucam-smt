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
package uk.ac.cam.eng.extraction.hadoop.extraction;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.ByteWritable;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.input.SequenceFileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.mapreduce.lib.output.SequenceFileOutputFormat;

import uk.ac.cam.eng.extraction.hadoop.datatypes.ProvenanceCountMap;
import uk.ac.cam.eng.extraction.hadoop.datatypes.RuleInfoWritable;
import uk.ac.cam.eng.extraction.hadoop.datatypes.RuleWritable;
import uk.ac.cam.eng.extraction.hadoop.util.Util;

/**
 * 
 * @author Aurelien Waite
 * @date 28 May 2014
 */
public class RuleCompression {

	public static class CompressMapper
			extends
			Mapper<RuleWritable, RuleInfoWritable, RuleWritable, ProvenanceCountMap> {

		private static final IntWritable ONE = new IntWritable(1);

		private ProvenanceCountMap provenanceMap = new ProvenanceCountMap();

		private Map<Text, ByteWritable> prov2Id = new HashMap<>();

		private static final ByteWritable ALL = new ByteWritable((byte) 0);

		@Override
		protected void setup(Context context) throws IOException,
				InterruptedException {
			super.setup(context);
			String provString = context.getConfiguration().get(
					ProvenanceCountMap.PROV);
			String[] provs = provString.split(",");
			if (provs.length + 1 >= Byte.MAX_VALUE) {
				throw new RuntimeException(
						String.format(
								"Number of provenances is %d which is greater than 128",
								provs.length));
			}
			for (int i = 0; i < provs.length; ++i) {
				prov2Id.put(new Text(provs[i]),
						new ByteWritable((byte) (i + 1)));
			}

		}

		@Override
		protected void map(RuleWritable key, RuleInfoWritable value,
				Context context) throws IOException, InterruptedException {
			provenanceMap.clear();
			provenanceMap.put(ALL, ONE);
			for (Writable prov : value.getProvenance().keySet()) {
				if (prov2Id.keySet().contains(prov)) {
					provenanceMap.put(prov2Id.get(prov), ONE);
				}
			}
			context.write(key, provenanceMap);
		}

	}

	public static class CompressReducer
			extends
			Reducer<RuleWritable, ProvenanceCountMap, RuleWritable, ProvenanceCountMap> {

		private ProvenanceCountMap compressed = new ProvenanceCountMap();

		@Override
		protected void reduce(
				RuleWritable key,
				Iterable<ProvenanceCountMap> values,
				Reducer<RuleWritable, ProvenanceCountMap, RuleWritable, ProvenanceCountMap>.Context context)
				throws IOException, InterruptedException {
			compressed.clear();
			for (ProvenanceCountMap provs : values) {
				compressed.increment(provs);
			}
			context.write(key, compressed);
		}
	}

	public static Job getJob(Configuration conf) throws IOException {
		conf.set("mapred.reduce.child.java.opts", "-Xmx4096m");
		Job job = new Job(conf, "Rule compression");
		job.setJarByClass(RuleCompression.class);
		job.setMapOutputKeyClass(RuleWritable.class);
		job.setMapOutputValueClass(ProvenanceCountMap.class);
		job.setOutputKeyClass(RuleWritable.class);
		job.setOutputValueClass(ProvenanceCountMap.class);
		job.setMapperClass(CompressMapper.class);
		job.setReducerClass(CompressReducer.class);
		job.setCombinerClass(CompressReducer.class);
		job.setInputFormatClass(SequenceFileInputFormat.class);
		job.setOutputFormatClass(SequenceFileOutputFormat.class);
		FileOutputFormat.setCompressOutput(job, true);
		return job;
	}

	/**
	 * 
	 * Rule file from rule extraction is huge. It contains a single entry for
	 * each instance. This job reduces down for a single rule with a count.
	 * 
	 * @param args
	 * @throws IOException
	 * @throws InterruptedException
	 * @throws ClassNotFoundException
	 */
	public static void main(String[] args) throws IOException,
			ClassNotFoundException, InterruptedException {
		Configuration conf = new Configuration();
		conf.set("mapred.reduce.child.java.opts", "-Xmx4096m");
		Util.ApplyConf(args[2], conf);
		Job job = getJob(conf);
		FileInputFormat.setInputPaths(job, args[0]);
		FileOutputFormat.setOutputPath(job, new Path(args[1]));
		job.waitForCompletion(true);
	}

}
