
// =================================================================================================
//
// File description:
// Header file for data structure types which are shared between the CPU and GPU for performing
//   the equijoin of two tables.
//
// Original file information:
// Institution.... SURFsara <www.surfsara.nl>
// Original Author......... Cedric Nugteren <cedric.nugteren@surfsara.nl>
// "Remixing" programmer.. Ted Li
// Changed at..... 2020-06-15
// License........ MIT license
// 
// =================================================================================================

#ifndef DATA_STRUCTURES_OPENCL_H
#define DATA_STRUCTURES_OPENCL_H

/*
 * One plus the maximum number of characters in a first name;
 *   remember null character termination!
 */
#define FIRST_NAME_MAX_LEN 21
/*
 * One plus maximum number of characters in an EAN13 barcode;
 *   again null character termination!
 */
#define EAN13_MAX_CHARS 14
/*
 * Value of Integer ID of customer representing a NULL value
 * for a customer ID in the table join result.
 */
#define NULL_CUSTOMER_ID 0l
/*
 * Value of customer name representing a NULL value in the
 * table join result; it's just a null character.
 */
#define NULL_CUSTOMER_NAME '\0'
/*
 * Index of the null character that represents a NULL
 * value in the join result; the index is for accessing 
 * the appropriate position in "first_name" char array
 * field of the "Joined_Results_Table_Row" struct in
 * the table join result.
 */
#define NULL_CHARACTER_POS 0
/*
 * Character representing customer is inactive in the
 * customer table.
 */
#define CUSTOMER_INACTIVE_FLAG 'N'
/*
 * Character representing customer who is still active
 * in the customer table.
 */
#define CUSTOMER_ACTIVE_FLAG 'Y'
/*
 * Number of fields in a Joined_Results_Table_Row struct
 */
#define JOINED_RESULTS_TABLE_ROW_FIELDS_COUNT 5


/*
 * A hashed row of an extremely simplified
 *    customer table for a hypothetical
 *    online marketplace such as Amazon
 *    or Ebay; the index at which it is stored
 *    in the "table" field of a Hashed_Customer_Table
 *    is the result of hashing the "customer_id".
 *    - customer_id: Integer ID of a customer
 *    - first_name: just the first name of a customer
 *    - active_customer: whether or not the customer
 *                       is still active, as defined
 *                       by CUSTOMER_INACTIVE_FLAG and
 *                       CUSTOMER_ACTIVE_FLAG.
 */
struct Hashed_Customer_Table_Row {
     unsigned long customer_id;
     char first_name[FIRST_NAME_MAX_LEN];
     char active_customer;
};
/*
 * A struct containing:
 *    - table: a pointer to the first entry of the
 *      simplified hashed customer table
 *    - num_records: the number of records in the
 *      hashed customer table
 */
struct Hashed_Customer_Table {
    struct Hashed_Customer_Table_Row* table;
    unsigned long num_records;
};

/*
 * A row of an extremely simplified
 *    purchases table for a hypothetical
 *    online marketplace documenting:
 *  - time_of_purchase: Time of purchase in
 *    nanoseconds since Jan 1 1970
 *  - customer_id: Integer ID of customer who
 *    made the purchase
 *  - ean13: the EAN13 barcode of the product purchased
 *  - quantity_purchased: the quantity of the product purchased
 */
struct Purchases_Table_Row {
    unsigned long time_of_purchase;
    unsigned long customer_id;
    char ean13[EAN13_MAX_CHARS];
    unsigned long quantity_purchased;
};
/*
 * A struct containing:
 *    - table: a pointer to the first entry of the
 *      simplified purchases table
 *    - num_records: the number of records in the
 *      purchases table
 */
struct Purchases_Table {
    struct Purchases_Table_Row* table;
    unsigned long num_records;
};

/*
 * A row of the result of joining
 *    Purchases_Table and Hashed_Customer_Table
 *    together; all rows either consists
 *    of either active or inactive customers,
 *    but NOT both.
 *    - time_of_purchase: Time of purchase in
 *      nanoseconds since Jan 1 1970
 *    - customer_id_customer: the Integer
 *      ID of the customer from the Hashed_Customer_Table
 *    - first_name_customer: The first name of a customer
 *      from the Hashed_Customer_Table
 *    - ean13: the EAN13 barcode of the product purchased
 *    - quantity_purchased: the quantity of the product purchased
 */
struct Joined_Results_Table_Row { 
    unsigned long time_of_purchase;
    unsigned long customer_id_customer;
    char first_name_customer[FIRST_NAME_MAX_LEN];
    char ean13[EAN13_MAX_CHARS];
    unsigned long quantity_purchased;
};
/*
 * A struct containing:
 *    - table: a pointer to the first entry of the
 *      joined result of the hashed customer and purchases
 *      table; all rows either consists of either active
 *      or inactive customers, but NOT both.
 *    - num_records: the number of records in the
 *      table containing the joined results
 */
struct Joined_Results_Table {
    struct Joined_Results_Table_Row* table;
    unsigned long num_records;
};

/* 
 * A group of "table" structs that
 * the programmer wishes to pass
 * around in both device and main
 * memory to various functions.
 */
struct List_Of_Tables {
    struct Hashed_Customer_Table* hashed_customer_table;
    struct Purchases_Table* purchases_table;
    struct Joined_Results_Table* results_table;
};

/* 
 * Hash function used to hash the customer
 * id into a row index value to be stored
 * under the "table" field of a "Hashed_Customer_Table"
 *
 * It's just the identity function minus one for
 * now to keep customer lookup time at constant time
 * in the hashed customer table with the simplest and
 * most straightforward implementation as possible.
 */
#define customer_id_to_row_index(customer_id) (customer_id - 1)

#endif // DATA_STRUCTURES_OPENCL_H

// =================================================================================================

