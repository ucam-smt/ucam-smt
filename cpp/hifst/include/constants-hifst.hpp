#ifndef CONSTANTS_HIFST_HPP
#define CONSTANTS_HIFST_HPP

namespace HifstConstants {
/// List of constants to be used both across program options and class runners.

//createssgrammar and hifst

//const string kServerEnable="server.enable";
const std::string kServerPort = "server.port";

const std::string kFeatureweights = "featureweights";

const std::string kGrammarLoad = "grammar.load";
const std::string kGrammarFeatureweights = "grammar.featureweights";
const std::string kGrammarStorepatterns = "grammar.storepatterns";
const std::string kGrammarStorentorder = "grammar.storentorder";

const std::string kSourceLoad = "source.load";
const std::string kTargetStore = "target.store";

const std::string kPreproTokenizeEnable = "prepro.tokenize.enable";
const std::string kPreproTokenizeLanguage = "prepro.tokenize.language";
const std::string kPreproAddsentencemarkers = "prepro.addsentencemarkers";
const std::string kPreproWordmapLoad = "prepro.wordmap.load";

const std::string kPatternstoinstancesMaxspan = "patternstoinstances.maxspan";
const std::string kPatternstoinstancesGapmaxspan =
  "patternstoinstances.gapmaxspan";
const std::string kPatternstoinstancesStore = "patternstoinstances.store";

const std::string kSsgrammarStore = "ssgrammar.store";
const std::string kSsgrammarAddoovsEnable = "ssgrammar.addoovs.enable";
const std::string kSsgrammarAddoovsSourcedeletions =
  "ssgrammar.addoovs.sourcedeletions";

// only hifst
const std::string kReferencefilterLoad = "referencefilter.load";
const std::string kReferencefilterLoadSemiring = "referencefilter.load.semiring";
const std::string kReferencefilterWrite = "referencefilter.write";
const std::string kReferencefilterSubstring = "referencefilter.substring";
const std::string kReferencefilterPrunereferenceweight =
  "referencefilter.prunereferenceweight";
const std::string kReferencefilterPrunereferenceshortestpath =
  "referencefilter.prunereferenceshortestpath";

//currently not used as an option
const std::string kReferencefilterNosubstringStore =
  "referencefilter.nosubstring.store";

const std::string kCykparserHrmaxheight = "cykparser.hrmaxheight";
const std::string kCykparserHmax = "cykparser.hmax";
const std::string kCykparserHmin = "cykparser.hmin";
const std::string kCykparserNtexceptionsmaxspan =
  "cykparser.ntexceptionsmaxspan";

const std::string kHifstLatticeStore = "hifst.lattice.store";
const std::string kHifstLatticeOptimize = "hifst.lattice.optimize";
const std::string kHifstAlilatsmode = "hifst.alilatsmode";
const std::string kHifstUsepdt = "hifst.usepdt";
const std::string kHifstRtnopt = "hifst.rtnopt";
const std::string kHifstOptimizecells = "hifst.optimizecells";
const std::string kHifstReplacefstbyarcNonterminals =
  "hifst.replacefstbyarc.nonterminals";
const std::string kHifstReplacefstbyarcNumstates =
  "hifst.replacefstbyarc.numstates";
const std::string kHifstReplacefstbyarcExceptions =
  "hifst.replacefstbyarc.exceptions";
const std::string kHifstLocalpruneEnable = "hifst.localprune.enable";
const std::string kHifstLocalpruneLmLoad = "hifst.localprune.lm.load";
const std::string kHifstLocalpruneLmFeatureweights =
  "hifst.localprune.lm.featureweights";
const std::string kHifstLocalpruneLmWordpenalty = "hifst.localprune.lm.wps";
const std::string kHifstLocalpruneConditions = "hifst.localprune.conditions";
const std::string kHifstLocalpruneNumstates = "hifst.localprune.numstates";
const std::string kHifstPrune = "hifst.prune";
const std::string kHifstWritertn = "hifst.writertn";

const std::string kHifstDisableRuleFeatures = "hifst.disablerulefeatures";

const std::string kHifstStripSpecialEpsilonLabels = "hifst.lattice.optimize.stripspecialepsilonlabels";

const std::string kPostproInput = "postpro.input";

const std::string kPostproWordmapLoad = "postpro.wordmap.load";
const std::string kPostproDetokenizeEnable = "postpro.detokenize.enable";
const std::string kPostproDetokenizeLanguage = "postpro.detokenize.language";
const std::string kPostproCapitalizefirstwordEnable =
  "postpro.capitalizefirstword.enable";

const std::string kStatsHifstWrite = "stats.hifst.write";
const std::string kStatsHifstCykgridEnable = "stats.hifst.cykgrid.enable";
const std::string kStatsHifstCykgridCellwidth = "stats.hifst.cykgrid.cellwidth";
//  const string kStatsWrite="stats.write";


// alilats2splats
const std::string kRuleflowerlatticeFilterbyalilats =
  "ruleflowerlattice.filterbyalilats";
const std::string kRuleflowerlatticeLoad = "ruleflowerlattice.load";
const std::string kRuleflowerlatticeStore = "ruleflowerlattice.store";
const std::string kRuleflowerlatticeFeatureweights =
  "ruleflowerlattice.featureweights";
const std::string kSparseweightvectorlatticeLoadalilats =
  "sparseweightvectorlattice.loadalilats";
const std::string kSparseweightvectorlatticeStore =
  "sparseweightvectorlattice.store";
const std::string kSparseweightvectorlatticeStorenbestfile =
  "sparseweightvectorlattice.storenbestfile";
const std::string kSparseweightvectorlatticeWordmap =
  "sparseweightvectorlattice.wordmap";
const std::string kSparseweightvectorlatticeStorefeaturefile =
  "sparseweightvectorlattice.storefeaturefile";
const std::string kSparseweightvectorlatticeFirstsparsefeatureatindex =
  "sparseweightvectorlattice.firstsparsefeatureatindex";

const std::string kSparseweightvectorlatticeStorenolm =
  "sparseweightvectorlattice.storenolm";
const std::string kSparseweightvectorlatticeStripSpecialEpsilonLabels =
  "sparseweightvectorlattice.stripspecialepsilonlabels";

// rules2weights specific
const std::string kRulesToWeightsEnable = "rulestoweights.enable";
const std::string kRulesToWeightsNumberOfLanguageModels = "rulestoweights.numlms";
const std::string kRulesToWeightsLoadalilats = "rulestoweights.loadalilats";
const std::string kRulesToWeightsLatticeStore = "rulestoweights.store";
const std::string kRulesToWeightsLoadGrammar = "rulestoweights.loadgrammar";
const std::string kRulesToWeightsLatticeFilterbyAlilats = "rulestoweights.filterbyalilats";

// lmbr-specific

const std::string kLmbrLexstdarc =
  "lexstdarc"; // TODO; should be unified with --semiring
const std::string kLmbrLoadEvidencespace = "load.evidencespace";
const std::string kLmbrLoadHypothesesspace = "load.hypothesesspace";
const std::string kLmbrWritedecoder = "writedecoder";
const std::string kLmbrWriteonebest = "writeonebest";
const std::string kLmbrMinorder = "minorder";
const std::string kLmbrMaxorder = "maxorder";
const std::string kLmbrAlpha = "alpha";
const std::string kLmbrWps = "wps";
const std::string kLmbrP = "p";
const std::string kLmbrR = "r";
const std::string kLmbrT = "T";
const std::string kLmbrPreprune = "preprune";

// hifst-client
const std::string kHifstHost = "host";
const std::string kHifstPort = "port";

}

#endif

