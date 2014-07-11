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
package uk.ac.cam.eng.rule.retrieval;

import java.io.IOException;
import java.util.Collections;
import java.util.Iterator;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.hbase.io.hfile.CacheConfig;
import org.apache.hadoop.hbase.io.hfile.HFile;
import org.apache.hadoop.hbase.io.hfile.HFileScanner;
import org.apache.hadoop.io.DataInputBuffer;
import org.apache.hadoop.io.DataOutputBuffer;
import org.apache.hadoop.io.Text;

import uk.ac.cam.eng.extraction.hadoop.datatypes.FeatureMap;
import uk.ac.cam.eng.extraction.hadoop.datatypes.RuleWritable;
import uk.ac.cam.eng.extraction.hadoop.datatypes.TargetFeatureList;
import uk.ac.cam.eng.rulebuilding.features.EnumRuleType;
import uk.ac.cam.eng.util.Pair;

/**
 * 
 * @author Aurelien Waite
 * @date 28 May 2014
 */
public class HFileRuleReader implements
		Iterable<Pair<RuleWritable, FeatureMap>> {

	HFileScanner scanner;

	private final DataInputBuffer in = new DataInputBuffer();
	private final DataOutputBuffer out = new DataOutputBuffer();
	private final RuleWritable rule = new RuleWritable();
	private final TargetFeatureList value = new TargetFeatureList();
	Text key = new Text();

	public HFileRuleReader(HFile.Reader hfReader) {
		scanner = hfReader.getScanner(false, false);
		rule.setLeftHandSide(EnumRuleType.EXTRACTED.getLhs());
		rule.setSource(key);
	}

	private void readValue() {
		in.reset(scanner.getValue().array(), scanner.getValue().arrayOffset(),
				scanner.getValue().limit());
		try {
			value.readFields(in);
		} catch (IOException e) {
			// Should not happen! Only reading buffered bytes
			throw new RuntimeException(e);
		}
	}

	public boolean seek(Text source) throws IOException {
		out.reset();
		source.write(out);
		int pos = scanner.seekTo(out.getData(), 0, out.getLength());
		if (pos == 0) {
			key.set(source);
			return true;
		} else {
			return false;
		}
	}

	public Iterable<Pair<RuleWritable, FeatureMap>> getRulesForSource() {
		readValue();
		final Iterator<Pair<Text, FeatureMap>> instance = value.iterator();

		return new Iterable<Pair<RuleWritable, FeatureMap>>() {

			@Override
			public Iterator<Pair<RuleWritable, FeatureMap>> iterator() {
				return new Iterator<Pair<RuleWritable, FeatureMap>>() {

					@Override
					public boolean hasNext() {
						return instance.hasNext();
					}

					@Override
					public Pair<RuleWritable, FeatureMap> next() {
						Pair<Text, FeatureMap> next = instance.next();
						rule.setTarget(next.getFirst());
						return Pair.createPair(rule, next.getSecond());
					}

					@Override
					public void remove() {
						throw new UnsupportedOperationException();

					}
				};
			}
		};
	}

	private Text readSource() {
		in.reset(scanner.getKey().array(), scanner.getKey().arrayOffset(),
				scanner.getKey().limit());
		try {
			key.readFields(in);
		} catch (IOException e) {
			// Should not happen! We are only reading buffered bytes.
			throw new RuntimeException(e);
		}
		return key;
	}

	@Override
	public Iterator<Pair<RuleWritable, FeatureMap>> iterator() {
		boolean temp = false;
		try {
			temp = scanner.seekTo();
		} catch (IOException e) {
			e.printStackTrace();
		}
		final boolean isNotEmpty = temp;
		if (!isNotEmpty) {
			return Collections.<Pair<RuleWritable, FeatureMap>> emptyList()
					.iterator();
		}
		readSource();
		return new Iterator<Pair<RuleWritable, FeatureMap>>() {

			Iterator<Pair<RuleWritable, FeatureMap>> targetIter;

			boolean hasNext = isNotEmpty;

			@Override
			public boolean hasNext() {
				return hasNext || targetIter.hasNext();
			}

			@Override
			public Pair<RuleWritable, FeatureMap> next() {
				if (targetIter == null) {
					targetIter = getRulesForSource().iterator();
					try {
						hasNext = scanner.next();
					} catch (IOException e) {
						e.printStackTrace();
						hasNext = false;
					}
				}
				if (targetIter.hasNext()) {
					return targetIter.next();
				} else if (hasNext) {
					readSource();
					targetIter = getRulesForSource().iterator();
					try {
						hasNext = scanner.next();
					} catch (IOException e) {
						e.printStackTrace();
						hasNext = false;
					}
					return targetIter.next();
				}
				return null;
			}

			@Override
			public void remove() {
				throw new UnsupportedOperationException();
			}

		};
	}

	public static void main(String[] args) throws IOException {
		Configuration conf = new Configuration();
		CacheConfig cacheConf = new CacheConfig(conf);
		int count = 0;
		for (String fileName : args) {
			int fileCount = 0;
			System.out.println("Reading file " + fileName);
			HFile.Reader hfReader = HFile.createReader(FileSystem.get(conf),
					new Path(fileName), cacheConf);
			HFileRuleReader ruleReader = new HFileRuleReader(hfReader);
			for (@SuppressWarnings("unused")
			Pair<RuleWritable, FeatureMap> entry : ruleReader) {
				++count;
				++fileCount;
			}
			System.out.println(fileCount + "\t" + fileName);
		}
		System.out.println(count + "\ttotal");
	}
}
