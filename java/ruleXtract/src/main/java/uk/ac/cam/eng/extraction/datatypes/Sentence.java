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

import java.util.Arrays;

/**
 * This class represents a sentence
 * 
 * @author Juan Pino
 * @date 28 May 2014
 */
public final class Sentence { // final because immutable class

	private final int[] words;

	/**
	 * @return the words
	 */
	public int[] getWords() {
		return words;
	}

	public Sentence(String input) {
		String[] parts = input.split("\\s+");
		words = new int[parts.length];
		for (int i = 0; i < parts.length; i++) {
			words[i] = Integer.parseInt(parts[i]);
		}
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Object#toString()
	 */
	@Override
	public String toString() {
		return "Sentence [words=" + Arrays.toString(words) + "]";
	}

}
