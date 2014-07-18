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

import java.util.List;

import org.apache.hadoop.io.ArrayWritable;
import org.apache.hadoop.io.Writable;

import uk.ac.cam.eng.extraction.datatypes.AlignmentLink;

/**
 * Writable to represent an alignment. An alignment is simply an array of links,
 * so {@link AlignmentWritable} is just and extension of {@link ArrayWritable}
 * and the default constructor uses an {@link AlignmentLink}.
 * 
 * @author Juan Pino
 * @date 14 July 2014
 * 
 */
public class AlignmentWritable extends ArrayWritable {

	public AlignmentWritable() {
		super(AlignmentLink.class);
	}

	public AlignmentWritable(List<AlignmentLink> alignment) {
		super(AlignmentLink.class);
		AlignmentLink[] links = new AlignmentLink[alignment.size()];
		alignment.toArray(links);
		this.set(links);
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
		Writable[] theseValues = get();
		AlignmentWritable other = (AlignmentWritable) obj;
		Writable[] otherValues = other.get();
		if (theseValues == null && otherValues != null) {
			return false;
		}
		if (theseValues != null && otherValues == null) {
			return false;
		}
		if (theseValues == otherValues) {
			return true;
		}
		int length = theseValues.length;
		if (length != otherValues.length) {
			return false;
		}
		for (int i = 0; i < length; i++) {
			AlignmentLink thisElt = (AlignmentLink) theseValues[i];
			AlignmentLink otherElt = (AlignmentLink) otherValues[i];
			if (!(thisElt == null ? otherElt == null : thisElt.equals(otherElt))) {
				return false;
			}
		}
		return true;
	}

	@Override
	public int hashCode() {
		Writable[] theseValues = get();
		if (theseValues == null) {
			return 0;
		}
		int result = 1;
		for (Writable element : theseValues) {
			AlignmentLink castElement = (AlignmentLink) element;
			result = 31 * result
					+ (castElement == null ? 0 : castElement.hashCode());
		}
		return result;
	}

	public String toString() {
		String res = "";
		String sep = "";
		for (Writable elt : get()) {
			AlignmentLink link = (AlignmentLink) elt;
			res += sep + link.sourcePosition + "-" + link.targetPosition;
			sep = " ";
		}
		return res;
	}
}
