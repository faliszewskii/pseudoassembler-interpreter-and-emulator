#pragma once

enum err_code {         // Lista obslugiwanych bledow
    error_none,
    error_wrong_argument,
    error_wrong_amount_of_arguments,
    error_wrong_file,
    error_unable_to_create_output_file,
    error_machine_code_reading_fail,
    error_pseudoassembler_code_reading_fail,
    error_load_labels_fail,
    error_interpret_directives_fail,
    error_interpret_orders_fail,
    error_execution_unreadable_code,
    error_execution_prohibited_operation,
    error_execution_memory_allocation_fail,
    error_parsing_memory_allocation_fail,
    error_parsing_too_many_arguments,
    error_parsing_too_few_arguments,
    error_parsing_label_starting_with_a_digit,
    error_parsing_wrong_instruction,
    error_parsing_unrecognised_data_type,
    error_parsing_forbidden_whitespaces,
    error_parsing_unrecognised_instruction,
    error_parsing_unrecognised_label,
    error_parsing_expected_address,
    error_parsing_expected_opening_bracket,
    error_parsing_expected_closing_bracket,
    error_parsing_expected_comma,
    error_parsing_expected_asterisk,
    error_parsing_wrong_address,
    error_parsing_wrong_register,
    error_parsing_expected_register,
    error_parsing_expected_values,
    error_parsing_wrong_values,

    
};

int handleError(enum err_code, int);    // Funkcja komunikujaca uzytkownika o bledach w programie. Przy pewnych bledach powoduje terminacje procesu.