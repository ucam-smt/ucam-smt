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

package uk.ac.cam.eng.rule.filtering;

// TODO remove hard coded indices

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.function.Consumer;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.ByteWritable;
import org.apache.hadoop.io.IntWritable;

import uk.ac.cam.eng.extraction.Rule;
import uk.ac.cam.eng.extraction.RuleString;
import uk.ac.cam.eng.extraction.hadoop.datatypes.IntWritableCache;
import uk.ac.cam.eng.extraction.hadoop.datatypes.RuleData;
import uk.ac.cam.eng.rule.features.Feature;
import uk.ac.cam.eng.rule.retrieval.RulePattern;
import uk.ac.cam.eng.rule.retrieval.SidePattern;
import uk.ac.cam.eng.util.CLI;
import uk.ac.cam.eng.util.Pair;

/**
 * This class filters rules according to constraints
 * 
 * @author Juan Pino
 * @author Aurelien Waite
 * @date 28 May 2014
 */
public class RuleFilter {

	private static class RuleCountComparator implements
			Comparator<Pair<Rule, RuleData>> {

		private final ByteWritable countIndex;

		public RuleCountComparator(int countIndex) {
			this.countIndex = new ByteWritable((byte) countIndex);
		}

		@Override
		public int compare(Pair<Rule, RuleData> a, Pair<Rule, RuleData> b) {
			int aValue = a.getSecond().getProvCounts().containsKey(countIndex) ? a
					.getSecond().getProvCounts().get(countIndex).get()
					: 0;
			int bValue = b.getSecond().getProvCounts().containsKey(countIndex) ? b
					.getSecond().getProvCounts().get(countIndex).get()
					: 0;
			// We want descending order!
			int countDiff = bValue < aValue ? -1 : (bValue == aValue ? 0 : 1);
			if (countDiff != 0) {
				return countDiff;
			} else {
				return (a.getFirst().compareTo(b.getFirst()));
			}
		}
	}

	private static class SourcePhraseConstraint {
		// Number of Occurrences
		final int nOcc;
		// Number of Translations
		final int nTrans;

		public SourcePhraseConstraint(String nOcc, String nTrans) {
			this.nOcc = Integer.parseInt(nOcc);
			this.nTrans = Integer.parseInt(nTrans);
		}
	}

	final private double minSource2TargetPhraseLog;
	final private double minTarget2SourcePhraseLog;
	final private double minSource2TargetRuleLog;
	final private double minTarget2SourceRuleLog;
	// allowed patterns
	private Set<RulePattern> allowedPatterns = new HashSet<RulePattern>();

	private Map<SidePattern, SourcePhraseConstraint> sourcePatternConstraints = new HashMap<>();

	boolean provenanceUnion;

	public RuleFilter(CLI.FilterParams params, Configuration conf)
			throws FileNotFoundException, IOException {
		provenanceUnion = params.provenanceUnion;
		minSource2TargetPhraseLog = Math.log(params.minSource2TargetPhrase);
		minTarget2SourcePhraseLog = Math.log(params.minTarget2SourcePhrase);
		minSource2TargetRuleLog = Math.log(params.minSource2TargetRule);
		minTarget2SourceRuleLog = Math.log(params.minTarget2SourceRule);
		loadConfig(params.allowedPatternsFile, conf,
				line -> allowedPatterns.add(RulePattern.parsePattern(line)));
		loadConfig(params.sourcePatterns, conf,
				line -> {
					String[] parts = line.split(" ");
					if (parts.length != 3) {
						throw new RuntimeException(
								"line should have 3 fields: " + line);
					}
					sourcePatternConstraints.put(
							SidePattern.parsePattern(parts[0]),
							new SourcePhraseConstraint(parts[1], parts[2]));
				});
	}

	private void loadConfig(String fileName, Configuration conf,
			Consumer<String> block) throws FileNotFoundException, IOException {
		Path path = new Path(fileName);
		FileSystem fs = path.getFileSystem(conf);
		try (BufferedReader br = new BufferedReader(new InputStreamReader(
				fs.open(path)))) {
			for (String line = br.readLine(); line != null; line = br
					.readLine()) {
				if (line.startsWith("#") || line.isEmpty()) {
					continue;
				}
				block.accept(line);
			}
		}
	}

	public boolean filterSource(RuleString source) {
		SidePattern sourcePattern = source.toPattern();
		if (sourcePattern.isPhrase()) {
			return false;
		} else if (sourcePatternConstraints.containsKey(sourcePattern)) {
			return false;
		}
		return true;
	}

