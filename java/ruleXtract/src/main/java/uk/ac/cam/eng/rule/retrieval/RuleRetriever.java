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
import java.io.FilenameFilter;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.zip.GZIPOutputStream;

import org.apache.commons.lang.time.StopWatch;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.conf.Configured;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.hbase.io.hfile.CacheConfig;
import org.apache.hadoop.hbase.io.hfile.HFile;
import org.apache.hadoop.hbase.util.BloomFilter;
import org.apache.hadoop.hbase.util.BloomFilterFactory;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Partitioner;
import org.apache.hadoop.mapreduce.lib.partition.HashPartitioner;
import org.apache.hadoop.util.Tool;
import org.apache.hadoop.util.ToolRunner;

import uk.ac.cam.eng.extraction.datatypes.Rule;
import uk.ac.cam.eng.extraction.hadoop.datatypes.AlignmentAndFeatureMap;
import uk.ac.cam.eng.extraction.hadoop.datatypes.RuleWritable;
import uk.ac.cam.eng.extraction.hadoop.util.Util;
import uk.ac.cam.eng.rulebuilding.features.EnumRuleType;
import uk.ac.cam.eng.rulebuilding.features.FeatureCreator;

import com.beust.jcommander.JCommander;
import com.beust.jcommander.Parameter;
import com.beust.jcommander.ParameterException;
import com.beust.jcommander.Parameters;

/**
 * 
 * @author Juan Pino
 * @author Aurelien Waite
 * @date 28 May 2014
 * 
 * Rory's improvements to the retriever over Juan's original implementation
 * include: 
 * 1) A better search algorithm that does not query unigrams out of order of the main search 
 * 2) A more compact HFile format, which results in much smaller HFiles and faster access
 * 3) Multithreaded access to HFile partitions.
 */
public class RuleRetriever extends Configured implements Tool {

	private static final String RETRIEVEL_THREADS = "retrieval.threads";

	private BloomFilter[] bfs;

	private HFileRuleReader[] readers;

	private Partitioner<Text, NullWritable> partitioner = new HashPartitioner<>();

	RuleFilter filter;

	private String asciiConstraintsFileName;

	Set<RuleWritable> asciiConstraints;

	Set<RuleWritable> foundAsciiConstraints = new HashSet<>();

	Set<Text> testVocab;

	Set<Text> foundTestVocab = new HashSet<>();

	private int MAX_SOURCE_PHRASE;

	private void setup(String testFile, Configuration conf)
			throws FileNotFoundException, IOException {

		asciiConstraintsFileName = conf.get("pass_through_rules");
		String filterConfig = conf.get("filter_config");
		if (filterConfig == null) {
			System.err
					.println("Missing property 'filter_config' in the config");
			System.exit(1);
		}
		MAX_SOURCE_PHRASE = conf.getInt("max_source_phrase", 5);
		filter = new RuleFilter(conf);
		asciiConstraints = getAsciiConstraints();
		Set<Text> fullTestVocab = getTestVocab(testFile);
		Set<Text> asciiVocab = getAsciiVocab();
		testVocab = new HashSet<>();
		for (Text word : fullTestVocab) {
			if (!asciiVocab.contains(word)) {
				testVocab.add(word);
			}
		}

	}

