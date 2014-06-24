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

import uk.ac.cam.eng.extraction.datatypes.Rule;
import uk.ac.cam.eng.extraction.hadoop.datatypes.RuleWritable;

/**
 * This class represents a pattern (e.g. wXw-wXw)
 * 
 * @author Juan Pino
 * @date 28 May 2014
 */
public class RulePattern {

	private SidePattern sourcePattern;
	private SidePattern targetPattern;

	public RulePattern(SidePattern sourcePattern, SidePattern targetPattern) {
		this.sourcePattern = sourcePattern;
		this.targetPattern = targetPattern;
	}

	public static RulePattern parsePattern(String patternString) {
		// X_W-W_X
		String[] sourceTarget = patternString.split("-");
		if (sourceTarget.length != 2) {
			System.err.println("Malformed pattern: " + patternString);
			System.exit(1);
		}
		return new RulePattern(SidePattern.parsePattern(sourceTarget[0]),
				SidePattern.parsePattern(sourceTarget[1]));
	}

	public static RulePattern parsePattern(String sourcePatternString,
			String targetPatternString) {
		return new RulePattern(SidePattern.parsePattern2(sourcePatternString),
				SidePattern.parsePattern2(targetPatternString));
	}

	public static RulePattern getPattern(RuleWritable source,
			RuleWritable target) {
		return new RulePattern(SidePattern.getSourcePattern(source),
				SidePattern.getTargetPattern(target));
	}

	public static RulePattern getPattern(Rule rule) {
		return new RulePattern(SidePattern.getSourcePattern(rule),
				SidePattern.getTargetPattern(rule));
	}

	public boolean isSwappingNT() {
		if (!sourcePattern.hasMoreThan1NT()) {
			return false;
		}
		return (sourcePattern.getFirstNT() != targetPattern.getFirstNT());
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
				+ ((sourcePattern == null) ? 0 : sourcePattern.hashCode());
		result = prime * result
				+ ((targetPattern == null) ? 0 : targetPattern.hashCode());
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
		RulePattern other = (RulePattern) obj;
		if (sourcePattern == null) {
			if (other.sourcePattern != null)
				return false;
		} else if (!sourcePattern.equals(other.sourcePattern))
			return false;
		if (targetPattern == null) {
			if (other.targetPattern != null)
				return false;
		} else if (!targetPattern.equals(other.targetPattern))
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
		return sourcePattern.toString() + " " + targetPattern.toString();
	}
}
