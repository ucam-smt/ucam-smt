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
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map.Entry;

import org.apache.hadoop.io.ByteWritable;
import org.apache.hadoop.io.DataInputBuffer;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.io.WritableComparable;
import org.apache.hadoop.io.WritableComparator;
import org.apache.hadoop.io.WritableUtils;
import org.apache.hadoop.mapreduce.Partitioner;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.partition.HashPartitioner;

import uk.ac.cam.eng.extraction.datatypes.Rule;
import uk.ac.cam.eng.extraction.hadoop.datatypes.AlignmentAndFeatureMap;
import uk.ac.cam.eng.extraction.hadoop.datatypes.FeatureMap;
import uk.ac.cam.eng.extraction.hadoop.datatypes.ProvenanceCountMap;
import uk.ac.cam.eng.extraction.hadoop.datatypes.RuleInfoWritable;
import uk.ac.cam.eng.extraction.hadoop.datatypes.RuleWritable;

/**
 * Computes marginal counts. Used for phrase based probability computation.
 * Rules need to be unique! For marginal reduction we assume the rule is three
 * adjacent text fields. If this format changes then jobs using this class will
 * no longer work!
 * 
 * @author Aurelien Waite
 * @date 28 May 2014
 */
public class MarginalReducer extends
		Reducer<RuleWritable, RuleInfoWritable, RuleWritable, AlignmentAndFeatureMap> {

	public static abstract class MRPartitioner extends
			Partitioner<RuleWritable, RuleInfoWritable> {

		Partitioner<Text, RuleInfoWritable> defaultPartitioner = new HashPartitioner<>();

		@Override
		public int getPartition(RuleWritable key, RuleInfoWritable value,
				int numPartitions) {
			return defaultPartitioner.getPartition(getMarginal(key), null,
					numPartitions);
		}

		protected abstract Text getMarginal(RuleWritable key);
	}

	/**
	 * Stores all the interesting positions in the byte array for a rule
	 * writable
	 * 
	 * @author aaw35
	 * 
	 */
	private static class RuleWritableSplit {
		int lhsStart;

		int lhsLength;

		int sourceStart;

		int sourceLength;

		int targetStart;

		int targetLength;

	}

	/**
	 * From the hadoop documentation it seems that comparators need to be thread
	 * safe. This class represents all the state that can be placed in a
	 * ThreadLocal
	 * 
	 * @author aaw35
	 * 
	 */
	private static class MRComparatorState {

		Text.Comparator comparator = new Text.Comparator();
		DataInputBuffer inBytes = new DataInputBuffer();

		RuleWritableSplit split1 = new RuleWritableSplit();

		RuleWritableSplit split2 = new RuleWritableSplit();

	}

	/**
	 * Note that this comparator is extremely sensitive to the rule writable
	 * binary format Change with care!
	 * 
	 * @author aaw35
	 * 
	 */
	public static abstract class MRComparator extends WritableComparator {

		ThreadLocal<MRComparatorState> threadLocalState = new ThreadLocal<>();

		public MRComparator() {
			super(RuleWritable.class);
		}

		protected abstract boolean isSource2Target();

		protected Text getMarginal(RuleWritable r) {
			if (isSource2Target()) {
				return r.getSource();
			} else {
				return r.getTarget();
			}
		}

		protected Text getNonMarginal(RuleWritable r) {
			if (isSource2Target()) {
				return r.getTarget();
			} else {
				return r.getSource();
			}
		}

		MRComparatorState getState() {
			MRComparatorState state = threadLocalState.get();
			if (state == null) {
				state = new MRComparatorState();
				threadLocalState.set(state);
			}
			return state;
		}

		@SuppressWarnings("rawtypes")
		@Override
		public int compare(WritableComparable a, WritableComparable b) {
			WritableComparator comparator = getState().comparator;
			int first = comparator.compare(getMarginal((RuleWritable) a),
					getMarginal((RuleWritable) b));
			if (first != 0) {
				return first;
			} else {
				return comparator.compare(getNonMarginal((RuleWritable) a),
						getNonMarginal((RuleWritable) b));
			}
		}

		private void findSplits(byte[] b, int s, int l,
				RuleWritableSplit split, DataInputBuffer inBytes) {

			try {
				split.lhsStart = WritableUtils.decodeVIntSize(b[s]) + s;
				inBytes.reset(b, s, l);
				split.lhsLength = WritableUtils.readVInt(inBytes);
				int sourceN = split.lhsLength + split.lhsStart;
				split.sourceStart = WritableUtils.decodeVIntSize(b[sourceN])
						+ sourceN;
				inBytes.reset(b, sourceN, l - (sourceN - s));
				split.sourceLength = WritableUtils.readVInt(inBytes);
				int targetN = split.sourceStart + split.sourceLength;
				split.targetStart = WritableUtils.decodeVIntSize(b[targetN])
						+ targetN;
				split.targetLength = l - (split.targetStart - s);
			} catch (IOException e) {
				throw new RuntimeException(
						"MRComparator should not throw this exception", e);
			}
		}

		private int sourceCompare(byte[] b1, RuleWritableSplit s1, byte[] b2,
				RuleWritableSplit s2) {
			return compareBytes(b1, s1.sourceStart, s1.sourceLength, b2,
					s2.sourceStart, s2.sourceLength);
		}

		private int targetCompare(byte[] b1, RuleWritableSplit s1, byte[] b2,
				RuleWritableSplit s2) {
			return compareBytes(b1, s1.targetStart, s1.targetLength, b2,
					s2.targetStart, s2.targetLength);
		}

		@Override
		public int compare(byte[] b1, int s1, int l1, byte[] b2, int s2, int l2) {
			MRComparatorState state = getState();
			RuleWritableSplit split1 = state.split1;
			RuleWritableSplit split2 = state.split2;
			findSplits(b1, s1, l1, split1, state.inBytes);
			findSplits(b2, s2, l2, split2, state.inBytes);

			int firstCompare;
			int secondCompare;
			if (isSource2Target()) {
				firstCompare = sourceCompare(b1, split1, b2, split2);
			} else {
				firstCompare = targetCompare(b1, split1, b2, split2);
			}
			if (firstCompare != 0) {
				return firstCompare;
			} else {
				if (isSource2Target()) {
					secondCompare = targetCompare(b1, split1, b2, split2);
				} else {
					secondCompare = sourceCompare(b1, split1, b2, split2);
				}
			}
			return secondCompare;

		}
	}

	private static class RuleCount {

		final RuleWritable rule;
		final RuleInfoWritable counts;

		public RuleCount(RuleWritable rule, RuleInfoWritable counts) {
			this.rule = rule;
			this.counts = counts;
		}

	}

	public static final String SOURCE_TO_TARGET = "rulextract.source2target";

	private static final String S2T_FEATURE_NAME = "source2target_probability";

	private static final String T2S_FEATURE_NAME = "target2source_probability";

	ProvenanceCountMap totals = new ProvenanceCountMap();

	List<RuleCount> ruleCounts = new ArrayList<>();

	Text marginal = new Text();

	boolean source2Target = true;

	int[] mappings;

	private FeatureMap features = new FeatureMap();

	private AlignmentAndFeatureMap alignmentAndFeatures = new AlignmentAndFeatureMap();

	private Text getMarginal(RuleWritable rule) {
		if (source2Target) {
			return rule.getSource();
		} else {
			return rule.getTarget();
		}
	}

	private String getFeatureName() {
		if (source2Target) {
			return S2T_FEATURE_NAME;
		} else {
			return T2S_FEATURE_NAME;
		}
	}

	@Override
	protected void setup(Context context) throws IOException,
			InterruptedException {
		super.setup(context);
		String s2tString = context.getConfiguration().get(SOURCE_TO_TARGET);
		if (s2tString == null) {
			throw new RuntimeException("Need to set configuration value "
					+ SOURCE_TO_TARGET);
		}
		source2Target = Boolean.valueOf(s2tString);
		mappings = ProvenanceCountMap.getFeatureIndex(getFeatureName(),
				context.getConfiguration());
	}

	private void marginalReduce(Iterable<RuleCount> rules,
			ProvenanceCountMap totals, Context context) throws IOException,
			InterruptedException {
		for (RuleCount rw : rules) {
			features.clear();
			for (Entry<ByteWritable, IntWritable> entry : rw.counts
					.getProvenanceCountMap().entrySet()) {
				double probability = (double) entry.getValue().get()
						/ (double) totals.get(entry.getKey()).get();
				int featureIndex = mappings[(int) entry.getKey().get()];
				features.put(featureIndex, probability);
				++featureIndex;
				features.put(featureIndex, entry.getValue().get());
			}
			RuleWritable outKey = rw.rule;
			if (!source2Target) {
				Rule r = new Rule(outKey);
				if (r.isSwapping()) {
					r.invertNonTerminals();
					outKey = new RuleWritable(r.invertNonTerminals());
				}
			}
			// add alignment info
			alignmentAndFeatures.set(rw.counts.getAlignmentCountMapWritable(),
					features);
			context.write(outKey, alignmentAndFeatures);
		}

	}

	@Override
	public void run(Context context) throws IOException, InterruptedException {
		setup(context);

		while (context.nextKey()) {
			RuleWritable key = context.getCurrentKey();
			// First Key!
			if (marginal.getLength() == 0) {
				marginal.set(getMarginal(key));
			}
			Iterator<RuleInfoWritable> it = context.getValues().iterator();
			RuleInfoWritable currentRuleInfo = it.next();

			if (it.hasNext()) {
				throw new RuntimeException("Non-unique rule! " + key);
			}
			if (!marginal.equals(getMarginal(key))) {
				marginalReduce(ruleCounts, totals, context);
				totals.clear();
				ruleCounts.clear();
			}
			marginal.set(getMarginal(key));
			ruleCounts.add(new RuleCount(new RuleWritable(key),
					new RuleInfoWritable(currentRuleInfo)));
			totals.increment(currentRuleInfo.getProvenanceCountMap());
		}
		marginalReduce(ruleCounts, totals, context);
		cleanup(context);
	}
}
