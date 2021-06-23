
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
   4. `sudo -H pip install Faker`

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
   tables for either active customers only or inactive customers only.  You may also tweak the macro
   value of NUM_THREADS_IN_BLOCK listed in "./include/equijoin_opencl.h" to be any positive non-zero integer,
   BUT the number of rows (i.e. 2,000,000) in the example purchases table (located at "./data/example_purchases_data.csv")
   **MUST be divisible by** the value of NUM_THREADS_IN_BLOCK (in "./include/equijoin_opencl.h") **if
   the value of EXAMPLE_OR_CUSTOM_FILES is "1"** in "./include/table_utilities.h"; **otherwise
   the main C program WILL SEGFAULT**.

7. You may also adjust the DESIRED_PLATFORM_INDEX macro value in "./include/equijoin_gpu-vs-cpu.h" for
   running parallelized hash equijoin probing in OpenCL on different OpenCL platforms on your machine.
   However, **if you have only 1 OpenCL platform installed on your machine, you MUST set
   DESIRED_PLATFORM_INDEX to 0**.

## Instructions To Run Main C Program Using Custom Input Data Tables ##

1. Change macro value of EXAMPLE_OR_CUSTOM_FILES to "0" in "./include/table_utilities.h"
    - Also set IS_CUSTOMER_ACTIVE to either "CUSTOMER_ACTIVE_FLAG" or "CUSTOMER_INACTIVE_FLAG" in
      "include/table_utilities.h" depending on whether you want the equijoin results generated
      to contain only data about active or inactive customers (but not both).

2. `./generate_custom_data.py [number of customer table rows] [number of purchases table rows]`
   (MAKE SURE you have the Faker pip package installed for this one)
    - For example, run `./generate_custom_data.py 1000000 3000000`
    - Also, tweak the value of NUM_THREADS_IN_BLOCK listed in "./include/equijoin_opencl.h" if
      necessary so that the number of purchase table rows you specified in this step **is divisible
      by** the value of NUM_THREADS_IN_BLOCK; **otherwise the main C program WILL SEGFAULT**.

3. `./generate_custom_data_equijoin_result_ref.sh` (MAKE SURE you have sqlite3 installed for this one)
 
4. `make all` and run resulting executable.

5. If desired, change macro value of IS_CUSTOMER_ACTIVE (valid values are either "CUSTOMER_ACTIVE_FLAG"
   or "CUSTOMER_INACTIVE_FLAG"), run `make all`, and then run recompiled executable again.

# Performance Metrics

## Main C Program Output With The Program Running OpenCL Using GPU ##

### On Machine With A Ryzen 5 2600 and A RX Vega 56 ###

##### With Example Customer Table of 1.5 Millon Records and Example Purchases Table of 2 Million Records ######

<pre>
Currently reading table in './data/example_customer_data.csv' from disk into memory...

Currently reading table in './data/example_purchases_data.csv' from disk into memory...

>>> OpenCL program compiler result message: - 

>>> Performing parallelized hash equijoin probing on OpenCL device with 250 work-items per workgroup
Parallelized hash equijoin probing of hashed customer table with 1500000 row(s) and purchases table with 2000000 row(s) on OpenCL device took 0.118785 seconds

Currently writing results table to './data/example_results/parallel_example_join_result_active_customers.csv' on disk...

>>> Performing serial hash equijoin probing in main memory
Serial hash equijoin probing of hashed customer table with 1500000 row(s) and purchases table with 2000000 row(s) in main memory took 0.185644 seconds

Currently writing results table to './data/example_results/serial_example_join_result_active_customers.csv' on disk...

>>> Table stored at './data/example_results/parallel_example_join_result_active_customers.csv' currently being verified
    using table stored at './data/example_results/example_correct_join_result_active_customers.csv'.
Congratulations, both of your tables are identical in content!

>>> Table stored at './data/example_results/serial_example_join_result_active_customers.csv' currently being verified
    using table stored at './data/example_results/example_correct_join_result_active_customers.csv'.
Congratulations, both of your tables are identical in content!

</pre>

##### With Custom Customer Table of 20 Millon Records and Custom Purchases Table of 60 Million Records ######

 - **Note:** it took **about an hour total** (on my machine at least) to generate the tables needed for
   running the main C program with tens of millions of records per table.  So be prepared for steps 2 and
   3 to take a while if you want to follow the instructions under "**Instructions To Run Main C Program Using Custom Input Data Tables**"
   with a very large number of customer table rows and purchases tables rows specified (e.g. doing step 2
   as `./generate_custom_data.py 20000000 60000000` and then doing step 3).

<pre>
Currently reading table in './data/custom_customer_data.csv' from disk into memory...

Currently reading table in './data/custom_purchases_data.csv' from disk into memory...

>>> OpenCL program compiler result message: - 

>>> Performing parallelized hash equijoin probing on OpenCL device with 250 work-items per workgroup
Parallelized hash equijoin probing of hashed customer table with 20000000 row(s) and purchases table with 60000000 row(s) on OpenCL device took 2.770294 seconds

Currently writing results table to './data/custom_results/parallel_custom_join_result_active_customers.csv' on disk...

>>> Performing serial hash equijoin probing in main memory
Serial hash equijoin probing of hashed customer table with 20000000 row(s) and purchases table with 60000000 row(s) in main memory took 6.713951 seconds

Currently writing results table to './data/custom_results/serial_custom_join_result_active_customers.csv' on disk...

>>> Table stored at './data/custom_results/parallel_custom_join_result_active_customers.csv' currently being verified
    using table stored at './data/custom_results/custom_correct_join_result_active_customers.csv'.
Congratulations, both of your tables are identical in content!

>>> Table stored at './data/custom_results/serial_custom_join_result_active_customers.csv' currently being verified
    using table stored at './data/custom_results/custom_correct_join_result_active_customers.csv'.
Congratulations, both of your tables are identical in content!

</pre>

# TODOs

 - Unit testing of both versions of hash equijoin in main C program with edge cases (e.g. only 1 record each
    for both the customer and purchases table of 1).
 - (Maybe) Have filepath definitions all in one file for better program maintainability.

# Miscellaneous

 - *.empty_file* in "./data/custom_results" is just there so that git may at least include the
   "./data/custom_results" directory in version control so that the main C program and the
   scripts have all the directories they need to work properly.

