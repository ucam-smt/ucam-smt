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
 * Copyright 2014 - Juan Pino, Aurelien Waite, William Byrne
 *******************************************************************************/
package uk.ac.cam.eng.extraction.hadoop.util;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.zip.GZIPInputStream;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.MapWritable;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.io.SequenceFile;
import org.apache.hadoop.io.Text;

import uk.ac.cam.eng.extraction.hadoop.datatypes.TextArrayWritable;
import uk.ac.cam.eng.util.CLI;

/**
 * Load all the word aligned parallel text onto HDFS ready to have rules
 * extracted
 * 
 * @author Aurelien Waite
 * @author Juan Pino
 * @date 28 May 2014
 */
public class ExtractorDataLoader {

	/**
	 * Loads word aligned parallel text to HDFS.
	 * 
	 * @param sourceTextFile
	 *            The source text file, gzipped, with one sentence per line,
	 *            same number of lines as targetTextFile.
	 * @param targetTextFile
	 *            The target text file, gzipped, with one sentence per line,
	 *            same number of lines as sourceTextFile.
	 * @param wordAlignmentFile
	 *            The word alignment file, gzipped, one alignment per line in
	 *            Berkeley format ("0-0<SPACE>1-2, etc.", zero-based source
	 *            index on the left), same number of lines as sourceTextFile.
	 * @param provenanceFile
	 *            The provenance file, gzipped, one set of provenances per line
	 *            with format "prov1<SPACE>prov2, etc.", same number of lines as
	 *            sourceTextFile.
	 * @param hdfsName
	 * @throws IOException
	 */
	public void loadTrainingData2Hdfs(String sourceTextFile,
			String targetTextFile, String wordAlignmentFile,
			String provenanceFile, String hdfsName)
			throws FileNotFoundException, IOException {

		try (BufferedReader src = new BufferedReader(new InputStreamReader(
				new GZIPInputStream(new FileInputStream(sourceTextFile))));
				BufferedReader trg = new BufferedReader(
						new InputStreamReader(new GZIPInputStream(
								new FileInputStream(targetTextFile))));
				BufferedReader align = new BufferedReader(
						new InputStreamReader(new GZIPInputStream(
								new FileInputStream(wordAlignmentFile))));
				BufferedReader prov = new BufferedReader(
						new InputStreamReader(new GZIPInputStream(
								new FileInputStream(provenanceFile))))) {

			String srcLine = null, trgLine = null, alignLine = null, provLine = null;
			Configuration conf = new Configuration();
			Path path = new Path(hdfsName);
			FileSystem fs = path.getFileSystem(conf);
			try (SequenceFile.Writer writer = new SequenceFile.Writer(fs, conf,
					path, MapWritable.class, TextArrayWritable.class)) {
				Text sourceSentenceText = new Text();
				Text targetSentenceText = new Text();
				Text alignmentText = new Text();
				Text[] array = new Text[3];
				array[0] = sourceSentenceText;
				array[1] = targetSentenceText;
				array[2] = alignmentText;
				TextArrayWritable arrayWritable = new TextArrayWritable();
				// metadata: provenance, e.g. genre, collection, training
				// instance
				// id, doc id, etc.
				MapWritable metadata = new MapWritable();

				while ((srcLine = src.readLine()) != null
						&& (trgLine = trg.readLine()) != null
						&& (alignLine = align.readLine()) != null
						&& (provLine = prov.readLine()) != null) {
					metadata.clear();
					String[] provenances = provLine.split("\\s+");
					for (String provenance : provenances) {
						metadata.put(new Text(provenance), NullWritable.get());
					}
					sourceSentenceText.set(srcLine);
					targetSentenceText.set(trgLine);
					// note, alignLine can be the empty string
					alignmentText.set(alignLine);
					arrayWritable.set(array);
					writer.append(metadata, arrayWritable);
				}
			}
		}
	}

	public static void main(String[] args) throws FileNotFoundException,
			IOException {
		CLI.ExtractorDataLoaderParameters params = new CLI.ExtractorDataLoaderParameters();
		Util.parseCommandLine(args, params);
		ExtractorDataLoader loader = new ExtractorDataLoader();
		loader.loadTrainingData2Hdfs(params.sourceTextFile,
				params.targetTextFile, params.alignmentFile,
				params.provenanceFile, params.hdfsName);

	}
}
