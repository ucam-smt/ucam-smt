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

import resource._
import scala.collection.JavaConverters._
import scala.collection.immutable._
import scala.io.Source
import java.util.zip.GZIPInputStream
import scala.collection.mutable.ArrayBuffer
import org.scalatest._
import org.apache.hadoop.io.SequenceFile
import org.apache.hadoop.fs.FileSystem
import org.apache.hadoop.io.MapWritable
import uk.ac.cam.eng.extraction.hadoop.datatypes.TextArrayWritable
import org.apache.hadoop.fs.Path
import org.apache.hadoop.io.Text


/**
 * A test of compatibility mode. The output of the extractor is compared with the previous
 * C++ based extractor. 
 * 
 * The current code completely matches around 60% of the previous output. Any more changes to
 * the extraction algorithm should match or increase this level of compatibility.
 */

class ExtractIntegrationSpec extends FlatSpec with Matchers {

  "The extractor" should "match previous output" in {
    diffs should be(9368)
  }

  def getSource(f: String) =
    Source.fromInputStream(new GZIPInputStream(this.getClass.getClassLoader.getResourceAsStream(f)))

  def readOldDiff(oldIter: Iterator[String]): String = {
    val rules = new ArrayBuffer[String]
    for (line <- oldIter) {
      if (line == "--") {
        return rules.sorted.mkString("\n")
      }
      val fields = line.split(" ")
      val elem = (fields(1) + " " + fields(2)).replace("X1", "V").replace("X2", "V1").replace("X", "V")
      rules ++= (for (i <- 0 until fields(0).toInt) yield elem)
    }
    return rules.sorted.mkString("\n")
  }

  def diffs(): Int = {
    RuleExtractorTest.folder.create()
    RuleExtractorTest.setupFileSystem()
    val opt = new ExtractOptions(9, 5, 5, 10, true, true)
    val extract = Extract.extract(opt)_
    var diffs = 0
    for (
      reader <- managed(new SequenceFile.Reader(
        FileSystem.get(RuleExtractorTest.conf), new Path(RuleExtractorTest.trainingDataFile.getPath()),
        RuleExtractorTest.conf)); oldRules <- managed(getSource("oldExtractOut.gz"))
    ) {
      val key = new MapWritable()
      val value = new TextArrayWritable()
      val offset = 0
      var count = offset
      for (i <- 0 until offset) {
        reader.next(key, value)
      }
      val oldIter = oldRules.getLines
      val start = System.currentTimeMillis()
      while (reader.next(key, value)) {
        count += 1
        val strings = value.get.map { _ match { case t: Text => t.toString() } }
        val src = strings(0)
        val trg = strings(1)
        val align = strings(2)
        val newString = extract(src, trg, align).map { _ match { case (r, a) => r.toString() } }.toList.sorted.mkString("\n")
        val oldString = readOldDiff(oldIter)
        if (oldString != newString) diffs += 1
      }
    }
    RuleExtractorTest.folder.delete()
    diffs
  }

}