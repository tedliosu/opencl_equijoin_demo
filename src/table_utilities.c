
// Necessary libraries and headers for functions
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <bsd/string.h>
#include "data_structures_opencl.h"
#include "table_utilities.h"


/*
 * Parameters:
 * - const char * file_location --- String representation of a relative or absolute filepath of a file
 * - const size_t buffer_size --- Size of buffer used to buffer file reads from the file
 *                                specified by "file_location"
 *
 * Returns: a guaranteed valid read-only file handle where reads are fully buffered
 *          using a buffer of size "buffer_size"
 */
static inline FILE * open_file_read_only(const char * file_location, const size_t buffer_size) {

      // Open up file for reading only
      FILE * file_handle = fopen(file_location, FOPEN_READ_ONLY_MODE);
      // Inform user if file didn't open correctly and abort program
      if (file_handle == NULL) {
          int global_err_num = errno;
          fprintf(stderr, "Error opening %s: %s.\n", file_location, strerror(global_err_num));
          exit(global_err_num);
      }
      // Set buffer size for doing input from file
      int func_status = setvbuf(file_handle, NULL, _IOFBF, buffer_size);
      // Assert buffer resizing was done correctly.
      assert(func_status == EXIT_SUCCESS);

      return file_handle;

}


/*
 * Parameters:
 * - char * file_line --- a line of a CSV file which only stores a customer data table;
 *                          i.e. a string representation of a record from the customer
 *                          data table on disk.
 * - struct Hashed_Customer_Table_Row * customer_table_row --- a row of a hashed customer table in which
 *                                                             the record from the parameter "file_line"
 *                                                             will be stored after parsing the data
 *                                                             from the record.  Because the hashing
 *                                                             occurs when looking for the appropriate
 *                                                             row index to store each row in the hashed
 *                                                             customer table, and not when each CSV file
 *                                                             line (i.e. string representation of a hashed
 *                                                             customer table row) is being converted to
 *                                                             a hashed customer table row, I didn't include
 *                                                             the word "hashed" within the parameter name
 *                                                             nor the function's name.
 */
static inline void csv_file_line_to_customer_table_row(char* file_line,
                                                        struct Hashed_Customer_Table_Row* customer_table_row) {
    // Assert non-null pointers
    assert(file_line != NULL);
    assert(customer_table_row != NULL);
    // Pointer used by the "strtok_r" function to keep track of remaining unparsed data
    char* rest_of_string;
    /*
     * Pointer used by strtoul function to store the rest of a string
     * after parsing out the "long int" from a string.
     */
    char* rest_of_strtoul_string;

    // A token of data read from the char array "file_line", initialized to the first token read
    char *token = strtok_r(file_line, CSV_DELIMITERS, &rest_of_string);
    // First token is the customer id; store it appropriately
    customer_table_row->customer_id = strtoul(token, &rest_of_strtoul_string, BASE_10_RADIX);

    // Second token is the customer name; copy over to appropriate field
    token = strtok_r(NULL, CSV_DELIMITERS, &rest_of_string);
    strlcpy(customer_table_row->first_name, token, FIRST_NAME_MAX_LEN);

    // Third and final token is character indicating whether or not customer is currently active
    token = strtok_r(NULL, CSV_DELIMITERS, &rest_of_string);
    customer_table_row->active_customer = token[IS_ACTIVE_CUSTOMER_CHAR_INDEX];

    // No more tokens to be processed

}

/*
 * Parameters:
 * - char * file_line --- a line of a CSV file which only stores a purchases data table;
 *                          i.e. a string representation of a record from the purchases
 *                          data table on disk.
 * - struct Purchases_Table_Row * purchases_table_row --- a row of a purchases table in which
 *                                                        the record from the parameter "file_line"
 *                                                        will be stored after parsing the data
 *                                                        from the record. 
 */
