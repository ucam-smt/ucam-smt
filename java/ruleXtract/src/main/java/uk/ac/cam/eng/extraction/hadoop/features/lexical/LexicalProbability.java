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
/**
 * 
 */

package uk.ac.cam.eng.extraction.hadoop.features.lexical;

import java.io.IOException;
import java.util.Arrays;
import java.util.List;
import java.util.Map;

import uk.ac.cam.eng.extraction.datatypes.Rule;
import uk.ac.cam.eng.extraction.hadoop.datatypes.RuleWritable;

/**
 * Helper class for Source2TargetProbabilityReducer. Computes the
 * source-to-target lexical probability.
 * 
 * @author Juan Pino
 * @author Aurelien Waite
 * @date 28 May 2014
 */
public class LexicalProbability {

	private final double minSum = 4.24e-18; // exp(-40)

	boolean source2target;

	public LexicalProbability(boolean source2target) {
		this.source2target = source2target;
	}

	public void buildQuery(RuleWritable ruleWritable, int noOfProvs,
			Map<List<Integer>, Double> batchWordAlignments) {
		Rule rule = new Rule(ruleWritable);
		List<Integer> sourceWords;
		List<Integer> targetWords;
		if (source2target) {
			sourceWords = rule.getSourceWords();
			targetWords = rule.getTargetWords();
		} else {
			sourceWords = rule.getTargetWords();
			targetWords = rule.getSourceWords();
		}
		if (sourceWords.size() > 1) {
			targetWords.add(0);
		}
		for (Integer sourceWord : sourceWords) {
			for (Integer targetWord : targetWords) {
				for (int i = 0; i < noOfProvs; ++i) {
					Integer[] key;
					key = new Integer[] { i, sourceWord, targetWord };
					batchWordAlignments.put(Arrays.asList(key),
							Double.MAX_VALUE);
				}
			}

		}
	}

	public double value(RuleWritable ruleWritable, byte prov,
			Map<List<Integer>, Double> batchWordAlignments) throws IOException {
		double lexprob = 1;
		Rule rule = new Rule(ruleWritable);
		List<Integer> sourceWords;
		List<Integer> targetWords;
		if (source2target) {
			sourceWords = rule.getSourceWords();
			targetWords = rule.getTargetWords();
		} else {
			sourceWords = rule.getTargetWords();
			targetWords = rule.getSourceWords();
		}
		if (sourceWords.size() > 1) {
			targetWords.add(0);
		}
		for (Integer sourceWord : sourceWords) {
			double sum = 0;
			for (Integer targetWord : targetWords) {
				Integer[] key;
				key = new Integer[] { (int) prov, sourceWord, targetWord };
				List<Integer> serverKey = Arrays.asList(key);
				if (batchWordAlignments.containsKey(serverKey)) {
					double val = batchWordAlignments.get(serverKey);
					// System.out.println(serverKey + "\t" + val);
					sum += val;
				}
			}
			if (sum > 0) {
				lexprob *= sum;
			} else {
				lexprob *= minSum;
			}

		}
		lexprob /= Math.pow(targetWords.size(), sourceWords.size());
		// TODO could use the log in the computation
		return Math.log(lexprob);
	}

}
