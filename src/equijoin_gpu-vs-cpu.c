
// =================================================================================================
// Project: 
// Exploring hash equijoin in OpenCL.
//
// Original file information:
// Institution.... SURFsara <www.surfsara.nl>
// Original Author......... Cedric Nugteren <cedric.nugteren@surfsara.nl>
// "Remixing" programmer.. Ted Li
// Changed at..... 2021-05-15
// License........ MIT license
// 
// =================================================================================================

// Libraries used by this program with custom headers
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include "data_structures_opencl.h"
#include "table_utilities.h"
#include "equijoin_opencl.h"
#include "equijoin_serial.h"
#include "equijoin_gpu-vs-cpu.h"

// =================================================================================================

/*
 * Initialize "results_table" as an empty table with
 * "num_records" empty records; "num_records" MUST be
 * greater than zero.
 */
void initialize_results_table(struct Joined_Results_Table ** results_table, const unsigned long num_records) {

     // Assert non-null pointer and num_records > 0
     assert(results_table != NULL);
     assert(num_records > 0);

     *results_table = malloc(sizeof(**results_table));
     // Assert malloc didn't have any errors
     assert(*results_table != NULL);
     (*results_table)->num_records = num_records;
     // Now initialize all entries of table to be "blank" records
     (*results_table)->table = malloc(sizeof(*((*results_table)->table)) * num_records);
     assert((*results_table)->table != NULL);

}

/* 
 * Given a specified file location containing an OpenCL program
 * reads the entire contents of the file into memory and returns
 * a pointer marking the start of the contents in memory.
 */
char* get_opencl_program_code(char* opencl_program_file_location) {
   
      // No null pointers allowed
      assert(opencl_program_file_location != NULL);

      char *source_code_content = NULL;
      const char *file_read_mode = "r";
      const char *file_open_error_msg = "Error opening %s: %s.\n";
      const char *file_io_error_msg = "Error reading %s: %s\n";
      FILE *opencl_prog_file = fopen(opencl_program_file_location, file_read_mode);

      if (opencl_prog_file == NULL) {
         int global_err_num = errno;
         fprintf(stderr, file_open_error_msg, opencl_program_file_location,
                                                       strerror(global_err_num));
         exit(global_err_num);
      } else {
            size_t init_buffer_size = 0;
            ssize_t num_bytes_read = getdelim(&source_code_content, &init_buffer_size,
                                                      TEXT_FILE_DELIM, opencl_prog_file);
            if (num_bytes_read < 0) {
                   int global_err_num = errno;
                   fprintf(stderr, file_io_error_msg, opencl_program_file_location,
                                                                strerror(global_err_num));
                   exit(global_err_num);
            }
      }

      return source_code_content;

}

/*
 * Setup procedure for executing OpenCL programs.  The procedure involves
 * creating an execution context to be used by the OpenCL-programmed device
 * (in this case the GPU or CPU), setting up the queue which is used to store
 * the kernels to be executed by the device within the execution context, and
 * then dynamically compiling the program containing the kernels which are to be
 * executed once the kernels (or even multiple copies of each kernel) gets added
 * to the queue.
 */