static inline void csv_file_line_to_purchases_table_row(char* file_line,
                                                        struct Purchases_Table_Row* purchases_table_row) {
    // Assert non-null pointers
    assert(file_line != NULL);
    assert(purchases_table_row != NULL);
    // Pointer used by the "strtok_r" function to keep track of remaining unparsed data
    char* rest_of_string;
    /*
     * Pointer used by strtoul function to store the rest of a string
     * after parsing out the "long int" from a string.
     */
    char* rest_of_strtoul_string;

    // A token of data read from the char array "file_line", initialized to the first token read
    char *token = strtok_r(file_line, CSV_DELIMITERS, &rest_of_string);
    // First token is the time of when a purchase was made in epoch nanoseconds; store it appropriately
    purchases_table_row->time_of_purchase = strtoul(token, &rest_of_strtoul_string, BASE_10_RADIX);
    
    // Second token is the ID of the customer who made the purchase; store it appropriately
    token = strtok_r(NULL, CSV_DELIMITERS, &rest_of_string);
    purchases_table_row->customer_id = strtoul(token, &rest_of_strtoul_string, BASE_10_RADIX); 

    // Third token is the EAN13 barcode of the product purchased; copy over to appropriate field
    token = strtok_r(NULL, CSV_DELIMITERS, &rest_of_string);
    strlcpy(purchases_table_row->ean13, token, EAN13_MAX_CHARS);

    // Fourth and final token is quantity of product purchased; store appropriately
    token = strtok_r(NULL, CSV_DELIMITERS, &rest_of_string);
    purchases_table_row->quantity_purchased = strtoul(token, &rest_of_strtoul_string, BASE_10_RADIX); 

    // No more tokens to be processed

}


/*
 * Parameters:
 * - char * line_read --- a line of a CSV file which only stores a customer data table;
 *                        i.e. a string representation of a record from the customer
 *                        data table on disk.
 * - size_t * current_table_row_count --- current number of rows in the table referred to
 *                                        by the "hashed_customer_table" parameter; the
 *                                        parameter stores records from the customer table
 *                                        file.
 * - struct Hashed_Customer_Table** hashed_customer_table -- pointer to pointer of a customer
 *                                                           table in memory where each record
 *                                                           is hashed into a specific table
 *                                                           row according to the data in the
 *                                                           record.
 *
 * This function takes the string representation of a record from the customer data table
 * on disk, hashes the record to calculate where to store the record in "hashed_customer_table",
 * and then stores it in "hashed_customer_table".
 */
static inline void hash_and_store_record_in_hashed_customer_table(char* line_read,
                                                                  size_t *current_table_row_count,
                                                                  struct Hashed_Customer_Table** hashed_customer_table) {

    /* 
     * Constant multiplier governing the factor used to increase
     * the table size in memory when there's still more records
     * to be read from the file but the table in memory has ran
     * out of space.
     */
    const size_t expansion_factor = 2;
    /* 
     * Resulting row index after hashing a record of a customer
     * table using the customer ID.
     */
    unsigned long row_index = 0;
    // A row of a customer table read from a file
    struct Hashed_Customer_Table_Row table_row;

    /*
     * Parse each record read from customer table on disk into its constituents
     * and store results in memory.
     */
    csv_file_line_to_customer_table_row(line_read, &table_row);
    row_index = customer_id_to_row_index(table_row.customer_id);
    // Record number of records read from file
    ++((*hashed_customer_table)->num_records);
    /* 
     * If there's not enough room left in table in memory to store
     * additional records from file, increase the size of the table to
     * accomodate for more records.
     */
    if ((*hashed_customer_table)->num_records > *current_table_row_count) {
        *current_table_row_count = expansion_factor * *current_table_row_count;
        (*hashed_customer_table)->table =
                        reallocarray((*hashed_customer_table)->table,
                                            *current_table_row_count,
                                            sizeof(*((*hashed_customer_table)->table)));
         // Assert reallocation was successful
         assert((*hashed_customer_table)->table != NULL);
    }
    // Store parsed results in memory
    (*hashed_customer_table)->table[row_index].customer_id =
                                          table_row.customer_id;
    (*hashed_customer_table)->table[row_index].active_customer =
                                          table_row.active_customer;
    strlcpy((*hashed_customer_table)->table[row_index].first_name,
                              table_row.first_name, FIRST_NAME_MAX_LEN);

}

/*
 * Parameters:
 * - char * line_read --- a line of a CSV file which only stores a purchases data table;
 *                        i.e. a string representation of a record from the purchases
 *                        data table on disk.
 * - size_t * current_table_row_count --- current number of rows in the table referred to
 *                                        by the "purchases_table" parameter; the parameter
 *                                        stores records from the purchases table file.
 * - struct Purchases_Table** purchases_table -- pointer to pointer of a purchases
 *                                               table in memory.
 *
 * This function takes the string representation of a record from the purchases data table
 * on disk and stores it in "purchases_table"
 */
