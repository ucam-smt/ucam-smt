#ifndef CONSTANTS_FSTTOOLS_HPP
#define CONSTANTS_FSTTOOLS_HPP

namespace HifstConstants {
/// List of constants to be used both across program options and class runners.

//  const string kNThreads="nthreads";
//  const string kRange="range";
const std::string kRangeExtended = kRange + ",r";
const std::string kInput = "input";
const std::string kInputExtended = "input,i";
const std::string kOutput = "output";
const std::string kOutputExtended = "output,o";

const std::string kHifstSemiring = "semiring";
const std::string kHifstSemiringStdArc = "stdarc";
const std::string kHifstSemiringLexStdArc = "lexstdarc";
const std::string kHifstSemiringTupleArc = "tuplearc";
const std::string kHifstSemiringExtended = "semiring,s";

// Language model

const std::string kLmFeatureweights = "lm.featureweights";
const std::string kLmLoad = "lm.load";
const std::string kLmWordmap = "lm.wordmap";
const std::string kLmWordPenalty = "lm.wps";
const std::string kLmLogTen = "lm.log10";

const std::string kLatticeLoad = "lattice.load";
const std::string kLatticeLoadDeleteLmCost = "lattice.load.deletelmcost";
const std::string kLatticeStore = "lattice.store";
const std::string kStatsWrite = "stats.write";

// Disambig/recaser

const std::string kRecaserLmLoad = "recaser.lm.load";
const std::string kRecaserLmFeatureweight = "recaser.lm.scale";
const std::string kRecaserUnimapLoad = "recaser.unimap.load";
const std::string kRecaserUnimapWeight = "recaser.unimap.scale";
const std::string kRecaserPrune = "recaser.prune";
const std::string kRecaserInput = "recaser.input";
const std::string kRecaserInputExtended = kRecaserInput + ",i";
const std::string kRecaserOutput = "recaser.output";
const std::string kRecaserOutputExtended = kRecaserOutput + ",o";

// not used TODO: check
const std::string kRecaserLmWps = "recaser.lm.wps";
const std::string kRecaserLmWordmap = "recaser.lm.wordmap";

// printstrings
const std::string kUnique = "unique";
const std::string kUniqueExtended = kUnique + ",u";
const std::string kNbest = "nbest";
const std::string kNbestExtended = kNbest + ",n";
const std::string kSparseFormat = "sparseformat";
const std::string kSparseDotProduct = "dotproduct";
const std::string kTupleArcWeights = "tuplearc.weights";
const std::string kWeight = "weight";
const std::string kWeightExtended = kWeight + ",w";
const std::string kLabelMap = "label-map";
const std::string kLabelMapExtended = kLabelMap + ",m";
const std::string kPrintOutputLabels = "print-output-labels";
const std::string kPrintOutputLabelsExtended = kPrintOutputLabels + ",pol";

// samplehyps
const std::string kWordRefs = "word_refs";
const std::string kIntRefs = "int_refs";
const std::string kExternalTokenizer = "external_tokenizer";
const std::string kWordMap = "word_map";
const std::string kAlpha = "alpha";
const std::string kNSamples= "num_samples";
const std::string kNegativeExamples= "negative_examples";
const std::string kDontNegate= "dont_negate";
const std::string kBinaryTarget= "binary_target";
const std::string kRandomSeed= "random_seed";

// lexmap
const std::string kAction = "action";
const std::string kActionLex2std = "lex2std";
const std::string kActionStd2lex = "std2lex";
const std::string kActionProjectweight2 = "projectweight2";
// tunewp
const std::string kEpsilonLabels = "epsilons";
const std::string kWordPenalty = "word_penalty";
const std::string kWordPenaltyExtended = kWordPenalty + ",wp";
//instead of arc_type, use semiring

}

#endif

