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

import org.apache.commons.collections.map.LRUMap;
import org.apache.hadoop.io.ByteWritable;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Writable;

/**
 * 
 * @author Aurelien Waite
 * @date 28 May 2014
 */
public class ProvenanceCountMap implements Writable,
		Map<ByteWritable, IntWritable> {

	// Probably should fix the integer cache to work in the same way
	// as the cache in the Integer class
	private static LRUMap lruIntCache = new LRUMap(100000);

	private static Map<Byte, ByteWritable> bytesCache = new HashMap<>();

	private Map<ByteWritable, IntWritable> instance = new HashMap<ByteWritable, IntWritable>();

	public ProvenanceCountMap() {

	}

	public ProvenanceCountMap(ProvenanceCountMap other) {
		instance.putAll(other.instance);
	}

	public Map<ByteWritable, IntWritable> getInstance() {
		return instance;
	}

	private static ByteWritable getCached(byte b) {
		if (bytesCache.containsKey(b)) {
			return bytesCache.get(b);
		}
		ByteWritable result = new ByteWritable(b);
		bytesCache.put(b, result);
		return result;
	}

	private static IntWritable getCached(int i) {
		if (lruIntCache.containsKey(i)) {
			return (IntWritable) lruIntCache.get(i);
		}
		IntWritable result = new IntWritable(i);
		lruIntCache.put(i, result);
		return result;
	}


	public void increment(ProvenanceCountMap newCounts) {
		for (Entry<ByteWritable, IntWritable> provCount : newCounts.entrySet()) {
			ByteWritable key = provCount.getKey();
			if (containsKey(key)) {
				put(key, getCached(get(key).get() + newCounts.get(key).get()));
			} else {
				put(key, newCounts.get(key));
			}
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

	public IntWritable get(Object key) {
		return instance.get(key);
	}

	public IntWritable put(ByteWritable key, IntWritable value) {
		return instance.put(key, value);
	}

	public IntWritable remove(Object key) {
		return instance.remove(key);
	}

	public void putAll(Map<? extends ByteWritable, ? extends IntWritable> m) {
		instance.putAll(m);
	}

	public void clear() {
		instance.clear();
	}

	public Set<ByteWritable> keySet() {
		return instance.keySet();
	}

	public Collection<IntWritable> values() {
		return instance.values();
	}

	public Set<java.util.Map.Entry<ByteWritable, IntWritable>> entrySet() {
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

	@Override
	public void write(DataOutput out) throws IOException {
		out.writeByte(instance.size());
		for (Entry<ByteWritable, IntWritable> entry : instance.entrySet()) {
			entry.getKey().write(out);
			entry.getValue().write(out);
		}

	}

	@Override
	public void readFields(DataInput in) throws IOException {
		instance.clear();
		byte length = in.readByte();
		for (int i = 0; i < length; ++i) {
			byte key = in.readByte();
			int value = in.readInt();
			instance.put(getCached(key), getCached(value));
		}

	}

}
