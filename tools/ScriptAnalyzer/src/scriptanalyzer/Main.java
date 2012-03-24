/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package scriptanalyzer;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;

/**
 *
 * @author pap
 */
public class Main {

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        // TODO code application logic here
        HashMap<String, Operator> opMap = new HashMap<String, Operator>();
        HashMap<String, Parameter> paramMap = new HashMap<String, Parameter>();
        String fileName = "paramout.txt";
        int gsize = 0;
        File file = new File(fileName);
        try{
            BufferedReader reader = new BufferedReader(new FileReader(file));
            String line = null;
            while ((line = reader.readLine()) != null) {
                String tokens[] = line.split(";");
                DataSample d = new DataSample();
                d.optype = tokens[0].split("=")[1];
                d.paramName = tokens[1].split("=")[1];
                d.byteSize = Integer.parseInt(tokens[2].split("=")[1]);
                Operator op = opMap.get(d.optype);
                if(op == null) {
                    op = new Operator();
                    op.name = d.optype;
                    opMap.put(op.name, op);
                }
                op.AddSample(d);
                String paramID = "op" + d.optype + "::" + d.paramName;
                Parameter p = paramMap.get(paramID);
                if(p == null) {
                    p = new Parameter();
                    p.name = paramID;
                    paramMap.put(paramID, p);
                }
                p.AddSample(d);
                gsize += d.byteSize;
            }

            LinkedList<Operator> opList = new LinkedList<Operator>();
            Iterator<Operator> it = opMap.values().iterator();
            while(it.hasNext()) {
                Operator op = it.next();
                opList.add(op);
            }
            Collections.sort(opList);

            LinkedList<Parameter> paramList = new LinkedList<Parameter>();
            Iterator<Parameter> itp = paramMap.values().iterator();
            while(itp.hasNext()) {
                Parameter p = itp.next();
                paramList.add(p);
            }
            Collections.sort(paramList);

            FileWriter outFile = new FileWriter("ParamLog_by_Optype.csv");
            PrintWriter out = new PrintWriter(outFile);
            String SEPARATOR = ";   ";
            it = opList.descendingIterator();
            while(it.hasNext()) {
                Operator op = it.next();
                out.println("Operator " + op.name + SEPARATOR + "TotalSize = " + op.size + SEPARATOR + " multiplicity = " + op.samples.size());
            }
            out.close();

            outFile = new FileWriter("ParamLog_by_Param.csv");
            out = new PrintWriter(outFile);
            itp = paramList.descendingIterator();
            while(itp.hasNext()) {
                Parameter p = itp.next();
                out.println(p.name + SEPARATOR + "TotalSize = " + p.size + SEPARATOR + " multiplicity = " + p.samples.size());
            }
            out.close();

        } catch (Exception e) {
            e.printStackTrace();
        }

    }

}
