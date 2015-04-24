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
import java.io.ByteArrayOutputStream
import java.io.DataOutputStream
import java.io.DataInputStream
import java.io.ByteArrayInputStream


class RuleSpec extends FlatSpec with Matchers {

  "A rule" should "serialise and deserialise" in {
    val ruleString = "82073_V_28500_V1_2575 8107_V_1547_V1_205"
    val rule = new Rule(ruleString)
    val byteOut = new ByteArrayOutputStream()
    val out = new DataOutputStream(byteOut)
    rule.write(out)
    val in = new DataInputStream(new ByteArrayInputStream(byteOut.toByteArray()))
    val rule2 = new Rule()
    rule2.readFields(in)
    rule2.toString() should be (ruleString)
  } 
  
  "A rule" should "be invertiable" in {
    val ruleString = "82073_V_28500_V1_2575 8107_V_1547_V1_205"
    val rule = new Rule(ruleString)
    rule.isSwapping() should be (false)
    
    val ruleSwapString = "82073_V_28500_V1_2575 8107_V1_1547_V_205"
    val ruleSwap = new Rule(ruleSwapString)
    ruleSwap.isSwapping() should be (true)
    ruleSwap.invertNonTerminals() should not be (ruleSwap)
    val inverted = new Rule("82073_V1_28500_V_2575 8107_V_1547_V1_205")
    ruleSwap.invertNonTerminals() should be (inverted)
  }
}