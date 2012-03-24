/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package eplayercppcollector;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;

/**
 *
 * @author Pap
 */
public class Main {

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        HashMap<String, String> hm = new HashMap<String, String>();
        LinkedList<String> classes = new LinkedList<String>();
        // TODO code application logic here
        File file = new File("productionsources.gen");
        try{
            BufferedReader reader = new BufferedReader(new FileReader(file));
            String line = null;
            while ((line = reader.readLine()) != null) {
                String[] tokens = line.split(";");
                classes.add(tokens[0]);
                String cppFile = tokens[1];
                if(hm.get(cppFile) == null) {
                    hm.put(cppFile, cppFile);
                }
            }

            FileWriter outFile = new FileWriter("../code/eplayer3/production.cpp");
//            FileWriter outFile = new FileWriter("production.cpp");
            PrintWriter out = new PrintWriter(outFile);

            out.write("// ******************************************\r\n");
            out.write("// *** GENERATED FROM SOURCE FILES **********\r\n");
            out.write("// ******************************************\r\n");
            out.write("#include \"production.hpp\"\r\n\r\n");
            Iterator<String> fiter = hm.values().iterator();
            while(fiter.hasNext()) {
                String cppFileName = fiter.next();
                while(-1 != cppFileName.indexOf('\\'))
                    cppFileName = cppFileName.replace('\\', '/');

                File cppf = new File(cppFileName);

                BufferedReader r = new BufferedReader(new FileReader(cppFileName));
                String s = null;
                while ((s = r.readLine()) != null) {
                    if(s.startsWith("#include") && s.contains("\"")) {
                        String lineTokens[] = s.split("\"");
                        String path = lineTokens[1];
                        while(-1 != path.indexOf('\\'))
                            path = path.replace('\\', '/');
                        String pathTokens[] = path.split("/");
                        int numUp = 0;
                        for(int i = 0; i < pathTokens.length; i++)
                            if("..".equals(pathTokens[i]))
                                numUp++;
                        String fullPathTokens[] = cppFileName.split("/");
                        String finalPath = "";
                        for(int i = 0; i < fullPathTokens.length - numUp - 1; i++)
                            finalPath += fullPathTokens[i] + "/";
                        for(int i = 0; i < pathTokens.length; i++)
                            if(!"..".equals(pathTokens[i]))
                                finalPath += pathTokens[i] + ((i == pathTokens.length - 1) ? "" : "/");
                        s = "#include \"" + finalPath + "\"";
                    }
                    out.write(s + "\r\n");
                }
                r.close();
                out.write("\r\n");
            }

            // write final constructor code
            out.write("eIOperator* SCRIPT_OP_CREATOR::createOp(unsigned int nr) {\r\n");
            out.write("        switch(nr) {\r\n");
            int nr = 0;
            Iterator<String> iter = classes.iterator();
            while(iter.hasNext())
                out.write("         case " + (nr++) + ": return new " + iter.next() + "();\r\n");
            out.write("         default: eASSERT(eFALSE);\r\n");
            out.write("        }\r\n");
            out.write("};\r\n");
            out.close();
        } catch (Exception e) {
            System.out.println("EROOR ");
            e.printStackTrace();
        }
    }

}
