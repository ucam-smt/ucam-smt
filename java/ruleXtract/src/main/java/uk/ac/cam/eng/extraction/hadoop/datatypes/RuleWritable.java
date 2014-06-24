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

package uk.ac.cam.eng.extraction.hadoop.datatypes;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.util.LinkedList;
import java.util.List;

import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.io.WritableComparable;

import uk.ac.cam.eng.extraction.datatypes.Rule;
import uk.ac.cam.eng.rule.retrieval.RulePattern;

/**
 * This class represents a writable rule, it's essentially the same as a Rule
 * but is convenient to be used in the map-reduced framework
 * 
 * @author Juan Pino
 * @date 28 May 2014
 */
public class RuleWritable implements WritableComparable<RuleWritable> {

	protected Text leftHandSide;
	protected Text source;
	protected Text target;

	public Text getLeftHandSide() {
		return leftHandSide;
	}

	/**
	 * @param leftHandSide the leftHandSide to set
	 */
	public void setLeftHandSide(Text leftHandSide) {
		this.leftHandSide = leftHandSide;
	}

	/**
	 * @return the source
	 */
	public Text getSource() {
		return source;
	}

	/**
	 * @param source the source to set
	 */
	public void setSource(Text source) {
		this.source = source;
	}

	/**
	 * @return the target
	 */
	public Text getTarget() {
		return target;
	}

	/**
	 * @param target the target to set
	 */
	public void setTarget(Text target) {
		this.target = target;
	}

	public RuleWritable() {
		leftHandSide = new Text();
		source = new Text();
		target = new Text();
	}

	public RuleWritable(RuleWritable other) {
		leftHandSide = new Text(other.leftHandSide);
		source = new Text(other.source);
		target = new Text(other.target);
	}

	public RuleWritable(Rule r) {
		String[] parts = r.toString().split("\\s+");
		leftHandSide = new Text(parts[0]);
		source = new Text(parts[1]);
		if (parts.length == 3) {
			target = new Text(parts[2]);
		} else {
			target = new Text();
		}
	}

	/**
	 * Builds a RuleWritable from a source and a target. Needs to do a deep copy
	 * for use in the source-to-target and target-to-source reducers
	 * 
	 * @param source
	 * @param target
	 */
	public RuleWritable(RuleWritable source, RuleWritable target) {
		leftHandSide = new Text(source.leftHandSide);
		this.source = new Text(source.source);
		this.target = new Text(target.target);
	}

	public static RuleWritable makeSourceMarginal(Rule r) {
		String[] parts = r.toString().split("\\s+");
		RuleWritable res = new RuleWritable();
		res.leftHandSide = new Text(parts[0]);
		res.source = new Text(parts[1]);
		res.target = new Text();
		return res;
	}

	/**
	 * Keep only the source information. In case of a rule with two
	 * nonterminals, use the order X1...X2 in the source
	 * 
	 * @param r
	 */
	public void makeSourceMarginal(RuleWritable r) {
		makeSourceMarginal(r, true);
	}

	public void makeSourceMarginal(RuleWritable r, boolean source2target) {
		this.leftHandSide = r.leftHandSide;
		if (source2target) {
			this.source = r.source;
		} else {
			Rule rule = new Rule(r);
			if (rule.isSwapping()) {
				RuleWritable ruleInvertOnTheSource = new RuleWritable(
						rule.invertNonTerminals());
				this.source = ruleInvertOnTheSource.source;
			} else {
				this.source = r.source;
			}
		}
		this.target = new Text();
	}

	public static RuleWritable makeTargetMarginal(Rule r) {
		String[] parts = r.toString().split("\\s+");
		RuleWritable res = new RuleWritable();
		res.leftHandSide = new Text(parts[0]);
		res.target = new Text(parts[2]);
		res.source = new Text();
		return res;
	}

	public void makeTargetMarginal(RuleWritable r) {
		makeTargetMarginal(r, true);
	}

	public void makeTargetMarginal(RuleWritable r, boolean source2target) {
		this.leftHandSide = r.leftHandSide;
		this.source = new Text();
		if (source2target) {
			this.target = r.target;
		} else {
			Rule rule = new Rule(r);
			if (rule.isSwapping()) {
				RuleWritable ruleInvertOnTheSource = new RuleWritable(
						rule.invertNonTerminals());
				this.target = ruleInvertOnTheSource.target;
			} else {
				this.target = r.target;
			}
		}
	}

	/**
	 * Used for the pattern probability feature.
	 * 
	 * @return the pattern for this rule
	 */
	public RuleWritable getPattern() {
		// TODO i don't like it
		// create new object to avoid problems (e.g. with the source pattern
		// partitioner)
		RuleWritable res = new RuleWritable();
		if (isPattern()) {
			res.leftHandSide = new Text();
			res.source = new Text(this.source);
			res.target = new Text(this.target);
			return res;
		}
		Rule rule = new Rule(this);
		RulePattern rulePattern = RulePattern.getPattern(rule);
		String[] parts = rulePattern.toString().split("\\s+");
		if (parts.length != 2) {
			System.err.println("Rule pattern malformed: "
					+ rulePattern.toString());
			System.exit(1);
		}
		res.leftHandSide = new Text();
		res.source = new Text(parts[0]);
		res.target = new Text(parts[1]);
		return res;
	}

