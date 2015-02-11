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

package uk.ac.cam.eng.extraction.datatypes;

import java.util.ArrayList;
import java.util.List;

import uk.ac.cam.eng.extraction.hadoop.datatypes.RuleWritable;

/**
 * This class describes a phrasal or hierarchical rule
 * 
 * @author Juan Pino
 * @date 28 May 2014
 */
public final class Rule { // final because immutable class

	// TODO when extending, find another way
	// TODO introduce boolean saying monotonic or not ?
	private final static int X = -1;
	private final static int X1 = -2;
	private final static int X2 = -3;
	private final static int S = -4;
	private final static int V = -5;

	/**
	 * Left hand side of the rule (X typically)
	 */
	private final int leftHandSide;

	/**
	 * Source side of the rule
	 */
	private final List<Integer> source;

	/**
	 * Target side of the rule
	 */
	private final List<Integer> target;

	private final List<AlignmentLink> alignment;

	public Rule(int lhs, List<Integer> src, List<Integer> trg) {
		this.leftHandSide = lhs;
		this.source = new ArrayList<Integer>(src);
		this.target = new ArrayList<Integer>(trg);
		this.alignment = null;
	}

	public Rule(List<Integer> src, List<Integer> trg) {
		this.leftHandSide = 0; // TODO modify this to make general
		this.source = new ArrayList<Integer>(src);
		this.target = new ArrayList<Integer>(trg);
		this.alignment = null;
	}

	private void buildAlignment(int sourceIndex, int sourceStartIndex,
			int targetStartIndex, int targetEndIndex, Alignment a) {
		if (!a.isSourceAligned(sourceIndex)) {
			return;
		}
		for (int trgIdx : a.getS2t().get(sourceIndex)) {
			if (targetStartIndex <= trgIdx && trgIdx <= targetEndIndex) {
				alignment.add(new AlignmentLink(sourceIndex - sourceStartIndex,
						trgIdx - targetStartIndex));
			}
		}
	}

	/**
	 * Constructor for a phrase based rule. The constructor uses the alignment
	 * information to compute the number of unaligned words in the source and in
	 * the target.
	 * TODO call in RuleExtractor to this constructor is probably duplicated
	 * logic and leads to duplicated loops
	 * 
	 * @param sourceStartIndex
	 * @param sourceEndIndex
	 * @param targetStartIndex
	 * @param targetEndIndex
	 * @param sp
	 * @param a
	 */
	public Rule(int sourceStartIndex, int sourceEndIndex, int targetStartIndex,
			int targetEndIndex, SentencePair sp, Alignment a) {
		this.leftHandSide = 0;
		source = new ArrayList<Integer>();
		alignment = new ArrayList<AlignmentLink>();
		for (int sourceIndex = sourceStartIndex; sourceIndex <= sourceEndIndex; sourceIndex++) {
			source.add(sp.getSource().getWords()[sourceIndex]);
			// compute alignment indices relative to the rule
			buildAlignment(sourceIndex, sourceStartIndex, targetStartIndex,
					targetEndIndex, a);
		}
		target = new ArrayList<Integer>();
		for (int targetIndex = targetStartIndex; targetIndex <= targetEndIndex; targetIndex++) {
			target.add(sp.getTarget().getWords()[targetIndex]);
		}
	}

	private void buildAlignment(int sourceIndex, int sourceStartIndex,
			int minTargetIndex, int maxTargetIndex, int minTargetIndexX,
			int maxTargetIndexX, int srcXSpanMinusOne, int trgXSpanMinusOne,
			boolean sourceIndexBeforeX, Alignment a) {
		if (!a.isSourceAligned(sourceIndex)) {
			return;
		}
		for (int trgIdx : a.getS2t().get(sourceIndex)) {
			if ((minTargetIndex <= trgIdx && trgIdx < minTargetIndexX)
					|| (maxTargetIndexX < trgIdx && trgIdx <= maxTargetIndex)) {
				int ruleSrcIdx = sourceIndexBeforeX ? sourceIndex
						- sourceStartIndex : sourceIndex - sourceStartIndex
						- srcXSpanMinusOne;
				int ruleTrgIdx = -1;
				if (trgIdx < minTargetIndexX) {
					ruleTrgIdx = trgIdx - minTargetIndex;
				} else if (trgIdx > maxTargetIndexX) {
					ruleTrgIdx = trgIdx - minTargetIndex - trgXSpanMinusOne;
				}
				alignment.add(new AlignmentLink(ruleSrcIdx, ruleTrgIdx));
			}
		}
	}

