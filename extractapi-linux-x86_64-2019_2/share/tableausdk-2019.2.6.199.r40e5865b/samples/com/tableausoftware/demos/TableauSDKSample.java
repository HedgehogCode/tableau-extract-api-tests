/**
 * -----------------------------------------------------------------------------
 *
 *  This file is the copyrighted property of Tableau Software and is protected
 *  by registered patents and other applicable U.S. and international laws and
 *  regulations.
 *
 *  Unlicensed use of the contents of this file is prohibited. Please refer to
 *  the NOTICES.txt file for further details.
 *
 * -----------------------------------------------------------------------------
 */
package com.tableausoftware.demos;

import java.util.HashMap;

import com.tableausoftware.TableauException;
import com.tableausoftware.common.*;
import com.tableausoftware.hyperextract.*;

public final class TableauSDKSample {
    /**
     * -------------------------------------------------------------------------
     *  Display Usage
     * -------------------------------------------------------------------------
     */
    private static void displayUsage() {
        System.err.print(""
            + "A simple demonstration of the Tableau Extract API.\n"
            + "\n"
            + "USAGE: tableauSDKSample-c [COMMAND] [OPTIONS]\n"
            + "\n"
            + "COMMANDS:"
            + "\n"
            + " -h, --help            Show this help message and exit\n"
            + " -b, --build           If an extract named FILENAME exists in the current directory,\n"
            + "                       extend it with sample data.\n"
            + "                       If no Tableau extract named FILENAME exists in the current directory,\n"
            + "                       create one and populate it with sample data.\n"
            + "                       (default=False)\n"
            + "\n"
            + "OPTIONS:\n"
            + " -s, --spatial         Include spatial data when creating a new extract.\n"
            + "                       If an extract is being extended, this argument is ignored.\n"
            + "                       (default=False)\n"
            + "\n"
            + " -m, --multitable      Create multi-table extract.\n"
            + "                       Note that multi-table storage for extracts is supported starting with Tableau 2018.3.\n"
            + "                       (default=False)\n"
            + "\n"
            + " -f FILENAME, --filename FILENAME\n"
            + "                       FILENAME of the extract to be created or extended.\n"
            + "                       (default='order-c.hyper')\n"
            + "\n" );
    }


    /**
     * -------------------------------------------------------------------------
     *  Parse Arguments
     * -------------------------------------------------------------------------
     *  (NOTE: This function returns 'null' if there are any invalid arguments)
     */
    private static HashMap<String, String> parseArguments(
        String[] arguments
    )
    {
        if ( arguments.length == 0 ) {
            return null;
        }

        HashMap<String, String> options = new HashMap<String, String>();
        for ( int i = 0; i < arguments.length; ++i ) {
            switch( arguments[ i ] ) {
            case "-h":
            case "--help":
                options.put( "help", "true" );
                return options;
            case "-b":
            case "--build":
                options.put( "build", "true" );
                break;
            case "-s":
            case "--spatial":
                options.put( "spatial", "true" );
                break;
            case "-m":
            case "--multitable":
                options.put( "multitable", "true" );
                break;
            case "-f":
            case "--filename":
                if ( i + 1 >= arguments.length ) {
                    return null;
                }
                options.put( "filename", arguments[ ++i ] );
                break;
            default:
                return null;
            }
        }

        //  Defaults
        //  options.put( "build" ) = false;
        //  options.put( "spatial" ) = false;
        if ( !options.containsKey( "filename" ) ) {
            options.put( "filename", "order-java.hyper" );
        }

        return options;
    }

    /**
     * -------------------------------------------------------------------------
     *  Create or Open Extract
     * -------------------------------------------------------------------------
     *  (NOTE: This function assumes that the Tableau SDK Extract API is initialized)
     */
    private static Extract createOrOpenExtract(
        String filename,
        boolean useSpatial,
        boolean createMultipleTables
    )
    {
        Extract extract = null;
        try {
            //  Create Extract Object
            //  (NOTE: TabExtractCreate() opens an existing extract with the given
            //   filename if one exists or creates a new extract with the given filename
            //   if one does not)
            extract = new Extract( filename );

            //  Define `Extract`/`Products` table (If we are creating a new extract)
            String tableName = createMultipleTables ? "Products" : "Extract";
            if ( !extract.hasTable( tableName ) ) {
                TableDefinition schema = new TableDefinition();
                // NOTE: Collations make string operations very expensive and this may
                // slow down the overall workbook performance, so use them with care.
                schema.setDefaultCollation( Collation.BINARY );
                schema.addColumn( "Purchased",       Type.DATETIME );
                schema.addColumn( "Product",         Type.CHAR_STRING );
                schema.addColumn( "uProduct",        Type.UNICODE_STRING );
                schema.addColumn( "Price",           Type.DOUBLE );
                schema.addColumn( "Quantity",        Type.INTEGER );
                schema.addColumn( "Taxed",           Type.BOOLEAN );
                schema.addColumn( "Expiration Date", Type.DATE );
                schema.addColumnWithCollation( "Produkt", Type.CHAR_STRING, Collation.DE );
                if ( useSpatial ) {
                    schema.addColumn( "Destination", Type.SPATIAL );
                }
                if ( createMultipleTables ) {
                    schema.addColumn( "SupplierKey",     Type.INTEGER );
                }
                Table table = extract.addTable( tableName, schema );
                if ( table == null ) {
                    System.err.println( "A fatal error occured while creating the table" );
                    System.err.println( "Exiting now." );
                    System.exit( -1 );
                }
            }

            //  Define `Suppliers` table (If we are creating a new multi-table extract)
            if ( createMultipleTables && !extract.hasTable( "Suppliers" ) ) {
                TableDefinition schema = new TableDefinition();
                schema.addColumn( "SupplierKey",     Type.INTEGER );
                schema.addColumn( "Supplier",        Type.CHAR_STRING );
                schema.addColumn( "Address",         Type.CHAR_STRING );
                Table table = extract.addTable( "Suppliers", schema );
                if ( table == null ) {
                    System.err.println( "A fatal error occured while creating the `Suppliers` table" );
                    System.err.println( "Exiting now." );
                    System.exit( -1 );
                }
            }
        }
        catch ( TableauException e ) {
            System.err.println( "A fatal error occurred while creating the extract:" );
            System.err.println( e.getMessage() );
            System.err.println( "Printing stack trace now:" );
            e.printStackTrace( System.err );
            System.err.println( "Exiting now." );
            System.exit( -1 );
        }
        catch ( Throwable t ) {
            System.err.println( "An unknown error occured while creating the extract" );
            System.err.println( "Printing stack trace now:" );
            t.printStackTrace( System.err );
            System.err.println( "Exiting now." );
            System.exit( -1 );
        }

        return extract;
    }

