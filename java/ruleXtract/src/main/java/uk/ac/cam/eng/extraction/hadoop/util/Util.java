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
/**
 * 
 */

package uk.ac.cam.eng.extraction.hadoop.util;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.lang.reflect.Field;
import java.nio.ByteBuffer;
import java.util.Properties;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.io.BytesWritable;
import org.apache.hadoop.io.DataInputBuffer;
import org.apache.hadoop.io.Writable;

import uk.ac.cam.eng.extraction.hadoop.datatypes.RuleWritable;
import uk.ac.cam.eng.extraction.hadoop.features.MapReduceFeature;

import com.beust.jcommander.JCommander;

/**
 * Set of utilities. Static methods.
 * 
 * @author Aurelien Waite
 * @author Juan Pino
 * @date 28 May 2014
 */
public class Util {

	private Util() {

	}

	public static byte[] object2ByteArray(Writable obj) throws IOException {
		ByteArrayOutputStream buffer = new ByteArrayOutputStream();
		DataOutputStream out = new DataOutputStream(buffer);
		obj.write(out);
		return buffer.toByteArray();
	}

	/**
	 * Get the bytes between 0 and the length of a BytesWritable. Note that the
	 * first byte starts at 0 because there is no offset.
	 * 
	 * @param bytesWritable
	 * @return the array of byte from 0 to the length of bytesWritable.
	 */
	public static byte[] getBytes(BytesWritable bytesWritable) {
		byte[] buffer = bytesWritable.getBytes();
		int length = bytesWritable.getLength();
		byte[] res = new byte[length];
		for (int i = 0; i < length; i++) {
			res[i] = buffer[i];
		}
		return res;
	}

	public static RuleWritable bytes2RuleWritable(ByteBuffer bytes) {
		DataInputBuffer in = new DataInputBuffer();
		in.reset(bytes.array(), bytes.arrayOffset(), bytes.limit());
		RuleWritable value = new RuleWritable();
		try {
			value.readFields(in);
		} catch (IOException e) {
			// Byte buffer is memory backed so no exception is possible. Just in
			// case chain it to a runtime exception
			throw new RuntimeException(e);
		}
		return value;
	}

	public static BytesWritable bytes2BytesWritable(ByteBuffer bytes) {
		byte[] bytesArr = new byte[bytes.limit()];
		for (int i = 0; i < bytes.limit(); i++) {
			bytesArr[i] = bytes.get(i);
		}
		BytesWritable res = new BytesWritable(bytesArr);
		return res;
	}

	public static RuleWritable byteArray2RuleWritable(byte[] bytes) {
		DataInputBuffer in = new DataInputBuffer();
		in.reset(bytes, 0, bytes.length);
		RuleWritable value = new RuleWritable();
		try {
			value.readFields(in);
		} catch (IOException e) {
			// Byte buffer is memory backed so no exception is possible. Just in
			// case chain it to a runtime exception
			throw new RuntimeException(e);
		}
		return value;
	}

	public static void ApplyConf(JCommander cmd, String suffix,
			Configuration conf) throws IllegalArgumentException,
			IllegalAccessException {
		Object params = cmd.getObjects().get(0);
		for (Field field : params.getClass().getDeclaredFields()) {
			String name = field.getName();
			String value = (String) field.get(params);
			conf.set(name, value);
		}
		String mapreduceFeatures = conf.get("mapreduce_features");
		if (mapreduceFeatures != null) {
			// initial feature index is zero, then increments with the number of
			// features of each feature type. nextFeatureIndex is used to
			// prevent
			// conf to be overwritten before being used.
			int featureIndex = 0, nextFeatureIndex = 0;
			for (String featureString : mapreduceFeatures.split(",")) {
				MapReduceFeature feature =
						MapReduceFeature.findFromConf(featureString);
				if (feature.isProvenanceFeature()) {
					for (String provenance : conf.get("provenance").split(",")) {
						featureIndex = nextFeatureIndex;
						// the next feature index is the current plus the number
						// of
						// features
						// of the current feature class.
						nextFeatureIndex += feature.getNumberOfFeatures();
						conf.setInt(feature.getConfName() + "-" + provenance
								+ suffix, featureIndex);
					}
				} else {
					featureIndex = nextFeatureIndex;
					// the next feature index is the current plus the number of
					// features
					// of the current feature class.
					nextFeatureIndex += feature.getNumberOfFeatures();
					conf.setInt(feature.getConfName() + suffix, featureIndex);
				}
			}
		}
	}

	public static void ApplyConf(Properties p, String suffix, Configuration conf)
			throws IOException {
		for (String prop : p.stringPropertyNames()) {
			conf.set(prop, p.getProperty(prop));
		}
		String mapreduceFeatures = conf.get("mapreduce_features");
		if (mapreduceFeatures == null) {
			System.err.println("ERROR: no mapreduce feature set");
			System.exit(1);
		}
		// initial feature index is zero, then increments with the number of
		// features of each feature type. nextFeatureIndex is used to
		// prevent
		// conf to be overwritten before being used.
		int featureIndex = 0, nextFeatureIndex = 0;
		for (String featureString : mapreduceFeatures.split(",")) {
			MapReduceFeature feature = MapReduceFeature
					.findFromConf(featureString);
			if (feature.isProvenanceFeature()) {
				for (String provenance : conf.get("provenance").split(",")) {
					featureIndex = nextFeatureIndex;
					// the next feature index is the current plus the number
					// of
					// features
					// of the current feature class.
					nextFeatureIndex += feature.getNumberOfFeatures();
					conf.setInt(feature.getConfName() + "-" + provenance
							+ suffix, featureIndex);
				}
			} else {
				featureIndex = nextFeatureIndex;
				// the next feature index is the current plus the number of
				// features
				// of the current feature class.
				nextFeatureIndex += feature.getNumberOfFeatures();
				conf.setInt(feature.getConfName() + suffix, featureIndex);
			}
		}
	}

	public static void ApplyConf(String configFile, String suffix,
			Configuration conf) throws FileNotFoundException, IOException {
		Properties p = new Properties();
		p.load(new FileInputStream(configFile));
		ApplyConf(p, suffix, conf);
	}

	public static void ApplyConf(String configFile, Configuration conf)
			throws FileNotFoundException, IOException {
		ApplyConf(configFile, "", conf);
	}

	public static void ApplyConf(Properties p, Configuration conf)
			throws IOException {
		ApplyConf(p, "", conf);
	}
}
