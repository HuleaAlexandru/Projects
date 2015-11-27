import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;

class ReduceTask {
	
	String numeDocument;
	ArrayList<HashMap<String, Integer> > hashes;
	
	public ReduceTask(String numeDocument, ArrayList<HashMap<String, Integer> > hashes) {
		this.numeDocument = numeDocument;
		this.hashes = hashes;
	}
	
	public HashMap<String, Integer> merge() {
		for (int i = 1; i < hashes.size(); i++) {
			HashMap<String, Integer> newHash = new HashMap<String, Integer>();
			for (java.util.Map.Entry<String, Integer> e : hashes.get(0).entrySet()) {
				String cheie = e.getKey();
				Integer valoare = e.getValue();
				if (hashes.get(i).containsKey(e.getKey())) {
					newHash.put(cheie, valoare +  hashes.get(i).get(cheie));
				}
				else {
					newHash.put(cheie, valoare);
				}
			}
			
			for (java.util.Map.Entry<String, Integer> e : hashes.get(i).entrySet()) {
				String cheie = e.getKey();
				if (!hashes.get(0).containsKey(cheie)) {
					newHash.put(cheie, e.getValue());
				}
			}
			
			hashes.set(0, newHash);
		}
		return hashes.get(0);
	} 
}

class ReduceWorker extends Thread {
	ReduceWorkPool wp;
	
	public ReduceWorker(ReduceWorkPool workpool) {
		this.wp = workpool;
	}
	
	public HashMap<String, Integer> processTask(ReduceTask task) {
		return task.merge();
	}
	
	public void run() {
		while (true) {
			ReduceTask task = wp.getWork();
			if (task == null) {
				break;
			}
			HashMap<String, Integer> hash = processTask(task);
			this.wp.putResult(task.numeDocument, hash);
		}
	}
}

public class ReduceWorkPool {
	int nrThreads;
	int nrWaiting;
	boolean end; 
	
	String filename;
	LinkedList<ReduceTask> reduceTasks;
	HashMap <String, ArrayList<HashMap<String, Integer> > > reduceHashes;
	HashMap <String, HashMap<String, Integer> > hashes;
	
	public ReduceWorkPool(int nrThreads, HashMap<String, ArrayList<HashMap<String, Integer> > > reduceHashes) {
		this.nrThreads = nrThreads;
		this.nrWaiting = 0;
		this.end = false;
		this.reduceTasks = new LinkedList<ReduceTask>();
		this.reduceHashes = reduceHashes;
	}

	public synchronized ReduceTask getWork() { 
		
		if (this.reduceTasks.size() == 0) {
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
		return reduceTasks.remove();
	}

	public void putWork(ReduceTask task) {
		this.reduceTasks.add(task);
	}
	
	public synchronized void putResult(String filename, HashMap<String, Integer> hash) {
		ArrayList<HashMap<String, Integer> > array = new ArrayList<HashMap<String,Integer> >();
		array.add(hash);
		this.reduceHashes.put(filename, array);
	}
}
