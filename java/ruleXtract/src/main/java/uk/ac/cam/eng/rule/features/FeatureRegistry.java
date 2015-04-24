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

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.hadoop.io.IntWritable;

import uk.ac.cam.eng.extraction.Rule;
import uk.ac.cam.eng.extraction.hadoop.datatypes.FeatureMap;
import uk.ac.cam.eng.extraction.hadoop.datatypes.IntWritableCache;
import uk.ac.cam.eng.extraction.hadoop.datatypes.ProvenanceProbMap;
import uk.ac.cam.eng.extraction.hadoop.datatypes.RuleData;

public class FeatureRegistry {

	// Used for provenance features which don't have a probability
	private final static double DEFAULT_S2T_PHRASE_LOG_PROB = -4.7;

	private final static double DEFAULT_T2S_PHRASE_LOG_PROB = -7;

	private final static double DEFAULT_LEX_VALUE = -40;

	private final List<Feature> allFeatures;

	private final Map<Feature, int[]> indexMappings = new HashMap<>();

	private final int noOfProvs;

	private final double[] zeroNonProv = new double[] { 0 };

	private final double[] zeroProv;

	private final Map<Integer, Double> defaultFeatures;

	private final Map<Integer, Double> defaultOOVFeatures;

	private final Map<Integer, Double> defaultPassThroughFeatures;

	private final Map<Integer, Double> defaultDeletionFeatures;

	private final Map<Integer, Double> defaultGlueFeatures;

	private final Map<Integer, Double> defaultDeleteGlueFeatures;

	private final Map<Integer, Double> defaultGlueStartOrEndFeatures;

	private final boolean hasLexicalFeatures;

	public FeatureRegistry(String featureString, String provenanceString) {
		String[] featureSplit = featureString.split(",");
		noOfProvs = provenanceString.split(",").length;
		List<Feature> features = new ArrayList<>();
		int indexCounter = 1; // 1-based
		boolean lexFeatures = false;
		for (String fString : featureSplit) {
			int[] mappings;
			Feature f = Feature.findFromConf(fString);
			lexFeatures |= Feature.ComputeLocation.LEXICAL_SERVER == f.computed;
			features.add(f);
			if (Feature.Scope.PROVENANCE == f.scope) {
				mappings = new int[noOfProvs];
				for (int i = 0; i < noOfProvs; ++i) {
					mappings[i] = indexCounter++;
				}
			} else {
				mappings = new int[] { indexCounter++ };
			}
			indexMappings.put(f, mappings);
		}
		allFeatures = Collections.unmodifiableList(features);
		zeroProv = new double[noOfProvs];
		hasLexicalFeatures = lexFeatures;
		Arrays.fill(zeroProv, 0.0);
		defaultFeatures = createDefaultData();
		defaultOOVFeatures = createOOVDefaultData();
		defaultPassThroughFeatures = createPassThroughDefaultData();
		defaultDeletionFeatures = createDeletionDefaultData();
		defaultGlueFeatures = createGlueDefaultData();
		defaultDeleteGlueFeatures = createDeleteGlueDefaultData();
		defaultGlueStartOrEndFeatures = createGlueStartOrEndDefaultData();
	}

	public int[] getFeatureIndices(Feature... features) {
		List<int[]> mappings = new ArrayList<int[]>(features.length);
		int totalSize = 0;
		for (Feature feature : features) {
			if (!indexMappings.containsKey(feature)) {
				throw new IllegalArgumentException("Feature "
						+ feature.getConfName() + " is not in the registry");
			}
			int[] mapping = indexMappings.get(feature);
			mappings.add(mapping);
			totalSize += mapping.length;
		}
		int[] result = new int[totalSize];
		int counter = 0;
		for (int[] mapping : mappings) {
			for (int index : mapping) {
				result[counter++] = index;
			}
		}
		return result;
	}

	public boolean containsFeature(Feature f) {
		return allFeatures.contains(f);
	}

	public List<Feature> getFeatures() {
		return allFeatures;
	}