static inline void store_record_in_purchases_table(char* line_read,
                                                   size_t *current_table_row_count,
                                                   struct Purchases_Table** purchases_table) {

    /* 
     * Constant multiplier governing the factor used to increase
     * the table size in memory when there's still more records
     * to be read from the file but the table in memory has ran
     * out of space.
     */
    const size_t expansion_factor = 2;
    // A row of a purchases table read from a file
    struct Purchases_Table_Row table_row;

    /*
     * Parse each record read from purchases table on disk into its constituents
     * and store results in memory.
     */
    csv_file_line_to_purchases_table_row(line_read, &table_row);
    /*
     * Record index where current record read should be inserted into the
     * table, and record number of records read from file.
     */
    unsigned long current_record_index = ((*purchases_table)->num_records)++;
    /* 
     * If there's not enough room left in table in memory to store
     * additional records from file, increase the size of the table to
     * accomodate for more records.
     */
    if ((*purchases_table)->num_records > *current_table_row_count) {
        *current_table_row_count = expansion_factor * *current_table_row_count;
        (*purchases_table)->table =
                       reallocarray((*purchases_table)->table,
                                     *current_table_row_count,
                                     sizeof(*((*purchases_table)->table)));
         // Assert reallocation was successful
         assert((*purchases_table)->table != NULL);
    }
    // Store parsed results in memory
    (*purchases_table)->table[current_record_index].customer_id=
                                                 table_row.customer_id;
    (*purchases_table)->table[current_record_index].time_of_purchase =
                                                 table_row.time_of_purchase;
    (*purchases_table)->table[current_record_index].quantity_purchased =
                                                 table_row.quantity_purchased;
    strlcpy((*purchases_table)->table[current_record_index].ean13,
                                          table_row.ean13, EAN13_MAX_CHARS);

}


struct Hashed_Customer_Table* read_hashed_customer_table_from_csv_file(const char* file_location) {

    // Assert non-null pointers
    assert(file_location != NULL);

    // Ideal buffer size for most SSD's and HDD's for doing file IO
    const size_t buffer_size = 4096;
    // Current size of hash table in memory used to store customer table records
    size_t current_table_row_count = 1;
    // Line read from file
    char * line_read = NULL;
    // Size of buffer storing line read from file
    size_t line_buffer_size = 0;
    // Number of characters read per line
    ssize_t num_char_read = 0;
    /*
     *  Create and initialize table in memory for storing each record of
     *  the customer table after reading and hashing each record from the
     *  customer table file.
     */
    struct Hashed_Customer_Table* hashed_customer_table = malloc(sizeof(*hashed_customer_table));
    // Assert malloc was successful
    assert(hashed_customer_table != NULL);
    hashed_customer_table->table =
            malloc(sizeof(*(hashed_customer_table->table)) * current_table_row_count);
    assert(hashed_customer_table->table != NULL);
    // No records loaded from file initially
    hashed_customer_table->num_records = 0;

    // Open up file for reading only
    FILE * customer_table_file = open_file_read_only(file_location, buffer_size);

    // Inform user this program is beginning to load file from disk into memory
    printf(FILE_BEING_READ_MSG, file_location);    

    // Discard header from table being read; not needed to be stored in memory
    num_char_read = getline(&line_read, &line_buffer_size, customer_table_file);
   
    // Read entire table from file into memory
    while (true) {
       
       num_char_read = getline(&line_read, &line_buffer_size, customer_table_file);

       if (num_char_read < 0) {
           // EOF reached; exit loop for reading file line by line. 
           break;
       } else {
           hash_and_store_record_in_hashed_customer_table(line_read,
                                                          &current_table_row_count,
                                                          &hashed_customer_table);
       }

    }

    // Trim away empty table rows which weren't filled
    hashed_customer_table->table =
            reallocarray(hashed_customer_table->table,
                            hashed_customer_table->num_records,
                            sizeof(*(hashed_customer_table->table)));
    // Assert reallocation was successful
    assert(hashed_customer_table->table != NULL);
    // Done with file; close it
    fclose(customer_table_file);

    return hashed_customer_table;

}

struct Purchases_Table* read_purchases_table_from_csv_file(const char* file_location) {

    // Assert non-null pointers
    assert(file_location != NULL);

    // Ideal buffer size for most SSD's and HDD's for doing file IO
    const size_t buffer_size = 4096;
    // Current size of table in memory used to store purchases table records
    size_t current_table_row_count = 1;
    // Line read from file
    char * line_read = NULL;
    // Size of buffer storing line read from file
    size_t line_buffer_size = 0;
    // Number of characters read per line
    ssize_t num_char_read = 0;
    /*
     *  Create and initialize table in memory for storing each record of
     *  the purchases table after reading each record from the purchases
     *  table file.
     */
    struct Purchases_Table* purchases_table = malloc(sizeof(*purchases_table));
    // Assert malloc was successful
    assert(purchases_table != NULL);
    purchases_table->table =
            malloc(sizeof(*(purchases_table->table)) * current_table_row_count);
    assert(purchases_table->table != NULL);
    // No records loaded from file initially
    purchases_table->num_records = 0;

