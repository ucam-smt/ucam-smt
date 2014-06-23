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
package uk.ac.cam.eng.extraction.hadoop.features;

/**
 * 
 * @author Aurelien Waite
 * @date 28 May 2014
 */
public enum MapReduceFeature {

	SOURCE2TARGET_PROBABILITY(2), TARGET2SOURCE_PROBABILITY(2), SOURCE2TARGET_LEXICAL_PROBABILITY(
			1), TARGET2SOURCE_LEXICAL_PROBABILITY(1), SOURCE2TARGET_PATTERN_PROBABILITY(
			1), TARGET2SOURCE_PATTERN_PROBABILITY(1), UNALIGNED_WORDS(1), BINARY_PROVENANCE(
			1), SOURCE2TARGET_PROBABILITY_PRIOR(2), TARGET2SOURCE_PROBABILITY_PRIOR(
			2), PROVENANCE_SOURCE2TARGET_PROBABILITY(2, true), PROVENANCE_TARGET2SOURCE_PROBABILITY(
			2, true), PROVENANCE_SOURCE2TARGET_LEXICAL_PROBABILITY(1, true), PROVENANCE_TARGET2SOURCE_LEXICAL_PROBABILITY(
			1, true);

	private final int noOfFeatures;

	private final boolean provenanceFeature;

	private MapReduceFeature(int noOfFeatures, boolean provenanceFeature) {
		this.noOfFeatures = noOfFeatures;
		this.provenanceFeature = provenanceFeature;
	}

	private MapReduceFeature(int noOfFeatures) {
		this(noOfFeatures, false);
	}

	public int getNumberOfFeatures() {
		return noOfFeatures;
	}

	public boolean isProvenanceFeature() {
		return provenanceFeature;
	}

	public String getConfName() {
		return name().toLowerCase();
	}

	public static MapReduceFeature findFromConf(String name) {
		return valueOf(name.toUpperCase());
	}

}
