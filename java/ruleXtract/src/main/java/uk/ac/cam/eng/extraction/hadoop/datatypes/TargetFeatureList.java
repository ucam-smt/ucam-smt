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
package uk.ac.cam.eng.extraction.hadoop.datatypes;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;

import org.apache.hadoop.io.Text;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.WritableUtils;

import uk.ac.cam.eng.util.Pair;

/**
 * 
 * @author Aurelien Waite
 * @date 28 May 2014
 */
public class TargetFeatureList implements List<Pair<Text, FeatureMap>>,
		Writable {

	private List<Pair<Text, FeatureMap>> instance = new ArrayList<>();

	public int size() {
		return instance.size();
	}

	public boolean isEmpty() {
		return instance.isEmpty();
	}

	public boolean contains(Object o) {
		return instance.contains(o);
	}

	public Iterator<Pair<Text, FeatureMap>> iterator() {
		return instance.iterator();
	}

	public Object[] toArray() {
		return instance.toArray();
	}

	public <T> T[] toArray(T[] a) {
		return instance.toArray(a);
	}

	public boolean add(Pair<Text, FeatureMap> e) {
		return instance.add(e);
	}

	public boolean remove(Object o) {
		return instance.remove(o);
	}

	public boolean containsAll(Collection<?> c) {
		return instance.containsAll(c);
	}

	public boolean addAll(Collection<? extends Pair<Text, FeatureMap>> c) {
		return instance.addAll(c);
	}

	public boolean addAll(int index,
			Collection<? extends Pair<Text, FeatureMap>> c) {
		return instance.addAll(index, c);
	}

	public boolean removeAll(Collection<?> c) {
		return instance.removeAll(c);
	}

	public boolean retainAll(Collection<?> c) {
		return instance.retainAll(c);
	}

	public void clear() {
		instance.clear();
	}

	public boolean equals(Object o) {
		return instance.equals(o);
	}

	public int hashCode() {
		return instance.hashCode();
	}

	public Pair<Text, FeatureMap> get(int index) {
		return instance.get(index);
	}

	public Pair<Text, FeatureMap> set(int index, Pair<Text, FeatureMap> element) {
		return instance.set(index, element);
	}

	public void add(int index, Pair<Text, FeatureMap> element) {
		instance.add(index, element);
	}

	public Pair<Text, FeatureMap> remove(int index) {
		return instance.remove(index);
	}

	public int indexOf(Object o) {
		return instance.indexOf(o);
	}

	public int lastIndexOf(Object o) {
		return instance.lastIndexOf(o);
	}

	public ListIterator<Pair<Text, FeatureMap>> listIterator() {
		return instance.listIterator();
	}

	public ListIterator<Pair<Text, FeatureMap>> listIterator(int index) {
		return instance.listIterator(index);
	}

	public List<Pair<Text, FeatureMap>> subList(int fromIndex, int toIndex) {
		return instance.subList(fromIndex, toIndex);
	}

	@Override
	public void write(DataOutput out) throws IOException {
		WritableUtils.writeVInt(out, instance.size());
		for (Pair<Text, FeatureMap> entry : instance) {
			entry.getFirst().write(out);
			entry.getSecond().write(out);
		}
	}

	@Override
	public void readFields(DataInput in) throws IOException {
		instance.clear();
		int size = WritableUtils.readVInt(in);
		for (int i = 0; i < size; ++i) {
			Text target = new Text();
			target.readFields(in);
			FeatureMap features = new FeatureMap();
			features.readFields(in);
			instance.add(Pair.createPair(target, features));
		}
	}

}
