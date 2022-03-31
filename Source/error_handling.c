#include "error_handling.h"
#include <stdlib.h>


int handleError(enum err_code code, int line) {

    int terminate = 0;

    printf("Error: ");

    switch (code) {
    case error_none:
        printf("No error.\n");
        return 0;
        break;
    case error_wrong_argument:
        printf("Wrong argument.\n");
        terminate = 1;
        break;
    case error_wrong_amount_of_arguments:
        printf("Wrong amount of arguments.\n");
        terminate = 1;
        break;
    case error_wrong_file:
        printf("Wrong file.\n");
        terminate = 1;
        break;
    case error_unable_to_create_output_file:
        printf("Unable to create an output file.\n");
        terminate = 1;
        break;
    case error_pseudoassembler_code_reading_fail:
        printf("Pseudoassembler code reading failure.\n");
        terminate = 1;
        break;
    case error_machine_code_reading_fail:
        printf("Machine code reading failure.\n");
        terminate = 1;
        break;
    case error_load_labels_fail:
        printf("Labels loading failure.\n");
        terminate = 1;
        break;
    case error_interpret_directives_fail:
        printf("Directives interpretation failure.\n");
        terminate = 1;
        break;
    case error_interpret_orders_fail:
        printf("Orders interpretation failure.\n");
        terminate = 1;
        break;
    case error_execution_unreadable_code:
        printf("Unable to read the machine code.\n");
        terminate = 1;
        break;
    case error_execution_prohibited_operation:
        printf("Prohibited operation (tried to access non-allocated memory, overflow or division by zero).\n");
        terminate = 1;
        break;
    case error_execution_memory_allocation_fail:
        printf("Failed to allocate memory when simulating a program.\n");
        terminate = 1;
        break;
    case error_parsing_memory_allocation_fail:
        printf("Failed to allocate memory when reading Pseudoassembler code.\n");
        terminate = 1;
        break;
    case error_parsing_too_many_arguments:
        printf("Too many arguments in line: %d.\n", line);
        break;
    case error_parsing_too_few_arguments:
        printf("Too few arguments in line: %d.\n", line);
        break;
    case error_parsing_label_starting_with_a_digit:
        printf("Label starting with a digit in line: %d.\n", line);
        break;
    case error_parsing_wrong_instruction:
        printf("Wrong instruction in line: %d.\n", line);
        break;    
    case error_parsing_unrecognised_data_type:
        printf("Unrecognised data type in line: %d.\n", line);
        break;
    case error_parsing_forbidden_whitespaces:
        printf("Forbidden whitespaces in line: %d.\n", line);
        break;
    case error_parsing_unrecognised_instruction:
        printf("Unrecognised instruction in line: %d.\n", line);
        break;
    case error_parsing_unrecognised_label:
        printf("Unrecognised label in line: %d.\n", line);
        break;
    case error_parsing_expected_address:
        printf("Expected address in line: %d.\n", line);
        break;
    case error_parsing_expected_opening_bracket:
        printf("Expected \'(\' in line: %d.\n", line);
        break;
    case error_parsing_expected_closing_bracket:
        printf("Expected \')\' in line: %d.\n", line);
        break;
    case error_parsing_expected_comma:
        printf("Expected \',\' in line: %d.\n", line);
        break;
    case error_parsing_expected_asterisk:
        printf("Expected \'*\' in line: %d.\n", line);
        break;
    case error_parsing_wrong_address:
        printf("Wrong address in line: %d.\n", line);
        break;
    case error_parsing_wrong_register:
        printf("Wrong register in line: %d.\n", line);
        break;
    case error_parsing_expected_register:
        printf("Expected register in line: %d.\n", line);
        break;
    case error_parsing_expected_values:
        printf("Expected values in line: %d.\n", line);
        break;
    case error_parsing_wrong_values:
        printf("Wrong values in line: %d.\n", line);
        break;
    default:
        printf("Wrong error code.\n");
        return 0;
        break;
    }

    if (terminate)
    {
        system("pause");
        exit(EXIT_FAILURE);
    }
        

    return 1;
}