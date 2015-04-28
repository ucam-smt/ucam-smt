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

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.FilenameFilter;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.zip.GZIPOutputStream;

import org.apache.commons.lang.time.StopWatch;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.hbase.io.hfile.CacheConfig;
import org.apache.hadoop.hbase.io.hfile.HFile;
import org.apache.hadoop.hbase.util.BloomFilter;
import org.apache.hadoop.hbase.util.BloomFilterFactory;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.mapreduce.Partitioner;
import org.apache.hadoop.mapreduce.lib.partition.HashPartitioner;

import uk.ac.cam.eng.extraction.Rule;
import uk.ac.cam.eng.extraction.RuleString;
import uk.ac.cam.eng.extraction.Symbol;
import uk.ac.cam.eng.extraction.hadoop.util.Util;
import uk.ac.cam.eng.rule.features.Feature;
import uk.ac.cam.eng.rule.features.FeatureRegistry;
import uk.ac.cam.eng.rule.filtering.RuleFilter;
import uk.ac.cam.eng.util.CLI;

import com.beust.jcommander.ParameterException;

/**
 * 
 * @author Juan Pino
 * @author Aurelien Waite
 * @date 28 May 2014
 * 
 *       Aurelien's improvements to the retriever over Juan's original
 *       implementation include: 1) A better search algorithm that does not
 *       query unigrams out of order of the main search 2) A more compact HFile
 *       format, which results in much smaller HFiles and faster access 3)
 *       Multithreaded access to HFile partitions.
 */
public class RuleRetriever {

	private BloomFilter[] bfs;

	private HFileRuleReader[] readers;

	private Partitioner<RuleString, NullWritable> partitioner = new HashPartitioner<>();

	RuleFilter filter;

	Set<Rule> passThroughRules = new HashSet<>();

	Set<Rule> foundPassThroughRules = new HashSet<>();

	Set<RuleString> testVocab = new HashSet<>();

	Set<RuleString> foundTestVocab = new HashSet<>();

	Map<RuleString, Set<Integer>> sourceToSentenceId = new HashMap<>();

	List<Set<Symbol>> targetSideVocab = new ArrayList<>();

	private int maxSourcePhrase;

	FeatureRegistry fReg;

	private int noOfFeatures;

	private String targetVocabFile;

	private void setup(String testFile, CLI.RuleRetrieverParameters params)
			throws FileNotFoundException, IOException {
		fReg = new FeatureRegistry(params.features.features,
				params.rp.prov.provenance);
		noOfFeatures = fReg.getFeatureIndices(fReg.getFeatures().toArray(
				new Feature[0])).length;
		filter = new RuleFilter(params.fp, new Configuration());
		maxSourcePhrase = params.rp.maxSourcePhrase;
		Set<RuleString> passThroughVocab = new HashSet<>();
		Set<RuleString> fullTestVocab = getTestVocab(testFile);
		for(Rule r : getPassThroughRules(params.passThroughRules)){
			if(fullTestVocab.contains(r.source())){
				passThroughVocab.add(r.source());
				passThroughRules.add(r);
			}
		}
		testVocab = new HashSet<>();
		for (RuleString word : fullTestVocab) {
			if (!passThroughVocab.contains(word)) {
				testVocab.add(word);
			}
		}
		targetVocabFile = params.vocab;
	}

	private void loadDir(String dirString) throws IOException {
		File dir = new File(dirString);
		Configuration conf = new Configuration();
		CacheConfig cacheConf = new CacheConfig(conf);
		if (!dir.isDirectory()) {
			throw new IOException(dirString + " is not a directory!");
		}
		File[] names = dir.listFiles(new FilenameFilter() {
			@Override
			public boolean accept(File dir, String name) {
				return name.endsWith("hfile");
			}
		});
		if (names.length == 0) {
			throw new IOException("No hfiles in " + dirString);
		}
		bfs = new BloomFilter[names.length];
		readers = new HFileRuleReader[names.length];
		for (File file : names) {
			String name = file.getName();
			int i = Integer.parseInt(name.substring(7, 12));
			HFile.Reader hfReader = HFile.createReader(
					FileSystem.getLocal(conf), new Path(file.getPath()),
					cacheConf);
			bfs[i] = BloomFilterFactory.createFromMeta(
					hfReader.getGeneralBloomFilterMetadata(), hfReader);
			readers[i] = new HFileRuleReader(hfReader);
		}
	}

