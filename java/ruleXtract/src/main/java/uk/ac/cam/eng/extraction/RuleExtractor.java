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

package uk.ac.cam.eng.extraction;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.logging.Logger;

import org.apache.hadoop.conf.Configuration;

import uk.ac.cam.eng.extraction.datatypes.Alignment;
import uk.ac.cam.eng.extraction.datatypes.Block;
import uk.ac.cam.eng.extraction.datatypes.Rule;
import uk.ac.cam.eng.extraction.datatypes.SentencePair;
import uk.ac.cam.eng.util.Pair;

/**
 * This class extracts hiero rules. The inputs are an Alignment and a
 * SentencePair.
 * 
 * @author Juan Pino
 * @date 28 May 2014
 */
public class RuleExtractor {

	// TODO replace logger with hadoop logger
	private static Logger logger = Logger
			.getLogger("uk.ac.cam.eng.extraction.ruleextractor");

	// protected for testing
	protected int MAX_SOURCE_PHRASE = 5; // TODO revise this value, put in
											// constructor or something
	protected int MAX_SOURCE_ELEMENTS = 5; // TODO revise this value, put in
											// constructor or something
	protected int MAX_TERMINAL_LENGTH = 5; // TODO revise this value, put in
											// constructor or something
	protected int MAX_NONTERMINAL_LENGTH = 10; // TODO revise this value, put in
												// constructor or something

	// in case of monotonic alignment of a block, a nonterminal can cover
	// different areas, and the same rule can be extracted several times. If
	// the following variable is true, then the same rule is only counted once.
	protected boolean REMOVE_MONOTONIC_REPEATS = false;
	private Map<Integer, Map<Integer, Boolean>> XtermX;
	private Map<Integer, Map<Integer, Boolean>> termX;
	private Map<Integer, Map<Integer, Boolean>> Xterm;

	private boolean source2target;

	public RuleExtractor(Configuration conf) {
		MAX_SOURCE_PHRASE = conf.getInt("max_source_phrase", 5);
		MAX_SOURCE_ELEMENTS = conf.getInt("max_source_elements", 5);
		MAX_TERMINAL_LENGTH = conf.getInt("max_terminal_length", 5);
		MAX_NONTERMINAL_LENGTH = conf.getInt("max_nonterminal_length", 10);
		source2target = conf.getBoolean("source2target", true);
		REMOVE_MONOTONIC_REPEATS = conf.getBoolean("remove_monotonic_repeats",
				false);
		XtermX = new HashMap<Integer, Map<Integer, Boolean>>();
		termX = new HashMap<Integer, Map<Integer, Boolean>>();
		Xterm = new HashMap<Integer, Map<Integer, Boolean>>();
	}

	private static String usage() {
		String res = "Usage: RuleExtractor configFile";
		return res;
	}

	/**
	 * Extracts rules, for now extract phrase pairs according to Viterbi
	 * alignments
	 * 
	 * @param a The alignment
	 * @param sp The sentence pair
	 * @return a list of rules
	 */
	public List<Rule> extract(Alignment a, SentencePair sp) {
		List<Rule> res = extractPhrasePairs(a, sp);
		if (REMOVE_MONOTONIC_REPEATS) {
			XtermX.clear();
			termX.clear();
			Xterm.clear();
		}
		res.addAll(extractHieroRule(a, sp));
		return res;
	}

