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
 * @author Juan Pino
 * @date 17 July 2014
 */
public class AlignmentAndFeatureMap implements Writable {

	public static final AlignmentAndFeatureMap EMPTY = new AlignmentAndFeatureMap(
			AlignmentCountMapWritable.EMPTY, FeatureMap.EMPTY);

	private AlignmentCountMapWritable alignment;
	private FeatureMap featureMap;

	public AlignmentAndFeatureMap() {
		alignment = new AlignmentCountMapWritable();
		featureMap = new FeatureMap();
	}

	public AlignmentAndFeatureMap(AlignmentCountMapWritable alignment,
			FeatureMap featureMap) {
		this.alignment = alignment;
		this.featureMap = featureMap;
	}

	public FeatureMap getFeatureMap() {
		return featureMap;
	}

	public AlignmentCountMapWritable getAlignment() {
		return alignment;
	}

	public void set(AlignmentCountMapWritable alignment, FeatureMap featureMap) {
		this.alignment = alignment;
		this.featureMap = featureMap;
	}

	public void setAlignment(AlignmentCountMapWritable alignment) {
		this.alignment = alignment;
	}

	public void setFeatureMap(FeatureMap featureMap) {
		this.featureMap = featureMap;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.apache.hadoop.io.Writable#write(java.io.DataOutput)
	 */
	@Override
	public void write(DataOutput out) throws IOException {
		alignment.write(out);
		featureMap.write(out);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.apache.hadoop.io.Writable#readFields(java.io.DataInput)
	 */
	@Override
	public void readFields(DataInput in) throws IOException {
		alignment.readFields(in);
		featureMap.readFields(in);
	}

	public String toString() {
		return alignment.toString() + "\t" + featureMap.toString();
	}
}