	private Set<Rule> getPassThroughRules(String passThroughRulesFileName) throws IOException {
		Set<Rule> res = new HashSet<>();
		try (BufferedReader br = new BufferedReader(new FileReader(
				passThroughRulesFileName))) {
			String line;
			Pattern regex = Pattern.compile(".*: (.*) # (.*)");
			Matcher matcher;
			while ((line = br.readLine()) != null) {
				matcher = regex.matcher(line);
				if (matcher.matches()) {
					String[] sourceString = matcher.group(1).split(" ");
					String[] targetString = matcher.group(2).split(" ");
					if (sourceString.length != targetString.length) {
						throw new IOException("Malformed pass through rules file: "
										+ passThroughRulesFileName);
					}
					List<Symbol> source = new ArrayList<Symbol>();
					List<Symbol> target = new ArrayList<Symbol>();
					int i = 0;
					while (i < sourceString.length) {
						if (i % maxSourcePhrase == 0 && i > 0) {
							Rule rule = new Rule(source, target);
							res.add(rule);
							source.clear();
							target.clear();
						}
						source.add(Symbol.deserialise(Integer
								.parseInt(sourceString[i])));
						target.add(Symbol.deserialise(Integer
								.parseInt(targetString[i])));
						i++;
					}
					Rule rule = new Rule(source, target);
					res.add(rule);
				} else {
					throw new IOException("Malformed pass through rules file: "
							+ passThroughRulesFileName);
				}
			}
		}
		return res;
	}


	private Set<RuleString> getTestVocab(String testFile)
			throws FileNotFoundException, IOException {
		Set<RuleString> res = new HashSet<>();
		try (BufferedReader br = new BufferedReader(new FileReader(testFile))) {
			String line;
			while ((line = br.readLine()) != null) {
				String[] parts = line.split("\\s+");
				for (String part : parts) {
					RuleString v = new RuleString();
					v.add(Symbol.deserialise(part));
					res.add(v);
				}
			}
		}
		return res;
	}

	public void writeGlueRules(BufferedWriter out) {
		writeRule(EnumRuleType.S, new Rule("S_D_X S_D_X"),
				fReg.getDefaultDeleteGlueFeatures(), out);
		writeRule(EnumRuleType.S, new Rule("S_X S_X"),
				fReg.getDefaultGlueFeatures(), out);
		writeRule(EnumRuleType.X, new Rule("V V"),
				new TreeMap<Integer, Double>(), out);
		writeRule(EnumRuleType.S, new Rule("1 1"),
				fReg.getDefaultGlueStartOrEndFeatures(), out);
		writeRule(EnumRuleType.X, new Rule("2 2"),
				fReg.getDefaultGlueStartOrEndFeatures(), out);
	}

	private List<Set<RuleString>> generateQueries(String testFileName,
			CLI.RuleRetrieverParameters params) throws IOException {
		PatternInstanceCreator patternInstanceCreator = new PatternInstanceCreator(
				params, filter.getPermittedSourcePatterns());
		List<Set<RuleString>> queries = new ArrayList<>(readers.length);
		for (int i = 0; i < readers.length; ++i) {
			queries.add(new HashSet<RuleString>());
		}
		targetSideVocab.add(Collections.emptySet());
		try (BufferedReader reader = new BufferedReader(new FileReader(
				testFileName))) {
			int count = 1;
			for (String line = reader.readLine(); line != null; line = reader
					.readLine(), ++count) {
				targetSideVocab.add(new HashSet<>());
				StopWatch stopWatch = new StopWatch();
				stopWatch.start();
				Set<Rule> rules = patternInstanceCreator
						.createSourcePatternInstances(line);
				Collection<RuleString> sources = new ArrayList<>(rules.size());
				for (Rule rule : rules) {
					RuleString source = rule.source();
					sources.add(source);
					if (!sourceToSentenceId.containsKey(source)) {
						sourceToSentenceId.put(source, new HashSet<>());
					}
					sourceToSentenceId.get(source).add(count);
				}
				for (RuleString source : sources) {
					if (filter.filterSource(source)) {
						continue;
					}
					int partition = partitioner.getPartition(source, null,
							queries.size());
					queries.get(partition).add(source);
				}
				System.out.println("Creating patterns for line " + count
						+ " took " + (double) stopWatch.getTime() / 1000d
						+ " seconds");
			}
		}
		return queries;
	}

