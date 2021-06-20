
# Program Business Case

  - A hypothetical online business has a customers table identifying all active and inactive customers,
    and a purchases table listing out the timestamp, EAN13 barcode of item purchased, and quantity of
    each item purchased for each customer.  Each customer is uniquely identified by his/her customer ID
    in both tables.  Given these tables, equijoin the tables on the customerID column of each table
    using two different versions of hash equijoin and see which version equijoins the two tables the
    fastest.  One version of hash equijoin has the probing stage be parallelized in OpenCL, while the
    other version has the probing stage be a serial algorithm.

  - In addition, the equijoin result of each algorithm must contain either all active or all inactive
    customers, but NOT both.  The user must be able to generate tables for either active or inactive
    customers only by simply changing one parameter to each algorithm.

  - I didn't parallelize the build stage of hash equijoin for both algorithms because I combined the build
    stage with reading the input files from disk to avoid storing a separate table in memory to minimize
    memory usage and reduce program runtime (i.e. maximize efficiency).

  - For more info on what hash equijoin is and how it works, please refer to [this Wikipedia article](https://en.wikipedia.org/wiki/Hash_join#Classic_hash_join); it's the same article I used as a reference to develop the main C program.

# Instructions For Running The Hash Equijoin Program

1. Setup OpenCL development on your machine appropriately
   1. Refer to [these set of instructions](https://github.com/tedliosu/opencl_install_instructions)
       if **running a Linux distro** on an AMD GPU and/or any kind of CPU.
   2. Next (if on a Linux distro), make sure your "CPPFLAGS" and
       "LDFLAGS" environment variables are set appropriately. (e.g.
       CPPFLAGS="-I/opt/rocm/opencl/include" LDFLAGS="-L/opt/rocm/opencl/lib").
   3. Finally (if on a Linux distro); install the following packages (or the equivalent
        packages for your distro):
        `sqlite3 libbsd-dev`

2. Git clone this repository

3. Change working directory to cloned repository

4. Run `make all`

5. Run the resulting executable. The resulting executable will pit the parallelized OpenCL version of hash
   equijoin's probing stage against the serial version of hash equijoin probing for equijoining an example
   customer table and an example purchases table on the customerID column. The executable will report the
   time it took to perform each version of the hash equijoin probing.  Next, the executable will check the
   equijoin result of each version of the probing algorithm against a known correct result generated using
   sqlite, and will print out a message of congratulations for each version of the equijoin probing
   algorithm if each algorithm performed the hash equijoin probing correctly.

6. You may tweak the IS_CUSTOMER_ACTIVE macro value in "./include/table_utilities.h" to be either
   "CUSTOMER_ACTIVE_FLAG" or "CUSTOMER_INACTIVE_FLAG" to see the program generate the equijoin result
   tables for either active customers only or inactive customers only.

7. You may also adjust the DESIRED_PLATFORM_INDEX macro value in "./include/equijoin_gpu-vs-cpu.h" for
   running parallelized hash equijoin probing in OpenCL on different OpenCL platforms on your machine.
   However, **if you have only 1 OpenCL platform installed on your machine, you MUST set
   DESIRED_PLATFORM_INDEX to 0**.

## Instructions To Run Main C Program Using Custom Input Data Tables And Custom Correct Result Table ##

1. Change macro value of EXAMPLE_OR_CUSTOM_FILES to "0" in "./include/table_utilities.h"
    - Also set IS_CUSTOMER_ACTIVE to either "CUSTOMER_ACTIVE_FLAG" or "CUSTOMER_INACTIVE_FLAG" in
      "include/table_utilities.h" depending on whether you want the equijoin results generated
      to contain only data about active or inactive customers (but not both).

2. `./generate_custom_data.py [number of customer table rows] [number of purchases table rows]`
    - For example, run `./generate_custom_data.py 1000000 3000000`

3. `./generate_custom_data_equijoin_result_ref.sh` (MAKE SURE you have sqlite3 installed for this one)
 
4. `make all` and run resulting executable.

5. If desired, change macro value of IS_CUSTOMER_ACTIVE (valid values are either "CUSTOMER_ACTIVE_FLAG"
   or "CUSTOMER_INACTIVE_FLAG"), run `make all`, and then run recompiled executable again.

# TODOs

 - Unit testing of both versions of hash equijoin in main C program with edge cases (e.g. only 1 record each
    for both the customer and purchases table of 1).
 - (Maybe) Have filepath definitions all in one file for better program maintainability.

