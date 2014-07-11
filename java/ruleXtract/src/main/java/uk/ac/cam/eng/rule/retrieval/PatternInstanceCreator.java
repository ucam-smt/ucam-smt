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

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.hadoop.conf.Configuration;

import uk.ac.cam.eng.extraction.datatypes.Rule;

// TODO rename this to PatternInstanceCreator

/**
 * This class creates pattern instances given a test set
 * 
 * @author Juan Pino
 * @date 28 May 2014
 */
public class PatternInstanceCreator {

	// protected for testing
	protected int MAX_SOURCE_PHRASE = 5; // TODO revise this value, put in
											// constructor or something
	protected int MAX_SOURCE_ELEMENTS = 5; // TODO revise this value, put in
											// constructor or something
	protected int MAX_TERMINAL_LENGTH = 5; // TODO revise this value, put in
											// constructor or something
	protected int MAX_NONTERMINAL_LENGTH = 10; // TODO revise this value, put in

	protected int HR_MAX_HEIGHT = 10;

	private String patternFile;

	List<SidePattern> sidePatterns = new ArrayList<SidePattern>();

	public PatternInstanceCreator(Configuration conf) {
		MAX_SOURCE_PHRASE = conf.getInt("max_source_phrase", 5);
		MAX_SOURCE_ELEMENTS = conf.getInt("max_source_elements", 5);
		MAX_TERMINAL_LENGTH = conf.getInt("max_terminal_length", 5);
		MAX_NONTERMINAL_LENGTH = conf.getInt("max_nonterminal_length", 10);
		HR_MAX_HEIGHT = conf.getInt("hr_max_height", 10);
		patternFile = conf.get("source_patterns");
		if (patternFile == null) {
			System.err.println("Missing property 'patternfile' in the config");
			System.exit(1);
		}
	}

	public void createSourcePatterns() throws FileNotFoundException,
			IOException {
		try (BufferedReader br = new BufferedReader(new FileReader(patternFile))) {
			String line;
			String[] parts;
			while ((line = br.readLine()) != null) {
				parts = line.split("\\s+");
				sidePatterns.add(new SidePattern(Arrays.asList(parts)));
			}
		}
	}

	/**
	 * @param sidePatterns
	 * @param res
	 * @param line
	 * @return
	 */
	public Set<Rule> createSourcePatternInstances(String line) {
		Set<Rule> res = new HashSet<>();
		String[] parts;
		parts = line.split(" ");
		List<Integer> sourceSentence = new ArrayList<Integer>();
		for (int i = 0; i < parts.length; i++) {
			sourceSentence.add(Integer.parseInt(parts[i]));
			List<Integer> sourcePhrase = new ArrayList<Integer>();
			for (int j = 0; j < MAX_SOURCE_PHRASE && j < parts.length - i; j++) {
				sourcePhrase.add(Integer.parseInt(parts[i + j]));
				// add source phrase
				Rule r = new Rule(sourcePhrase, new ArrayList<Integer>());
				res.add(r);
			}
		}
		Set<Rule> sourcePatternInstances = getPatternInstancesFromSourceSentence(
				sourceSentence, sidePatterns);
		res.addAll(sourcePatternInstances);
		return res;
	}

	protected Set<Rule> getPatternInstancesFromSourceSentence(
			List<Integer> sourceSentence, List<SidePattern> sidePatterns) {
		Set<Rule> res = new HashSet<Rule>();
		for (SidePattern sidePattern : sidePatterns) {
			for (int i = 0; i < sourceSentence.size(); i++) {
				res.addAll(getPatternInstancesFromSourceAndPattern2(
						sourceSentence, sidePattern, i, 0, 0, 0));
			}
		}
		return res;
	}

	protected Set<Rule> merge(Rule partialLeft, Set<Rule> partialRight) {
		Set<Rule> res = new HashSet<Rule>();
		List<Integer> sourceLeft = partialLeft.getSource();
		if (partialRight.isEmpty()) {
			res.add(new Rule(sourceLeft, new ArrayList<Integer>()));
			return res;
		}
		for (Rule r : partialRight) {
			List<Integer> merged = new ArrayList<Integer>();
			List<Integer> sourceRight = r.getSource();
			merged.addAll(sourceLeft);
			merged.addAll(sourceRight);
			res.add(new Rule(merged, new ArrayList<Integer>()));
		}
		return res;
	}

