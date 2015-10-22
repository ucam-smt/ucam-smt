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

sealed trait Phrase {
  def start: Int
  def end: Int
}

sealed trait oneNT {
  def startX: Int
  def endX: Int
}

sealed abstract class Span extends Phrase 

case class PhraseSpan(override val start: Int, override val end: Int) extends Span

case class OneNTSpan(override val start: Int, override val end: Int,
                     override val startX: Int, override val endX: Int) extends Span with oneNT

case class TwoNTSpan(override val start: Int, override val end: Int,
                     override val startX: Int, override val endX: Int, startX2: Int, endX2: Int)
  extends Span with oneNT
  