void configure_opencl_env(cl_context *context, cl_command_queue* queue,
                                                      cl_program *program) {
    
    // No null pointers allowed
    assert(context != NULL);
    assert(queue != NULL);
    assert(program != NULL);

    cl_device_id device;
    char deviceName[MAX_LEN];
    cl_queue_properties queue_properties[] = {CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0};
    cl_platform_id *platforms = malloc(sizeof(*platforms) * NUM_CL_PLATFORMS);
    
    clGetPlatformIDs(NUM_CL_PLATFORMS, platforms, NULL);
    clGetDeviceIDs(platforms[DESIRED_PLATFORM_INDEX], CL_DEVICE_TYPE_DEFAULT, NUM_CL_DEVICES, &device, NULL);
    *context = clCreateContext(NULL, NUM_CL_DEVICES, &device, NULL, NULL, NULL);
    *queue = clCreateCommandQueueWithProperties(*context, device, queue_properties, NULL);

    // read OpenCL program file into string
    const char* opencl_program_string = get_opencl_program_code(PROGRAM_FILE);

    clGetDeviceInfo(device, CL_DEVICE_NAME, MAX_LEN, deviceName, NULL);


    if (opencl_program_string != NULL) {

        // Compile the opencl_program
        *program = clCreateProgramWithSource(*context, OPENCL_PROGS, &opencl_program_string, NULL, NULL);
        clBuildProgram(*program, 0, NULL, OPENCL_COMPILER_OPTIONS, NULL, NULL);

        // Get info generated by compiler and output any compiler-generated messages to user
        size_t logSize;
        clGetProgramBuildInfo(*program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
        char* messages = (char*)malloc((1+logSize)*sizeof(char));
        clGetProgramBuildInfo(*program, device, CL_PROGRAM_BUILD_LOG, logSize, messages, NULL);
        messages[logSize] = TEXT_FILE_DELIM;
        printf(">>> OpenCL program compiler result message: - %s\n\n", messages); 
        free(messages);
    
    }

    // Platforms already acquired; free malloc'ed memory
    free(platforms);

}


// Testing hash join of two tables using a custom OpenCL program.
int main(int argc, char* argv[]) {
    
    // All variable declarations
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;
    cl_mem hashed_customer_table_buffer;
    cl_mem purchases_table_buffer;
    cl_mem results_table_buffer;

    struct Hashed_Customer_Table* hashed_customer_table = read_hashed_customer_table_from_csv_file(CUSTOMER_TABLE_FILE_PATH);
    struct Purchases_Table* purchases_table = read_purchases_table_from_csv_file(PURCHASES_TABLE_FILE_PATH); 
   
    // Initialize results table according to how many rows "purchases_table" has.
    struct Joined_Results_Table* results_table;
    initialize_results_table(&results_table, purchases_table->num_records);
   
    struct timespec current_time;
    double equijoin_start_time, equijoin_end_time;

    struct List_Of_Tables tables_list = {
                                          hashed_customer_table,
                                          purchases_table,
                                          results_table
                                        };
    struct Cl_Mem_Operands_List cl_mem_ops = {
                                               &hashed_customer_table_buffer,
                                               &purchases_table_buffer,
                                               &results_table_buffer
                                             };
    
    configure_opencl_env(&context, &queue, &program);

    // Get time of when parallelized hash equijoin probing starts executing
    timespec_get(&current_time, TIME_UTC);
    equijoin_start_time = (double) current_time.tv_sec + ((double) current_time.tv_nsec) / NANOSECS_IN_SEC;

    load_tables_hash_equijoin_probe(&context, &queue, tables_list, cl_mem_ops);

    opencl_hash_equijoin_probe(&queue, &program, &kernel, tables_list, cl_mem_ops, IS_CUSTOMER_ACTIVE);

    // Get time of when parallelized hash equijoin probing finishes executing
    timespec_get(&current_time, TIME_UTC);
    equijoin_end_time = (double) current_time.tv_sec + ((double) current_time.tv_nsec) / NANOSECS_IN_SEC;

    /* 
     * Give back to the system main memory and device memory space used for OpenCL as
     * OpenCL device is no longer used after this point.
     */
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseMemObject(*(cl_mem_ops.hashed_customer_table_buffer));
    clReleaseMemObject(*(cl_mem_ops.purchases_table_buffer));
    clReleaseMemObject(*(cl_mem_ops.joined_results_table_buffer));

    // Report to user time spent on parallelized hash equijoin probing in OpenCL
    printf(EQUIJOIN_PARALLEL_MESSAGE,
                 tables_list.hashed_customer_table->num_records,
                 tables_list.purchases_table->num_records,
                   equijoin_end_time - equijoin_start_time);
    
    // Write result of parallelized hash equijoin to disk
    write_results_table_to_csv_file(tables_list.results_table, PARALLEL_RESULTS_TABLE_FILE_PATH);

    /*
     * Reset equijoin results table so that serial hash
     * equijoin may use the same variables to store its
     * results.
     */
    free(results_table->table);
    free(results_table);
    // Re-initialize results table according to how many rows "purchases_table" has.
    initialize_results_table(&results_table, purchases_table->num_records);
    tables_list.results_table = results_table;

    // Get time of when serial hash equijoin probing starts executing
    timespec_get(&current_time, TIME_UTC);
    equijoin_start_time = (double) current_time.tv_sec + ((double) current_time.tv_nsec) / NANOSECS_IN_SEC;

    serial_hash_equijoin_probe(tables_list, IS_CUSTOMER_ACTIVE);

    // Get time of when serial hash equijoin probing finishes executing
    timespec_get(&current_time, TIME_UTC);
    equijoin_end_time = (double) current_time.tv_sec + ((double) current_time.tv_nsec) / NANOSECS_IN_SEC;

    // Report to user time spent on serial hash equijoin probing in OpenCL
    printf(EQUIJOIN_SERIAL_MESSAGE,
                 tables_list.hashed_customer_table->num_records,
                 tables_list.purchases_table->num_records,
                   equijoin_end_time - equijoin_start_time);

    // Write result of serial hash equijoin to disk
    write_results_table_to_csv_file(tables_list.results_table, SERIAL_RESULTS_TABLE_FILE_PATH);

    // Check output result of each equijoin against known correct result.
    assert_equijoin_results_tables_equality(PARALLEL_RESULTS_TABLE_FILE_PATH, RESULTS_REF_TABLE_FILE_PATH);
    assert_equijoin_results_tables_equality(SERIAL_RESULTS_TABLE_FILE_PATH, RESULTS_REF_TABLE_FILE_PATH);

    // Empty main memory of all data stored
    free(hashed_customer_table->table);
    free(hashed_customer_table);
    free(purchases_table->table);
    free(purchases_table);
    free(results_table->table);
    free(results_table);

    return EXIT_SUCCESS;
}

// =================================================================================================
