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
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import uk.ac.cam.eng.extraction.Rule;
import uk.ac.cam.eng.extraction.hadoop.datatypes.ProvenanceProbMap;
import uk.ac.cam.eng.extraction.hadoop.datatypes.RuleData;
import uk.ac.cam.eng.rule.features.Feature;
import uk.ac.cam.eng.rule.retrieval.EnumRuleType;
import uk.ac.cam.eng.util.CLI;
import uk.ac.cam.eng.util.Pair;

/**
 * 
 * @author Aurelien Waite
 * @date 28 May 2014
 */
public class TTableClient {

	private String hostName;

	private int port;

	private LexicalProbability prob;

	private Map<List<Integer>, Double> wordAlignments = new HashMap<>();

	private int noOfProvs;
	
	Feature lexicalF;
	
	Feature provLexicalF;

	public void setup(CLI.ServerParams params,
			int noOfProvs, boolean source2Target) {
		if (source2Target) {
			port = params.ttableS2TServerPort;
			hostName = params.ttableS2THost;
			lexicalF = Feature.SOURCE2TARGET_LEXICAL_PROBABILITY;
			provLexicalF = Feature.PROVENANCE_SOURCE2TARGET_LEXICAL_PROBABILITY;
		} else {
			port = params.ttableT2SServerPort;
			hostName = params.ttableT2SHost;
			lexicalF = Feature.TARGET2SOURCE_LEXICAL_PROBABILITY;
			provLexicalF = Feature.PROVENANCE_TARGET2SOURCE_LEXICAL_PROBABILITY;
		}
		prob = new LexicalProbability(source2Target);
		this.noOfProvs = noOfProvs +1;
	}

	private double[] query(Collection<List<Integer>> query) throws IOException {

		try (Socket clientSocket = new Socket(hostName, port);
				DataInputStream in = new DataInputStream(
						new BufferedInputStream(clientSocket.getInputStream(),
								TTableServer.BUFFER_SIZE));
				DataOutputStream out = new DataOutputStream(
						new BufferedOutputStream(
								clientSocket.getOutputStream(),
								TTableServer.BUFFER_SIZE))) {

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
			return result;
		} catch (java.net.ConnectException e) {
			String message = "Failed to connect to ttable server. Hostname: "
					+ hostName + " Port: " + port;
			throw new IOException(message, e);
		}
	}

	public void queryRules(
			Map<Rule, Pair<EnumRuleType, RuleData>> rules)
			throws IOException {
		for (Entry<Rule, Pair<EnumRuleType,RuleData>> entry : rules.entrySet()) {
			Rule key = entry.getKey();
			// Need to add the 0th element for the global scope
			prob.buildQuery(key, noOfProvs, wordAlignments);
		}
		List<List<Integer>> keys = new ArrayList<>(wordAlignments.keySet());
		double[] results = query(keys);
		wordAlignments.clear();
		for (int i = 0; i < keys.size(); ++i) {
			if (results[i] != Double.MAX_VALUE) {
				wordAlignments.put(keys.get(i), results[i]);
			}
		}
		for (Entry<Rule, Pair<EnumRuleType,RuleData>> entry : rules.entrySet()) {
			Rule key = entry.getKey();
			RuleData features = entry.getValue().getSecond();
			ProvenanceProbMap provLexProbs = new ProvenanceProbMap();
			for (int j = 0; j < noOfProvs ; ++j) {
				double lexProb = prob.value(key, (byte) j, wordAlignments);
				if(j==0){
					ProvenanceProbMap globalLexProb = new ProvenanceProbMap();
					globalLexProb.put(j, lexProb);
					features.getFeatures().put(lexicalF, globalLexProb);
				}else{
					provLexProbs.put(j, lexProb);
				}
				features.getFeatures().put(provLexicalF, provLexProbs);
			}
		}
	}

}