	private Map<Rule, RuleData> filterRulesBySource(SidePattern sourcePattern,
			List<Pair<Rule, RuleData>> rules, int provMapping) {
		Map<Rule, RuleData> results = new HashMap<>();
		// If the source side is a phrase, then we want everything
		if (sourcePattern.isPhrase()) {
			rules.forEach(entry -> results.put(entry.getFirst(),
					entry.getSecond()));
			return results;
		}
		int numberTranslations = 0;
		int numberTranslationsMonotone = 0; // case with more than 1 NT
		int numberTranslationsInvert = 0;
		int prevCount = -1;
		double nTransConstraint = sourcePatternConstraints.get(sourcePattern).nTrans;
		ByteWritable countIndex = new ByteWritable((byte) provMapping);
		for (Pair<Rule, RuleData> entry : rules) {
			// number of translations per source threshold
			// in case of ties we either keep or don't keep the ties
			// depending on the config
			RulePattern rulePattern = RulePattern.getPattern(entry.getFirst());
			int count = (int) entry.getSecond().getProvCounts().get(countIndex)
					.get();
			boolean notTied = count != prevCount;
			boolean moreThan1NT = sourcePattern.hasMoreThan1NT();
			if (notTied
					&& ((moreThan1NT
							&& nTransConstraint <= numberTranslationsMonotone && nTransConstraint <= numberTranslationsInvert) || (!moreThan1NT && nTransConstraint <= numberTranslations))) {
				break;
			}
			results.put(entry.getFirst(), entry.getSecond());
			if (moreThan1NT) {
				if (rulePattern.isSwappingNT()) {
					++numberTranslationsInvert;
				} else {
					++numberTranslationsMonotone;
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
	 * @return
	 */
	private boolean filterRule(Feature s2t, Feature t2s,
			SidePattern sourcePattern, Rule rule, RuleData data, int provMapping) {
		IntWritable provIW = IntWritableCache.createIntWritable(provMapping);
		// Immediately filter if there is data for this rule under this
		// provenance
		if (!data.getFeatures().get(s2t).containsKey(provIW)) {
			return true;
		}
		RulePattern rulePattern = RulePattern.getPattern(rule);
		if (!(sourcePattern.isPhrase() || allowedPatterns.contains(rulePattern))) {
			return true;
		}
		double source2targetProbability = data.getFeatures().get(s2t)
				.get(provIW).get();
		double target2sourceProbability = data.getFeatures().get(t2s)
				.get(provIW).get();
		int numberOfOccurrences = (int) data.getProvCounts()
				.get(new ByteWritable((byte) provMapping)).get();

		if (sourcePattern.isPhrase()) {
			// source-to-target threshold
			if (source2targetProbability <= minSource2TargetPhraseLog) {
				return true;
			}
			// target-to-source threshold
			if (target2sourceProbability <= minTarget2SourcePhraseLog) {
				return true;
			}
		} else {
			// source-to-target threshold
			if (source2targetProbability <= minSource2TargetRuleLog) {
				return true;
			}
			// target-to-source threshold
			if (target2sourceProbability <= minTarget2SourceRuleLog) {
				return true;
			}
			// minimum number of occurrence threshold
			if (numberOfOccurrences < sourcePatternConstraints
					.get(sourcePattern).nOcc) {
				return true;
			}
		}
		return false;
	}

	public Map<Rule, RuleData> filter(SidePattern sourcePattern,
			List<Pair<Rule, RuleData>> toFilter) {
		// Establish the provenances used in these rules
		Set<Integer> provenances = new HashSet<>();
		if (!provenanceUnion) {
			provenances.add(0);
		} else {
			for (Pair<?, RuleData> entry : toFilter) {
				for (ByteWritable prov : entry.getSecond().getProvCounts()
						.keySet()) {
					provenances.add((int) prov.get());
				}
			}
		}
		Map<Rule, RuleData> filtered = new HashMap<>();
		for (int i : provenances) {
			Feature s2t = i == 0 ? Feature.SOURCE2TARGET_PROBABILITY
					: Feature.PROVENANCE_SOURCE2TARGET_PROBABILITY;
			Feature t2s = i == 0 ? Feature.TARGET2SOURCE_PROBABILITY
					: Feature.PROVENANCE_TARGET2SOURCE_PROBABILITY;
			List<Pair<Rule, RuleData>> rules = new LinkedList<>();
			Collections.sort(toFilter, new RuleCountComparator(i));
			for (Pair<Rule, RuleData> entry : toFilter) {
				Rule rule = entry.getFirst();
				RuleData data = entry.getSecond();
				if (!filterRule(s2t, t2s, sourcePattern, rule, data, i)) {
					rules.add(Pair.createPair(new Rule(rule), data));
				}
			}
			filtered.putAll(filterRulesBySource(sourcePattern, rules, i));
		}
		return filtered;
	}

	public Collection<SidePattern> getPermittedSourcePatterns() {
		return sourcePatternConstraints.keySet();
	}
}
