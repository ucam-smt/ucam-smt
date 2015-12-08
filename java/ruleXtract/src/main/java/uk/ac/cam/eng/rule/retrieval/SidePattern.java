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

/**
 * This class represents a pattern for one side of a rule, e.g. wXw
 * 
 * @author Juan Pino
 * @date 28 May 2014
 */
public class SidePattern {

	private List<String> pattern;
	private int numberOfNT;

	public SidePattern(List<String> pattern) {
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

	public static SidePattern parsePattern(String patternString) {
		String[] parts = patternString.split("_");
		List<String> elements = new ArrayList<String>();
		for (String part : parts) {
			if (part.equals("V")) {
				elements.add("-1");
			} else if (part.equals("V1")) {
				elements.add("-2");
			} else if (part.equals("W")) {
				elements.add("w");
			} else {
				throw new RuntimeException("Malformed pattern: " + patternString);
			}
		}
		return new SidePattern(elements);
	}


	public boolean isPhrase() {
		return (pattern.size() == 1 && pattern.get(0).equals("w"));
	}

	public boolean hasMoreThan1NT() {
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
