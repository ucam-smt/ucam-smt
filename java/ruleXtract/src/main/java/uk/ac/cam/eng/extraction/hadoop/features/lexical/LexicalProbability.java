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

import uk.ac.cam.eng.extraction.Rule;
import uk.ac.cam.eng.extraction.Symbol;
import uk.ac.cam.eng.extraction.Terminal;

/**
 * Helper class for Source2TargetProbabilityReducer. Computes the
 * source-to-target lexical probability.
 * 
 * @author Juan Pino
 * @author Aurelien Waite
 * @date 28 May 2014
 */
class LexicalProbability {

	private final double minSum = 4.24e-18; // exp(-40)

	private boolean source2target;

	public LexicalProbability(boolean source2target) {
		this.source2target = source2target;
	}

	public void buildQuery(Rule ruleWritable, int noOfProvs,
			Map<List<Integer>, Double> batchWordAlignments) {
		Rule rule = new Rule(ruleWritable);
		List<Integer> sourceWords;
		List<Integer> targetWords;
		if (source2target) {
			sourceWords = rule.getSource();
			targetWords = rule.getTarget();
		} else {
			sourceWords = rule.getTarget();
			targetWords = rule.getSource();
		}
		if (sourceWords.size() > 1) {
			targetWords.add(0);
		}
		for (int sourceWord : sourceWords) {
			for (int targetWord : targetWords) {
				for (int i = 0; i < noOfProvs; ++i) {
					Integer[] key;
					key = new Integer[] { i, sourceWord, targetWord };
					batchWordAlignments.put(Arrays.asList(key),
							Double.MAX_VALUE);
				}
			}

		}
	}

	public double value(Rule ruleWritable, byte prov,
			Map<List<Integer>, Double> batchWordAlignments) throws IOException {
		double lexprob = 1;
		Rule rule = new Rule(ruleWritable);
		List<Integer> sourceWords;
		List<Integer> targetWords;
		if (source2target) {
			sourceWords = rule.source().getTerminals();
			targetWords = rule.target().getTerminals();
		} else {
			sourceWords = rule.target().getTerminals();
			targetWords = rule.source().getTerminals();
		}
		if (sourceWords.size() > 1) {
			targetWords.add(0);
		}
		for (int sourceWord : sourceWords) {
			double sum = 0;
			for (int targetWord : targetWords) {
				Integer[] key;
				key = new Integer[] { (int) prov, sourceWord, targetWord};
				List<Integer> serverKey = Arrays.asList(key);
				if (batchWordAlignments.containsKey(serverKey)) {
					double val = batchWordAlignments.get(serverKey);
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
		return Math.log(lexprob);
	}

}
