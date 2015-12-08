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

import com.beust.jcommander.JCommander;
import com.beust.jcommander.Parameter;
import com.beust.jcommander.ParameterException;
import com.beust.jcommander.ParametersDelegate;

/**
 * Set of utilities. Static methods.
 * 
 * @author Aurelien Waite
 * @author Juan Pino
 * @date 28 May 2014
 */
public final class Util {

	private Util() {

	}

	/**
	 * Private recursive helper function to set properties based on JCommander
	 * parameter objects
	 * 
	 * @param params
	 * @param conf
	 * @throws IllegalArgumentException
	 * @throws IllegalAccessException
	 */
	private static void setProps(Object params, Configuration conf)
			throws IllegalArgumentException, IllegalAccessException {
		for (Field field : params.getClass().getDeclaredFields()) {
			Parameter paramAnnotation = field.getAnnotation(Parameter.class);
			if (paramAnnotation != null) {
				// Use the first name only in annotation
				String name = paramAnnotation.names()[0];
				Object val = field.get(params);
				if(val == null){
					throw new RuntimeException("Null value for " + name);
				}
				Class<?> clazz = val.getClass();
				if (Integer.class == clazz) {
					conf.setInt(name, (Integer) val);
				}else if (Double.class == clazz) {
					conf.set(name, val.toString()); 
				}else if (Boolean.class == clazz) {
					conf.setBoolean(name, (Boolean) val);
				} else if (String.class == clazz) {
					conf.set(name, (String) val);
				}
			} else if (field.getAnnotation(ParametersDelegate.class) != null) {
				setProps(field.get(params), conf);
			}
		}
	}

	public static void ApplyConf(Object params, Configuration conf)
			throws IllegalArgumentException, IllegalAccessException {
		setProps(params, conf);
	}

	public static JCommander parseCommandLine(String[] args, Object params) {
		JCommander cmd = new JCommander();
		cmd.setAcceptUnknownOptions(true);
		cmd.addObject(params);
		try {
			cmd.parse(args);
		} catch (ParameterException e) {
			System.err.println(e.getMessage());
			cmd.usage();
			throw e;
		}
		return cmd;
	}

}
