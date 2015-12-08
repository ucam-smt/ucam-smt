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
package uk.ac.cam.eng.rule.retrieval;

import uk.ac.cam.eng.extraction.D$;
import uk.ac.cam.eng.extraction.S$;
import uk.ac.cam.eng.extraction.V$;
import uk.ac.cam.eng.extraction.X$;



/**
 * 
 * @author Aurelien Waite
 * @date 28 May 2014
 */
public enum EnumRuleType {
	X(X$.MODULE$), V(V$.MODULE$), S(S$.MODULE$), D(D$.MODULE$);

	private EnumRuleType(Object lhs) {
		this.lhs = lhs;
	}

	private final Object lhs;

	public Object getLhs() {
		return lhs;
	}
}
