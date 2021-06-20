
/* 
 * File description: Serial version of hash equijoin probing.
 *   Implementation based on https://en.wikipedia.org/wiki/Hash_join#Classic_hash_join
 */

#include "equijoin_serial.h"
#include "data_structures_opencl.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <bsd/string.h>

void serial_hash_equijoin_probe(struct List_Of_Tables tables_list, const char is_customer_active) {

    // No table included within the "List_Of_Tables" parameter shall refer to a NULL value
    assert(tables_list.hashed_customer_table != NULL);
    assert(tables_list.purchases_table != NULL);
    assert(tables_list.results_table != NULL);
    assert(tables_list.hashed_customer_table->table != NULL);
    assert(tables_list.purchases_table->table != NULL);
    assert(tables_list.results_table->table != NULL);
    // Row count of each table MUST be greater than zero
    assert(tables_list.hashed_customer_table->num_records > 0);
    assert(tables_list.purchases_table->num_records > 0);
    assert(tables_list.results_table->num_records > 0);
    // Check that "is_customer_active" is of valid value
    assert(is_customer_active == CUSTOMER_ACTIVE_FLAG ||
                    is_customer_active == CUSTOMER_INACTIVE_FLAG);

    // Notify user hash join probing is about to start
    printf(NOTIFY_USER_SERIAL_HASH_JOIN_OP);

    /*
     * Scan each row of the purchases table, and join with the appropriate row
     * from the customers table by probing the hashed customers table; current
     * row index of results table is also equal to current row index of purchases
     * table.
     */
    for (unsigned long result_table_row = 0;
         result_table_row < tables_list.purchases_table->num_records;
                                                     ++result_table_row) {
    
      /*
       * Retrieve row index of matching record from hashed customer
       * table using hash function macro.
       */
      unsigned long hashed_customer_table_row =
              customer_id_to_row_index(tables_list.purchases_table->
                                          table[result_table_row].customer_id);

      /*
       * If the customer table record's active_customer flag and the is_customer_active
       * parameter both indicate the same thing on whether or not customer is active, then
       * join the customer table record and the matching purchase table record together.
       */
      if (tables_list.hashed_customer_table->
                       table[hashed_customer_table_row].active_customer ==
                                                          is_customer_active) {

         tables_list.results_table->table[result_table_row].time_of_purchase =
                 tables_list.purchases_table->table[result_table_row].time_of_purchase;
         tables_list.results_table->table[result_table_row].customer_id_customer = 
                 tables_list.hashed_customer_table->table[hashed_customer_table_row].customer_id;
         tables_list.results_table->table[result_table_row].quantity_purchased = 
                 tables_list.purchases_table->table[result_table_row].quantity_purchased;
         strlcpy(tables_list.results_table->table[result_table_row].first_name_customer,
                 tables_list.hashed_customer_table->table[hashed_customer_table_row].first_name,
                                                                               FIRST_NAME_MAX_LEN);
         strlcpy(tables_list.results_table->table[result_table_row].ean13,
                 tables_list.purchases_table->table[result_table_row].ean13,
                                                              EAN13_MAX_CHARS);

      } else {
      
          /*
           * If customer status indicated by the "active_customer" field and the
           * "is_customer_active" parameter don't agree, then (as defined in the
           * included header file) insert NULL_CUSTOMER_ID into each customer id
           * attribute (as stored in the "customer_id_customer" field in each
           * "Joined_Results_Table_Row" struct/row) and insert NULL_CUSTOMER_NAME
           * into the customer first name attribute (as stored in the
           * "first_name_customer" field in each "Joined_Results_Table_Row" struct/row).
           * No need to copy anything over from either the customer table nor the
           * purchases table.
           */
          tables_list.results_table->table[result_table_row].customer_id_customer = NULL_CUSTOMER_ID;
          tables_list.results_table->table[result_table_row].
                       first_name_customer[NULL_CHARACTER_POS] = NULL_CUSTOMER_NAME;

      }

    }


}

