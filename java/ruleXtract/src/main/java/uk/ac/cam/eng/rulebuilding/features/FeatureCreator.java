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

import java.io.BufferedWriter;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.SortedMap;
import java.util.TreeMap;

import org.apache.hadoop.conf.Configuration;

import uk.ac.cam.eng.extraction.datatypes.Rule;
import uk.ac.cam.eng.extraction.hadoop.datatypes.AlignmentAndFeatureMap;
import uk.ac.cam.eng.extraction.hadoop.datatypes.FeatureMap;
import uk.ac.cam.eng.extraction.hadoop.datatypes.RuleWritable;

/**
 * This class creates a set of features given a list of rules
 * 
 * @author Juan Pino
 * @author Aurelien Waite
 * @date 28 May 2014
 */
public class FeatureCreator {

	public static final String MAPRED_SUFFIX = "-mapreduce";

	// list of all features
	private Map<String, Feature> features;
	// list of selected features in order
	private String[] selectedFeatures;
	// configuration (features, feature indices)
	private Configuration conf;

	public FeatureCreator(Configuration conf) {
		this.conf = conf;
		features = new HashMap<String, Feature>();
		features.put("source2target_probability",
				new Source2TargetProbability());
		features.put("target2source_probability",
				new Target2SourceProbability());
		features.put("word_insertion_penalty", new WordInsertionPenalty());
		features.put("rule_insertion_penalty", new RuleInsertionPenalty());
		features.put("glue_rule", new GlueRule());
		features.put("insert_scale", new InsertScale());
		features.put("rule_count_1", new RuleCount1());
		features.put("rule_count_2", new RuleCount2());
		features.put("rule_count_greater_than_2", new RuleCountGreaterThan2());
		features.put("source2target_lexical_probability",
				new Source2TargetLexicalProbability());
		features.put("target2source_lexical_probability",
				new Target2SourceLexicalProbability());
		String provenance = conf.get("provenance");
		if (provenance != null) {
			String[] provenances = provenance.split(",");
			for (String prov : provenances) {
				features.put("provenance_source2target_probability-" + prov,
						new ProvenanceSource2TargetProbability(prov));
				features.put("provenance_target2source_probability-" + prov,
						new ProvenanceTarget2SourceProbability(prov));
				features.put("provenance_source2target_lexical_probability-"
						+ prov, new ProvenanceSource2TargetLexicalProbability(
						prov));
				features.put("provenance_target2source_lexical_probability-"
						+ prov, new ProvenanceTarget2SourceLexicalProbability(
						prov));
			}
		}
		String selectedFeaturesString = conf.get("features");
		if (selectedFeaturesString == null) {
			System.err
					.println("Missing property " + "'features' in the config");
			System.exit(1);
		}
		selectedFeatures = selectedFeaturesString.split(",");
		// initial feature index is zero, then increments with the number of
		// features of each feature type. nextFeatureIndex is used to prevent
		// conf to be overwritten before being used.
		int featureIndex = 0, nextFeatureIndex = 0;
		for (String selectedFeature : selectedFeatures) {
			if (selectedFeature
					.equals("provenance_source2target_lexical_probability")
					|| selectedFeature
							.equals("provenance_target2source_lexical_probability")
					|| selectedFeature
							.equals("provenance_source2target_probability")
					|| selectedFeature
							.equals("provenance_target2source_probability")) {
				for (String prov : conf.get("provenance").split(",")) {
					featureIndex = nextFeatureIndex;
					nextFeatureIndex += features.get(
							selectedFeature + "-" + prov).getNumberOfFeatures(
							conf);
					conf.setInt(selectedFeature + "-" + prov, featureIndex);
				}
			} else {
				featureIndex = nextFeatureIndex;
				nextFeatureIndex += features.get(selectedFeature)
						.getNumberOfFeatures(conf);
				// TODO change this (maybe something like
				// conf.setInt(selectedFeature+"-"+"startindex", featureIndex))
				conf.setInt(selectedFeature + "-nbfeats",
						features.get(selectedFeature).getNumberOfFeatures(conf));
				conf.setInt(selectedFeature, featureIndex);
			}
		}
	}

	private Map<Integer, Double> createFeatures(String featureName,
			RuleWritable rule, FeatureMap mapReduceFeatures,
			EnumRuleType ruleType) {
		switch (ruleType) {
		case GLUE:
			return features.get(featureName).valueGlue(new Rule(rule),
					mapReduceFeatures, conf);
		case ASCII_OOV_DELETE:
			return features.get(featureName).valueAsciiOovDeletion(
					new Rule(rule), mapReduceFeatures, conf);
		default:
			return features.get(featureName).value(new Rule(rule),
					mapReduceFeatures, conf);
		}

	}

