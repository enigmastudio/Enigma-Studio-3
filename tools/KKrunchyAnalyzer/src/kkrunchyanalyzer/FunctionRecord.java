package kkrunchyanalyzer;

/**
 *
 * @author Pap
 */
public class FunctionRecord implements Comparable<FunctionRecord> {
    String  m_name;
    String  m_objectFile;
    int     m_size;

    public FunctionRecord(String name, String objFile, int size) {
        this.m_name = name;
        this.m_size = size;
        this.m_objectFile = objFile;
    }

    public int compareTo(FunctionRecord other) {
        if(this.m_size < other.m_size)
            return 1;
        else
            if(this.m_size > other.m_size)
                return -1;
        return 0;
    }
}
