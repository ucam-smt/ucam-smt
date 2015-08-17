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

package uk.ac.cam.eng.extraction.hadoop.datatypes;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

import org.apache.hadoop.io.ByteWritable;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Writable;

import uk.ac.cam.eng.extraction.Alignment;


/**
 * Additional info about a rule that is used to build mapreduce features. The
 * ExtractedData is the output value in the extraction mapper and the input
 * key in a mapreduce feature mapper. We don't need to make this class implement
 * WritableComparable because it should never be used as an input key to a
 * reducer.
 * 
 * @author Juan Pino
 * @date 28 May 2014
 */
public class ExtractedData implements Writable {

	private ProvenanceCountMap provenance;
	private AlignmentCountMapWritable alignment;

	public ExtractedData() {
		provenance = new ProvenanceCountMap();
		alignment = new AlignmentCountMapWritable();
	}

	public ProvenanceCountMap getProvenanceCountMap() {
		return provenance;
	}

	public AlignmentCountMapWritable getAlignmentCountMapWritable() {
		return alignment;
	}

	public void clear() {
		provenance.clear();
		alignment.clear();
	}

	public void putProvenanceCount(
			ByteWritable provenanceName, IntWritable count) {
		provenance.put(provenanceName, count);
	}

	public void putAlignmentCount(Alignment align, int count) {
		alignment.put(align, count);
	}

	public void increment(ExtractedData other) {
		provenance.increment(other.provenance);
		alignment.increment(other.alignment);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.apache.hadoop.io.Writable#write(java.io.DataOutput)
	 */
	@Override
	public void write(DataOutput out) throws IOException {
		provenance.write(out);
		alignment.write(out);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.apache.hadoop.io.Writable#readFields(java.io.DataInput)
	 */
	@Override
	public void readFields(DataInput in) throws IOException {
		provenance.readFields(in);
		alignment.readFields(in);
	}

	public String toString() {
		return provenance.toString() + "\t" + alignment.toString();
	}
}