	/*
	 * private Map<Integer, Number> createFeatureAsciiOovDeletion( String
	 * featureName, GeneralPairWritable3 asciiOovDeletionRule) { return
	 * features.get(featureName).valueAsciiOovDeletion( new
	 * Rule(asciiOovDeletionRule.getFirst()), asciiOovDeletionRule.getSecond(),
	 * conf); }
	 */

	/*
	 * private Map<Integer, Number> createFeatureGlueRule(String featureName,
	 * GeneralPairWritable3 glueRule) { return
	 * features.get(featureName).valueGlue( new Rule(glueRule.getFirst()),
	 * glueRule.getSecond(), conf); }
	 */

	public void writeRule(RuleWritable rule, AlignmentAndFeatureMap features,
			EnumRuleType ruleType, BufferedWriter out) {
		SortedMap<Integer, Double> processedFeatures = createFeatures(rule,
				features.getSecond(), ruleType);
		StringBuilder res = new StringBuilder();
		res.append(rule);
		// the following line is an example of how to print out the alignments
		//res.append(" " + features.getFirst());
		for (int featureIndex : processedFeatures.keySet()) {
			double featureValue = processedFeatures.get(featureIndex);
			// one-based index
			int index = featureIndex + 1;
			if (Math.floor(featureValue) == featureValue) {
				int featureValueInt = (int) featureValue;
				res.append(String.format(" %d@%d", featureValueInt, index));
			} else {
				res.append(String.format(" %f@%d", featureValue, index));
			}
		}
		res.append("\n");
		synchronized (out) {
			try {
				out.write(res.toString());
			} catch (IOException e) {
				e.printStackTrace();
				System.exit(1);
			}

		}
	}

	public SortedMap<Integer, Double> createFeatures(RuleWritable rule,
			FeatureMap map, EnumRuleType ruleType) {
		SortedMap<Integer, Double> allFeatures = new TreeMap<>();
		for (String featureName : selectedFeatures) {
			if (featureName
					.equals("provenance_source2target_lexical_probability")
					|| featureName
							.equals("provenance_target2source_lexical_probability")
					|| featureName
							.equals("provenance_source2target_probability")
					|| featureName
							.equals("provenance_target2source_probability")) {
				for (String provenance : conf.get("provenance").split(",")) {
					Map<Integer, Double> features = createFeatures(featureName
							+ "-" + provenance, rule, map, ruleType);
					for (Integer featureIndex : features.keySet()) {
						if (allFeatures.containsKey(featureIndex)) {
							System.err
									.println("ERROR: feature index already exists: "
											+ featureIndex);
							System.exit(1);
						}
						Number feature = features.get(featureIndex);
						allFeatures.put(featureIndex, feature.doubleValue());
					}
				}
			} else {
				Map<Integer, Double> features = createFeatures(featureName,
						rule, map, ruleType);
				for (Integer featureIndex : features.keySet()) {
					if (allFeatures.containsKey(featureIndex)) {
						System.err
								.println("ERROR: feature index already exists: "
										+ featureIndex);
						System.exit(1);
					}
					Number feature = features.get(featureIndex);
					allFeatures.put(featureIndex, feature.doubleValue());
				}
			}
		}
		return allFeatures;
	}

	/*
	 * private GeneralPairWritable3 createFeaturesAsciiOovDeletion(
	 * GeneralPairWritable3 asciiOovDeletionRule) { GeneralPairWritable3 res =
	 * new GeneralPairWritable3();
	 * res.setFirst(asciiOovDeletionRule.getFirst()); SortedMapWritable
	 * allFeatures = new SortedMapWritable(); for (String featureName :
	 * selectedFeatures) { if (featureName
	 * .equals("provenance_source2target_lexical_probability") || featureName
	 * .equals("provenance_target2source_lexical_probability") || featureName
	 * .equals("provenance_source2target_probability") || featureName
	 * .equals("provenance_target2source_probability")) { for (String provenance
	 * : conf.get("provenance").split(",")) { Map<Integer, Number> features =
	 * createFeatureAsciiOovDeletion( featureName + "-" + provenance,
	 * asciiOovDeletionRule); for (Integer featureIndex : features.keySet()) {
	 * IntWritable featureIndexWritable = new IntWritable( featureIndex); if
	 * (allFeatures.containsKey(featureIndexWritable)) { System.err
	 * .println("ERROR: feature index already exists: " + featureIndex);
	 * System.exit(1); } Number feature = features.get(featureIndex);
	 * allFeatures.put(featureIndexWritable, new
	 * DoubleWritable(feature.doubleValue())); } } } else { Map<Integer, Number>
	 * features = createFeatureAsciiOovDeletion( featureName,
	 * asciiOovDeletionRule); for (Integer featureIndex : features.keySet()) {
	 * IntWritable featureIndexWritable = new IntWritable( featureIndex); if
	 * (allFeatures.containsKey(featureIndexWritable)) { System.err
	 * .println("ERROR: feature index already exists: " + featureIndex);
	 * System.exit(1); } Number feature = features.get(featureIndex);
	 * allFeatures.put(featureIndexWritable, new DoubleWritable(
	 * feature.doubleValue())); } } } res.setSecond(allFeatures); return res; }
	 */