    // Open up file for reading only
    FILE * purchases_table_file = open_file_read_only(file_location, buffer_size);
    
    // Inform user this program is beginning to load file from disk into memory
    printf(FILE_BEING_READ_MSG, file_location);

    // Discard header from table being read; not needed to be stored in memory
    num_char_read = getline(&line_read, &line_buffer_size, purchases_table_file);
   
    // Read entire table from file into memory
    while (true) {
       
       num_char_read = getline(&line_read, &line_buffer_size, purchases_table_file);

       if (num_char_read < 0) {
           // EOF reached; exit loop for reading file line by line. 
           break;
       } else {
           store_record_in_purchases_table(line_read,
                                           &current_table_row_count,
                                                    &purchases_table);
       }

    }

    // Trim away empty table rows which weren't filled
    purchases_table->table =
            reallocarray(purchases_table->table,
                            purchases_table->num_records,
                            sizeof(*(purchases_table->table)));
    // Assert reallocation was successful
    assert(purchases_table->table != NULL);
    // Done with file; close it
    fclose(purchases_table_file);

    return purchases_table;

}

void write_results_table_to_csv_file(struct Joined_Results_Table* results_table, const char* file_location) {

    // Assert non-null pointers
    assert(file_location != NULL);
    assert(results_table != NULL);
    assert(results_table->table != NULL);

    // Ideal buffer size for most SSD's and HDD's for doing file IO
    const size_t buffer_size = 4096;

    // Open up file for writing to only
    FILE * results_table_file = fopen(file_location, FOPEN_OVERWRITE_ONLY_MODE);
    // Inform user if file didn't open correctly and abort program
    if (results_table_file == NULL) {
       int global_err_num = errno;
       fprintf(stderr, "Error opening %s: %s.\n", file_location, strerror(global_err_num));
       exit(global_err_num);
    }
    // Set buffer size for writing contents of "results_table" to file
    int func_status = setvbuf(results_table_file, NULL, _IOFBF, buffer_size);
    // Assert buffer resizing was done correctly.
    assert(func_status == EXIT_SUCCESS);
    // Inform user this program is beginning to write table contents to disk.
    printf(WRITING_TABLE_TO_FILE_MSG, file_location);

    // Write table header to disk first
    fprintf(results_table_file, JOINED_RESULT_TABLE_HEADER);


    // Write each row of equijoin results table to disk
    for (unsigned long row_index = 0l; row_index < results_table->num_records; ++row_index) {
      
      // Skip over writing table records to file where customer id and customer name are null values
      if (results_table->table[row_index].customer_id_customer != NULL_CUSTOMER_ID ||
          results_table->table[row_index].first_name_customer[NULL_CHARACTER_POS] != NULL_CUSTOMER_NAME) {

          int num_char_written = fprintf(results_table_file, JOINED_RESULT_TABLE_ROW_FORMAT,
                                                             results_table->table[row_index].time_of_purchase,
                                                             results_table->table[row_index].customer_id_customer,
                                                             results_table->table[row_index].first_name_customer,
                                                             results_table->table[row_index].ean13,
                                                             results_table->table[row_index].quantity_purchased);
          // Make sure there are no problems writing each row of results table to disk.
          assert(num_char_written >= 0);
      }

   }


    // Done writing to file; close it
    fclose(results_table_file);

}


