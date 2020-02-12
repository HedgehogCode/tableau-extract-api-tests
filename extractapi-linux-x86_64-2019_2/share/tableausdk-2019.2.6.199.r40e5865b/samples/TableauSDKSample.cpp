//------------------------------------------------------------------------------
//
//  This file is the copyrighted property of Tableau Software and is protected
//  by registered patents and other applicable U.S. and international laws and
//  regulations.
//
//  Unlicensed use of the contents of this file is prohibited. Please refer to
//  the NOTICES.txt file for further details.
//
//------------------------------------------------------------------------------
#if defined(__APPLE__) && defined(__MACH__)
#include <TableauHyperExtract/TableauHyperExtract_cpp.h>
#else
#include "TableauHyperExtract_cpp.h"
#endif

#include <codecvt>
#include <cstring>
#include <iostream>
#include <locale>
#include <map>
#include <string>

using namespace Tableau;

//------------------------------------------------------------------------------
//  Display Usage
//------------------------------------------------------------------------------
void DisplayUsage()
{
    std::cerr << "A simple demonstration of the Tableau Extract API:" << std::endl
              << std::endl
              << "USAGE: tableauSDKSample-cpp [COMMAND] [OPTIONS]" << std::endl
              << std::endl
              << "COMMANDS:" << std::endl
              << "  -h, --help           Show this help message and exit" << std::endl
              << std::endl
              << "  -b, --build          If a Tableau extract named FILENAME exists in the current directory,"
              << std::endl
              << "                       extend it with sample data." << std::endl
              << "                       If no Tableau extract named FILENAME exists in the current directory,"
              << std::endl
              << "                       create one and populate it with sample data." << std::endl
              << std::endl
              << "OPTIONS:" << std::endl
              << " -s, --spatial         Include spatial data when creating a new extract." << std::endl
              << "                       If an extract is being extended, this argument is ignored." << std::endl
              << "                       (default=False)" << std::endl
              << std::endl
              << " -m, --multitable      Create multi-table extract." << std::endl
              << "                       Note that multi-table storage for extracts is supported starting with Tableau 2018.3." << std::endl
              << "                       (default=False)" << std::endl
              << std::endl
              << " -f FILENAME, --filename FILENAME" << std::endl
              << "                       FILENAME of the extract to be created or extended." << std::endl
              << "                       (default='order-cpp.hyper')" << std::endl;
}

//------------------------------------------------------------------------------
//  Parse Arguments
//------------------------------------------------------------------------------
//  Parse Command Line Arguments or Populate Defaults
//
//  Returns 'true' if all arguments are successfully parsed
//  Returns 'false' if there are any invalid arguments or no arguments
bool ParseArguments(int argc, char* argv[], std::map<std::string, std::wstring>& options)
{
    if (argc == 0)
    {
        return false;
    }

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t, 0x10ffff, std::codecvt_mode::little_endian>> converter;
    for (int i = 0; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
        {
            options[std::string("help")] = std::wstring(L"true");
            return true;
        }
        else if (!strcmp(argv[i], "-b") || !strcmp(argv[i], "--build"))
        {
            options[std::string("build")] = std::wstring(L"true");
        }
        else if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--spatial"))
        {
            options[std::string("spatial")] = std::wstring(L"true");
        }
        else if (!strcmp(argv[i], "-m") || !strcmp(argv[i], "--multitable"))
        {
            options[std::string("multitable")] = std::wstring(L"true");
        }
        else if (!strcmp(argv[i], "-f") || !strcmp(argv[i], "--filename"))
        {
            if (i >= argc)
            {
                return false;
            }
            options[std::string("filename")] = converter.from_bytes(argv[++i]);
        }
        else
        {
            return false;
        }
    }

    //  Defaults
    //  options["build"] = "false";
    //  options["spatial"] = "false";
    if (!options.count("filename"))
    {
        options["filename"] = L"order-cpp.hyper";
    }

    return true;
}

