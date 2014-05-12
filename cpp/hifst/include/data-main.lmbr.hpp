// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use these files except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Copyright 2012 - Gonzalo Iglesias, Adrià de Gispert, William Byrne

#ifndef DATA_MAIN_LMBR_HPP
#define DATA_MAIN_LMBR_HPP

namespace ucam {
namespace lmbr {

struct lmbrtunedata;

class LmbrTaskData {
 public:
  LmbrTaskData() : sidx (0), lmbronebest (NULL) {};
  uint sidx;
  unordered_map<std::string, void *> fsts;
  lmbrtunedata *lmbronebest;
};

}
} // end namespaces
#endif
