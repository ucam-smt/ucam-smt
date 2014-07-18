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

package uk.ac.cam.eng.extraction.datatypes;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.file.tfile.Utils;

/**
 * @author Juan Pino
 * @date 15 July 2014
 */
public class AlignmentLink implements Writable {

	public AlignmentLink() {
		sourcePosition = -1;
		targetPosition = -1;
	}

	public AlignmentLink(int srcPos, int trgPos) {
		sourcePosition = srcPos;
		targetPosition = trgPos;
	}

	public int sourcePosition;
	public int targetPosition;

	@Override
	public void write(DataOutput out) throws IOException {
		Utils.writeVInt(out, sourcePosition);
		Utils.writeVInt(out, targetPosition);
	}

	@Override
	public void readFields(DataInput in) throws IOException {
		sourcePosition = Utils.readVInt(in);
		targetPosition = Utils.readVInt(in);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + sourcePosition;
		result = prime * result + targetPosition;
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj) {
			return true;
		}
		if (obj == null) {
			return false;
		}
		if (getClass() != obj.getClass()) {
			return false;
		}
		AlignmentLink other = (AlignmentLink) obj;
		if (sourcePosition != other.sourcePosition) {
			return false;
		}
		if (targetPosition != other.targetPosition) {
			return false;
		}
		return true;
	}
}
