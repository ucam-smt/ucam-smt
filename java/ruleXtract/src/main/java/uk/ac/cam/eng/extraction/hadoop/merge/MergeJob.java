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
package uk.ac.cam.eng.extraction.hadoop.merge;

import java.io.IOException;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.conf.Configured;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.MultipleInputs;
import org.apache.hadoop.mapreduce.lib.input.SequenceFileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.util.StringUtils;
import org.apache.hadoop.util.Tool;
import org.apache.hadoop.util.ToolRunner;

import uk.ac.cam.eng.extraction.hadoop.datatypes.AlignmentAndFeatureMap;
import uk.ac.cam.eng.extraction.hadoop.datatypes.FeatureMap;
import uk.ac.cam.eng.extraction.hadoop.datatypes.RuleInfoWritable;
import uk.ac.cam.eng.extraction.hadoop.datatypes.RuleWritable;
import uk.ac.cam.eng.extraction.hadoop.datatypes.TargetFeatureList;
import uk.ac.cam.eng.extraction.hadoop.util.SimpleHFileOutputFormat;
import uk.ac.cam.eng.extraction.hadoop.util.Util;
import uk.ac.cam.eng.util.Pair;

import com.beust.jcommander.JCommander;
import com.beust.jcommander.Parameter;
import com.beust.jcommander.ParameterException;
import com.beust.jcommander.Parameters;

/**
 * 
 * @author Aurelien Waite
 * @date 28 May 2014
 */
public class MergeJob extends Configured implements Tool {

	private static class MergeFeatureMapper
			extends
			Mapper<RuleWritable, FeatureMap, RuleWritable, AlignmentAndFeatureMap> {

		private AlignmentAndFeatureMap alignmentAndFeatures = new AlignmentAndFeatureMap();

		@Override
		protected void map(RuleWritable key, FeatureMap value,
				Context context) throws IOException, InterruptedException {
			alignmentAndFeatures.setSecond(value);
			context.write(key, alignmentAndFeatures);
		}

	}
	
	private static class MergeRuleMapper
			extends
			Mapper<RuleWritable, RuleInfoWritable, RuleWritable, AlignmentAndFeatureMap> {

		private AlignmentAndFeatureMap alignmentAndFeatures = new AlignmentAndFeatureMap();

		@Override
		protected void map(RuleWritable key, RuleInfoWritable value,
				Context context)
				throws IOException, InterruptedException {
			alignmentAndFeatures.setFirst(value.getAlignmentCountMapWritable());
			context.write(key, alignmentAndFeatures);
		}
	}

	private static class MergeCombiner extends
			Reducer<RuleWritable, AlignmentAndFeatureMap, RuleWritable, AlignmentAndFeatureMap> {

		private AlignmentAndFeatureMap alignmentAndFeatures = new AlignmentAndFeatureMap();

		@Override
		protected void reduce(RuleWritable key,
				Iterable<AlignmentAndFeatureMap> values,
				Context context) throws IOException, InterruptedException {
			alignmentAndFeatures.clear();
			for (AlignmentAndFeatureMap value : values) {
				alignmentAndFeatures.merge(value);
			}
			context.write(key, alignmentAndFeatures);
		}
	}

	private static class MergeReducer extends
			Reducer<RuleWritable, AlignmentAndFeatureMap, Text, TargetFeatureList> {

		private TargetFeatureList list = new TargetFeatureList();

		private Text source = new Text();

		@Override
		protected void reduce(RuleWritable key,
				Iterable<AlignmentAndFeatureMap> values,
				Context context) throws IOException, InterruptedException {
			// First rule!
			if (source.getLength() == 0) {
				source.set(key.getSource());
			}
			if (!source.equals(key.getSource())) {
				context.write(source, list);
				list.clear();
				source.set(key.getSource());
			}
			AlignmentAndFeatureMap alignmentAndFeatures = new AlignmentAndFeatureMap();
			for (AlignmentAndFeatureMap value : values) {
				alignmentAndFeatures.merge(value);
			}
			list.add(Pair.createPair(new Text(key.getTarget()),
					alignmentAndFeatures));
		}

		@Override
		protected void cleanup(Context context) throws IOException,
				InterruptedException {
			super.cleanup(context);
			context.write(source, list);
		}
	}

	public static Job getJob(Configuration conf) throws IOException {

		conf.set("mapreduce.map.java.opts", "-Xmx200m");
		conf.set("mapreduce.reduce.java.opts", "-Xmx2048m");

		Job job = new Job(conf);
		job.setJarByClass(MergeJob.class);
		job.setJobName("Merge");
		job.setSortComparatorClass(MergeComparator.class);
		job.setPartitionerClass(MergePartitioner.class);
		job.setReducerClass(MergeReducer.class);
		job.setCombinerClass(MergeCombiner.class);
		job.setMapOutputKeyClass(RuleWritable.class);
		job.setMapOutputValueClass(AlignmentAndFeatureMap.class);
		job.setOutputKeyClass(RuleWritable.class);
		job.setOutputValueClass(AlignmentAndFeatureMap.class);
		job.setInputFormatClass(SequenceFileInputFormat.class);
		job.setOutputFormatClass(SimpleHFileOutputFormat.class);
		return job;
	}

	/**
	 * Defines command line args.
	 */
	@Parameters(separators = "=")
	public static class MergeJobParameters {
		@Parameter(names = { "--input_features" }, description = "Comma separated directories on HDFS with computed features", required = true)
		public String input_features;

		@Parameter(names = { "--input_rules" }, description = "HDFS directory with extracted rules", required = true)
		public String input_rules;

		@Parameter(names = { "--output", "-o" }, description = "Output directory on HDFS that will contain rules and features in HFile format", required = true)
		public String output;
	}

	public int run(String[] args) throws IllegalArgumentException,
			IllegalAccessException, IOException, ClassNotFoundException,
			InterruptedException {
		MergeJobParameters params = new MergeJobParameters();
		JCommander cmd = new JCommander(params);

		try {
			cmd.parse(args);
			Configuration conf = getConf();
			Util.ApplyConf(cmd, "", conf);
			Job job = getJob(conf);
			
			String[] featurePathNames = params.input_features.split(",");
			Path[] featurePaths = StringUtils.stringToPath(featurePathNames);
			for (Path featurePath : featurePaths) {
				MultipleInputs.addInputPath(job, featurePath, SequenceFileInputFormat.class, MergeFeatureMapper.class);
			}
			Path rulePath = new Path(params.input_rules);
			MultipleInputs.addInputPath(job, rulePath,
					SequenceFileInputFormat.class, MergeRuleMapper.class);

			FileOutputFormat.setOutputPath(job, new Path(params.output));
			
			return job.waitForCompletion(true) ? 0 : 1;
		} catch (ParameterException e) {
			System.err.println(e.getMessage());
			cmd.usage();
		}

		return 1;
	}

	public static void main(String[] args) throws Exception {
		int res = ToolRunner.run(new MergeJob(), args);
		System.exit(res);
	}
}
