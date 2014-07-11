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

package uk.ac.cam.eng.rule.retrieval;

// TODO remove hard coded indices

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.SortedSet;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;

import uk.ac.cam.eng.extraction.hadoop.datatypes.FeatureMap;
import uk.ac.cam.eng.extraction.hadoop.datatypes.IntWritableCache;
import uk.ac.cam.eng.extraction.hadoop.datatypes.ProvenanceCountMap;
import uk.ac.cam.eng.extraction.hadoop.datatypes.RuleWritable;
import uk.ac.cam.eng.util.Pair;

/**
 * This class filters rules according to constraints in a config
 * 
 * @author Juan Pino
 * @author Aurelien Waite
 * @date 28 May 2014
 */
public class RuleFilter {

	private static class RuleCountComparator implements
			Comparator<Pair<RuleWritable, FeatureMap>> {

		private final IntWritable countIndex;

		public RuleCountComparator(IntWritable countIndex) {
			this.countIndex = countIndex;
		}

		public int compare(Pair<RuleWritable, FeatureMap> a,
				Pair<RuleWritable, FeatureMap> b) {
			// We want descending order!
			int countDiff = b.getSecond().get(countIndex)
					.compareTo(a.getSecond().get(countIndex));
			if (countDiff != 0) {
				return countDiff;
			} else {
				return (a.getFirst().compareTo(b.getFirst()));
			}
		}
	}

	// TODO put the default in the code
	private double minSource2TargetPhrase;
	private double minTarget2SourcePhrase;
	private double minSource2TargetRule;
	private double minTarget2SourceRule;
	// allowed patterns
	private Set<RulePattern> allowedPatterns;
	// skipped patterns: they count towards the maximum number of translation
	// per source
	// threshold but are not included in the filtered rule file
	private Set<RulePattern> skipPatterns;
	private Map<SidePattern, Map<String, Double>> sourcePatternConstraints;
	// decides whether to keep all the rules that fall within the number
	// of translations per source threshold in case of a tie

	private final Map<String, Integer> s2tIndices = new HashMap<>();
	private final Map<String, Integer> t2sIndices = new HashMap<>();
	private final Map<String, RuleCountComparator> comparators = new HashMap<>();

	// when using provenance features, we can either keep the rules coming
	// from the main table and add features corresponding to the provenance
	// tables (default) or keep the union of the rules coming from the main
	// and provenance tables
	private boolean provenanceUnion = false;

	public RuleFilter(Configuration conf) throws FileNotFoundException,
			IOException {
		int alls2t = conf.getInt("source2target_probability-mapreduce", 0);
		s2tIndices.put("", alls2t);
		t2sIndices.put("",
				conf.getInt("target2source_probability-mapreduce", 0));
		comparators.put(
				"",
				new RuleCountComparator(IntWritableCache
						.createIntWritable(alls2t + 1)));
		for (String provenance : conf.getStrings(ProvenanceCountMap.PROV)) {

			int s2tIndex = conf.getInt("provenance_source2target_probability-"
					+ provenance + "-mapreduce", 0);
			s2tIndices.put(provenance, s2tIndex);
			int t2sIndex = conf.getInt("provenance_target2source_probability-"
					+ provenance + "-mapreduce", 0);
			t2sIndices.put(provenance, t2sIndex);
			comparators.put(provenance, new RuleCountComparator(
					IntWritableCache.createIntWritable(s2tIndex + 1)));
		}
		String filterConfig = conf.get("filter_config");
		if (filterConfig == null) {
			System.err
					.println("Missing property 'filter_config' in the config");
			System.exit(1);
		}
		loadConfig(filterConfig);
		System.out.println(sourcePatternConstraints);
	}

	/**
	 * @return the provenanceUnion
	 */
	public boolean isProvenanceUnion() {
		return provenanceUnion;
	}

	// TODO use Properties instead
	private void loadConfig(String configFile) throws FileNotFoundException,
			IOException {
		try (BufferedReader br = new BufferedReader(new FileReader(configFile))) {
			String line;
			String[] parts;
			while ((line = br.readLine()) != null) {
				if (line.startsWith("#")) {
					continue;
				}
				if (line.isEmpty()) {
					continue;
				}
				parts = line.split("\\s+");
				if (parts.length == 0) {
					System.err.println("Malformed config file");
					System.exit(1);
				}
				String[] featureValue = parts[0].split("=", 2);
				if (featureValue[0].equals("min_source2target_phrase")) {
					minSource2TargetPhrase = Double
							.parseDouble(featureValue[1]);
				} else if (featureValue[0].equals("min_target2source_phrase")) {
					minTarget2SourcePhrase = Double
							.parseDouble(featureValue[1]);
				} else if (featureValue[0].equals("min_source2target_rule")) {
					minSource2TargetRule = Double.parseDouble(featureValue[1]);
				} else if (featureValue[0].equals("min_target2source_rule")) {
					minTarget2SourceRule = Double.parseDouble(featureValue[1]);
				} else if (featureValue[0].equals("allowed_source_pattern")) {
					if (sourcePatternConstraints == null) {
						sourcePatternConstraints = new HashMap<SidePattern, Map<String, Double>>();
					}
					Map<String, Double> constraints = new HashMap<String, Double>();
					for (int i = 1; i < parts.length; i++) {
						String[] constraintValue = parts[i].split("=", 2);
						constraints.put(constraintValue[0],
								Double.parseDouble(constraintValue[1]));
					}
					sourcePatternConstraints.put(
							SidePattern.parsePattern(featureValue[1]),
							constraints);
				} else if (featureValue[0].equals("allowed_pattern")) {
					if (allowedPatterns == null) {
						allowedPatterns = new HashSet<RulePattern>();
					}
					allowedPatterns.add(RulePattern
							.parsePattern(featureValue[1]));
				} else if (featureValue[0].equals("skip_pattern")) {
					if (skipPatterns == null) {
						skipPatterns = new HashSet<>();
					}
					skipPatterns.add(RulePattern.parsePattern(featureValue[1]));
				} else if (featureValue[0].equals("provenance_union")) {
					provenanceUnion = Boolean.parseBoolean(featureValue[1]);
				}
			}
		}
	}