	/*
	 * private GeneralPairWritable3 createFeaturesGlueRule( GeneralPairWritable3
	 * glueRule) { GeneralPairWritable3 res = new GeneralPairWritable3();
	 * res.setFirst(glueRule.getFirst()); SortedMapWritable allFeatures = new
	 * SortedMapWritable(); for (String featureName : selectedFeatures) { if
	 * (featureName .equals("provenance_source2target_lexical_probability") ||
	 * featureName .equals("provenance_target2source_lexical_probability") ||
	 * featureName .equals("provenance_source2target_probability") ||
	 * featureName .equals("provenance_target2source_probability")) { for
	 * (String provenance : conf.get("provenance").split(",")) { Map<Integer,
	 * Number> features = createFeatureGlueRule( featureName + "-" + provenance,
	 * glueRule); for (Integer featureIndex : features.keySet()) { IntWritable
	 * featureIndexWritable = new IntWritable( featureIndex); if
	 * (allFeatures.containsKey(featureIndexWritable)) { System.err
	 * .println("ERROR: feature index already exists: " + featureIndex);
	 * System.exit(1); } Number feature = features.get(featureIndex);
	 * allFeatures.put(featureIndexWritable, new
	 * DoubleWritable(feature.doubleValue())); } } } else { Map<Integer, Number>
	 * features = createFeatureGlueRule( featureName, glueRule); for (Integer
	 * featureIndex : features.keySet()) { IntWritable featureIndexWritable =
	 * new IntWritable( featureIndex); if
	 * (allFeatures.containsKey(featureIndexWritable)) { System.err
	 * .println("ERROR: feature index already exists: " + featureIndex);
	 * System.exit(1); } Number feature = features.get(featureIndex);
	 * allFeatures.put(featureIndexWritable, new DoubleWritable(
	 * feature.doubleValue())); } } } res.setSecond(allFeatures); return res; }
	 */

	/*
	 * public List<GeneralPairWritable3> createFeatures(
	 * List<GeneralPairWritable3> rulesAndMapReduceFeatures) {
	 * List<GeneralPairWritable3> res = new ArrayList<GeneralPairWritable3>();
	 * for (GeneralPairWritable3 ruleAndMapReduceFeatures :
	 * rulesAndMapReduceFeatures) { GeneralPairWritable3 ruleAndFeatures =
	 * createFeatures(ruleAndMapReduceFeatures); res.add(ruleAndFeatures); }
	 * return res; }
	 */

	/*
	 * public List<GeneralPairWritable3> createFeaturesAsciiOovDeletion(
	 * List<GeneralPairWritable3> asciiOovDeletionRules) {
	 * List<GeneralPairWritable3> res = new ArrayList<>(); for
	 * (GeneralPairWritable3 asciiOovDeletionRule : asciiOovDeletionRules) {
	 * GeneralPairWritable3 asciiOovDeletionRuleAndFeatures =
	 * createFeaturesAsciiOovDeletion(asciiOovDeletionRule);
	 * res.add(asciiOovDeletionRuleAndFeatures); } return res; }
	 */

	/*
	 * public List<GeneralPairWritable3> createFeaturesGlueRules(
	 * List<GeneralPairWritable3> glueRules) { List<GeneralPairWritable3> res =
	 * new ArrayList<>(); for (GeneralPairWritable3 glueRule : glueRules) {
	 * GeneralPairWritable3 glueRuleAndFeatures =
	 * createFeaturesGlueRule(glueRule); res.add(glueRuleAndFeatures); } return
	 * res; }
	 */
}
