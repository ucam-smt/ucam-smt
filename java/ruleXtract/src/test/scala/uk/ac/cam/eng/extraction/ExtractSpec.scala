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

package uk.ac.cam.eng.extraction

import org.scalatest._

class ExtractSpec extends FlatSpec with Matchers {

  val sourceString = "3 4 5 6"
  val targetString = "13 14 15 16"
  val alignString = "0-0 1-2 3-1"
  
  val output = List(Rule("V_4_V1 V_V1_15"), Rule("V_4_5_V1 V_V1_15"), Rule("V_4_5_6 V_14_15"), 
      Rule("V_5 V"), Rule("V_5_V1 V1_V"), Rule("V_5_6 14_V"), Rule("V_6 14_V"), Rule("3 13"), 
      Rule("3_V 13_V"), Rule("3_V_5_V1 13_V1_V"), Rule("3_V_5_6 13_14_V"), Rule("3_V_5_6 13_14_V"), 
      Rule("3_V_5_6 13_14_V"), Rule("3_V_5_6 13_14_V_16"), Rule("3_V_6 13_14_V"), Rule("3_V_6 13_14_V"),
      Rule("3_V_6 13_14_V"), Rule("3_V_6 13_14_V_16"), Rule("3_4_V 13_V_15"), Rule("3_4_5_V 13_V_15"), 
      Rule("3_4_5_6 13_14_15"), Rule("3_4_5_6 13_14_15_16"), Rule("4 15"), Rule("4 15_16"), Rule("4_V V_15"),
      Rule("4_5 15"), Rule("4_5 15_16"), Rule("4_5_V V_15"), Rule("4_5_6 14_15"), Rule("4_5_6 14_15_16"), 
      Rule("5_6 14"), Rule("6 14")).sorted(S2TOrdering)
  
  "Source '" + sourceString + "', target '" + targetString + "', and alignments '" + 
    alignString + "'" should " extract the rules " + output.mkString(", ") in {
    val opt = new ExtractOptions(9, 5, 5, 10, true, false)
    val extract = Extract.extract(opt)_
    val rules = extract(sourceString, targetString, alignString).map(_._1).sorted(S2TOrdering) 
    rules.size should be(output.size)
    for ((r, o) <- rules zip output) (r should be(o))
  } 
  
}