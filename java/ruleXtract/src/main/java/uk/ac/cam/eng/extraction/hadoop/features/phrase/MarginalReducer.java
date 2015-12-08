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

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map.Entry;

import org.apache.hadoop.io.ByteWritable;
import org.apache.hadoop.io.DataInputBuffer;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.WritableComparable;
import org.apache.hadoop.io.WritableComparator;
import org.apache.hadoop.io.WritableUtils;
import org.apache.hadoop.mapreduce.Reducer;

import uk.ac.cam.eng.extraction.Rule;
import uk.ac.cam.eng.extraction.Symbol;
import uk.ac.cam.eng.extraction.hadoop.datatypes.FeatureMap;
import uk.ac.cam.eng.extraction.hadoop.datatypes.ProvenanceCountMap;
import uk.ac.cam.eng.extraction.hadoop.datatypes.ProvenanceProbMap;
import uk.ac.cam.eng.rule.features.Feature;

/**
 * Computes marginal counts. Used for phrase based probability computation.
 * Rules need to be unique! For marginal reduction we assume the rule is three
 * adjacent text fields. If this format changes then jobs using this class will
 * no longer work!
 * 
 * @author Aurelien Waite
 * @date 28 May 2014
 */
class MarginalReducer extends
		Reducer<Rule, ProvenanceCountMap, Rule, FeatureMap> {

	/**
	 * Stores all the interesting positions in the byte array for a rule
	 * writable
	 * 
	 * @author aaw35
	 * 
	 */
	private static class RuleWritableSplit {
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
	 * @author Aurelien Waite
	 * 
	 */
	private static class MRComparatorState {
		
		DataInputBuffer inBytes = new DataInputBuffer();

		RuleWritableSplit split1 = new RuleWritableSplit();

		RuleWritableSplit split2 = new RuleWritableSplit();

	}

	/**
	 * Note that this comparator is extremely sensitive to the rule writable
	 * binary format. Change with care!
	 * 
	 * @author Aurelien Waite
	 * 
	 */
	public static abstract class MRComparator extends WritableComparator {

		private ThreadLocal<MRComparatorState> threadLocalState = new ThreadLocal<>();

		public MRComparator() {
			super(Rule.class);
		}

		protected abstract boolean isSource2Target();

		private MRComparatorState getState() {
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
			ByteArrayOutputStream bytes = new ByteArrayOutputStream();
			DataOutputStream out = new DataOutputStream(bytes);
			((Rule)a).write(out);
			byte[] bytesA = bytes.toByteArray();
			bytes.reset();
			((Rule)b).write(out);
			byte[] bytesB = bytes.toByteArray();
			return compare(bytesA, 0, bytesA.length, bytesB, 0, bytesB.length);
		}

		private void findSplits(byte[] b, int s, int l,
				RuleWritableSplit split, DataInputBuffer inBytes) {

			try {
				split.sourceStart = WritableUtils.decodeVIntSize(b[s])
						+ s;
				inBytes.reset(b, s, l);
				int pointer = split.sourceStart;
				int len = WritableUtils.readVInt(inBytes);
				for(int i=0; i< len; ++i){
					pointer += WritableUtils.decodeVIntSize(b[pointer]);
				}
				split.sourceLength = pointer - split.sourceStart;
				int targetN = pointer;
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

		final Rule rule;
		final ProvenanceCountMap counts;

		public RuleCount(Rule rule, ProvenanceCountMap counts) {
			this.rule = rule;
			this.counts = counts;
		}

	}

	public static final String SOURCE_TO_TARGET = "rulextract.source2target";

	private ProvenanceCountMap totals = new ProvenanceCountMap();

	private List<RuleCount> ruleCounts = new ArrayList<>();

	private List<Symbol> marginal = new ArrayList<>();

	private boolean source2Target = true;

	private ProvenanceProbMap provProbs = new ProvenanceProbMap();
	
	private ProvenanceProbMap globalProb = new ProvenanceProbMap();

	private Feature globalF;
	
	private Feature provF;
	
	private FeatureMap features = new FeatureMap();
	
	private List<Symbol> getMarginal(Rule rule) {
		if (source2Target) {
			return rule.getSource();
		} else {
			return rule.getTarget();
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
		if (source2Target) {
					globalF = Feature.SOURCE2TARGET_PROBABILITY;
					provF = Feature.PROVENANCE_SOURCE2TARGET_PROBABILITY;
		} else {
					globalF = Feature.TARGET2SOURCE_PROBABILITY;
					provF = Feature.PROVENANCE_TARGET2SOURCE_PROBABILITY;
		}
	}

	private void marginalReduce(Iterable<RuleCount> rules,
			ProvenanceCountMap totals, Context context) throws IOException,
			InterruptedException {
		for (RuleCount rc : rules) {
			provProbs.clear();
			globalProb.clear();
			features.clear();
			for (Entry<ByteWritable, IntWritable> entry : rc.counts.entrySet()) {
				double probability = (double) entry.getValue().get()
						/ (double) totals.get(entry.getKey()).get();
				int key = (int) entry.getKey().get();
				if(key==0){
					globalProb.put(key, Math.log(probability));
				}
				else{
					provProbs.put(key, Math.log(probability));	
				}
			}
			features.put(globalF, globalProb);
			features.put(provF, provProbs);
			Rule outKey = rc.rule;
			if (!source2Target) {
				if (outKey.isSwapping()) {
					outKey = outKey.invertNonTerminals();
				}
			}
			context.write(outKey, features);
		}

	}

	@Override
	public void run(Context context) throws IOException, InterruptedException {
		setup(context);

		while (context.nextKey()) {
			Rule key = context.getCurrentKey();
			// First Key!
			if (marginal.size() == 0) {
				marginal.addAll(getMarginal(key));
			}
			Iterator<ProvenanceCountMap> it = context.getValues().iterator();
			ProvenanceCountMap counts = it.next();

			if (it.hasNext()) {
				throw new RuntimeException("Non-unique rule! " + key);
			}
			if (!marginal.equals(getMarginal(key))) {
				marginalReduce(ruleCounts, totals, context);
				totals.clear();
				ruleCounts.clear();
			}
			marginal.clear();
			marginal.addAll(getMarginal(key));
			ruleCounts.add(new RuleCount(new Rule(key),
					new ProvenanceCountMap(counts)));
			totals.increment(counts);
		}
		marginalReduce(ruleCounts, totals, context);
		cleanup(context);
	}
}
