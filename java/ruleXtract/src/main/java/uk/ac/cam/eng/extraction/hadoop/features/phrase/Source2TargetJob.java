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
import java.util.List;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Partitioner;
import org.apache.hadoop.mapreduce.lib.input.SequenceFileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.SequenceFileOutputFormat;
import org.apache.hadoop.mapreduce.lib.partition.HashPartitioner;
import org.apache.hadoop.util.ToolRunner;

import uk.ac.cam.eng.extraction.Rule;
import uk.ac.cam.eng.extraction.Symbol;
import uk.ac.cam.eng.extraction.hadoop.datatypes.ExtractedData;
import uk.ac.cam.eng.extraction.hadoop.datatypes.FeatureMap;
import uk.ac.cam.eng.extraction.hadoop.datatypes.ProvenanceCountMap;

/**
 * 
 * @author Aurelien Waite
 * @author Juan Pino
 * @date 28 May 2014
 */
public class Source2TargetJob extends PhraseJob{

	public static class Source2TargetComparator extends
			MarginalReducer.MRComparator {

		@Override
		protected boolean isSource2Target() {
			return true;
		}

	}

	private static class Source2TargetPartitioner extends
			Partitioner<Rule, ProvenanceCountMap> {

		private Partitioner<List<Symbol>, ProvenanceCountMap> defaultPartitioner = new HashPartitioner<>();

		@Override
		public int getPartition(Rule key, ProvenanceCountMap value,
				int numPartitions) {
			return defaultPartitioner.getPartition(key.getSource(), value,
					numPartitions);
		}

	}

	private static class KeepProvenanceCountsOnlyMapper
			extends
			Mapper<Rule, ExtractedData, Rule, ProvenanceCountMap> {

		@Override
		protected void map(Rule key, ExtractedData value,
				Context context) throws IOException, InterruptedException {
			context.write(key, value.getProvenanceCountMap());
		}

	}

	@Override
	public Job getJob(Configuration conf) throws IOException {
		conf.setIfUnset("mapreduce.map.child.java.opts", "-Xmx200m");
		conf.setIfUnset("mapreduce.reduce.child.java.opts", "-Xmx5128m");
		conf.setIfUnset("mapreduce.map.memory.mb", "1000");
		conf.setIfUnset("mapreduce.reduce.memory.mb", "6000");
		conf.setBoolean(MarginalReducer.SOURCE_TO_TARGET, true);
		Job job = new Job(conf);
		job.setJarByClass(Source2TargetJob.class);
		job.setJobName("Source2Taget");
		job.setSortComparatorClass(Source2TargetComparator.class);
		job.setPartitionerClass(Source2TargetPartitioner.class);
		job.setMapperClass(KeepProvenanceCountsOnlyMapper.class);
		job.setReducerClass(MarginalReducer.class);
		job.setMapOutputKeyClass(Rule.class);
		job.setMapOutputValueClass(ProvenanceCountMap.class);
		job.setOutputKeyClass(Rule.class);
		job.setOutputValueClass(FeatureMap.class);
		job.setInputFormatClass(SequenceFileInputFormat.class);
		job.setOutputFormatClass(SequenceFileOutputFormat.class);
		return job;
	}


	public static void main(String[] args) throws Exception {
		int res = ToolRunner.run(new Source2TargetJob(), args);
		System.exit(res);
	}
}
