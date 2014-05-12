//Copyright (c) 2012, University of Cambridge
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met://
//
//    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
//    * Neither the name of the University of Cambridge nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "ParamsConfig.h"
#include "MertCommon.h"

void ParamsConfig::Initialize (const PARAMS& params_min,
                               const PARAMS& params_max) {
  m_params_min = params_min;
  m_params_max = params_max;
  if (m_params_min.empty() || m_params_max.empty() ) {
    return;
  } else if (m_params_min.size() != m_params_min.size() ) {
    throw std::invalid_argument ("invalid minimum and maximum parameters");
  }
  for (int k = 0; k < params_min.size(); ++k) {
    if (m_params_min[k] > m_params_max[k]) {
      throw std::invalid_argument ("invalid minimum and maximum parameters");
    }
  }
}

bool ParamsConfig::check_in_range (const unsigned int k,
                                   const double gamma) const {
  if (!m_params_max.empty() && (fabs (gamma) >= m_params_max[k]) ) {
    tracer
        << "    parameters not updated: gamma < minimum parameter value of "
        << m_params_max[k] << '\n';
    return false;
  }
  if (!m_params_min.empty() && (fabs (gamma) <= m_params_min[k]) ) {
    tracer
        << "    parameters not updated: gamma > maximum parameter value of "
        << m_params_min[k] << '\n';
    return false;
  }
  return true;
}

