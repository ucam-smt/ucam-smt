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
import org.apache.hadoop.hbase.KeyValue;
import org.apache.hadoop.hbase.io.hfile.CacheConfig;
import org.apache.hadoop.hbase.io.hfile.HFile;
import org.apache.hadoop.hbase.io.hfile.HFileScanner;
import org.apache.hadoop.io.DataInputBuffer;
import org.apache.hadoop.io.DataOutputBuffer;

import scala.Array;
import uk.ac.cam.eng.extraction.Rule;
import uk.ac.cam.eng.extraction.RuleString;
import uk.ac.cam.eng.extraction.hadoop.datatypes.RuleData;
import uk.ac.cam.eng.extraction.hadoop.datatypes.TargetFeatureList;
import uk.ac.cam.eng.util.Pair;

/**
 * 
 * @author Aurelien Waite
 * @date 28 May 2014
 */
public class HFileRuleReader implements Iterable<Pair<Rule, RuleData>> {

	private HFileScanner scanner;

	private final DataInputBuffer in = new DataInputBuffer();
	private final DataOutputBuffer out = new DataOutputBuffer();
	private final Rule rule = new Rule();
	private final TargetFeatureList value = new TargetFeatureList();
	private RuleString key = new RuleString();

	public HFileRuleReader(HFile.Reader hfReader) {
		scanner = hfReader.getScanner(false, false);
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

	public boolean seek(RuleString source) throws IOException {
		out.reset();
		source.write(out);
		byte[] empty = Array.emptyByteArray();
		KeyValue kv = new KeyValue(out.getData(), 0, out.getLength(), empty, 0,
				0, empty, 0, 0, 0l, KeyValue.Type.Put, empty, 0, 0);
		int pos = scanner.seekTo(kv.getBuffer(), kv.getKeyOffset(),
				kv.getKeyLength());
		if (pos == 0) {
			key.set(source);
			rule.setSource(key);
			return true;
		} else {
			return false;
		}
	}

	public Iterable<Pair<Rule, RuleData>> getRulesForSource() {
		readValue();
		final Iterator<Pair<RuleString, RuleData>> instance = value.iterator();

		return new Iterable<Pair<Rule, RuleData>>() {

			@Override
			public Iterator<Pair<Rule, RuleData>> iterator() {
				return new Iterator<Pair<Rule, RuleData>>() {

					@Override
					public boolean hasNext() {
						return instance.hasNext();
					}

					@Override
					public Pair<Rule, RuleData> next() {
						Pair<RuleString, RuleData> next = instance.next();
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

	private RuleString readSource() {
		// Have to put the ROW_LENGTH_SIZE due to KeyValue structure
		in.reset(scanner.getKey().array(), scanner.getKey().arrayOffset()
				+ KeyValue.ROW_LENGTH_SIZE, scanner.getKey().limit());
		key.readFields(in);
		rule.setSource(key);
		return key;
	}

	@Override
	public Iterator<Pair<Rule, RuleData>> iterator() {
		boolean temp = false;
		try {
			temp = scanner.seekTo();
		} catch (IOException e) {
			throw new RuntimeException(e);
		}
		final boolean isNotEmpty = temp;
		if (!isNotEmpty) {
			return Collections.<Pair<Rule, RuleData>> emptyList().iterator();
		}
		readSource();
		return new Iterator<Pair<Rule, RuleData>>() {

			Iterator<Pair<Rule, RuleData>> targetIter;

			boolean hasNext = isNotEmpty;

			@Override
			public boolean hasNext() {
				return hasNext || targetIter.hasNext();
			}

			@Override
			public Pair<Rule, RuleData> next() {
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
			Pair<Rule, RuleData> entry : ruleReader) {
				++count;
				++fileCount;
			}
			System.out.println(fileCount + "\t" + fileName);
		}
		System.out.println(count + "\ttotal");
	}
}
