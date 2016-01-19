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

import Symbol._

import scala.collection.mutable.ArrayBuffer
import scala.math.Ordering.Implicits.seqDerivedOrdering
import org.apache.hadoop.io.Writable
import org.apache.hadoop.io.WritableUtils
import org.apache.hadoop.io.WritableComparable
import collection.JavaConversions._
import uk.ac.cam.eng.rule.retrieval.SidePattern
import uk.ac.cam.eng.extraction.hadoop.merge.MergeComparator

class RuleString extends ArrayBuffer[Symbol] with Writable with WritableComparable[RuleString] {

  def readFields(in: java.io.DataInput): Unit = {
    clear();
    for (i <- 0 until WritableUtils.readVInt(in))
      this += Symbol.deserialise(WritableUtils.readVInt(in))
  }

  def write(out: java.io.DataOutput): Unit = {
    WritableUtils.writeVInt(out, size)
    for (s <- this) WritableUtils.writeVInt(out, s.serialised)
  }
  
  override def compareTo(other : RuleString) = RuleString.comparator.compare(this, other)

  // Methods needed for java

  def javaSize(): Int = size

  def set(other: RuleString) = {
    clear
    this ++= other
  }

  def add(other: Symbol) = this += other

  override def toString() = this.map(_.toString).mkString("_")

  def toPattern(): SidePattern = new SidePattern(this.map { s =>
    if(!Symbol.isNonTerminal(s))
      "w"
    else
      s.serialised.toString()
  }.foldLeft(new ArrayBuffer[String]){(pattern, sym)=> 
      if (!pattern.isEmpty && pattern.last == sym) pattern else pattern += sym})

  def getWordCount() = this.count (
    !Symbol.isNonTerminal(_)
  )
 
  def getTerminals() : java.util.List[Integer]= this.filterNot(Symbol.isNonTerminal(_))
    .map{s => toJavaInteger(s.serialised)}
  
}

object RuleString{
  val comparator = new MergeComparator
  
  def apply(s : String) : RuleString = new RuleString ++= s.split("_").map(Symbol.deserialise(_))
  
}

class Rule(val source: RuleString, val target: RuleString) extends Equals
  with Writable with WritableComparable[Rule] {

  def this() = this(new RuleString, new RuleString)

  def this(str: String) = {
    this()
    val parsed = str.split(" ").map(_.split("_").map(Symbol.deserialise(_)))
    if (parsed.size != 2) throw new Exception("Bad format for rule: " + str)
    source ++= parsed(0)
    target ++= parsed(1)
  }

  def this(other: Rule) =
    this(new RuleString ++= other.source, new RuleString ++= other.target)

  def this(src: java.util.List[java.lang.Integer], trg: java.util.List[java.lang.Integer]) =
    this(new RuleString ++= src.map(deserialise(_)), new RuleString ++= trg.map(deserialise(_)))

  override def toString() = source.toString() + " " + target.toString()

  private def isSwappingString(str: Seq[Symbol]): Boolean = {
    for (symbol <- str)
      if (symbol == V) return false
      else if (symbol == V1) return true
    false
  }

  def isSwapping() = isSwappingString(source) || isSwappingString(target)

  def invertString(str: Seq[Symbol]) = {
    val results = new RuleString
    for (symbol <- str)
      if (symbol == V1) results += V
      else if (symbol == V) results += V1
      else results += symbol
    results
  }

  def invertNonTerminals(): Rule = 
    if (isSwapping)
      new Rule(invertString(source), invertString(target))
    else
      this
    
  def canEqual(other: Any) = {
    other.isInstanceOf[uk.ac.cam.eng.extraction.Rule]
  }

  override def equals(other: Any) = {
    other match {
      case that: uk.ac.cam.eng.extraction.Rule => that.canEqual(Rule.this) && source == that.source && target == that.target
      case _                                   => false
    }
  }

  override def hashCode() = {
    val prime = 41
    prime * (prime + source.hashCode) + target.hashCode
  }

  def readFields(in: java.io.DataInput): Unit = {
    source.readFields(in);
    target.readFields(in)
  }

  def write(out: java.io.DataOutput): Unit = {
    source.write(out)
    target.write(out)
  }

  override def compareTo(other: Rule) = S2TOrdering.compare(this, other)

  def getSource(): java.util.List[java.lang.Integer] = source.map{s => toJavaInteger(s.serialised)}

  def getTarget(): java.util.List[java.lang.Integer] = target.map{s => toJavaInteger(s.serialised)}

  private def set(str: RuleString, other: RuleString) = {
    str.clear()
    str ++= other
  }

  def setSource(other: RuleString) = set(source, other)

  def setTarget(other: RuleString) = set(target, other)

}

object S2TOrdering extends Ordering[Rule] {
  val vecOrdering = seqDerivedOrdering[ArrayBuffer, Symbol]

  override def compare(r1: Rule, r2: Rule) = {
    val diff = vecOrdering.compare(r1.source, r2.source)
    if (diff == 0) {
      vecOrdering.compare(r1.target, r2.target)
    } else {
      diff
    }
  }

}

object Rule{

  def apply(s : String) : Rule = new Rule(s)
  
  def unapply(r : Rule) : Option[String] = Some(r.toString())
}