	private void loadDir(String dirString, Configuration conf)
			throws IOException {
		File dir = new File(dirString);
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
			// Files are generated by a hadoop reducer have the part
			// number between the 7 and 12th characters, eg part-r-00001
			int i = Integer.parseInt(name.substring(7, 12));
			System.out.println("File has bucket number " +i);
			HFile.Reader hfReader = HFile.createReader(
					FileSystem.getLocal(conf), new Path(file.getPath()),
					cacheConf);
			bfs[i] = BloomFilterFactory.createFromMeta(
					hfReader.getBloomFilterMetadata(), hfReader);
			readers[i] = new HFileRuleReader(hfReader);
		}
	}

	private Set<RuleWritable> getAsciiConstraints() throws IOException {
		Set<RuleWritable> res = new HashSet<>();
		try (BufferedReader br = new BufferedReader(new FileReader(
				asciiConstraintsFileName))) {
			String line;
			Pattern regex = Pattern.compile(".*: (.*) # (.*)");
			Matcher matcher;
			while ((line = br.readLine()) != null) {
				matcher = regex.matcher(line);
				if (matcher.matches()) {
					String[] sourceString = matcher.group(1).split(" ");
					String[] targetString = matcher.group(2).split(" ");
					if (sourceString.length != targetString.length) {
						System.err.println("Malformed ascii constraint file: "
								+ asciiConstraintsFileName);
						System.exit(1);
					}
					List<Integer> source = new ArrayList<Integer>();
					List<Integer> target = new ArrayList<Integer>();
					int i = 0;
					while (i < sourceString.length) {
						if (i % MAX_SOURCE_PHRASE == 0 && i > 0) {
							Rule rule = new Rule(-1, source, target);
							res.add(new RuleWritable(rule));
							source.clear();
							target.clear();
						}
						source.add(Integer.parseInt(sourceString[i]));
						target.add(Integer.parseInt(targetString[i]));
						i++;
					}
					Rule rule = new Rule(-1, source, target);
					res.add(new RuleWritable(rule));
				} else {
					System.err.println("Malformed ascii constraint file: "
							+ asciiConstraintsFileName);
					System.exit(1);
				}
			}
		}
		return res;
	}

	private Set<Text> getAsciiVocab() throws IOException {
		// TODO simplify all template writing
		// TODO getAsciiVocab is redundant with getAsciiConstraints
		Set<Text> res = new HashSet<>();
		try (BufferedReader br = new BufferedReader(new FileReader(
				asciiConstraintsFileName))) {
			String line;
			Pattern regex = Pattern.compile(".*: (.*) # (.*)");
			Matcher matcher;
			while ((line = br.readLine()) != null) {
				matcher = regex.matcher(line);
				if (matcher.matches()) {
					String[] sourceString = matcher.group(1).split(" ");
					// only one word
					if (sourceString.length == 1) {
						res.add(new Text(sourceString[0]));
					}
				} else {
					System.err.println("Malformed ascii constraint file: "
							+ asciiConstraintsFileName);
					System.exit(1);
				}
			}
		}
		return res;
	}

	private Set<Text> getTestVocab(String testFile)
			throws FileNotFoundException, IOException {
		Set<Text> res = new HashSet<>();
		try (BufferedReader br = new BufferedReader(new FileReader(testFile))) {
			String line;
			while ((line = br.readLine()) != null) {
				String[] parts = line.split("\\s+");
				for (String part : parts) {
					res.add(new Text(part));
				}
			}
		}
		return res;
	}

	public Collection<RuleWritable> getGlueRules() {
		List<RuleWritable> res = new ArrayList<>();
		List<Integer> sideGlueRule1 = new ArrayList<Integer>();
		sideGlueRule1.add(-4);
		sideGlueRule1.add(-1);
		Rule glueRule1 = new Rule(-4, sideGlueRule1, sideGlueRule1);
		res.add(new RuleWritable(glueRule1));
		List<Integer> sideGlueRule2 = new ArrayList<Integer>();
		sideGlueRule2.add(-1);
		Rule glueRule2 = new Rule(-1, sideGlueRule2, sideGlueRule2);
		res.add(new RuleWritable(glueRule2));
		List<Integer> sideGlueRule3 = new ArrayList<>();
		sideGlueRule3.add(-1);
		Rule glueRule3 = new Rule(-4, sideGlueRule3, sideGlueRule3);
		res.add(new RuleWritable(glueRule3));
		List<Integer> startSentenceSide = new ArrayList<Integer>();
		startSentenceSide.add(1);
		Rule startSentence = new Rule(-1, startSentenceSide, startSentenceSide);
		res.add(new RuleWritable(startSentence));
		List<Integer> endSentenceSide = new ArrayList<Integer>();
		endSentenceSide.add(2);
		Rule endSentence = new Rule(-1, endSentenceSide, endSentenceSide);
		res.add(new RuleWritable(endSentence));
		return res;
	}

	private List<Set<Text>> generateQueries(String testFileName,
			Configuration conf) throws IOException {
		PatternInstanceCreator patternInstanceCreator = new PatternInstanceCreator(
				conf);
		patternInstanceCreator.createSourcePatterns();
		List<Set<Text>> queries = new ArrayList<>(readers.length);
		for (int i = 0; i < readers.length; ++i) {
			queries.add(new HashSet<Text>());
		}
		try (BufferedReader reader = new BufferedReader(new FileReader(
				testFileName))) {
			int count = 0;
			for (String line = reader.readLine(); line != null; line = reader
					.readLine()) {
				StopWatch stopWatch = new StopWatch();
				stopWatch.start();
				Set<Rule> rules = patternInstanceCreator
						.createSourcePatternInstances(line);
				Collection<Text> sources = new ArrayList<>(rules.size());
				for (Rule rule : rules) {
					Text source = (new RuleWritable(rule)).getSource();
					sources.add(source);
				}
				for (Text source : sources) {
					if (filter.filterSource(source)) {
						continue;
					}
					int partition = partitioner.getPartition(source, null,
							queries.size());
					queries.get(partition).add(source);
				}
				System.out.println("Creating patterns for line " + ++count
						+ " took " + (double) stopWatch.getTime() / 1000d
						+ " seconds");
			}
		}
		return queries;
	}

	/**
	 * Defines command line args.
	 */
	@Parameters(separators = "=")
	public static class RuleRetrieverParameters {
		@Parameter(names = { "--max_source_phrase" }, description = "Maximum source phrase length in a phrase-based rule")
		public String max_source_phrase = "9";

		@Parameter(names = { "--max_source_elements" }, description = "Maximum number of source elements (terminals and nonterminals) in a hiero rule")
		public String max_source_elements = "5";

		@Parameter(names = { "--max_terminal_length" }, description = "Maximum number of consecutive terminals in a hiero rule")
		public String max_terminal_length = "5";

		@Parameter(names = { "--max_nonterminal_span" }, description = "Maximum number of source terminals covered by a right-hand-side source nonterminal in a hiero rule")
		public String max_nonterminal_span = "10";

		@Parameter(names = { "--hr_max_height" }, description = "Maximum number of source terminals covered by the left-hand-side nonterminal in a hiero rule")
		public String hr_max_height = "10";

		@Parameter(names = { "--mapreduce_features" }, description = "Comma-separated list of mapreduce features", required = true)
		public String mapreduce_features;

		@Parameter(names = { "--provenance" }, description = "Comma-separated list of provenances")
		public String provenance;

		@Parameter(names = { "--features" }, description = "Comma-separated list of features, including mapreduce features", required = true)
		public String features;

		@Parameter(names = { "--pass_through_rules" }, description = "File containing pass-through rules")
		public String pass_through_rules;

		@Parameter(names = { "--filter_config" }, description = "File containing additional filtering configuration, e.g. min source-to-target probability")
		public String filter_config;

		@Parameter(names = { "--source_patterns" }, description = "File containing a list of allowed source patterns")
		public String source_patterns;

		@Parameter(names = { "--ttable_s2t_server_port" }, description = "TTable source-to-target server port")
		public String ttable_s2t_server_port = "4949";

		@Parameter(names = { "--ttable_s2t_host" }, description = "TTable source-to-target host name")
		public String ttable_s2t_host = "localhost";

		@Parameter(names = { "--ttable_t2s_server_port" }, description = "TTable target-to-source server port")
		public String ttable_t2s_server_port = "9494";

		@Parameter(names = { "--ttable_t2s_host" }, description = "TTable target-to-source host name")
		public String ttable_t2s_host = "localhost";

		@Parameter(names = { "--retrieval_threads" }, description = "Number of threads for retrieval, corresponds to the number of hfiles")
		public String retrieval_threads;

		@Parameter(names = { "--hfile" }, description = "Directory containing the hfiles")
		public String hfile;

		@Parameter(names = { "--test_file" }, description = "File containing the sentences to be translated")
		public String test_file;

		@Parameter(names = { "--rules" }, description = "Output file containing filtered rules")
		public String rules;
	}

	/**
	 * @param args
	 * @throws IOException
	 * @throws FileNotFoundException
	 * @throws InterruptedException
	 * @throws IllegalAccessException
	 * @throws IllegalArgumentException
	 */
	public int run(String[] args) throws FileNotFoundException, IOException,
			InterruptedException, IllegalArgumentException,
			IllegalAccessException {
		RuleRetrieverParameters params = new RuleRetrieverParameters();
		JCommander cmd = new JCommander(params);

		try {
			cmd.parse(args);
			Configuration conf = getConf();
			Util.ApplyConf(cmd, FeatureCreator.MAPRED_SUFFIX, conf);
			RuleRetriever retriever = new RuleRetriever();
			retriever.loadDir(params.hfile, conf);
			retriever.setup(params.test_file, conf);
			StopWatch stopWatch = new StopWatch();
			stopWatch.start();
			System.err.println("Generating query");
			List<Set<Text>> queries = retriever.generateQueries(
					params.test_file, conf);
			System.err.printf("Query took %d seconds to generate\n",
					stopWatch.getTime() / 1000);
			System.err.println("Executing queries");
			try (BufferedWriter out = new BufferedWriter(
					new OutputStreamWriter(new GZIPOutputStream(
							new FileOutputStream(params.rules))))) {
				FeatureCreator features = new FeatureCreator(conf);
				ExecutorService threadPool = Executors.newFixedThreadPool(Integer.parseInt(params.retrieval_threads));

				for (int i = 0; i < queries.size(); ++i) {
					HFileRuleQuery query = new HFileRuleQuery(
							retriever.readers[i], retriever.bfs[i], out,
							queries.get(i), features, retriever, conf);
					threadPool.execute(query);
				}
				threadPool.shutdown();
				threadPool.awaitTermination(1, TimeUnit.DAYS);
				// Add ascii constraints not already found in query
				for (RuleWritable asciiConstraint : retriever.asciiConstraints) {
					if (!retriever.foundAsciiConstraints
							.contains(asciiConstraint)) {
						features.writeRule(asciiConstraint,
								AlignmentAndFeatureMap.EMPTY,
								EnumRuleType.ASCII_OOV_DELETE, out);
					}
				}
				// Add Deletetion and OOV rules
				RuleWritable deletionRuleWritable = new RuleWritable();
				deletionRuleWritable.setLeftHandSide(new Text(
						EnumRuleType.ASCII_OOV_DELETE.getLhs()));
				deletionRuleWritable.setTarget(new Text("0"));
				RuleWritable oovRuleWritable = new RuleWritable();
				oovRuleWritable.setLeftHandSide(new Text(
						EnumRuleType.ASCII_OOV_DELETE.getLhs()));
				oovRuleWritable.setTarget(new Text(""));
				for (Text source : retriever.testVocab) {
					if (retriever.foundTestVocab.contains(source)) {
						deletionRuleWritable.setSource(source);
						features.writeRule(deletionRuleWritable,
								AlignmentAndFeatureMap.EMPTY,
								EnumRuleType.ASCII_OOV_DELETE, out);
					} else {
						oovRuleWritable.setSource(source);
						features.writeRule(oovRuleWritable,
								AlignmentAndFeatureMap.EMPTY,
								EnumRuleType.ASCII_OOV_DELETE, out);
					}
				}
				// Glue rules
				for (RuleWritable glueRule : retriever.getGlueRules()) {
					features.writeRule(glueRule, AlignmentAndFeatureMap.EMPTY,
							EnumRuleType.GLUE, out);
				}
			}
			System.out.println(retriever.foundAsciiConstraints);
			System.out.println(retriever.foundTestVocab);
		} catch (ParameterException e) {
			System.err.println(e.getMessage());
			cmd.usage();
		}

		return 1;
	}

	public static void main(String[] args) throws Exception {
		int res = ToolRunner.run(new RuleRetriever(), args);
		System.exit(res);
	}
}