	/**
	 * Constructor for a hierarchical rule with one nonterminal. This
	 * constructor uses alignment information to compute the number of unaligned
	 * words in the source and in the target.
	 * 
	 * @param sourceStartIndex
	 * @param sourceEndIndex
	 * @param minTargetIndex
	 * @param maxTargetIndex
	 * @param sourceStartIndexX
	 * @param sourceEndIndexX
	 * @param minTargetIndexX
	 * @param maxTargetIndexX
	 * @param sp
	 */
	public Rule(int sourceStartIndex, int sourceEndIndex, int minTargetIndex,
			int maxTargetIndex, int sourceStartIndexX, int sourceEndIndexX,
			int minTargetIndexX, int maxTargetIndexX, SentencePair sp,
			Alignment a) {
		this.leftHandSide = 0;
		source = new ArrayList<Integer>();
		alignment = new ArrayList<AlignmentLink>();
		int srcXSpanMinusOne = sourceEndIndexX - sourceStartIndexX;
		int trgXSpanMinusOne = maxTargetIndexX - minTargetIndexX;
		for (int sourceIndex = sourceStartIndex; sourceIndex < sourceStartIndexX; sourceIndex++) {
			source.add(sp.getSource().getWords()[sourceIndex]);
			// compute alignment indices relative to the rule
			buildAlignment(sourceIndex, sourceStartIndex, minTargetIndex,
					maxTargetIndex, minTargetIndexX, maxTargetIndexX,
					srcXSpanMinusOne, trgXSpanMinusOne, true, a);
		}
		source.add(X);
		// add alignment between the nonterminals
		alignment.add(new AlignmentLink(sourceStartIndexX - sourceStartIndex,
				minTargetIndexX - minTargetIndex));
		for (int sourceIndex = sourceEndIndexX + 1; sourceIndex <= sourceEndIndex; sourceIndex++) {
			source.add(sp.getSource().getWords()[sourceIndex]);
			// compute alignment indices relative to the rule
			buildAlignment(sourceIndex, sourceStartIndex, minTargetIndex,
					maxTargetIndex, minTargetIndexX, maxTargetIndexX,
					srcXSpanMinusOne, trgXSpanMinusOne, false, a);
		}
		target = new ArrayList<Integer>();
		for (int targetIndex = minTargetIndex; targetIndex < minTargetIndexX; targetIndex++) {
			target.add(sp.getTarget().getWords()[targetIndex]);
		}
		target.add(X);
		for (int targetIndex = maxTargetIndexX + 1; targetIndex <= maxTargetIndex; targetIndex++) {
			target.add(sp.getTarget().getWords()[targetIndex]);
		}
	}

	private enum StartMiddleEnd {
		START, MIDDLE, END
	}

	private void buildAlignment(int sourceIndex, int sourceStartIndex,
			int minTargetIndex, int maxTargetIndex, int minTargetIndexFirstNT,
			int maxTargetIndexFirstNT, int minTargetIndexSecondNT,
			int maxTargetIndexSecondNT, int srcXSpanMinusOne,
			int srcX2SpanMinusOne, int firstTrgNTSpanMinusOne,
			int secondTrgNTSpanMinusOne, StartMiddleEnd sourcePosition,
			Alignment a) {
		if (!a.isSourceAligned(sourceIndex)) {
			return;
		}
		for (int trgIdx : a.getS2t().get(sourceIndex)) {
			if (trgIdx < minTargetIndex
					|| trgIdx > maxTargetIndex
					|| (minTargetIndexFirstNT <= trgIdx && trgIdx <= maxTargetIndexFirstNT)
					|| (minTargetIndexSecondNT <= trgIdx && trgIdx <= maxTargetIndexSecondNT)) {
				continue;
			}
			int ruleTrgIdx = -1;
			int ruleSrcIdx = -1;
			switch (sourcePosition) {
			case START:
				ruleSrcIdx = sourceIndex - sourceStartIndex;
				break;
			case MIDDLE:
				ruleSrcIdx = sourceIndex - sourceStartIndex - srcXSpanMinusOne;
				break;
			case END:
				ruleSrcIdx = sourceIndex - sourceStartIndex - srcXSpanMinusOne - srcX2SpanMinusOne;
				break;
			}
			if (trgIdx < minTargetIndexFirstNT) {
				ruleTrgIdx = trgIdx - minTargetIndex;
			} else if (maxTargetIndexFirstNT < trgIdx && trgIdx < minTargetIndexSecondNT) {
				ruleTrgIdx = trgIdx - minTargetIndex - firstTrgNTSpanMinusOne;
			} else if (trgIdx > maxTargetIndexSecondNT) {
				ruleTrgIdx = trgIdx - minTargetIndex - firstTrgNTSpanMinusOne
						- secondTrgNTSpanMinusOne;
			}
			alignment.add(new AlignmentLink(ruleSrcIdx, ruleTrgIdx));
		}
	}