	public Comparator<Pair<RuleWritable, FeatureMap>> getComparator(String prov) {
		return comparators.get(prov);
	}

	public boolean filterSource(Text source) {
		SidePattern sourcePattern = SidePattern.getPattern(source.toString());
		if (sourcePattern.isPhrase()) {
			return false;
		} else if (sourcePatternConstraints.containsKey(sourcePattern)) {
			return false;
		}
		return true;
	}

	public List<Pair<RuleWritable, FeatureMap>> filterRulesBySource(
			SidePattern sourcePattern,
			SortedSet<Pair<RuleWritable, FeatureMap>> rules, String provenance) {
		List<Pair<RuleWritable, FeatureMap>> results = new ArrayList<>();
		int numberTranslations = 0;
		int numberTranslationsMonotone = 0; // case with more than 1 NT
		int numberTranslationsInvert = 0;
		int prevCount = -1;
		IntWritable countIndex = IntWritableCache.createIntWritable(s2tIndices
				.get(provenance) + 1);
		for (Pair<RuleWritable, FeatureMap> entry : rules) {
			// number of translations per source threshold
			// in case of ties we either keep or don't keep the ties
			// depending on the config

			RulePattern rulePattern = RulePattern.getPattern(entry.getFirst(),
					entry.getFirst());
			boolean doNotSkip = skipPatterns == null
					|| !skipPatterns.contains(rulePattern);
			int count = (int) entry.getSecond().get(countIndex).get();

			if (doNotSkip && !sourcePattern.isPhrase()) {
				if (sourcePattern.hasMoreThan1NT()) {
					if (sourcePatternConstraints.get(sourcePattern).get(
							"ntrans") <= numberTranslationsMonotone
							&& sourcePatternConstraints.get(sourcePattern).get(
									"ntrans") <= numberTranslationsInvert
							&& count != prevCount) {
						break;
					}
				} else if (sourcePatternConstraints.get(sourcePattern).get(
						"ntrans") <= numberTranslations
						&& count != prevCount) {
					break;
				}
			}

			if (sourcePattern.isPhrase()) {
				results.add(entry);
			} else if (doNotSkip) {
				if (sourcePattern.hasMoreThan1NT()) {

					if (rulePattern.isSwappingNT()
							&& (count == prevCount || sourcePatternConstraints
									.get(sourcePattern).get("ntrans") > numberTranslationsInvert)) {
						results.add(entry);
					} else if (count == prevCount
							|| sourcePatternConstraints.get(sourcePattern).get(
									"ntrans") > numberTranslationsMonotone) {
						results.add(entry);
					}
				} else {
					results.add(entry);
				}
			}
			if (sourcePattern.hasMoreThan1NT()) {
				if (rulePattern.isSwappingNT()) {
					numberTranslationsInvert++;
				} else {
					numberTranslationsMonotone++;
				}
			}
			numberTranslations++;
			prevCount = count;

		}
		return results;
	}

	/**
	 * Indicates whether we should filter a rule. The decision is based on a
	 * particular provenance.
	 * 
	 * @param sourcePattern The source pattern (e.g. w X)
	 * @param rule The rule
	 * @param features The features for the rule. The filtering criteria are
	 * based on these features.
	 * @param provenance The provenance used as a criterion for filtering.
	 * @return
	 */
	public boolean filterRule(SidePattern sourcePattern, RuleWritable rule,
			FeatureMap features, String provenance) {
		int s2tIndex = s2tIndices.get(provenance);
		IntWritable source2targetProbabilityIndex = IntWritableCache
				.createIntWritable(s2tIndex);
		// if the rule does not have that provenance, we automatically filter it
		if (!features.containsKey(source2targetProbabilityIndex)) {
			return true;
		}
		int t2sIndex = t2sIndices.get(provenance);
		IntWritable target2sourceProbabilityIndex = IntWritableCache
				.createIntWritable(t2sIndex);
		IntWritable countIndex = IntWritableCache
				.createIntWritable(s2tIndex + 1);

		double source2targetProbability = features.get(
				source2targetProbabilityIndex).get();
		double target2sourceProbability = features.get(
				target2sourceProbabilityIndex).get();
		int numberOfOccurrences = (int) features.get(countIndex).get();

		RulePattern rulePattern = RulePattern.getPattern(rule, rule);
		if (!sourcePattern.isPhrase()
				&& !allowedPatterns.contains(rulePattern)
				&& (skipPatterns == null || !skipPatterns.contains(rulePattern))) {
			return true;
		}
		if (sourcePattern.isPhrase()) {
			// source-to-target threshold
			if (source2targetProbability <= minSource2TargetPhrase) {
				return true;
			}
			// target-to-source threshold
			if (target2sourceProbability <= minTarget2SourcePhrase) {
				return true;
			}
		} else {
			// source-to-target threshold
			if (source2targetProbability <= minSource2TargetRule) {
				return true;
			}
			// target-to-source threshold
			if (target2sourceProbability <= minTarget2SourceRule) {
				return true;
			}
			// minimum number of occurrence threshold
			if (sourcePatternConstraints.get(sourcePattern).containsKey("nocc")) {
				if (numberOfOccurrences < sourcePatternConstraints.get(
						sourcePattern).get("nocc")) {
					return true;
				}
			}
		}
		return false;
	}
}
