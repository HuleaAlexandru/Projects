import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;

class CompareTask {
	
	String filename1;
	String filename2;
	HashMap<String, Integer>  hash1;
	HashMap<String, Integer>  hash2;
	ArrayList<HashMap<String, Integer> > hashes;
	
	public CompareTask(String filename1, String filename2, HashMap<String, Integer> hash1, HashMap<String, Integer> hash2) {
		this.filename1 = filename1;
		this.filename2 = filename2;
		this.hash1 = hash1;
		this.hash2 = hash2;
	}
	
	public long totalWords(HashMap<String, Integer>  hash) {
		long result = 0;
		for (java.util.Map.Entry<String, Integer> e : hash.entrySet()) {
			result += e.getValue();
		}
		return result;
	}
	
	public double f(String word, HashMap<String, Integer> hash, long total) {
		if (hash.containsKey(word)) {
			return hash.get(word)*100./total;
		}
		else {
			return 0;
		}
	} 
	
	public Result sim() {
		double sum = 0;
		long  total1 = totalWords(hash1);
		long total2 = totalWords(hash2);
		
		for (java.util.Map.Entry<String, Integer> e : hash1.entrySet()) {
			sum += (f(e.getKey(), hash1, total1) * (f(e.getKey(), hash2, total2)));
		}
		return new Result(filename1, filename2, sum *1./ 100);
	}
}

class CompareWorker extends Thread {
	CompareWorkPool wp;
	
	public CompareWorker(CompareWorkPool workpool) {
		this.wp = workpool;
	}
	
	public Result execute(CompareTask task) {
		return task.sim();
	}
	
	public void run() {
		while (true) {
			CompareTask task = wp.getWork();
			if (task == null) {
				break;
			}
			this.wp.addResult(this.execute(task));
		}
	}
}

public class CompareWorkPool {
	int nrThreads;
	int nrWaiting;
	boolean end; 
	
	ArrayList<Result> results;
	ArrayList<String> files;
	LinkedList<CompareTask> compareTasks;
	
	public CompareWorkPool(int nrThreads, ArrayList<String> files) {
		this.nrThreads = nrThreads;
		this.nrWaiting = 0;
		this.end = false;
		this.results = new ArrayList<Result>();
		this.files = files;
		this.compareTasks = new LinkedList<CompareTask>();
	}
	
	public synchronized CompareTask getWork() { 
		
		if (this.compareTasks.size() == 0) {
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
		return compareTasks.remove();
	}

	public void putWork(CompareTask task) {
		this.compareTasks.add(task);
	}	
	
	public synchronized void addResult(Result result) {
		this.results.add(result);
	}
}