	/**
	 * This method extract phrase pairs from a Viterbi alignment and a sentence
	 * pair. Protected for testing.
	 * 
	 * @param a
	 * @param sp
	 * @return
	 */
	protected List<Rule> extractPhrasePairs(Alignment a, SentencePair sp) {

		List<Rule> res = new ArrayList<Rule>();

		// loop over source index (beginning of phrase)
		for (int sourceStartIndex = 0; sourceStartIndex < sp.getSource()
				.getWords().length; sourceStartIndex++) {
			// source phrase built on the fly
			List<Integer> sourcePhrase = new ArrayList<Integer>();
			// maintain the minimum and maximum target index aligned to the
			// source phrase
			int minTargetIndex = a.getMinTargetIndex(sourceStartIndex);
			int maxTargetIndex = a.getMaxTargetIndex(sourceStartIndex);
			// loop over source index (end of phrase)
			for (int sourceEndIndex = sourceStartIndex; sourceEndIndex < Math
					.min(sourceStartIndex + MAX_SOURCE_PHRASE, sp.getSource()
							.getWords().length); sourceEndIndex++) {
				// update the sourcePhrase
				sourcePhrase.add(sp.getSource().getWords()[sourceEndIndex]);
				// update minimum and maximum target index aligned to the source
				// phrase
				int minTargetIndexCandidate = a
						.getMinTargetIndex(sourceEndIndex);
				int maxTargetIndexCandidate = a
						.getMaxTargetIndex(sourceEndIndex);
				if (minTargetIndexCandidate < minTargetIndex)
					minTargetIndex = minTargetIndexCandidate;
				if (maxTargetIndexCandidate > maxTargetIndex)
					maxTargetIndex = maxTargetIndexCandidate;
				if (minTargetIndex > maxTargetIndex) // occurs when haven't
														// found any aligned
														// word
														// yet
					continue;

				// check if the target phrase between positions minTargetIndex
				// and maxTargetIndex and the source phrase are consistent with
				// the alignment
				boolean consistent = true;
				List<Integer> targetPhrase = new ArrayList<Integer>();
				for (int targetIndex = minTargetIndex; targetIndex <= maxTargetIndex; targetIndex++) {
					targetPhrase.add(sp.getTarget().getWords()[targetIndex]);
					if (a.isTargetAligned(targetIndex)
							&& (a.getMinSourceIndex(targetIndex) < sourceStartIndex || a
									.getMaxSourceIndex(targetIndex) > sourceEndIndex)) {
						consistent = false;
						break;
					}
				}

				// we found a phrase pair
				if (consistent) {
					Rule r = new Rule(sourceStartIndex, sourceEndIndex,
							minTargetIndex, maxTargetIndex, sp, a);
					res.add(r);
					List<Rule> extendedUnaligned = extendUnalignedBoundaryWord(
							sourceStartIndex, sourceEndIndex, minTargetIndex,
							maxTargetIndex, a, sp);
					res.addAll(extendedUnaligned);
				}
			}
		}
		return res;
	}

	/**
	 * This method extends an extracted phrase pair with unaligned boundary
	 * words on the target side
	 * 
	 * @param sourceStartIndex
	 * @param sourceEndIndex
	 * @param targetStartIndex
	 * @param targetEndIndex
	 * @param a
	 * @param sp
	 * @return
	 */
	private List<Rule> extendUnalignedBoundaryWord(int sourceStartIndex,
			int sourceEndIndex, int targetStartIndex, int targetEndIndex,
			Alignment a, SentencePair sp) {
		List<Rule> res = new ArrayList<Rule>();
		int prev = 0; // number of previous target unaligned words
		int targetExtendIndex = targetStartIndex - 1;
		// i>0 because i=0 is a position reserved for NULL. In this version, we
		// don't make
		// use of the alignment to NULL because we use symmetrized alignments
		// but that could
		// be changed
		while (targetExtendIndex >= 0 && !a.isTargetAligned(targetExtendIndex)) {
			prev++;
			Rule r = new Rule(sourceStartIndex, sourceEndIndex,
					targetExtendIndex, targetEndIndex, sp, a);
			res.add(r);
			targetExtendIndex--;
		}
		int foll = 0; // number of following target unaligned words
		targetExtendIndex = targetEndIndex + 1;
		while (targetExtendIndex < sp.getTarget().getWords().length
				&& !a.isTargetAligned(targetExtendIndex)) {
			foll++;
			Rule r = new Rule(sourceStartIndex, sourceEndIndex,
					targetStartIndex, targetExtendIndex, sp, a);
			res.add(r);
			targetExtendIndex++;
		}

		if (prev > 0 && foll > 0) { // if there are unaligned words in both
									// sides:
			for (targetExtendIndex = 1; targetExtendIndex <= prev; targetExtendIndex++) { // for
																							// each
																							// start
																							// (including
																							// at
																							// least
																							// one
																							// previous)
				int start = targetStartIndex - targetExtendIndex;
				for (int k = 1; k <= foll; k++) { // for each end (including at
													// least one following)
					int end = targetEndIndex + k;
					Rule r = new Rule(sourceStartIndex, sourceEndIndex, start,
							end, sp, a);
					res.add(r);
				}
			}
		}
		return res;
	}

