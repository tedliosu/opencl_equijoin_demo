#!/bin/bash

#############################################################################
#
# Description - this script is responsible for generating the correct equijoin
#               result tables AFTER the "./generate_custom_data.py" script
#               finishes generating the input customer and purchases tables.
#               This way the main C equijoin program has the correct equijoin
#               results it needs to verify that it performed the equijoins
#               correctly.
#
# Author - Ted Li
#
#
#############################################################################


## GLOBAL VARIABLES DEFINITION SECTION ##

# the "true" command always returns the value programs
#   return when they exit successfully, so store that
#   value for use later when exiting this program.
EXIT_SUCCESS="$(true && echo "$?")"
# the "false" command always returns the value programs
#   return when they exit with a general error, so store that
#   value for use later when handling errors
EXIT_FAILURE="$(true && echo "$?")"

# Filepath to the customer data table to be equijoined with
#   purchases table
CUSTOMER_TABLE_FILE="./data/custom_customer_data.csv"
# Filepath to purchases table
PURCHASES_TABLE_FILE="./data/custom_purchases_data.csv"
# Filepath to correct equijoin result table containing data only about inactive customers
read CORRECT_EQUIJOIN_TABLE_INACTIVE_CUSTOMERS <<- EOF
	./data/custom_results/custom_correct_join_result_inactive_customers.csv
EOF
# Filepath to correct equijoin result table containing data only about active customers
read CORRECT_EQUIJOIN_TABLE_ACTIVE_CUSTOMERS <<- EOF
	./data/custom_results/custom_correct_join_result_active_customers.csv
EOF
# Table header for both equijoin result tables; the value of this variable
#	 is what gets written to each file storing the correct equijoin result
#	 when sqlite3 doesn't write anything to the file because the equijoin
#	 result is an empty table.
read EQUIJOIN_RESULT_TABLE_HEADER <<- EOF
	epochTimePurchased,customerID,customerName,purchaseEAN13,purchaseQuantity
EOF

## END GLOBAL VARIABLES DEFINITION SECTION ##


## BEGIN MAIN PROGRAM ##

# If sqlite3 isn't installed, then ask user to try running
#   this script after installing sqlite3.
if [[ -z $(which sqlite3) ]]; then
   printf "\nPlease try running this script again AFTER "
   printf "installing sqlite3 on your machine.\n\n"
   exit "$EXIT_FAILURE"
fi

# Notify user of next action
printf "\nGenerating correct equijoin results using sqlite3 for main\n"
printf "    C program to use for checking correctness of the C program's\n"
printf "    equijoin results; this may take some time.\n"

# Generate correct equijoin results by joining customer data table
#    and purchases data table on customerID column. The results
#    consists of two separate csv files where the first file is a
#    table consisting only of active customers and the other is a
#    table consisting only of inactive customers; sort the results
#    by timestamp of each purchase in each resulting table.
sqlite3 -batch -csv -header <<- EOF
	.output $CORRECT_EQUIJOIN_TABLE_ACTIVE_CUSTOMERS
	.import $CUSTOMER_TABLE_FILE customer_data
	.import $PURCHASES_TABLE_FILE purchases_data
	SELECT
		purchases_data.epochTimePurchased,
		customer_data.customerID,
		customer_data.customerName,
		purchases_data.purchaseEAN13,
		purchases_data.purchaseQuantity
	FROM purchases_data JOIN customer_data
			ON customer_data.customerID = purchases_data.customerID
		AND customer_data.isActiveCustomer = 'Y'
		ORDER BY purchases_data.epochTimePurchased;
	.output $CORRECT_EQUIJOIN_TABLE_INACTIVE_CUSTOMERS
	SELECT
		purchases_data.epochTimePurchased,
		customer_data.customerID,
		customer_data.customerName,
		purchases_data.purchaseEAN13,
		purchases_data.purchaseQuantity
	FROM purchases_data JOIN customer_data
			ON customer_data.customerID = purchases_data.customerID
		AND customer_data.isActiveCustomer = 'N'
		ORDER BY purchases_data.epochTimePurchased;
	.quit
EOF

# Next two if statements handle edge cases where either of the two
#    equijoin results is an empty table. sqlite3 doesn't print anything
#    to each file storing the equijoin result table if the corresponding
#    equijoin result is an empty table.
#
# As each equijoin results file is simply an empty file if sqlite3
#	 outputs an empty table to the file, just overwrite the file
#	 with the actual correct equijoin result for these edge cases.
if [[ $(stat --printf="%s" "$CORRECT_EQUIJOIN_TABLE_ACTIVE_CUSTOMERS") -eq 0 ]]; then
		printf "%s\n" "$EQUIJOIN_RESULT_TABLE_HEADER" > "$CORRECT_EQUIJOIN_TABLE_ACTIVE_CUSTOMERS"
fi
if [[ $(stat --printf="%s" "$CORRECT_EQUIJOIN_TABLE_INACTIVE_CUSTOMERS") -eq 0 ]]; then
		printf "%s\n" "$EQUIJOIN_RESULT_TABLE_HEADER" > "$CORRECT_EQUIJOIN_TABLE_INACTIVE_CUSTOMERS"
fi

printf "\n"

exit "$EXIT_SUCCESS"

## END MAIN PROGRAM ##


