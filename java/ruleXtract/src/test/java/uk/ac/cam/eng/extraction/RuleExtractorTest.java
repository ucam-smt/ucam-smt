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

package uk.ac.cam.eng.extraction;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.URISyntaxException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.hadoop.conf.Configuration;
import org.junit.Assert;
import org.junit.Test;

import uk.ac.cam.eng.extraction.datatypes.Alignment;
import uk.ac.cam.eng.extraction.datatypes.AlignmentLink;
import uk.ac.cam.eng.extraction.datatypes.Rule;
import uk.ac.cam.eng.extraction.datatypes.SentencePair;
import uk.ac.cam.eng.util.Pair;

/**
 * @author jmp84
 *
 */
public class RuleExtractorTest {

	private Map<Rule, Pair<Integer, Map<List<AlignmentLink>, Integer>>>
			getExpectedRules() throws FileNotFoundException, IOException, URISyntaxException {
		Map<Rule, Pair<Integer, Map<List<AlignmentLink>, Integer>>> res = 
				new HashMap<>();
		try (BufferedReader br =
				new BufferedReader(
						new InputStreamReader(
								new FileInputStream(
										new File(
												getClass().getResource(
														"/extractedRules.txt").toURI()))))) {
			String line = null;
			while ((line = br.readLine()) != null) {
				String[] parts = line.split(" ");
				if (parts.length != 4) {
					throw new RuntimeException("Malformed rule: " + line);
				}
				int lhs = Integer.parseInt(parts[0]);
				String[] sourceWords = parts[1].split("_");
				String[] targetWords = parts[2].split("_");
				List<Integer> sourceIds = new ArrayList<>();
				List<Integer> targetIds = new ArrayList<>();
				for (String sourceWord : sourceWords) {
					sourceIds.add(Integer.parseInt(sourceWord));
				}
				for (String targetWord : targetWords) {
					targetIds.add(Integer.parseInt(targetWord));
				}
				String[] alignmentLinks = parts[3].split("_");
				List<AlignmentLink> alignment = new ArrayList<>();
				for (String alignmentLink : alignmentLinks) {
					String[] srcTrg = alignmentLink.split("-");
					if (srcTrg.length != 2) {
						throw new RuntimeException(
								"Malformed rule (alignment): " + line);
					}
					alignment.add(new AlignmentLink(
							Integer.parseInt(srcTrg[0]),
							Integer.parseInt(srcTrg[1])));
				}
				Rule r = new Rule(lhs, sourceIds, targetIds);
				if (!res.containsKey(r)) {
					Map<List<AlignmentLink>, Integer> alignments =
							new HashMap<>();
					alignments.put(alignment, 1);
					res.put(r, new Pair<>(1, alignments));
				} else {
					Pair<Integer, Map<List<AlignmentLink>, Integer>> countAlignments =
							res.get(r);
					countAlignments.setFirst(countAlignments.getFirst() + 1);
					Map<List<AlignmentLink>, Integer> alignments =
							countAlignments.getSecond();
					if (!alignments.containsKey(alignment)) {
						alignments.put(alignment, 1);
					} else {
						alignments.put(alignment, alignments.get(alignment) + 1);
					}
				}
			}
		}
		return res;
	}

	private Map<Rule, Pair<Integer, Map<List<AlignmentLink>, Integer>>>
			getActualRules(List<Rule> extractedRules) {
		Map<Rule, Pair<Integer, Map<List<AlignmentLink>, Integer>>> res = 
				new HashMap<>();
		for (Rule r : extractedRules) {
			List<AlignmentLink> alignment = r.getAlignment();
			if (!res.containsKey(r)) {
				Map<List<AlignmentLink>, Integer> alignments =
						new HashMap<>();
				alignments.put(alignment, 1);
				res.put(r, new Pair<>(1, alignments));
			} else {
				Pair<Integer, Map<List<AlignmentLink>, Integer>> countAlignments =
						res.get(r);
				countAlignments.setFirst(countAlignments.getFirst() + 1);
				Map<List<AlignmentLink>, Integer> alignments =
						countAlignments.getSecond();
				if (!alignments.containsKey(alignment)) {
					alignments.put(alignment, 1);
				} else {
					alignments.put(alignment, alignments.get(alignment) + 1);
				}
			}
		}
		return res;
	}

	/**
	 * Test method for {@link uk.ac.cam.eng.extraction.RuleExtractor#extract(uk.ac.cam.eng.extraction.datatypes.Alignment, uk.ac.cam.eng.extraction.datatypes.SentencePair)}.
	 * @throws URISyntaxException 
	 * @throws IOException 
	 * @throws FileNotFoundException 
	 */
	@Test
	public final void testExtract() throws FileNotFoundException, IOException, URISyntaxException {
		SentencePair sentencePair = new SentencePair(
				"45187 82073 15 22 28500 18 2575 31846 3 102 25017 133794 19 21379 5 566 957608 3532 5 26635 155153 725236 4",
				"5023 8107 12 11 1547 14 205 55755 25 12 1226 22 11 36053 26 158559 16746 53 6119 9 3 16497 14412 115 10105 113 6 3 2904 514343 16497 5");
		Alignment alignment = new Alignment(
				"0-0 1-1 2-2 3-2 3-3 4-4 5-5 6-6 7-7 9-8 10-9 11-10 15-11 10-13 16-13 16-14 11-15 16-15 11-16 12-17 13-18 14-19 15-21 16-21 16-22 16-23 19-24 20-28 16-29 17-29 21-29 21-30 22-31",
				sentencePair);
		Configuration conf = new Configuration();
		conf.setInt("max_source_phrase", 9);
		conf.setInt("max_source_elements", 5);
		conf.setInt("max_terminal_length", 5);
		conf.setInt("max_nonterminal_length", 10);
		conf.setBoolean("remove_monotonic_repeats", true);
		RuleExtractor ruleExtractor = new RuleExtractor(conf);
		List<Rule> extractedRules =
				ruleExtractor.extract(alignment, sentencePair);
		Map<Rule, Pair<Integer, Map<List<AlignmentLink>, Integer>>>
				actualRules = getActualRules(extractedRules);
		Map<Rule, Pair<Integer, Map<List<AlignmentLink>, Integer>>>
				expectedRules = getExpectedRules();
		Assert.assertEquals(expectedRules, actualRules);
	}

}
