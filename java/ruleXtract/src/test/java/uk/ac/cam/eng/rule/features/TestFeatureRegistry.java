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

package uk.ac.cam.eng.rule.features;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;

import org.junit.Test;

public class TestFeatureRegistry {

	public static final int[] S2T_LEX_PROB = new int[]{10};
	public static final int[] PROV_S2T_LEX_PROB = new int[]{20, 21, 22, 23};
	public static final int[] MULTIPLE = new int[]{11, 12, 13, 14, 15, 16, 17, 18, 19};
	
	
	@Test
	public void testFeatureRegistry() {
		FeatureRegistry fReg = new FeatureRegistry(
				"source2target_probability,target2source_probability,word_insertion_penalty,rule_insertion_penalty,glue_rule,insert_scale,rule_count_1,rule_count_2,rule_count_greater_than_2,source2target_lexical_probability,target2source_lexical_probability,provenance_source2target_probability,provenance_target2source_probability,provenance_source2target_lexical_probability,provenance_target2source_lexical_probability",
				"cc,nc,yx,web");
		assertEquals(15, fReg.getFeatures().size());
		assertArrayEquals(S2T_LEX_PROB,fReg
				.getFeatureIndices(Feature.SOURCE2TARGET_LEXICAL_PROBABILITY));
		assertArrayEquals(PROV_S2T_LEX_PROB, fReg
						.getFeatureIndices(Feature.PROVENANCE_SOURCE2TARGET_LEXICAL_PROBABILITY));
		assertArrayEquals(MULTIPLE, fReg.getFeatureIndices(
				Feature.TARGET2SOURCE_LEXICAL_PROBABILITY,
				Feature.PROVENANCE_SOURCE2TARGET_PROBABILITY,
				Feature.PROVENANCE_TARGET2SOURCE_PROBABILITY));
	}

}
