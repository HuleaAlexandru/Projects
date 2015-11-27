public class Result implements Comparable<Result> {
	String file1;
	String file2;
	double similarity;
	
	Result(String file1, String file2, double similarity) {
		this.file1 = file1;
		this.file2 = file2;
		this.similarity = similarity;
	}

	@Override
	public int compareTo(Result result) {
		double res = result.similarity - this.similarity;
		if(res >= 0) {
			return 1; 
		}
		else {
			return -1;
		}
	}
}
