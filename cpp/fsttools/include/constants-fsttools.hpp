#ifndef CONSTANTS_FSTTOOLS_HPP
#define CONSTANTS_FSTTOOLS_HPP

namespace HifstConstants {
/// List of constants to be used both across program options and class runners.

//  const string kNThreads="nthreads";
//  const string kRange="range";
std::string const kRangeExtended = kRange + ",r";
std::string const kInput = "input";
std::string const kInputExtended = "input,i";
std::string const kOutput = "output";
std::string const kOutputExtended = "output,o";

std::string const kHifstSemiring = "semiring";
std::string const kHifstSemiringStdArc = "stdarc";
std::string const kHifstSemiringLexStdArc = "lexstdarc";
std::string const kHifstSemiringTupleArc = "tuplearc";
std::string const kHifstSemiringExtended = "semiring,s";

// Language model
std::string const kLmFeatureweights = "lm.featureweights";
std::string const kLmLoad = "lm.load";
std::string const kLmWordmap = "lm.wordmap";
std::string const kLmWordPenalty = "lm.wps";
std::string const kLmLogTen = "lm.log10";

std::string const kLatticeLoad = "lattice.load";
std::string const kLatticeLoadDeleteLmCost = "lattice.load.deletelmcost";
std::string const kLatticeStore = "lattice.store";
std::string const kStatsWrite = "stats.write";

std::string const kUseBilingualModel = "usebilm";
std::string const kUseBilingualModelSourceSize = "usebilm.sourcesize";
std::string const kUseBilingualModelSourceSentenceFile = "usebilm.sourcesentencefile";
std::string const kTune = "tune";
std::string const kTuneWordPenaltyRange = "tune.wp";
std::string const kTuneWrite = "tune.write";
// Disambig/recaser

std::string const kRecaserLmLoad = "recaser.lm.load";
std::string const kRecaserLmFeatureweight = "recaser.lm.scale";
std::string const kRecaserUnimapLoad = "recaser.unimap.load";
std::string const kRecaserUnimapWeight = "recaser.unimap.scale";
std::string const kRecaserPrune = "recaser.prune";
std::string const kRecaserInput = "recaser.input";
std::string const kRecaserInputExtended = kRecaserInput + ",i";
std::string const kRecaserOutput = "recaser.output";
std::string const kRecaserOutputExtended = kRecaserOutput + ",o";

// not used TODO: check
std::string const kRecaserLmWps = "recaser.lm.wps";
std::string const kRecaserLmWordmap = "recaser.lm.wordmap";

// printstrings
std::string const kUnique = "unique";
std::string const kUniqueExtended = kUnique + ",u";
std::string const kNbest = "nbest";
std::string const kNbestExtended = kNbest + ",n";
std::string const kSentBleu = "sbleu";
std::string const kSparseFormat = "sparseformat";
std::string const kSparseDotProduct = "dotproduct";
std::string const kTupleArcWeights = "tuplearc.weights";
std::string const kWeight = "weight";
std::string const kWeightExtended = kWeight + ",w";
std::string const kWeightPrecision = "weight.precision";
std::string const kLabelMap = "label-map";
std::string const kLabelMapExtended = kLabelMap + ",m";
std::string const kPrintOutputLabels = "print-output-labels";
std::string const kPrintOutputLabelsExtended = kPrintOutputLabels + ",pol";
std::string const kPrintInputOutputLabels = "print-input-output-labels";
std::string const kIntersectionWithHypothesesLoad = "intersection-with-hypotheses.load";

std::string const kSuppress = "suppress-hyps";
std::string const kLibLinRankFormat = "liblinear-ranking";

// samplehyps
std::string const kWordRefs = "word_refs";
std::string const kIntRefs = "int_refs";
std::string const kExternalTokenizer = "external_tokenizer";
std::string const kWordMap = "word_map";
std::string const kAlpha = "alpha";
std::string const kNSamples= "num_samples";
std::string const kNegativeExamples= "negative_examples";
std::string const kDontNegate= "dont_negate";
std::string const kBinaryTarget= "binary_target";
std::string const kRandomSeed= "random_seed";

// lexmap
std::string const kAction = "action";
std::string const kActionLex2std = "lex2std";
std::string const kActionStd2lex = "std2lex";
std::string const kActionProjectweight2 = "projectweight2";
// tunewp
std::string const kEpsilonLabels = "epsilons";
std::string const kWordPenalty = "word_penalty";
std::string const kWordPenaltyExtended = kWordPenalty + ",wp";
//instead of arc_type, use semiring

std::string const kYes = "yes";
std::string const kNo = "no";
std::string const kUserWpRange = "%%wp%%";

std::string const kDeterminizeOutput = "determinize-output";
std::string const kMinimize = "minimize";
std::string const kExitOnFirstPassFailure = "exit-on-first-pass-failure";
std::string const kUseOpenFst = "use-openfst";

}

namespace HC = HifstConstants;

#endif

