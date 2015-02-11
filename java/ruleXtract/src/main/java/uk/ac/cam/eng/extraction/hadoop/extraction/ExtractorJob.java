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

import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.conf.Configured;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.ByteWritable;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.MapWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.input.SequenceFileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.mapreduce.lib.output.SequenceFileOutputFormat;
import org.apache.hadoop.util.Tool;
import org.apache.hadoop.util.ToolRunner;

import uk.ac.cam.eng.extraction.RuleExtractor;
import uk.ac.cam.eng.extraction.datatypes.Alignment;
import uk.ac.cam.eng.extraction.datatypes.Rule;
import uk.ac.cam.eng.extraction.datatypes.SentencePair;
import uk.ac.cam.eng.extraction.hadoop.datatypes.AlignmentWritable;
import uk.ac.cam.eng.extraction.hadoop.datatypes.ProvenanceCountMap;
import uk.ac.cam.eng.extraction.hadoop.datatypes.RuleInfoWritable;
import uk.ac.cam.eng.extraction.hadoop.datatypes.RuleWritable;
import uk.ac.cam.eng.extraction.hadoop.datatypes.TextArrayWritable;
import uk.ac.cam.eng.extraction.hadoop.util.Util;

import com.beust.jcommander.JCommander;
import com.beust.jcommander.Parameter;
import com.beust.jcommander.ParameterException;
import com.beust.jcommander.Parameters;

/**
 * 
 * @author Juan Pino
 * @author Aurelien Waite
 * @date 28 May 2014
 */
public class ExtractorJob extends Configured implements Tool {

	/**
	 * 
	 * @param conf
	 * @return
	 * @throws IOException
	 */
	public static Job getJob(Configuration conf) throws IOException {
		conf.set("mapred.map.child.java.opts", "-Xmx200m");
		conf.set("mapred.reduce.child.java.opts", "-Xmx4096m");
		Job job = new Job(conf, "Rule extraction");
		job.setJarByClass(ExtractorJob.class);
		job.setMapOutputKeyClass(RuleWritable.class);
		job.setMapOutputValueClass(RuleInfoWritable.class);
		job.setOutputKeyClass(RuleWritable.class);
		job.setOutputValueClass(RuleInfoWritable.class);
		job.setMapperClass(ExtractorMapper.class);
		job.setReducerClass(ExtractorReducer.class);
		job.setCombinerClass(ExtractorReducer.class);
		job.setInputFormatClass(SequenceFileInputFormat.class);
		job.setOutputFormatClass(SequenceFileOutputFormat.class);
		FileOutputFormat.setCompressOutput(job, true);
		return job;
	}

	/**
	 * Mapper for rule extraction. Extracts the rules and writes the rule and
	 * additional info (unaligned words, etc.). We separate the rule from its
	 * additional info to be flexible and avoid equality problems whenever we
	 * add more info to the rule. The output will be the input to mapreduce
	 * features.
	 */
	private static class ExtractorMapper
			extends
			Mapper<MapWritable, TextArrayWritable, RuleWritable, RuleInfoWritable> {

		private static final IntWritable ONE = new IntWritable(1);

		private RuleInfoWritable ruleInfo = new RuleInfoWritable();

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

		/*
		 * (non-Javadoc)
		 * 
		 * @see org.apache.hadoop.mapreduce.Mapper#map(java.lang.Object,
		 * java.lang.Object, org.apache.hadoop.mapreduce.Mapper.Context)
		 */
		@Override
		protected void map(MapWritable key, TextArrayWritable value,
				Context context) throws IOException, InterruptedException {
			Configuration conf = context.getConfiguration();
			String sourceSentence = ((Text) value.get()[0]).toString();
			String targetSentence = ((Text) value.get()[1]).toString();
			String wordAlign = ((Text) value.get()[2]).toString();
			SentencePair sp = new SentencePair(sourceSentence, targetSentence);
			Alignment a = new Alignment(wordAlign, sp);
			RuleExtractor re = new RuleExtractor(conf);
			for (Rule r : re.extract(a, sp)) {
				RuleWritable rw = new RuleWritable(r);
				AlignmentWritable aw = new AlignmentWritable(r.getAlignment());
				ruleInfo.clear();
				ruleInfo.putProvenanceCount(ALL, ONE);
				for (Writable prov : key.keySet()) {
					if (prov2Id.keySet().contains(prov)) {
						ruleInfo.putProvenanceCount(prov2Id.get(prov), ONE);
					}
				}
				ruleInfo.putAlignmentCount(aw, 1);
				context.write(rw, ruleInfo);
			}
		}
	}

	private static class ExtractorReducer
			extends
			Reducer<RuleWritable, RuleInfoWritable, RuleWritable, RuleInfoWritable> {

		private RuleInfoWritable compressed = new RuleInfoWritable();

		@Override
		protected void reduce(
				RuleWritable key,
				Iterable<RuleInfoWritable> values, Context context)
				throws IOException, InterruptedException {
			compressed.clear();
			for (RuleInfoWritable value : values) {
				compressed.increment(value);
			}
			context.write(key, compressed);
		}
	}

	/**
	 * Defines command line args.
	 */
	@Parameters(separators = "=")
	public static class ExtractorJobParameters {
		@Parameter(names = { "--input", "-i" }, description = "Input training data on HDFS", required = true)
		public String input;

		@Parameter(names = { "--output", "-o" }, description = "Output rules on HDFS", required = true)
		public String output;

		@Parameter(names = { "--remove_monotonic_repeats"}, description = "Gives an "
				+ "occurrence count of 1 to monotonic hiero rules (e.g. "
				+ "phrase-pair <a b c, d e f> with alignment 0-0 1-1 2-2 "
				+ "generates hiero rule <a X, d X> twice but the count is "
				+ "still one)")
		public String remove_monotonic_repeats = "true";

		@Parameter(names = { "--max_source_phrase"}, description = "Maximum source phrase length in a phrase-based rule")
		public String max_source_phrase = "9";
		
		@Parameter(names = { "--max_source_elements"}, description = "Maximum number of source elements (terminals and nonterminals) in a hiero rule")
		public String max_source_elements = "5";

		@Parameter(names = { "--max_terminal_length" }, description = "Maximum number of consecutive source terminals in a hiero rule")
		public String max_terminal_length = "5";

		@Parameter(names = { "--max_nonterminal_span" }, description = "Maximum number of source terminals covered by a right-hand-side source nonterminal in a hiero rule")
		public String max_nonterminal_span = "10";

		@Parameter(names = { "--provenance" }, description = "Comma-separated provenances")
		public String provenance;
	}

	public int run(String[] args) throws FileNotFoundException, IOException,
			ClassNotFoundException, InterruptedException,
			IllegalArgumentException, IllegalAccessException {

		ExtractorJobParameters params = new ExtractorJobParameters();
		JCommander cmd = new JCommander(params);

		try {
			cmd.parse(args);
			Configuration conf = getConf();
			Util.ApplyConf(cmd, "", conf);
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
		int res = ToolRunner.run(new ExtractorJob(), args);
		System.exit(res);
	}
}
