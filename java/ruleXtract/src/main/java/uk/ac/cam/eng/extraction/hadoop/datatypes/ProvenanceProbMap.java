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
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import org.apache.hadoop.io.DoubleWritable;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.WritableUtils;

/**
 * 
 * @author Aurelien Waite
 * @date 28 May 2014
 */
public class ProvenanceProbMap implements Writable,
		Map<IntWritable, DoubleWritable> {

	static final ProvenanceProbMap EMPTY = new ProvenanceProbMap() {
		public DoubleWritable put(IntWritable key, DoubleWritable value) {
			throw new UnsupportedOperationException();
		};
	};

	private Map<IntWritable, DoubleWritable> instance = new HashMap<>();

	public ProvenanceProbMap() {

	}

	public ProvenanceProbMap(ProvenanceProbMap value) {
		for (Entry<IntWritable, DoubleWritable> entry : value.entrySet()) {
			put(entry.getKey(), entry.getValue());
		}
	}

	public int size() {
		return instance.size();
	}

	public boolean isEmpty() {
		return instance.isEmpty();
	}

	public boolean containsKey(Object key) {
		return instance.containsKey(key);
	}

	public boolean containsValue(Object value) {
		return instance.containsValue(value);
	}

	public DoubleWritable get(Object key) {
		return instance.get(key);
	}

	public DoubleWritable put(IntWritable key, DoubleWritable value) {
		return instance.put(key, value);
	}

	/**
	 * Put method which tries to reduce object allocation
	 */
	public DoubleWritable put(int key, double value) {
		IntWritable keyObject = IntWritableCache.createIntWritable(key);
		if (instance.containsKey(keyObject) && instance.get(keyObject) != null) {
			instance.get(keyObject).set(value);
			return instance.get(keyObject);
		} else {
			return instance.put(keyObject, new DoubleWritable(value));
		}

	}

	public DoubleWritable remove(Object key) {
		return instance.remove(key);
	}

	public void putAll(Map<? extends IntWritable, ? extends DoubleWritable> m) {
		instance.putAll(m);
	}

	public void clear() {
		instance.clear();
	}

	public Set<IntWritable> keySet() {
		return instance.keySet();
	}

	public Collection<DoubleWritable> values() {
		return instance.values();
	}

	public Set<java.util.Map.Entry<IntWritable, DoubleWritable>> entrySet() {
		return instance.entrySet();
	}

	public boolean equals(Object o) {
		return instance.equals(o);
	}

	public int hashCode() {
		return instance.hashCode();
	}

	public String toString() {
		return instance.toString();
	}

	public void merge(ProvenanceProbMap other) {
		int expectedSize = size() + other.size();
		putAll(other);
		if (expectedSize != size()) {
			throw new RuntimeException("Two features with the same id: " + this
					+ " " + other);
		}
	}

	@Override
	public void write(DataOutput out) throws IOException {
		WritableUtils.writeVInt(out, instance.size());
		for (Entry<IntWritable, DoubleWritable> entry : instance.entrySet()) {
			WritableUtils.writeVInt(out, entry.getKey().get());
			entry.getValue().write(out);
		}
	}

	@Override
	public void readFields(DataInput in) throws IOException {
		instance.clear();
		int length = WritableUtils.readVInt(in);
		for (int i = 0; i < length; ++i) {
			int key = WritableUtils.readVInt(in);
			double value = in.readDouble();
			put(key, value);
		}

	}

}
