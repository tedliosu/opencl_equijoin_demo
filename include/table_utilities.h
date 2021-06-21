/* 
 * Author - Ted Li
 *
 * This file defines various functions and macros used to
 * print tables in memory, load tables from disk, write
 * a table to disk, and compare equality of two tables
 * on disk.
 */

#ifndef TABLE_UTILITIES_H
#define TABLE_UTILITIES_H

#include "data_structures_opencl.h"

/* 
 * Whether to have join results reflect data about active or inactive customers.
 *
 * If value of this macro is "CUSTOMER_ACTIVE_FLAG", then set PARALLEL_RESULTS_TABLE_FILE_PATH
 * macro to refer to either "parallel_example_join_result_active_customers.csv" or
 * "parallel_custom_join_result_active_customers.csv" depending on value of
 * EXAMPLE_OR_CUSTOM_FILES macro as defined below.  If value of this macro
 * is "CUSTOMER_INACTIVE_FLAG", then set PARALLEL_RESULTS_TABLE_FILE_PATH to refer to
 * a file name that ends in "inactive_customers.csv". Similar logic is applied
 * to setting the value of the macro SERIAL_RESULTS_TABLE_FILE_PATH and
 * RESULTS_REF_TABLE_FILE_PATH.
 */
#define IS_CUSTOMER_ACTIVE CUSTOMER_ACTIVE_FLAG
/* 
 * Whether to use the "example" data files (e.g. "example_customer_data.csv",
 * "parallel_example_join_result_active_customers.csv") or the "custom" data files (e.g.
 * "custom_customer_data.csv", "serial_custom_join_result_active_customers.csv")
 * to read from and write to for doing the equijoining of the tables; 1 stands
 * for "example" data files, 0 stands for "custom" data files.
 */
#define EXAMPLE_OR_CUSTOM_FILES 1
/*
 * Definitions of file paths of different tables stored on disk; defined
 * according to values of the macros EXAMPLE_OR_CUSTOM_FILES and IS_CUSTOMER_ACTIVE
 * as stated each macro's documentation above.
 */
#if (EXAMPLE_OR_CUSTOM_FILES)
    
    #define CUSTOMER_TABLE_FILE_PATH "./data/example_customer_data.csv"
    #define PURCHASES_TABLE_FILE_PATH "./data/example_purchases_data.csv"
    
    #if (IS_CUSTOMER_ACTIVE == CUSTOMER_ACTIVE_FLAG)
        #define PARALLEL_RESULTS_TABLE_FILE_PATH "./data/example_results"\
                                                 "/parallel_example_join_result_active_customers.csv"
        #define SERIAL_RESULTS_TABLE_FILE_PATH "./data/example_results"\
                                               "/serial_example_join_result_active_customers.csv"
        #define RESULTS_REF_TABLE_FILE_PATH "./data/example_results"\
                                            "/example_correct_join_result_active_customers.csv" 
    #elif (IS_CUSTOMER_ACTIVE == CUSTOMER_INACTIVE_FLAG)
        #define PARALLEL_RESULTS_TABLE_FILE_PATH "./data/example_results"\
                                                 "/parallel_example_join_result_inactive_customers.csv"
        #define SERIAL_RESULTS_TABLE_FILE_PATH "./data/example_results"\
                                               "/serial_example_join_result_inactive_customers.csv"
        #define RESULTS_REF_TABLE_FILE_PATH "./data/example_results"\
                                            "/example_correct_join_result_inactive_customers.csv" 
    #endif

#else

    #define CUSTOMER_TABLE_FILE_PATH "./data/custom_customer_data.csv"
    #define PURCHASES_TABLE_FILE_PATH "./data/custom_purchases_data.csv"

    #if (IS_CUSTOMER_ACTIVE == CUSTOMER_ACTIVE_FLAG)
        #define PARALLEL_RESULTS_TABLE_FILE_PATH "./data/custom_results"\
                                                 "/parallel_custom_join_result_active_customers.csv"
        #define SERIAL_RESULTS_TABLE_FILE_PATH "./data/custom_results"\
                                               "/serial_custom_join_result_active_customers.csv"
        #define RESULTS_REF_TABLE_FILE_PATH "./data/custom_results"\
                                            "/custom_correct_join_result_active_customers.csv" 
    #elif (IS_CUSTOMER_ACTIVE == CUSTOMER_INACTIVE_FLAG)
        #define PARALLEL_RESULTS_TABLE_FILE_PATH "./data/custom_results"\
                                                 "/parallel_custom_join_result_inactive_customers.csv"
        #define SERIAL_RESULTS_TABLE_FILE_PATH "./data/custom_results"\
                                               "/serial_custom_join_result_inactive_customers.csv"
        #define RESULTS_REF_TABLE_FILE_PATH "./data/custom_results"\
                                            "/custom_correct_join_result_inactive_customers.csv" 
    #endif

#endif

// Different modes for specifying how "fopen" should open a file
#define FOPEN_READ_ONLY_MODE "r"
#define FOPEN_OVERWRITE_ONLY_MODE "w"

