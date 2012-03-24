package kkrunchyanalyzer;

import java.util.*;

/**
 *
 * @author Pap
 */
public class FunctionMapper {
    HashMap<String, ObjectFile> m_map = new HashMap<String, ObjectFile>();


    public  ObjectFile  getObjectFile(String name) {
        ObjectFile res = m_map.get(name);
        if(res == null) {
            res = new ObjectFile(name);
            this.m_map.put(name, res);
        }
        return res;
    }

    public LinkedList<ObjectFile>   GetSorted() {
        LinkedList<ObjectFile> res = new LinkedList<ObjectFile>(this.m_map.values());
        Collections.sort(res);
        return res;
    }
}