	// TODO review this to make it work with arbitrary number of nonterminals
	// protected for testing
	protected List<Rule> extractHieroRule(Alignment a, SentencePair sp) {
		List<Rule> res = new ArrayList<Rule>();

		// first get the regular blocks, that is the boundaries for the phrase
		// pairs that would be extracted
		// these are maximum width blocks, they cannot be extended
		List<Block> regularBlocks = getRegularBlocks(a, sp);
		// for (Block b: regularBlocks) {
		// System.err.println(b);
		// }

		// something here about monotonic repetitions
		res.addAll(extractInternalBlockRules(regularBlocks, a, sp));

		// finally extract the rules "inter-blocks"
		res.addAll(extractInterBlockRules(regularBlocks, a, sp));
		return res;
	}

	// protected for testing
	protected List<Block> getRegularBlocks(Alignment a, SentencePair sp) {
		List<Block> res = new ArrayList<Block>();
		int sourceStartIndex = 0;
		int sourceEndIndex = 0;
		int targetStartIndex = 0;
		int targetEndIndex = 0;
		while (sourceStartIndex <= sp.getSource().getWords().length
				&& targetStartIndex <= sp.getTarget().getWords().length) {
			Block next = findNextBlock(sourceStartIndex, sourceEndIndex,
					targetStartIndex, targetEndIndex, a, sp);

			sourceStartIndex = next.sourceStartIndex;
			sourceEndIndex = next.sourceEndIndex;
			targetStartIndex = next.targetStartIndex;
			targetEndIndex = next.targetEndIndex;

			if (targetStartIndex >= sp.getTarget().getWords().length
					&& sourceStartIndex >= sp.getSource().getWords().length) {
				// do nothing
			} else if (targetStartIndex < sp.getTarget().getWords().length
					&& !a.isTargetAligned(targetStartIndex)) {
				sourceEndIndex--;
			} else if (sourceStartIndex < sp.getSource().getWords().length
					&& !a.isSourceAligned(sourceStartIndex)) {
				targetEndIndex--;
			}

			if (targetStartIndex >= sp.getTarget().getWords().length
					|| sourceStartIndex >= sp.getSource().getWords().length) {
				break;
			}
			if (sourceStartIndex <= sourceEndIndex
					&& targetStartIndex <= targetEndIndex) {
				res.add(new Block(sourceStartIndex, sourceEndIndex,
						targetStartIndex, targetEndIndex));
			}
			sourceStartIndex = sourceEndIndex + 1;
			targetStartIndex = targetEndIndex + 1;
			sourceEndIndex = sourceStartIndex;
			targetEndIndex = targetStartIndex;
		}
		return res;
	}

	private Block findNextBlock(int sourceStartIndex, int sourceEndIndex,
			int targetStartIndex, int targetEndIndex, Alignment a,
			SentencePair sp) {
		if (targetStartIndex >= sp.getTarget().getWords().length
				|| sourceStartIndex >= sp.getSource().getWords().length
				|| !a.isTargetAligned(targetStartIndex)
				|| !a.isSourceAligned(sourceStartIndex)) {
			return new Block(sourceStartIndex, sourceEndIndex,
					targetStartIndex, targetEndIndex);
		}
		boolean cont = true;
		while (cont) {
			cont = false;
			for (int sourceIndex = sourceStartIndex; sourceIndex <= sourceEndIndex; sourceIndex++) {
				if (a.getMaxTargetIndex(sourceIndex) > targetEndIndex) {
					targetEndIndex = a.getMaxTargetIndex(sourceIndex);
					cont = true;
				}
			}
			for (int targetIndex = targetStartIndex; targetIndex <= targetEndIndex; targetIndex++) {
				if (a.getMaxSourceIndex(targetIndex) > sourceEndIndex) {
					sourceEndIndex = a.getMaxSourceIndex(targetIndex);
					cont = true;
				}
			}
		}
		return new Block(sourceStartIndex, sourceEndIndex, targetStartIndex,
				targetEndIndex);
	}

