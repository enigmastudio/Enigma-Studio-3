package scriptanalyzer;

import java.util.LinkedList;

/**
 *
 * @author pap
 */
public class Parameter  implements Comparable<Parameter> {
    String name;
    LinkedList<DataSample> samples = new LinkedList<DataSample>();
    int size = 0;
    void AddSample(DataSample sample) {
        this.samples.add(sample);
        this.size += sample.byteSize;
    }

    public int compareTo(Parameter other) {
        if(this.size < other.size)
            return -1;
        if(this.size > other.size)
            return 1;
        return 0;
    }

}
