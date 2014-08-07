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
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * 
 * @author Juan Pino
 * @date 28 May 2014
 */
public final class Alignment {

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Object#toString()
	 */
	@Override
	public String toString() {
		return "Alignment [s2t=" + s2t + "]";
	}

	private static Logger logger = Logger
			.getLogger("uk.ac.cam.eng.extraction.alignment");

	/**
	 * set of links in a sentence pair Viterbi aligned arranged by source index
	 */
	private final List<List<Integer>> s2t;
	private final List<Integer> s2tmax;
	private final List<Integer> s2tmin;

	/**
	 * set of links in a sentence pair Viterbi aligned arranged by target index
	 */
	private final List<List<Integer>> t2s;
	private final List<Integer> t2smax;
	private final List<Integer> t2smin;

	/**
	 * This constructor reads an alignment in Berkeley format and fills the
	 * fields f2e and e2f
	 * 
	 * @param wordAlign alignment in Berkeley format
	 * @param side1source 1 or 2
	 */
	public Alignment(String wordAlign, SentencePair sp) {
		s2t = new ArrayList<List<Integer>>(sp.getSource().getWords().length);
		for (int i = 0; i < sp.getSource().getWords().length; i++) {
			s2t.add(null);
		}
		s2tmax = new ArrayList<Integer>(sp.getSource().getWords().length);
		for (int i = 0; i < sp.getSource().getWords().length; i++) {
			s2tmax.add(Integer.MIN_VALUE);
		}
		s2tmin = new ArrayList<Integer>(sp.getSource().getWords().length);
		for (int i = 0; i < sp.getSource().getWords().length; i++) {
			s2tmin.add(Integer.MAX_VALUE);
		}
		t2s = new ArrayList<List<Integer>>(sp.getTarget().getWords().length);
		for (int i = 0; i < sp.getTarget().getWords().length; i++) {
			t2s.add(null);
		}
		t2smax = new ArrayList<Integer>(sp.getTarget().getWords().length);
		for (int i = 0; i < sp.getTarget().getWords().length; i++) {
			t2smax.add(Integer.MIN_VALUE);
		}
		t2smin = new ArrayList<Integer>(sp.getTarget().getWords().length);
		for (int i = 0; i < sp.getTarget().getWords().length; i++) {
			t2smin.add(Integer.MAX_VALUE);
		}
		// handle empty alignment case
		String[] links = new String[0]; // empty array, not null though
		if (!wordAlign.isEmpty()) {
			links = wordAlign.split("\\s+");
		}
		for (String link : links) {
			String[] linkSrcposTrgpos = link.split("-");
			if (linkSrcposTrgpos.length != 2) {
				logger.log(Level.SEVERE, "Alignment link bad format: " + link
						+ "\n" + "word alignment: " + wordAlign + "\n"
						+ "sentencepair: " + sp);
				System.exit(1);
			}
			int sourcePosition = -1, targetPosition = -1;
			sourcePosition = Integer.parseInt(linkSrcposTrgpos[0]);
			targetPosition = Integer.parseInt(linkSrcposTrgpos[1]);
			if (s2t.get(sourcePosition) != null) {
				s2t.get(sourcePosition).add(targetPosition);
			} else {
				List<Integer> newEntry = new ArrayList<Integer>();
				newEntry.add(targetPosition);
				s2t.set(sourcePosition, newEntry);
			}
			if (targetPosition > s2tmax.get(sourcePosition)) {
				s2tmax.set(sourcePosition, targetPosition);
			}
			if (targetPosition < s2tmin.get(sourcePosition)) {
				s2tmin.set(sourcePosition, targetPosition);
			}
			if (t2s.get(targetPosition) != null) {
				t2s.get(targetPosition).add(sourcePosition);
			} else {
				List<Integer> newEntry = new ArrayList<Integer>();
				newEntry.add(sourcePosition);
				t2s.set(targetPosition, newEntry);
			}
			if (sourcePosition > t2smax.get(targetPosition)) {
				t2smax.set(targetPosition, sourcePosition);
			}
			if (sourcePosition < t2smin.get(targetPosition)) {
				t2smin.set(targetPosition, sourcePosition);
			}
		}
	}

	public List<List<Integer>> getS2t() {
		return s2t;
	}

	public int getMinTargetIndex(int sourceIndex) {
		return s2tmin.get(sourceIndex);
	}

	public int getMaxTargetIndex(int sourceIndex) {
		return s2tmax.get(sourceIndex);
	}

	public int getMinSourceIndex(int targetIndex) {
		return t2smin.get(targetIndex);
	}

	public int getMaxSourceIndex(int targetIndex) {
		return t2smax.get(targetIndex);
	}

	public boolean isSourceAligned(int sourceIndex) {
		return (s2t.get(sourceIndex) != null);
	}

	public boolean isTargetAligned(int targetIndex) {
		return (t2s.get(targetIndex) != null);
	}
}