	public void writeRule(EnumRuleType LHS, Rule rule,
			Map<Integer, Double> processedFeatures, BufferedWriter out) {
		StringBuilder res = new StringBuilder();
		res.append(LHS.getLhs()).append(" ").append(rule);
		for (int i = 0; i < noOfFeatures; ++i) {
			// Features are 1-indexed
			double featureValue = processedFeatures.containsKey(i + 1) ? -1
					* processedFeatures.get(i + 1) : 0.0;
			if (Math.floor(featureValue) == featureValue) {
				res.append(String.format(" %d", (int) featureValue));
			} else {
				res.append(String.format(" %f", featureValue));
			}
		}
		res.append("\n");
		synchronized (out) {
			try {
				out.write(res.toString());
			} catch (IOException e) {
				e.printStackTrace();
				System.exit(1);
			}
		}
	}

	/**
	 * @param args
	 * @throws IOException
	 * @throws FileNotFoundException
	 * @throws InterruptedException
	 * @throws IllegalAccessException
	 * @throws IllegalArgumentException
	 */
	public static void main(String[] args) throws FileNotFoundException,
			IOException, InterruptedException, IllegalArgumentException,
			IllegalAccessException {
		CLI.RuleRetrieverParameters params = new CLI.RuleRetrieverParameters();
		try {
			Util.parseCommandLine(args, params);
		} catch (ParameterException e) {
			return;
		}
		RuleRetriever retriever = new RuleRetriever();
		retriever.loadDir(params.hfile);
		retriever.setup(params.testFile, params);
		StopWatch stopWatch = new StopWatch();
		stopWatch.start();
		System.err.println("Generating query");
		List<Set<RuleString>> queries = retriever.generateQueries(
				params.testFile, params);
		System.err.printf("Query took %d seconds to generate\n",
				stopWatch.getTime() / 1000);
		System.err.println("Executing queries");
		try (BufferedWriter out = new BufferedWriter(new OutputStreamWriter(
				new GZIPOutputStream(new FileOutputStream(params.rules))))) {
			ExecutorService threadPool = Executors
					.newFixedThreadPool(params.retrievalThreads);
			for (int i = 0; i < queries.size(); ++i) {
				HFileRuleQuery query = new HFileRuleQuery(retriever.readers[i],
						retriever.bfs[i], out, queries.get(i), retriever,
						params.sp);
				threadPool.execute(query);
			}
			threadPool.shutdown();
			threadPool.awaitTermination(1, TimeUnit.DAYS);
			// Add pass through rule not already found in query
			for (Rule passThroughRule : retriever.passThroughRules) {
				if (!retriever.foundPassThroughRules.contains(passThroughRule)) {
					retriever.writeRule(EnumRuleType.X, passThroughRule,
							retriever.fReg.getDefaultPassThroughRuleFeatures(),
							out);
				}
			}
			// Add Deletion and OOV rules
			Rule deletionRuleWritable = new Rule();
			RuleString dr = new RuleString();
			dr.add(Symbol.deserialise(-7));
			deletionRuleWritable.setTarget(dr);
			Rule oovRuleWritable = new Rule();
			RuleString oov = new RuleString();
			oov.add(Symbol.deserialise(-6));
			oovRuleWritable.setTarget(oov);
			for (RuleString source : retriever.testVocab) {
				// If in the vocab then write deletion rule
				if (retriever.foundTestVocab.contains(source)) {
					deletionRuleWritable.setSource(source);
					retriever.writeRule(EnumRuleType.D, deletionRuleWritable,
							retriever.fReg.getDefaultDeletionFeatures(), out);
					// Otherwise is an OOV
				} else {
					oovRuleWritable.setSource(source);
					retriever.writeRule(EnumRuleType.X, oovRuleWritable,
							retriever.fReg.getDefaultOOVFeatures(), out);
				}
			}
			// Glue rules
			retriever.writeGlueRules(out);
		}
		System.out.println(retriever.foundPassThroughRules);
		System.out.println(retriever.foundTestVocab);
		if (retriever.targetVocabFile != null) {
			try (BufferedWriter out = new BufferedWriter(new FileWriter(
					retriever.targetVocabFile))) {
				for (Set<Symbol> words : retriever.targetSideVocab.subList(1,
						retriever.targetSideVocab.size())) {
					out.write("1 2");
					for(Symbol word : words){
						out.write(" " + word);
					}
					out.write("\n");
				}
			}
		}
	}

}
