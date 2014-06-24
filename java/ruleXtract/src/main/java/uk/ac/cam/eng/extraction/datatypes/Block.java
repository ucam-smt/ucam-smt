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
 * This class represents a regular block, that is the widest phrase pair that
 * can be extracted from a Viterbi alignment in Hiero extraction
 * 
 * @author Juan Pino
 * @date 28 May 2014
 */
public final class Block { // final because immutable class

	public final int sourceStartIndex;
	public final int sourceEndIndex;
	public final int targetStartIndex;
	public final int targetEndIndex;

	/**
	 * @param sourceStartIndex
	 * @param sourceEndIndex
	 * @param targetStartIndex
	 * @param targetEndIndex
	 */
	public Block(int sourceStartIndex, int sourceEndIndex,
			int targetStartIndex, int targetEndIndex) {
		this.sourceStartIndex = sourceStartIndex;
		this.sourceEndIndex = sourceEndIndex;
		this.targetStartIndex = targetStartIndex;
		this.targetEndIndex = targetEndIndex;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Object#toString()
	 */
	@Override
	public String toString() {
		return "Block [sourceStartIndex=" + sourceStartIndex
				+ ", sourceEndIndex=" + sourceEndIndex + ", targetStartIndex="
				+ targetStartIndex + ", targetEndIndex=" + targetEndIndex + "]";
	}

}