    /**
     * -------------------------------------------------------------------------
     *  Populate Extract
     * -------------------------------------------------------------------------
     *  (NOTE: This function assumes that the Tableau SDK Extract API is initialized)
     */
    private static void populateExtract(
        Extract extract,
        boolean useSpatial,
        boolean createMultipleTables
    )
    {
        try {
            // Populate `Extract`/`Products` table
            {
                //  Get Schema
                String tableName = createMultipleTables ? "Products" : "Extract";
                Table table = extract.openTable( tableName );
                TableDefinition tableDef = table.getTableDefinition();

                //  Insert Data
                Row row = new Row( tableDef );
                row.setDateTime( 0, 2012, 7, 3, 11, 40, 12, 4550 ); //  Purchased
                row.setCharString( 1, "Beans" );                    //  Product
                row.setString( 2, "uniBeans" );                     //  Unicode Product
                row.setDouble( 3, 1.08 );                           //  Price
                row.setDate( 6, 2029, 1, 1 );                       //  Expiration Date
                row.setCharString( 7, "Bohnen" );                   //  Produkt
                if ( useSpatial ) {
                    row.setSpatial( 8, "POINT (30 10)" );           //  Destination
                }
                for ( int i = 0; i < 10; ++i  ) {
                    row.setInteger( 4, i * 10 );                    //  Quantity
                    row.setBoolean( 5, i % 2 == 1 );                //  Taxed
                    if ( createMultipleTables ) {
                        row.setInteger( useSpatial ? 9 : 8, i % 3 ); //  SupplierKey
                    }
                    table.insert( row );
                }
            }

            // Populate `Suppliers` table
            if (createMultipleTables) {
                //  Get Schema
                Table table = extract.openTable( "Suppliers" );
                TableDefinition tableDef = table.getTableDefinition();

                //  Insert Data
                Row row = new Row( tableDef );
                row.setCharString( 1, "Bean Supplier" );            //  Supplier
                row.setCharString( 2, "42 Bean Street, Beantown" ); //  Address
                for ( int i = 0; i < 3; ++i  ) {
                    row.setInteger( 0, i );                         //  SupplierKey
                    table.insert( row );
                }
            }
        }
        catch ( TableauException e ) {
            System.err.println( "A fatal error occurred while populating the extract:" );
            System.err.println( e.getMessage() );
            System.err.println( "Printing stack trace now:" );
            e.printStackTrace( System.err );
            System.err.println( "Exiting now." );
            System.exit( -1 );
            }
        catch ( Throwable t ) {
            System.err.println( "An unknown error occured while populating the extract" );
            System.err.println( "Printing stack trace now:" );
            t.printStackTrace( System.err );
            System.err.println( "Exiting now." );
            System.exit( -1 );
        }
    }

    /**
     * -------------------------------------------------------------------------
     *  Main
     * -------------------------------------------------------------------------
     */
    public static void main( String[] arguments ) {
        //  Parse Arguments
        HashMap<String, String> options = parseArguments( arguments );
        if ( options == null || options.containsKey( "help" ) ) {
            displayUsage();
            return;
        }

        //  Extract API Demo
        if ( options.containsKey( "build" ) ) {
            try {
                //  Initialize the Tableau Extract API
                ExtractAPI.initialize();

                //  Create or Expand the Extract
                Extract extract = createOrOpenExtract( options.get( "filename" ), options.containsKey( "spatial" ), options.containsKey( "multitable" ) );
                populateExtract( extract, options.containsKey( "spatial" ), options.containsKey( "multitable" ) );

                //  Flush the Extract to Disk
                extract.close();

                // Close the Tableau Extract API
                ExtractAPI.cleanup();
            }
            catch ( TableauException e ) {
                System.err.println( "A fatal error occurred while opening or closing the Extract API:" );
                System.err.println( e.getMessage() );
                System.err.println( "Printing stack trace now:" );
                e.printStackTrace( System.err );
                System.err.println( "Exiting now." );
                System.exit( -1 );
            }
            catch ( Throwable t ) {
                System.err.println( "An unknown error occured while opening or closing the Extract API:" );
                System.err.println( "Printing stack trace now:" );
                t.printStackTrace( System.err );
                System.err.println( "Exiting now." );
                System.exit( -1 );
            }
        }
    }
}