	/**
	 * Used for the pattern probability feature.
	 * 
	 * @return the source pattern for this rule
	 */
	public RuleWritable getSourcePattern() {
		RuleWritable res = getPattern();
		res.target.clear();
		return res;
	}

	/**
	 * Used for the pattern probability feature.
	 * 
	 * @return the target pattern for this rule
	 */
	public RuleWritable getTargetPattern() {
		RuleWritable res = getPattern();
		res.source.clear();
		return res;
	}

	/**
	 * Used for the pattern probability feature.
	 * 
	 * @return true if this rule is actually a pattern
	 */
	public boolean isPattern() {
		return leftHandSide.toString().isEmpty();
	}

	/**
	 * @return true if the target side is empty
	 */
	public boolean isTargetEmpty() {
		return target.toString().isEmpty();
	}

	/**
	 * @return true is the source side is empty
	 */
	public boolean isSourceEmpty() {
		return source.toString().isEmpty();
	}

	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append(leftHandSide);
		sb.append(" ");
		sb.append(source);
		sb.append(" ");
		sb.append(target);
		return sb.toString();
	}

	/**
	 * Prints a rule as found in shallow grammar (.lex.gz)
	 * 
	 * @return
	 */
	public String toStringShallow() {
		Rule r = new Rule(this);
		// glue rules
		if (r.isConcatenatingGlue()) {
			return "S S_X S_X";
		}
		if (r.isStartSentence()) {
			return "X 1 <s><s><s>";
		}
		if (r.isEndSentence()) {
			return "X 2 </s>";
		}
		if (r.isStartingGlue()) {
			return "X V V";
		}
		// deletion, oov, ascii rules
		if (r.isDeletion()) {
			return "X " + source.toString() + " <dr>";
		}
		if (r.isOov()) {
			return "X " + source.toString() + " <oov>";
		}
		if (r.isAscii()) {
			return "X " + source.toString() + " " + target.toString();
		}
		// TODO finish this
		return "";
	}

	/**
	 * @return The length of the source side of the rule
	 */
	public int getSourceLength() {
		String[] parts = source.toString().split("_");
		return parts.length;
	}

	/**
	 * @return The length of the target side of the rule
	 */
	public int getTargetLength() {
		String[] parts = target.toString().split("_");
		return parts.length;
	}

	public List<Cooccurrence> getCooccurrences() {
		Rule rule = new Rule(this);
		List<Integer> sourceWords = rule.getSourceWords();
		List<Integer> targetWords = rule.getTargetWords();
		List<Cooccurrence> results = new LinkedList<>();
		for (Integer source : sourceWords) {
			if (source > 0) {
				for (Integer target : targetWords) {
					if (target > 0) {
						Cooccurrence cooc = new Cooccurrence();
						cooc.Set(new IntWritable(source), new IntWritable(
								target));
						results.add(cooc);
					}
				}
			}
		}
		return results;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.apache.hadoop.io.Writable#readFields(java.io.DataInput)
	 */
	@Override
	public void readFields(DataInput arg0) throws IOException {
		leftHandSide.readFields(arg0);
		source.readFields(arg0);
		target.readFields(arg0);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.apache.hadoop.io.Writable#write(java.io.DataOutput)
	 */
	@Override
	public void write(DataOutput arg0) throws IOException {
		leftHandSide.write(arg0);
		source.write(arg0);
		target.write(arg0);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Comparable#compareTo(java.lang.Object)
	 */
	@Override
	public int compareTo(RuleWritable arg0) {
		int cmp = leftHandSide.compareTo(arg0.leftHandSide);
		if (cmp != 0)
			return cmp;
		cmp = source.compareTo(arg0.source);
		if (cmp != 0)
			return cmp;
		return target.compareTo(arg0.target);
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
		result = prime * result
				+ ((leftHandSide == null) ? 0 : leftHandSide.hashCode());
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
		if (this == obj) {
			return true;
		}
		if (obj == null) {
			return false;
		}
		if (getClass() != obj.getClass()) {
			return false;
		}
		RuleWritable other = (RuleWritable) obj;
		if (leftHandSide == null) {
			if (other.leftHandSide != null) {
				return false;
			}
		} else if (!leftHandSide.equals(other.leftHandSide)) {
			return false;
		}
		if (source == null) {
			if (other.source != null) {
				return false;
			}
		} else if (!source.equals(other.source)) {
			return false;
		}
		if (target == null) {
			if (other.target != null) {
				return false;
			}
		} else if (!target.equals(other.target)) {
			return false;
		}
		return true;
	}

}
