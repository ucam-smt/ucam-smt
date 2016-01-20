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

case class Symbol (val serialised: Int) extends AnyVal with Ordered[Symbol] {

  def compare(that: Symbol) = this.serialised - that.serialised

  override def toString() = {
    import Symbol._
    this match{
      case V => "V"
      case V1 => "V1"
      case S => "S"
      case X => "X"
      case D => "D"
      case `oov` => "<oov>"
      case `dr` => "<dr>"
      case _ => serialised.toString
    }
  }

}

sealed trait SymbolClass

case object Terminal extends SymbolClass

case object NonTerminal extends SymbolClass


object Symbol {

  val V = Symbol(-1)
    
  val V1 = Symbol(-2)

  val S = Symbol(-3)

  val X = Symbol(-4)

  val D = Symbol(-5)

  val oov = Symbol(-6)

  val dr = Symbol(-7)
  
  val nonTerminalMappings = Map(V.toString() -> V, V1.toString() -> V1,
    S.toString() -> S, X.toString() -> X, D.toString -> D, 
    oov.toString() -> oov, dr.toString() -> dr)

  val nonTerminals = Set.empty ++ nonTerminalMappings.values

  def deserialise(symbol: Int): Symbol = Symbol(symbol)

  def deserialise(symbol: String): Symbol =
    nonTerminalMappings.getOrElse(symbol, Symbol(symbol.toInt))

  def isNonTerminal(s : Symbol) = nonTerminals.contains(s)

  def getStringRepresentation(i : Int) = Symbol.deserialise(i).toString()

}



