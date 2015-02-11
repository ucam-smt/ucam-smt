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
import java.util.HashMap;
import java.util.Map.Entry;

import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.file.tfile.Utils;

/**
 * @author Juan Pino
 * @date 14 July 2014
 */
class AlignmentCountMapWritable extends
		HashMap<AlignmentWritable, Integer> implements Writable {

	public static final AlignmentCountMapWritable EMPTY = new AlignmentCountMapWritable() {
		public Integer put(AlignmentWritable key, Integer value) {
			throw new UnsupportedOperationException();
		};
	};

	public AlignmentCountMapWritable() {

	}

	public void increment(AlignmentCountMapWritable newCounts) {
		for (Entry<AlignmentWritable, Integer> alignCount : newCounts
				.entrySet()) {
			AlignmentWritable key = alignCount.getKey();
			if (containsKey(key)) {
				put(key, get(key) + newCounts.get(key));
			} else {
				put(key, newCounts.get(key));
			}
		}
	}

	public void merge(AlignmentCountMapWritable other) {
		int expectedSize = size() + other.size();
		putAll(other);
		if (expectedSize != size()) {
			throw new RuntimeException("Merging twice the same alignment "
					+ this + other);
		}
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.apache.hadoop.io.Writable#write(java.io.DataOutput)
	 */
	@Override
	public void write(DataOutput out) throws IOException {
		Utils.writeVInt(out, size());
		for (AlignmentWritable a : keySet()) {
			a.write(out);
			Utils.writeVInt(out, get(a));
		}
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.apache.hadoop.io.Writable#readFields(java.io.DataInput)
	 */
	@Override
	public void readFields(DataInput in) throws IOException {
		clear();
		int size = Utils.readVInt(in);
		for (int i = 0; i < size; ++i) {
			AlignmentWritable a = new AlignmentWritable();
			a.readFields(in);
			int alignmentCount = Utils.readVInt(in);
			put(a, alignmentCount);
		}
	}

}
