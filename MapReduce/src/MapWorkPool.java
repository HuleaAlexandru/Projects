import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;

class MapTask {
	
	String numeDocument;
	int offset;
	int dimFrag;
	long length;
	
	public MapTask(String numeDocument, int offset,int dimFrag, long length) {
		this.numeDocument = numeDocument;
		this.offset = offset;
		this.dimFrag = dimFrag;
		this.length = length;
	}
	
	
	public ArrayList<String> read() throws IOException {
		ArrayList<String> cuvinte = new ArrayList<String>();
		
		File file = new File(this.numeDocument);
        RandomAccessFile raf = new RandomAccessFile(file, "r");
		int charRead = 0;
		
		if (offset != 0) {
			raf.seek(offset - 1);
			
			char c = (char) raf.readByte();
			if (!Main.esteSeparator(c)) {
				charRead = -1;
				while (!Main.esteSeparator(c) && charRead < dimFrag){
					c = (char) raf.readByte();
					charRead++;
				}
			}
		}
		else {
			raf.seek(0);
		}
	
		byte[] fragment = new byte[this.dimFrag - charRead];
		raf.read(fragment);
		
		StringBuffer cuvant = new StringBuffer("");
		for (int i = 0; i < this.dimFrag - charRead; i++) {
			char c = (char)fragment[i];
			if (!Main.esteSeparator(c)) {
				cuvant.append(c);
			}
			else {
				String cuv = new String(cuvant);
				if (!cuv.equals("")) {
					cuvinte.add(cuv.toLowerCase());
				}
				cuvant = new StringBuffer("");
			}
		}
		char ch = (char)fragment[this.dimFrag - charRead - 1];
		if (!Main.esteSeparator(ch)) {
			while(!Main.esteSeparator(ch) && raf.getFilePointer() < this.length) {
				ch = (char)raf.readByte();
				if (!Main.esteSeparator(ch)) 
					cuvant.append(ch);
			}
		}
		String cuv = new String(cuvant);
		if (!cuv.equals("")) {
			cuvinte.add(cuv.toLowerCase());
		}
		
		raf.close();
		return cuvinte;
	}
}

class MapWorker extends Thread {
	MapWorkPool wp;
	HashMap<String, Integer> map;

	public MapWorker(MapWorkPool workpool) {
		this.wp = workpool;
		this.map = new HashMap<String, Integer>();
	}

	void processTask(MapTask task) throws IOException {
		ArrayList<String> cuvinte = task.read();
		for (String cuvant : cuvinte) {
			if(this.map.containsKey(cuvant)) {
				int count = this.map.get(cuvant) + 1;
				//this.map.remove(cuvant);
				this.map.put(cuvant, count);
			}
			else {
				this.map.put(cuvant, 1);
			}
		}
	}
	
	public void run() {
		while (true) {
			MapTask task = wp.getWork();
			if (task == null) {
				break;
			}
			try {
				processTask(task);
				this.wp.addMap(task.numeDocument, this.map);
				this.map = new HashMap<String, Integer>();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}
}

public class MapWorkPool {
	int nrThreads;
	int nrWaiting;
	boolean end; 
	
	LinkedList<MapTask> mapTasks;
	HashMap<String, ArrayList<HashMap<String, Integer> > > mapHashes;
	
	public MapWorkPool(int nrThreads, HashMap<String, ArrayList<HashMap<String, Integer> > > mapHashes) {
		this.nrThreads = nrThreads;
		this.nrWaiting = 0;
		this.end = false;
		this.mapTasks = new LinkedList<MapTask>();
		this.mapHashes = mapHashes;
	}

	public synchronized MapTask getWork() { 
		
		if (this.mapTasks.size() == 0) {
			this.nrWaiting++;
			
			if (this.nrWaiting == this.nrThreads) {
				this.end = true;
				notifyAll();
				return null;
				
			} else {
				while (!this.end) {
					try {
						this.wait();
					} catch(Exception e) {e.printStackTrace();}
				}
				return null;
			}
		}
		return mapTasks.remove();
	}

	public void putWork(MapTask task) {
		this.mapTasks.add(task);
	}
	
	public synchronized void addMap(String filename, HashMap<String, Integer> hash) {
		ArrayList<HashMap<String , Integer> > array = this.mapHashes.get(filename);
		array.add(hash);
		this.mapHashes.put(filename, array);
	}
}
