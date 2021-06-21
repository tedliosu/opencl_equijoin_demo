
// =================================================================================================
//
// File description:
// Header file for custom OpenCL hash equijoin probe host functions that allocate device memory for
// the hash equijoin probe function and then perform the actual joining of tables; first function in
// this file is responsible for setting up parameters for ONE KERNEL ONLY, and then the second executes
// that kernel.
//
// Implementation of Kernel and table joining method inspired by the following websites:
// - https://en.wikipedia.org/wiki/Hash_join
//
// Original file information:
// Institution.... SURFsara <www.surfsara.nl>
// Original Author......... Cedric Nugteren <cedric.nugteren@surfsara.nl>
// "Remixing" programmer.. Ted Li
// Changed at..... 2020-06-15
// License........ MIT license
// 
// =================================================================================================

#ifndef EQUIJOIN_OPENCL_H
#define EQUIJOIN_OPENCL_H
#define CL_TARGET_OPENCL_VERSION 220
// Main OpenCL library include
#include <CL/cl.h>
/* 
 * Custom data structures for storing tables
 * to be joined and the joined result
 */
#include "data_structures_opencl.h"

/*
 * Threadblock sizes; 64 - 128 is what Intel recommends for most algorithms I believe
 * DO NOT USE THREAD BLOCKS LARGER THAN 256; it WILL BREAK the code.
 */
#define NUM_THREADS_IN_BLOCK 256
// Amount of offset of starting location of buffer contents in device memory
#define CL_BUFFER_OFFSET 0
/*
 * Number of dimensions of problem being solved
 *   In this case it's 1 b/c we're hash-joining
 *   tables represented by arrays of structs and
 *   each array is one dimensional
 */
#define OPERAND_DIMS 1

// File in which program containing OpenCL kernels is stored
#define PROGRAM_FILE "./src/equijoin_program.cl"
// Name of kernel function in OpenCL program file
#define KERNEL_FUNC_NAME "naive_hash_equijoin_probe"
// Compiler options for compiling contents of OpenCL program file
#define OPENCL_COMPILER_OPTIONS ""

/*
 * Message notifying user of start of hash join probing on OpenCL device
 * and informing user size of workgroups used to perform the join.
 */
#define NOTIFY_USER_HASH_JOIN_OP ">>> Performing parallelized hash equijoin "\
                                   "probing on OpenCL device with %d work-items per workgroup\n"

/* 
 * A group of operands where each operand points to a
 * memory "handle" (like a file handle in C but for a 
 * memory buffer) indicating a segment of data copied
 * over from main memory to device memory (could be
 * VRAM or even FPGA memory).  Each memory handle
 * corresponds to a copy of data pointed to by each
 * "table" field of each struct pointer in the
 * List_Of_Tables struct.
 */
struct Cl_Mem_Operands_List {
     cl_mem* hashed_customer_table_buffer;
     cl_mem* purchases_table_buffer;
     cl_mem* joined_results_table_buffer;
};

/* 
 * Load tables to be equijoined using hash join probing AND the empty
 * table used to store the joined result in into OpenCL device's memory;
 * the data will be processed by the kernel later on the OpenCL device.
 * Each table each HAS TO CONTAIN at least one row.
 * Parameter details:
 *   - context --- the OpenCL execution context for which the load the tables
 *   - queue --- the OpenCL command queue created from the aforementioned "context"
 *                in which to enqueue write commands to load the tables into OpenCL
 *                device memory.
 *   - tables_list --- a struct containing pointers to "table" structs where each
 *                     struct's "table" field points to data to be loaded into
 *                     OpenCL device's memory.
 *   - cl_operands --- a struct containing pointers where each pointer points
 *                      to a "buffer handle" of a segment of data in the OpenCL
 *                      device's memory, where each segment of data was copied
 *                      over from main memory.
 */
void load_tables_hash_equijoin_probe(cl_context *context, cl_command_queue* queue, 
                                      struct List_Of_Tables tables_list, struct Cl_Mem_Operands_List cl_operands);

/* 
 * Parameter details:
 * - queue --- the OpenCL command queue in which to enqueue commands for
 *              merging tables using hash join probing.
 * - cl_program* program --- MUST point to a program in memory which has a hash
 *                           join probe kernel function of signature:
 *                           __kernel void naive_hash_equijoin_probe
 *                                           (__global struct Hashed_Customer_Table_Row* hashed_customer_table,
 *                                            __global struct Purchases_Table_Row* purchases_table,
 *                                            __global struct Joined_Results_Table_Row* results_table,
 *                                                         const unsigned long hashed_customer_table_row_count)
 * - cl_kernel* kernel --- must point to a kernel function in memory whose function signature
 *                         is the "naive_hash_equijoin_probe" signature specified above.
 * - tables_list --- a struct containing pointers to "table" structs where each struct's "table"
 *                    field points to either a table in memory to be equijoined with another
 *                    table or an empty table to store the equijoined results in.
 * - cl_operands --- a struct containing pointers where each pointer points to a "memory buffer
 *                    handle" in the OpenCL device's memory; each of those memory buffers contains
 *                    either the table to be equijoined with another table or the table storing
 *                    the results of the equijoin.
 * - is_customer_active --- a flag variable indicating whether to have join results
 *                          contain data solely on active customers or solely on
 *                          inactive customers; this function DOES NOT support producing
 *                          a resulting table containing data for both types of customers
 *
 * Custom implementation of hash join probing using OpenCL;
 * Each set of tables being joined together is performed within
 * device memory. The tables being joined each HAS TO contain at
 * least one row.  The tables being joined together are "tables_list.hashed_customer_table"
 * and "tables_list.purchases_table", and the joined result is
 * then stored in "tables_list.results_table"
 */
void opencl_hash_equijoin_probe(cl_command_queue *queue, cl_program *program,
                                 cl_kernel* kernel, struct List_Of_Tables tables_list,
                                   struct Cl_Mem_Operands_List cl_operands, char is_customer_active);

#endif // EQUIJOIN_OPENCL_H
// =================================================================================================

