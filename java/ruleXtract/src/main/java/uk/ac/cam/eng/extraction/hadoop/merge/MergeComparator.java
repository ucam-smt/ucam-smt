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
package uk.ac.cam.eng.extraction.hadoop.merge;

import java.io.IOException;

import org.apache.hadoop.io.DataOutputBuffer;
import org.apache.hadoop.io.WritableComparable;
import org.apache.hadoop.io.WritableComparator;

import uk.ac.cam.eng.extraction.Rule;

/**
 * Need a byte level comparator for rule writable. Otherwise we are unable to
 * add it to the HFILE
 * 
 * @author Aurelien Waite
 * @date 28 May 2014
 */
public class MergeComparator extends WritableComparator {

	private final DataOutputBuffer buffera = new DataOutputBuffer();
	private final DataOutputBuffer bufferb = new DataOutputBuffer();

	public MergeComparator() {
		super(Rule.class);
	}

	@Override
	@SuppressWarnings("rawtypes")
	public int compare(WritableComparable a, WritableComparable b) {
		try {
			buffera.reset();
			a.write(buffera);
			bufferb.reset();
			b.write(bufferb);
			return compareBytes(buffera.getData(), 0, buffera.getLength(),
					bufferb.getData(), 0, bufferb.getLength());
		} catch (IOException e) {
			throw new RuntimeException(e);
		}
	}

	@Override
	public int compare(byte[] b1, int s1, int l1, byte[] b2, int s2, int l2) {
		return compareBytes(b1, s1, l1, b2, s2, l2);
	}

}