	private List<Rule> extractInternalBlockRules(List<Block> regularBlocks,
			Alignment a, SentencePair sp) {
		List<Rule> res = new ArrayList<Rule>();
		for (Block b : regularBlocks) {
			res.addAll(extractInternalBlockRules(b, a, sp));
		}
		return res;
	}

	private Pair<Integer, Integer> updateTargetLimit(Alignment a,
			int sourceStartIndex, int sourceEndIndex) {
		Pair<Integer, Integer> res = new Pair<Integer, Integer>();
		int minTargetIndex = a.getMinTargetIndex(sourceStartIndex);
		int maxTargetIndex = a.getMaxTargetIndex(sourceStartIndex);
		for (int sourceIndex = sourceStartIndex; sourceIndex <= sourceEndIndex; sourceIndex++) {
			int minTargetIndexCandidate = a.getMinTargetIndex(sourceIndex);
			int maxTargetIndexCandidate = a.getMaxTargetIndex(sourceIndex);
			if (minTargetIndexCandidate < minTargetIndex)
				minTargetIndex = minTargetIndexCandidate;
			if (maxTargetIndexCandidate > maxTargetIndex)
				maxTargetIndex = maxTargetIndexCandidate;
		}
		res.setFirst(minTargetIndex);
		res.setSecond(maxTargetIndex);
		return res;
	}

	private List<Rule> extractInternalBlockRules(Block regularBlock,
			Alignment a, SentencePair sp) {
		List<Rule> res = new ArrayList<Rule>();
		for (int sourceStartIndex = regularBlock.sourceStartIndex; sourceStartIndex <= regularBlock.sourceEndIndex; sourceStartIndex++) {
			for (int sourceEndIndex = sourceStartIndex + 1; sourceEndIndex <= regularBlock.sourceEndIndex; sourceEndIndex++) {
				// check links and update target limit
				Pair<Integer, Integer> targetLimit = updateTargetLimit(a,
						sourceStartIndex, sourceEndIndex);
				int offset = 0;
				for (int targetIndex = targetLimit.getFirst(); targetIndex <= targetLimit
						.getSecond(); targetIndex++) {
					if (a.getMinSourceIndex(targetIndex) < sourceStartIndex) {
						offset = a.getMinSourceIndex(targetIndex)
								- sourceStartIndex;
						break;
					}
					if (a.getMaxSourceIndex(targetIndex) > sourceEndIndex) {
						offset = a.getMaxSourceIndex(targetIndex)
								- sourceEndIndex;
						break;
					}
				}
				if (offset < 0) {
					break; // if negative offset, jump to another
							// sourceStartIndex (end the sourceEndIndex for
							// loop)
				} else if (offset > 0) {
					sourceEndIndex = sourceEndIndex + offset - 1; // if
																	// positive,
																	// add
																	// offset
																	// to jump
																	// to
																	// the
																	// adequate
																	// sourceEndIndex
				} else { // zero offset, found a plausible subregion
							// System.err.println("Extracting rules one nonterminal "
							// +
							// sourceStartIndex + " " + sourceEndIndex + " " +
							// minTargetIndex + " " + maxTargetIndex);
					res.addAll(extractRulesOneNonTerminal(sourceStartIndex,
							sourceEndIndex, targetLimit.getFirst(),
							targetLimit.getSecond(), a, sp));
					res.addAll(extractRulesTwoNonTerminal(sourceStartIndex,
							sourceEndIndex, targetLimit.getFirst(),
							targetLimit.getSecond(), a, sp));
				}
			}
		}
		return res;
	}

