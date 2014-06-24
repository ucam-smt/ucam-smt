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
package uk.ac.cam.eng.extraction.hadoop.features.phrase;

import java.io.IOException;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.conf.Configured;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Partitioner;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.input.SequenceFileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.mapreduce.lib.output.SequenceFileOutputFormat;
import org.apache.hadoop.mapreduce.lib.partition.HashPartitioner;
import org.apache.hadoop.util.Tool;
import org.apache.hadoop.util.ToolRunner;

import uk.ac.cam.eng.extraction.hadoop.datatypes.FeatureMap;
import uk.ac.cam.eng.extraction.hadoop.datatypes.ProvenanceCountMap;
import uk.ac.cam.eng.extraction.hadoop.datatypes.RuleWritable;
import uk.ac.cam.eng.extraction.hadoop.util.Util;

import com.beust.jcommander.JCommander;
import com.beust.jcommander.Parameter;
import com.beust.jcommander.ParameterException;
import com.beust.jcommander.Parameters;

/**
 * 
 * @author Aurelien Waite
 * @author Juan Pino
 * @date 28 May 2014
 */
public class Source2TargetJob extends Configured implements Tool {

	public static class Source2TargetComparator extends
			MarginalReducer.MRComparator {

		@Override
		protected boolean isSource2Target() {
			return true;
		}

	}

	public static class Source2TargetPartioner extends
			Partitioner<RuleWritable, ProvenanceCountMap> {

		Partitioner<Text, ProvenanceCountMap> defaultPartitioner = new HashPartitioner<>();

		@Override
		public int getPartition(RuleWritable key, ProvenanceCountMap value,
				int numPartitions) {
			return defaultPartitioner.getPartition(key.getSource(), value,
					numPartitions);
		}

	}

	public static Job getJob(Configuration conf) throws IOException {
		// conf.set("mapred.reduce.child.java.opts", "-Xmx5128m");
		conf.set("mapred.map.child.java.opts", "-Xmx200m");
		conf.set("mapred.reduce.child.java.opts", "-Xmx5128m");
		conf.setBoolean(MarginalReducer.SOURCE_TO_TARGET, true);
		Job job = new Job(conf);
		job.setJarByClass(Source2TargetJob.class);
		job.setJobName("Source2Taget");
		job.setSortComparatorClass(Source2TargetComparator.class);
		job.setPartitionerClass(Source2TargetPartioner.class);
		job.setReducerClass(MarginalReducer.class);
		job.setMapOutputKeyClass(RuleWritable.class);
		job.setMapOutputValueClass(ProvenanceCountMap.class);
		job.setOutputKeyClass(RuleWritable.class);
		job.setOutputValueClass(FeatureMap.class);
		job.setInputFormatClass(SequenceFileInputFormat.class);
		job.setOutputFormatClass(SequenceFileOutputFormat.class);
		return job;
	}

	/**
	 * Defines command line args.
	 */
	@Parameters(separators = "=")
	public static class Source2TargetJobParameters {
		@Parameter(names = { "--input", "-i" }, description = "Input rules on HDFS", required = true)
		public String input;

		@Parameter(names = { "--output", "-o" }, description = "Output source-to-target probabilities on HDFS", required = true)
		public String output;

		@Parameter(names = { "--mapreduce_features" }, description = "Comma-separated mapreduce features", required = true)
		public String mapreduce_features;

		@Parameter(names = { "--provenance" }, description = "Comma-separated provenances")
		public String provenance;
	}

	public int run(String[] args) throws IllegalArgumentException,
			IllegalAccessException, IOException, ClassNotFoundException,
			InterruptedException {
		Source2TargetJobParameters params = new Source2TargetJobParameters();
		JCommander cmd = new JCommander(params);

		try {
			cmd.parse(args);
			Configuration conf = getConf();
			Util.ApplyConf(cmd, conf);
			Job job = getJob(conf);
			FileInputFormat.setInputPaths(job, params.input);
			FileOutputFormat.setOutputPath(job, new Path(params.output));
			return job.waitForCompletion(true) ? 0 : 1;
		} catch (ParameterException e) {
			System.err.println(e.getMessage());
			cmd.usage();
		}

		return 1;
	}

	public static void main(String[] args) throws Exception {
		int res = ToolRunner.run(new Source2TargetJob(), args);
		System.exit(res);
	}
}
