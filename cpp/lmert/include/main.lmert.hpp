#ifndef MAIN_LMERT_HPP
#define MAIN_LMERT_HPP

#include <global_incls.hpp>
#include <custom_assert.hpp>
#include <global_decls.hpp>
#include <global_funcs.hpp>
#include <fst/fstlib.h>
#include <fst/script/print.h>
#include <logger.hpp>
#include <params.hpp>
#include <tropical-sparse-tuple-weight-incls.h>
#include <tropical-sparse-tuple-weight.h>
#include <tropical-sparse-tuple-weight-decls.h>
#include <szfstream.hpp>
#include <registrypo.hpp>
#include <taskinterface.hpp>
#include <range.hpp>
#include <addresshandler.hpp>
#include <fstio.hpp>
#include <constants-fsttools.hpp>

#include <main.lmert.init_param_options.hpp>

// parameter vector
typedef std::vector<float> PARAMS32;
std::ostream& operator<< (std::ostream& o, const PARAMS32& v) {
  for (unsigned int i=0; i<v.size(); i++) {
    o << v[i] << " ";
  }
  return o;
}

// word and sentence typedefs
typedef long long Wid;
typedef std::vector<Wid> SentenceIdx;
std::ostream& operator<< (std::ostream& o, const SentenceIdx& s) {
  for (unsigned int i = 0; i < s.size(); ++i) {
    if (i > 0) {
      o << " ";
    }
    o << s[i];
  }
  return o;
}

typedef unsigned int Sid;

#endif