	protected List<Rule> extractRulesOneNonTerminal(int sourceStartIndex,
			int sourceEndIndex, int minTargetIndex, int maxTargetIndex,
			Alignment a, SentencePair sp) {
		List<Rule> res = new ArrayList<Rule>();
		int offset = 0;
		for (int sourceStartIndexX = sourceStartIndex; sourceStartIndexX <= sourceEndIndex; sourceStartIndexX++) {
			for (int sourceEndIndexX = sourceStartIndexX; sourceEndIndexX <= sourceEndIndex; sourceEndIndexX++) {
				if (!a.isSourceAligned(sourceStartIndexX)
						|| !a.isSourceAligned(sourceEndIndexX)) {
					break;
				}
				Pair<Integer, Integer> targetLimit = updateTargetLimit(a,
						sourceStartIndexX, sourceEndIndexX);
				offset = 0;
				for (int targetIndex = targetLimit.getFirst(); targetIndex <= targetLimit
						.getSecond(); targetIndex++) {
					if (a.getMinSourceIndex(targetIndex) < sourceStartIndexX) {
						offset = a.getMinSourceIndex(targetIndex)
								- sourceStartIndexX;
						break;
					}
					if (a.getMaxSourceIndex(targetIndex) > sourceEndIndexX) {
						offset = a.getMaxSourceIndex(targetIndex)
								- sourceEndIndexX;
						break;
					}
				}
				if (targetLimit.getFirst() == minTargetIndex
						&& targetLimit.getSecond() == maxTargetIndex) {
					break;
				}
				if (offset < 0) {
					break;
				}
				if (offset > 0) {
					sourceEndIndexX = sourceEndIndexX + offset - 1;
				} else if (sourceStartIndexX != sourceStartIndex
						|| sourceEndIndexX != sourceEndIndex) {
					if (filterPassOneNonTerminalRule(sourceStartIndex,
							sourceEndIndex, sourceStartIndexX, sourceEndIndexX)) {
						Rule r = new Rule(sourceStartIndex, sourceEndIndex,
								minTargetIndex, maxTargetIndex,
								sourceStartIndexX, sourceEndIndexX,
								targetLimit.getFirst(),
								targetLimit.getSecond(), sp, a);
						res.add(r);
					}
				}
			}
		}
		return res;
	}

	protected List<Rule> extractRulesTwoNonTerminal(int sourceStartIndex,
			int sourceEndIndex, int minTargetIndex, int maxTargetIndex,
			Alignment a, SentencePair sp) {
		List<Rule> res = new ArrayList<Rule>();
		if (sourceEndIndex - sourceStartIndex < 2) { // we want at least 2
														// source words to
														// extract
														// a rule with two
														// nonterminals
			return res;
		}
		for (int sourceStartIndexX = sourceStartIndex; sourceStartIndexX < sourceEndIndex - 1; sourceStartIndexX++) {
			for (int sourceEndIndexX = sourceStartIndexX; sourceEndIndexX < sourceEndIndex - 1; sourceEndIndexX++) {
				if (!a.isSourceAligned(sourceStartIndexX)
						|| !a.isSourceAligned(sourceEndIndexX))
					break;
				Pair<Integer, Integer> targetLimit = updateTargetLimit(a,
						sourceStartIndexX, sourceEndIndexX);
				int offset = 0;
				for (int targetIndex = targetLimit.getFirst(); targetIndex <= targetLimit
						.getSecond(); targetIndex++) {
					if (a.getMinSourceIndex(targetIndex) < sourceStartIndexX) {
						offset = a.getMinSourceIndex(targetIndex)
								- sourceStartIndexX;
						break;
					}
					if (a.getMaxSourceIndex(targetIndex) > sourceEndIndexX) {
						offset = a.getMaxSourceIndex(targetIndex)
								- sourceEndIndexX;
						break;
					}
				}
				if (targetLimit.getFirst() == minTargetIndex
						&& targetLimit.getSecond() == maxTargetIndex) {
					break;
				}
				if (offset < 0) {
					break;
				} else if (offset > 0) {
					sourceEndIndexX = sourceEndIndexX + offset - 1;
				} else {
					for (int sourceStartIndexX2 = sourceEndIndexX + 2; sourceStartIndexX2 <= sourceEndIndex; sourceStartIndexX2++) {
						for (int sourceEndIndexX2 = sourceStartIndexX2; sourceEndIndexX2 <= sourceEndIndex; sourceEndIndexX2++) {
							if (!a.isSourceAligned(sourceStartIndexX2)
									|| !a.isSourceAligned(sourceEndIndexX2)) {
								break;
							}
							Pair<Integer, Integer> targetLimitX2 = updateTargetLimit(
									a, sourceStartIndexX2, sourceEndIndexX2);
							int offset2 = 0;
							for (int targetIndex = targetLimitX2.getFirst(); targetIndex <= targetLimitX2
									.getSecond(); targetIndex++) {
								if (a.getMinSourceIndex(targetIndex) < sourceStartIndexX2) {
									offset2 = a.getMinSourceIndex(targetIndex)
											- sourceStartIndexX2;
									break;
								}
								if (a.getMaxSourceIndex(targetIndex) > sourceEndIndexX2) {
									offset2 = a.getMaxSourceIndex(targetIndex)
											- sourceEndIndexX2;
									break;
								}
							}

							if (offset2 < 0)
								break;
							else if (offset2 > 0) {
								sourceEndIndexX2 = sourceEndIndexX2 + offset2
										- 1;
							} else {
								if (filterPassTwoNonTerminalRule(
										sourceStartIndex, sourceEndIndex,
										sourceStartIndexX, sourceEndIndexX,
										sourceStartIndexX2, sourceEndIndexX2, a)) {
									Rule r = new Rule(sourceStartIndex,
											sourceEndIndex, minTargetIndex,
											maxTargetIndex, sourceStartIndexX,
											sourceEndIndexX,
											targetLimit.getFirst(),
											targetLimit.getSecond(),
											sourceStartIndexX2,
											sourceEndIndexX2,
											targetLimitX2.getFirst(),
											targetLimitX2.getSecond(), sp,
											source2target, a);
									res.add(r);
								}
							}
						}
					}
				}
			}
		}
		return res;
	}