	/**
	 * Constructor for a hierarchical rule with two nonterminals. This
	 * constructor uses alignment information to compute the number of unaligned
	 * words in the source and in the target.
	 * 
	 * @param sourceStartIndex
	 * @param sourceEndIndex
	 * @param minTargetIndex
	 * @param maxTargetIndex
	 * @param sourceStartIndexX
	 * @param sourceEndIndexX
	 * @param minTargetIndexX
	 * @param maxTargetIndexX
	 * @param sourceStartIndexX2
	 * @param sourceEndIndexX2
	 * @param minTargetIndexX2
	 * @param maxTargetIndexX2
	 * @param sp
	 * @param a
	 */
	public Rule(int sourceStartIndex, int sourceEndIndex, int minTargetIndex,
			int maxTargetIndex, int sourceStartIndexX, int sourceEndIndexX,
			int minTargetIndexX, int maxTargetIndexX, int sourceStartIndexX2,
			int sourceEndIndexX2, int minTargetIndexX2, int maxTargetIndexX2,
			SentencePair sp, Alignment a) {
		this.leftHandSide = 0;
		source = new ArrayList<Integer>();
		target = new ArrayList<Integer>();
		alignment = new ArrayList<AlignmentLink>();
		int srcXSpanMinusOne = sourceEndIndexX - sourceStartIndexX;
		int srcX2SpanMinusOne = sourceEndIndexX2 - sourceStartIndexX2;
		int firstTrgNTSpanMinusOne, secondTrgNTSpanMinusOne;

		if (minTargetIndexX2 > maxTargetIndexX) {

			firstTrgNTSpanMinusOne = maxTargetIndexX - minTargetIndexX;
			secondTrgNTSpanMinusOne = maxTargetIndexX2 - minTargetIndexX2;

			for (int sourceIndex = sourceStartIndex; sourceIndex < sourceStartIndexX; sourceIndex++) {
				source.add(sp.getSource().getWords()[sourceIndex]);
				// compute alignment indices relative to the rule
				buildAlignment(sourceIndex, sourceStartIndex, minTargetIndex,
						maxTargetIndex, minTargetIndexX, maxTargetIndexX,
						minTargetIndexX2, maxTargetIndexX2, srcXSpanMinusOne,
						srcX2SpanMinusOne, firstTrgNTSpanMinusOne,
						secondTrgNTSpanMinusOne, StartMiddleEnd.START, a);
			}
			source.add(X1);
			// add alignment between the nonterminals
			alignment.add(new AlignmentLink(sourceStartIndexX
					- sourceStartIndex, minTargetIndexX - minTargetIndex));
			for (int sourceIndex = sourceEndIndexX + 1; sourceIndex < sourceStartIndexX2; sourceIndex++) {
				source.add(sp.getSource().getWords()[sourceIndex]);
				// compute alignment indices relative to the rule
				buildAlignment(sourceIndex, sourceStartIndex, minTargetIndex,
						maxTargetIndex, minTargetIndexX, maxTargetIndexX,
						minTargetIndexX2, maxTargetIndexX2, srcXSpanMinusOne,
						srcX2SpanMinusOne, firstTrgNTSpanMinusOne,
						secondTrgNTSpanMinusOne, StartMiddleEnd.MIDDLE, a);
			}
			source.add(X2);
			// add alignment between the nonterminals
			alignment.add(new AlignmentLink(sourceStartIndexX2
					- sourceStartIndex - srcXSpanMinusOne, minTargetIndexX2
					- minTargetIndex - firstTrgNTSpanMinusOne));
			for (int sourceIndex = sourceEndIndexX2 + 1; sourceIndex <= sourceEndIndex; sourceIndex++) {
				source.add(sp.getSource().getWords()[sourceIndex]);
				// compute alignment indices relative to the rule
				buildAlignment(sourceIndex, sourceStartIndex, minTargetIndex,
						maxTargetIndex, minTargetIndexX, maxTargetIndexX,
						minTargetIndexX2, maxTargetIndexX2, srcXSpanMinusOne,
						srcX2SpanMinusOne, firstTrgNTSpanMinusOne,
						secondTrgNTSpanMinusOne, StartMiddleEnd.END, a);
			}
			for (int targetIndex = minTargetIndex; targetIndex < minTargetIndexX; targetIndex++) {
				target.add(sp.getTarget().getWords()[targetIndex]);
			}
			target.add(X1);
			for (int targetIndex = maxTargetIndexX + 1; targetIndex < minTargetIndexX2; targetIndex++) {
				target.add(sp.getTarget().getWords()[targetIndex]);
			}
			target.add(X2);
			for (int targetIndex = maxTargetIndexX2 + 1; targetIndex <= maxTargetIndex; targetIndex++) {
				target.add(sp.getTarget().getWords()[targetIndex]);
			}
		}
		// when there is a non terminal swap, we prefer having the nonterminals
		// ordered in the source
		// (X1 ... X2) and swaped in the target (X2 ... X1) because otherwise,
		// when computing
		// source-to-target probability, the denominator would include two kinds
		// of source, the sources
		// with non terminal in order and the sources with nonterminal swaped.
		else {

			firstTrgNTSpanMinusOne = maxTargetIndexX2 - minTargetIndexX2;
			secondTrgNTSpanMinusOne = maxTargetIndexX - minTargetIndexX;

			for (int sourceIndex = sourceStartIndex; sourceIndex < sourceStartIndexX; sourceIndex++) {
				source.add(sp.getSource().getWords()[sourceIndex]);
				// compute alignment indices relative to the rule
				buildAlignment(sourceIndex, sourceStartIndex, minTargetIndex,
						maxTargetIndex, minTargetIndexX2, maxTargetIndexX2,
						minTargetIndexX, maxTargetIndexX, srcXSpanMinusOne,
						srcX2SpanMinusOne, firstTrgNTSpanMinusOne,
						secondTrgNTSpanMinusOne, StartMiddleEnd.START, a);
			}
			source.add(X1);
			// add alignment between the nonterminals
			alignment.add(new AlignmentLink(sourceStartIndexX
					- sourceStartIndex, minTargetIndexX - minTargetIndex
					- firstTrgNTSpanMinusOne));
			for (int sourceIndex = sourceEndIndexX + 1; sourceIndex < sourceStartIndexX2; sourceIndex++) {
				source.add(sp.getSource().getWords()[sourceIndex]);
				// compute alignment indices relative to the rule
				buildAlignment(sourceIndex, sourceStartIndex, minTargetIndex,
						maxTargetIndex, minTargetIndexX2, maxTargetIndexX2,
						minTargetIndexX, maxTargetIndexX, srcXSpanMinusOne,
						srcX2SpanMinusOne, firstTrgNTSpanMinusOne,
						secondTrgNTSpanMinusOne, StartMiddleEnd.MIDDLE, a);
			}
			source.add(X2);
			// add alignment between the nonterminals
			alignment.add(new AlignmentLink(sourceStartIndexX2
					- sourceStartIndex - srcXSpanMinusOne, minTargetIndexX2
					- minTargetIndex));
			for (int sourceIndex = sourceEndIndexX2 + 1; sourceIndex <= sourceEndIndex; sourceIndex++) {
				source.add(sp.getSource().getWords()[sourceIndex]);
				// compute alignment indices relative to the rule
				buildAlignment(sourceIndex, sourceStartIndex, minTargetIndex,
						maxTargetIndex, minTargetIndexX2, maxTargetIndexX2,
						minTargetIndexX, maxTargetIndexX, srcXSpanMinusOne,
						srcX2SpanMinusOne, firstTrgNTSpanMinusOne,
						secondTrgNTSpanMinusOne, StartMiddleEnd.END, a);
			}
			for (int targetIndex = minTargetIndex; targetIndex < minTargetIndexX2; targetIndex++) {
				target.add(sp.getTarget().getWords()[targetIndex]);
			}
			target.add(X2);
			for (int targetIndex = maxTargetIndexX2 + 1; targetIndex < minTargetIndexX; targetIndex++) {
				target.add(sp.getTarget().getWords()[targetIndex]);
			}
			target.add(X1);
			for (int targetIndex = maxTargetIndexX + 1; targetIndex <= maxTargetIndex; targetIndex++) {
				target.add(sp.getTarget().getWords()[targetIndex]);
			}
		}
	}