	/**
	 * The number of provanences, not including the global (all) provenance.
	 * 
	 * @return
	 */
	public int getNoOfProvs() {
		return noOfProvs;
	}

	/**
	 * An array of zeros appropriately sized for provenance.
	 * 
	 * Do not write to the arrays returned from this function. They are cached
	 * to reduce object allocation during retrieval
	 * 
	 * @param f
	 * @return An array of zeros
	 */
	public double[] getZeros(Feature f) {
		if (Feature.Scope.PROVENANCE == f.scope) {
			return zeroProv;
		} else {
			return zeroNonProv;
		}
	}

	private void addDefault(Feature f, Map<Integer, Double> vals, double val) {
		if (allFeatures.contains(f)) {
			int[] mappings = getFeatureIndices(f);
			for (int mapping : mappings) {
				vals.put(mapping, val);
			}
		}
	}

	/**
	 * If any default values needed to be created then put them in this function
	 * 
	 * @return
	 */
	private Map<Integer, Double> createDefaultData() {
		// Provenance phrase probabilities need default values
		Map<Integer, Double> defaultFeatures = new HashMap<Integer, Double>();
		addDefault(Feature.PROVENANCE_SOURCE2TARGET_PROBABILITY,
				defaultFeatures, DEFAULT_S2T_PHRASE_LOG_PROB);
		addDefault(Feature.PROVENANCE_TARGET2SOURCE_PROBABILITY,
				defaultFeatures, DEFAULT_T2S_PHRASE_LOG_PROB);
		addDefault(Feature.RULE_INSERTION_PENALTY, defaultFeatures, 1d);
		return defaultFeatures;
	}

	/**
	 * Create data for OOVs with default vals for the lexical probs
	 * 
	 * @return
	 */
	private Map<Integer, Double> createPassThroughDefaultData() {
		// We need to add default values for lexical probs
		Map<Integer, Double> defaultFeatures = new HashMap<Integer, Double>();
		addDefault(Feature.SOURCE2TARGET_LEXICAL_PROBABILITY, defaultFeatures,
				DEFAULT_LEX_VALUE);
		addDefault(Feature.TARGET2SOURCE_LEXICAL_PROBABILITY, defaultFeatures,
				DEFAULT_LEX_VALUE);
		addDefault(Feature.PROVENANCE_SOURCE2TARGET_LEXICAL_PROBABILITY,
				defaultFeatures, DEFAULT_LEX_VALUE);
		addDefault(Feature.PROVENANCE_TARGET2SOURCE_LEXICAL_PROBABILITY,
				defaultFeatures, DEFAULT_LEX_VALUE);
		return defaultFeatures;
	}

	private Map<Integer, Double> createOOVDefaultData() {
		Map<Integer, Double> defaultFeatures = new HashMap<Integer, Double>();
		addDefault(Feature.INSERT_SCALE, defaultFeatures, -1d);
		return defaultFeatures;
	}

	private Map<Integer, Double> createDeletionDefaultData() {
		Map<Integer, Double> defaultFeatures = new HashMap<Integer, Double>();
		addDefault(Feature.INSERT_SCALE, defaultFeatures, -1d);
		return defaultFeatures;
	}

	private Map<Integer, Double> createGlueDefaultData() {
		Map<Integer, Double> defaultFeatures = new HashMap<Integer, Double>();
		addDefault(Feature.GLUE_RULE, defaultFeatures, 1d);
		return defaultFeatures;
	}

	private Map<Integer, Double> createDeleteGlueDefaultData() {
		Map<Integer, Double> defaultFeatures = new HashMap<Integer, Double>();
		addDefault(Feature.GLUE_RULE, defaultFeatures, 2d);
		return defaultFeatures;
	}

	private Map<Integer, Double> createGlueStartOrEndDefaultData() {
		Map<Integer, Double> defaultFeatures = new HashMap<Integer, Double>();
		addDefault(Feature.RULE_COUNT_GREATER_THAN_2, defaultFeatures, 1d);
		addDefault(Feature.RULE_INSERTION_PENALTY, defaultFeatures, 1d);
		addDefault(Feature.WORD_INSERTION_PENALTY, defaultFeatures, 1d);
		return defaultFeatures;
	}