	/**
	 * @param regularBlocks
	 * @param a
	 * @param sp
	 * @return
	 */
	private List<Rule> extractInterBlockRules(List<Block> regularBlocks,
			Alignment a, SentencePair sp) {
		List<Rule> res = new ArrayList<Rule>();
		for (int blockIndex1 = 0; blockIndex1 < regularBlocks.size(); blockIndex1++) {
			for (int blockIndex2 = blockIndex1 + 1; blockIndex2 < regularBlocks
					.size(); blockIndex2++) {
				res.addAll(extractRulesOneNonTerminal(
						regularBlocks.get(blockIndex1).sourceStartIndex,
						regularBlocks.get(blockIndex2).sourceEndIndex,
						regularBlocks.get(blockIndex1).targetStartIndex,
						regularBlocks.get(blockIndex2).targetEndIndex, a, sp));
				res.addAll(extractRulesTwoNonTerminal(
						regularBlocks.get(blockIndex1).sourceStartIndex,
						regularBlocks.get(blockIndex2).sourceEndIndex,
						regularBlocks.get(blockIndex1).targetStartIndex,
						regularBlocks.get(blockIndex2).targetEndIndex, a, sp));
			}
		}
		return res;
	}

	private boolean filterPassOneNonTerminalRule(int sourceStartIndex,
			int sourceEndIndex, int sourceStartIndexX, int sourceEndIndexX) {
		boolean res = (((sourceEndIndex - sourceStartIndex + 1)
				- (sourceEndIndexX - sourceStartIndexX + 1) + 1) <= MAX_SOURCE_ELEMENTS
				&& sourceStartIndexX - sourceStartIndex <= MAX_TERMINAL_LENGTH
				&& sourceEndIndex - sourceEndIndexX <= MAX_TERMINAL_LENGTH && sourceEndIndexX
				- sourceStartIndexX + 1 <= MAX_NONTERMINAL_LENGTH);
		if (!res) {
			return false;
		}
		// TODO something about monotonic repetitions
		if (REMOVE_MONOTONIC_REPEATS
				&& isMonotonicRepeatOneNonterminal(sourceStartIndex,
						sourceEndIndex, sourceStartIndexX, sourceEndIndexX)) {
			return false;
		}
		return true;
	}

