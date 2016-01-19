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

package uk.ac.cam.eng

package object extraction {

  // First element is the rule, second is the set of alignments found for this rule, third is the start span of the parent, and fourth is the end span of the parent
  type RuleTuple = (Rule, Alignment, Span)
  type ASet = scala.collection.mutable.HashSet[Alignment]
  type RSet = scala.collection.mutable.HashSet[Rule]
  type RulePair = (Rule, Alignment)

  object RuleTupleOrdering extends Ordering[RuleTuple] {
    override def compare(t1: RuleTuple, t2: RuleTuple) = S2TOrdering.compare(t1._1, t2._1)
  }

  def toJavaInteger(i : Int) : Integer = i
}