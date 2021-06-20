
// =================================================================================================
// File description:
// Contains implementations of host functions for hash equijoin probing; 1st function in this file is to
// set up parameters for executing ONE KERNEL, and the 2nd function in this file executes the kernel
// to join the two tables via hash equijoin probing.
//
// Original file information:
// Institution.... SURFsara <www.surfsara.nl>
// Original Author......... Cedric Nugteren <cedric.nugteren@surfsara.nl>
// "Remixing" programmer.. Ted Li
// Changed at..... 2021-05-15
// License........ MIT license
// 
// =================================================================================================

// Libraries used by the functions in this file with custom headers
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "equijoin_opencl.h"

// =================================================================================================

void load_tables_hash_equijoin_probe(cl_context *context, cl_command_queue* queue,
                                              struct List_Of_Tables tables_list,
                                                 struct Cl_Mem_Operands_List cl_operands) {     
    // No null pointers allowed
    assert(context != NULL);
    assert(queue != NULL);
    assert(tables_list.hashed_customer_table != NULL);
    assert(tables_list.hashed_customer_table->table != NULL);
    assert(tables_list.hashed_customer_table->table->first_name != NULL);
    assert(tables_list.purchases_table != NULL);
    assert(tables_list.purchases_table->table != NULL);
    assert(tables_list.purchases_table->table->ean13 != NULL);
    assert(tables_list.results_table != NULL);
    assert(tables_list.results_table->table != NULL);
    assert(tables_list.results_table->table->ean13 != NULL);
    assert(tables_list.results_table->table->first_name_customer != NULL);
    assert(cl_operands.hashed_customer_table_buffer != NULL);
    assert(cl_operands.joined_results_table_buffer != NULL);
    assert(cl_operands.purchases_table_buffer != NULL);
    // Each table HAS to have at least 1 row
    assert(tables_list.hashed_customer_table->num_records >= 1);
    assert(tables_list.purchases_table->num_records >= 1);
    assert(tables_list.results_table->num_records >= 1);

    cl_int func_error_code;
    /* 
     * Number of write commands that'll have to be executed
     * successfully before this function returns.
     */
    const cl_int num_of_write_events = 3;
    /*
     * Index of each write command in the write_events array
     */
    const unsigned int hashed_customer_table_write_index = 0,
                       purchases_table_write_index = 1,
                       results_table_write_index = 2;
    /*
     * List of all write commands that'll need to be completed
     * before this function returns.
     */
    cl_event* write_events = malloc(num_of_write_events * sizeof(cl_event));

    // Create buffers to be filled on OpenCL device based on execution environment.
    *(cl_operands.hashed_customer_table_buffer) = clCreateBuffer(*context, CL_MEM_READ_ONLY, 
                                                             (tables_list.hashed_customer_table->num_records) *
                                                                  sizeof(*(tables_list.hashed_customer_table->table)),
                                                                                         NULL, &func_error_code);
    *(cl_operands.purchases_table_buffer) = clCreateBuffer(*context, CL_MEM_READ_ONLY, 
                                                             (tables_list.purchases_table->num_records) *
                                                                  sizeof(*(tables_list.purchases_table->table)),
                                                                                         NULL, &func_error_code);
    *(cl_operands.joined_results_table_buffer) = clCreateBuffer(*context, CL_MEM_READ_WRITE, 
                                                                 (tables_list.results_table->num_records) *
                                                                   sizeof(*(tables_list.results_table->table)),
                                                                                          NULL, &func_error_code);

    /*
     * Copy tables to the buffers created on the OpenCL device
     */
    func_error_code = clEnqueueWriteBuffer(*queue, *(cl_operands.hashed_customer_table_buffer),
                                                        CL_NON_BLOCKING, CL_BUFFER_OFFSET,
                                                 (tables_list.hashed_customer_table->num_records) *
                                                    sizeof(*(tables_list.hashed_customer_table->table)),
                                                         tables_list.hashed_customer_table->table, 0,
                                                          NULL, &(write_events[hashed_customer_table_write_index]));
    func_error_code = clEnqueueWriteBuffer(*queue, *(cl_operands.purchases_table_buffer),
                                                        CL_NON_BLOCKING, CL_BUFFER_OFFSET,
                                                 (tables_list.purchases_table->num_records) *
                                                    sizeof(*(tables_list.purchases_table->table)),
                                                         tables_list.purchases_table->table, 0,
                                                          NULL, &(write_events[purchases_table_write_index]));
    func_error_code = clEnqueueWriteBuffer(*queue, *(cl_operands.joined_results_table_buffer),
                                                        CL_NON_BLOCKING, CL_BUFFER_OFFSET,
                                                 (tables_list.results_table->num_records) *
                                                    sizeof(*(tables_list.results_table->table)),
                                                         tables_list.results_table->table, 0,
                                                          NULL, &(write_events[results_table_write_index]));

    // Wait for all write commands to finish executing
    clWaitForEvents(num_of_write_events, write_events);
    // Writing to OpenCL device memory finished; free memory storing list of write command events.
    free(write_events);

}

