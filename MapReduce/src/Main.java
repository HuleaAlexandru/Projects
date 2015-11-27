import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Scanner;

public class Main {
	
	public static boolean esteSeparator(char c) {
		boolean result = false;
		switch(c) {
			case ';':
			case ':':
			case '?':
			case '~':
			case '\\':
			case '.':
			case ',':
			case '>':
			case '<':
			case '`':
			case '[':
			case ']':
			case '{':
			case '}':
			case '(':
			case ')':
			case '!':
			case '@':
			case '#':
			case '$':
			case '%':
			case '^':
			case '&':
			case '-':
			case '_':
			case '+':
			case '\'':
			case '=':
			case '*':
			case '\"':
			case '|':
			case ' ':
			case '\t':
			case '\n':
				result = true;
				break;
		}
	return result;
	}
	
	public static void main(String args[]) throws IOException {
		
		int NT = Integer.parseInt(args[0]); // nr de threaduri
		String in = args[1];
		String out = args[2];
		
		Scanner scanner = new Scanner(new File(in));
		
		int D = scanner.nextInt();	// dimensiunea fragmentelor
		double X = Double.parseDouble(scanner.next()); // pragul de similaritate
		int ND = scanner.nextInt(); // numarul de documente
		ArrayList<String> files = new ArrayList<String>(); // numele documentelor
		for (int i = 0; i < ND; i++) {
			files.add(scanner.next());
		}
		
		scanner.close();
		
		HashMap<String, ArrayList<HashMap<String, Integer> > > mapHashes;
		mapHashes = new HashMap<String, ArrayList<HashMap<String,Integer> > >();
		for (int i = 0; i < files.size(); i++) {
			mapHashes.put(files.get(i), new ArrayList<HashMap<String,Integer> >());
		}
		
		// Map part
		MapWorkPool wp = new MapWorkPool(NT, mapHashes);
		
		for(int i = 0; i < ND; i++) {
			File file = new File(files.get(i));
			long length = file.length();
			int j;
			for (j = 0; j < length / D; j++) {
				wp.putWork(new MapTask(files.get(i), D*j, D, length));
			}
			if (length % D != 0) {
				wp.putWork(new MapTask(files.get(i), D*j, (int)(length - D*j), length));
			}
		}
		
		MapWorker[] workers = new MapWorker[NT];
		
		for (int i = 0; i < NT; i ++) {
			workers[i] = new MapWorker(wp);
		}
		
		for (int i = 0; i < NT; i ++) {
			workers[i].start();
		}
		
		for (int i = 0; i < NT; i ++) {
			try {
				workers[i].join();
			}
			catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
		
		// Reduce part
		ReduceWorkPool rwp = new ReduceWorkPool(NT, mapHashes);
		for (int i = 0; i < ND; i++) {
			String filename = files.get(i);
			rwp.putWork(new ReduceTask(filename, mapHashes.get(filename)));
		}
		
		ReduceWorker[] reduceWorkers = new ReduceWorker[NT];
		
		for (int i = 0; i < NT; i ++) {
			reduceWorkers[i] = new ReduceWorker(rwp);
		}
		
		for (int i = 0; i < NT; i ++) {
			reduceWorkers[i].start();
		}
		
		for (int i = 0; i < NT; i ++) {
			try {
				reduceWorkers[i].join();
			}
			catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
		
		//Compare part
		CompareWorkPool cwp = new CompareWorkPool(NT, files);
		for (int i = 0; i < ND - 1; i++) {
			for (int j = i + 1; j < ND; j++) {
				String filename1 = files.get(i);
				String filename2 = files.get(j);
				cwp.putWork(new CompareTask(filename1, filename2, mapHashes.get(filename1).get(0), 
						mapHashes.get(filename2).get(0)));
			}
		}
		
		CompareWorker[] compareWorkers = new CompareWorker[NT];
		
		for (int i = 0; i < NT; i ++) {
			compareWorkers[i] = new CompareWorker(cwp);
		}
		
		for (int i = 0; i < NT; i ++) {
			compareWorkers[i].start();
		}
		
		for (int i = 0; i < NT; i ++) {
			try {
				compareWorkers[i].join();
			}
			catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
		
		//Sort part
		Collections.sort(cwp.results);
		
		FileWriter writer = new FileWriter(new File(out));
		for (int i = 0; cwp.results.size() > i && cwp.results.get(i).similarity > X ; i++) {
			int sim = (int)(cwp.results.get(i).similarity*10000);
			writer.write(cwp.results.get(i).file1 + ";" + cwp.results.get(i).file2 + ";" + sim*1./10000  + "\n");
		}
		writer.close();
	}
}