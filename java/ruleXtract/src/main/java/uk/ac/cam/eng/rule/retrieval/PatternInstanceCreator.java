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

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import uk.ac.cam.eng.extraction.Rule;
import uk.ac.cam.eng.extraction.Symbol;
import uk.ac.cam.eng.util.CLI;

/**
 * This class creates pattern instances given a test set
 * 
 * @author Juan Pino
 * @author Aurelien Waite
 * @date 28 May 2014
 */
class PatternInstanceCreator {

	private final int maxSourcePhrase;

	private final int maxSourceElements;

	private final int maxTerminalLength;

	private final int maxNonTerminalSpan;

	private final int hrMaxHeight;

	private final Set<SidePattern> sidePatterns;

	public PatternInstanceCreator(
			CLI.RuleRetrieverParameters params,
			Collection<SidePattern> sidePatterns) {
		maxSourcePhrase = params.rp.maxSourcePhrase;
		maxSourceElements = params.rp.maxSourceElements;
		maxTerminalLength = params.rp.maxTerminalLength;
		maxNonTerminalSpan = params.rp.maxNonTerminalSpan;
		hrMaxHeight = params.hr_max_height;
		this.sidePatterns = new HashSet<>(sidePatterns);
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
			List<Symbol> sourcePhrase = new ArrayList<Symbol>();
			for (int j = 0; j < maxSourcePhrase && j < parts.length - i; j++) {
				sourcePhrase.add(Symbol.deserialise(Integer.parseInt(parts[i + j])));
				// add source phrase
				Rule r = new Rule(sourcePhrase, new ArrayList<Symbol>());
				res.add(r);
			}
		}
		Set<Rule> sourcePatternInstances = getPatternInstancesFromSourceSentence(
				sourceSentence, sidePatterns);
		for(Rule r : sourcePatternInstances){
			if(sidePatterns.contains(r.source().toPattern())){
				res.add(r);
			}
		}
		return res;
	}

	private Set<Rule> getPatternInstancesFromSourceSentence(
			List<Integer> sourceSentence, Set<SidePattern> sidePatterns) {
		Set<Rule> res = new HashSet<Rule>();
		for (SidePattern sidePattern : sidePatterns) {
			for (int i = 0; i < sourceSentence.size(); i++) {
				res.addAll(getPatternInstancesFromSourceAndPattern2(
						sourceSentence, sidePattern, i, 0, 0, 0));
			}
		}
		return res;
	}

	private Set<Rule> merge(Rule partialLeft, Set<Rule> partialRight) {
		Set<Rule> res = new HashSet<Rule>();
		List<Symbol> sourceLeft = partialLeft.getSource();
		if (partialRight.isEmpty()) {
			res.add(new Rule(sourceLeft, new ArrayList<Symbol>()));
			return res;
		}
		for (Rule r : partialRight) {
			List<Symbol> merged = new ArrayList<Symbol>();
			List<Symbol> sourceRight = r.getSource();
			merged.addAll(sourceLeft);
			merged.addAll(sourceRight);
			res.add(new Rule(merged, new ArrayList<Symbol>()));
		}
		return res;
	}

	private Set<Rule> getPatternInstancesFromSourceAndPattern2(
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
		// we already have max source elements
		if (nbSrcElt >= maxSourceElements) {
			return res;
		}
		// we already cover hr max height
		if (nbCoveredWords >= hrMaxHeight) {
			return res;
		}
		if (sourceSentence.size() - startSentenceIndex == sidePattern.size()
				- startPatternIndex) {
			if (nbSrcElt + sidePattern.size() - startPatternIndex > maxSourceElements) {
				return res;
			}
			if (nbCoveredWords + sourceSentence.size() - startSentenceIndex > hrMaxHeight) {
				return res;
			}
			List<Symbol> patternInstance = new ArrayList<Symbol>();
			for (int i = 0; i < sourceSentence.size() - startSentenceIndex; i++) {
				if (sidePattern.get(startPatternIndex + i).equals("w")) {
					patternInstance.add(Symbol.deserialise(sourceSentence.get(startSentenceIndex
							+ i)));
				} else {
					patternInstance.add(Symbol.deserialise(Integer.parseInt(sidePattern
							.get(startPatternIndex + i))));
				}
			}
			Rule r = new Rule(patternInstance, new ArrayList<Symbol>());
			res.add(r);
			return res;
		}
		List<Symbol> partialPattern = new ArrayList<Symbol>();
		if (sidePattern.get(startPatternIndex).equals("w")) {
			for (int i = startSentenceIndex; i < sourceSentence.size()
					- (sidePattern.size() - startPatternIndex - 1)
					&& i < startSentenceIndex + maxTerminalLength
					&& i < startSentenceIndex + maxSourceElements - nbSrcElt
					&& i < startSentenceIndex + hrMaxHeight - nbCoveredWords; i++) {
				partialPattern.add(Symbol.deserialise(sourceSentence.get(i)));
				Rule r = new Rule(partialPattern, new ArrayList<Symbol>());
				Set<Rule> right = getPatternInstancesFromSourceAndPattern2(
						sourceSentence, sidePattern, i + 1,
						startPatternIndex + 1, nbSrcElt + i
								- startSentenceIndex + 1, nbCoveredWords + i
								- startSentenceIndex + 1);
				Set<Rule> merged = merge(r, right);
				res.addAll(merged);
			}
		} else {
			partialPattern.add(Symbol.deserialise(Integer.parseInt(sidePattern
					.get(startPatternIndex))));
			Rule r = new Rule(partialPattern, new ArrayList<Symbol>());
			for (int i = startSentenceIndex; i < sourceSentence.size()
					- (sidePattern.size() - startPatternIndex - 1)
					&& i < startSentenceIndex + maxNonTerminalSpan
					&& i < startSentenceIndex + hrMaxHeight - nbCoveredWords; i++) {
				Set<Rule> merged = merge(
						r,
						getPatternInstancesFromSourceAndPattern2(
								sourceSentence, sidePattern, i + 1,
								startPatternIndex + 1, nbSrcElt + 1,
								nbCoveredWords + i - startSentenceIndex + 1));
				res.addAll(merged);
			}
		}
		return res;
	}
}
