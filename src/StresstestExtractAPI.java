import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.util.UUID;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import com.tableausoftware.TableauException;
import com.tableausoftware.hyperextract.ExtractAPI;

public class StresstestExtractAPI {

	private static final int NUM_RUNNERS = 100;

	private StresstestExtractAPI() {
	}

	public static void main(String[] args) throws IOException, InterruptedException, TableauException {
		System.out.println(System.getenv().get("LD_LIBRARY_PATH"));
		ExtractAPI.initialize();
		stressTest();
	}

	private static void stressTest() throws IOException, InterruptedException {
		final String tmpDir = Files.createTempDirectory("tableau-test-" + UUID.randomUUID().toString()).toAbsolutePath()
				.toString();
		System.out.println("Temp dir is " + tmpDir);
		final ExecutorService executor = Executors.newFixedThreadPool(NUM_RUNNERS);

		// Run many writers
		final Future[] futures = new Future[NUM_RUNNERS];
		for (int i = 0; i < NUM_RUNNERS; i++) {
			final String destination = tmpDir + File.pathSeparator + "extract" + i + ".hyper";
			futures[i] = executor.submit(new Utils.ExtractWriter(destination));
		}

		// Get the results
		for (int i = 0; i < NUM_RUNNERS; i++) {
			try {
				futures[i].get();
			} catch (final ExecutionException e) {
				System.out.println("Got execution exception:");
				e.printStackTrace();
			}
		}

		executor.shutdownNow();
	}

}