//------------------------------------------------------------------------------
//  Create or Open Extract
//------------------------------------------------------------------------------
std::shared_ptr<Extract> CreateExtract(const std::wstring& fileName, const bool useSpatial, bool createMultipleTables)
{
    std::shared_ptr<Extract> extractPtr = nullptr;
    try
    {
        //  Create Extract Object
        //  (NOTE: TabExtractCreate() opens an existing extract with the given
        //   filename if one exists or creates a new extract with the given filename
        //   if one does not)
        extractPtr = std::make_shared<Extract>(fileName);

        //  Define `Extract`/`Products` table
        auto tableName = createMultipleTables ? L"Products" : L"Extract";
        if (!extractPtr->HasTable(tableName))
        {
            TableDefinition schema;
            // NOTE: Collations make string operations very expensive and this may
            // slow down the overall workbook performance, so use them with care.
            schema.SetDefaultCollation(Collation_Binary);
            schema.AddColumn(L"Purchased", Type_DateTime);
            schema.AddColumn(L"Product", Type_CharString);
            schema.AddColumn(L"uProduct", Type_UnicodeString);
            schema.AddColumn(L"Price", Type_Double);
            schema.AddColumn(L"Quantity", Type_Integer);
            schema.AddColumn(L"Taxed", Type_Boolean);
            schema.AddColumn(L"Expiration Date", Type_Date);
            schema.AddColumnWithCollation(L"Produkt", Type_CharString, Collation_de);
            if (useSpatial)
            {
                schema.AddColumn(L"Destination", Type_Spatial);
            }
            if (createMultipleTables)
            {
                schema.AddColumn(L"SupplierKey", Type_Integer);
            }

            std::shared_ptr<Table> tablePtr = extractPtr->AddTable(tableName, schema);
            if (tablePtr == nullptr)
            {
                std::wcerr << L"A fatal error occurred while creating the new table" << std::endl
                           << L"Exiting Now." << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        //  Define `Suppliers` table (If we are creating a new multi-table extract)
        if (createMultipleTables && !extractPtr->HasTable(L"Suppliers"))
        {
            TableDefinition schema;
            schema.AddColumn(L"SupplierKey", Type_Integer);
            schema.AddColumn(L"Supplier", Type_CharString);
            schema.AddColumn(L"Address", Type_CharString);

            std::shared_ptr<Table> tablePtr = extractPtr->AddTable(L"Suppliers", schema);
            if (tablePtr == nullptr)
            {
                std::wcerr << L"A fatal error occurred while creating the `Suppliers` table" << std::endl
                           << L"Exiting Now." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }
    catch (const TableauException& e)
    {
        std::wcerr << L"A fatal error occurred while creating the new extract: " << std::endl
                   << e.GetMessage() << std::endl
                   << L"Exiting Now." << std::endl;
        exit(EXIT_FAILURE);
    }

    return extractPtr;
}

//------------------------------------------------------------------------------
//  Populate Extract
//------------------------------------------------------------------------------
void PopulateExtract(const std::shared_ptr<Extract> extractPtr, bool useSpatial, bool createMultipleTables)
{
    try
    {
        // Populate `Extract`/`Products` table
        {
            //  Get Schema
            auto tableName = createMultipleTables ? L"Products" : L"Extract";
            std::shared_ptr<Table> tablePtr = extractPtr->OpenTable(tableName);
            std::shared_ptr<Tableau::TableDefinition> schema = tablePtr->GetTableDefinition();

            //  Insert Data
            Tableau::Row row(*schema);
            row.SetDateTime(0, 2012, 7, 3, 11, 40, 12, 4550); //  Purchased
            row.SetCharString(1, "Beans");                    //  Product
            row.SetString(2, L"uniBeans");                    //  Unicode Product
            row.SetDouble(3, 1.08);                           //  Price
            row.SetDate(6, 2029, 1, 1);                       //  Expiration Date
            row.SetCharString(7, "Bohnen");                   //  Produkt
            if (useSpatial)
            {
                row.SetSpatial(8, "POINT (30 10)"); //  Destination
            }
            for (int i = 0; i < 10; ++i)
            {
                row.SetInteger(4, i * 10);     //  Quantity
                row.SetBoolean(5, i % 2 == 1); //  Taxed
                if (createMultipleTables)
                {
                    row.SetInteger(useSpatial ? 9 : 8, i % 3); //  SupplierKey
                }
                tablePtr->Insert(row);
            }
        }

        // Populate `Suppliers` table
        if (createMultipleTables) {
            //  Get Schema
            std::shared_ptr<Table> tablePtr = extractPtr->OpenTable(L"Suppliers");
            std::shared_ptr<Tableau::TableDefinition> schema = tablePtr->GetTableDefinition();

            //  Insert Data
            Tableau::Row row(*schema);
            row.SetCharString(1, "Bean Supplier");            //  Supplier
            row.SetCharString(2, "42 Bean Street, Beantown"); //  Address
            for (int i = 0; i < 3; ++i)
            {
                row.SetInteger(0, i);                         //  SupplierKey
                tablePtr->Insert(row);
            }
        }
    }
    catch (const TableauException& e)
    {
        std::wcerr << L"A fatal error occurred while populating the extract: " << std::endl
                   << e.GetMessage() << std::endl
                   << L"Exiting Now." << std::endl;
        exit(EXIT_FAILURE);
    }
}

//------------------------------------------------------------------------------
//  Main
//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    //  Parse Arguments
    std::map<std::string, std::wstring> options;
    if (!ParseArguments(argc - 1, argv + 1, options) || options.count("help"))
    {
        DisplayUsage();
        exit(EXIT_SUCCESS);
    }

    //  Extract API Demo
    if (options.count("build") > 0)
    {
        //  Initialize the Tableau Extract API
        ExtractAPI::Initialize();

        //  Create or Expand the Extract
        std::shared_ptr<Extract> extractPtr = nullptr;
        extractPtr = std::move(CreateExtract(options["filename"], options.count("spatial") > 0, options.count("multitable") > 0));
        PopulateExtract(extractPtr, options.count("spatial") > 0, options.count("multitable") > 0);

        //  Flush the Extract to Disk
        extractPtr->Close();

        //  Close the Tableau Extract API
        ExtractAPI::Cleanup();
    }

    exit(EXIT_SUCCESS);
}
