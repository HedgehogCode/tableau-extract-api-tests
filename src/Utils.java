import com.tableausoftware.TableauException;
import com.tableausoftware.common.Collation;
import com.tableausoftware.common.Type;
import com.tableausoftware.hyperextract.Extract;
import com.tableausoftware.hyperextract.Row;
import com.tableausoftware.hyperextract.Table;
import com.tableausoftware.hyperextract.TableDefinition;

class Utils {

	private static final int NUM_ROWS = 100;

	private Utils() {
	}

	/** Write a hyper extract to the given file */
	static void writeHyperFile(final String destination) throws TableauException {

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

	static class ExtractWriter implements Runnable {
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
