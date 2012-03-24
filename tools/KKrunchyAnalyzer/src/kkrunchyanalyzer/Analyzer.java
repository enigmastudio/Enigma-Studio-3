package kkrunchyanalyzer;

import java.io.*;
import java.util.*;
import java.io.File;

/**
 *
 * @author Pap
 */
public class Analyzer {

    FunctionMapper              m_mapper = new FunctionMapper();
    LinkedList<FunctionRecord>  m_pureRecs = new LinkedList<FunctionRecord>();
    HashMap<String, PureRec>    m_pureFunctions = new HashMap<String, PureRec>();
    FunctionMapper              m_classMapper = new FunctionMapper();

    public  void Parse(String fileName) {
        System.out.println("Analyzing KKrunchy logs ... ");
        boolean storeSize = true;
        File file = new File(fileName);
        try{
            BufferedReader reader = new BufferedReader(new FileReader(file));
            String line = null;
            while ((line = reader.readLine()) != null) {
                if(line.contains("Functions by object file and packed size"))
                    break;
                if(line.contains(".obj")) {

                    // extract obj file name
                    String[] tokens = line.split(" ");
                    String objFileName = tokens[tokens.length - 1];
                    int maxPos = line.indexOf(objFileName);
//                    System.out.println(tokens[tokens.length - 1]);

                    // extract file size
                    int endPos = line.indexOf(":");
                    int pos = endPos;
                    while(line.charAt(pos) != ' ')
                        pos--;
                    int startPos = pos + 1;
                    int functionSize = Integer.parseInt(line.substring(startPos, endPos));
//                    System.out.println(functionSize);

                    if(functionSize != 0) {
                        // extract function name
                        startPos = endPos + 1;
                        while(line.charAt(startPos) == ' ')
                            startPos++;
                        endPos = maxPos;
                        String functionName = line.substring(startPos, endPos);
//                        System.out.println(functionName);

                        {
                            String funcString = functionName.replaceAll(" ", "").replaceAll("\t", "");
                            String[] functionTokens = funcString.split("::");
                            if(functionTokens.length == 2) {
                                String className = functionTokens[0];
                                String methodName = functionTokens[1];
                                m_classMapper.getObjectFile(className).AddFunction(methodName, functionSize);
//                                System.out.println("Adding [" + className + "]::[" + functionName + "] " + functionSize);
                            }
                        }

                        m_mapper.getObjectFile(objFileName).AddFunction(functionName, functionSize);
                        m_pureRecs.add(new FunctionRecord(functionName, objFileName, functionSize));
                        if(storeSize) {
                            PureRec s = this.m_pureFunctions.get(functionName);
                            if(s == null) {
                                s = new PureRec();
                                this.m_pureFunctions.put(functionName, s);
                            }
                            s.cnt++;
                            s.sizesum += functionSize;
                        }
                    }
                }
            }
            
            System.out.println("Parsing complete");
            Collections.sort(this.m_pureRecs);

            String SEPARATOR = ";   ";
            for(int r = 0; r < 2; r++) {
                String outFileName = (r == 0) ? "KKLOG_by_ObjectFiles.csv" : "KKLOG_by_Classes.csv";
                FileWriter outFile = new FileWriter(outFileName);
                PrintWriter out = new PrintWriter(outFile);

                LinkedList<ObjectFile> fileList = (r == 0) ? m_mapper.GetSorted() : m_classMapper.GetSorted();
                Iterator<ObjectFile>    fiter = fileList.iterator();
                while(fiter.hasNext()) {
                    ObjectFile f = fiter.next();
                    String outObjectFileName = f.m_name;
                    String outObjectFileSize = "" + f.m_size;
                    Iterator<FunctionRecord>    recIter = f.m_records.iterator();
                    while(recIter.hasNext()) {
                        FunctionRecord rec = recIter.next();
                        String outFunctionName = rec.m_name;
                        String outFunctionSize = "" + rec.m_size;
                        String st = outObjectFileSize + SEPARATOR + outObjectFileName + SEPARATOR + outFunctionSize + SEPARATOR + outFunctionName;
    //                    System.out.println(st);
                        out.println(st);
                    }
                }
                out.close();
            }

            FileWriter outFile = new FileWriter("KKLOG_by_Functions.csv");
            PrintWriter out = new PrintWriter(outFile);
            Iterator<FunctionRecord>    recIter = this.m_pureRecs.iterator();
            while(recIter.hasNext()) {
                FunctionRecord rec = recIter.next();
                String st = rec.m_size + SEPARATOR + rec.m_name + SEPARATOR + this.m_mapper.getObjectFile(rec.m_objectFile).m_size  + SEPARATOR + rec.m_objectFile;
//                System.out.println(st);
                out.println(st);
            }
            out.close();

            LinkedList<FunctionRecord>  m_singleRecs = new LinkedList<FunctionRecord>();
            Iterator<Map.Entry<String, PureRec>> iter = this.m_pureFunctions.entrySet().iterator();
            while(iter.hasNext()) {
                Map.Entry<String, PureRec> e = iter.next();
                m_singleRecs.add(new FunctionRecord(e.getKey(), e.getValue().cnt + " refs", e.getValue().sizesum));
            }
            Collections.sort(m_singleRecs);
            outFile = new FileWriter("KKLOG_by_Functions_global.csv");
            out = new PrintWriter(outFile);
            recIter = m_singleRecs.iterator();
            while(recIter.hasNext()) {
                FunctionRecord rec = recIter.next();
                String st = rec.m_size + SEPARATOR + rec.m_name + SEPARATOR + this.m_mapper.getObjectFile(rec.m_objectFile).m_size  + SEPARATOR + rec.m_objectFile;
//                System.out.println(st);
                out.println(st);
            }
            out.close();
            

        } catch (Exception e) {
            e.printStackTrace();
        }


        System.out.println("KKrunchy log analyzation written !");
    }
}