	protected Set<Rule> getPatternInstancesFromSourceAndPattern2(
			List<Integer> sourceSentence, SidePattern sidePattern,
			int startSentenceIndex, int startPatternIndex, int nbSrcElt,
			int nbCoveredWords) {
		Set<Rule> res = new HashSet<Rule>();
		if (startSentenceIndex >= sourceSentence.size()) {
			return res;
		}
		if (startPatternIndex >= sidePattern.size()) {
			return res;
		}
		// pattern is too big for the (rest of the) sentence, e.g. pattern wXw
		// for the phrase 2_3
		if (sourceSentence.size() - startSentenceIndex < sidePattern.size()
				- startPatternIndex) {
			return res;
		}
		// we already have MAX_SOURCE_ELEMENTS source elements
		if (nbSrcElt >= MAX_SOURCE_ELEMENTS) {
			return res;
		}
		// we already cover HR_MAX_HEIGHT
		if (nbCoveredWords >= HR_MAX_HEIGHT) {
			return res;
		}
		if (sourceSentence.size() - startSentenceIndex == sidePattern.size()
				- startPatternIndex) {
			if (nbSrcElt + sidePattern.size() - startPatternIndex > MAX_SOURCE_ELEMENTS) {
				return res;
			}
			if (nbCoveredWords + sourceSentence.size() - startSentenceIndex > HR_MAX_HEIGHT) {
				return res;
			}
			List<Integer> patternInstance = new ArrayList<Integer>();
			for (int i = 0; i < sourceSentence.size() - startSentenceIndex; i++) {
				if (sidePattern.get(startPatternIndex + i).equals("w")) {
					patternInstance.add(sourceSentence.get(startSentenceIndex
							+ i));
				} else {
					patternInstance.add(Integer.parseInt(sidePattern
							.get(startPatternIndex + i)));
				}
			}
			Rule r = new Rule(patternInstance, new ArrayList<Integer>());
			res.add(r);
			return res;
		}
		List<Integer> partialPattern = new ArrayList<Integer>();
		if (sidePattern.get(startPatternIndex).equals("w")) {
			for (int i = startSentenceIndex; i < sourceSentence.size()
					- (sidePattern.size() - startPatternIndex - 1)
					&& i < startSentenceIndex + MAX_TERMINAL_LENGTH
					&& i < startSentenceIndex + MAX_SOURCE_ELEMENTS - nbSrcElt
					&& i < startSentenceIndex + HR_MAX_HEIGHT - nbCoveredWords; i++) {
				partialPattern.add(sourceSentence.get(i));
				Rule r = new Rule(partialPattern, new ArrayList<Integer>());
				Set<Rule> right = getPatternInstancesFromSourceAndPattern2(
						sourceSentence, sidePattern, i + 1,
						startPatternIndex + 1, nbSrcElt + i
								- startSentenceIndex + 1, nbCoveredWords + i
								- startSentenceIndex + 1);
				Set<Rule> merged = merge(r, right);
				res.addAll(merged);
			}
		} else {
			partialPattern.add(Integer.parseInt(sidePattern
					.get(startPatternIndex)));
			Rule r = new Rule(partialPattern, new ArrayList<Integer>());
			for (int i = startSentenceIndex; i < sourceSentence.size()
					- (sidePattern.size() - startPatternIndex - 1)
					&& i < startSentenceIndex + MAX_NONTERMINAL_LENGTH
					&& i < startSentenceIndex + HR_MAX_HEIGHT - nbCoveredWords; i++) {
				Set<Rule> merged = merge(
						r,
						getPatternInstancesFromSourceAndPattern2(
								sourceSentence, sidePattern, i + 1,
								startPatternIndex + 1, nbSrcElt + 1,
								nbCoveredWords + i - startSentenceIndex + 1));
				// System.err.println("merged: " + merged);
				res.addAll(merged);
			}
		}
		return res;
	}
}
