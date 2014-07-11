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

import java.util.Map;

import org.apache.hadoop.conf.Configuration;

import uk.ac.cam.eng.extraction.datatypes.Rule;
import uk.ac.cam.eng.extraction.hadoop.datatypes.FeatureMap;

/**
 * This interface represents a feature that can be computed on the fly, for
 * example the word insertion penalty
 * 
 * @author Juan Pino
 * @author Aurelien Waite
 * @date 28 May 2014
 */
public interface Feature {

	public Map<Integer, Double> value(Rule r, FeatureMap mapReduceFeatures,
			Configuration conf);

	public Map<Integer, Double> valueAsciiOovDeletion(Rule r,
			FeatureMap mapReduceFeatures, Configuration conf);

	public Map<Integer, Double> valueGlue(Rule r, FeatureMap mapReduceFeatures,
			Configuration conf);

	public int getNumberOfFeatures(Configuration conf);

}
