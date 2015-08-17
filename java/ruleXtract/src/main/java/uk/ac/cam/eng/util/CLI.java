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
 *******************************************************************************/

package uk.ac.cam.eng.util;

import com.beust.jcommander.Parameter;
import com.beust.jcommander.Parameters;
import com.beust.jcommander.ParametersDelegate;

/**
 * The command line interface for RuleXtract
 * @author Aurelien Waite
 *
 */
public final class CLI {

	@Parameters(separators = "=")
	public static class Features {

		public static final String FEATURES = "--features";

		@Parameter(names = { FEATURES }, description = "Comma-separated features", required = true)
		public String features;
	}

	@Parameters(separators = "=")
	public static class Provenance {

		public static final String PROV = "--provenance";

		@Parameter(names = { PROV }, description = "Comma-separated provenances", required = true)
		public String provenance;
	}

	@Parameters(separators = "=")
	public static class RuleParameters {
		public static final String MAX_SOURCE_PHRASE = "--max_source_phrase";
		@Parameter(names = { MAX_SOURCE_PHRASE }, description = "Maximum source phrase length in a phrase-based rule")
		public int maxSourcePhrase = 9;

		public static final String MAX_SOURCE_ELEMENTS = "--max_source_elements";
		@Parameter(names = { MAX_SOURCE_ELEMENTS }, description = "Maximum number of source elements (terminals and nonterminals) in a hiero rule")
		public int maxSourceElements = 5;

		public static final String MAX_TERMINAL_LENGTH = "--max_terminal_length";
		@Parameter(names = { MAX_TERMINAL_LENGTH }, description = "Maximum number of consecutive source terminals in a hiero rule")
		public int maxTerminalLength = 5;

		public static final String MAX_NONTERMINAL_SPAN = "--max_nonterminal_span";
		@Parameter(names = { MAX_NONTERMINAL_SPAN }, description = "Maximum number of source terminals covered by a right-hand-side source nonterminal in a hiero rule")
		public int maxNonTerminalSpan = 10;

		@ParametersDelegate
		public Provenance prov = new Provenance();
	}

	/**
	 * Defines command line args.
	 */
	@Parameters(separators = "=")
	public static class ExtractorJobParameters {
		@Parameter(names = { "--input", "-i" }, description = "Input training data on HDFS", required = true)
		public String input;

		@Parameter(names = { "--output", "-o" }, description = "Output rules on HDFS", required = true)
		public String output;

		public static final String REMOVE_MONOTONIC_REPEATS = "--remove_monotonic_repeats";
		@Parameter(names = { REMOVE_MONOTONIC_REPEATS }, description = "Gives an "
				+ "occurrence count of 1 to monotonic hiero rules (e.g. "
				+ "phrase-pair <a b c, d e f> with alignment 0-0 1-1 2-2 "
				+ "generates hiero rule <a X, d X> twice but the count is "
				+ "still one)")
		public boolean removeMonotonicRepeats = true;
		
		public static final String COMPATIBILITY_MODE = "--compatibility_mode";
		@Parameter(names = { COMPATIBILITY_MODE }, description = "Replicates old-style rule extraction")
		public boolean compability_mode = false;

		@ParametersDelegate
		public RuleParameters rp = new RuleParameters();

	}

	@Parameters(separators = "=")
	public static class MarginalReducerParameters {
		@Parameter(names = { "--input", "-i" }, description = "Input rules on HDFS", required = true)
		public String input;

		@Parameter(names = { "--output", "-o" }, description = "Output source-to-target probabilities on HDFS", required = true)
		public String output;
	}

	@Parameters(separators = "=")
	public static class MergeJobParameters {
		@Parameter(names = { "--input_features" }, description = "Comma separated directories on HDFS with computed features", required = true)
		public String inputFeatures;

		@Parameter(names = { "--input_rules" }, description = "HDFS directory with extracted rules", required = true)
		public String inputRules;

		@Parameter(names = { "--output", "-o" }, description = "Output directory on HDFS that will contain rules and features in HFile format", required = true)
		public String output;
		
		@ParametersDelegate
		public FilterParams fp = new FilterParams();
	}

	@Parameters(separators = "=")
	public static class ServerParams {
		@Parameter(names = { "--ttable_s2t_server_port" }, description = "TTable source-to-target server port")
		public int ttableS2TServerPort = 4949;

		@Parameter(names = { "--ttable_s2t_host" }, description = "TTable source-to-target host name")
		public String ttableS2THost = "localhost";

		@Parameter(names = { "--ttable_t2s_server_port" }, description = "TTable target-to-source server port")
		public int ttableT2SServerPort = 9494;

		@Parameter(names = { "--ttable_t2s_host" }, description = "TTable target-to-source host name")
		public String ttableT2SHost = "localhost";
	}

