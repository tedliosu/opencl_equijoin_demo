
/*
 * File description:
 *   Header file for function implementing hash equijoin probing serially in C.
 */

#ifndef EQUIJOIN_SERIAL_H
#define EQUIJOIN_SERIAL_H 

#include "data_structures_opencl.h"

/*
 * Message notifying user start of serial hash join probing
 */
#define NOTIFY_USER_SERIAL_HASH_JOIN_OP "Performing serial hash equijoin probing in main memory\n"

/*
 * Serial implementation of hash equijoin probing in C.
 * Parameters:
 * - struct List_Of_Tables tables_list --- Group of tables involved in the hash equijoin.  "tables_list.hashed_customer_table"
 *                                          is the build side of the equijoin, "tables_list.purchases_table" is the
 *                                          probe side of the equijoin, and "tables_list.results_table" is 
 *                                          the resulting table formed from joining "tables_list.hashed_customer_table"
 *                                          and "tables_list.purchases_table" together.  Please refer to the included
 *                                          "data_structures_opencl.h" on what each field of "tables_list" represents
 *                                          more in detail.
 * - const char is_customer_active --- Flag variable indicating whether to have "tables_list.results_table"
 *                                     contain only data about active or inactive customers
 *                                     (but not both).
 *
 * This function executes the probe of the hash equijoin of "tables_list.hashed_customer_table"
 * (a.k.a. the hashed customer table) and "tables_list.purchases_table" (a.k.a. the purchases table).
 * The hashed customer table and the purchases table are joined together by this function on the customer
 * id column of each table, and the result is stored under the pointer "tables_list.results_table".
 *
 * Since the performance of this function is being compared to the performance of the OpenCL kernel
 * version of this function, and the OpenCL kernel version is unable to resize the results table
 * generated as a result of the equijoin, this function will follow the same principle as the
 * OpenCL kernel of inserting NULL_CUSTOMER_ID (as defined in the included header file) into the
 * customer ID column of the results table and NULL_CUSTOMER_NAME (as defined in the included
 * header file) into the customer first name column in the results table when the "is_customer_active"
 * parameter DOES NOT equal to the "active_customer" field of the corresponding record from the
 * customer table during the hash equijoin probing. 
 *
 */
void serial_hash_equijoin_probe(struct List_Of_Tables tables_list, const char is_customer_active);

#endif // EQUIJOIN_SERIAL_H

