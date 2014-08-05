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
package uk.ac.cam.eng.extraction.hadoop.features.lexical;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.Closeable;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.hadoop.conf.Configuration;

import uk.ac.cam.eng.extraction.datatypes.Rule;
import uk.ac.cam.eng.extraction.hadoop.datatypes.AlignmentAndFeatureMap;
import uk.ac.cam.eng.extraction.hadoop.datatypes.ProvenanceCountMap;
import uk.ac.cam.eng.extraction.hadoop.datatypes.RuleWritable;
import uk.ac.cam.eng.extraction.hadoop.util.Util;
import uk.ac.cam.eng.rulebuilding.features.FeatureCreator;
import uk.ac.cam.eng.util.Pair;

/**
 * 
 * @author Aurelien Waite
 * @date 28 May 2014
 */
public class TTableClient implements Closeable {

	public static final String S2T_FEATURE_NAME = "source2target_lexical_probability";

	public static final String T2S_FEATURE_NAME = "target2source_lexical_probability";

	private static final String S2T_HOST_NAME = "ttable.s2t.host";

	private static final String T2S_HOST_NAME = "ttable.t2s.host";

	private String hostName;

	private int port;

	private Socket clientSocket;

	private LexicalProbability prob;

	private DataInputStream in;

	private DataOutputStream out;

	Map<List<Integer>, Double> wordAlignments = new HashMap<>();

	int[] mapping;

	public void setup(Configuration conf, boolean source2Target)
			throws UnknownHostException {
		String featureName;
		if (source2Target) {
			port = conf.getInt(TTableServer.TTABLE_S2T_SERVER_PORT, -1);
			hostName = conf.get(S2T_HOST_NAME);
			featureName = TTableClient.S2T_FEATURE_NAME;
		} else {
			port = conf.getInt(TTableServer.TTABLE_T2S_SERVER_PORT, -1);
			hostName = conf.get(T2S_HOST_NAME);
			featureName = TTableClient.T2S_FEATURE_NAME;
		}
		if (port == -1) {
			throw new UnknownHostException("TTable server port incorrect!");
		}
		mapping = ProvenanceCountMap.getFeatureIndex(featureName,
				FeatureCreator.MAPRED_SUFFIX, conf);
		prob = new LexicalProbability(source2Target);
	}

	public double[] query(Collection<List<Integer>> query) throws IOException {

		clientSocket = new Socket(hostName, port);
		in = new DataInputStream(new BufferedInputStream(
				clientSocket.getInputStream(), TTableServer.BUFFER_SIZE));
		out = new DataOutputStream(new BufferedOutputStream(
				clientSocket.getOutputStream(), TTableServer.BUFFER_SIZE));
		double[] result = new double[query.size()];
		out.writeInt(query.size());
		for (List<Integer> queryKey : query) {
			out.writeInt(queryKey.get(0));
			out.writeInt(queryKey.get(1));
			out.writeInt(queryKey.get(2));
		}
		out.flush();

		for (int i = 0; i < query.size(); ++i) {
			double val = in.readDouble();
			result[i] = val;
		}
		clientSocket.close();
		in.close();
		out.close();
		return result;

	}

	public void queryRules(
			List<Pair<RuleWritable, AlignmentAndFeatureMap>> rules)
			throws IOException {
		for (Pair<RuleWritable, AlignmentAndFeatureMap> entry : rules) {
			RuleWritable key = entry.getFirst();
			prob.buildQuery(key, mapping.length, wordAlignments);
		}
		List<List<Integer>> keys = new ArrayList<>(wordAlignments.keySet());
		double[] results = query(keys);
		wordAlignments.clear();
		for (int i = 0; i < keys.size(); ++i) {
			if (results[i] != Double.MAX_VALUE) {
				wordAlignments.put(keys.get(i), results[i]);
			}
		}
		for (Pair<RuleWritable, AlignmentAndFeatureMap> entry : rules) {
			RuleWritable key = entry.getFirst();
			AlignmentAndFeatureMap features = entry.getSecond();
			for (int j = 0; j < mapping.length; ++j) {
				double lexProb = prob.value(key, (byte) j, wordAlignments);
				if (features.getSecond().containsKey(mapping[j])) {
					throw new RuntimeException(
							"FeatureMap already contains entry for "
									+ mapping[j] + " " + key + " " + features);
				}
				features.getSecond().put(mapping[j], lexProb);
			}
		}
	}

	@Override
	public void close() throws IOException {
		clientSocket.close();
		in.close();
		out.close();
	}

	/**
	 * @param args
	 * @throws IOException
	 * @throws FileNotFoundException
	 */
	public static void main(String[] args) throws FileNotFoundException,
			IOException {
		String props = args[0];
		Configuration conf = new Configuration();
		Util.ApplyConf(props, FeatureCreator.MAPRED_SUFFIX, conf);
		boolean s2t = "s2t".equals(args[1]);
		try (TTableClient client = new TTableClient()) {
			client.setup(conf, s2t);
			BufferedReader reader = new BufferedReader(new InputStreamReader(
					System.in));
			for (String line = reader.readLine(); line != null; line = reader
					.readLine()) {
				// while(true){
				try {
					String[] fields = line.split("\\s");
					RuleWritable rule = new RuleWritable(new Rule(fields[0],
							fields[1]));
					AlignmentAndFeatureMap map = new AlignmentAndFeatureMap();
					List<Pair<RuleWritable, AlignmentAndFeatureMap>> query = new ArrayList<>();
					query.add(Pair.createPair(rule, map));
					client.queryRules(query);
					System.out.println(rule + "\t" + map);
				} catch (Throwable e) {
					e.printStackTrace();
				}
			}
		}

	}

}
