/**
 * *****************************************************************************
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
 * *****************************************************************************
 */

package uk.ac.cam.eng

import resource._
import scala.concurrent._
import scala.concurrent.duration._
import ExecutionContext.Implicits.global

import scala.collection.JavaConversions.iterableAsScalaIterable
import org.apache.hadoop.conf.Configuration
import org.apache.hadoop.fs.FileSystem
import org.apache.hadoop.fs.Path
import org.apache.hadoop.hbase.io.hfile.CacheConfig
import org.apache.hadoop.hbase.io.hfile.HFile
import org.apache.hadoop.io.IntWritable
import org.apache.hadoop.io.SequenceFile
import org.apache.hadoop.io.Writable
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat
import org.apache.hadoop.mapreduce.lib.input.MultipleInputs
import org.apache.hadoop.mapreduce.lib.input.SequenceFileInputFormat
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat
import org.apache.hadoop.util.ReflectionUtils
import org.scalatest.BeforeAndAfterAll
import org.scalatest.ConfigMap
import org.scalatest.FunSuite
import uk.ac.cam.eng.extraction.Rule
import uk.ac.cam.eng.extraction.RuleExtractorTest
import uk.ac.cam.eng.extraction.RuleString
import uk.ac.cam.eng.extraction.hadoop.extraction.ExtractorJob
import uk.ac.cam.eng.extraction.hadoop.features.phrase.Source2TargetJob
import uk.ac.cam.eng.extraction.hadoop.features.phrase.Target2SourceJob
import uk.ac.cam.eng.extraction.hadoop.merge.MergeJob
import uk.ac.cam.eng.extraction.hadoop.merge.MergeJob.MergeFeatureMapper
import uk.ac.cam.eng.extraction.hadoop.merge.MergeJob.MergeRuleMapper
import uk.ac.cam.eng.rule.features.Feature
import uk.ac.cam.eng.rule.retrieval.HFileRuleReader
import uk.ac.cam.eng.util.CLI
import uk.ac.cam.eng.util.CLI.FilterParams
import uk.ac.cam.eng.extraction.hadoop.datatypes.FeatureMap
import uk.ac.cam.eng.extraction.hadoop.datatypes.ExtractedData


/**
 * An integration test of the full extraction pipeline.
 */
class PipelineTest extends FunSuite with BeforeAndAfterAll {

  override def beforeAll(configMap: ConfigMap) {
    RuleExtractorTest.folder.create()
    RuleExtractorTest.setupFileSystem()
  }

  override def afterAll(configMap: ConfigMap) {
    RuleExtractorTest.folder.delete()
  }

  def filterSequenceFile(input: Path, output: Path, conf: Configuration) = {
    val reader = new SequenceFile.Reader(FileSystem.get(conf), input, conf);
    val key = ReflectionUtils.newInstance(
      reader.getKeyClass(), conf).asInstanceOf[Writable]
    val value = ReflectionUtils.newInstance(
      reader.getValueClass(), conf).asInstanceOf[Writable]
    for (writer <- 
        managed(new SequenceFile.Writer(FileSystem.get(conf), conf, output, key.getClass, value.getClass))) {
      var count = 0
      while (reader.next(key, value)) {
        if (count % 10 == 0)
          writer.append(key, value)
        count += 1
      }
    }
  }

  def assertWithDelta(expected: Double)(result: Double) = {
    val delta = 1D / 1024D
    assert(Math.abs(expected - result) < delta)
  }