	public Map<Integer, Double> getDefaultFeatures() {
		return new HashMap<Integer, Double>(defaultFeatures);
	}

	public Map<Integer, Double> getDefaultOOVFeatures() {
		return new HashMap<Integer, Double>(defaultOOVFeatures);
	}

	public Map<Integer, Double> getDefaultDeletionFeatures() {
		return new HashMap<Integer, Double>(defaultDeletionFeatures);
	}

	public Map<Integer, Double> getDefaultGlueFeatures() {
		return new HashMap<Integer, Double>(defaultGlueFeatures);
	}

	public Map<Integer, Double> getDefaultDeleteGlueFeatures() {
		return new HashMap<Integer, Double>(defaultDeleteGlueFeatures);
	}

	public Map<Integer, Double> getDefaultGlueStartOrEndFeatures() {
		return new HashMap<Integer, Double>(defaultGlueStartOrEndFeatures);
	}

	public Map<Integer, Double> getDefaultPassThroughRuleFeatures() {
		return new HashMap<Integer, Double>(defaultPassThroughFeatures);
	}

	private static final ProvenanceProbMap checkedGetProbs(Feature f,
			FeatureMap features) {
		ProvenanceProbMap probs = features.get(f);
		if (probs == null) {
			throw new RuntimeException("No data for feature " + f.getConfName());
		}
		return probs;
	}

	/**
	 * If we find a pass through rule in the data then we use its lexical
	 * features but nothing else. Slightly crazy!
	 * 
	 * @param features
	 * @param defaults
	 * @return
	 */
	public Map<Integer, Double> createFoundPassThroughRuleFeatures(
			FeatureMap features) {
		Map<Integer, Double> defaults = getDefaultPassThroughRuleFeatures();
		allFeatures
				.stream()
				.filter((f) -> Feature.ComputeLocation.LEXICAL_SERVER == f.computed)
				.forEach(
						(f) -> {
							int[] mappings = indexMappings.get(f);
							ProvenanceProbMap probs = checkedGetProbs(f,
									features);
							for (int index : mappings) {
								IntWritable indexIntW = IntWritableCache
										.createIntWritable(index);
								double ffVal = 0.0;
								if (probs.containsKey(indexIntW)) {
									ffVal = probs.get(indexIntW).get();
								}
								if (ffVal != 0.0) {
									defaults.put(index, ffVal);
								}
							}
						});
		return defaults;
	}

	private static void setVal(int mapping, double val,
			Map<Integer, Double> features) {
		// Default val in sparse tuple arc is 0. Delete default val.
		if (val == 0.0) {
			features.remove(mapping);
		} else {
			features.put(mapping, val);
		}
	}

	public Map<Integer, Double> processFeatures(Rule rule, RuleData data) {
		Map<Integer, Double> processedFeatures = getDefaultFeatures();
		for (Feature f : allFeatures) {
			int[] mappings = indexMappings.get(f);
			if (Feature.ComputeLocation.RETRIEVAL == f.computed) {
				double[] results = FeatureFunctionRegistry.computeFeature(f,
						rule, data, this);
				if (results == null) {
					continue;
				}
				for (int i = 0; i < results.length; ++i) {
					setVal(mappings[i], results[i], processedFeatures);
				}
			} else {
				ProvenanceProbMap probs = checkedGetProbs(f, data.getFeatures());
				for (int i = 0; i < mappings.length; ++i) {
					// Provenances are 1-indexed with the 0th element reserved
					// for the global
					// scope.
					int index = Feature.Scope.PROVENANCE == f.scope ? i + 1 : i;
					if (probs.containsKey(IntWritableCache
							.createIntWritable(index))) {
						double ffVal = probs.get(
								IntWritableCache.createIntWritable(index))
								.get();
						setVal(mappings[i], ffVal, processedFeatures);
					}
				}
			}
		}
		return processedFeatures;
	}

	public boolean hasLexicalFeatures() {
		return hasLexicalFeatures;
	}

}
