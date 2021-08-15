
// =================================================================================================
// File Description: 
// Bitonic sorting in OpenCL main header file 
//
// Original file information:
// Institution.... SURFsara <www.surfsara.nl>
// Original Author......... Cedric Nugteren <cedric.nugteren@surfsara.nl>
// Refactoring programmer.. Ted Li
// Changed at..... 2021-04-02
// License........ MIT license
// 
// =================================================================================================

#ifndef EQUIJOIN_GPU_VS_CPU_H
#define EQUIJOIN_GPU_VS_CPU_H 

#include "equijoin_opencl.h"

// Number of OpenCL platforms on host machine
#define NUM_CL_PLATFORMS 2
/*
 * Index of desired OpenCL platform (e.g. Portable Computing
 *    Language, AMD Accelerated Parallel Processing) in
 *    list of platforms returned.
 */
#define DESIRED_PLATFORM_INDEX 0
// Number of OpenCL devices per OpenCL platform
#define NUM_CL_DEVICES 1
// Number of OpenCL programs to be loaded
#define OPENCL_PROGS 1
// OpenCL device name max length
#define MAX_LEN 1024
// Delimiter for reading non-csv text files
#define TEXT_FILE_DELIM '\0'

// Number of nanoseconds in a second
#define NANOSECS_IN_SEC 1000000000.0

// Messages to user informing time took to perform different joins and size of tables joined
#define EQUIJOIN_PARALLEL_MESSAGE "Parallelized hash equijoin probing of hashed customer"\
                                   " table with %ld row(s) and purchases table with"\
                                   " %ld row(s) on OpenCL device took %lf seconds\n\n"
#define EQUIJOIN_SERIAL_MESSAGE "Serial hash equijoin probing of hashed customer"\
                                   " table with %ld row(s) and purchases table with"\
                                   " %ld row(s) in main memory took %lf seconds\n\n"

/*
 * Message informing user which table is being checked for correctness and which table
 * is being used as the reference for the correct result
 */
#define CHECK_RESULTS_MESSAGE ">>> Table stored at '%s' currently being verified\n"\
                                                "    using table stored at '%s'.\n"

#endif // EQUIJOIN_GPU_VS_CPU_H
// =================================================================================================