void assert_equijoin_results_tables_equality(const char* first_equijoin_result_table_file,
                                              const char* second_equijoin_result_table_file) {

     assert(first_equijoin_result_table_file != NULL);
     assert(second_equijoin_result_table_file != NULL);

     // Ideal buffer size for most SSD's and HDD's for doing file IO
     const size_t buffer_size = 4096;
     // Lines read from each file
     char *line_read_first_file = NULL, *line_read_second_file = NULL;
     // Sizes of buffer storing line read from each file
     size_t line_buffer_size_first_file = 0, line_buffer_size_second_file = 0;
     // Number of characters read per line for each file
     ssize_t num_char_read_first_file = 0, num_char_read_second_file = 0;
     /*
      * Pointers used by the "strtok_r" function to keep track of
      * remaining unparsed data for each file being read.
      */
     char *rest_of_string_first_file_line, *rest_of_string_second_file_line;
    
     // Open up files for reading only
     FILE * first_equijoin_table_file = open_file_read_only(first_equijoin_result_table_file, buffer_size);
     FILE * second_equijoin_table_file = open_file_read_only(second_equijoin_result_table_file, buffer_size);

     /*
      * Read each file line by line and compare if the two different
      * lines between the two files at the same line number in each file
      * are identical field for field.
      */
     while (true) {
     
        num_char_read_first_file = getline(&line_read_first_file, &line_buffer_size_first_file,
                                                                        first_equijoin_table_file);
        num_char_read_second_file = getline(&line_read_second_file, &line_buffer_size_second_file,
                                                                        second_equijoin_table_file);

        if (num_char_read_first_file < 0 && num_char_read_second_file < 0) {
            // EOF reached for both files; exit loop
            break;
        } else {
            /*
             * Read tokens from each line read and assert that first
             * token read from first file is the same as first token
             * read from second file, second token read from first
             * file is the same as second token read from second file,
             * etc.
             */
            char* token_first_file = strtok_r(line_read_first_file, CSV_DELIMITERS,
                                                     &rest_of_string_first_file_line);
            char* token_second_file = strtok_r(line_read_second_file, CSV_DELIMITERS,
                                                     &rest_of_string_second_file_line);
            assert(strcmp(token_first_file, token_second_file) == 0);
            for (int token_read_count = 1;
                 token_read_count < JOINED_RESULTS_TABLE_ROW_FIELDS_COUNT;
                                                         ++token_read_count) {
                token_first_file = strtok_r(NULL, CSV_DELIMITERS,
                                                     &rest_of_string_first_file_line);
                token_second_file = strtok_r(NULL, CSV_DELIMITERS,
                                                     &rest_of_string_second_file_line); 
                assert(strcmp(token_first_file, token_second_file) == 0);
            }
        }

     }

     /*
      * If this point has been reached successfully, print
      * statement to user signifying all assertions have
      * passed and therefore each file contains an identical
      * table.
      */
     printf(ASSERTION_PASSED_INFORM_USER);

     // Done with files, close them
     fclose(first_equijoin_table_file);
     fclose(second_equijoin_table_file);

}


void print_hashed_customer_table(struct Hashed_Customer_Table* hashed_customer_table) {
   
   // No null pointers allowed for parameter
   assert(hashed_customer_table != NULL);
   assert(hashed_customer_table->table != NULL);

   // Headers for table to be printed out
   printf(HASHED_CUSTOMER_TABLE_HEADER);

   // Print out each row of hashed customer table
   for (unsigned long row_index = 0l; row_index < hashed_customer_table->num_records; ++row_index) {
       printf(HASHED_CUSTOMER_TABLE_ROW_FORMAT,
                hashed_customer_table->table[row_index].customer_id,
                hashed_customer_table->table[row_index].first_name,
                hashed_customer_table->table[row_index].active_customer);
   }

   printf("\n");

}

void print_purchases_table(struct Purchases_Table* purchases_table) {
   
   // No null pointers allowed for parameter
   assert(purchases_table != NULL);
   assert(purchases_table->table != NULL);

   // Headers for table to be printed out
   printf(PURCHASES_TABLE_HEADER);

   // Print out each row of purchases table
   for (unsigned long row_index = 0l; row_index < purchases_table->num_records; ++row_index) {
       printf(PURCHASES_TABLE_ROW_FORMAT,
                purchases_table->table[row_index].time_of_purchase,
                purchases_table->table[row_index].customer_id,
                purchases_table->table[row_index].ean13,
                purchases_table->table[row_index].quantity_purchased);
   }

   printf("\n");

}

void print_joined_results_table(struct Joined_Results_Table* joined_results_table) {
   
   // No null pointers allowed for parameter
   assert(joined_results_table != NULL);
   assert(joined_results_table->table != NULL);

   printf(JOINED_RESULT_TABLE_HEADER);

   // Print out each row of equijoin results table
   for (unsigned long row_index = 0l; row_index < joined_results_table->num_records; ++row_index) {
      
      // Skip over printing out table records where customer id and customer name are null values
      if (joined_results_table->table[row_index].customer_id_customer != NULL_CUSTOMER_ID ||
          joined_results_table->table[row_index].first_name_customer[NULL_CHARACTER_POS] != NULL_CUSTOMER_NAME) {

          printf(JOINED_RESULT_TABLE_ROW_FORMAT,
                 joined_results_table->table[row_index].time_of_purchase,
                 joined_results_table->table[row_index].customer_id_customer,
                 joined_results_table->table[row_index].first_name_customer,
                 joined_results_table->table[row_index].ean13,
                 joined_results_table->table[row_index].quantity_purchased);

      }

   }

   printf("\n");

}


