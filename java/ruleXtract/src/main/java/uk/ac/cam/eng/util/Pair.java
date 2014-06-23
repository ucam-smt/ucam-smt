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

package uk.ac.cam.eng.util;

/**
 * 
 * @author Juan Pino
 * @date 28 May 2014
 * @param <T>
 * @param <U>
 */
public class Pair<T, U> {

	private T first;
	private U second;

	public Pair() {
	}

	/**
	 * @param first
	 * @param second
	 */
	public Pair(T first, U second) {
		this.first = first;
		this.second = second;
	}

	public static <F, S> Pair<F, S> createPair(F first, S second) {
		return new Pair<F, S>(first, second);
	}

	/**
	 * @return the first
	 */
	public T getFirst() {
		return first;
	}

	/**
	 * @param first the first to set
	 */
	public void setFirst(T first) {
		this.first = first;
	}

	/**
	 * @return the second
	 */
	public U getSecond() {
		return second;
	}

	/**
	 * @param second the second to set
	 */
	public void setSecond(U second) {
		this.second = second;
	}

	@Override
	public String toString() {
		return first + "\t" + second;
	}

}
