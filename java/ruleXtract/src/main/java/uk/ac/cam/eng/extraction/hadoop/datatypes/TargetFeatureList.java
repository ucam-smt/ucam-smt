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
import java.util.ArrayList;

import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.WritableUtils;

import uk.ac.cam.eng.extraction.RuleString;
import uk.ac.cam.eng.util.Pair;

/**
 * 
 * @author Aurelien Waite
 * @author Juan Pino
 * @date 28 May 2014
 */
public class TargetFeatureList extends
		ArrayList<Pair<RuleString, RuleData>> implements Writable {

	private static final long serialVersionUID = 1L;

	@Override
	public void write(DataOutput out) throws IOException {
		WritableUtils.writeVInt(out, size());
		for (Pair<RuleString, RuleData> entry : this) {
			entry.getFirst().write(out);
			entry.getSecond().write(out);
		}
	}

	@Override
	public void readFields(DataInput in) throws IOException {
		clear();
		int size = WritableUtils.readVInt(in);
		for (int i = 0; i < size; ++i) {
			RuleString target = new RuleString();
			target.readFields(in);
			RuleData alignmentAndFeatures = new RuleData();
			alignmentAndFeatures.readFields(in);
			add(Pair.createPair(target, alignmentAndFeatures));
		}
	}

}