	public Rule(RuleWritable rw) {
		this.leftHandSide = Integer.parseInt(rw.getLeftHandSide().toString());
		this.source = new ArrayList<Integer>();
		String[] rwSource = rw.getSource().toString().split("_");
		for (String rws : rwSource) {
			this.source.add(Integer.parseInt(rws));
		}
		this.target = new ArrayList<Integer>();
		String[] rwTarget = rw.getTarget().toString().split("_");
		for (String rwt : rwTarget) {
			if (!rwt.isEmpty()) { // check in case the target was empty
				this.target.add(Integer.parseInt(rwt));
			}
		}
		this.alignment = null;
	}

	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append(leftHandSide);
		sb.append(" ");
		for (int i = 0; i < source.size(); i++) {
			sb.append(source.get(i) + "_");
		}
		sb.deleteCharAt(sb.length() - 1);
		sb.append(" ");
		for (int i = 0; i < target.size(); i++) {
			sb.append(target.get(i) + "_");
		}
		sb.deleteCharAt(sb.length() - 1);
		return sb.toString();
	}

	/**
	 * This method counts the number of target words. It is used for the word
	 * insertion penalty feature.
	 * 
	 * @return The number of target words.
	 */
	public int nbTargetWords() {
		// TODO have nbNonTerminal properly set
		// return target.size() - nbNonTerminal;
		int res = 0;
		for (int targetElement : target) {
			if (targetElement > 0) {
				res++;
			}
		}
		return res;
	}

	/**
	 * Decides if the rule is a concatenating glue rule (S-->SX,SX) or not
	 * 
	 * @return True if the rule is S-->SX,SX
	 */
	public boolean isConcatenatingGlue() {
		if (source.size() == 2 && source.get(0) == S && source.get(1) == X
				&& target.size() == 2 && target.get(0) == S
				&& target.get(1) == X)
			return true;
		return false;
	}

	/**
	 * @return
	 */
	public boolean isStartingGlue() {
		if (source.size() == 1 && source.get(0) == X && target.size() == 1
				&& target.get(0) == X)
			return true;
		return false;
	}

	public boolean isStartSentence() {
		return (source.size() == 1 && source.get(0) == 1 && target.size() == 1 && target
				.get(0) == 1);
	}

	public boolean isEndSentence() {
		return (source.size() == 1 && source.get(0) == 2 && target.size() == 1 && target
				.get(0) == 2);
	}

	public boolean isDeletion() {
		return (target.size() == 1 && target.get(0) == 0);
	}

	public boolean isOov() {
		return (leftHandSide == -1 && source.size() == 1 && target.size() == 0);
	}

	public boolean isAscii() {
		return (leftHandSide == -1 && source.size() == 1 && target.size() == 1);
	}

	public boolean isSwapping() {
		// check if the rule is swapping because we have X2...X1 in the source
		for (int sourceElement : source) {
			if (sourceElement < 0) {
				if (sourceElement == X2) {
					return true;
				}
				break;
			}
		}
		// check if the rule is swapping because we have X2...X1 in the target
		for (int targetElement : target) {
			if (targetElement < 0) {
				if (targetElement == X2) {
					return true;
				}
				break;
			}
		}
		return false;
	}

	public Rule invertNonTerminals() {
		List<Integer> src = new ArrayList<Integer>();
		List<Integer> trg = new ArrayList<Integer>();
		for (int sourceElement : source) {
			if (sourceElement == X2) {
				src.add(X1);
			} else if (sourceElement == X1) {
				src.add(X2);
			} else {
				src.add(sourceElement);
			}
		}
		for (int targetElement : target) {
			if (targetElement == X2) {
				trg.add(X1);
			} else if (targetElement == X1) {
				trg.add(X2);
			} else {
				trg.add(targetElement);
			}
		}
		return new Rule(src, trg);
	}

	public int getLeftHandSide() {
		return leftHandSide;
	}

	public List<Integer> getSource() {
		return source;
	}

	public List<Integer> getTarget() {
		return target;
	}

	public List<AlignmentLink> getAlignment() {
		return alignment;
	}

	public List<Integer> getSourceWords() {
		List<Integer> res = new ArrayList<Integer>();
		for (int sourceElement : source) {
			// if positive then it is a terminal
			if (sourceElement > 0) {
				res.add(sourceElement);
			}
		}
		return res;
	}

	public List<Integer> getTargetWords() {
		List<Integer> res = new ArrayList<Integer>();
		for (int targetElement : target) {
			// if nonnegative then it is a terminal
			// if zero, then it is a deletion rule
			if (targetElement >= 0) {
				res.add(targetElement);
			}
		}
		return res;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Object#hashCode()
	 */
	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + leftHandSide;
		result = prime * result + ((source == null) ? 0 : source.hashCode());
		result = prime * result + ((target == null) ? 0 : target.hashCode());
		return result;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Object#equals(java.lang.Object)
	 */
	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		Rule other = (Rule) obj;
		if (leftHandSide != other.leftHandSide)
			return false;
		if (source == null) {
			if (other.source != null)
				return false;
		} else if (!source.equals(other.source))
			return false;
		if (target == null) {
			if (other.target != null)
				return false;
		} else if (!target.equals(other.target))
			return false;
		return true;
	}
}
