package uk.ac.cam.eng.extraction.hadoop.util;

import java.io.Externalizable;
import java.io.IOException;

import org.apache.hadoop.io.Writable;

public interface HadoopExternalizable extends Writable, Externalizable {

	default void readExternal(java.io.ObjectInput in) throws IOException{
		readFields(in);
	}
		    
	default void writeExternal(java.io.ObjectOutput out) throws IOException{
		write(out); 
	}
	
}