	/**
	 * @param sourceStartIndex
	 * @param sourceEndIndex
	 * @param sourceStartIndexX
	 * @param sourceEndIndexX
	 * @param sourceStartIndexX2
	 * @param sourceEndIndexX2
	 * @return
	 */
	private boolean filterPassTwoNonTerminalRule(int sourceStartIndex,
			int sourceEndIndex, int sourceStartIndexX, int sourceEndIndexX,
			int sourceStartIndexX2, int sourceEndIndexX2, Alignment a) {
		boolean middleTerminalAligned = false;
		for (int sourceIndex = sourceEndIndexX + 1; sourceIndex < sourceStartIndexX2; sourceIndex++) {
			if (a.isSourceAligned(sourceIndex)) {
				middleTerminalAligned = true;
				break;
			}
		}
		boolean res = (middleTerminalAligned
				&& ((sourceEndIndex - sourceStartIndex + 1)
						- (sourceEndIndexX - sourceStartIndexX + 1)
						- (sourceEndIndexX2 - sourceStartIndexX2 + 1) + 2) <= MAX_SOURCE_ELEMENTS
				&& sourceStartIndexX - sourceStartIndex <= MAX_TERMINAL_LENGTH
				&& sourceStartIndexX2 - sourceEndIndexX <= MAX_TERMINAL_LENGTH
				&& sourceEndIndex - sourceEndIndexX2 <= MAX_TERMINAL_LENGTH
				&& sourceEndIndexX - sourceStartIndexX + 1 <= MAX_NONTERMINAL_LENGTH && sourceEndIndexX2
				- sourceStartIndexX2 + 1 <= MAX_NONTERMINAL_LENGTH);
		if (!res) {
			return res;
		}
		if (REMOVE_MONOTONIC_REPEATS
				&& isMonotonicRepeatTwoNonterminal(sourceStartIndex,
						sourceEndIndex, sourceStartIndexX, sourceEndIndexX,
						sourceStartIndexX2, sourceEndIndexX2)) {
			return false;
		}
		return true;
	}

	private boolean isMonotonicRepeatOneNonterminal(int sourceStartIndex,
			int sourceEndIndex, int sourceStartIndexX, int sourceEndIndexX) {
		if (sourceStartIndexX == sourceStartIndex) {
			if (Xterm.containsKey(sourceEndIndexX)
					&& Xterm.get(sourceEndIndexX).containsKey(sourceEndIndex)) {
				return true;
			}
			if (!Xterm.containsKey(sourceEndIndexX)) {
				Map<Integer, Boolean> newElt = new HashMap<Integer, Boolean>();
				newElt.put(sourceEndIndex, true);
				Xterm.put(sourceEndIndexX, newElt);
			} else {
				Xterm.get(sourceEndIndexX).put(sourceEndIndex, true);
			}
		}
		if (sourceEndIndexX == sourceEndIndex) {
			if (termX.containsKey(sourceStartIndex)
					&& termX.get(sourceStartIndex).containsKey(
							sourceStartIndexX)) {
				return true;
			}
			if (!termX.containsKey(sourceStartIndex)) {
				Map<Integer, Boolean> newElt = new HashMap<Integer, Boolean>();
				newElt.put(sourceStartIndexX, true);
				termX.put(sourceStartIndex, newElt);
			} else {
				termX.get(sourceStartIndex).put(sourceStartIndexX, true);
			}
		}
		return false;
	}

	private boolean isMonotonicRepeatTwoNonterminal(int sourceStartIndex,
			int sourceEndIndex, int sourceStartIndexX, int sourceEndIndexX,
			int sourceStartIndexX2, int sourceEndIndexX2) {
		if (sourceStartIndexX == sourceStartIndex
				&& sourceEndIndexX2 == sourceEndIndex) {
			if (XtermX.containsKey(sourceEndIndexX)
					&& XtermX.get(sourceEndIndexX).containsKey(
							sourceStartIndexX2)) {
				return true;
			}
			if (!XtermX.containsKey(sourceEndIndexX)) {
				Map<Integer, Boolean> newElt = new HashMap<Integer, Boolean>();
				newElt.put(sourceStartIndexX2, true);
				XtermX.put(sourceEndIndexX, newElt);
			} else {
				XtermX.get(sourceEndIndexX).put(sourceStartIndexX2, true);
			}
		}
		return false;
	}
}
