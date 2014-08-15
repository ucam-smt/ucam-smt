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

import java.lang.reflect.Field;

import org.apache.hadoop.conf.Configuration;

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
}
