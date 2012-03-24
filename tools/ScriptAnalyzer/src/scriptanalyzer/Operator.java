package scriptanalyzer;

import java.util.HashMap;
import java.util.LinkedList;

/**
 *
 * @author pap
 */
public class Operator implements Comparable<Operator> {
    String  name;
    HashMap<String, Parameter> paramMap = new HashMap<String, Parameter>();
    LinkedList<DataSample> samples = new LinkedList<DataSample>();

    int size = 0;
    void AddSample(DataSample sample) {
        this.samples.add(sample);
        Parameter p = paramMap.get(sample.paramName);
        if(p == null) {
            p = new Parameter();
            p.name = sample.paramName;
            this.paramMap.put(p.name, p);
        }
        p.AddSample(sample);
        this.size += sample.byteSize;
    }

    public int compareTo(Operator other) {
        if(this.size < other.size)
            return -1;
        if(this.size > other.size)
            return 1;
        return 0;
    }

}
