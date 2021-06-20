#!/bin/python3

#############################################################################
#
# Description - This program generates custom customer and purchases tables
#               in CSV file format for the main C equijoin program to equijoin
#               the two tables together on the "customerID" column of each table.
#
# Author - Ted Li
#
##############################################################################


## IMPORTS SECTION ##

# For fetching command line args.
import sys
# for random data generation seeding
import time
# pip install Faker - for generating realistic fake data
from faker import Faker
# for declaring variables as non-modifiable
from typing import Final

## END IMPORTS SECTION ##


## GLOBAL VARIABLES SECTION ##

# Positions of each parameter specifying number of records to be generated
#     for either the customer data table or purchases data table in the array
#     of arguments scanned from the command line.
NUM_RECORDS_CUSTOMER_PARAM_POS: Final = 1
NUM_RECORDS_PURCHASES_PARAM_POS: Final = 2

# relative filepath of file to output generated customer table to.
OUTPUT_CUSTOMER_TABLE_FILE: Final = "./data/custom_customer_data.csv"
# relative filepath of file to output generated purchases table to.
OUTPUT_PURCHASES_TABLE_FILE: Final = "./data/custom_purchases_data.csv"
# write-only file mode indicator string for "open" function
WRITE_ONLY_MODE: Final = "w"

# CSV headers of customer table file and purchases table file.
HEADER_CUSTOMER_TABLE: Final = "\"customerID\",\"customerName\",\"isActiveCustomer\"\n"
HEADER_PURCHASES_TABLE: Final = "\"epochTimePurchased\",\"customerID\",\"purchaseEAN13\",\"purchaseQuantity\"\n"

# Character representing customer who is still active in the customer table.
CUSTOMER_ACTIVE_FLAG: Final = "Y"
# Character representing customer who is inactive in the customer table.
CUSTOMER_INACTIVE_FLAG: Final = "N"
# Number representing smallest possible customerID value
SMALLEST_CUSTOMER_ID: Final = 1
# Max and min number of nanoseconds between each "epochTimePurchased"
#    timestamp in the purchases table
MAX_NS_GAP: Final = 2000
MIN_NS_GAP: Final = 1
# Minimum number of purchases of a product a customer can make
MIN_PURCHASE_PRODUCT_COUNT = 1


## END GLOBAL VARIABLES SECTION ##


## BEGIN CLASSES AND FUNCTIONS DECLARATIONS SECTION ##

# https://www.geeksforgeeks.org/context-manager-in-python/, because
#     file descriptor leakage is never a good thing.
class FileManager():
    # Parameters:
    #
    # - filename - path of file to be opened for file IO by an instance of this class.
    #
    # - mode - what kind of file IO operation an instance of this class will be
    #          performing on the file specified by "filename"; e.g. reading,
    #          writing, appending, etc.
    #
    def __init__(self, filename, mode):
        self.filename = filename
        self.mode = mode
        self.file = None
                                              
    def __enter__(self):
        self.file = open(self.filename, self.mode)
        return self.file

    def __exit__(self, exception_type, exception_value, exception_traceback):
        self.file.close()
        # if an exception was caught, display to user associated stacktrace
        if exception_type is not None:
            print(exception_traceback)


def main():

    if len(sys.argv) > NUM_RECORDS_PURCHASES_PARAM_POS:

        # Check to make sure that each argument from command line
        #    can be used as number of records in a table.
        assert sys.argv[NUM_RECORDS_CUSTOMER_PARAM_POS].isnumeric() and\
                            int(sys.argv[NUM_RECORDS_CUSTOMER_PARAM_POS]) > 0,\
                            "invalid value for number of customer table rows "\
                            "to be generated"
        assert sys.argv[NUM_RECORDS_PURCHASES_PARAM_POS].isnumeric() and\
                            int(sys.argv[NUM_RECORDS_PURCHASES_PARAM_POS]) > 0,\
                            "invalid value for number of purchases table rows "\
                            "to be generated"
        num_customer_table_rows = int(sys.argv[NUM_RECORDS_CUSTOMER_PARAM_POS])
        num_purchases_table_rows = int(sys.argv[NUM_RECORDS_PURCHASES_PARAM_POS])
        # create new fake data generator and seed with current time
        fake_data_gen = Faker()
        Faker.seed(time.time_ns())
        print(f"\nGenerating customer table with {num_customer_table_rows} random records\n"
                f"    and purchases table with {num_purchases_table_rows} random records; this may\n"
                f"    take some time\n")
        # Generate customer table data and write to appropriately pre-defined file.
        with FileManager(OUTPUT_CUSTOMER_TABLE_FILE, WRITE_ONLY_MODE) as customer_table_file:
            # Write header first to file
            customer_table_file.write(HEADER_CUSTOMER_TABLE)
            # Write records of faked data to file, where each record consists of a
            #    integer customerID, first name, and a character denoting whether
            #    or not the corresponding customer is active or not. The integer
            #    customerID is generated through incrementing an integer, and the
            #    other two attributes in the record are randomly generated.
            for customer_id in range(SMALLEST_CUSTOMER_ID,
                                        SMALLEST_CUSTOMER_ID + num_customer_table_rows):
                first_name = fake_data_gen.first_name()
                is_customer_active = fake_data_gen.pybool()
                if is_customer_active:
                    customer_table_file.write(f"{customer_id},\"{first_name}\",{CUSTOMER_ACTIVE_FLAG}\n")
                else:
                    customer_table_file.write(f"{customer_id},\"{first_name}\",{CUSTOMER_INACTIVE_FLAG}\n")
        # Generate purchases table data and write to appropriately pre-defined file.
        with FileManager(OUTPUT_PURCHASES_TABLE_FILE, WRITE_ONLY_MODE) as purchases_table_file:
            # Write header first to file
            purchases_table_file.write(HEADER_PURCHASES_TABLE)
            # Initialize "epochTimePurchased" timestamp to current time in Epoch nanoseconds
            epoch_time_purchased = time.time_ns();
            # Write records of faked data to file, where each record consists of a
            #    integer timestamp in Epoch nanoseconds, a integer customerID,
            #    EAN13 product code, and the quantity of the product purchased
            #    where the product is specified by the EAN13 code.
            for purchase_table_row_index in range(num_purchases_table_rows):
                customer_id = fake_data_gen.pyint(min_value=SMALLEST_CUSTOMER_ID,
                                                    max_value=num_customer_table_rows)

                ean13_barcode = fake_data_gen.ean13()
                quantity_purchased = fake_data_gen.pyint(min_value=MIN_PURCHASE_PRODUCT_COUNT)
                purchases_table_file.write(f"{epoch_time_purchased},{customer_id},"\
                                           f"\"{ean13_barcode}\",{quantity_purchased}\n")
                epoch_time_purchased += fake_data_gen.pyint(min_value=MIN_NS_GAP, max_value=MAX_NS_GAP)

    else:

        # If not sufficient number of arguments provided
        #   on command line for this program, print usage
        #   statement.
        print(f"\nUsage: {sys.argv[0]} "\
                f"[number of customer table rows] "\
                 f"[number of purchases table rows]\n")

## END FUNCTION DECLARATIONS SECTION ##


if __name__ == "__main__":
        main()


