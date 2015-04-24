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

import scala.collection.mutable.ArrayBuffer
import scala.collection.mutable.HashMap

import org.apache.hadoop.io.Writable
import org.apache.hadoop.io.WritableUtils

class Alignment extends Writable {

  val s2t = new HashMap[Int, ArrayBuffer[Int]]
  val t2s = new HashMap[Int, ArrayBuffer[Int]]

  def this(alignmentString: String) = {
    this()
    val alignments = alignmentString.split(" ") map (x =>
      try Some(x split ("-") map (_.toInt))
      catch {case _: NumberFormatException => None})
    for (alignment <- alignments.flatten) {
      addAlignment(alignment(0), alignment(1))
    }
    prepareAlignments()
  }

  def this(other: Alignment) = {
    this()
    for ((key, value) <- other.s2t)
      s2t(key) = new ArrayBuffer[Int] ++= value
    for ((key, value) <- other.t2s)
      t2s(key) = new ArrayBuffer[Int] ++= value
  }

  def getS2T(index: Int) = {
    s2t(index)
  }

  def getT2S(index: Int) = {
    t2s(index)
  }

  def isTargetAligned(index: Int): Boolean = {
    t2s.contains(index)
  }

  def isSourceAligned(index: Int): Boolean = {
    s2t.contains(index)
  }

  private def addAlignment(sourceIndex: Int, targetIndex: Int) {
    s2t.getOrElseUpdate(sourceIndex, new ArrayBuffer) += targetIndex
  }

  def prepareAlignments(): Alignment = {
    for ((k, a) <- s2t) {
      s2t(k) = a.sortWith(_ < _)
      for (t <- a)
        t2s.getOrElseUpdate(t, new ArrayBuffer) += k
    }
    for ((k, a) <- t2s) t2s(k) = a.sortWith(_ < _)
    this
  }

  def clear() = {
    s2t.clear()
    t2s.clear()
  }

  override def toString(): String = {
    val buff = new StringBuffer()
    s2t.foreach {
      case (key, value) => {
        for (t <- value) {
          buff.append(key).append("-").append(t).append(" ")
        }
      }
    }
    buff.toString
  }

  def canEqual(other: Any) = {
    other.isInstanceOf[uk.ac.cam.eng.extraction.Alignment]
  }

  override def equals(other: Any) = {
    other match {
      case that: uk.ac.cam.eng.extraction.Alignment => that.canEqual(Alignment.this) && s2t == that.s2t && t2s == that.t2s
      case _                                        => false
    }
  }

  override def hashCode() = {
    val prime = 41
    prime * (prime + s2t.hashCode) + t2s.hashCode
  }

  private def adjustIndex(a: Int, span: OneNTSpan): Int =
    if (a < span.startX)
      a - span.start
    else
      a - span.start - (span.endX + 1 - span.startX) + 1

  private def adjustIndex2NT(a: Int, span: TwoNTSpan): Int = {
    val inverted = span.startX2 < span.startX
    val (ntSpan1Start, ntSpan1End) = if (inverted) (span.startX2, span.endX2) else (span.startX, span.endX)
    val (ntSpan2Start, ntSpan2End) = if (inverted) (span.startX, span.endX) else (span.startX2, span.endX2)
    val ntSpan1Offset = (ntSpan1End + 1 - ntSpan1Start) - 1
    if (a < ntSpan1Start)
      a - span.start
    else if (a < ntSpan2Start)
      a - span.start - ntSpan1Offset
    else
      a - span.start - ntSpan1Offset - (ntSpan2End + 1 - ntSpan2Start) + 1
  }

  def extractPhraseAlignment(spans: (Span, Span)): Alignment = {
    val phraseAlignment = new Alignment
    val isInPhrase = (src: Int, srcSpan: Span) => src >= srcSpan.start && src <= srcSpan.end
    val isIn1NT = (src: Int, srcSpan: oneNT) => src < srcSpan.startX || src > srcSpan.endX
    val isIn2NT = (src: Int, srcSpan: TwoNTSpan) => src < srcSpan.startX2 || src > srcSpan.endX2
    s2t.foreach {
      case (src, value) => value.foreach { trg =>
        {
          spans match {
            case (srcSpan: PhraseSpan, trgSpan: PhraseSpan) =>
              if (isInPhrase(src, srcSpan))
                phraseAlignment.addAlignment(src - srcSpan.start, trg - trgSpan.start)
            case (srcSpan: OneNTSpan, trgSpan: OneNTSpan) =>
              if (isInPhrase(src, srcSpan) && isIn1NT(src, srcSpan))
                phraseAlignment.addAlignment(adjustIndex(src, srcSpan), adjustIndex(trg, trgSpan))
            case (srcSpan: TwoNTSpan, trgSpan: TwoNTSpan) =>
              if (isInPhrase(src, srcSpan) && isIn1NT(src, srcSpan) && isIn2NT(src, srcSpan))
                phraseAlignment.addAlignment(adjustIndex2NT(src, srcSpan), adjustIndex2NT(trg, trgSpan))
            case _ =>
          }
        }
      }
    }
    phraseAlignment
  }

  def readFields(in: java.io.DataInput): Unit = {
    s2t.clear()
    t2s.clear()
    for (i <- 0 until WritableUtils.readVInt(in)) {
      val s = WritableUtils.readVInt(in)
      for (j <- 0 until WritableUtils.readVInt(in)) {
        val t = WritableUtils.readVInt(in)
        addAlignment(s, t)
      }
    }
  }

  def write(out: java.io.DataOutput): Unit = {
    WritableUtils.writeVInt(out, s2t.size)
    for ((s, a) <- s2t) {
      WritableUtils.writeVInt(out, s)
      WritableUtils.writeVInt(out, a.size)
      for (t <- a) {
        WritableUtils.writeVInt(out, t)
      }
    }
  }

}