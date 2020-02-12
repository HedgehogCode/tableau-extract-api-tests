import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.util.UUID;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import com.tableausoftware.TableauException;
import com.tableausoftware.common.Collation;
import com.tableausoftware.common.Type;
import com.tableausoftware.hyperextract.Extract;
import com.tableausoftware.hyperextract.ExtractAPI;
import com.tableausoftware.hyperextract.Row;
import com.tableausoftware.hyperextract.Table;
import com.tableausoftware.hyperextract.TableDefinition;

public class StresstestExtractAPI {

	private static final int NUM_RUNNERS = 100;

	private static final int NUM_ROWS = 100;

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
			futures[i] = executor.submit(new ExtractWriter(destination));
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

	/** Write a hyper extract to the given file */
	private static void writeHyperFile(final String destination) throws TableauException {

		// Create the table definition
		final TableDefinition td = new TableDefinition();
		td.setDefaultCollation(Collation.EN_US);
		td.addColumn("col", Type.UNICODE_STRING);

		// Create the extract
		final Extract extract = new Extract(destination);
		final Table table = extract.addTable("table", td);

		// Write data
		for (int i = 0; i < NUM_ROWS; i++) {
			final Row row = new Row(td);
			row.setString(0, "My string " + i);
			table.insert(row);
		}

		// Close the extract
		extract.close();
	}

	private static class ExtractWriter implements Runnable {
		private final String destination;

		public ExtractWriter(final String destination) {
			this.destination = destination;
		}

		@Override
		public void run() {
			try {
				writeHyperFile(destination);
			} catch (final TableauException e) {
				throw new RuntimeException(e);
			}
		}
	}
}