  test("The rule extraction job") {
    val conf = new Configuration
    conf.set("mapreduce.framework.name", "local");
    conf.setInt(CLI.RuleParameters.MAX_SOURCE_PHRASE, 9)
    conf.setInt(CLI.RuleParameters.MAX_SOURCE_ELEMENTS, 5)
    conf.setInt(CLI.RuleParameters.MAX_TERMINAL_LENGTH, 5)
    conf.setInt(CLI.RuleParameters.MAX_NONTERMINAL_SPAN, 10)
    conf.setBoolean(CLI.ExtractorJobParameters.REMOVE_MONOTONIC_REPEATS, true)
    conf.setBoolean(CLI.ExtractorJobParameters.COMPATIBILITY_MODE, true)
    conf.set(CLI.Provenance.PROV, "all");
    val job = ExtractorJob.getJob(conf)
    val trainingData = new Path(RuleExtractorTest.trainingDataFile.getAbsolutePath)
    val filteredData = new Path(RuleExtractorTest.folder.newFile().getAbsolutePath)
    filterSequenceFile(trainingData, filteredData, conf)
    FileInputFormat.setInputPaths(job, filteredData)
    val extractOut = new Path("extractOut")
    FileOutputFormat.setOutputPath(job, extractOut);
    job.waitForCompletion(true);

    val s2tOut = new Path("s2t")
    val s2tJob = (new Source2TargetJob).getJob(conf)
    FileInputFormat.setInputPaths(s2tJob, extractOut)
    FileOutputFormat.setOutputPath(s2tJob, s2tOut);
    val fs2t = Future {
      s2tJob.waitForCompletion(true)
    }
    val t2sOut = new Path("t2s")
    val t2sJob = (new Target2SourceJob).getJob(conf)
    FileInputFormat.setInputPaths(t2sJob, extractOut)
    FileOutputFormat.setOutputPath(t2sJob, t2sOut);
    val ft2s = Future {
      t2sJob.waitForCompletion(true)
    }
    Await.ready(fs2t, 1 hours)
    Await.ready(ft2s, 1 hours)

    conf.set(FilterParams.MIN_SOURCE2TARGET_PHRASE, "0.01");
    conf.set(FilterParams.MIN_TARGET2SOURCE_PHRASE, "1e-10");
    conf.set(FilterParams.MIN_SOURCE2TARGET_RULE, "0.01");
    conf.set(FilterParams.MIN_TARGET2SOURCE_RULE, "1e-10");
    conf.setBoolean(FilterParams.PROVENANCE_UNION, true);
    val patternsFile = RuleExtractorTest.copyDataToTestDir("/CF.rulextract.patterns").toPath.toUri.toString;
    conf.set(FilterParams.SOURCE_PATTERNS, patternsFile)
    val allowedFile = RuleExtractorTest.copyDataToTestDir("/CF.rulextract.filter.allowedonly").toPath.toUri.toString;
    conf.set(FilterParams.ALLOWED_PATTERNS, allowedFile)

    val mergeJob = MergeJob.getJob(conf);
    for (featurePath <- List(s2tOut, t2sOut)) {
      MultipleInputs.addInputPath(mergeJob, featurePath,
        classOf[SequenceFileInputFormat[Rule, FeatureMap]], classOf[MergeFeatureMapper]);
    }
    MultipleInputs.addInputPath(mergeJob, extractOut,
      classOf[SequenceFileInputFormat[Rule, ExtractedData]], classOf[MergeRuleMapper]);
    val mergeOut = new Path("mergeOut")
    FileOutputFormat.setOutputPath(mergeJob, mergeOut);
    mergeJob.waitForCompletion(true)

    val cacheConf = new CacheConfig(conf);
    val hfReader = HFile.createReader(FileSystem.get(conf),
      new Path(mergeOut, "part-r-00000.hfile"), cacheConf);
    val reader = new HFileRuleReader(hfReader);
    var count = 0
    var notTested = 0
    for (entry <- reader) {
      count += 1
      val data = entry.getSecond
      entry.getFirst match {
        case Rule("5660 1294") => {
          assertWithDelta(-1.0986122886681098)(data.getFeatures.get(Feature.SOURCE2TARGET_PROBABILITY).get(new IntWritable(0)).get)
          assertWithDelta(-0.6931471805599453)(data.getFeatures.get(Feature.TARGET2SOURCE_PROBABILITY).get(new IntWritable(0)).get)
        }
        case Rule("1804_6 2967_8_3") => {
          assertWithDelta(-1.3862943611198906)(data.getFeatures.get(Feature.SOURCE2TARGET_PROBABILITY).get(new IntWritable(0)).get)
          assertWithDelta(0.0)(data.getFeatures.get(Feature.TARGET2SOURCE_PROBABILITY).get(new IntWritable(0)).get)
        }
        case Rule("V_437 V_3_920") => {
          assertWithDelta(-0.916290731874155)(data.getFeatures.get(Feature.SOURCE2TARGET_PROBABILITY).get(new IntWritable(0)).get)
          assertWithDelta(0.0)(data.getFeatures.get(Feature.TARGET2SOURCE_PROBABILITY).get(new IntWritable(0)).get)
        }
        case Rule("2617_3_10619_2675 507_4015_3083") => {
          assertWithDelta(0.0)(data.getFeatures.get(Feature.SOURCE2TARGET_PROBABILITY).get(new IntWritable(0)).get)
          assertWithDelta(0.0)(data.getFeatures.get(Feature.TARGET2SOURCE_PROBABILITY).get(new IntWritable(0)).get)
        }
        case Rule("222_1148_34716_151055_5_265808 1819_1857_3312_9_3_670870") => {
          assertWithDelta(0.0)(data.getFeatures.get(Feature.SOURCE2TARGET_PROBABILITY).get(new IntWritable(0)).get)
          assertWithDelta(0.0)(data.getFeatures.get(Feature.TARGET2SOURCE_PROBABILITY).get(new IntWritable(0)).get)
        }
        case _ => notTested += 1
      }
    }
    assertResult(125834)(count)
    assertResult(5)(count - notTested)

    val seekReader = new HFileRuleReader(hfReader)
    assert(seekReader.seek(RuleString("V_437")))
    assert(seekReader.getRulesForSource.foldLeft(0)((count, _) => count + 1) == 2)
  }
}