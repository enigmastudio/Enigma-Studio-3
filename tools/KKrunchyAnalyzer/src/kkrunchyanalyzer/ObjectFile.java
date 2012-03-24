package kkrunchyanalyzer;

import java.util.*;

/**
 *
 * @author Pap
 */
public class ObjectFile implements Comparable<ObjectFile> {
    String  m_name;
    int     m_size = 0;

    LinkedList<FunctionRecord>  m_records = new LinkedList<FunctionRecord>();

    public  ObjectFile(String name) {
        this.m_name = name;
    }

    public  void AddFunction(String name, int size) {
        FunctionRecord rec = new FunctionRecord(name, this.m_name, size);
        this.m_records.add(rec);
        this.m_size += size;

        // sort
        Collections.sort(m_records);
    }

    public int compareTo(ObjectFile other) {
        if(this.m_size < other.m_size)
            return 1;
        else
            if(this.m_size > other.m_size)
                return -1;
        return 0;
    }

}