// Delimiters used in CSV files
#define CSV_DELIMITERS ",\"\n\r"
// Value representing radix of data stored in tables
#define BASE_10_RADIX 10
/*
 * Index of character in a token representing whether or
 * not a customer is active, as read from a string
 * representation of a record of a customer table on
 * disk.
 */
#define IS_ACTIVE_CUSTOMER_CHAR_INDEX 0

/*
 * Next three macros are headers of tables to be printed out
 */
#define HASHED_CUSTOMER_TABLE_HEADER "\"customerID\",\"customerName\",\"isActiveCustomer\"\n"
#define PURCHASES_TABLE_HEADER "\"epochTimePurchased\",\"customerID\",\"purchaseEAN13\",\"purchaseQuantity\"\n"
#define JOINED_RESULT_TABLE_HEADER "\"epochTimePurchased\",\"customerID\","\
                                    "\"customerName\",\"purchaseEAN13\",\"purchaseQuantity\"\n"
/*
 * Next three macros are format strings used to print out
 * tables in the style of a CSV file
 */
#define HASHED_CUSTOMER_TABLE_ROW_FORMAT "%ld,\"%s\",%c\n"
#define PURCHASES_TABLE_ROW_FORMAT "%ld,%ld,\"%s\",%ld\n"
#define JOINED_RESULT_TABLE_ROW_FORMAT "%ld,%ld,\"%s\",\"%s\",%ld\n"

/*
 * Format string used to inform user which table from which file is being read from disk.
 */
#define FILE_BEING_READ_MSG "Currently reading table in '%s' from disk into memory...\n\n"
// Format string used to inform user where the table join result is being written to.
#define WRITING_TABLE_TO_FILE_MSG "Currently writing results table to '%s' on disk...\n\n"

/*
 * Parameter(s):
 * - const char * file_location: String representation of a relative or absolute filepath of a CSV file
 *                               containing ONLY a syntactically correct customer table stored on disk.
 *                               The file contents should look something like:
 *                               '"customerID","customerName","isActiveCustomer"
 *                                1,"James",N
 *                                2,"Mary",Y
 *                                3,"Joe",Y
 *                                [etc...]
 *                               '
 * Returns(s):
 * - A pointer to a struct containing a hashed customer table; this function hashes each record of the
 *   customer table (as the table is being read from disk) into the appropriate row index within the
 *   "table" field of the struct returned. Hashing the records as they're being read saves time later
 *   when this table is to be equijoined with the purchases table.
 *
 * IMPORTANT - the table in the file being read from is assumed to have a header.
 */
struct Hashed_Customer_Table* read_hashed_customer_table_from_csv_file(const char* file_location);

/*
 * Parameter(s):
 * - const char * file_location: String representation of a relative or absolute filepath of a CSV file
 *                               containing ONLY a syntactically correct purchases table stored on disk.
 *                               The file contents should look something like:
 *                               '"epochTimePurchased","customerID","purchaseEAN13","purchaseQuantity"
 *                                1623447438954609116,3,8411267314328,3
 *                                1623447509730429941,1,1709605413695,5
 *                                1623447545886452717,1,7925552529867,2
 *                                [etc...]
 *                               '
 * Returns(s):
 * - A pointer to a struct containing a purchases table; this function loads each row of the
 *   purchases table from disk into memory (i.e. into the struct returned).
 *
 * IMPORTANT - the table in the file being read from is assumed to have a header.
 */
struct Purchases_Table* read_purchases_table_from_csv_file(const char* file_location);

/*
 * Parameter(s):
 * - struct Joined_Results_Table * results_table: A pointer to a table containing the equijoined results
 *                                                of two different tables from disk.
 * - const char * file_location: String representation of a relative or absolute filepath of a CSV file to
 *                               which the table stored under "results_table" will be written.  If the file
 *                               doesn't already exist, the file will be created and filled with contents
 *                               in CSV form from "results_table".  If the file already exists, then the
 *                               old contents WILL BE OVERWRITTEN with the contents from "results_table".
 */
void write_results_table_to_csv_file(struct Joined_Results_Table* results_table, const char* file_location);

/*
 * The following three functions each prints out to screen the
 * contents of the table specified by the struct parameter.
 */
void print_hashed_customer_table(struct Hashed_Customer_Table* hashed_customer_table);
void print_purchases_table(struct Purchases_Table* purchases_table);
void print_joined_results_table(struct Joined_Results_Table* joined_results_table);

/*
 * Message informing user that two tables on disk are equal to each other
 */
#define ASSERTION_PASSED_INFORM_USER "Congratulations, both of your tables are identical in content!\n\n"
/*
 * Assert equality between two equijoin results tables on disk,
 *   where each table is represented by a CSV file on disk.
 * Parameter details:
 *     - first_equijoin_result_table_file: the file containing the first equijoin 
 *                                          result table to be tested for equality.
 *     - second_equijoin_result_table_file: the file containing the second equijoin
 *                                          result table to be tested for equality.
 */
void assert_equijoin_results_tables_equality(const char* first_equijoin_result_table_file,
                                             const char* second_equijoin_result_table_file);



#endif // TABLE_UTILITIES_H

