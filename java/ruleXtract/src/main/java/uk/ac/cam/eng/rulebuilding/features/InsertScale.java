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

package uk.ac.cam.eng.rulebuilding.features;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import org.apache.hadoop.conf.Configuration;

import uk.ac.cam.eng.extraction.datatypes.Rule;
import uk.ac.cam.eng.extraction.hadoop.datatypes.FeatureMap;

/**
 * 
 * @author Juan Pino
 * @author Aurelien Waite
 * @date 28 May 2014
 */
public class InsertScale implements Feature {

	private final static String featureName = "insert_scale";

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * uk.ac.cam.eng.rulebuilding.features.Feature#value(uk.ac.cam.eng.extraction
	 * .datatypes.Rule, org.apache.hadoop.io.ArrayWritable)
	 */
	@Override
	public Map<Integer, Double> value(Rule r, FeatureMap mapReduceFeatures,
			Configuration conf) {
		return Collections.emptyMap();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * uk.ac.cam.eng.rulebuilding.features.Feature#valueAsciiOovDeletion(uk.
	 * ac.cam.eng.extraction.datatypes.Rule, org.apache.hadoop.io.ArrayWritable)
	 */
	@Override
	public Map<Integer, Double> valueAsciiOovDeletion(Rule r,
			FeatureMap mapReduceFeatures, Configuration conf) {
		Map<Integer, Double> res = new HashMap<>();
		int featureIndex = conf.getInt(featureName, 0);
		// deletion rule
		if (r.getTargetWords().size() == 1 && r.getTargetWords().get(0) == 0) {
			res.put(featureIndex, -1d);
		}
		// oov rule
		else if (r.getTargetWords().size() == 0) {
			res.put(featureIndex, -1d);
		}
		return res;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see uk.ac.cam.eng.rulebuilding.features.Feature#valueGlue(uk.ac.cam.eng.
	 * extraction.datatypes.Rule, org.apache.hadoop.io.ArrayWritable)
	 */
	@Override
	public Map<Integer, Double> valueGlue(Rule r, FeatureMap mapReduceFeatures,
			Configuration conf) {
		return Collections.emptyMap();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * uk.ac.cam.eng.rulebuilding.features.Feature#getNumberOfFeatures(org.apache
	 * .hadoop.conf.Configuration)
	 */
	@Override
	public int getNumberOfFeatures(Configuration conf) {
		return 1;
	}
}
