//------------------------------------------------------------------------------
//
//  This file is the copyrighted property of Tableau Software and is protected
//  by registered patents and other applicable U.S. and international laws and
//  regulations.
//
//  Unlicensed use of the contents of this file is prohibited. Please refer to
//  the NOTICES.txt file for further details.
//
//  NOTE: This sample requires a C99 or higher compiler, i.e. Microsoft Visual
//  C compiler 2013 and above, and GCC with C99.
//
//------------------------------------------------------------------------------
#if defined(__APPLE__) && defined(__MACH__)
#include <TableauHyperExtract/TableauHyperExtract.h>
#else
#include "TableauHyperExtract.h"
#endif

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//------------------------------------------------------------------------------
//  TableauString Helpers
//------------------------------------------------------------------------------
#define CreateTableauStringConst( STR, NAME )                                  \
    static const wchar_t NAME##_str[] = STR;                                   \
    TableauWChar NAME[ sizeof( NAME##_str ) / sizeof( wchar_t ) ];             \
    ToTableauString( NAME##_str, NAME )

#if defined(_WIN32) || defined(_WIN64)
#define TO_WCHAR_FMT_STR L"%S"
#else
#define TO_WCHAR_FMT_STR L"%s"
#endif
#define CreateTableauStringOption( ARG, OPTION )                               \
    wchar_t *buf = ( wchar_t * )calloc( strlen( ARG ) + 1, sizeof( wchar_t ) );\
    OPTION = ( TableauWChar * )calloc( strlen( ARG ) + 1, sizeof( TableauWChar ) ); \
    swprintf( buf, strlen( ARG ) * sizeof( wchar_t ), TO_WCHAR_FMT_STR, ARG ); \
    ToTableauString( buf, OPTION );                                            \
    free( buf )

//------------------------------------------------------------------------------
//  Error Handling Helpers
//------------------------------------------------------------------------------
void
TryOp(
    TAB_RESULT result,
    const char *description,
    const wchar_t *message
)
{
    if ( result != TAB_RESULT_Success ) {
        fprintf( stderr,
            "A fatal error occurred in %s:\n %ls \nExiting now.\n",
             description, message );
        exit( EXIT_FAILURE );
    }
}

#define ExtractTryOp( OP )                                                     \
    TryOp( OP, __FUNCTION__, TabGetLastErrorMessage() );

//------------------------------------------------------------------------------
//  Display Usage
//------------------------------------------------------------------------------
void
DisplayUsage()
{
    fprintf( stderr,
        "A simple demonstration of the Tableau Extract API.\n"
        "\n"
        "USAGE: tableauSDKSample-c [COMMAND] [OPTIONS]\n"
        "\n"
        "COMMANDS:"
        "\n"
        " -h, --help            Show this help message and exit\n"
        " -b, --build           If an extract named FILENAME exists in the current directory,\n"
        "                       extend it with sample data.\n"
        "                       If no Tableau extract named FILENAME exists in the current directory,\n"
        "                       create one and populate it with sample data.\n"
        "                       (default=False)\n"
        "\n"
        "OPTIONS:\n"
        " -s, --spatial         Include spatial data when creating a new extract.\n"
        "                       If an extract is being extended, this argument is ignored.\n"
        "                       (default=False)\n"
        "\n"
        " -m, --multitable      Create multi-table extract.\n"
        "                       Note that multi-table storage for extracts is supported starting with Tableau 2018.3.\n"
        "                       (default=False)\n"
        "\n"
        " -f FILENAME, --filename FILENAME\n"
        "                       FILENAME of the extract to be created or extended.\n"
        "                       (default='order-c.hyper')\n"
        "\n" );
}

//------------------------------------------------------------------------------
//  Parse Arguments
//------------------------------------------------------------------------------
#define NUM_OPTIONS 5
enum OPTIONS {
    HELP = 0,
    BUILD,
    SPATIAL,
    MULTI_TABLE,
    FILENAME
};
static TableauWChar *FALSE = (TableauWChar *)0x0;
static TableauWChar *TRUE = (TableauWChar *)0x1;

//  Parse Command Line Arguments or Populate Defaults
//
//  Returns 'true' if all arguments are successfully parsed
//  Returns 'false' if there are any invalid arguments or no arguments
//  (NOTE: if 'false' is returned, 'options' may contain NULL pointers)
//  (NOTE: if ParseArguments() is called, FreeArguments() must be called on the
//   same 'options' array before the program exits)
bool ParseArguments(
    int argc,
    char *argv[],
    TableauWChar *options[ NUM_OPTIONS ]
)
{
    if ( argc == 0 ){
        return false;
    }

    for ( int i = 0; i < argc; ++i ) {
        if ( !strcmp( argv[ i ], "-h" ) || !strcmp( argv[ i ], "--help" ) ) {
            options[ HELP ] = TRUE;
            return true;
        }
        else if ( !strcmp( argv[ i ], "-b" ) || !strcmp( argv[ i ], "--build" ) ) {
            options[ BUILD ] = TRUE;
        }
        else if ( !strcmp( argv[ i ], "-s" ) || !strcmp( argv[ i ], "--spatial" ) ) {
            options[ SPATIAL ] = TRUE;
        }
        else if ( !strcmp( argv[ i ], "-m" ) || !strcmp( argv[ i ], "--multitable" ) ) {
            options[ MULTI_TABLE ] = TRUE;
        }
        else if ( !strcmp( argv[ i ], "-f" ) || !strcmp( argv[ i ], "--filename" ) ) {
            i++;
            if (i >= argc) {
                return false;
            }
            CreateTableauStringOption( argv[ i ], options[ FILENAME ] );
        }
    }

    //  Defaults
    //  options[ BUILD ] = FALSE;
    //  options[ SPATIAL ] = FALSE;
    //  options[ MULTI_TABLE ] = FALSE;
    if ( options[ FILENAME ] == NULL ) {
        CreateTableauStringOption( "order-c.hyper", options[ FILENAME ] );
    }

    return true;
}

//  Remove Previously Parsed Command Line Arguments from the Stack
void FreeOptions(
    TableauWChar **options,
    size_t numOptions
)
{
    for ( int i = 0; i < numOptions; ++i ) {
        if ( options[ i ] != FALSE && options[ i ] != TRUE ) {
            free( options[ i ] );
        }
    }
}

//------------------------------------------------------------------------------
//  Create or Open Extract
//------------------------------------------------------------------------------
//  (NOTE: This function assumes that the Tableau SDK Extract API is initialized)
void
CreateOrOpenExtract(
    TAB_HANDLE         *extractHandlePtr,
    const TableauString filename,
    const bool          useSpatial,
    bool                createMultipleTables
)
{
    //  Create Extract Object
    //  (NOTE: TabExtractCreate() opens an existing extract with the given
    //   filename if one exists or creates a new extract with the given filename
    //   if one does not)
    ExtractTryOp( TabExtractCreate( extractHandlePtr, filename ) );

    // Create `Extract`/`Products` table
    {
        TAB_HANDLE hSchema = NULL;
        TAB_HANDLE hTable = NULL;

        CreateTableauStringConst( L"Purchased", sPurchased );
        CreateTableauStringConst( L"Product", sProduct );
        CreateTableauStringConst( L"uProduct", sUProduct );
        CreateTableauStringConst( L"Price", sPrice );
        CreateTableauStringConst( L"Quantity", sQuantity );
        CreateTableauStringConst( L"Taxed", sTaxed );
        CreateTableauStringConst( L"Expiration Date", sExpirationDate );
        CreateTableauStringConst( L"Produkt", sProdukt );
        CreateTableauStringConst( L"Spatial", sSpatial );
        CreateTableauStringConst( L"SupplierKey", sSupplierKey );
        CreateTableauStringConst( L"Extract", sExtract );
        CreateTableauStringConst( L"Products", sProducts );

        //  Define Table Schema (If we are creating a new extract)
        int hasTable = 0;
        if (createMultipleTables) {
            ExtractTryOp( TabExtractHasTable( *extractHandlePtr, sProducts, &hasTable ) );
        } else {
            ExtractTryOp( TabExtractHasTable( *extractHandlePtr, sExtract, &hasTable ) );
        }
        if ( !hasTable ) {
            ExtractTryOp( TabTableDefinitionCreate( &hSchema ) );
            // NOTE: Collations make string operations very expensive and this may
            // slow down the overall workbook performance, so use them with care.
            ExtractTryOp( TabTableDefinitionSetDefaultCollation( hSchema, TAB_COLLATION_Binary) );
            ExtractTryOp( TabTableDefinitionAddColumn(  hSchema, sPurchased, TAB_TYPE_DateTime ) );
            ExtractTryOp( TabTableDefinitionAddColumn( hSchema, sProduct, TAB_TYPE_CharString ) );
            ExtractTryOp( TabTableDefinitionAddColumn( hSchema, sUProduct, TAB_TYPE_UnicodeString ) );
            ExtractTryOp( TabTableDefinitionAddColumn( hSchema, sPrice, TAB_TYPE_Double ) );
            ExtractTryOp( TabTableDefinitionAddColumn( hSchema, sQuantity, TAB_TYPE_Integer ) );
            ExtractTryOp( TabTableDefinitionAddColumn( hSchema, sTaxed, TAB_TYPE_Boolean ) );
            ExtractTryOp( TabTableDefinitionAddColumn( hSchema, sExpirationDate, TAB_TYPE_Date ) );
            ExtractTryOp( TabTableDefinitionAddColumnWithCollation( hSchema, sProdukt, TAB_TYPE_CharString, TAB_COLLATION_de ) );
            if ( useSpatial ) {
                ExtractTryOp( TabTableDefinitionAddColumn( hSchema, sSpatial, TAB_TYPE_Spatial ) );
            }
            if ( createMultipleTables ) {
                ExtractTryOp( TabTableDefinitionAddColumn( hSchema, sSupplierKey, TAB_TYPE_Integer ) );
                ExtractTryOp( TabExtractAddTable( *extractHandlePtr, sProducts, hSchema, &hTable ) );
            } else {
                ExtractTryOp( TabExtractAddTable( *extractHandlePtr, sExtract, hSchema, &hTable ) );
            }
            if ( hTable == NULL ) {
                fprintf(stderr, "A fatal error occurred while creating the table.\nExiting now.\n" );
                exit( EXIT_FAILURE );
            }
            ExtractTryOp( TabTableDefinitionClose( hSchema ) );
        }
    }

    // Create `Suppliers` table
    if (createMultipleTables) {
        TAB_HANDLE hSchema = NULL;
        TAB_HANDLE hTable = NULL;

        CreateTableauStringConst( L"SupplierKey", sSupplierKey );
        CreateTableauStringConst( L"Supplier", sSupplier );
        CreateTableauStringConst( L"Address", sAddress );
        CreateTableauStringConst( L"Suppliers", sSuppliers );

        //  Define Table Schema (If we are creating a new extract)
        int hasTable = 0;
        ExtractTryOp( TabExtractHasTable( *extractHandlePtr, sSuppliers, &hasTable ) );
        if ( !hasTable ) {
            ExtractTryOp( TabTableDefinitionCreate( &hSchema ) );
            ExtractTryOp( TabTableDefinitionAddColumn( hSchema, sSupplierKey, TAB_TYPE_Integer ) );
            ExtractTryOp( TabTableDefinitionAddColumn( hSchema, sSupplier, TAB_TYPE_CharString ) );
            ExtractTryOp( TabTableDefinitionAddColumn( hSchema, sAddress, TAB_TYPE_CharString ) );
            ExtractTryOp( TabExtractAddTable( *extractHandlePtr, sSuppliers, hSchema, &hTable ) );
            if ( hTable == NULL ) {
                fprintf(stderr, "A fatal error occurred while creating the `Suppliers` table.\nExiting now.\n" );
                exit( EXIT_FAILURE );
            }
            ExtractTryOp( TabTableDefinitionClose( hSchema ) );
        }
    }
}

//------------------------------------------------------------------------------
//  Populate Extract
//------------------------------------------------------------------------------
//  (NOTE: This function assumes that the Tableau SDK Extract API is initialized)
void
PopulateExtract(
    TAB_HANDLE extractHandle,
    bool useSpatial,
    bool createMultipleTables
)
{
    TAB_HANDLE hTable = NULL;
    TAB_HANDLE hRow = NULL;
    TAB_HANDLE hSchema = NULL;
    int i;

    // Populate `Extract`/`Products` table
    {
        CreateTableauStringConst( L"Extract", sExtract );
        CreateTableauStringConst( L"Products", sProducts );
        CreateTableauStringConst( L"uniBeans", sUniBeans );

        //  Get Schema
        if (createMultipleTables) {
            ExtractTryOp( TabExtractOpenTable( extractHandle, sProducts, &hTable ) );
        } else {
            ExtractTryOp( TabExtractOpenTable( extractHandle, sExtract, &hTable ) );
        }
        ExtractTryOp( TabTableGetTableDefinition( hTable, &hSchema ) );

        //  Insert Data
        ExtractTryOp( TabRowCreate( &hRow, hSchema ) );
        ExtractTryOp( TabRowSetDateTime( hRow, 0, 2012, 7, 3, 11, 40, 12, 4550 ) ); //  Purchased
        ExtractTryOp( TabRowSetCharString( hRow, 1, "Beans" ) );                    //  Product
        ExtractTryOp( TabRowSetString( hRow, 2, sUniBeans ) );                      //  Unicode Product
        ExtractTryOp( TabRowSetDouble( hRow, 3, 1.08 ) );                           //  Price
        ExtractTryOp( TabRowSetDate( hRow, 6, 2029, 1, 1 ) );                       //  Expiration Date
        ExtractTryOp( TabRowSetCharString( hRow, 7, "Bohnen" ) );                   //  Produkt
        if (useSpatial) {
            ExtractTryOp(TabRowSetSpatial(hRow, 8, "POINT (30 10)"));               //  Destination
        }
        for ( i = 0; i < 10; ++i ) {
            ExtractTryOp( TabRowSetInteger( hRow, 4, i * 10 ) );                    //  Quantity
            ExtractTryOp( TabRowSetBoolean( hRow, 5, i % 2 ) );                     //  Taxed
            if (createMultipleTables) {
                ExtractTryOp( TabRowSetInteger( hRow, useSpatial ? 9 : 8, i % 3 ) ); //  SupplierKey
            }
            ExtractTryOp( TabTableInsert( hTable, hRow ) );
        }
        //  Close Schema
        ExtractTryOp( TabRowClose( hRow ) );
        ExtractTryOp( TabTableDefinitionClose( hSchema ) );
    }

    // Populate `Suppliers` table
    if (createMultipleTables) {
        CreateTableauStringConst( L"Suppliers", sSuppliers );

        //  Get Schema
        ExtractTryOp( TabExtractOpenTable( extractHandle, sSuppliers, &hTable ) );
        ExtractTryOp( TabTableGetTableDefinition( hTable, &hSchema ) );

        //  Insert Data
        ExtractTryOp( TabRowCreate( &hRow, hSchema ) );
        ExtractTryOp( TabRowSetCharString( hRow, 1, "Bean Supplier" ) );            //  Supplier
        ExtractTryOp( TabRowSetCharString( hRow, 2, "42 Bean Street, Beantown" ) ); //  Address
        for ( i = 0; i < 3; ++i ) {
            ExtractTryOp( TabRowSetInteger( hRow, 0, i ) );                         //  SupplierKey
            ExtractTryOp( TabTableInsert( hTable, hRow ) );
        }
        //  Close Schema
        ExtractTryOp( TabRowClose( hRow ) );
        ExtractTryOp( TabTableDefinitionClose( hSchema ) );
    }
}

//------------------------------------------------------------------------------
//  Main
//------------------------------------------------------------------------------
int
main( int argc, char* argv[] )
{
    //  Parse Arguments
    TableauWChar *options[ NUM_OPTIONS];
    memset( options, 0, NUM_OPTIONS * sizeof( TableauWChar* ) );
    if ( !ParseArguments( argc - 1, argv  + 1, options ) || options[ HELP ] != NULL ) {
        DisplayUsage();
        FreeOptions( options, NUM_OPTIONS );
        return 0;
    }

    //  Extract API Demo
    if ( options[ BUILD ] != FALSE ) {
        //  Initialize the Tableau Extract API
        TryOp( TabExtractAPIInitialize(), "initializing the Extract API", TabGetLastErrorMessage() );

        //  Create or Expand the Extract
        TAB_HANDLE extractHandle = NULL;
        CreateOrOpenExtract( &extractHandle, options[ FILENAME ], options[ SPATIAL ] != FALSE, options[ MULTI_TABLE ] != FALSE );
        PopulateExtract( extractHandle, options[ SPATIAL ] != FALSE, options[ MULTI_TABLE ] != FALSE );

        //  Flush the Extract to Disk
        TryOp( TabExtractClose( extractHandle ), "closing an extract", TabGetLastErrorMessage() );

        // Close the Tableau Extract API
        TryOp( TabExtractAPICleanup(), "cleaning up the Extract API", TabGetLastErrorMessage() );
    }

    FreeOptions( options, NUM_OPTIONS );
    return 0;
}