void opencl_hash_equijoin_probe(cl_command_queue *queue, cl_program *program,
                                      cl_kernel* kernel, struct List_Of_Tables tables_list,
                                        struct Cl_Mem_Operands_List cl_operands, char is_customer_active) {
    // No null pointers allowed
    assert(program != NULL);
    assert(queue != NULL);
    assert(kernel != NULL);
    assert(tables_list.hashed_customer_table != NULL);
    assert(tables_list.hashed_customer_table->table != NULL);
    assert(tables_list.hashed_customer_table->table->first_name != NULL);
    assert(tables_list.purchases_table != NULL);
    assert(tables_list.purchases_table->table != NULL);
    assert(tables_list.purchases_table->table->ean13 != NULL);
    assert(tables_list.results_table != NULL);
    assert(tables_list.results_table->table != NULL);
    assert(tables_list.results_table->table->ean13 != NULL);
    assert(tables_list.results_table->table->first_name_customer != NULL);
    assert(cl_operands.hashed_customer_table_buffer != NULL);
    assert(cl_operands.joined_results_table_buffer != NULL);
    assert(cl_operands.purchases_table_buffer != NULL);
    // Each table HAS to have at least 1 row
    assert(tables_list.hashed_customer_table->num_records >= 1);
    assert(tables_list.purchases_table->num_records >= 1);
    assert(tables_list.results_table->num_records >= 1);

    // The last event to be performed in the command queue on the OpenCL device
    cl_event event;

    // Generate the kernel runtime from the compiled OpenCL program.
    *kernel = clCreateKernel(*program, KERNEL_FUNC_NAME, NULL);

    /* 
     * Specify size of each thread block and size of result output table
     * for the kernel to be executed
     */
    const size_t local[OPERAND_DIMS] = { NUM_THREADS_IN_BLOCK };
    const size_t global[OPERAND_DIMS] = { tables_list.results_table->num_records };

    // Notify user hash join probing starts now
    printf(NOTIFY_USER_HASH_JOIN_OP, NUM_THREADS_IN_BLOCK);

    // Set arguments for equijoin kernel
    clSetKernelArg(*kernel, 0, sizeof(*(cl_operands.hashed_customer_table_buffer)),
                                        (void*)cl_operands.hashed_customer_table_buffer);
    clSetKernelArg(*kernel, 1, sizeof(*(cl_operands.purchases_table_buffer)),
                                        (void*)cl_operands.purchases_table_buffer);
    clSetKernelArg(*kernel, 2, sizeof(*(cl_operands.joined_results_table_buffer)),
                                        (void*)cl_operands.joined_results_table_buffer);
    clSetKernelArg(*kernel, 3, sizeof(is_customer_active), (void*)&is_customer_active);
    
    // Enqueue equijoin task to command queue to execute the equijoin.
    clEnqueueNDRangeKernel(*queue, *kernel, OPERAND_DIMS, NULL, global, local, 0, NULL, &event);

    // Wait for table equijoining to be finished
    clWaitForEvents(1, &event);

    // Copy the result of the table join back to main memory
    clEnqueueReadBuffer(*queue, *(cl_operands.joined_results_table_buffer), CL_BLOCKING,
                                 CL_BUFFER_OFFSET, tables_list.results_table->num_records *
                                                  sizeof(*(tables_list.results_table->table)),
                                                  tables_list.results_table->table, 0, NULL, NULL);
    
}

// =================================================================================================
