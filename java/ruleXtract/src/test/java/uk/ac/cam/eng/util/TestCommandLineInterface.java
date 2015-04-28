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

package uk.ac.cam.eng.util;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import org.apache.hadoop.conf.Configuration;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.ClassRule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

import scala.reflect.internal.Trees.This;
import uk.ac.cam.eng.extraction.hadoop.util.Util;
import uk.ac.cam.eng.util.CLI;

public class TestCommandLineInterface {

	private static final String TEST_CONFIG="/TestConfigFile";
	
	public static File testConfig;
	
	@ClassRule
	public static TemporaryFolder folder = new TemporaryFolder();
	
	@BeforeClass
	public static void setup() throws IOException{
		testConfig = folder.newFile();
		try (OutputStream writer = new FileOutputStream(testConfig)) {
			try (InputStream configFile = TestCommandLineInterface.class.getResourceAsStream(
					TEST_CONFIG)) {
				for (int in = configFile.read(); in != -1; in = configFile.read()) {
					writer.write(in);
				}
			}
		}
	}
	
	@Test
	public void testConfigFile() {
		CLI.ExtractorJobParameters params = new CLI.ExtractorJobParameters();
		String[] args = ("--input=foo --output=bar @" + testConfig.getAbsolutePath()).split(" ");
		Util.parseCommandLine(args, params);
	}
	
	@Test
	public void testApplyConf() throws IllegalArgumentException, IllegalAccessException, IOException{
		CLI.ExtractorJobParameters params = new CLI.ExtractorJobParameters();
		String[] args = ("--input=foo --output=bar @" + testConfig.getAbsolutePath()).split(" ");
		Util.parseCommandLine(args, params);
		Configuration conf = new Configuration();
		Util.ApplyConf(params, conf);
		Assert.assertTrue(conf.getBoolean(CLI.ExtractorJobParameters.REMOVE_MONOTONIC_REPEATS,false));
		String prov = conf.get(CLI.Provenance.PROV);
		Assert.assertEquals("cc,nc,yx,web", prov);
	}

}
