
#include "/home/bkupuntu/personal_sandboxes/c_sandbox/opencl_basics/work/opencl_equijoin_demo/include/data_structures_opencl.h"

/*
 * Hash join probing kernel operating on global memory in OpenCL.
 * 
 * Each instance of the kernel scans one record from the purchases table
 * and joins that record with the appropriate record from the hashed customer
 * table. The resulting record is then stored at the same row index as it
 * had appeared in the purchases table.
 *
 * However, if the record from the hashed customer table during the joining
 * process is found to be that of an inactive customer when the is_customer_active
 * parameter indicates to put only records of active customers in the results
 * table (and vice versa for the parameter indicating storing only records of
 * inactive customers in the result table) the macro NULL_CUSTOMER_ID (as defined
 * in the included header file) is inserted for the customer ID column and value of
 * the macro NULL_CUSTOMER_NAME (as defined in the included header file) is
 * inserted for the customer first name column in the results table. This is
 * because buffers cannot be dynamically sized in device memory in OpenCL.
 *
 * Implementation of kernel inspired by following websites:
 *   https://en.wikipedia.org/wiki/Hash_join
 *
 * Parameter details:
 *   - hashed_customer_table: Hashed table of customers, where each row/array entry
 *                            consists of an integer customer id and a customer
 *                            name, and each record must be already hashed by
 *                            the host device into the proper bucket based
 *                            on the customer id.
 *   - purchases_table: Table of purchases, where each row/array entry
 *                      consists of time of purchase in nanoseconds since
 *                      Jan 1 1970, integer ID of customer who made the purchase,
 *                      the EAN13 barcode of the product purchased, and
 *                      the quantity of the product purchased.
 *   - results_table: Resulting table after joining purchases_table and
 *                    hashed_customer_table together on the customer id column of
 *                    each table
 *   - is_customer_active: Flag variable indicating whether to have results_table
 *                          contain only data about active or inactive customers
 *                          (but not both).
 */
__kernel void naive_hash_equijoin_probe(__global struct Hashed_Customer_Table_Row* hashed_customer_table,
                                                      __global struct Purchases_Table_Row* purchases_table,
                                                     __global struct Joined_Results_Table_Row* results_table,
                                                                                 const char is_customer_active)
{
   /* 
    * Value representing the first work-item dimension in the OpenCL programming model, as
    * the tables are arrays of structs, and arrays are one-dimensional data structures and
    * therefore we are only concerned with that first dimension.
    */
   const unsigned int first_dimension_num = 0;
   
   /*
    * The current row of the equijoin result table, which in this case also represents
    * the current row of the purchases table being scanned for the equijoin.
    */
   unsigned long result_table_row = get_global_id(first_dimension_num);

   /*
    * Retrieve row index of matching record from hashed customer
    * table using hash function macro.
    */
   unsigned long hashed_customer_table_row = 
                    customer_id_to_row_index(purchases_table[result_table_row].customer_id);
   
   /*
    * If the customer table record's active_customer flag and the is_customer_active
    * parameter both indicate the same thing on whether or not customer is active, then
    * join the customer table record and the matching purchase table record together. 
    */
    if (hashed_customer_table[hashed_customer_table_row].active_customer == is_customer_active) {
           
          results_table[result_table_row].time_of_purchase =
                        purchases_table[result_table_row].time_of_purchase;
          results_table[result_table_row].customer_id_customer =
                        hashed_customer_table[hashed_customer_table_row].customer_id;
          results_table[result_table_row].quantity_purchased =
                        purchases_table[result_table_row].quantity_purchased;
          /*
           * Copy string fields over character by character as there is no
           * "strcpy"-like function in OpenCL.
           */
          #pragma unroll
          for (unsigned int char_index = 0; char_index < FIRST_NAME_MAX_LEN; ++char_index) {
                results_table[result_table_row].first_name_customer[char_index] =
                            hashed_customer_table[hashed_customer_table_row].first_name[char_index];
          }
          #pragma unroll
          for (unsigned int char_index = 0; char_index < EAN13_MAX_CHARS; ++char_index) {
                results_table[result_table_row].ean13[char_index] =
                            purchases_table[result_table_row].ean13[char_index];
          }
    
    } else {

          /*
           * If customer status indicated by the "active_customer" field and the "is_customer_active"
           * parameter don't agree, then (as defined in the included header file) insert NULL_CUSTOMER_ID
           * into each customer id attribute (as stored in the "customer_id_customer" field in each
           * "Joined_Results_Table_Row" struct/row) and insert NULL_CUSTOMER_NAME into the customer
           * first name attribute (as stored in the "first_name_customer" field in each "Joined_Results_Table_Row"
           * struct/row). No need to copy anything over from either the customer table nor the purchases table.
           */
           results_table[result_table_row].customer_id_customer = NULL_CUSTOMER_ID;
           results_table[result_table_row].first_name_customer[NULL_CHARACTER_POS] = 
                                                                       NULL_CUSTOMER_NAME;

   }

}

