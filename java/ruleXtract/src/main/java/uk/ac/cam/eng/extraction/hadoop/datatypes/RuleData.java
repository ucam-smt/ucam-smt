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

package uk.ac.cam.eng.extraction.hadoop.datatypes;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

import org.apache.hadoop.io.Writable;


/**
 * @author Aurelien Waite
 * @date 7 November 2014
 */
public class RuleData implements Writable {

	private ProvenanceCountMap provCounts;
	
	private AlignmentCountMapWritable alignments;
	
	private FeatureMap features;
	
	/**
	 * This is mutable, do not mutate!
	 */
	public static final RuleData EMPTY = new RuleData(new ProvenanceCountMap(),
			AlignmentCountMapWritable.EMPTY, FeatureMap.EMPTY);

	public RuleData() {
		provCounts = new ProvenanceCountMap();
		alignments = new AlignmentCountMapWritable();
		features = new FeatureMap();
	}
	
	public RuleData(RuleData other){
		this.provCounts = new ProvenanceCountMap(other.provCounts);
		this.alignments = new AlignmentCountMapWritable(other.alignments);
		this.features = new FeatureMap(other.features);
	}

	public RuleData(ProvenanceCountMap provCounts, AlignmentCountMapWritable alignments,
			FeatureMap features) {
		this.provCounts = provCounts;
		this.alignments = alignments;
		this.features = features;
	}

	public void clear() {
		provCounts.clear();
		alignments.clear();
		features.clear();
	}

	public void merge(RuleData other) {
		provCounts.putAll(other.provCounts);
		features.merge(other.features);
		alignments.merge(other.alignments);
	}

	@Override
	public void readFields(DataInput in) throws IOException {
		provCounts.readFields(in);
		features.readFields(in);
		alignments.readFields(in);
	}

	@Override
	public void write(DataOutput out) throws IOException {
		provCounts.write(out);
		features.write(out);
		alignments.write(out);	
	}

	public ProvenanceCountMap getProvCounts() {
		return provCounts;
	}

	public AlignmentCountMapWritable getAlignments() {
		return alignments;
	}

	public FeatureMap getFeatures() {
		return features;
	}

	public void setProvCounts(ProvenanceCountMap provCounts) {
		this.provCounts = provCounts;
	}

	public void setAlignments(AlignmentCountMapWritable alignments) {
		this.alignments = alignments;
	}

	public void setFeatures(FeatureMap features) {
		this.features = features;
	}

	@Override
	public String toString() {
		return "RuleData [provCounts=" + provCounts + ", alignments="
				+ alignments + ", features=" + features + "]";
	}
	
	
}
