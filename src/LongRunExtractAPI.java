import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.UUID;

import com.tableausoftware.TableauException;
import com.tableausoftware.hyperextract.ExtractAPI;

public class LongRunExtractAPI {

	private LongRunExtractAPI() {
	}

	public static void main(String[] args) throws TableauException, IOException {
		System.out.println(System.getenv().get("LD_LIBRARY_PATH"));
		longRun();
	}

	private static void longRun() throws TableauException, IOException {
		final String tmpDir = Files.createTempDirectory("tableau-test-" + UUID.randomUUID().toString()).toAbsolutePath()
				.toString();
		final String destination = tmpDir + File.pathSeparator + "extract.hyper";
		long filesWritten = 0;
		while (true) {
			ExtractAPI.initialize();
			Utils.writeHyperFile(destination);
			Files.delete(Paths.get(destination));
			ExtractAPI.cleanup();

			filesWritten++;
			System.out.println("File written: " + filesWritten);
		}
	}
}
