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
 *******************************************************************************/

package uk.ac.cam.eng.rule.features;

import org.apache.hadoop.io.ByteWritable;

import uk.ac.cam.eng.extraction.Rule;
import uk.ac.cam.eng.rule.features.FeatureFunctionRegistry.FeatureFunctionInputData;

public final class FeatureFunctions {

	static final double[] one = new double[] { 1 };

	static final double[] minusOne = new double[] { -1 };

	static final double[] empty = new double[] { 0 };

	private static final ByteWritable zeroIndex = new ByteWritable((byte) 0);


	static double[] ruleCount1(Rule r, FeatureFunctionInputData data) {
		// 0 element is count over the entire data
		int count = data.counts.get(zeroIndex)
				.get();
		if (count == 1) {
			return one;
		} else
			return empty;
	}

	static double[] ruleCount2(Rule r, FeatureFunctionInputData data) {
		// 0 element is count over the entire data
		int count = data.counts.get(zeroIndex)
				.get();
		if (count == 2) {
			return one;
		} else
			return empty;
	}

	static double[] ruleGreaterThan2(Rule r, FeatureFunctionInputData data) {
		// 0 element is count over the entire data
		int count = data.counts.get(zeroIndex)
				.get();
		if (count > 2) {
			return one;
		}
		return empty;
	}

	static double[] noOfWords(Rule r, FeatureFunctionInputData data) {
		return new double[]{r.target().getWordCount()};
	}

}