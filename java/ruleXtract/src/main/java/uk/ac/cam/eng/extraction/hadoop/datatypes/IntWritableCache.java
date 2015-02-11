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
import java.io.IOException;

import org.apache.hadoop.io.IntWritable;

/**
 * Store commonly used IntWritables. Based on Java's IntegerCache
 * 
 * @author Aurelien Waite
 * @date 28 May 2014
 */
public class IntWritableCache {
	private static final int low = -128;
	private static final int high = 127;
	private static final IntWritable cache[];

	private static class ImmutableIntWritable extends IntWritable {

		boolean set = false;

		ImmutableIntWritable(int i) {
			super(i);
		}

		@Override
		public void set(int value) {
			if (set) {
				throw new UnsupportedOperationException();
			}
			set = true;
			super.set(value);
		}

		@Override
		public void readFields(DataInput in) throws IOException {
			throw new UnsupportedOperationException();
		}
	}

	static {
		cache = new IntWritable[(high - low) + 1];
		int j = low;
		for (int k = 0; k < cache.length; k++)
			cache[k] = new ImmutableIntWritable(j++);
	}

	private IntWritableCache() {
	}

	public static IntWritable createIntWritable(int i) {
		if (i >= low && i <= high)
			return cache[i + (-low)];
		return new IntWritable(i);
	}
}