	@Parameters(separators = "=")
	public static class FilterParams {
		public static final String MIN_SOURCE2TARGET_PHRASE = "--min_source2target_phrase";
		@Parameter(names = {MIN_SOURCE2TARGET_PHRASE}, description = "Minimum source to target probability for phrase based rules", required = true)
		public double minSource2TargetPhrase;

		public static final String MIN_TARGET2SOURCE_PHRASE =  "--min_target2source_phrase";
		@Parameter(names = {MIN_TARGET2SOURCE_PHRASE }, description = "Minimum target to source probability for phrase based rules", required = true)
		public double minTarget2SourcePhrase;

		public static final String MIN_SOURCE2TARGET_RULE = "--min_source2target_rule" ;
		@Parameter(names = {MIN_SOURCE2TARGET_RULE }, description = "Minimum source to target probability for hierarchical rules", required = true)
		public double minSource2TargetRule;

		public static final String MIN_TARGET2SOURCE_RULE = "--min_target2source_rule" ;
		@Parameter(names = {MIN_TARGET2SOURCE_RULE }, description = "Minimum target to source probability for hierarchical rules", required = true)
		public double minTarget2SourceRule;

		public static final String PROVENANCE_UNION = "--provenance_union";
		@Parameter(names = { PROVENANCE_UNION }, description = "Union rules extracted from different provenances")
		public boolean provenanceUnion;

		public static final String ALLOWED_PATTERNS =  "--allowed_patterns";
		@Parameter(names = {ALLOWED_PATTERNS }, description = "File containing a list of allowed rule patterns", required = true)
		public String allowedPatternsFile;
	
		public static final String SOURCE_PATTERNS = "--source_patterns";
		@Parameter(names = { SOURCE_PATTERNS }, description = "File containing a list of allowed source patterns", required = true)
		public String sourcePatterns;
	}

	@Parameters(separators = "=")
	public static class RuleRetrieverParameters {

		@Parameter(names = { "--hr_max_height" }, description = "Maximum number of source terminals covered by the left-hand-side nonterminal in a hiero rule")
		public int hr_max_height = 10;

		@ParametersDelegate
		public Features features = new Features();

		@ParametersDelegate
		public RuleParameters rp = new RuleParameters();

		@Parameter(names = { "--pass_through_rules" }, description = "File containing pass-through rules")
		public String passThroughRules;

		@ParametersDelegate
		public ServerParams sp = new ServerParams();

		@Parameter(names = { "--retrieval_threads" }, description = "Number of threads for retrieval, corresponds to the number of hfiles", required=true)
		public int retrievalThreads;

		@Parameter(names = { "--hfile" }, description = "Directory containing the hfiles")
		public String hfile;

		@Parameter(names = { "--test_file" }, description = "File containing the sentences to be translated")
		public String testFile;

		@Parameter(names = { "--rules" }, description = "Output file containing filtered rules", required=true)
		public String rules;
		
		@Parameter(names = { "--vocab" }, description = "Output file containing vocab to be used for language model filtering")
		public String vocab;

		@ParametersDelegate
		public FilterParams fp = new FilterParams();

	}

	@Parameters(separators = "=")
	public static class TTableServerParameters {

		@Parameter(names = { "--ttable_server_template" }, description = "TTable target-to-source host name", required = true)
		public String ttableServerTemplate;

		@Parameter(names = { "--ttable_direction" }, description = "TTable direction for the lexical model ('s2t' or 't2s')", required = true)
		public String ttableDirection;

		@Parameter(names = { "--ttable_language_pair" }, description = "TTable language pair for the lexical model (e.g. 'en2ru' or 'ru2en')", required = true)
		public String ttableLanguagePair;

		@ParametersDelegate
		public Provenance prov = new Provenance();

		@Parameter(names = { "--min_lex_prob" }, description = "Minimum probability for a Model 1 entry. Entries with lower probability are discarded.")
		public double minLexProb = 0.0;

		@ParametersDelegate
		public ServerParams sp = new ServerParams();
	}

	/**
	 * Defines command line args.
	 */
	@Parameters(separators = "=")
	public static class ExtractorDataLoaderParameters {
		@Parameter(names = { "--source", "-src" }, description = "Source text file", required = true)
		public String sourceTextFile;

		@Parameter(names = { "--target", "-trg" }, description = "Target text file", required = true)
		public String targetTextFile;

		@Parameter(names = { "--alignment", "-align" }, description = "Word alignment file", required = true)
		public String alignmentFile;

		@Parameter(names = { "--provenance_file", "-prov_file" }, description = "Provenance file", required = true)
		public String provenanceFile;

		@Parameter(names = { "--hdfsout", "-hdfs" }, description = "Output file name on HDFS", required = true)
		public String hdfsName;
	}

}
