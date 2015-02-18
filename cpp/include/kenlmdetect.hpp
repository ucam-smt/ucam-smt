// Adapted from kenlm code
#ifndef KENLM_UTIL_HPP
#define KENLM_UTIL_HPP

#ifndef KENLM_MAX_ORDER
#else 


#include <lm/binary_format.hh>
#include <lm/model.hh>
#ifdef WITH_NPLM
#include <lm/wrappers/nplm.hh>
#endif 
/**
 * \file
 */

namespace ucam {
namespace util {


/**
 * Detects kenlm language model types
 * code recycled from kenlm tool
 */

enum {KENLM_NPLM=-1};

inline int detectkenlm (std::string const& kenlmfile) {
  lm::ngram::ModelType model_type;
  if (kenlmfile == "" ) {
    FORCELINFO("Empty language model file name. Sets to Probing.");
    return lm::ngram::PROBING;
  }
  if (lm::ngram::RecognizeBinary (kenlmfile.c_str(), model_type) ) {
    switch (model_type) {
    case lm::ngram::PROBING:
    case lm::ngram::REST_PROBING:
    case lm::ngram::TRIE:
    case lm::ngram::QUANT_TRIE:
    case lm::ngram::ARRAY_TRIE:
    case lm::ngram::QUANT_ARRAY_TRIE:
      return model_type;
    default:
      LERROR ("Unrecognized kenlm model type " << model_type );
      exit (EXIT_FAILURE);
    }
#ifdef WITH_NPLM
  } else if (lm::np::Model::Recognize(kenlmfile)) {
    return KENLM_NPLM;
#endif
  } else { // possibly arpa file?
    return lm::ngram::PROBING;
  }
}

}} // end namespaces

#endif
#endif
