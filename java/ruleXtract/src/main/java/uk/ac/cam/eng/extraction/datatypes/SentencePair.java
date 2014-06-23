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

/**
 * This class represents a sentence pair
 * 
 * @author Juan Pino
 * @date 28 May 2014
 */
public final class SentencePair { // final because immutable class

	public SentencePair(String source, String target) {
		this.source = new Sentence(source);
		this.target = new Sentence(target);
	}

	/**
	 * Source sentence
	 */
	private final Sentence source;

	/**
	 * Target sentence
	 */
	private final Sentence target;

	/**
	 * @return the source
	 */
	public Sentence getSource() {
		return source;
	}

	/**
	 * @return the target
	 */
	public Sentence getTarget() {
		return target;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Object#toString()
	 */
	@Override
	public String toString() {
		return "SentencePair [source=" + source + ", target=" + target + "]";
	}

}
