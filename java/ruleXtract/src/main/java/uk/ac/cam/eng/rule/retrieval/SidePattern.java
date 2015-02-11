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
import java.util.List;

import uk.ac.cam.eng.extraction.datatypes.Rule;
import uk.ac.cam.eng.extraction.hadoop.datatypes.RuleWritable;

/**
 * This class represents a pattern for one side of a rule, e.g. wXw
 * 
 * @author Juan Pino
 * @date 28 May 2014
 */
class SidePattern {

	private List<String> pattern;
	private int numberOfNT;

	SidePattern(List<String> pattern) {
		this.pattern = pattern;
		numberOfNT = 0;
		for (String elt : pattern) {
			if (!elt.equals("w")) {
				numberOfNT++;
			}
		}
	}

	int size() {
		return pattern.size();
	}

	String get(int index) {
		return pattern.get(index);
	}

	static SidePattern parsePattern(String patternString) {
		String[] parts = patternString.split("_");
		List<String> elements = new ArrayList<String>();
		for (String part : parts) {
			if (part.equals("X")) {
				elements.add("-1");
			} else if (part.equals("X1")) {
				elements.add("-2");
			} else if (part.equals("X2")) {
				elements.add("-3");
			} else if (part.equals("W")) {
				elements.add("w");
			} else {
				System.err.println("Malformed pattern: " + patternString);
				System.exit(1);
			}
		}
		return new SidePattern(elements);
	}

	static SidePattern getPattern(String patternString) {
		String parts[] = patternString.split("_");
		List<String> pattern = new ArrayList<String>();
		boolean consecutiveTerminals = false;
		for (String part : parts) {
			if (part.equals("-1") || part.equals("-2") || part.equals("-3")) {
				pattern.add(part);
				consecutiveTerminals = false;
			} else {
				if (!consecutiveTerminals) {
					pattern.add("w");
				}
				consecutiveTerminals = true;
			}
		}
		return new SidePattern(pattern);
	}

	private static SidePattern getPattern(List<Integer> ruleSide) {
		List<String> pattern = new ArrayList<String>();
		boolean consecutiveTerminals = false;
		for (Integer elt : ruleSide) {
			if (elt < 0) {
				pattern.add(elt.toString());
				consecutiveTerminals = false;
			} else {
				if (!consecutiveTerminals) {
					pattern.add("w");
				}
				consecutiveTerminals = true;
			}
		}
		return new SidePattern(pattern);
	}

	static SidePattern getSourcePattern(RuleWritable rule) {
		return getPattern(rule.getSource().toString());
	}

	static SidePattern getTargetPattern(RuleWritable rule) {
		return getPattern(rule.getTarget().toString());
	}

	static SidePattern getSourcePattern(Rule rule) {
		return getPattern(rule.getSource());
	}

	static SidePattern getTargetPattern(Rule rule) {
		return getPattern(rule.getTarget());
	}

	public boolean isPhrase() {
		return (pattern.size() == 1 && pattern.get(0).equals("w"));
	}

	boolean hasMoreThan1NT() {
		return (numberOfNT > 1);
	}

	public int getFirstNT() {
		for (String elt : pattern) {
			if (!elt.equals("w")) {
				return Integer.parseInt(elt);
			}
		}
		return 0;
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
		result = prime * result + ((pattern == null) ? 0 : pattern.hashCode());
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
		SidePattern other = (SidePattern) obj;
		if (pattern == null) {
			if (other.pattern != null)
				return false;
		} else if (!pattern.equals(other.pattern))
			return false;
		return true;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Object#toString()
	 */
	@Override
	public String toString() {
		StringBuilder res = new StringBuilder();
		if (!pattern.isEmpty()) {
			res.append(pattern.get(0));
		}
		for (int i = 1; i < pattern.size(); i++) {
			res.append("_").append(pattern.get(i));
		}
		return res.toString();
	}
}
